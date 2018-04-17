/************************************************************************
** Includes
*************************************************************************/
#include <string.h>

#include "cfe.h"

#include "ulr_app.h"
#include "ulr_msg.h"
#include "ulr_version.h"
#include "px4lib.h"



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Local Defines                                                   */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#define ULR_MIN_DISTANCE		(0.0f)
#define ULR_MAX_DISTANCE		(50.0f)


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Instantiate the application object.                             */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
ULR oULR;



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Default constructor.                                            */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
ULR::ULR()
{

}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Destructor constructor.                                         */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
ULR::~ULR()
{

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Initialize event tables.                                        */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 ULR::InitEvent()
{
    int32  iStatus=CFE_SUCCESS;

    /* Register the table with CFE */
    iStatus = CFE_EVS_Register(0, 0, CFE_EVS_BINARY_FILTER);
    if (iStatus != CFE_SUCCESS)
    {
        (void) CFE_ES_WriteToSysLog("ULR - Failed to register with EVS (0x%08lX)\n", iStatus);
    }

    return iStatus;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Initialize Message Pipes                                        */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 ULR::InitPipe()
{
    int32  iStatus=CFE_SUCCESS;

    /* Init schedule pipe and subscribe to wakeup messages */
    iStatus = CFE_SB_CreatePipe(&SchPipeId,
    		ULR_SCH_PIPE_DEPTH,
			ULR_SCH_PIPE_NAME);
    if (iStatus == CFE_SUCCESS)
    {
        iStatus = CFE_SB_SubscribeEx(ULR_MEASURE_MID, SchPipeId, CFE_SB_Default_Qos, ULR_MEASURE_MID_MAX_MSG_COUNT);
        if (iStatus != CFE_SUCCESS)
        {
            (void) CFE_EVS_SendEvent(ULR_SUBSCRIBE_ERR_EID, CFE_EVS_ERROR,
            		"Sch Pipe failed to subscribe to ULR_MEASURE_MID. (0x%08lX)",
                    iStatus);
            goto ULR_InitPipe_Exit_Tag;
        }

        iStatus = CFE_SB_SubscribeEx(ULR_SEND_HK_MID, SchPipeId, CFE_SB_Default_Qos, ULR_SEND_HK_MID_MAX_MSG_COUNT);
        if (iStatus != CFE_SUCCESS)
        {
            (void) CFE_EVS_SendEvent(ULR_SUBSCRIBE_ERR_EID, CFE_EVS_ERROR,
					 "CMD Pipe failed to subscribe to ULR_SEND_HK_MID. (0x%08X)",
					 (unsigned int)iStatus);
            goto ULR_InitPipe_Exit_Tag;
        }
    }
    else
    {
        (void) CFE_EVS_SendEvent(ULR_PIPE_INIT_ERR_EID, CFE_EVS_ERROR,
			 "Failed to create SCH pipe (0x%08lX)",
			 iStatus);
        goto ULR_InitPipe_Exit_Tag;
    }

    /* Init command pipe and subscribe to command messages */
    iStatus = CFE_SB_CreatePipe(&CmdPipeId,
    		ULR_CMD_PIPE_DEPTH,
			ULR_CMD_PIPE_NAME);
    if (iStatus == CFE_SUCCESS)
    {
        /* Subscribe to command messages */
        iStatus = CFE_SB_Subscribe(ULR_CMD_MID, CmdPipeId);

        if (iStatus != CFE_SUCCESS)
        {
            (void) CFE_EVS_SendEvent(ULR_SUBSCRIBE_ERR_EID, CFE_EVS_ERROR,
				 "CMD Pipe failed to subscribe to ULR_CMD_MID. (0x%08lX)",
				 iStatus);
            goto ULR_InitPipe_Exit_Tag;
        }
    }
    else
    {
        (void) CFE_EVS_SendEvent(ULR_PIPE_INIT_ERR_EID, CFE_EVS_ERROR,
			 "Failed to create CMD pipe (0x%08lX)",
			 iStatus);
        goto ULR_InitPipe_Exit_Tag;
    }

ULR_InitPipe_Exit_Tag:
    return iStatus;
}
    

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Initialize Global Variables                                     */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ULR::InitData()
{
    /* Init housekeeping message. */
    CFE_SB_InitMsg(&HkTlm,
    		ULR_HK_TLM_MID, sizeof(HkTlm), TRUE);

	HkTlm.MinDistance = ULR_MIN_DISTANCE;
	HkTlm.MaxDistance = ULR_MAX_DISTANCE;
	HkTlm.CurrentDistance = -1.0f;
	HkTlm.Covariance = ULR_SENS_VARIANCE;
	HkTlm.Type = PX4_DISTANCE_SENSOR_RADAR;
	HkTlm.SensorID = 0;
	HkTlm.SensorOrientation = PX4_SENSOR_ORIENTATION_ROLL_180;

	/* Init output messages */
	CFE_SB_InitMsg(&DistanceSensor,
		PX4_DISTANCE_SENSOR_MID, sizeof(PX4_DistanceSensorMsg_t), TRUE);

	DistanceSensor.MinDistance = ULR_MIN_DISTANCE;
	DistanceSensor.MaxDistance = ULR_MAX_DISTANCE;
	/* Make evident that this range sample is invalid */
	DistanceSensor.CurrentDistance = -1.0f;
	DistanceSensor.Covariance = ULR_SENS_VARIANCE;
	DistanceSensor.Type = PX4_DISTANCE_SENSOR_RADAR;
	DistanceSensor.ID = 0;
	DistanceSensor.Orientation = PX4_SENSOR_ORIENTATION_ROLL_180;

	ParserState = ULR_PARSER_STATE_UNINITIALIZED;
	memset(ParserBuffer, 0, sizeof(ParserBuffer));
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* ULR initialization                                              */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 ULR::InitApp()
{
    int32  iStatus   = CFE_SUCCESS;
    int8   hasEvents = 0;

    iStatus = InitEvent();
    if (iStatus != CFE_SUCCESS)
    {
        (void) CFE_ES_WriteToSysLog("ULR - Failed to init events (0x%08lX)\n", iStatus);
        goto ULR_InitApp_Exit_Tag;
    }
    else
    {
        hasEvents = 1;
    }

    iStatus = InitPipe();
    if (iStatus != CFE_SUCCESS)
    {
        goto ULR_InitApp_Exit_Tag;
    }

    InitData();

    iStatus = InitConfigTbl();
    if (iStatus != CFE_SUCCESS)
    {
        goto ULR_InitApp_Exit_Tag;
    }

    iStatus = InitDevice();
    if (iStatus != CFE_SUCCESS)
    {
        goto ULR_InitApp_Exit_Tag;
    }

    iStatus = InitListenerTask();
    if (iStatus != CFE_SUCCESS)
    {
        goto ULR_InitApp_Exit_Tag;
    }

ULR_InitApp_Exit_Tag:
    if (iStatus == CFE_SUCCESS)
    {
        (void) CFE_EVS_SendEvent(ULR_INIT_INF_EID, CFE_EVS_INFORMATION,
                                 "Initialized.  Version %d.%d.%d.%d",
								 ULR_MAJOR_VERSION,
								 ULR_MINOR_VERSION,
								 ULR_REVISION,
								 ULR_MISSION_REV);
    }
    else
    {
        if (hasEvents == 1)
        {
            (void) CFE_ES_WriteToSysLog("ULR - Application failed to initialize\n");
        }
    }

    return iStatus;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Receive and Process Messages                                    */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int32 ULR::RcvSchPipeMsg(int32 iBlocking)
{
    int32           iStatus=CFE_SUCCESS;
    CFE_SB_Msg_t*   MsgPtr=NULL;
    CFE_SB_MsgId_t  MsgId;

    /* Stop Performance Log entry */
    CFE_ES_PerfLogExit(ULR_MAIN_TASK_PERF_ID);

    /* Wait for WakeUp messages from scheduler */
    iStatus = CFE_SB_RcvMsg(&MsgPtr, SchPipeId, iBlocking);

    /* Start Performance Log entry */
    CFE_ES_PerfLogEntry(ULR_MAIN_TASK_PERF_ID);

    if (iStatus == CFE_SUCCESS)
    {
        MsgId = CFE_SB_GetMsgId(MsgPtr);
        switch (MsgId)
        {
            case ULR_MEASURE_MID:
                ReportDistance();
                break;

            case ULR_SEND_HK_MID:
            	ProcessCmdPipe();
                ReportHousekeeping();
                break;

            default:
                (void) CFE_EVS_SendEvent(ULR_MSGID_ERR_EID, CFE_EVS_ERROR,
                     "Recvd invalid SCH msgId (0x%04X)", MsgId);
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
        (void) CFE_EVS_SendEvent(ULR_RCVMSG_ERR_EID, CFE_EVS_ERROR,
			  "SCH pipe read error (0x%08lX).", iStatus);
    }

    return iStatus;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Process Incoming Commands                                       */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void ULR::ProcessCmdPipe()
{
    int32 iStatus = CFE_SUCCESS;
    CFE_SB_Msg_t*   CmdMsgPtr=NULL;
    CFE_SB_MsgId_t  CmdMsgId;

    /* Process command messages until the pipe is empty */
    while (1)
    {
        iStatus = CFE_SB_RcvMsg(&CmdMsgPtr, CmdPipeId, CFE_SB_POLL);
        if(iStatus == CFE_SUCCESS)
        {
            CmdMsgId = CFE_SB_GetMsgId(CmdMsgPtr);
            switch (CmdMsgId)
            {
                case ULR_CMD_MID:
                    ProcessAppCmds(CmdMsgPtr);
                    break;

                default:
                    /* Bump the command error counter for an unknown command.
                     * (This should only occur if it was subscribed to with this
                     *  pipe, but not handled in this switch-case.) */
                    HkTlm.usCmdErrCnt++;
                    (void) CFE_EVS_SendEvent(ULR_MSGID_ERR_EID, CFE_EVS_ERROR,
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
            (void) CFE_EVS_SendEvent(ULR_RCVMSG_ERR_EID, CFE_EVS_ERROR,
                  "CMD pipe read error (0x%08lX)", iStatus);
            break;
        }
    }
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Process ULR Commands                                            */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void ULR::ProcessAppCmds(CFE_SB_Msg_t* MsgPtr)
{
    uint32  uiCmdCode=0;

    if (MsgPtr != NULL)
    {
        uiCmdCode = CFE_SB_GetCmdCode(MsgPtr);
        switch (uiCmdCode)
        {
            case ULR_NOOP_CC:
                HkTlm.usCmdCnt++;
                (void) CFE_EVS_SendEvent(ULR_CMD_NOOP_EID, CFE_EVS_INFORMATION,
					"Recvd NOOP. Version %d.%d.%d.%d",
					ULR_MAJOR_VERSION,
					ULR_MINOR_VERSION,
					ULR_REVISION,
					ULR_MISSION_REV);
                break;

            case ULR_RESET_CC:
                HkTlm.usCmdCnt = 0;
                HkTlm.usCmdErrCnt = 0;
                break;

            default:
                HkTlm.usCmdErrCnt++;
                (void) CFE_EVS_SendEvent(ULR_CC_ERR_EID, CFE_EVS_ERROR,
                                  "Recvd invalid command code (%u)", (unsigned int)uiCmdCode);
                break;
        }
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Send ULR Housekeeping                                           */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void ULR::ReportHousekeeping()
{
    OS_MutSemTake(Mutex);
	HkTlm.MinDistance = DistanceSensor.MinDistance;
	HkTlm.MaxDistance = DistanceSensor.MaxDistance;
	HkTlm.CurrentDistance = DistanceSensor.CurrentDistance;
	HkTlm.Covariance = DistanceSensor.Covariance;
	HkTlm.Type = DistanceSensor.Type;
	HkTlm.SensorID = DistanceSensor.ID;
	HkTlm.SensorOrientation = DistanceSensor.Orientation;
    OS_MutSemGive(Mutex);

    CFE_SB_TimeStampMsg((CFE_SB_Msg_t*)&HkTlm);
    CFE_SB_SendMsg((CFE_SB_Msg_t*)&HkTlm);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Publish Output Data                                             */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ULR::ReportDistance()
{
    //OS_MutSemTake(Mutex);
    //CFE_SB_SendMsg((CFE_SB_Msg_t*)&DistanceSensor);
    //OS_MutSemGive(Mutex);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Verify Command Length                                           */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
boolean ULR::VerifyCmdLength(CFE_SB_Msg_t* MsgPtr,
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

            (void) CFE_EVS_SendEvent(ULR_MSGLEN_ERR_EID, CFE_EVS_ERROR,
                              "Rcvd invalid msgLen: msgId=0x%08X, cmdCode=%d, "
                              "msgLen=%d, expectedLen=%d",
                              MsgId, usCmdCode, usMsgLen, usExpectedLen);
            HkTlm.usCmdErrCnt++;
        }
    }

    return bResult;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* ULR Application C style main entry point.                       */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
extern "C" void ULR_AppMain()
{
    oULR.AppMain();
}

extern "C" void ULR_ListenerTaskMain()
{
    oULR.ListenerTaskMain();
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* ULR Application C++ style main entry point.                     */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ULR::AppMain()
{
    /* Register the application with Executive Services */
    uiRunStatus = CFE_ES_APP_RUN;

    int32 iStatus = CFE_ES_RegisterApp();
    if (iStatus != CFE_SUCCESS)
    {
        (void) CFE_ES_WriteToSysLog("ULR - Failed to register the app (0x%08lX)\n", iStatus);
    }

    /* Start Performance Log entry */
    CFE_ES_PerfLogEntry(ULR_MAIN_TASK_PERF_ID);

    /* Perform application initializations */
    if (iStatus == CFE_SUCCESS)
    {
        iStatus = InitApp();
    }

    if (iStatus == CFE_SUCCESS)
    {
        /* Do not perform performance monitoring on startup sync */
        CFE_ES_PerfLogExit(ULR_MAIN_TASK_PERF_ID);
        CFE_ES_WaitForStartupSync(ULR_STARTUP_TIMEOUT_MSEC);
        CFE_ES_PerfLogEntry(ULR_MAIN_TASK_PERF_ID);
    }
    else
    {
        uiRunStatus = CFE_ES_APP_ERROR;
    }

    /* Application main loop */
    while (CFE_ES_RunLoop(&uiRunStatus) == TRUE)
    {
        RcvSchPipeMsg(ULR_SCH_PIPE_PEND_TIME);

        iStatus = AcquireConfigPointers();
        if(iStatus != CFE_SUCCESS)
        {
            /* We apparently tried to load a new table but failed.  Terminate the application. */
            uiRunStatus = CFE_ES_APP_ERROR;
        }
    }

    StopChild();

    /* Stop Performance Log entry */
    CFE_ES_PerfLogExit(ULR_MAIN_TASK_PERF_ID);

    /* Exit the application */
    CFE_ES_ExitApp(uiRunStatus);
}


bool   ULR::IsChecksumOk(void)
{
	uint8 checksum = 0;

	checksum = (UartMessage.VersionID + UartMessage.AltitudeH +
			UartMessage.AltitudeL + UartMessage.SNR) & 0xFF;

    if (UartMessage.Checksum == checksum)
    {
        return true;
    }
    else
    {
        OS_printf("UartMessage.Checksum  %hhu != checksum %hhu\n", UartMessage.Checksum, checksum);
        return false;
    }
}


bool  ULR::ChildContinueExec(void)
{
	bool result;

    OS_MutSemTake(Mutex);
    result = ChildContinueFlag;
    OS_MutSemGive(Mutex);

	return result;
}


void ULR::StopChild(void)
{
    OS_MutSemTake(Mutex);
    ChildContinueFlag = false;
    OS_MutSemGive(Mutex);
}


void  ULR::ListenerTaskMain(void)
{
	int32 iStatus = CFE_SUCCESS;
	uint8 buf[ULR_BUF_LEN];
	uint32 size = ULR_BUF_LEN;

	CFE_ES_RegisterChildTask();

	while(ChildContinueExec())
	{
		iStatus = ReadDevice(buf, &size);
		if(iStatus == CFE_SUCCESS)
		{
			for(uint32 i = 0; i < size; i++)
			{
				switch(ParserState)
				{
					case ULR_PARSER_STATE_UNINITIALIZED:
						if(buf[i] == 0xfe)
						{
							ParserState = ULR_PARSER_STATE_WAITING_FOR_VERSION_ID;
						}
						break;

					case ULR_PARSER_STATE_WAITING_FOR_HEADER:
						if(buf[i] == 0xfe)
						{
							ParserState = ULR_PARSER_STATE_WAITING_FOR_VERSION_ID;
						}
						else
						{
							(void) CFE_EVS_SendEvent(ULR_UNEXPECTED_DATA_BEFORE_HEADER, CFE_EVS_ERROR,
									"Received 0x%02x before header.", buf[i]);
							ParserState = ULR_PARSER_STATE_UNINITIALIZED;
						}
						break;

					case ULR_PARSER_STATE_WAITING_FOR_VERSION_ID:
						if(buf[i] == 0x01)
						{
							UartMessage.VersionID = buf[i];
							ParserState = ULR_PARSER_STATE_WAITING_FOR_ALT_BYTE_1;
						}
						else
						{
							(void) CFE_EVS_SendEvent(ULR_UNEXPECTED_DATA_BEFORE_VERSION_ID, CFE_EVS_ERROR,
									"Received 0x%02x before version ID.", buf[i]);
							ParserState = ULR_PARSER_STATE_UNINITIALIZED;
						}
						break;

					case ULR_PARSER_STATE_WAITING_FOR_ALT_BYTE_1:
						UartMessage.AltitudeL = buf[i];
						ParserState = ULR_PARSER_STATE_WAITING_FOR_ALT_BYTE_2;
						break;

					case ULR_PARSER_STATE_WAITING_FOR_ALT_BYTE_2:
						UartMessage.AltitudeH = buf[i];
						ParserState = ULR_PARSER_STATE_WAITING_FOR_SNR;
						break;

					case ULR_PARSER_STATE_WAITING_FOR_SNR:
						UartMessage.SNR = buf[i];
						ParserState = ULR_PARSER_STATE_WAITING_FOR_CHECKSUM;
						break;

					case ULR_PARSER_STATE_WAITING_FOR_CHECKSUM:
						UartMessage.Checksum = buf[i];
						if(IsChecksumOk())
						{
						    OS_MutSemTake(Mutex);
						    DistanceSensor.Timestamp = PX4LIB_GetPX4TimeUs();
							DistanceSensor.MinDistance = ULR_MIN_DISTANCE / 10.0f;
							DistanceSensor.MaxDistance = ULR_MAX_DISTANCE / 10.0f;
							DistanceSensor.CurrentDistance = ((UartMessage.AltitudeH << 8) + UartMessage.AltitudeL) / 100.0f;
							DistanceSensor.Covariance = ULR_SENS_VARIANCE;
							DistanceSensor.Type = PX4_DISTANCE_SENSOR_RADAR;
							DistanceSensor.ID = 0;
							DistanceSensor.Orientation = (PX4_SensorOrientation_t)0; //PX4_SENSOR_ORIENTATION_ROLL_180;

//							OS_printf("***************************\n");
//							OS_printf("  Minimum = %f\n", (double)DistanceSensor.MinDistance);
//							OS_printf("  Maximum = %f\n", (double)DistanceSensor.MaxDistance);
//							OS_printf("  Current = %f\n", (double)DistanceSensor.CurrentDistance);
//							OS_printf("  SensorType = %u\n", DistanceSensor.Type);
//							OS_printf("  SensorID = %u\n", DistanceSensor.ID);
//							OS_printf("  SensorOrientation = %u\n", DistanceSensor.Orientation);
//							OS_printf("  Covariance = %f\n", (double)DistanceSensor.Covariance);

						    CFE_SB_TimeStampMsg((CFE_SB_Msg_t*)&DistanceSensor);
						    CFE_SB_SendMsg((CFE_SB_Msg_t*)&DistanceSensor);
						    OS_MutSemGive(Mutex);
						}
						else
						{
							(void) CFE_EVS_SendEvent(ULR_INVALID_CHECKSUM, CFE_EVS_ERROR,
									"Invalid checksum.");
						}
						ParserState = ULR_PARSER_STATE_WAITING_FOR_HEADER;
						break;

					default:
						(void) CFE_EVS_SendEvent(ULR_INVALID_PARSER_STATE, CFE_EVS_ERROR,
								"Parser in invalid state.");
						ParserState = ULR_PARSER_STATE_UNINITIALIZED;
						break;
				}
			}
		}
	}

	CFE_ES_ExitChildTask();
}


int32 ULR::InitListenerTask(void)
{
    int32 Status = CFE_SUCCESS;

    /* Create mutex for shared data */
    Status = OS_MutSemCreate(&Mutex, ULR_MUTEX_NAME, 0);
	if (Status != CFE_SUCCESS)
	{
		goto end_of_function;
	}

	ChildContinueFlag = true;

	Status= CFE_ES_CreateChildTask(&ListenerTaskID,
								   ULR_LISTENER_TASK_NAME,
								   ULR_ListenerTaskMain,
								   NULL,
								   ULR_LISTENER_TASK_STACK_SIZE,
								   ULR_LISTENER_TASK_PRIORITY,
								   ULR_LISTENER_TASK_FLAGS);
	if (Status != CFE_SUCCESS)
	{
		goto end_of_function;
	}

	end_of_function:
	if (Status != CFE_SUCCESS)
	{
		CFE_EVS_SendEvent (ULR_LISTENER_CREATE_CHDTASK_ERR_EID,
						   CFE_EVS_ERROR,
						   "Listener child task failed.  CFE_ES_CreateChildTask returned: 0x%08lX",
						   Status);
	}

    return Status;
}


/************************/
/*  End of File Comment */
/************************/
