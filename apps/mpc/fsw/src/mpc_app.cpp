/************************************************************************
** Includes
*************************************************************************/
#include <string.h>

#include "cfe.h"

#include "mpc_app.h"
#include "mpc_msg.h"
#include "mpc_version.h"
#include "math/Matrix3F3.hpp"
#include "math/Vector2F.hpp"
#include "math/Quaternion.hpp"
#include <float.h>
#include <math.h>
#include "px4lib.h"
#include "geo/geo.h"
#include "math/Expo.hpp"
#include "math/Limits.hpp"

#define nan FP_NAN


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Local definitions                                               */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#define MPC_CONSTANTS_ONE_G    9.80665f   /* m/s^2		*/


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Instantiate the application object.                             */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
MPC oMPC;



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Default constructor.                                            */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
MPC::MPC()
{

}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Destructor constructor.                                         */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
MPC::~MPC()
{

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Initialize event tables.                                        */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 MPC::InitEvent()
{
    int32  iStatus=CFE_SUCCESS;

    /* Register the table with CFE */
    iStatus = CFE_EVS_Register(0, 0, CFE_EVS_BINARY_FILTER);
    if (iStatus != CFE_SUCCESS)
    {
        (void) CFE_ES_WriteToSysLog("MPC - Failed to register with EVS (0x%08lX)\n", iStatus);
    }

    return iStatus;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Initialize Message Pipes                                        */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 MPC::InitPipe()
{
    int32  iStatus=CFE_SUCCESS;

    /* Init schedule pipe and subscribe to wakeup messages */
    iStatus = CFE_SB_CreatePipe(&SchPipeId,
    		MPC_SCH_PIPE_DEPTH,
			MPC_SCH_PIPE_NAME);
    if (iStatus == CFE_SUCCESS)
    {
        iStatus = CFE_SB_SubscribeEx(MPC_WAKEUP_MID, SchPipeId, CFE_SB_Default_Qos, MPC_WAKEUP_MID_MAX_MSG_COUNT);
        if (iStatus != CFE_SUCCESS)
        {
            (void) CFE_EVS_SendEvent(MPC_SUBSCRIBE_ERR_EID, CFE_EVS_ERROR,
            		"Sch Pipe failed to subscribe to MPC_WAKEUP_MID. (0x%08lX)",
                    iStatus);
            goto MPC_InitPipe_Exit_Tag;
        }

        iStatus = CFE_SB_SubscribeEx(MPC_SEND_HK_MID, SchPipeId, CFE_SB_Default_Qos, MPC_SEND_HK_MID_MAX_MSG_COUNT);
        if (iStatus != CFE_SUCCESS)
        {
            (void) CFE_EVS_SendEvent(MPC_SUBSCRIBE_ERR_EID, CFE_EVS_ERROR,
					 "CMD Pipe failed to subscribe to MPC_SEND_HK_MID. (0x%08X)",
					 (unsigned int)iStatus);
            goto MPC_InitPipe_Exit_Tag;
        }

        iStatus = CFE_SB_SubscribeEx(PX4_CONTROL_STATE_MID, SchPipeId, CFE_SB_Default_Qos, 1);
        if (iStatus != CFE_SUCCESS)
        {
            (void) CFE_EVS_SendEvent(MPC_SUBSCRIBE_ERR_EID, CFE_EVS_ERROR,
					 "CMD Pipe failed to subscribe to PX4_CONTROL_STATE_MID. (0x%08lX)",
					 iStatus);
            goto MPC_InitPipe_Exit_Tag;
        }

        iStatus = CFE_SB_SubscribeEx(PX4_MANUAL_CONTROL_SETPOINT_MID, SchPipeId, CFE_SB_Default_Qos, 1);
        if (iStatus != CFE_SUCCESS)
        {
            (void) CFE_EVS_SendEvent(MPC_SUBSCRIBE_ERR_EID, CFE_EVS_ERROR,
					 "CMD Pipe failed to subscribe to PX4_MANUAL_CONTROL_SETPOINT_MID. (0x%08lX)",
					 iStatus);
            goto MPC_InitPipe_Exit_Tag;
        }

        iStatus = CFE_SB_SubscribeEx(PX4_HOME_POSITION_MID, SchPipeId, CFE_SB_Default_Qos, 1);
        if (iStatus != CFE_SUCCESS)
        {
            (void) CFE_EVS_SendEvent(MPC_SUBSCRIBE_ERR_EID, CFE_EVS_ERROR,
					 "CMD Pipe failed to subscribe to PX4_HOME_POSITION_MID. (0x%08lX)",
					 iStatus);
            goto MPC_InitPipe_Exit_Tag;
        }

        iStatus = CFE_SB_SubscribeEx(PX4_VEHICLE_CONTROL_MODE_MID, SchPipeId, CFE_SB_Default_Qos, 1);
        if (iStatus != CFE_SUCCESS)
        {
            (void) CFE_EVS_SendEvent(MPC_SUBSCRIBE_ERR_EID, CFE_EVS_ERROR,
					 "CMD Pipe failed to subscribe to PX4_VEHICLE_CONTROL_MODE_MID. (0x%08lX)",
					 iStatus);
            goto MPC_InitPipe_Exit_Tag;
        }

        iStatus = CFE_SB_SubscribeEx(PX4_POSITION_SETPOINT_TRIPLET_MID, SchPipeId, CFE_SB_Default_Qos, 1);
        if (iStatus != CFE_SUCCESS)
        {
            (void) CFE_EVS_SendEvent(MPC_SUBSCRIBE_ERR_EID, CFE_EVS_ERROR,
					 "CMD Pipe failed to subscribe to PX4_POSITION_SETPOINT_TRIPLET_MID. (0x%08lX)",
					 iStatus);
            goto MPC_InitPipe_Exit_Tag;
        }

        iStatus = CFE_SB_SubscribeEx(PX4_VEHICLE_STATUS_MID, SchPipeId, CFE_SB_Default_Qos, 1);
        if (iStatus != CFE_SUCCESS)
        {
            (void) CFE_EVS_SendEvent(MPC_SUBSCRIBE_ERR_EID, CFE_EVS_ERROR,
					 "CMD Pipe failed to subscribe to PX4_VEHICLE_STATUS_MID. (0x%08lX)",
					 iStatus);
            goto MPC_InitPipe_Exit_Tag;
        }

        iStatus = CFE_SB_SubscribeEx(PX4_VEHICLE_LAND_DETECTED_MID, SchPipeId, CFE_SB_Default_Qos, 1);
        if (iStatus != CFE_SUCCESS)
        {
            (void) CFE_EVS_SendEvent(MPC_SUBSCRIBE_ERR_EID, CFE_EVS_ERROR,
					 "CMD Pipe failed to subscribe to PX4_VEHICLE_LAND_DETECTED_MID. (0x%08lX)",
					 iStatus);
            goto MPC_InitPipe_Exit_Tag;
        }

        iStatus = CFE_SB_SubscribeEx(PX4_VEHICLE_LOCAL_POSITION_MID, SchPipeId, CFE_SB_Default_Qos, 1);
        if (iStatus != CFE_SUCCESS)
        {
            (void) CFE_EVS_SendEvent(MPC_SUBSCRIBE_ERR_EID, CFE_EVS_ERROR,
					 "CMD Pipe failed to subscribe to PX4_VEHICLE_LOCAL_POSITION_MID. (0x%08lX)",
					 iStatus);
            goto MPC_InitPipe_Exit_Tag;
        }
    }
    else
    {
        (void) CFE_EVS_SendEvent(MPC_PIPE_INIT_ERR_EID, CFE_EVS_ERROR,
			 "Failed to create SCH pipe (0x%08lX)",
			 iStatus);
        goto MPC_InitPipe_Exit_Tag;
    }

    /* Init command pipe and subscribe to command messages */
    iStatus = CFE_SB_CreatePipe(&CmdPipeId,
    		MPC_CMD_PIPE_DEPTH,
			MPC_CMD_PIPE_NAME);
    if (iStatus == CFE_SUCCESS)
    {
        /* Subscribe to command messages */
        iStatus = CFE_SB_Subscribe(MPC_CMD_MID, CmdPipeId);

        if (iStatus != CFE_SUCCESS)
        {
            (void) CFE_EVS_SendEvent(MPC_SUBSCRIBE_ERR_EID, CFE_EVS_ERROR,
				 "CMD Pipe failed to subscribe to MPC_CMD_MID. (0x%08lX)",
				 iStatus);
            goto MPC_InitPipe_Exit_Tag;
        }
    }
    else
    {
        (void) CFE_EVS_SendEvent(MPC_PIPE_INIT_ERR_EID, CFE_EVS_ERROR,
			 "Failed to create CMD pipe (0x%08lX)",
			 iStatus);
        goto MPC_InitPipe_Exit_Tag;
    }

MPC_InitPipe_Exit_Tag:
    return iStatus;
}
    

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Initialize Global Variables                                     */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void MPC::InitData()
{
	/* Init housekeeping message. */
	CFE_SB_InitMsg(&HkTlm,
		MPC_HK_TLM_MID, sizeof(HkTlm), TRUE);

	/* Init output messages */
	CFE_SB_InitMsg(&VehicleAttitudeSetpointMsg,
		PX4_VEHICLE_ATTITUDE_SETPOINT_MID, sizeof(PX4_VehicleAttitudeSetpointMsg_t), TRUE);

	//CFE_SB_InitMsg(&VehicleLocalVelocitySetpointMsg,
	//	PX4_VEHICLE_LOCAL_VELOCITY_SETPOINT_MID, sizeof(PX4_VehicleLocalVelocitySetpointMsg_t), TRUE);

	CFE_SB_InitMsg(&VehicleLocalPositionSetpointMsg,
		PX4_VEHICLE_LOCAL_POSITION_SETPOINT_MID, sizeof(PX4_VehicleLocalPositionSetpointMsg_t), TRUE);

	CFE_SB_InitMsg(&VehicleGlobalVelocitySetpointMsg,
		PX4_VEHICLE_GLOBAL_VELOCITY_SETPOINT_MID, sizeof(PX4_VehicleGlobalVelocitySetpointMsg_t), TRUE);

	Rotation.Identity();
	Yaw = 0.0f;
	YawTakeoff = 0.0f;
	InLanding = false;
	InTakeoff = false;
	LndReachedGround = false;
	VelZLp = 0;
	AccZLp = 0;
	VelMaxXY = 0.0f;
	HeadingResetCounter = 0;

	RefPos = {};

	memset(&ControlStateMsg, 0, sizeof(ControlStateMsg));
	memset(&ManualControlSetpointMsg, 0, sizeof(ManualControlSetpointMsg));
	memset(&HomePositionMsg, 0, sizeof(HomePositionMsg));
	memset(&VehicleControlModeMsg, 0, sizeof(VehicleControlModeMsg));
	memset(&PositionSetpointTripletMsg, 0, sizeof(PositionSetpointTripletMsg));
	memset(&VehicleStatusMsg, 0, sizeof(VehicleStatusMsg));
	memset(&VehicleLandDetectedMsg, 0, sizeof(VehicleLandDetectedMsg));
	memset(&VehicleLocalPositionMsg, 0, sizeof(VehicleLocalPositionMsg));
	memset(&VehicleLocalPositionSetpointMsg, 0, sizeof(VehicleLocalPositionSetpointMsg));

	Z_ResetCounter = 0;
	XY_ResetCounter = 0;
	VZ_ResetCounter = 0;
	VXY_ResetCounter = 0;
	HeadingResetCounter = 0;
	TakeoffVelLimit = 0.0f;

	RefTimestamp = 0;
	RefAlt = 0.0f;

	Position.Zero();
	PositionSetpoint.Zero();
	Velocity.Zero();
	VelocitySetpoint.Zero();
	VelocityPrevious.Zero();
	VelocityFF.Zero();
	VelocitySetpointPrevious.Zero();
	VelocityErrD.Zero();
	CurrentPositionSetpoint.Zero();
	ThrustInt.Zero();
	PosP.Zero();

	RSetpoint.Identity();

	ResetPositionSetpoint = true;
	ResetAltitudeSetpoint = true;
	DoResetAltPos = true;
	ModeAuto = false;
	PositionHoldEngaged = false;
	AltitudeHoldEngaged = false;
	RunPosControl = true;
	RunAltControl = true;

	ResetIntZ = true;
	ResetIntXY = true;
	ResetIntZManual = false;
	ResetYawSetpoint = true;

	HoldOffboardXY = false;
	HoldOffboardZ = false;
	LimitVelXY = false;

	GearStateInitialized = false;

	/* Let's be safe and have the landing gear down by default. */
	VehicleAttitudeSetpointMsg.LandingGear = -1.0f;

	WasArmed = false;
	WasLanded = true;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* MPC initialization                                              */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 MPC::InitApp()
{
    int32  iStatus   = CFE_SUCCESS;
    int8   hasEvents = 0;

    iStatus = InitEvent();
    if (iStatus != CFE_SUCCESS)
    {
        (void) CFE_ES_WriteToSysLog("MPC - Failed to init events (0x%08lX)\n", iStatus);
        goto MPC_InitApp_Exit_Tag;
    }
    else
    {
        hasEvents = 1;
    }

    iStatus = InitPipe();
    if (iStatus != CFE_SUCCESS)
    {
        goto MPC_InitApp_Exit_Tag;
    }

    InitData();

    iStatus = InitConfigTbl();
    if (iStatus != CFE_SUCCESS)
    {
        goto MPC_InitApp_Exit_Tag;
    }

MPC_InitApp_Exit_Tag:
    if (iStatus == CFE_SUCCESS)
    {
        (void) CFE_EVS_SendEvent(MPC_INIT_INF_EID, CFE_EVS_INFORMATION,
                                 "Initialized.  Version %d.%d.%d.%d",
								 MPC_MAJOR_VERSION,
								 MPC_MINOR_VERSION,
								 MPC_REVISION,
								 MPC_MISSION_REV);
    }
    else
    {
        if (hasEvents == 1)
        {
            (void) CFE_ES_WriteToSysLog("MPC - Application failed to initialize\n");
        }
    }

    return iStatus;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Receive and Process Messages                                    */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int32 MPC::RcvSchPipeMsg(int32 iBlocking)
{
    int32           iStatus=CFE_SUCCESS;
    CFE_SB_Msg_t*   MsgPtr=NULL;
    CFE_SB_MsgId_t  MsgId;

    /* Stop Performance Log entry */
    CFE_ES_PerfLogExit(MPC_MAIN_TASK_PERF_ID);

    /* Wait for WakeUp messages from scheduler */
    iStatus = CFE_SB_RcvMsg(&MsgPtr, SchPipeId, iBlocking);

    /* Start Performance Log entry */
    CFE_ES_PerfLogEntry(MPC_MAIN_TASK_PERF_ID);

    if (iStatus == CFE_SUCCESS)
    {
        MsgId = CFE_SB_GetMsgId(MsgPtr);
        switch (MsgId)
        {
            case MPC_WAKEUP_MID:
            	Execute();
                break;

            case MPC_SEND_HK_MID:
                ProcessCmdPipe();
                ReportHousekeeping();
                break;

            case PX4_CONTROL_STATE_MID:
                memcpy(&ControlStateMsg, MsgPtr, sizeof(ControlStateMsg));
                ProcessControlStateMsg();
                break;

            case PX4_MANUAL_CONTROL_SETPOINT_MID:
                memcpy(&ManualControlSetpointMsg, MsgPtr, sizeof(ManualControlSetpointMsg));
                break;

            case PX4_HOME_POSITION_MID:
                memcpy(&HomePositionMsg, MsgPtr, sizeof(HomePositionMsg));
                break;

            case PX4_VEHICLE_CONTROL_MODE_MID:
                memcpy(&VehicleControlModeMsg, MsgPtr, sizeof(VehicleControlModeMsg));
                break;

            case PX4_POSITION_SETPOINT_TRIPLET_MID:
                memcpy(&PositionSetpointTripletMsg, MsgPtr, sizeof(PositionSetpointTripletMsg));
                ProcessPositionSetpointTripletMsg();
                break;

            case PX4_VEHICLE_STATUS_MID:
                memcpy(&VehicleStatusMsg, MsgPtr, sizeof(VehicleStatusMsg));

                break;

            case PX4_VEHICLE_LAND_DETECTED_MID:
                memcpy(&VehicleLandDetectedMsg, MsgPtr, sizeof(VehicleLandDetectedMsg));
                break;

            case PX4_VEHICLE_LOCAL_POSITION_MID:
                memcpy(&VehicleLocalPositionMsg, MsgPtr, sizeof(VehicleLocalPositionMsg));
                ProcessVehicleLocalPositionMsg();
                break;

            default:
                (void) CFE_EVS_SendEvent(MPC_MSGID_ERR_EID, CFE_EVS_ERROR,
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
        (void) CFE_EVS_SendEvent(MPC_RCVMSG_ERR_EID, CFE_EVS_ERROR,
			  "SCH pipe read error (0x%08lX).", iStatus);
    }

    return iStatus;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Process Incoming Commands                                       */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void MPC::ProcessCmdPipe()
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
                case MPC_CMD_MID:
                    ProcessAppCmds(CmdMsgPtr);
                    break;

                default:
                    /* Bump the command error counter for an unknown command.
                     * (This should only occur if it was subscribed to with this
                     *  pipe, but not handled in this switch-case.) */
                    HkTlm.usCmdErrCnt++;
                    (void) CFE_EVS_SendEvent(MPC_MSGID_ERR_EID, CFE_EVS_ERROR,
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
            (void) CFE_EVS_SendEvent(MPC_RCVMSG_ERR_EID, CFE_EVS_ERROR,
                  "CMD pipe read error (0x%08lX)", iStatus);
            break;
        }
    }
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Process MPC Commands                                            */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void MPC::ProcessAppCmds(CFE_SB_Msg_t* MsgPtr)
{
    uint32  uiCmdCode=0;

    if (MsgPtr != NULL)
    {
        uiCmdCode = CFE_SB_GetCmdCode(MsgPtr);
        switch (uiCmdCode)
        {
            case MPC_NOOP_CC:
                HkTlm.usCmdCnt++;
                (void) CFE_EVS_SendEvent(MPC_CMD_NOOP_EID, CFE_EVS_INFORMATION,
					"Recvd NOOP. Version %d.%d.%d.%d",
					MPC_MAJOR_VERSION,
					MPC_MINOR_VERSION,
					MPC_REVISION,
					MPC_MISSION_REV);
                break;

            case MPC_RESET_CC:
                HkTlm.usCmdCnt = 0;
                HkTlm.usCmdErrCnt = 0;
                break;

            default:
                HkTlm.usCmdErrCnt++;
                (void) CFE_EVS_SendEvent(MPC_CC_ERR_EID, CFE_EVS_ERROR,
                                  "Recvd invalid command code (%u)", (unsigned int)uiCmdCode);
                break;
        }
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Send MPC Housekeeping                                           */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void MPC::ReportHousekeeping()
{
    CFE_SB_TimeStampMsg((CFE_SB_Msg_t*)&HkTlm);
    CFE_SB_SendMsg((CFE_SB_Msg_t*)&HkTlm);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Publish Output Data                                             */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void MPC::SendVehicleAttitudeSetpointMsg()
{
    CFE_SB_TimeStampMsg((CFE_SB_Msg_t*)&VehicleAttitudeSetpointMsg);
    CFE_SB_SendMsg((CFE_SB_Msg_t*)&VehicleAttitudeSetpointMsg);
}

//void MPC::SendVehicleLocalVelocitySetpointMsg()
//{
//    CFE_SB_TimeStampMsg((CFE_SB_Msg_t*)&VehicleLocalVelocitySetpointMsg);
//    CFE_SB_SendMsg((CFE_SB_Msg_t*)&VehicleLocalVelocitySetpointMsg);
//}

void MPC::SendVehicleLocalPositionSetpointMsg()
{
    CFE_SB_TimeStampMsg((CFE_SB_Msg_t*)&VehicleLocalPositionSetpointMsg);
    CFE_SB_SendMsg((CFE_SB_Msg_t*)&VehicleLocalPositionSetpointMsg);
}

void MPC::SendVehicleGlobalVelocitySetpointMsg()
{
    CFE_SB_TimeStampMsg((CFE_SB_Msg_t*)&VehicleGlobalVelocitySetpointMsg);
    CFE_SB_SendMsg((CFE_SB_Msg_t*)&VehicleGlobalVelocitySetpointMsg);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Verify Command Length                                           */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
boolean MPC::VerifyCmdLength(CFE_SB_Msg_t* MsgPtr,
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

            (void) CFE_EVS_SendEvent(MPC_MSGLEN_ERR_EID, CFE_EVS_ERROR,
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
/* MPC Application C style main entry point.                       */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
extern "C" void MPC_AppMain()
{
    oMPC.AppMain();
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* MPC Application C++ style main entry point.                     */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void MPC::AppMain()
{
    /* Register the application with Executive Services */
    uiRunStatus = CFE_ES_APP_RUN;

    int32 iStatus = CFE_ES_RegisterApp();
    if (iStatus != CFE_SUCCESS)
    {
        (void) CFE_ES_WriteToSysLog("MPC - Failed to register the app (0x%08lX)\n", iStatus);
    }

    /* Start Performance Log entry */
    CFE_ES_PerfLogEntry(MPC_MAIN_TASK_PERF_ID);

    /* Perform application initializations */
    if (iStatus == CFE_SUCCESS)
    {
        iStatus = InitApp();
    }

    if (iStatus == CFE_SUCCESS)
    {
        /* Do not perform performance monitoring on startup sync */
        CFE_ES_PerfLogExit(MPC_MAIN_TASK_PERF_ID);
        CFE_ES_WaitForStartupSync(MPC_STARTUP_TIMEOUT_MSEC);
        CFE_ES_PerfLogEntry(MPC_MAIN_TASK_PERF_ID);
    }
    else
    {
        uiRunStatus = CFE_ES_APP_ERROR;
    }

    /* Application main loop */
    while (CFE_ES_RunLoop(&uiRunStatus) == TRUE)
    {
        RcvSchPipeMsg(MPC_SCH_PIPE_PEND_TIME);

        iStatus = AcquireConfigPointers();
        if(iStatus != CFE_SUCCESS)
        {
            /* We apparently tried to load a new table but failed.  Terminate the application. */
            uiRunStatus = CFE_ES_APP_ERROR;
        }
    }

    /* Stop Performance Log entry */
    CFE_ES_PerfLogExit(MPC_MAIN_TASK_PERF_ID);

    /* Exit the application */
    CFE_ES_ExitApp(uiRunStatus);
}


void MPC::ProcessControlStateMsg(void)
{
	math::Quaternion q_att(
			ControlStateMsg.Q[0],
			ControlStateMsg.Q[1],
			ControlStateMsg.Q[2],
			ControlStateMsg.Q[3]);

	Rotation = q_att.RotationMatrix();
	math::Vector3F euler_angles;

	euler_angles = Rotation.ToEuler();

	Yaw = euler_angles[2];

	if(VehicleControlModeMsg.ControlManualEnabled)
	{
		if (HeadingResetCounter != ControlStateMsg.QuatResetCounter)
		{
			HeadingResetCounter = ControlStateMsg.QuatResetCounter;
			math::Quaternion delta_q(ControlStateMsg.DeltaQReset[0],
					ControlStateMsg.DeltaQReset[1],
					ControlStateMsg.DeltaQReset[2],
					ControlStateMsg.DeltaQReset[3]);

			// we only extract the heading change from the delta quaternion
			math::Vector3F delta_euler = delta_q.ToEuler();
			VehicleAttitudeSetpointMsg.YawBody += delta_euler[2];
		}
	}
}



void MPC::ProcessVehicleLocalPositionMsg(void)
{
	/* Check if a reset event has happened if the vehicle is in manual mode
	 * we will shift the setpoints of the states which were reset. In auto
	 * mode we do not shift the setpoints since we want the vehicle to track
	 * the original state.
	 */
	if (VehicleControlModeMsg.ControlManualEnabled)
	{
		if (Z_ResetCounter != VehicleLocalPositionMsg.Z_ResetCounter)
		{
			PositionSetpoint[2] = PositionSetpoint[2] + VehicleLocalPositionMsg.Delta_Z;
		}

		if (XY_ResetCounter != VehicleLocalPositionMsg.XY_ResetCounter)
		{
			PositionSetpoint[0] = PositionSetpoint[0] + VehicleLocalPositionMsg.Delta_XY[0];
			PositionSetpoint[1] = PositionSetpoint[1] + VehicleLocalPositionMsg.Delta_XY[1];
		}

		if (VZ_ResetCounter != VehicleLocalPositionMsg.VZ_ResetCounter)
		{
			VelocitySetpoint[2] = VelocitySetpoint[2] + VehicleLocalPositionMsg.Delta_VZ;
			VelocitySetpointPrevious[2] =  VelocitySetpointPrevious[2] + VehicleLocalPositionMsg.Delta_VZ;
		}

		if (VXY_ResetCounter != VehicleLocalPositionMsg.VXY_ResetCounter)
		{
			VelocitySetpoint[0] = VelocitySetpoint[0] + VehicleLocalPositionMsg.Delta_VXY[0];
			VelocitySetpoint[1] = VelocitySetpoint[1] + VehicleLocalPositionMsg.Delta_VXY[1];
			VelocitySetpointPrevious[0] = VelocitySetpointPrevious[0] + VehicleLocalPositionMsg.Delta_VXY[0];
			VelocitySetpointPrevious[1] = VelocitySetpointPrevious[1] + VehicleLocalPositionMsg.Delta_VXY[1];
		}
	}

	/* Update the reset counters in any case. */
	Z_ResetCounter = VehicleLocalPositionMsg.Z_ResetCounter;
	XY_ResetCounter = VehicleLocalPositionMsg.XY_ResetCounter;
	VZ_ResetCounter = VehicleLocalPositionMsg.VZ_ResetCounter;
	VXY_ResetCounter = VehicleLocalPositionMsg.VXY_ResetCounter;
}



void MPC::ProcessPositionSetpointTripletMsg(void)
{
	/* Set current position setpoint invalid if none of them (lat, lon and
	 * alt) is finite. */
	if (!isfinite(PositionSetpointTripletMsg.Current.Lat) &&
	    !isfinite(PositionSetpointTripletMsg.Current.Lon) &&
	    !isfinite(PositionSetpointTripletMsg.Current.Alt))
	{
		PositionSetpointTripletMsg.Current.Valid = false;
	}
}


void MPC::Execute(void)
{
	static uint64 t_prev = 0;

	uint64 t = PX4LIB_GetPX4TimeUs();
	float dt = t_prev != 0 ? (t - t_prev) / 1e6f : 0.004f;
	t_prev = t;

	/* Set default max velocity in xy to vel_max */
	VelMaxXY = ConfigTblPtr->XY_VEL_MAX;

	if (VehicleControlModeMsg.Armed && !WasArmed) {
		/* Reset setpoints and integrals on arming. */
		ResetPositionSetpoint = true;
		ResetAltitudeSetpoint = true;
		DoResetAltPos = true;
		VelocitySetpointPrevious.Zero();
		ResetIntZ = true;
		ResetIntXY = true;
		ResetYawSetpoint = true;
		YawTakeoff = Yaw;
	}

	WasArmed = VehicleControlModeMsg.Armed;

	/* Switch to smooth takeoff if we got out of landed state */
	if (!VehicleLandDetectedMsg.Landed && WasLanded)
	{
		InTakeoff = true;
		TakeoffVelLimit = -0.5f;
	}

	/* Set triplets to invalid if we just landed */
	if (VehicleLandDetectedMsg.Landed && !WasLanded)
	{
		PositionSetpointTripletMsg.Current.Valid = false;
	}

	WasLanded = VehicleLandDetectedMsg.Landed;

	UpdateRef();

	UpdateVelocityDerivative(dt);

	/* Reset the horizontal and vertical position hold flags for non-manual modes
	 * or if position / altitude is not controlled. */
	if (!VehicleControlModeMsg.ControlPositionEnabled || !VehicleControlModeMsg.ControlManualEnabled)
	{
		PositionHoldEngaged = false;
	}

	if (!VehicleControlModeMsg.ControlAltitudeEnabled || !VehicleControlModeMsg.ControlManualEnabled)
	{
		AltitudeHoldEngaged = false;
	}

	if(VehicleControlModeMsg.ControlAltitudeEnabled ||
			VehicleControlModeMsg.ControlPositionEnabled ||
			VehicleControlModeMsg.ControlClimbRateEnabled ||
			VehicleControlModeMsg.ControlVelocityEnabled ||
			VehicleControlModeMsg.ControlAccelerationEnabled)
	{
		DoControl(dt);
        OS_printf("vx: %f\n", VelocitySetpoint[0]);
        OS_printf("vy: %f\n", VelocitySetpoint[1]);
		/* Fill local position, velocity and thrust setpoint */
		VehicleLocalPositionSetpointMsg.Timestamp = PX4LIB_GetPX4TimeUs();
		VehicleLocalPositionSetpointMsg.X = PositionSetpoint[0];
		VehicleLocalPositionSetpointMsg.Y = PositionSetpoint[1];
		VehicleLocalPositionSetpointMsg.Z = PositionSetpoint[2];
		VehicleLocalPositionSetpointMsg.Yaw = VehicleAttitudeSetpointMsg.YawBody;
		VehicleLocalPositionSetpointMsg.VX = VelocitySetpoint[0];
		VehicleLocalPositionSetpointMsg.VY = VelocitySetpoint[1];
		VehicleLocalPositionSetpointMsg.VZ = VelocitySetpoint[2];

		/* Publish local position setpoint */
		SendVehicleLocalPositionSetpointMsg();
	}
	else
	{
		/* Position controller disabled, reset setpoints */
		ResetPositionSetpoint = true;
		ResetAltitudeSetpoint = true;
		DoResetAltPos = true;
		ModeAuto = false;
		ResetIntZ = true;
		ResetIntXY = true;
		LimitVelXY = false;

		/* Store last velocity in case a mode switch to position control occurs */
		VelocitySetpointPrevious = Velocity;
	}

	/* Generate attitude setpoint from manual controls */
	if (VehicleControlModeMsg.ControlManualEnabled && VehicleControlModeMsg.ControlAttitudeEnabled)
	{
		GenerateAttitudeSetpoint(dt);

	}
	else
	{
		ResetYawSetpoint = true;
		VehicleAttitudeSetpointMsg.YawSpMoveRate = 0.0f;
	}

	/* Update previous velocity for velocity controller D part */
	VelocityPrevious = Velocity;

	/* Publish attitude setpoint
	 * Do not publish if offboard is enabled but position/velocity/accel
	 * control is disabled, in this case the attitude setpoint is
	 * published by the mavlink app. Also do not publish if the vehicle is a
	 * VTOL and it's just doing a transition (the VTOL attitude control
	 * module will generate attitude setpoints for the transition).
	 */
	if (!(VehicleControlModeMsg.ControlOffboardEnabled &&
	      !(VehicleControlModeMsg.ControlPositionEnabled ||
	    		  VehicleControlModeMsg.ControlVelocityEnabled ||
				  VehicleControlModeMsg.ControlAccelerationEnabled)))
	{

		SendVehicleAttitudeSetpointMsg();
	}

	/* Reset altitude controller integral (hovering throttle) to manual
	 * throttle after manual throttle control */
	ResetIntZManual = VehicleControlModeMsg.Armed && VehicleControlModeMsg.ControlManualEnabled
			      && !VehicleControlModeMsg.ControlClimbRateEnabled;
}



void MPC::UpdateRef(void)
{
    
    if(VehicleStatusMsg.ArmingState == PX4_ARMING_STATE_STANDBY)
    {
	    if (VehicleLocalPositionMsg.RefTimestamp != RefTimestamp)
	    {
		    double LatitudeSetpoint;
    		double LongitudeSetpoint;
	    	float AltitudeSetpoint = 0.0f;
		    uint64 currentTime;

    		if(RefTimestamp != 0)
	    	{
		    	/* Calculate current position setpoint in global frame. */
			    map_projection_reproject(&RefPos, PositionSetpoint[0], PositionSetpoint[1], &LatitudeSetpoint, &LongitudeSetpoint);

    			/* The altitude setpoint is the reference altitude (Z up) plus the (Z down)
	    		 * NED setpoint, multiplied out to minus*/
		    	AltitudeSetpoint = RefAlt - PositionSetpoint[2];
    		}

	    	/* Update local projection reference including altitude. */
		    currentTime = PX4LIB_GetPX4TimeUs();
    		map_projection_init(&RefPos, VehicleLocalPositionMsg.RefLat, VehicleLocalPositionMsg.RefLon, currentTime);
	    	RefAlt = VehicleLocalPositionMsg.RefAlt;

    		if (RefTimestamp != 0)
	    	{
		    	/* Reproject position setpoint to new reference this effectively
			     * adjusts the position setpoint to keep the vehicle in its
			     * current local position. It would only change its global
			     * position on the next setpoint update. */
			    map_projection_project(&RefPos, LatitudeSetpoint, LongitudeSetpoint, &PositionSetpoint[0], &PositionSetpoint[1]);
			    PositionSetpoint[2] = -(AltitudeSetpoint - RefAlt);
		    }
		    RefTimestamp = VehicleLocalPositionMsg.RefTimestamp;
        }
	}
}



void MPC::UpdateVelocityDerivative(float dt)
{
	/* Update velocity derivative,
	 * independent of the current flight mode
	 */
	if (VehicleLocalPositionMsg.Timestamp == 0)
	{
		return;
	}

	/* TODO: this logic should be in the estimator, not the controller! */
	if (isfinite(VehicleLocalPositionMsg.X) &&
		isfinite(VehicleLocalPositionMsg.Y) &&
		isfinite(VehicleLocalPositionMsg.Z))
	{
		Position[0] = VehicleLocalPositionMsg.X;
		Position[1] = VehicleLocalPositionMsg.Y;

		if (ConfigTblPtr->ALT_MODE == 1 && VehicleLocalPositionMsg.DistBottomValid)
		{
			Position[2] = -VehicleLocalPositionMsg.DistBottom;
		}
		else
		{
			Position[2] = VehicleLocalPositionMsg.Z;
		}
	}

	if (isfinite(VehicleLocalPositionMsg.VX) &&
		isfinite(VehicleLocalPositionMsg.VY) &&
		isfinite(VehicleLocalPositionMsg.VZ))
	{
		Velocity[0] = VehicleLocalPositionMsg.VX;
		Velocity[1] = VehicleLocalPositionMsg.VY;

		if (ConfigTblPtr->ALT_MODE == 1 && VehicleLocalPositionMsg.DistBottomValid)
		{
			Velocity[2] = -VehicleLocalPositionMsg.DistBottomRate;
		}
		else
		{
			Velocity[2] = VehicleLocalPositionMsg.VZ;
		}
	}

	VelocityErrD[0] = VelXDeriv.Update(-Velocity[0], dt, ConfigTblPtr->VELD_LP);
	VelocityErrD[1] = VelYDeriv.Update(-Velocity[1], dt, ConfigTblPtr->VELD_LP);
	VelocityErrD[2] = VelZDeriv.Update(-Velocity[2], dt, ConfigTblPtr->VELD_LP);
}



void MPC::DoControl(float dt)
{
	VelocityFF.Zero();

	/* By default, run position/altitude controller. the control_* functions
	 * can disable this and run velocity controllers directly in this cycle */
	RunPosControl = true;
	RunAltControl = true;

	/* If not in auto mode, we reset limit_vel_xy flag. */
	if(VehicleControlModeMsg.ControlManualEnabled || VehicleControlModeMsg.ControlOffboardEnabled)
	{
		LimitVelXY = false;
	}

	if (VehicleControlModeMsg.ControlManualEnabled)
	{
		/* Manual control */
		ControlManual(dt);
		ModeAuto = false;

		/* We set tiplets to false.  This ensures that when switching to auto,
		 * the position controller will not use the old triplets but waits
		 * until triplets have been updated. */
		PositionSetpointTripletMsg.Current.Valid = false;

		HoldOffboardXY = false;
		HoldOffboardZ = false;
	}
	else
	{
		ControlNonManual(dt);
	}
}



void MPC::GenerateAttitudeSetpoint(float dt)
{
	/* Reset yaw setpoint to current position if needed. */
	if (ResetYawSetpoint)
	{
		ResetYawSetpoint = false;
		VehicleAttitudeSetpointMsg.YawBody = Yaw;
	}
	else if (!VehicleLandDetectedMsg.Landed &&
		   !(!VehicleControlModeMsg.ControlAltitudeEnabled && ManualControlSetpointMsg.Z < 0.1f))
	{
		/* Do not move yaw while sitting on the ground. */

		/* We want to know the real constraint, and global overrides manual. */
		const float yaw_rate_max = (math::radians(ConfigTblPtr->MAN_Y_MAX) < math::radians(ConfigTblPtr->MC_YAWRATE_MAX)) ? math::radians(ConfigTblPtr->MAN_Y_MAX) :
				math::radians(ConfigTblPtr->MC_YAWRATE_MAX);
		const float yaw_offset_max = yaw_rate_max / ConfigTblPtr->MC_YAW_P;

		VehicleAttitudeSetpointMsg.YawSpMoveRate = ManualControlSetpointMsg.R * yaw_rate_max;

		float yaw_target = _wrap_pi(VehicleAttitudeSetpointMsg.YawBody + VehicleAttitudeSetpointMsg.YawSpMoveRate * dt);
		float yaw_offs = _wrap_pi(yaw_target - Yaw);

		/* If the yaw offset became too big for the system to track stop
         * shifting it, only allow if it would make the offset smaller again. */
		if (fabsf(yaw_offs) < yaw_offset_max ||
		    (VehicleAttitudeSetpointMsg.YawSpMoveRate > 0 && yaw_offs < 0) ||
		    (VehicleAttitudeSetpointMsg.YawSpMoveRate < 0 && yaw_offs > 0))
		{
			VehicleAttitudeSetpointMsg.YawBody = yaw_target;
		}
	}

	/* Control throttle directly if no climb rate controller is active */
	if (!VehicleControlModeMsg.ControlClimbRateEnabled)
	{
		float thr_val = ThrottleCurve(ManualControlSetpointMsg.Z, ConfigTblPtr->THR_HOVER);

	    VehicleAttitudeSetpointMsg.Thrust = fmin(thr_val, ConfigTblPtr->MANTHR_MAX);

		/* Enforce minimum throttle if not landed */
		if (!VehicleLandDetectedMsg.Landed)
		{
			VehicleAttitudeSetpointMsg.Thrust = fmax(VehicleAttitudeSetpointMsg.Thrust, ConfigTblPtr->MANTHR_MIN);
		}
	}

	/* Control roll and pitch directly if no aiding velocity controller is active. */
	if (!VehicleControlModeMsg.ControlVelocityEnabled)
	{
		VehicleAttitudeSetpointMsg.RollBody = ManualControlSetpointMsg.Y * math::radians(ConfigTblPtr->MAN_TILT_MAX);
		VehicleAttitudeSetpointMsg.PitchBody = -ManualControlSetpointMsg.X * math::radians(ConfigTblPtr->MAN_TILT_MAX);

		/* Only if optimal recovery is not used, modify roll/pitch. */
		if (ConfigTblPtr->VT_OPT_RECOV_EN <= 0)
		{
			/* Construct attitude setpoint rotation matrix. modify the setpoints for roll
			 * and pitch such that they reflect the user's intention even if a yaw error
			 * (yaw_sp - yaw) is present. In the presence of a yaw error constructing a rotation matrix
			 * from the pure euler angle setpoints will lead to unexpected attitude behaviour from
			 * the user's view as the euler angle sequence uses the  yaw setpoint and not the current
			 * heading of the vehicle.
			 */

			/* Calculate our current yaw error. */
			float yaw_error = _wrap_pi(VehicleAttitudeSetpointMsg.YawBody - Yaw);

			// Compute the vector obtained by rotating a z unit vector by the rotation
			// given by the roll and pitch commands of the user
			math::Vector3F zB = {0, 0, 1};
			math::Matrix3F3 R_sp_roll_pitch = math::Matrix3F3::FromEuler(VehicleAttitudeSetpointMsg.RollBody, VehicleAttitudeSetpointMsg.PitchBody, 0);
			math::Vector3F z_roll_pitch_sp = R_sp_roll_pitch * zB;

			/* Transform the vector into a new frame which is rotated around the z axis
			 * by the current yaw error. this vector defines the desired tilt when we look
			 * into the direction of the desired heading.
			 */
			math::Matrix3F3 R_yaw_correction = math::Matrix3F3::FromEuler(0.0f, 0.0f, -yaw_error);
			z_roll_pitch_sp = R_yaw_correction * z_roll_pitch_sp;

			/* Use the formula z_roll_pitch_sp = R_tilt * [0;0;1]
			 * R_tilt is computed from_euler; only true if cos(roll) not equal zero
			 * -> valid if roll is not +-pi/2;
			 */
			VehicleAttitudeSetpointMsg.RollBody = -asinf(z_roll_pitch_sp[1]);
			VehicleAttitudeSetpointMsg.PitchBody = atan2f(z_roll_pitch_sp[0], z_roll_pitch_sp[2]);
		}

		/* Copy quaternion setpoint to attitude setpoint topic. */
		math::Quaternion q_sp(VehicleAttitudeSetpointMsg.RollBody, VehicleAttitudeSetpointMsg.PitchBody, VehicleAttitudeSetpointMsg.YawBody);
		q_sp.copyTo(VehicleAttitudeSetpointMsg.Q_D);
		VehicleAttitudeSetpointMsg.Q_D_Valid = true;
	}

	/* Only switch the landing gear up if we are not landed and if
	 * the user switched from gear down to gear up.
	 * If the user had the switch in the gear up position and took off ignore it
     * until he toggles the switch to avoid retracting the gear immediately on takeoff.
     */
	if (ManualControlSetpointMsg.GearSwitch == PX4_SWITCH_POS_ON && GearStateInitialized &&
	    !VehicleLandDetectedMsg.Landed)
	{
		VehicleAttitudeSetpointMsg.LandingGear = 1.0f;

	}
	else if(ManualControlSetpointMsg.GearSwitch == PX4_SWITCH_POS_ON)
	{
		VehicleAttitudeSetpointMsg.LandingGear = -1.0f;
		/* Switching the gear off does put it into a safe defined state. */
		GearStateInitialized = true;
	}

	VehicleAttitudeSetpointMsg.Timestamp = PX4LIB_GetPX4TimeUs();
}



void MPC::ControlManual(float dt)
{
	/* Velocity setpoint commanded by user stick input. */
	math::Vector3F man_vel_sp(0.0f, 0.0f, 0.0f);

	/* Entering manual control from non-manual control mode, reset alt/pos setpoints */
	if(ModeAuto == true)
	{
		ModeAuto = false;

		/* Reset alt pos flags if resetting is enabled. */
		if(DoResetAltPos == true)
		{
			ResetPositionSetpoint = true;
			ResetAltitudeSetpoint = true;
		}
	}

	/*
	 * Map from stick input to velocity setpoint.
	 */

	if(VehicleControlModeMsg.ControlAltitudeEnabled)
	{
		/* Set vertical velocity setpoint with throttle stick, remapping of
		 * manual.z [0,1] to up and down command [-1,1] */
		man_vel_sp[2] = -math::expof_deadzone(
				(ManualControlSetpointMsg.Z - 0.5f) * 2.0f,
				ConfigTblPtr->XY_MAN_EXPO, ConfigTblPtr->HOLD_DZ);

		/* Reset alt setpoint to current altitude if needed. */
		ResetAltSetpoint();
	}

	if (VehicleControlModeMsg.ControlPositionEnabled)
	{
		float man_vel_hor_length;
		math::Vector2F man_vel_hor;

		/* Set horizontal velocity setpoint with roll/pitch stick */
		man_vel_sp[0] = math::expof_deadzone(
				ManualControlSetpointMsg.X,
				ConfigTblPtr->XY_MAN_EXPO, ConfigTblPtr->HOLD_DZ);
		man_vel_sp[1] = math::expof_deadzone(
				ManualControlSetpointMsg.Y,
				ConfigTblPtr->XY_MAN_EXPO, ConfigTblPtr->HOLD_DZ);

		/* Get the horizontal component of the velocity vector. */
		man_vel_hor[0] = man_vel_sp[0];
		man_vel_hor[1] = man_vel_sp[1];

		/* Get the magnitude of the horizontal component. */
		man_vel_hor_length = man_vel_hor.Length();

		/* Saturate such that magnitude is never larger than 1 */
		if (man_vel_hor_length > 1.0f)
		{
			man_vel_sp[0] /= man_vel_hor_length;
			man_vel_sp[1] /= man_vel_hor_length;
		}

		/* Reset position setpoint to current position if needed */
		ResetPosSetpoint();
	}

	/* Prepare yaw to rotate into NED frame */
	float yaw_input_frame = VehicleControlModeMsg.ControlFixedHdgEnabled ? YawTakeoff : VehicleAttitudeSetpointMsg.YawBody;

	/* Prepare cruise speed (m/s) vector to scale the velocity setpoint */
	float vel_mag = (ConfigTblPtr->VEL_MAN_MAX < VelMaxXY) ? ConfigTblPtr->VEL_MAN_MAX : VelMaxXY;
	math::Vector3F vel_cruise_scale(vel_mag, vel_mag, (man_vel_sp[2] > 0.0f) ? ConfigTblPtr->Z_VEL_MAX_DN : ConfigTblPtr->Z_VEL_MAX_UP);

	/* Setpoint in NED frame and scaled to cruise velocity */
	man_vel_sp = math::Matrix3F3::FromEuler(0.0f, 0.0f, yaw_input_frame) * man_vel_sp.EMult(vel_cruise_scale);

	/*
	 * Assisted velocity mode: User controls velocity, but if velocity is small enough, position
	 * hold is activated for the corresponding axis.
	 */

	/* Want to get/stay in altitude hold if user has z stick in the middle (accounted for deadzone already) */
	const bool alt_hold_desired = VehicleControlModeMsg.ControlAltitudeEnabled && fabsf(man_vel_sp[2]) < FLT_EPSILON;

	/* Want to get/stay in position hold if user has xy stick in the middle (accounted for deadzone already) */
	const bool pos_hold_desired = VehicleControlModeMsg.ControlPositionEnabled &&
				      fabsf(man_vel_sp[0]) < FLT_EPSILON && fabsf(man_vel_sp[1]) < FLT_EPSILON;

	/* Check vertical hold engaged flag. */
	if (AltitudeHoldEngaged)
	{
		AltitudeHoldEngaged = alt_hold_desired;
	}
	else
	{
		/* Check if we switch to alt_hold_engaged. */
		bool smooth_alt_transition = alt_hold_desired &&
					     (ConfigTblPtr->HOLD_MAX_Z < FLT_EPSILON || fabsf(Velocity[2]) < ConfigTblPtr->HOLD_MAX_Z);

		/* During transition predict setpoint forward. */
		if (smooth_alt_transition)
		{
			/* Get max acceleration. */
			float max_acc_z = (Velocity[2] < 0.0f ? ConfigTblPtr->ACC_DOWN_MAX : -ConfigTblPtr->ACC_UP_MAX);

			/* Time to travel from current velocity to zero velocity. */
			float delta_t = fabsf(Velocity[2] / max_acc_z);

			/* Set desired position setpoint assuming max acceleraiton. */
			PositionSetpoint[2] = PositionSetpoint[2] + Velocity[2] * delta_t + 0.5f * max_acc_z * delta_t * delta_t;

			AltitudeHoldEngaged = true;
		}
	}

	/* Check horizontal hold engaged flag. */
	if (PositionHoldEngaged)
	{
		PositionHoldEngaged = pos_hold_desired;
	}
	else
	{
		/* Check if we switch to pos_hold_engaged. */
		float vel_xy_mag = sqrtf(Velocity[0] * Velocity[0] + Velocity[1] * Velocity[1]);
		bool smooth_pos_transition = pos_hold_desired &&
					     (ConfigTblPtr->HOLD_MAX_XY < FLT_EPSILON || vel_xy_mag < ConfigTblPtr->HOLD_MAX_XY);

		/* During transition predict setpoint forward. */
		if (smooth_pos_transition)
		{
			/* Time to travel from current velocity to zero velocity. */
			float delta_t = sqrtf(Velocity[0] * Velocity[0] + Velocity[1] * Velocity[1]) / ConfigTblPtr->ACC_HOR_MAX;

			/* p pos_sp in xy from max acceleration and current velocity */
			math::Vector2F pos(Position[0], Position[1]);
			math::Vector2F vel(Velocity[0], Velocity[1]);
			math::Vector2F pos_sp = pos + vel * delta_t - vel.Normalized() * 0.5f * ConfigTblPtr->ACC_HOR_MAX * delta_t * delta_t;
			PositionSetpoint[0] = pos_sp[0];
			PositionSetpoint[1] = pos_sp[1];

			PositionHoldEngaged = true;
		}
	}

	/* Set requested velocity setpoints */
	if (!AltitudeHoldEngaged)
	{
		PositionSetpoint[2] = Position[2];
		/* Request velocity setpoint to be used, instead of altitude setpoint */
		RunAltControl = false;
		VelocitySetpoint[2] = man_vel_sp[2];
	}

	if (!PositionHoldEngaged)
	{
		PositionSetpoint[0] = Position[0];
		PositionSetpoint[1] = Position[1];
		/* Request velocity setpoint to be used, instead of position setpoint */
		RunPosControl = false;
		VelocitySetpoint[0] = man_vel_sp[0];
		VelocitySetpoint[1] = man_vel_sp[1];
	}

	if (VehicleLandDetectedMsg.Landed)
	{
		/* Don't run controller when landed */
		ResetPositionSetpoint = true;
		ResetAltitudeSetpoint = true;
		ModeAuto = false;
		ResetIntZ = true;
		ResetIntXY = true;

		RSetpoint.Identity();

		VehicleAttitudeSetpointMsg.RollBody = 0.0f;
		VehicleAttitudeSetpointMsg.PitchBody = 0.0f;
		VehicleAttitudeSetpointMsg.YawBody = Yaw;
		VehicleAttitudeSetpointMsg.Thrust = 0.0f;

		VehicleAttitudeSetpointMsg.Timestamp = PX4LIB_GetPX4TimeUs();

	}
	else
	{
		ControlPosition(dt);
	}
}



void MPC::ControlNonManual(float dt)
{
	/* Select control source. */
	if(VehicleControlModeMsg.ControlOffboardEnabled)
	{
		/* Offboard control */
		ControlOffboard(dt);
		ModeAuto = false;

	}
	else
	{
		HoldOffboardXY = false;
		HoldOffboardZ = false;

		/* AUTO */
		ControlAuto(dt);
	}

	/* Weather-vane mode for vtol: disable yaw control */
	if (VehicleStatusMsg.IsVtol)
	{
		VehicleAttitudeSetpointMsg.DisableMcYawControl = PositionSetpointTripletMsg.Current.DisableMcYawControl;
	}
	else
	{
		VehicleAttitudeSetpointMsg.DisableMcYawControl = false;
	}

	/* Guard against any bad velocity values. */
	bool velocity_valid = isfinite(PositionSetpointTripletMsg.Current.VX) &&
			isfinite(PositionSetpointTripletMsg.Current.VY) &&
			PositionSetpointTripletMsg.Current.VelocityValid;

	/* Do not go slower than the follow target velocity when position tracking
	 * is active (set to valid)
	 */
	if (PositionSetpointTripletMsg.Current.Type == PX4_SETPOINT_TYPE_FOLLOW_TARGET &&
	    velocity_valid &&
		PositionSetpointTripletMsg.Current.PositionValid)
	{
		math::Vector3F ft_vel(PositionSetpointTripletMsg.Current.VX, PositionSetpointTripletMsg.Current.VY, 0);
		float cos_ratio = (ft_vel * VelocitySetpoint) / (ft_vel.Length() * VelocitySetpoint.Length());

		/* Only override velocity set points when uav is traveling in same
		 * direction as target and vector component is greater than
		 * calculated position set point velocity component.
		 */
		if (cos_ratio > 0)
		{
			ft_vel = ft_vel * cos_ratio;
			/* Min speed a little faster than target vel. */
			ft_vel = ft_vel + ft_vel.Normalized() * 1.5f;

		}
		else
		{
			ft_vel.Zero();
		}

		VelocitySetpoint[0] = fabsf(ft_vel[0]) > fabsf(VelocitySetpoint[0]) ? ft_vel[0] : VelocitySetpoint[0];
		VelocitySetpoint[1] = fabsf(ft_vel[1]) > fabsf(VelocitySetpoint[1]) ? ft_vel[1] : VelocitySetpoint[1];

		/* Track target using velocity only. */

	}
	else if(PositionSetpointTripletMsg.Current.Type == PX4_SETPOINT_TYPE_FOLLOW_TARGET &&
		   velocity_valid)
	{
		VelocitySetpoint[0] = PositionSetpointTripletMsg.Current.VX;
		VelocitySetpoint[1] = PositionSetpointTripletMsg.Current.VY;
	}

	/* Use constant descend rate when landing, ignore altitude setpoint. */
	if (PositionSetpointTripletMsg.Current.Valid
	    && PositionSetpointTripletMsg.Current.Type == PX4_SETPOINT_TYPE_LAND)
	{
		VelocitySetpoint[2] = ConfigTblPtr->LAND_SPEED;
		RunAltControl = false;
	}

	if (PositionSetpointTripletMsg.Current.Valid
	    && PositionSetpointTripletMsg.Current.Type == PX4_SETPOINT_TYPE_IDLE)
	{
		/* Idle state, don't run controller and set zero thrust. */
		RSetpoint.Identity();
		math::Quaternion qd(RSetpoint);
		qd.copyTo(VehicleAttitudeSetpointMsg.Q_D);
		VehicleAttitudeSetpointMsg.Q_D_Valid = true;

		VehicleAttitudeSetpointMsg.RollBody = 0.0f;
		VehicleAttitudeSetpointMsg.PitchBody = 0.0f;
		VehicleAttitudeSetpointMsg.YawBody = Yaw;
		VehicleAttitudeSetpointMsg.Thrust = 0.0f;

		VehicleAttitudeSetpointMsg.Timestamp = PX4LIB_GetPX4TimeUs();
	}
	else
	{
		ControlPosition(dt);
	}
}



float MPC::ThrottleCurve(float ctl, float ctr)
{
	float result;

	/* Piecewise linear mapping: 0:ctr -> 0:0.5
	 * and ctr:1 -> 0.5:1 */
	if (ctl < 0.5f)
	{
		result = 2.0f * ctl * ctr;
	}
	else
	{
		result = ctr + 2.0f * (ctl - 0.5f) * (1.0f - ctr);
	}

	return result;
}



void MPC::ResetPosSetpoint(void)
{
	if (ResetPositionSetpoint)
	{
		ResetPositionSetpoint = false;

		/* We have logic in the main function which chooses the velocity setpoint such that the attitude setpoint is
		 * continuous when switching into velocity controlled mode, therefore, we don't need to bother about resetting
		 * altitude in a special way. */
		PositionSetpoint[0] = Position[0];
		PositionSetpoint[1] = Position[1];
	}
}



void MPC::ResetAltSetpoint(void)
{
	if (ResetAltitudeSetpoint)
	{
		ResetAltitudeSetpoint = false;

		/* We have logic in the main function which chooses the velocity setpoint such that the attitude setpoint is
		 * continuous when switching into velocity controlled mode, therefore, we don't need to bother about resetting
		 * position in a special way. In position control mode the position will be reset anyway until the vehicle has reduced speed.
		 */
		PositionSetpoint[2] = Position[2];
	}
}



void MPC::ControlPosition(float dt)
{
	CalculateVelocitySetpoint(dt);

	if (VehicleControlModeMsg.ControlClimbRateEnabled || VehicleControlModeMsg.ControlVelocityEnabled ||
			VehicleControlModeMsg.ControlAccelerationEnabled)
	{
		CalculateThrustSetpoint(dt);
	}
	else
	{
		ResetIntZ = true;
	}
}



void MPC::ControlOffboard(float dt)
{
	if(PositionSetpointTripletMsg.Current.Valid)
	{
		if (VehicleControlModeMsg.ControlPositionEnabled && PositionSetpointTripletMsg.Current.PositionValid)
		{
			/* Control position */
			PositionSetpoint[0] = PositionSetpointTripletMsg.Current.X;
			PositionSetpoint[1] = PositionSetpointTripletMsg.Current.Y;
			RunPosControl = true;

			HoldOffboardXY = false;
		}
		else if(VehicleControlModeMsg.ControlVelocityEnabled &&
				PositionSetpointTripletMsg.Current.VelocityValid)
		{
			/* Control velocity */

			/* Reset position setpoint to current position if needed */
			ResetPosSetpoint();

			if (fabsf(PositionSetpointTripletMsg.Current.VX) <= FLT_EPSILON &&
			    fabsf(PositionSetpointTripletMsg.Current.VY) <= FLT_EPSILON &&
			    VehicleLocalPositionMsg.XY_Valid)
			{
				if (!HoldOffboardXY)
				{
					PositionSetpoint[0] = Position[0];
					PositionSetpoint[1] = Position[1];
					HoldOffboardXY = true;
				}

				RunPosControl = true;
			}
			else
			{
				if (PositionSetpointTripletMsg.Current.VelocityFrame == PX4_VELOCITY_FRAME_LOCAL_NED)
				{
					/* Set position setpoint move rate */
					VelocitySetpoint[0] = PositionSetpointTripletMsg.Current.VX;
					VelocitySetpoint[1] = PositionSetpointTripletMsg.Current.VY;
				}
				else if (PositionSetpointTripletMsg.Current.VelocityFrame == PX4_VELOCITY_FRAME_BODY_NED) {
					/* Transform velocity command from body frame to NED frame */
					VelocitySetpoint[0] = cosf(Yaw) * PositionSetpointTripletMsg.Current.VX - sinf(Yaw) * PositionSetpointTripletMsg.Current.VY;
					VelocitySetpoint[1] = sinf(Yaw) * PositionSetpointTripletMsg.Current.VX + cosf(Yaw) * PositionSetpointTripletMsg.Current.VY;
				}
				else
				{
		            (void) CFE_EVS_SendEvent(MPC_UNK_VEL_FRM_ERR_EID, CFE_EVS_ERROR,
		            		"Unknown velocity offboard coordinate frame. (%u)",
							PositionSetpointTripletMsg.Current.VelocityFrame);
				}

				RunPosControl = false;

				HoldOffboardXY = false;
			}
		}

		if (VehicleControlModeMsg.ControlAltitudeEnabled && PositionSetpointTripletMsg.Current.AltValid)
		{
			/* Control altitude as it is enabled. */
			PositionSetpoint[2] = PositionSetpointTripletMsg.Current.Z;
			RunAltControl = true;

			HoldOffboardZ = false;

		}
		else if (VehicleControlModeMsg.ControlClimbRateEnabled && PositionSetpointTripletMsg.Current.VelocityValid)
		{
			/* Reset alt setpoint to current altitude if needed */
			ResetAltSetpoint();

			if (fabsf(PositionSetpointTripletMsg.Current.VZ) <= FLT_EPSILON &&
			    VehicleLocalPositionMsg.Z_Valid)
			{
				if (!HoldOffboardZ)
				{
					PositionSetpoint[2] = Position[2];
					HoldOffboardZ = true;
				}

				RunAltControl = true;

			}
			else
			{
				/* Set position setpoint move rate */
				VelocitySetpoint[2] = PositionSetpointTripletMsg.Current.VZ;
				RunAltControl = false;

				HoldOffboardZ = false;
			}
		}

		if (PositionSetpointTripletMsg.Current.YawValid)
		{
			VehicleAttitudeSetpointMsg.YawBody = PositionSetpointTripletMsg.Current.Yaw;
		}
		else if (PositionSetpointTripletMsg.Current.YawspeedValid)
		{
			VehicleAttitudeSetpointMsg.YawBody = VehicleAttitudeSetpointMsg.YawBody + PositionSetpointTripletMsg.Current.Yawspeed * dt;
		}
	}
	else
	{
		HoldOffboardXY = false;
		HoldOffboardZ = false;
		ResetPosSetpoint();
		ResetAltSetpoint();
	}
}



void MPC::ControlAuto(float dt)
{
	/* Reset position setpoint on AUTO mode activation or if we are not in
	 * MC mode */
	if (!ModeAuto || !VehicleStatusMsg.IsRotaryWing)
	{
		if (!ModeAuto)
		{
			ModeAuto = true;
		}

		ResetPositionSetpoint = true;
		ResetAltitudeSetpoint = true;
	}

	/* Always check reset state of altitude and position control flags in auto. */
	ResetPosSetpoint();
	ResetAltSetpoint();

	bool current_setpoint_valid = false;
	bool previous_setpoint_valid = false;
	bool next_setpoint_valid = false;

	math::Vector3F prev_sp;
	math::Vector3F next_sp;

	if (PositionSetpointTripletMsg.Current.Valid)
	{
		/* Only project setpoints if they are finite, else use current
		 * position. */
		if (isfinite(PositionSetpointTripletMsg.Current.Lat) &&
		    isfinite(PositionSetpointTripletMsg.Current.Lon))
		{
			/* Project setpoint to local frame. */
			map_projection_project(&RefPos,
					PositionSetpointTripletMsg.Current.Lat, PositionSetpointTripletMsg.Current.Lon,
					       &CurrentPositionSetpoint[0], &CurrentPositionSetpoint[1]);
		}
		else
		{
			CurrentPositionSetpoint[0] = Position[0];
			CurrentPositionSetpoint[1] = Position[1];
		}

		/* Only project setpoints if they are finite, else use current position. */
		if (isfinite(PositionSetpointTripletMsg.Current.Alt))
		{
			CurrentPositionSetpoint[2] = -(PositionSetpointTripletMsg.Current.Alt - RefAlt);
		}
		else
		{
			CurrentPositionSetpoint[2] = Position[2];
		}

		if (isfinite(CurrentPositionSetpoint[0]) &&
				isfinite(CurrentPositionSetpoint[1]) &&
				isfinite(CurrentPositionSetpoint[2]))
		{
			current_setpoint_valid = true;
		}
	}

	if (PositionSetpointTripletMsg.Previous.Valid)
	{
		map_projection_project(&RefPos,
				PositionSetpointTripletMsg.Previous.Lat, PositionSetpointTripletMsg.Previous.Lon,
				       &prev_sp[0], &prev_sp[1]);
		prev_sp[2] = -(PositionSetpointTripletMsg.Previous.Alt - RefAlt);

		if (isfinite(prev_sp[0]) &&
				isfinite(prev_sp[1]) &&
				isfinite(prev_sp[2]))
		{
			previous_setpoint_valid = true;
		}
	}

	if (PositionSetpointTripletMsg.Next.Valid)
	{
		map_projection_project(&RefPos,
				PositionSetpointTripletMsg.Next.Lat, PositionSetpointTripletMsg.Next.Lon,
				       &next_sp[0], &next_sp[1]);
		next_sp[2] = -(PositionSetpointTripletMsg.Next.Alt - RefAlt);

		if (isfinite(next_sp[0]) &&
				isfinite(next_sp[1]) &&
				isfinite(next_sp[2]))
		{
			next_setpoint_valid = true;
		}
	}

	/* Set velocity limit if close to current setpoint and no next setpoint available. */
	math::Vector3F dist = CurrentPositionSetpoint - Position;
	LimitVelXY = (!next_setpoint_valid || (PositionSetpointTripletMsg.Current.Type == PX4_SETPOINT_TYPE_LOITER))
			&& (sqrtf(dist[0] * dist[0] + dist[1] * dist[1]) <= ConfigTblPtr->TARGET_THRE);

	if (current_setpoint_valid &&
	    (PositionSetpointTripletMsg.Current.Type != PX4_SETPOINT_TYPE_IDLE))
	{
		float cruising_speed_xy = GetCruisingSpeedXY();
		float cruising_speed_z = (CurrentPositionSetpoint[2] > Position[2]) ? ConfigTblPtr->Z_VEL_MAX_DN : ConfigTblPtr->Z_VEL_MAX_UP;

		/* Scaled space: 1 == position error resulting max allowed speed. */
		math::Vector3F cruising_speed(cruising_speed_xy, cruising_speed_xy, cruising_speed_z);

		/* If previous is valid, we want to follow line. */
		if (previous_setpoint_valid
		    && (PositionSetpointTripletMsg.Current.Type == PX4_SETPOINT_TYPE_POSITION  ||
		    		PositionSetpointTripletMsg.Current.Type == PX4_SETPOINT_TYPE_LOITER ||
					PositionSetpointTripletMsg.Current.Type == PX4_SETPOINT_TYPE_FOLLOW_TARGET))
		{
			math::Vector3F scale = PosP.EDivide(cruising_speed);

			/* Convert current setpoint to scaled space. */
			math::Vector3F curr_sp_s = CurrentPositionSetpoint.EMult(scale);

			/* By default, use current setpoint as is. */
			math::Vector3F pos_sp_s = curr_sp_s;

			const float minimum_dist = 0.01f;

			if ((CurrentPositionSetpoint - prev_sp).Length() > minimum_dist)
			{
				/* find X - cross point of unit sphere and trajectory */
				math::Vector3F pos_s = Position.EMult(scale);
				math::Vector3F prev_sp_s = prev_sp.EMult(scale);
				math::Vector3F prev_curr_s = curr_sp_s - prev_sp_s;
				math::Vector3F curr_pos_s = pos_s - curr_sp_s;
				float curr_pos_s_len = curr_pos_s.Length();

				/* We are close to current setpoint. */
				if (curr_pos_s_len < 1.0f)
				{
					/* If next is valid, we want to have smooth transition. */
					if (next_setpoint_valid && (next_sp - CurrentPositionSetpoint).Length() > minimum_dist)
					{
						math::Vector3F next_sp_s = next_sp.EMult(scale);

						/* Calculate angle prev - curr - next */
						math::Vector3F curr_next_s = next_sp_s - curr_sp_s;
						math::Vector3F prev_curr_s_norm = prev_curr_s.Normalized();

						/* cos(a) * curr_next, a = angle between current and next trajectory segments */
						float cos_a_curr_next = prev_curr_s_norm * curr_next_s;

						/* cos(b), b = angle pos - _curr_pos_sp - prev_sp */
						float cos_b = -curr_pos_s * prev_curr_s_norm / curr_pos_s_len;

						if (cos_a_curr_next > 0.0f && cos_b > 0.0f)
						{
							float curr_next_s_len = curr_next_s.Length();

							/* If curr - next distance is larger than unit radius, limit it. */
							if (curr_next_s_len > 1.0f)
							{
								cos_a_curr_next /= curr_next_s_len;
							}

							/* Feed forward position setpoint offset. */
							math::Vector3F pos_ff = prev_curr_s_norm *
										 cos_a_curr_next * cos_b * cos_b * (1.0f - curr_pos_s_len) *
										 (1.0f - expf(-curr_pos_s_len * curr_pos_s_len * 20.0f));
							pos_sp_s = pos_sp_s + pos_ff;
						}
					}
				}
				else
				{
					/* If not close to current setpoint, check if we are
					 * within cross_sphere_line. */
					bool near = CrossSphereLine(pos_s, 1.0f, prev_sp_s, curr_sp_s, pos_sp_s);

					if (!near)
					{
						/* We're far away from trajectory, pos_sp_s is set to
						 * the nearest point on the trajectory */
						pos_sp_s = pos_s + (pos_sp_s - pos_s).Normalized();
					}
				}
			}

			/* Move setpoint not faster than max allowed speed. */
			math::Vector3F pos_sp_old_s = PositionSetpoint.EMult(scale);

			/* Difference between current and desired position setpoints, 1 = max speed. */
			math::Vector3F d_pos_m = (pos_sp_s - pos_sp_old_s).EDivide(PosP);
			float d_pos_m_len = d_pos_m.Length();

			if (d_pos_m_len > dt)
			{
				pos_sp_s = pos_sp_old_s + (d_pos_m / d_pos_m_len * dt).EMult(PosP);
			}

			/* Scale back */
			PositionSetpoint = pos_sp_s.EDivide(scale);
		}
		else
		{
			/* We just have a current setpoint that we want to go to. */
			PositionSetpoint = CurrentPositionSetpoint;

			/* Set max velocity to cruise. */
			VelMaxXY = cruising_speed[0];
		}

		/* Update yaw setpoint if needed. */
		if (PositionSetpointTripletMsg.Current.YawspeedValid
		    && PositionSetpointTripletMsg.Current.Type == PX4_SETPOINT_TYPE_FOLLOW_TARGET)
		{
			VehicleAttitudeSetpointMsg.YawBody = VehicleAttitudeSetpointMsg.YawBody + PositionSetpointTripletMsg.Current.Yawspeed * dt;
		}
		else if (isfinite(PositionSetpointTripletMsg.Current.Yaw))
		{
			VehicleAttitudeSetpointMsg.YawBody = PositionSetpointTripletMsg.Current.Yaw;
		}

		/*
		 * If we're already near the current takeoff setpoint don't reset in case we switch back to posctl.
		 * this makes the takeoff finish smoothly.
		 */
		if ((PositionSetpointTripletMsg.Current.Type == PX4_SETPOINT_TYPE_TAKEOFF
		     || PositionSetpointTripletMsg.Current.Type == PX4_SETPOINT_TYPE_LOITER)
		    && PositionSetpointTripletMsg.Current.AcceptanceRadius > 0.0f
		    /* Need to detect we're close a bit before the navigator switches from takeoff to next waypoint */
		    && (Position - PositionSetpoint).Length() < PositionSetpointTripletMsg.Current.AcceptanceRadius * 1.2f)
		{
			DoResetAltPos = false;

		}
		else
		{
			/* Otherwise: in case of interrupted mission don't go to waypoint but stay at current position. */
			DoResetAltPos = true;
		}

		/* Handle the landing gear based on the manual landing alt. */
		const bool high_enough_for_landing_gear = (Position[2] < ConfigTblPtr->MIS_LTRMIN_ALT * 2.0f);

		/* During a mission or in loiter it's safe to retract the landing gear. */
		if ((PositionSetpointTripletMsg.Current.Type == PX4_SETPOINT_TYPE_POSITION ||
				PositionSetpointTripletMsg.Current.Type == PX4_SETPOINT_TYPE_LOITER) &&
		    !VehicleLandDetectedMsg.Landed &&
		    high_enough_for_landing_gear)
		{
			VehicleAttitudeSetpointMsg.LandingGear = 1.0f;
		}
		else if (PositionSetpointTripletMsg.Current.Type == PX4_SETPOINT_TYPE_TAKEOFF ||
				PositionSetpointTripletMsg.Current.Type == PX4_SETPOINT_TYPE_LAND ||
			   !high_enough_for_landing_gear)
		{
			/* During takeoff and landing, we better put it down again. */
			VehicleAttitudeSetpointMsg.LandingGear = -1.0f;
		}
		else
		{
			/* For the rest of the setpoint types, just leave it as is. */
		}
	}
	else
	{
		/* Idle or triplet not valid, set velocity setpoint to zero */
		VelocitySetpoint.Zero();
		RunPosControl = false;
		RunAltControl = false;
	}
}



void MPC::CalculateVelocitySetpoint(float dt)
{
	/* Run position & altitude controllers, if enabled (otherwise use already
	 * computed velocity setpoints) */
	if(RunPosControl)
	{
		/* If for any reason, we get a NaN position setpoint, we better just
		 * stay where we are.
		 */
		if(isfinite(PositionSetpoint[0]) && isfinite(PositionSetpoint[1]))
		{
			VelocitySetpoint[0] = (PositionSetpoint[0] - Position[0]) * PosP[0];
			VelocitySetpoint[1] = (PositionSetpoint[1] - Position[1]) * PosP[1];
		}
		else
		{
			VelocitySetpoint[0] = 0.0f;
			VelocitySetpoint[1] = 0.0f;
		}

    	/* Constrain xy velocities to defined limits after position scaling */
	    float vel_mag = (ConfigTblPtr->XY_CRUISE < VelMaxXY) ? ConfigTblPtr->XY_CRUISE : VelMaxXY;
        float vel_norm_xy = sqrtf(VelocitySetpoint[0] * VelocitySetpoint[0] +
			VelocitySetpoint[1] * VelocitySetpoint[1]);

        if (vel_norm_xy > vel_mag)
        {
            float scale = 1.0f;
            scale = vel_mag/vel_norm_xy;
            VelocitySetpoint[0] *= scale;
            VelocitySetpoint[1] *= scale;
        }
	}

	LimitAltitude();

	if (RunAltControl)
	{
		VelocitySetpoint[2] = (PositionSetpoint[2] - Position[2]) * PosP[2];
	}

	/* Make sure velocity setpoint is saturated in xy. */
	float vel_norm_xy = sqrtf(VelocitySetpoint[0] * VelocitySetpoint[0] +
			VelocitySetpoint[1] * VelocitySetpoint[1]);

	SlowLandGradualVelocityLimit();

	/* We are close to target and want to limit velocity in xy */
	if (LimitVelXY)
	{
		LimitVelXYGradually();
	}

	if (!VehicleControlModeMsg.ControlPositionEnabled)
	{
		ResetPositionSetpoint = true;
	}

	if (!VehicleControlModeMsg.ControlAltitudeEnabled)
	{
		ResetAltitudeSetpoint = true;
	}

	if (!VehicleControlModeMsg.ControlVelocityEnabled)
	{
		VelocitySetpointPrevious[0] = Velocity[0];
		VelocitySetpointPrevious[1] = Velocity[1];
		VelocitySetpoint[0] = 0.0f;
		VelocitySetpoint[1] = 0.0f;
	}

	if (!VehicleControlModeMsg.ControlClimbRateEnabled)
	{
		VelocitySetpoint[2] = 0.0f;
	}

	/* Limit vertical takeoff speed if we are in auto takeoff. */
	if (PositionSetpointTripletMsg.Current.Valid
	    && PositionSetpointTripletMsg.Current.Type == PX4_SETPOINT_TYPE_TAKEOFF
	    && !VehicleControlModeMsg.ControlManualEnabled)
	{
		VelocitySetpoint[2] = math::max(VelocitySetpoint[2], -ConfigTblPtr->TKO_SPEED);
	}

	/* Apply slew rate (aka acceleration limit) for smooth flying. */
	ApplyVelocitySetpointSlewRate(dt);
	VelocitySetpointPrevious = VelocitySetpoint;

	/* Make sure velocity setpoint is constrained in all directions. */
	if (vel_norm_xy > ConfigTblPtr->XY_VEL_MAX)
	{
		VelocitySetpoint[0] = VelocitySetpoint[0] * ConfigTblPtr->XY_VEL_MAX / vel_norm_xy;
		VelocitySetpoint[1] = VelocitySetpoint[1] * ConfigTblPtr->XY_VEL_MAX / vel_norm_xy;
	}

	VelocitySetpoint[2] = math::max(VelocitySetpoint[2], -ConfigTblPtr->Z_VEL_MAX_UP);

	/* Special velocity setpoint limitation for smooth takeoff. */
	if (InTakeoff)
	{
		InTakeoff = TakeoffVelLimit < -VelocitySetpoint[2];
		/* Ramp vertical velocity limit up to takeoff speed. */
		TakeoffVelLimit += -VelocitySetpoint[2] * dt / ConfigTblPtr->TKO_RAMP_T;
		/* Limit vertical velocity to the current ramp value. */
		VelocitySetpoint[2] = math::max(VelocitySetpoint[2], -TakeoffVelLimit);
	}

	/* Publish velocity setpoint */
	VehicleGlobalVelocitySetpointMsg.Timestamp = PX4LIB_GetPX4TimeUs();
	VehicleGlobalVelocitySetpointMsg.VX = VelocitySetpoint[0];
	VehicleGlobalVelocitySetpointMsg.VY = VelocitySetpoint[1];
	VehicleGlobalVelocitySetpointMsg.VZ = VelocitySetpoint[2];

	SendVehicleGlobalVelocitySetpointMsg();
}



void MPC::CalculateThrustSetpoint(float dt)
{
	/* Reset integrals if needed. */
	if (VehicleControlModeMsg.ControlClimbRateEnabled)
	{
		if (ResetIntZ)
		{
			ResetIntZ = false;
			ThrustInt[2] = 0.0f;
		}
	}
	else
	{
		ResetIntZ = true;
	}

	if (VehicleControlModeMsg.ControlVelocityEnabled)
	{
		if (ResetIntXY)
		{
			ResetIntXY = false;
			ThrustInt[0] = 0.0f;
			ThrustInt[1] = 0.0f;
		}
	}
	else
	{
		ResetIntXY = true;
	}

	/* Velocity error */
	math::Vector3F vel_err = VelocitySetpoint - Velocity;

	/* Thrust vector in NED frame. */
	math::Vector3F thrust_sp;

	if (VehicleControlModeMsg.ControlAccelerationEnabled && PositionSetpointTripletMsg.Current.AccelerationValid)
	{
		thrust_sp = math::Vector3F(PositionSetpointTripletMsg.Current.AX, PositionSetpointTripletMsg.Current.AY, PositionSetpointTripletMsg.Current.AZ);
	}
	else
	{
		thrust_sp = vel_err.EMult(VelP) + VelocityErrD.EMult(VelD)
			    + ThrustInt - math::Vector3F(0.0f, 0.0f, ConfigTblPtr->THR_HOVER);
	}

	if (!VehicleControlModeMsg.ControlVelocityEnabled && !VehicleControlModeMsg.ControlAccelerationEnabled)
	{
		thrust_sp[0] = 0.0f;
		thrust_sp[1] = 0.0f;
	}

	/* If still or already on ground command zero xy velocity and zero xy
	 * thrust_sp in body frame to consider uneven ground. */
	if (VehicleLandDetectedMsg.GroundContact && !InAutoTakeoff())
	{
		/* Thrust setpoint in body frame*/
		math::Vector3F thrust_sp_body = Rotation.Transpose() * thrust_sp;

		/* We dont want to make any correction in body x and y*/
		thrust_sp_body[0] = 0.0f;
		thrust_sp_body[1] = 0.0f;

		/* Make sure z component of thrust_sp_body is larger than 0 (positive thrust is downward) */
		thrust_sp_body[2] = thrust_sp[2] > 0.0f ? thrust_sp[2] : 0.0f;

		/* Convert back to local frame (NED) */
		thrust_sp = Rotation * thrust_sp_body;

		/* Set velocity setpoint to zero and reset position. */
		VelocitySetpoint[0] = 0.0f;
		VelocitySetpoint[1] = 0.0f;
		PositionSetpoint[0] = Position[0];
		PositionSetpoint[1] = Position[1];
	}

	if (!VehicleControlModeMsg.ControlClimbRateEnabled && !VehicleControlModeMsg.ControlAccelerationEnabled)
	{
		thrust_sp[2] = 0.0f;
	}

	/* Limit thrust vector and check for saturation. */
	bool saturation_xy = false;
	bool saturation_z = false;

	/* Limit min lift */
	float thr_min = ConfigTblPtr->THR_MIN;

	if (!VehicleControlModeMsg.ControlVelocityEnabled && thr_min < 0.0f)
	{
		/* Don't allow downside thrust direction in manual attitude mode. */
		thr_min = 0.0f;
	}

	float tilt_max = math::radians(ConfigTblPtr->TILTMAX_AIR);
	float thr_max = ConfigTblPtr->THR_MAX;

	/* Filter vel_z over 1/8sec */
	VelZLp = VelZLp * (1.0f - dt * 8.0f) + dt * 8.0f * Velocity[2];

	/* Filter vel_z change over 1/8sec */
	float vel_z_change = (Velocity[2] - VelocityPrevious[2]) / dt;
	AccZLp = AccZLp * (1.0f - dt * 8.0f) + dt * 8.0f * vel_z_change;

	/* We can only run the control if we're already in-air, have a takeoff setpoint,
	 * or if we're in offboard control.  Otherwise, we should just bail out. */
	if (VehicleLandDetectedMsg.Landed && !InAutoTakeoff())
	{
		/* Keep throttle low while still on ground. */
		thr_max = 0.0f;
	}
	else if (!VehicleControlModeMsg.ControlManualEnabled && PositionSetpointTripletMsg.Current.Valid &&
			PositionSetpointTripletMsg.Current.Type == PX4_SETPOINT_TYPE_LAND)
	{
		/* Adjust limits for landing mode.  Limit max tilt and min lift when landing. */
		tilt_max = math::radians(ConfigTblPtr->TILTMAX_LND);

		if (thr_min < 0.0f)
		{
			thr_min = 0.0f;
		}

		/* Descent stabilized.  We are landing. */
		if (!InLanding && !LndReachedGround
		    && (float)fabsf(AccZLp) < 0.1f
		    && VelZLp > 0.6f * ConfigTblPtr->LAND_SPEED)
		{
			InLanding = true;
		}

		float land_z_threshold = 0.1f;

		/* Assume ground.  cut thrust */
		if (InLanding
		    && VelZLp < land_z_threshold)
		{
			thr_max = 0.0f;
			InLanding = false;
			LndReachedGround = true;
		}
		else if(InLanding && VelZLp < math::min(0.3f * ConfigTblPtr->LAND_SPEED, 2.5f * land_z_threshold))
        {
			/* Not on ground but with ground contact, stop position and velocity control. */
			thrust_sp[0] = 0.0f;
			thrust_sp[1] = 0.0f;
			VelocitySetpoint[0] = Velocity[0];
			VelocitySetpoint[1] = Velocity[1];
			PositionSetpoint[0] = Position[0];
			PositionSetpoint[1] = Position[1];
		}

		/* Once we assumed to have reached the ground, always cut the thrust.
		 * Only free fall detection below can revoke this.
		 */
		if (!InLanding && LndReachedGround)
		{
			thr_max = 0.0f;
		}

		/* If we suddenly fall, reset landing logic and remove thrust limit. */
		if (LndReachedGround
		    /* XXX: magic value, assuming free fall above 4m/s2 acceleration */
		    && (AccZLp > 4.0f
			|| VelZLp > 2.0f * ConfigTblPtr->LAND_SPEED))
		{
			thr_max = ConfigTblPtr->THR_MAX;
			InLanding = true;
			LndReachedGround = false;
		}
	}
	else
	{
		InLanding = false;
		LndReachedGround = false;
	}

	/* Limit min lift */
	if (-thrust_sp[2] < thr_min)
	{
		thrust_sp[2] = -thr_min;

		/* Don't freeze altitude integral if it wants to throttle up. */
		saturation_z = vel_err[2] > 0.0f ? true : saturation_z;
	}

	if (VehicleControlModeMsg.ControlVelocityEnabled || VehicleControlModeMsg.ControlAccelerationEnabled)
	{
		/* Limit max tilt */
		if (thr_min >= 0.0f && tilt_max < M_PI / 2 - 0.05f)
		{
			/* Absolute horizontal thrust */
			float thrust_sp_xy_len = math::Vector2F(thrust_sp[0], thrust_sp[1]).Length();

			if (thrust_sp_xy_len > 0.01f)
			{
				/* Max horizontal thrust for given vertical thrust. */
				float thrust_xy_max = -thrust_sp[2] * tanf(tilt_max);

				if (thrust_sp_xy_len > thrust_xy_max)
				{
					float k = thrust_xy_max / thrust_sp_xy_len;
					thrust_sp[0] *= k;
					thrust_sp[1] *= k;

					/* Don't freeze x,y integrals if they both want to throttle down. */
					saturation_xy = ((vel_err[0] * VelocitySetpoint[0] < 0.0f) && (vel_err[1] * VelocitySetpoint[1] < 0.0f)) ? saturation_xy : true;
				}
			}
		}
	}

	if (VehicleControlModeMsg.ControlClimbRateEnabled && !VehicleControlModeMsg.ControlVelocityEnabled)
	{
		/* Thrust compensation when vertical velocity but not horizontal velocity is controlled. */
		float att_comp;

		const float tilt_cos_max = 0.7f;

		if (Rotation[2][2] > tilt_cos_max)
		{
			att_comp = 1.0f / Rotation[2][2];
		}
		else if (Rotation[2][2] > 0.0f)
		{
			att_comp = ((1.0f / tilt_cos_max - 1.0f) / tilt_cos_max) * Rotation[2][2] + 1.0f;
			saturation_z = true;
		}
		else
		{
			att_comp = 1.0f;
			saturation_z = true;
		}

		thrust_sp[2] *= att_comp;
	}

	/* Calculate desired total thrust amount in body z direction. */
	/* To compensate for excess thrust during attitude tracking errors we
	 * project the desired thrust force vector F onto the real vehicle's thrust axis in NED:
	 * body thrust axis [0,0,-1]' rotated by R is: R*[0,0,-1]' = -R_z */
	math::Vector3F R_z(Rotation[0][2], Rotation[1][2], Rotation[2][2]);
	math::Vector3F F(thrust_sp);

	/* Recalculate because it might have changed. */
	float thrust_body_z = F * -R_z;

	/* Limit max thrust. */
	if (fabsf(thrust_body_z) > thr_max)
	{
		if (thrust_sp[2] < 0.0f)
		{
			if (-thrust_sp[2] > thr_max)
			{
				/* Thrust Z component is too large, limit it. */
				thrust_sp[0] = 0.0f;
				thrust_sp[1] = 0.0f;
				thrust_sp[2] = -thr_max;
				saturation_xy = true;

				/* Don't freeze altitude integral if it wants to throttle down. */
				saturation_z = vel_err[2] < 0.0f ? true : saturation_z;
			}
			else
			{
				/* Preserve thrust Z component and lower XY, keeping altitude is more important than position. */
				float thrust_xy_max = sqrtf(thr_max * thr_max - thrust_sp[2] * thrust_sp[2]);
				float thrust_xy_abs = math::Vector2F(thrust_sp[0], thrust_sp[1]).Length();
				float k = thrust_xy_max / thrust_xy_abs;
				thrust_sp[0] *= k;
				thrust_sp[1] *= k;
				/* Don't freeze x,y integrals if they both want to throttle down */
				saturation_xy = ((vel_err[0] * VelocitySetpoint[0] < 0.0f) && (vel_err[1] * VelocitySetpoint[1] < 0.0f)) ? saturation_xy : true;
			}
		}
		else
		{
			/* Z component is positive, going down (Z is positive down in NED), simply limit thrust vector. */
			float k = thr_max / fabsf(thrust_body_z);
			thrust_sp = thrust_sp * k;
			saturation_xy = true;
			saturation_z = true;
		}

		thrust_body_z = thr_max;
	}

	VehicleAttitudeSetpointMsg.Thrust = math::max(thrust_body_z, thr_min);

	/* Update integrals */
	if (VehicleControlModeMsg.ControlVelocityEnabled && !saturation_xy)
	{
		ThrustInt[0] += vel_err[0] * VelI[0] * dt;
		ThrustInt[1] += vel_err[1] * VelI[1] * dt;
	}

	if (VehicleControlModeMsg.ControlClimbRateEnabled && !saturation_z)
	{
		ThrustInt[2] += vel_err[2] * VelI[2] * dt;
	}

	/* Calculate attitude setpoint from thrust vector. */
	if (VehicleControlModeMsg.ControlVelocityEnabled || VehicleControlModeMsg.ControlAccelerationEnabled)
	{
		/* Desired body_z axis = -normalize(thrust_vector) */
		math::Vector3F body_x;
		math::Vector3F body_y;
		math::Vector3F body_z;

		if (thrust_sp.Length() > FLT_EPSILON)
		{
			body_z = -thrust_sp.Normalized();
		}
		else
		{
			/* No thrust, set Z axis to safe value. */
			body_z.Zero();
			body_z[2] = 1.0f;
		}

		/* Vector of desired yaw direction in XY plane, rotated by PI/2. */
		math::Vector3F y_C(-sinf(VehicleAttitudeSetpointMsg.YawBody), cosf(VehicleAttitudeSetpointMsg.YawBody), 0.0f);

		if (fabsf(body_z[2]) > FLT_EPSILON)
		{
			/* Desired body_x axis, orthogonal to body_z. */
			body_x = y_C % body_z;

			/* Keep nose to front while inverted upside down. */
			if (body_z[2] < 0.0f)
			{
				body_x = -body_x;
			}

			body_x.Normalize();
		}
		else
		{
			/* Desired thrust is in XY plane, set X downside to construct
			 * correct matrix, but yaw component will not be used actually */
			body_x.Zero();
			body_x[2] = 1.0f;
		}

		/* Desired body_y axis */
		body_y = body_z % body_x;

		/* Fill rotation matrix */
		for (uint32 i = 0; i < 3; i++)
		{
			RSetpoint[i][0] = body_x[i];
			RSetpoint[i][1] = body_y[i];
			RSetpoint[i][2] = body_z[i];
		}

		/* Copy quaternion setpoint to attitude setpoint topic. */
		math::Quaternion q_sp(RSetpoint);
		q_sp.copyTo(VehicleAttitudeSetpointMsg.Q_D);
		VehicleAttitudeSetpointMsg.Q_D_Valid = true;

		/* Calculate euler angles, for logging only.  Must not be used for
		 * control. */
		math::Vector3F euler = RSetpoint.ToEuler();
		VehicleAttitudeSetpointMsg.RollBody = euler[0];
		VehicleAttitudeSetpointMsg.PitchBody = euler[1];
		/* Yaw already used to construct rot matrix, but actual rotation
		 * matrix can have different yaw near singularity. */
	}
	else if (!VehicleControlModeMsg.ControlManualEnabled)
	{
		/* Autonomous altitude control without position control (failsafe
		 * landing).  Force level attitude, don't change yaw. */
		RSetpoint = math::Matrix3F3::FromEuler(0.0f, 0.0f, VehicleAttitudeSetpointMsg.YawBody);

		/* Copy quaternion setpoint to attitude setpoint topic. */
		math::Quaternion q_sp(RSetpoint);
		q_sp.copyTo(VehicleAttitudeSetpointMsg.Q_D);
		VehicleAttitudeSetpointMsg.Q_D_Valid = true;

		VehicleAttitudeSetpointMsg.RollBody = 0.0f;
		VehicleAttitudeSetpointMsg.PitchBody = 0.0f;
	}

	/* Save thrust setpoint for logging. */
	VehicleLocalPositionSetpointMsg.AccX = thrust_sp[0] * MPC_CONSTANTS_ONE_G;
	VehicleLocalPositionSetpointMsg.AccY = thrust_sp[1] * MPC_CONSTANTS_ONE_G;
	VehicleLocalPositionSetpointMsg.AccZ = thrust_sp[2] * MPC_CONSTANTS_ONE_G;

	VehicleAttitudeSetpointMsg.Timestamp = PX4LIB_GetPX4TimeUs();
}



float MPC::GetCruisingSpeedXY(void)
{
	float res;

	/*
	 * In missions, the user can choose cruising speed different to default.
	 */
	res = ((isfinite(PositionSetpointTripletMsg.Current.CruisingSpeed) &&
			(PositionSetpointTripletMsg.Current.CruisingSpeed > 0.1f)) ?
					PositionSetpointTripletMsg.Current.CruisingSpeed : ConfigTblPtr->XY_CRUISE);

	return res;
}



bool MPC::CrossSphereLine(const math::Vector3F &sphere_c, const float sphere_r,
		const math::Vector3F &line_a, const math::Vector3F &line_b, math::Vector3F &res)
{
	bool result;

	/* Project center of sphere on line normalized AB. */
	math::Vector3F ab_norm = line_b - line_a;
	ab_norm.Normalize();
	math::Vector3F d = line_a + ab_norm * ((sphere_c - line_a) * ab_norm);
	float cd_len = (sphere_c - d).Length();

	if (sphere_r > cd_len)
	{
		/* We have triangle CDX with known CD and CX = R, find DX. */
		float dx_len = sqrtf(sphere_r * sphere_r - cd_len * cd_len);

		if ((sphere_c - line_b) * ab_norm > 0.0f)
		{
			/* Target waypoint is already behind us. */
			res = line_b;
		}
		else
		{
			/* Target is in front of us. */
			/* vector A->B on line */
			res = d + ab_norm * dx_len;
		}

		result = true;
	}
	else
	{
		/* Have no roots. Return D */
		/* Go directly to line */
		res = d;

		/* Previous waypoint is still in front of us. */
		if ((sphere_c - line_a) * ab_norm < 0.0f) {
			res = line_a;
		}

		/* Target waypoint is already behind us. */
		if ((sphere_c - line_b) * ab_norm > 0.0f) {
			res = line_b;
		}

		result = false;
	}

	return result;
}



void MPC::UpdateParamsFromTable(void)
{
	if(ConfigTblPtr != 0)
	{
		PosP[0] = ConfigTblPtr->XY_P;
		PosP[1] = ConfigTblPtr->XY_P;
		PosP[2] = ConfigTblPtr->Z_P;

		VelP[0] = ConfigTblPtr->XY_VEL_P;
		VelP[1] = ConfigTblPtr->XY_VEL_P;
		VelP[2] = ConfigTblPtr->Z_VEL_P;

		VelI[0] = ConfigTblPtr->XY_VEL_I;
		VelI[1] = ConfigTblPtr->XY_VEL_I;
		VelI[2] = ConfigTblPtr->Z_VEL_I;

		VelD[0] = ConfigTblPtr->XY_VEL_D;
		VelD[1] = ConfigTblPtr->XY_VEL_D;
		VelD[2] = ConfigTblPtr->Z_VEL_D;
	}
}



/**
 * Limit altitude based on several conditions
 */
void MPC::LimitAltitude(void)
{
	/* In altitude control, limit setpoint. */
	if (RunAltControl && PositionSetpoint[2] <= -VehicleLandDetectedMsg.AltMax)
	{
		PositionSetpoint[2] = -VehicleLandDetectedMsg.AltMax;
	}
	else
	{
		/* In velocity control mode and want to fly upwards. */
		if (!RunAltControl && VelocitySetpoint[2] <= 0.0f)
		{
			/* Time to travel to reach zero velocity. */
			float delta_t = -Velocity[2] / ConfigTblPtr->ACC_DOWN_MAX;

			/* Predicted position */
			float pos_z_next = Position[2] + Velocity[2] * delta_t + 0.5f *
					ConfigTblPtr->ACC_DOWN_MAX * delta_t * delta_t;

			if (pos_z_next <= -VehicleLandDetectedMsg.AltMax)
			{
				PositionSetpoint[2] = -VehicleLandDetectedMsg.AltMax;
				RunAltControl = true;
				return;
			}
		}
	}
}



void MPC::SlowLandGradualVelocityLimit(void)
{
	/*
	 * Make sure downward velocity (positive Z) is limited close to ground.
	 * for now we use the home altitude and assume that we want to land on a similar absolute altitude.
	 */
	float altitude_above_home = -Position[2] + HomePositionMsg.Z;
	float vel_limit = ConfigTblPtr->Z_VEL_MAX_DN;

	if (altitude_above_home < ConfigTblPtr->LAND_ALT2)
	{
		vel_limit = ConfigTblPtr->LAND_SPEED;
	}
	else if(altitude_above_home < ConfigTblPtr->LAND_ALT1)
	{
		/* Linear function between the two altitudes. */
		float a = (ConfigTblPtr->Z_VEL_MAX_DN - ConfigTblPtr->LAND_SPEED) / (ConfigTblPtr->LAND_ALT1 - ConfigTblPtr->LAND_ALT2);
		float b = ConfigTblPtr->LAND_SPEED - a * ConfigTblPtr->LAND_ALT2;
		vel_limit =  a * altitude_above_home + b;
	}

	VelocitySetpoint[2] = math::min(VelocitySetpoint[2], vel_limit);
}



void MPC::LimitVelXYGradually(void)
{
	/*
	 * The max velocity is defined by the linear line
	 * with x= (curr_sp - pos) and y = VelocitySetpoint with min limit of 0.01
	 */
	math::Vector3F dist = CurrentPositionSetpoint - Position;
	float slope = (GetCruisingSpeedXY() - 0.01f)  / ConfigTblPtr->TARGET_THRE;
	float vel_limit =  slope * sqrtf(dist[0] * dist[0] + dist[1] * dist[1]) + 0.01f;
	float vel_mag_xy = sqrtf(VelocitySetpoint[0] * VelocitySetpoint[0] + VelocitySetpoint[1] * VelocitySetpoint[1]);

	if (vel_mag_xy <= vel_limit)
	{
		return;
	}

	VelocitySetpoint[0] = VelocitySetpoint[0] / vel_mag_xy * vel_limit;
	VelocitySetpoint[1] = VelocitySetpoint[1] / vel_mag_xy * vel_limit;
}



void MPC::ApplyVelocitySetpointSlewRate(float dt)
{
	math::Vector3F acc = (VelocitySetpoint - VelocitySetpointPrevious) / dt;
	float acc_xy_mag = sqrtf(acc[0] * acc[0] + acc[1] * acc[1]);

	float acc_limit = ConfigTblPtr->ACC_HOR_MAX;

	/* Adapt slew rate if we are decelerating */
	if (Velocity * acc < 0)
	{
		acc_limit = ConfigTblPtr->DEC_HOR_MAX;
	}

	/* Limit total horizontal acceleration */
	if (acc_xy_mag > acc_limit)
	{
		VelocitySetpoint[0] = acc_limit * acc[0] / acc_xy_mag * dt + VelocitySetpointPrevious[0];
		VelocitySetpoint[1] = acc_limit * acc[1] / acc_xy_mag * dt + VelocitySetpointPrevious[1];
	}

	/* Limit vertical acceleration */
	float max_acc_z = acc[2] < 0.0f ? -ConfigTblPtr->ACC_UP_MAX : ConfigTblPtr->ACC_DOWN_MAX;

	if (fabsf(acc[2]) > fabsf(max_acc_z))
	{
		VelocitySetpoint[2] = max_acc_z * dt + VelocitySetpointPrevious[2];
	}
}



bool MPC::InAutoTakeoff(void)
{
	bool res = false;

	/*
	 * In auto mode, check if we do a takeoff
	 */
	if(PositionSetpointTripletMsg.Current.Valid &&
			PositionSetpointTripletMsg.Current.Type == PX4_SETPOINT_TYPE_TAKEOFF)
	{
	    /* We are in takeoff mode. */
	    res = true;
	}
	else if( VehicleControlModeMsg.ControlOffboardEnabled)
	{
		/* We are in takeoff mode. */
		res = true;
	}

	return res;
}

/************************/
/*  End of File Comment */
/************************/
