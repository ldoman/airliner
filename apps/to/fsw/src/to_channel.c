/****************************************************************************
*
*   Copyright (c) 2017 Windhover Labs, L.L.C. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in
*    the documentation and/or other materials provided with the
*    distribution.
* 3. Neither the name Windhover Labs nor the names of its
*    contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
* OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
* AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
* ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
*****************************************************************************/

#include "to_app.h"
#include "to_channel.h"
#include <string.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Run the Classifier algorithm                                    */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 TO_Channel_OpenChannel(uint32 index, char *ChannelName,
		char *ConfigTableName, char *ConfigTableFileName,
		char *DumpTableName)
{
	uint32 iStatus = CFE_SUCCESS;
	char pipeName[OS_MAX_API_NAME];
	TO_ChannelData_t *channel;

	if(index >= TO_MAX_CHANNELS)
	{
		iStatus = -1;
		goto end_of_function;
	}

	channel = &TO_AppData.ChannelData[index];

	TO_Channel_LockByRef(channel);

    if(channel->State == TO_CHANNEL_CLOSED)
    {
		sprintf(pipeName, "TO_%s", ChannelName);

		strncpy(channel->ChannelName,
				ChannelName, sizeof(channel->ChannelName));
		strncpy(channel->ConfigTableName,
				ConfigTableName, sizeof(channel->ConfigTableName));
		strncpy(channel->ConfigTableFileName,
				ConfigTableFileName, sizeof(channel->ConfigTableFileName));
		strncpy(channel->DumpTableName,
				DumpTableName, sizeof(channel->DumpTableName));

		iStatus = TO_OutputQueue_Buildup(channel);
		if(iStatus != 0)
		{
			TO_Channel_UnlockByRef(channel);
			goto end_of_function;
		}

		/* Init data pipe and subscribe to messages on the data pipe */
		iStatus = CFE_SB_CreatePipe(&channel->DataPipeId,
									 TO_DATA_PIPE_DEPTH,
									 pipeName);
		if (iStatus != CFE_SUCCESS)
		{
			(void) CFE_EVS_SendEvent(TO_INIT_ERR_EID, CFE_EVS_ERROR,
									 "Failed to create channel '%s' pipe (0x%08X)",
									 ChannelName,
									 (unsigned int)iStatus);
			TO_Channel_UnlockByRef(channel);
			goto end_of_function;
		}

		channel->State = TO_CHANNEL_OPENED;
		iStatus = TO_InitConfigTbl(channel);
		if (iStatus != CFE_SUCCESS)
		{
			CFE_EVS_SendEvent(TO_INIT_ERR_EID, CFE_EVS_ERROR,
							  "Failed to init config tables (0x%08X)",
							  (unsigned int)iStatus);
			channel->State = TO_CHANNEL_CLOSED;
			TO_Channel_UnlockByRef(channel);
			goto end_of_function;
		}
    }
	TO_Channel_UnlockByRef(channel);

end_of_function:
	return iStatus;
}


void TO_Channel_ProcessTelemetryAll(void)
{
    int32 iStatus = CFE_SUCCESS;
    uint32 i = 0;

    for(i = 0; i < TO_MAX_CHANNELS; ++i)
    {
	    TO_ChannelData_t *channel = &TO_AppData.ChannelData[i];
	    if(channel->State == TO_CHANNEL_OPENED)
	    {
	    	TO_Channel_ProcessTelemetry(channel);
	    }
    }
}


void TO_Channel_ProcessTelemetry(TO_ChannelData_t *channel)
{
	TO_Channel_LockByRef(channel);
	TO_Classifier_Run(channel);
	TO_Scheduler_Run(channel);
	TO_Channel_UnlockByRef(channel);
}



void TO_Channel_ResetCountsAll(void)
{
    int32 iStatus = CFE_SUCCESS;
    uint32 i = 0;

    for(i = 0; i < TO_MAX_CHANNELS; ++i)
    {
	    TO_ChannelData_t *channel = &TO_AppData.ChannelData[i];
	    if(channel->State == TO_CHANNEL_OPENED)
	    {
	    	TO_Channel_ResetCounts(channel);
	    }
    }
}



void TO_Channel_ResetCounts(TO_ChannelData_t *channel)
{
	TO_Channel_LockByRef(channel);
	TO_MessageFlow_ResetCountsAll(channel);
	TO_PriorityQueue_ResetCountsAll(channel);
	TO_OutputQueue_ResetCounts(channel);
	TO_Channel_UnlockByRef(channel);
}

void  TO_Channel_LockByIndex(uint32 index)
{
	if(index < TO_MAX_CHANNELS)
	{
		TO_ChannelData_t *channel = &TO_AppData.ChannelData[index];
		TO_Channel_LockByRef(channel);
	}
}



void  TO_Channel_UnlockByIndex(uint32 index)
{
	if(index < TO_MAX_CHANNELS)
	{
		TO_ChannelData_t *channel = &TO_AppData.ChannelData[index];
		TO_Channel_UnlockByRef(channel);
	}
}



void  TO_Channel_LockByRef(TO_ChannelData_t *channel)
{
	OS_MutSemTake(channel->MutexID);
}



void  TO_Channel_UnlockByRef(TO_ChannelData_t *channel)
{
	OS_MutSemGive(channel->MutexID);
}



int32 TO_Channel_InitAll(void)
{
	uint32 i = 0;
	int iStatus = 0;

	for(i = 0; i < TO_MAX_CHANNELS; ++i)
	{
		iStatus = TO_Channel_Init(i);
		if(iStatus != 0)
		{
			goto end_of_function;
		}
	}

end_of_function:
	return iStatus;
}



int32 TO_Channel_Init(uint32 index)
{
	int iStatus = 0;
	char mutexName[OS_MAX_API_NAME];

	if(index >= TO_MAX_CHANNELS)
	{
		iStatus = -1;
		goto end_of_function;
	}

	TO_ChannelData_t *channel = &TO_AppData.ChannelData[index];
	snprintf(mutexName, OS_MAX_API_NAME, "TO_MUT_%u", (unsigned int)index);
	iStatus = OS_MutSemCreate(&channel->MutexID, mutexName, 0);

end_of_function:
	return iStatus;
}



void TO_Channel_CleanupAll(void)
{
	uint32 i = 0;

	TO_OutputChannel_CustomCleanupAll();

	for(i = 0; i < TO_MAX_CHANNELS; ++i)
	{
		TO_Channel_Cleanup(i);
	}

}



void TO_Channel_Cleanup(uint32 index)
{
	if(index < TO_MAX_CHANNELS)
	{
		TO_ChannelData_t *channel = &TO_AppData.ChannelData[index];

		TO_MessageFlow_TeardownAll(channel);
		TO_PriorityQueue_TeardownAll(channel);
		TO_OutputQueue_Teardown(channel);

		OS_MutSemDelete(channel->MutexID);
	}
}



uint8 TO_Channel_State(uint32 index)
{
    return TO_AppData.ChannelData[index].State;
}


