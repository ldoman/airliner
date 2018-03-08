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

/************************************************************************
** Pragmas
*************************************************************************/

/************************************************************************
** Includes
*************************************************************************/
#include <string.h>

#include "cfe.h"
#include "ccsds.h"

#include "ci_app.h"
#include "ci_msg.h"
#include "ci_events.h"
#include "ci_version.h"
#include "pb_lib.h"



/************************************************************************
** Local Defines
*************************************************************************/

/************************************************************************
** Local Structure Declarations
*************************************************************************/

/************************************************************************
** External Global Variables
*************************************************************************/

/************************************************************************
** Global Variables
*************************************************************************/
CI_AppData_t  CI_AppData;

/************************************************************************
** Local Variables
*************************************************************************/

/************************************************************************
** Local Function Definitions
*************************************************************************/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Initialize event tables                                         */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int32 CI_InitEvent()
{
    int32  iStatus=CFE_SUCCESS;
    int32  ind = 0;

    /* Initialize the event filter table.
     * Note: 0 is the CFE_EVS_NO_FILTER mask and event 0 is reserved (not used) */
    memset((void*)CI_AppData.EventTbl, 0x00, sizeof(CI_AppData.EventTbl));

    /* TODO: Choose the events you want to filter.  CFE_EVS_MAX_EVENT_FILTERS
     * limits the number of filters per app.  An explicit CFE_EVS_NO_FILTER 
     * (the default) has been provided as an example. */
    CI_AppData.EventTbl[  ind].EventID = CI_RESERVED_EID;
    CI_AppData.EventTbl[ind++].Mask    = CFE_EVS_NO_FILTER;

    CI_AppData.EventTbl[  ind].EventID = CI_INF_EID;
    CI_AppData.EventTbl[ind++].Mask    = CFE_EVS_NO_FILTER;

    CI_AppData.EventTbl[  ind].EventID = CI_CONFIG_TABLE_ERR_EID;
    CI_AppData.EventTbl[ind++].Mask    = CFE_EVS_NO_FILTER;

    CI_AppData.EventTbl[  ind].EventID = CI_CDS_ERR_EID;
    CI_AppData.EventTbl[ind++].Mask    = CFE_EVS_NO_FILTER;

    CI_AppData.EventTbl[  ind].EventID = CI_PIPE_ERR_EID;
    CI_AppData.EventTbl[ind++].Mask    = CFE_EVS_NO_FILTER;

    CI_AppData.EventTbl[  ind].EventID = CI_MSGID_ERR_EID;
    CI_AppData.EventTbl[ind++].Mask    = CFE_EVS_NO_FILTER;

    CI_AppData.EventTbl[  ind].EventID = CI_MSGLEN_ERR_EID;
    CI_AppData.EventTbl[ind++].Mask    = CFE_EVS_NO_FILTER;

    /* Register the table with CFE */
    iStatus = CFE_EVS_Register(CI_AppData.EventTbl,
                               CI_EVT_CNT, CFE_EVS_BINARY_FILTER);
    if (iStatus != CFE_SUCCESS)
    {
        (void) CFE_ES_WriteToSysLog("CI - Failed to register with EVS (0x%08X)\n", (unsigned int)iStatus);
    }

    return (iStatus);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Initialize Message Pipes                                        */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int32 CI_InitPipe()
{
    int32  iStatus=CFE_SUCCESS;

    /* Init schedule pipe and subscribe to wakeup messages */
    iStatus = CFE_SB_CreatePipe(&CI_AppData.SchPipeId,
                                 CI_SCH_PIPE_DEPTH,
                                 CI_SCH_PIPE_NAME);
    if (iStatus == CFE_SUCCESS)
    {
        iStatus = CFE_SB_SubscribeEx(CI_WAKEUP_MID, CI_AppData.SchPipeId, CFE_SB_Default_Qos, CI_SCH_PIPE_WAKEUP_RESERVED);

        if (iStatus != CFE_SUCCESS)
        {
            (void) CFE_EVS_SendEvent(CI_INIT_ERR_EID, CFE_EVS_ERROR,
                                     "Sch Pipe failed to subscribe to CI_WAKEUP_MID. (0x%08X)",
                                     (unsigned int)iStatus);
            goto CI_InitPipe_Exit_Tag;
        }

        iStatus = CFE_SB_SubscribeEx(CI_SEND_HK_MID, CI_AppData.SchPipeId, CFE_SB_Default_Qos, CI_SCH_PIPE_SEND_HK_RESERVED);

        if (iStatus != CFE_SUCCESS)
        {
            (void) CFE_EVS_SendEvent(CI_INIT_ERR_EID, CFE_EVS_ERROR,
                                     "CMD Pipe failed to subscribe to CI_SEND_HK_MID. (0x%08X)",
                                     (unsigned int)iStatus);
            goto CI_InitPipe_Exit_Tag;
        }

        iStatus = CFE_SB_SubscribeEx(CI_PROCESS_TIMEOUTS_MID, CI_AppData.SchPipeId, CFE_SB_Default_Qos, CI_SCH_PIPE_PROC_TIMEOUT_RESERVED);

		if (iStatus != CFE_SUCCESS)
		{
			(void) CFE_EVS_SendEvent(CI_INIT_ERR_EID, CFE_EVS_ERROR,
									 "CMD Pipe failed to subscribe to CI_PROCESS_TIMEOUTS_MID. (0x%08X)",
									 (unsigned int)iStatus);
			goto CI_InitPipe_Exit_Tag;
		}

    }
    else
    {
        (void) CFE_EVS_SendEvent(CI_INIT_ERR_EID, CFE_EVS_ERROR,
                                 "Failed to create SCH pipe (0x%08X)",
                                 (unsigned int)iStatus);
        goto CI_InitPipe_Exit_Tag;
    }

    /* Init command pipe and subscribe to command messages */
    iStatus = CFE_SB_CreatePipe(&CI_AppData.CmdPipeId,
                                 CI_CMD_PIPE_DEPTH,
                                 CI_CMD_PIPE_NAME);
    if (iStatus == CFE_SUCCESS)
    {
        /* Subscribe to command messages */
        iStatus = CFE_SB_Subscribe(CI_CMD_MID, CI_AppData.CmdPipeId);

        if (iStatus != CFE_SUCCESS)
        {
            (void) CFE_EVS_SendEvent(CI_INIT_ERR_EID, CFE_EVS_ERROR,
                                     "CMD Pipe failed to subscribe to CI_CMD_MID. (0x%08X)",
                                     (unsigned int)iStatus);
            goto CI_InitPipe_Exit_Tag;
        }
    }
    else
    {
        (void) CFE_EVS_SendEvent(CI_INIT_ERR_EID, CFE_EVS_ERROR,
                                 "Failed to create CMD pipe (0x%08X)",
                                 (unsigned int)iStatus);
        goto CI_InitPipe_Exit_Tag;
    }

    /* Init data pipe and subscribe to messages on the data pipe */
    iStatus = CFE_SB_CreatePipe(&CI_AppData.DataPipeId,
                                 CI_DATA_PIPE_DEPTH,
                                 CI_DATA_PIPE_NAME);
    if (iStatus == CFE_SUCCESS)
    {
        /* TODO:  Add CFE_SB_Subscribe() calls for other apps' output data here.
        **
        ** Examples:
        **     CFE_SB_Subscribe(GNCEXEC_OUT_DATA_MID, CI_AppData.DataPipeId);
        */
    }
    else
    {
        (void) CFE_EVS_SendEvent(CI_INIT_ERR_EID, CFE_EVS_ERROR,
                                 "Failed to create Data pipe (0x%08X)",
                                 (unsigned int)iStatus);
        goto CI_InitPipe_Exit_Tag;
    }

CI_InitPipe_Exit_Tag:
    return (iStatus);
}
    

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Initialize Global Variables                                     */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int32 CI_InitData()
{
    int32  iStatus=CFE_SUCCESS;

    /* Init input data */
    memset((void*)&CI_AppData.InData, 0x00, sizeof(CI_AppData.InData));

    /* Init output data */
    memset((void*)&CI_AppData.OutData, 0x00, sizeof(CI_AppData.OutData));
    CFE_SB_InitMsg(&CI_AppData.OutData,
                   CI_OUT_DATA_MID, sizeof(CI_AppData.OutData), TRUE);

    /* Init housekeeping packet */
    memset((void*)&CI_AppData.HkTlm, 0x00, sizeof(CI_AppData.HkTlm));
    CFE_SB_InitMsg(&CI_AppData.HkTlm,
                   CI_HK_TLM_MID, sizeof(CI_AppData.HkTlm), TRUE);

    return (iStatus);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* CI initialization                                              */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int32 CI_InitApp()
{
    int32  iStatus   = CFE_SUCCESS;
    int8   hasEvents = 0;
    CI_AppData.IngestActive = TRUE;

    iStatus = CI_InitEvent();
    if (iStatus != CFE_SUCCESS)
    {
        (void) CFE_ES_WriteToSysLog("CI - Failed to init events (0x%08X)\n", (unsigned int)iStatus);
        goto CI_InitApp_Exit_Tag;
    }
    else
    {
        hasEvents = 1;
    }

    iStatus = CI_InitPipe();
    if (iStatus != CFE_SUCCESS)
    {
        (void) CFE_EVS_SendEvent(CI_INIT_ERR_EID, CFE_EVS_ERROR,
                                 "Failed to init pipes (0x%08X)",
                                 (unsigned int)iStatus);
        goto CI_InitApp_Exit_Tag;
    }

    iStatus = CI_InitData();
    if (iStatus != CFE_SUCCESS)
    {
        (void) CFE_EVS_SendEvent(CI_INIT_ERR_EID, CFE_EVS_ERROR,
                                 "Failed to init data (0x%08X)",
                                 (unsigned int)iStatus);
        goto CI_InitApp_Exit_Tag;
    }

    iStatus = CI_InitTbls();
    if (iStatus != CFE_SUCCESS)
    {
        (void) CFE_EVS_SendEvent(CI_INIT_ERR_EID, CFE_EVS_ERROR,
                                 "Failed to init config tables (0x%08X)",
                                 (unsigned int)iStatus);
        goto CI_InitApp_Exit_Tag;
    }

    iStatus = CI_InitCdsTbl();
    if (iStatus != CFE_SUCCESS)
    {
        (void) CFE_EVS_SendEvent(CI_INIT_ERR_EID, CFE_EVS_ERROR,
                                 "Failed to init CDS table (0x%08X)",
                                 (unsigned int)iStatus);
        goto CI_InitApp_Exit_Tag;
    }

    /* Initialize custom code. */
    iStatus = CI_InitCustom();
    if(iStatus != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("CI - InitCustom failed (%i)\n", (int)iStatus);
        goto CI_InitApp_Exit_Tag;
    }

    /* Register the cleanup callback */
    iStatus = OS_TaskInstallDeleteHandler(&CI_CleanupCallback);
    if (iStatus != CFE_SUCCESS)
    {
        (void) CFE_EVS_SendEvent(CI_INIT_ERR_EID, CFE_EVS_ERROR,
                                 "Failed to init register cleanup callback (0x%08X)",
                                 (unsigned int)iStatus);
        goto CI_InitApp_Exit_Tag;
    }

    /* Initialize listener child task. */
    iStatus = CI_InitListenerTask();
    if(iStatus != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("CI - Failed to initialize listener child task (0x%08X)\n", (unsigned int)iStatus);
        goto CI_InitApp_Exit_Tag;
    }

    /* Register the cleanup callback */
    OS_TaskInstallDeleteHandler((void*)&CI_CleanupCallback);

CI_InitApp_Exit_Tag:
    if (iStatus == CFE_SUCCESS)
    {
        (void) CFE_EVS_SendEvent(CI_INIT_INF_EID, CFE_EVS_INFORMATION,
                                 "Initialized.  Version %d.%d.%d.%d",
                                 CI_MAJOR_VERSION,
                                 CI_MINOR_VERSION,
                                 CI_REVISION,
                                 CI_MISSION_REV);
    }
    else
    {
        if (hasEvents == 1)
        {
            (void) CFE_EVS_SendEvent(CI_INIT_ERR_EID, CFE_EVS_ERROR, "Application failed to initialize");
        }
        else
        {
            (void) CFE_ES_WriteToSysLog("CI - Application failed to initialize\n");
        }
    }

    return (iStatus);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* CI Cleanup                                                     */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void CI_CleanupCallback()
{
    CI_CleanupCustom();
    CI_AppData.IngestActive = FALSE;
	OS_MutSemGive(CI_AppData.ConfigTblMutex);
	OS_MutSemGive(CI_AppData.TimeoutTblMutex);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Get Command Table Data Row                                 	   */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

CI_CmdData_t *CI_GetRegisterdCmd(CFE_SB_MsgId_t msgID, uint16 cmdCode)
{
	uint32 				i = 0;
	CI_CmdData_t		*CmdData = NULL;

	if(msgID == 0)
	{
		goto CI_GetRegisterdCmd_Exit_Tag;
	}

	/* Lock the mutex */
	OS_MutSemTake(CI_AppData.ConfigTblMutex);

	/* Get command from config table with same params */
	for(i = 0; i < CI_MAX_RGST_CMDS; ++i)
	{
		if(CI_AppData.ConfigTblPtr->cmds[i].mid == msgID)
		{
			if(CI_AppData.ConfigTblPtr->cmds[i].code == cmdCode)
			{
				CmdData = &CI_AppData.ConfigTblPtr->cmds[i];
				goto CI_GetRegisterdCmd_Exit_Tag;
			}
		}
	}

CI_GetRegisterdCmd_Exit_Tag:
	/* Unlock the mutex */
	OS_MutSemGive(CI_AppData.ConfigTblMutex);

	return CmdData;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Get Command Table Data Row Index	                          	   */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

uint32 CI_GetRegisterdCmdIdx(CFE_SB_MsgId_t msgID, uint16 cmdCode)
{
	uint32 				i = -1;

	if(msgID == 0)
	{
		goto CI_GetRegisterdCmdIdx_Exit_Tag;
	}

	/* Lock the mutex */
	OS_MutSemTake(CI_AppData.ConfigTblMutex);

	/* Get command from config table with same params */
	for(i = 0; i < CI_MAX_RGST_CMDS; ++i)
	{
		if(CI_AppData.ConfigTblPtr->cmds[i].mid == msgID)
		{
			if(CI_AppData.ConfigTblPtr->cmds[i].code == cmdCode)
			{
				goto CI_GetRegisterdCmdIdx_Exit_Tag;
			}
		}
	}

	i = -1;

CI_GetRegisterdCmdIdx_Exit_Tag:
	/* Unlock the mutex */
	OS_MutSemGive(CI_AppData.ConfigTblMutex);

	return i;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Authorize Command			                                   */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void CI_CmdAuthorize(CFE_SB_Msg_t* MsgPtr)
{
	uint16              ExpectedLength = sizeof(CI_CmdAuthData_t);
	CI_CmdData_t		*CmdData = NULL;
	CI_CmdAuthData_t 	*authDataPtr;
	uint32				i = -1;

	/* Verify command packet length */
	if (CI_VerifyCmdLength (MsgPtr, ExpectedLength))
	{
		/* Check if command is not registered */
		authDataPtr = ((CI_CmdAuthData_t *) MsgPtr);
		CmdData = CI_GetRegisterdCmd(authDataPtr->msgID, authDataPtr->cmdCode);
		if (CmdData == NULL)
		{
			CFE_EVS_SendEvent (CI_CMD_AUTH_NOT_REG_EID, CFE_EVS_ERROR, "Cmd (%i) not registered for msgId (0x%04X)",
								authDataPtr->cmdCode, (unsigned short)authDataPtr->msgID);
			CI_AppData.HkTlm.usCmdErrCnt++;
			return;
		}

		/* Check if command is not 2-step */
		if (CmdData->step != STEP_2)
		{
			CFE_EVS_SendEvent (CI_CMD_AUTH_INV_MODE_EID, CFE_EVS_ERROR, "Cmd (%i) for msgId (0x%04X) not 2-step",
								authDataPtr->cmdCode, (unsigned short)authDataPtr->msgID);
			CI_AppData.HkTlm.usCmdErrCnt++;
			return;
		}

		/* Check if command is already authorized */
		if (CmdData->state == AUTHORIZED)
		{
			CFE_EVS_SendEvent (CI_CMD_AUTH_INV_STATE_EID, CFE_EVS_ERROR, "Cmd (%i) already authorized for msgId (0x%04X)",
								authDataPtr->cmdCode, (unsigned short)authDataPtr->msgID);
			CI_AppData.HkTlm.usCmdErrCnt++;
			return;
		}

		i = CI_GetRegisterdCmdIdx(authDataPtr->msgID, authDataPtr->cmdCode);

		/* Lock the mutexes */
		OS_MutSemTake(CI_AppData.ConfigTblMutex);
		OS_MutSemTake(CI_AppData.TimeoutTblMutex);

		/* Update values */
		CmdData->state = AUTHORIZED;
		CI_AppData.TimeoutTbl.time[i] = CI_CMD_MAX_TIMEOUT;

		/* Unlock the mutexes */
		OS_MutSemGive(CI_AppData.ConfigTblMutex);
		OS_MutSemGive(CI_AppData.TimeoutTblMutex);

		CFE_EVS_SendEvent (CI_CMD_AUTHORIZED_EID, CFE_EVS_INFORMATION, "Cmd (%i) authorized for msgId (0x%04X)",
							authDataPtr->cmdCode, (unsigned short)authDataPtr->msgID);
		CI_AppData.HkTlm.usCmdCnt++;
	}

	return;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Deauthorize Command 				                               */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void CI_CmdDeauthorize(CFE_SB_Msg_t* MsgPtr)
{
	uint16              ExpectedLength = sizeof(CI_CmdAuthData_t);
	CI_CmdData_t		*CmdData = NULL;
	CI_CmdAuthData_t 	*authDataPtr;
	uint32				i = -1;

	/* Verify command packet length */
	if (CI_VerifyCmdLength (MsgPtr, ExpectedLength))
	{
		/* Check if command is not registered */
		authDataPtr = ((CI_CmdAuthData_t *) MsgPtr);
		CmdData = CI_GetRegisterdCmd(authDataPtr->msgID, authDataPtr->cmdCode);
		if (CmdData == NULL)
		{
			CFE_EVS_SendEvent (CI_CMD_DEAUTH_NOT_REG_EID, CFE_EVS_ERROR, "Cmd (%i) not registered for msgId (0x%04X)",
								authDataPtr->cmdCode, (unsigned short)authDataPtr->msgID);
			CI_AppData.HkTlm.usCmdErrCnt++;
			return;
		}

		/* Check if command is not 2-step */
		if (CmdData->step != STEP_2)
		{
			CFE_EVS_SendEvent (CI_CMD_DEAUTH_INV_MODE_EID, CFE_EVS_ERROR, "Cmd (%i) for msgId (0x%04X) not 2-step",
								authDataPtr->cmdCode, (unsigned short)authDataPtr->msgID);
			CI_AppData.HkTlm.usCmdErrCnt++;
			return;
		}

		/* Check if command is already unauthorized */
		if (CmdData->state == UNAUTHORIZED)
		{
			CFE_EVS_SendEvent (CI_CMD_DEAUTH_INV_STATE_EID, CFE_EVS_ERROR, "Cmd (%i) for msgId (0x%04X) not authorized",
								authDataPtr->cmdCode, (unsigned short)authDataPtr->msgID);
			CI_AppData.HkTlm.usCmdErrCnt++;
			return;
		}

		i = CI_GetRegisterdCmdIdx(authDataPtr->msgID, authDataPtr->cmdCode);

		/* Lock the mutexes */
		OS_MutSemTake(CI_AppData.ConfigTblMutex);
		OS_MutSemTake(CI_AppData.TimeoutTblMutex);

		/* Update values */
		CmdData->state = UNAUTHORIZED;
		CI_AppData.TimeoutTbl.time[i] = 0;

		/* Unlock the mutexes */
		OS_MutSemGive(CI_AppData.ConfigTblMutex);
		OS_MutSemGive(CI_AppData.TimeoutTblMutex);

		CFE_EVS_SendEvent (CI_CMD_DEAUTHORIZED_EID, CFE_EVS_INFORMATION, "Cmd (%i) deauthorized for msgId (0x%04X)",
							authDataPtr->cmdCode, (unsigned short)authDataPtr->msgID);
		CI_AppData.HkTlm.usCmdCnt++;
	}


	return;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Register Command				                                   */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void CI_CmdRegister(CFE_SB_Msg_t* MsgPtr)
{
	uint16              ExpectedLength = sizeof(CI_CmdRegData_t);
	CI_CmdData_t		*CmdData = NULL;
	CI_CmdRegData_t 	*regDataPtr;
	uint32 				i = 0;

	/* Verify command packet length */
	if (CI_VerifyCmdLength (MsgPtr, ExpectedLength))
	{
		/* Verify msgID is nonzero */
		regDataPtr = ((CI_CmdRegData_t *) MsgPtr);
		if (regDataPtr->msgID != 0)
		{
			/* Check if command is registered */
			CmdData = CI_GetRegisterdCmd(regDataPtr->msgID, regDataPtr->cmdCode);
			if (CmdData == NULL)
			{
				/* Lock the config mutex */
				OS_MutSemTake(CI_AppData.ConfigTblMutex);

				/* Find first unused index in config table to add command */
				for(i = 0; i < CI_MAX_RGST_CMDS; ++i)
				{
					if(CI_AppData.ConfigTblPtr->cmds[i].mid == 0)
					{
						CmdData = &CI_AppData.ConfigTblPtr->cmds[i];
						break;
					}
				}

				/* If entry isn't NULL set to passed params */
				if(CmdData != NULL)
				{
					/* Lock the timeout mutex */
					OS_MutSemTake(CI_AppData.TimeoutTblMutex);

					/* Update values */
					CmdData->mid = regDataPtr->msgID;
					CmdData->code = regDataPtr->cmdCode;
					CmdData->step = regDataPtr->step;
					CmdData->state = UNAUTHORIZED;
					CmdData->RouteCount = 0;
					CmdData->log = regDataPtr->log;
					CI_AppData.TimeoutTbl.time[i] = 0;

					/* Unlock the timeout mutex */
					OS_MutSemGive(CI_AppData.TimeoutTblMutex);

					CFE_EVS_SendEvent (CI_CMD_REGISTERED_EID, CFE_EVS_INFORMATION, "Cmd (%i) registered for msgId (0x%04X)",
										regDataPtr->cmdCode, (unsigned short)regDataPtr->msgID);
					CI_AppData.HkTlm.usCmdCnt++;
				}
				else
				{
					CFE_EVS_SendEvent (CI_CMD_REG_ERR_EID, CFE_EVS_ERROR, "Unable to register cmd (%i) for msgId (0x%04X)",
										regDataPtr->cmdCode, (unsigned short)regDataPtr->msgID);
					CI_AppData.HkTlm.usCmdErrCnt++;
				}

				/* Unlock the config mutex */
				OS_MutSemGive(CI_AppData.ConfigTblMutex);
			}
			else
			{
				CFE_EVS_SendEvent (CI_CMD_ALREADY_REGISTERED_EID, CFE_EVS_ERROR, "Cmd (%i) already registered for msgId (0x%04X)",
									regDataPtr->cmdCode, (unsigned short)regDataPtr->msgID);
				CI_AppData.HkTlm.usCmdErrCnt++;
			}
		}
		else
		{
			CFE_EVS_SendEvent (CI_CMD_REG_ERR_EID, CFE_EVS_ERROR, "Unable to register cmd (%i) for msgId (0x%04X)",
								regDataPtr->cmdCode, (unsigned short)regDataPtr->msgID);
			CI_AppData.HkTlm.usCmdErrCnt++;
		}
	}

	return;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Deregister Command			                                   */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void CI_CmdDeregister(CFE_SB_Msg_t* MsgPtr)
{
	uint16              ExpectedLength = sizeof(CI_CmdAuthData_t);
	CI_CmdData_t		*CmdData = NULL;
	CI_CmdAuthData_t 	*regDataPtr;
	uint32 				i = 0;

	/* Verify command packet length */
	if (CI_VerifyCmdLength (MsgPtr, ExpectedLength))
	{
		/* Verify msgID is nonzero */
		regDataPtr = ((CI_CmdAuthData_t *) MsgPtr);
		if (regDataPtr->msgID != 0)
		{
			/* Check if command is registered */
			CmdData = CI_GetRegisterdCmd(regDataPtr->msgID, regDataPtr->cmdCode);
			if (CmdData != NULL)
			{
				i = CI_GetRegisterdCmdIdx(regDataPtr->msgID, regDataPtr->cmdCode);

				/* Lock the mutex */
				OS_MutSemTake(CI_AppData.ConfigTblMutex);
				OS_MutSemTake(CI_AppData.TimeoutTblMutex);

				/* Update values */
				CmdData->mid = 0;
				CmdData->code = 0;
				CmdData->step = 0;
				CmdData->state = 0;
				CmdData->RouteCount = 0;
				CmdData->log = 0;
				CI_AppData.TimeoutTbl.time[i] = 0;

				/* Unlock the mutex */
				OS_MutSemGive(CI_AppData.ConfigTblMutex);
				OS_MutSemGive(CI_AppData.TimeoutTblMutex);
				CFE_EVS_SendEvent (CI_CMD_DEREGISTERED_EID, CFE_EVS_INFORMATION, "Cmd (%i) deregistered for msgId (0x%04X)",
									regDataPtr->cmdCode, (unsigned short)regDataPtr->msgID);
				CI_AppData.HkTlm.usCmdCnt++;
			}
			else
			{
				CFE_EVS_SendEvent (CI_CMD_NOT_REGISTERED_EID, CFE_EVS_ERROR, "Cmd (%i) not registered for msgId (0x%04X)",
									regDataPtr->cmdCode, (unsigned short)regDataPtr->msgID);
				CI_AppData.HkTlm.usCmdErrCnt++;
			}
		}
		else
		{
			CFE_EVS_SendEvent (CI_CMD_REG_ERR_EID, CFE_EVS_ERROR, "Unable to deregister cmd (%i) for msgId (0x%04X)",
								regDataPtr->cmdCode, (unsigned short)regDataPtr->msgID);
			CI_AppData.HkTlm.usCmdErrCnt++;
		}
	}

	return;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Update Registered Commands	                                   */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void CI_UpdateCmdReg(CFE_SB_Msg_t* MsgPtr)
{
	uint16              ExpectedLength = sizeof(CI_CmdRegData_t);
	CI_CmdData_t		*CmdData = NULL;
	CI_CmdRegData_t 	*regDataPtr;
	uint32 				i = 0;

	/* Verify command packet length */
	if (CI_VerifyCmdLength (MsgPtr, ExpectedLength))
	{
		/* Verify msgID is nonzero */
		regDataPtr = ((CI_CmdRegData_t *) MsgPtr);
		if (regDataPtr->msgID != 0)
		{
			/* Check if command is registered */
			CmdData = CI_GetRegisterdCmd(regDataPtr->msgID, regDataPtr->cmdCode);
			if (CmdData != NULL)
			{
				if (CmdData->step == STEP_2)
				{
					if (CmdData->state == AUTHORIZED)
					{
						CFE_EVS_SendEvent (CI_CMD_UPDT_REG_INVLD_STATE_EID,
											CFE_EVS_ERROR,
											"Invalid state of cmd (%i) for msgId (0x%04X) to update",
											regDataPtr->cmdCode, (unsigned short)regDataPtr->msgID);
						CI_AppData.HkTlm.usCmdErrCnt++;
						goto CI_UpdateCmdReg_Exit_Tag;
					}
				}

				i = CI_GetRegisterdCmdIdx(regDataPtr->msgID, regDataPtr->cmdCode);

				/* Lock the mutex */
				OS_MutSemTake(CI_AppData.ConfigTblMutex);
				OS_MutSemTake(CI_AppData.TimeoutTblMutex);

				/* Update values */
				CmdData->mid = regDataPtr->msgID;
				CmdData->code = regDataPtr->cmdCode;
				CmdData->step = regDataPtr->step;
				CmdData->state = UNAUTHORIZED;
				CmdData->RouteCount = 0;
				CmdData->log = regDataPtr->log;
				CI_AppData.TimeoutTbl.time[i] = 0;

				/* Unlock the mutex */
				OS_MutSemGive(CI_AppData.ConfigTblMutex);
				OS_MutSemGive(CI_AppData.TimeoutTblMutex);

				CFE_EVS_SendEvent (CI_CMD_UPDATE_REG_EID, CFE_EVS_INFORMATION, "Cmd (%i) for msgId (0x%04X) updated",
									regDataPtr->cmdCode, (unsigned short)regDataPtr->msgID);
				CI_AppData.HkTlm.usCmdCnt++;
			}
			else
			{
				CFE_EVS_SendEvent (CI_CMD_NOT_REGISTERED_EID, CFE_EVS_ERROR, "Cmd (%i) not registered for msgId (0x%04X)",
									regDataPtr->cmdCode, (unsigned short)regDataPtr->msgID);
				CI_AppData.HkTlm.usCmdErrCnt++;
			}
		}
		else
		{
			CFE_EVS_SendEvent (CI_CMD_REG_ERR_EID, CFE_EVS_ERROR, "Unable to update cmd (%i) for msgId (0x%04X)",
								regDataPtr->cmdCode, (unsigned short)regDataPtr->msgID);
			CI_AppData.HkTlm.usCmdErrCnt++;
		}
	}

CI_UpdateCmdReg_Exit_Tag:
	return;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Process Authorized Command Timeouts                             */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void CI_ProcessTimeouts(void)
{
	uint32 			i = 0;
	CI_CmdData_t	*CmdData = NULL;

	/* Lock the timeout table mutex */
	OS_MutSemTake(CI_AppData.TimeoutTblMutex);

	/* Iterate over table and decrement all authorized cmd timeouts */
	for(i = 0; i < CI_MAX_RGST_CMDS; ++i)
	{
		if(CI_AppData.TimeoutTbl.time[i] > 0)
		{
			CI_AppData.TimeoutTbl.time[i]--;
			if(CI_AppData.TimeoutTbl.time[i] == 0)
			{
				/* Lock the config table mutex */
				OS_MutSemTake(CI_AppData.ConfigTblMutex);

				CI_AppData.ConfigTblPtr->cmds[i].state = UNAUTHORIZED;
				CFE_EVS_SendEvent (CI_CMD_AUTH_TIMEOUT_EID,
								   CFE_EVS_INFORMATION,
								   "Cmd (%u) for msgId (0x%04X) authorization timeout",
								   CI_AppData.ConfigTblPtr->cmds[i].code,
								   CI_AppData.ConfigTblPtr->cmds[i].mid);

				/* Unlock the config table mutex */
				OS_MutSemGive(CI_AppData.ConfigTblMutex);
			}
		}
	}

	/* Unlock the timeout table mutex */
	OS_MutSemGive(CI_AppData.TimeoutTblMutex);

	return;
}



boolean CI_ValidatePriHdr(CFE_SB_Msg_t* MsgPtr, uint32 MsgSize)
{
	boolean 			bResult = FALSE;
	CCSDS_CmdPkt_t		*cmdPkt = 0;


	/* Verify CCSDS version */
	if (CCSDS_RD_VERS(MsgPtr->Hdr) != 0)
	{
		goto CI_ValidatePriHdr_Exit_Tag;
	}

	/* Verify secondary header present */
	if (CCSDS_RD_SHDR(MsgPtr->Hdr) == 0)
	{
		goto CI_ValidatePriHdr_Exit_Tag;
	}

	/* Verify packet type is cmd */
	if ((CCSDS_RD_TYPE(MsgPtr->Hdr) != CCSDS_CMD) &&
		(CCSDS_RD_TYPE(MsgPtr->Hdr) != CCSDS_TLM))
	{
		goto CI_ValidatePriHdr_Exit_Tag;
	}

	bResult = TRUE;

CI_ValidatePriHdr_Exit_Tag:
	return bResult;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Determine Validity of Command                                   */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

boolean CI_ValidateCmd(CFE_SB_Msg_t* MsgPtr, uint32 MsgSize)
{
	boolean 			bResult = FALSE;
	uint32  			usMsgLen = 0;
	CCSDS_CmdPkt_t		*cmdPkt = 0;

	/* Verify packet type is cmd */
	if (CCSDS_RD_TYPE(MsgPtr->Hdr) != CCSDS_CMD)
	{
		goto CI_ValidateCmd_Exit_Tag;
	}

	/* Verify length */
	usMsgLen = CFE_SB_GetTotalMsgLength(MsgPtr);
	if (usMsgLen != MsgSize)
	{
		goto CI_ValidateCmd_Exit_Tag;
	}

	/* Verify valid checksum */
	cmdPkt = (CCSDS_CmdPkt_t *)MsgPtr;
	if (CCSDS_RD_CHECKSUM(cmdPkt->SecHdr) != 0)
	{
		if (CFE_SB_ValidateChecksum((CFE_SB_MsgPtr_t)MsgPtr) != TRUE)
		{
			goto CI_ValidateCmd_Exit_Tag;
		}
	}
	else
	{
		/* If no checksum present check the CI_CHECKSUM_REQUIRED
		 * macro defined in the mission config */
		if (CI_CHECKSUM_REQUIRED == 1)
		{
			goto CI_ValidateCmd_Exit_Tag;
		}
	}

	bResult = TRUE;

CI_ValidateCmd_Exit_Tag:
	return bResult;
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Determine Validity of Telemetry                                 */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

boolean CI_ValidateTlm(CFE_SB_Msg_t* MsgPtr, uint32 MsgSize)
{
	boolean 			bResult = FALSE;
	uint32  			usMsgLen = 0;
	CCSDS_CmdPkt_t		*cmdPkt = 0;

	/* Verify packet type is telemetry */
	if (CCSDS_RD_TYPE(MsgPtr->Hdr) != CCSDS_TLM)
	{
		goto CI_ValidateTlm_Exit_Tag;
	}

	/* Verify length */
	usMsgLen = CFE_SB_GetTotalMsgLength(MsgPtr);
	if (usMsgLen != MsgSize)
	{
		goto CI_ValidateTlm_Exit_Tag;
	}

	bResult = TRUE;

CI_ValidateTlm_Exit_Tag:
	return bResult;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Lookup Command Authorization                                    */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

boolean CI_GetCmdAuthorized(CFE_SB_Msg_t* MsgPtr)
{
	boolean 			bResult = FALSE;
	CFE_SB_MsgId_t  	msgID = 0;
	CI_CmdData_t		*CmdData = NULL;
	uint16 				cmdCode = 0;
	uint32 				i = 0;

	/* Check if command is not registered */
	msgID = CFE_SB_GetMsgId(MsgPtr);
	cmdCode = CFE_SB_GetCmdCode(MsgPtr);
	CmdData = CI_GetRegisterdCmd(msgID, cmdCode);
	if (CmdData == NULL)
	{
		if (CI_AppData.IngestBehavior == BHV_OPTIMISTIC)
		{
			bResult = TRUE;
		}

		goto CI_GetCmdAuthorized_Exit_tag;
	}

	/* Check command stepping */
	if (CmdData->step == STEP_1)
	{
		bResult = TRUE;
		goto CI_GetCmdAuthorized_Exit_tag;
	}

	/* Check if command is authorized */
	if (CmdData->state == AUTHORIZED)
	{
		bResult = TRUE;

		i = CI_GetRegisterdCmdIdx(msgID, cmdCode);

		/* Lock the mutexes */
		OS_MutSemTake(CI_AppData.ConfigTblMutex);
		OS_MutSemTake(CI_AppData.TimeoutTblMutex);

		/* Update values */
		CmdData->state = UNAUTHORIZED;
		CI_AppData.TimeoutTbl.time[i] = 0;

		/* Unlock the mutexes */
		OS_MutSemGive(CI_AppData.ConfigTblMutex);
		OS_MutSemGive(CI_AppData.TimeoutTblMutex);

		goto CI_GetCmdAuthorized_Exit_tag;
	}

CI_GetCmdAuthorized_Exit_tag:
	if (bResult == FALSE)
	{
		CI_AppData.HkTlm.IngestErrorCount++;
		CFE_EVS_SendEvent (CI_CMD_UNAUTHORIZED_EID, CFE_EVS_ERROR, "Cmd (%i) not authorized for msgId (0x%04X)", cmdCode, msgID);
	}
	return bResult;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Determine Validity of Serialized Command                        */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

boolean CI_ValidateSerialCmd(CFE_SB_Msg_t* MsgPtr)
{
	boolean 			bResult = FALSE;

	/* Verify CCSDS version */
	if (CCSDS_RD_VERS(MsgPtr->Hdr) != 0)
	{
		goto CI_ValidateSerialCmd_Exit_Tag;
	}

	/* Verify secondary header present */
	if (CCSDS_RD_SHDR(MsgPtr->Hdr) == 0)
	{
		goto CI_ValidateSerialCmd_Exit_Tag;
	}

	/* Verify packet type is cmd */
	if (CCSDS_RD_TYPE(MsgPtr->Hdr) != CCSDS_CMD)
	{
		//goto CI_ValidateSerialCmd_Exit_Tag; // TODO: commenting to allow sending serialized tlm
	}

	bResult = TRUE;

CI_ValidateSerialCmd_Exit_Tag:
	return bResult;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Deserialize Message                                    		   */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

uint32 CI_DeserializeMsg(CFE_SB_MsgPtr_t CmdMsgPtr)
{
	CFE_SB_MsgId_t  	msgId = 0;
	uint32  			msgSize = 0;
	uint16 				cmdCode = 0;
	uint32  			payloadSize = 0;
	uint32  			hdrSize = 0;
	boolean				valid = TRUE;
	PBLib_DecodeFuncPtr_t	decodeFunc = 0;
	char				decodeBuf[CI_MAX_ENC_LEN] = "\0";

	msgSize = CFE_SB_GetTotalMsgLength(CmdMsgPtr);
	valid = CI_ValidateSerialCmd(CmdMsgPtr);
	if(!valid)
	{
		CFE_EVS_SendEvent (CI_CMD_INVALID_EID, CFE_EVS_ERROR, "Rcvd invalid cmd for deserialization");
		msgSize = 0;
		goto CI_DeserializeMsg_Exit_Tag;
	}

	/* Get required params */
	msgId = CFE_SB_GetMsgId(CmdMsgPtr);
	hdrSize = CFE_SB_MsgHdrSize(msgId);
	cmdCode = CFE_SB_GetCmdCode(CmdMsgPtr);
	payloadSize = CFE_SB_GetUserDataLength(CmdMsgPtr);

	/* Get deserialization function from PBL */
	decodeFunc = PBLIB_GetCmdDeserializationFunc(msgId, cmdCode);
	if(decodeFunc == 0)
	{
		msgSize = 0;
		CFE_EVS_SendEvent (CI_NO_DECODE_FUNC_EID, CFE_EVS_ERROR, "MsgId (0x%04X) cmd (%i) has no deserialization function",
							msgId, cmdCode);
		goto CI_DeserializeMsg_Exit_Tag;
	}

	/* Copy message payload into expected format */
	memcpy(decodeBuf, CFE_SB_GetUserData(CmdMsgPtr), payloadSize);

	/* Call decode function */
	payloadSize = decodeFunc(decodeBuf, CI_MAX_ENC_LEN - hdrSize, CmdMsgPtr);
	msgSize = payloadSize;

	/* Create new SB msg from deserialized data */
	CFE_SB_InitMsg(CmdMsgPtr, msgId, msgSize, FALSE);
	CFE_SB_SetTotalMsgLength(CmdMsgPtr, msgSize);
	CFE_SB_GenerateChecksum(CmdMsgPtr);

CI_DeserializeMsg_Exit_Tag:
	return msgSize;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Spawn Child Task				                                   */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int32 CI_InitListenerTask(void)
{
    int32 Status = CFE_SUCCESS;

    /* Create mutex for config table */
    Status = OS_MutSemCreate(&CI_AppData.ConfigTblMutex, CI_CFG_TBL_MUTEX_NAME, 0);
	if (Status != CFE_SUCCESS)
	{
		goto CI_InitListenerTask_Exit_Tag;
	}

	/* Create mutex for timeout dump table */
	Status = OS_MutSemCreate(&CI_AppData.TimeoutTblMutex, CI_TIME_TBL_MUTEX_NAME, 0);
	if (Status != CFE_SUCCESS)
	{
		goto CI_InitListenerTask_Exit_Tag;
	}

	Status= CFE_ES_CreateChildTask(&CI_AppData.ListenerTaskID,
								   CI_LISTENER_TASK_NAME,
								   CI_ListenerTaskMain,
								   NULL,
								   CI_LISTENER_TASK_STACK_SIZE,
								   CI_LISTENER_TASK_PRIORITY,
								   0);

	if (Status != CFE_SUCCESS)
	{
		goto CI_InitListenerTask_Exit_Tag;
	}

	Status= CFE_ES_CreateChildTask(&CI_AppData.SerialListenerTaskID,
									CI_SERIAL_LISTENER_TASK_NAME,
									CI_SerializedListenerTaskMain,
									NULL,
									CI_SERIAL_LISTENER_TASK_STACK_SIZE,
									CI_SERIAL_LISTENER_TASK_PRIORITY,
									0);

	if (Status != CFE_SUCCESS)
	{
		goto CI_InitListenerTask_Exit_Tag;
	}

CI_InitListenerTask_Exit_Tag:
	if (Status != CFE_SUCCESS)
	{
		CFE_EVS_SendEvent (CI_LISTENER_CREATE_CHDTASK_ERR_EID,
						   CFE_EVS_ERROR,
						   "Listener child task failed.  CFE_ES_CreateChildTask returned: 0x%08lX",
						   Status);
	}

    return Status;
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Process Ingest Cmd			                                   */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void CI_ProcessIngestCmd(CFE_SB_MsgPtr_t CmdMsgPtr, uint32 MsgSize)
{
    CFE_SB_MsgId_t  CmdMsgId = 0;

	if(MsgSize > 0)
	{
		/* If number of bytes received less than max */
		if (MsgSize <= CI_MAX_CMD_INGEST)
		{
            if(CI_ValidatePriHdr(CmdMsgPtr, MsgSize) == TRUE)
			{
				uint32 msgType = CCSDS_RD_TYPE(CmdMsgPtr->Hdr);

#ifdef CI_ALLOW_TELEMETRY
		        if(msgType == CCSDS_TLM)
		        {
					if (CI_ValidateTlm(CmdMsgPtr, MsgSize) == TRUE)
					{
						CFE_ES_PerfLogEntry(CI_SOCKET_RCV_PERF_ID);
						CI_AppData.HkTlm.IngestMsgCount++;
						CFE_SB_SendMsg(CmdMsgPtr);
						CFE_ES_PerfLogExit(CI_SOCKET_RCV_PERF_ID);
					}
		        }
#endif

				if(msgType == CCSDS_CMD)
				{
					/* Verify validity of cmd */
					if (CI_ValidateCmd(CmdMsgPtr, MsgSize) == TRUE)
					{
						/* Check if cmd is for CI and route if so */
						CmdMsgId = CFE_SB_GetMsgId(CmdMsgPtr);
						if (CI_CMD_MID == CmdMsgId)
						{
							CI_ProcessNewAppCmds(CmdMsgPtr);
						}
						else
						{
							/* Verify cmd is authorized */
							if (CI_GetCmdAuthorized(CmdMsgPtr) == TRUE)
							{
								CFE_ES_PerfLogEntry(CI_SOCKET_RCV_PERF_ID);
								CI_AppData.HkTlm.IngestMsgCount++;
								CFE_SB_SendMsg(CmdMsgPtr);
								CFE_ES_PerfLogExit(CI_SOCKET_RCV_PERF_ID);
							}
						}
					}
				}
			}
			else
			{
				CI_AppData.HkTlm.usCmdErrCnt++;
				CFE_EVS_SendEvent (CI_CMD_INVALID_EID, CFE_EVS_ERROR, "Rcvd invalid cmd (%i) for msgId (0x%04X)",
									CFE_SB_GetCmdCode(CmdMsgPtr), CmdMsgId);
			}
		}
		else
		{
			CI_AppData.HkTlm.IngestErrorCount++;
			CFE_EVS_SendEvent(CI_CMD_INGEST_ERR_EID, CFE_EVS_ERROR,
							  "Message too long.  Size = %lu", MsgSize);
		}
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Child Task Listener Main		                                   */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void CI_ListenerTaskMain(void)
{
    int32 			Status = -1;
    uint32  		MsgSize = 0;
    CFE_SB_MsgPtr_t CmdMsgPtr;
    CFE_SB_MsgId_t  CmdMsgId;

    PBLIB_RegisterCmdMessage(7209, 2, "EA_StartCmd_t");
    //PBLIB_RegisterCmdMessage(6150, 18, "CFE_ES_OverWriteSysLogCmd_t");

	Status = CFE_ES_RegisterChildTask();
	if (Status == CFE_SUCCESS)
	{
		/* Ingest Loop */
		do{
			/* Receive cmd and gather data on it */
			MsgSize = CI_MAX_CMD_INGEST;
			CI_ReadMessage(CI_AppData.IngestBuffer, &MsgSize);
			CmdMsgPtr = (CFE_SB_MsgPtr_t)CI_AppData.IngestBuffer;

			/* Process the cmd */
			CI_ProcessIngestCmd(CmdMsgPtr, MsgSize);

			/* Wait before next iteration */
			OS_TaskDelay(100);
		}while(CI_AppData.IngestActive == TRUE);

		CFE_ES_ExitChildTask();
	}
	else
	{
		/* Can't send event or write to syslog because this task isn't registered with the cFE. */
		OS_printf("CI Listener Child Task Registration failed!\n");
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Child Task Serialized Listener Main		                       */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void CI_SerializedListenerTaskMain(void)
{
    int32 			Status = -1;
    uint32  		MsgSize = 0;
    uint32  		payloadSize = 0;
    uint32  		hdrSize = 0;
    CFE_SB_MsgPtr_t CmdMsgPtr;
    CFE_SB_MsgId_t  CmdMsgId;

	Status = CFE_ES_RegisterChildTask();
	if (Status == CFE_SUCCESS)
	{
		/* Ingest Loop */
		do{
			/* Receive cmd and gather data on it */
			MsgSize = CI_MAX_CMD_INGEST;
			CI_ReadSerializedMessage(CI_AppData.SerialIngestBuffer, &MsgSize);
			CmdMsgPtr = (CFE_SB_MsgPtr_t)CI_AppData.SerialIngestBuffer;
			payloadSize = CFE_SB_GetUserDataLength(CmdMsgPtr);
			hdrSize = CFE_SB_MsgHdrSize(CFE_SB_GetMsgId(CmdMsgPtr));

			/* Don't need to deserialize if there is no payload */
			if (payloadSize > 0)
			{
				MsgSize = CI_DeserializeMsg(CmdMsgPtr);
			}

			/* Process the cmd */
			if(MsgSize > 0)
	        {
		        /* If number of bytes received less than max */
		        if (MsgSize <= CI_MAX_CMD_INGEST)
		        {
			        /* Verify validity of cmd */
                    CmdMsgId = CFE_SB_GetMsgId(CmdMsgPtr);
			        if (CI_ValidateSerialCmd(CmdMsgPtr) == TRUE)
			        {
				        /* Check if cmd is for CI and route if so */
				        if (CI_CMD_MID == CmdMsgId)
				        {
					        CI_ProcessNewAppCmds(CmdMsgPtr);
				        }
				        else
				        {
					        /* Verify cmd is authorized */
					        CFE_ES_PerfLogEntry(CI_SOCKET_RCV_PERF_ID);
					        CI_AppData.HkTlm.IngestMsgCount++;
					        CFE_SB_SendMsg(CmdMsgPtr);
					        CFE_ES_PerfLogExit(CI_SOCKET_RCV_PERF_ID);
				        }
			        }
			        else
			        {
				        CI_AppData.HkTlm.usCmdErrCnt++;
				        CFE_EVS_SendEvent (CI_CMD_INVALID_EID, CFE_EVS_ERROR, "Rcvd invalid serial cmd (%i) for msgId (0x%04X)",
									        CFE_SB_GetCmdCode(CmdMsgPtr), CmdMsgId);
			        }
		        }
		        else
		        {
			        CI_AppData.HkTlm.IngestErrorCount++;
			        CFE_EVS_SendEvent(CI_CMD_INGEST_ERR_EID, CFE_EVS_ERROR,
							          "Message too long.  Size = %lu", MsgSize);
		        }
	        }

			/* Wait before next iteration */
			OS_TaskDelay(100);
		}while(CI_AppData.IngestActive == TRUE);

		CFE_ES_ExitChildTask();
	}
	else
	{
		/* Can't send event or write to syslog because this task isn't registered with the cFE. */
		OS_printf("CI Listener Child Task Registration failed!\n");
	}
}
    

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Receive and Process CI Messages                                 */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int32 CI_RcvMsg(int32 iBlocking)
{
    int32           iStatus=CFE_SUCCESS;
    CFE_SB_Msg_t*   MsgPtr=NULL;
    CFE_SB_MsgId_t  MsgId;

    /* Stop Performance Log entry */
    CFE_ES_PerfLogExit(CI_MAIN_TASK_PERF_ID);

    /* Wait for WakeUp messages from scheduler */
    iStatus = CFE_SB_RcvMsg(&MsgPtr, CI_AppData.SchPipeId, iBlocking);

    /* Start Performance Log entry */
    CFE_ES_PerfLogEntry(CI_MAIN_TASK_PERF_ID);

    if (iStatus == CFE_SUCCESS)
    {
        MsgId = CFE_SB_GetMsgId(MsgPtr);
        switch (MsgId)
        {
            case CI_WAKEUP_MID:
                CI_ProcessNewCmds();
                CI_ProcessNewData();
                /* TODO:  Add more code here to handle other things when app wakes up */

                /* The last thing to do at the end of this Wakeup cycle should be to
                 * automatically publish new output. */
                CI_SendOutData();
                break;

            case CI_SEND_HK_MID:
                CI_ReportHousekeeping();
                break;

            case CI_PROCESS_TIMEOUTS_MID:
            	CI_ProcessTimeouts();
				break;

            default:
                (void) CFE_EVS_SendEvent(CI_MSGID_ERR_EID, CFE_EVS_ERROR,
                                  "Recvd invalid SCH msgId (0x%04X)", (unsigned short)MsgId);
        }
    }
    else if (iStatus == CFE_SB_NO_MESSAGE)
    {
        /* TODO: If there's no incoming message, you can do something here, or 
         * nothing.  Note, this section is dead code only if the iBlocking arg
         * is CFE_SB_PEND_FOREVER. */
        iStatus = CFE_SUCCESS;
    }
    else if (iStatus == CFE_SB_TIME_OUT)
    {
        /* TODO: If there's no incoming message within a specified time (via the
         * iBlocking arg, you can do something here, or nothing.  
         * Note, this section is dead code only if the iBlocking arg
         * is CFE_SB_PEND_FOREVER. */
        iStatus = CFE_SUCCESS;
    }
    else
    {
        /* TODO: This is an example of exiting on an error (either CFE_SB_BAD_ARGUMENT, or
         * CFE_SB_PIPE_RD_ERROR).
         */
        (void) CFE_EVS_SendEvent(CI_PIPE_ERR_EID, CFE_EVS_ERROR,
			  "SB pipe read error (0x%08X), app will exit", (unsigned int)iStatus);
        CI_AppData.uiRunStatus= CFE_ES_APP_ERROR;
    }

    return (iStatus);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Process Incoming Data                                           */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void CI_ProcessNewData()
{
    int iStatus = CFE_SUCCESS;
    CFE_SB_Msg_t*   DataMsgPtr=NULL;
    CFE_SB_MsgId_t  DataMsgId;

    /* Process telemetry messages till the pipe is empty */
    while (1)
    {
        iStatus = CFE_SB_RcvMsg(&DataMsgPtr, CI_AppData.DataPipeId, CFE_SB_POLL);
        if (iStatus == CFE_SUCCESS)
        {
            DataMsgId = CFE_SB_GetMsgId(DataMsgPtr);
            switch (DataMsgId)
            {
                /* TODO:  Add code to process all subscribed data here
                **
                ** Example:
                **     case NAV_OUT_DATA_MID:
                **         CI_ProcessNavData(DataMsgPtr);
                **         break;
                */

                default:
                    (void) CFE_EVS_SendEvent(CI_MSGID_ERR_EID, CFE_EVS_ERROR,
                                      "Recvd invalid data msgId (0x%04X)", (unsigned short)DataMsgId);
                    break;
            }
        }
        else if (iStatus == CFE_SB_NO_MESSAGE)
        {
            break;
        }
        else
        {
            (void) CFE_EVS_SendEvent(CI_PIPE_ERR_EID, CFE_EVS_ERROR,
                  "Data pipe read error (0x%08X)", (unsigned int)iStatus);
            CI_AppData.uiRunStatus = CFE_ES_APP_ERROR;
            break;
        }
    }
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Process Incoming Commands                                       */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void CI_ProcessNewCmds()
{
    int iStatus = CFE_SUCCESS;
    CFE_SB_Msg_t*   CmdMsgPtr=NULL;
    CFE_SB_MsgId_t  CmdMsgId;

    /* Process command messages till the pipe is empty */
    while (1)
    {
        iStatus = CFE_SB_RcvMsg(&CmdMsgPtr, CI_AppData.CmdPipeId, CFE_SB_POLL);
        if(iStatus == CFE_SUCCESS)
        {
            CmdMsgId = CFE_SB_GetMsgId(CmdMsgPtr);
            switch (CmdMsgId)
            {
                case CI_CMD_MID:
                    CI_ProcessNewAppCmds(CmdMsgPtr);
                    break;

                /* TODO:  Add code to process other subscribed commands here
                **
                ** Example:
                **     case CFE_TIME_DATA_CMD_MID:
                **         CI_ProcessTimeDataCmd(CmdMsgPtr);
                **         break;
                */

                default:
                    /* Bump the command error counter for an unknown command.
                     * (This should only occur if it was subscribed to with this
                     *  pipe, but not handled in this switch-case.) */
                    CI_AppData.HkTlm.usCmdErrCnt++;
                    (void) CFE_EVS_SendEvent(CI_MSGID_ERR_EID, CFE_EVS_ERROR,
                                      "Recvd invalid CMD msgId (0x%04X)", (unsigned short)CmdMsgId);
                    break;
            }
        }
        else if (iStatus == CFE_SB_NO_MESSAGE)
        {
            break;
        }
        else
        {
            (void) CFE_EVS_SendEvent(CI_PIPE_ERR_EID, CFE_EVS_ERROR,
                  "CMD pipe read error (0x%08X)", (unsigned int)iStatus);
            CI_AppData.uiRunStatus = CFE_ES_APP_ERROR;
            break;
        }
    }
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Process CI Commands                                            */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void CI_ProcessNewAppCmds(CFE_SB_Msg_t* MsgPtr)
{
    uint32  uiCmdCode=0;

    if (MsgPtr != NULL)
    {
        uiCmdCode = CFE_SB_GetCmdCode(MsgPtr);
        switch (uiCmdCode)
        {
            case CI_NOOP_CC:
                CI_AppData.HkTlm.usCmdCnt++;
                (void) CFE_EVS_SendEvent(CI_CMD_INF_EID, CFE_EVS_INFORMATION,
                                  "Recvd NOOP cmd (%u), Version %d.%d.%d.%d",
                                  (unsigned int)uiCmdCode,
                                  CI_MAJOR_VERSION,
                                  CI_MINOR_VERSION,
                                  CI_REVISION,
                                  CI_MISSION_REV);
                break;

            case CI_RESET_CC:
                CI_AppData.HkTlm.usCmdCnt = 0;
                CI_AppData.HkTlm.usCmdErrCnt = 0;
                CI_AppData.HkTlm.IngestMsgCount = 0;
                CI_AppData.HkTlm.IngestErrorCount = 0;
                (void) CFE_EVS_SendEvent(CI_CMD_INF_EID, CFE_EVS_INFORMATION,
                                  "Recvd RESET cmd (%u)", (unsigned int)uiCmdCode);
                break;

            case CI_AUTH_CMD_CC:
            	CI_CmdAuthorize(MsgPtr);
				break;

            case CI_DEAUTH_CMD_CC:
            	CI_CmdDeauthorize(MsgPtr);
				break;

            case CI_REG_CMD_CC:
            	CI_CmdRegister(MsgPtr);
				break;

            case CI_DEREG_CMD_CC:
            	CI_CmdDeregister(MsgPtr);
				break;

            case CI_UPDT_CMD_CC:
            	CI_UpdateCmdReg(MsgPtr);
				break;

            default:
                CI_AppData.HkTlm.usCmdErrCnt++;
                (void) CFE_EVS_SendEvent(CI_MSGID_ERR_EID, CFE_EVS_ERROR,
                                  "Recvd invalid cmdId (%u)", (unsigned int)uiCmdCode);
                break;
        }
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Send CI Housekeeping                                           */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void CI_ReportHousekeeping()
{
    /* TODO:  Add code to update housekeeping data, if needed, here.  */

    CFE_SB_TimeStampMsg((CFE_SB_Msg_t*)&CI_AppData.HkTlm);
    int32 iStatus = CFE_SB_SendMsg((CFE_SB_Msg_t*)&CI_AppData.HkTlm);
    if (iStatus != CFE_SUCCESS)
    {
        /* TODO: Decide what to do if the send message fails. */
    }
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Publish Output Data                                             */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void CI_SendOutData()
{
    /* TODO:  Add code to update output data, if needed, here.  */

    CFE_SB_TimeStampMsg((CFE_SB_Msg_t*)&CI_AppData.OutData);
    int32 iStatus = CFE_SB_SendMsg((CFE_SB_Msg_t*)&CI_AppData.OutData);
    if (iStatus != CFE_SUCCESS)
    {
        /* TODO: Decide what to do if the send message fails. */
    }
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Verify Command Length                                           */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

boolean CI_VerifyCmdLength(CFE_SB_Msg_t* MsgPtr,
                           uint16 usExpectedLen)
{
    boolean bResult  = TRUE;
    uint16  usMsgLen = 0;

    if (MsgPtr != NULL)
    {
        usMsgLen = CFE_SB_GetTotalMsgLength(MsgPtr);

        if (usExpectedLen != usMsgLen)
        {
            bResult = FALSE;
            CFE_SB_MsgId_t MsgId = CFE_SB_GetMsgId(MsgPtr);
            uint16 usCmdCode = CFE_SB_GetCmdCode(MsgPtr);

            (void) CFE_EVS_SendEvent(CI_MSGLEN_ERR_EID, CFE_EVS_ERROR,
                              "Rcvd invalid msgLen: msgId=0x%08X, cmdCode=%d, "
                              "msgLen=%d, expectedLen=%d",
                              MsgId, usCmdCode, usMsgLen, usExpectedLen);
            CI_AppData.HkTlm.usCmdErrCnt++;
        }
    }

    return (bResult);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* CI application entry point and main process loop               */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void CI_AppMain()
{
    /* Register the application with Executive Services */
    CI_AppData.uiRunStatus = CFE_ES_APP_RUN;

    int32 iStatus = CFE_ES_RegisterApp();
    if (iStatus != CFE_SUCCESS)
    {
        (void) CFE_ES_WriteToSysLog("CI - Failed to register the app (0x%08X)\n", (unsigned int)iStatus);
    }

    /* Start Performance Log entry */
    CFE_ES_PerfLogEntry(CI_MAIN_TASK_PERF_ID);

    /* Perform application initializations */
    if (iStatus == CFE_SUCCESS)
    {
        iStatus = CI_InitApp();
    }

    if (iStatus == CFE_SUCCESS)
    {
        /* Do not perform performance monitoring on startup sync */
        CFE_ES_PerfLogExit(CI_MAIN_TASK_PERF_ID);
        CFE_ES_WaitForStartupSync(CI_STARTUP_TIMEOUT_MSEC);
        CFE_ES_PerfLogEntry(CI_MAIN_TASK_PERF_ID);
    }
    else
    {
        CI_AppData.uiRunStatus = CFE_ES_APP_ERROR;
    }

    /* Application main loop */
    while (CFE_ES_RunLoop(&CI_AppData.uiRunStatus) == TRUE)
    {
        int32 iStatus = CI_RcvMsg(CI_SCH_PIPE_PEND_TIME);
        if (iStatus != CFE_SUCCESS)
        {
            /* TODO: Decide what to do for other return values in CI_RcvMsg(). */
        }
        /* TODO: This is only a suggestion for when to update and save CDS table.
        ** Depends on the nature of the application, the frequency of update
        ** and save can be more or less independently.
        */

        CI_UpdateCdsTbl();
        CI_SaveCdsTbl();
    }

    /* Stop Performance Log entry */
    CFE_ES_PerfLogExit(CI_MAIN_TASK_PERF_ID);

    /* Exit the application */
    CFE_ES_ExitApp(CI_AppData.uiRunStatus);
}

/************************/
/*  End of File Comment */
/************************/
