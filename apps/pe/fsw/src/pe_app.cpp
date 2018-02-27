/************************************************************************
** Includes
*************************************************************************/
#include <string.h>

#include "cfe.h"

#include "pe_app.h"
#include "pe_msg.h"
#include "pe_version.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Instantiate the application object.                             */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
PE oPE;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Default constructor.                                            */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
PE::PE()
{

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Destructor constructor.                                         */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
PE::~PE()
{

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Initialize event tables.                                        */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 PE::InitEvent()
{
    int32  iStatus=CFE_SUCCESS;
    int32  ind = 0;

    CFE_EVS_BinFilter_t   EventTbl[PE_MAX_EVENT_FILTERS];

    /* Initialize the event filter table.
     * Note: 0 is the CFE_EVS_NO_FILTER mask and event 0 is reserved (not used) */
    memset(EventTbl, 0x00, sizeof(EventTbl));
    
    /* TODO: Choose the events you want to filter.  CFE_EVS_MAX_EVENT_FILTERS
     * limits the number of filters per app.  An explicit CFE_EVS_NO_FILTER 
     * (the default) has been provided as an example. */
    EventTbl[  ind].EventID = PE_RESERVED_EID;
    EventTbl[ind++].Mask    = CFE_EVS_NO_FILTER;
    EventTbl[  ind].EventID = PE_ESTIMATOR_INF_EID;
    EventTbl[ind++].Mask    = CFE_EVS_NO_FILTER;
    EventTbl[  ind].EventID = PE_ESTIMATOR_ERR_EID;
    EventTbl[ind++].Mask    = CFE_EVS_NO_FILTER;
    EventTbl[  ind].EventID = PE_BARO_FAULT_ERR_EID;
    EventTbl[ind++].Mask    = CFE_EVS_NO_FILTER;
    EventTbl[  ind].EventID = PE_BARO_OK_INF_EID;
    EventTbl[ind++].Mask    = CFE_EVS_NO_FILTER;
    EventTbl[  ind].EventID = PE_BARO_TIMEOUT_ERR_EID;
    EventTbl[ind++].Mask    = CFE_EVS_NO_FILTER;
    EventTbl[  ind].EventID = PE_GPS_TIMEOUT_ERR_EID;
    EventTbl[ind++].Mask    = CFE_EVS_NO_FILTER;
    EventTbl[  ind].EventID = PE_GPS_FAULT_ERR_EID;
    EventTbl[ind++].Mask    = CFE_EVS_NO_FILTER;
    EventTbl[  ind].EventID = PE_GPS_OK_INF_EID;
    EventTbl[ind++].Mask    = CFE_EVS_NO_FILTER;
    EventTbl[  ind].EventID = PE_LAND_FAULT_ERR_EID;
    EventTbl[ind++].Mask    = CFE_EVS_NO_FILTER;
    EventTbl[  ind].EventID = PE_LAND_OK_INF_EID;
    EventTbl[ind++].Mask    = CFE_EVS_NO_FILTER;
    EventTbl[  ind].EventID = PE_LAND_TIMEOUT_ERR_EID;
    EventTbl[ind++].Mask    = CFE_EVS_NO_FILTER;

    /* Register the table with CFE */
    iStatus = CFE_EVS_Register(EventTbl, ind, CFE_EVS_BINARY_FILTER);
    if (iStatus != CFE_SUCCESS)
    {
        (void) CFE_ES_WriteToSysLog("PE - Failed to register with EVS (0x%08lX)\n", iStatus);
    }

    return iStatus;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Initialize Message Pipes                                        */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 PE::InitPipe()
{
    int32  iStatus=CFE_SUCCESS;

    /* Init schedule pipe and subscribe to wakeup messages */
    iStatus = CFE_SB_CreatePipe(&SchPipeId,
    		PE_SCH_PIPE_DEPTH,
			PE_SCH_PIPE_NAME);
    if (iStatus == CFE_SUCCESS)
    {
        iStatus = CFE_SB_SubscribeEx(PE_WAKEUP_MID, SchPipeId, CFE_SB_Default_Qos, PE_WAKEUP_MID_MAX_MSG_COUNT);
        if (iStatus != CFE_SUCCESS)
        {
            (void) CFE_EVS_SendEvent(PE_SUBSCRIBE_ERR_EID, CFE_EVS_ERROR,
            		"Sch Pipe failed to subscribe to PE_WAKEUP_MID. (0x%08lX)",
                    iStatus);
            goto PE_InitPipe_Exit_Tag;
        }

        iStatus = CFE_SB_SubscribeEx(PE_SEND_HK_MID, SchPipeId, CFE_SB_Default_Qos, PE_SEND_HK_MID_MAX_MSG_COUNT);
        if (iStatus != CFE_SUCCESS)
        {
            (void) CFE_EVS_SendEvent(PE_SUBSCRIBE_ERR_EID, CFE_EVS_ERROR,
					 "CMD Pipe failed to subscribe to PE_SEND_HK_MID. (0x%08X)",
					 (unsigned int)iStatus);
            goto PE_InitPipe_Exit_Tag;
        }
        iStatus = CFE_SB_SubscribeEx(PX4_VEHICLE_GPS_POSITION_MID, SchPipeId, CFE_SB_Default_Qos, 1);
        if (iStatus != CFE_SUCCESS)
        {
            (void) CFE_EVS_SendEvent(PE_SUBSCRIBE_ERR_EID, CFE_EVS_ERROR,
					 "CMD Pipe failed to subscribe to PX4_VEHICLE_GPS_POSITION_MID. (0x%08lX)",
					 iStatus);
            goto PE_InitPipe_Exit_Tag;
        }
        iStatus = CFE_SB_SubscribeEx(PX4_VEHICLE_STATUS_MID, SchPipeId, CFE_SB_Default_Qos, 1);
        if (iStatus != CFE_SUCCESS)
        {
            (void) CFE_EVS_SendEvent(PE_SUBSCRIBE_ERR_EID, CFE_EVS_ERROR,
					 "CMD Pipe failed to subscribe to PX4_VEHICLE_STATUS_MID. (0x%08lX)",
					 iStatus);
            goto PE_InitPipe_Exit_Tag;
        }
        iStatus = CFE_SB_SubscribeEx(PX4_VEHICLE_LAND_DETECTED_MID, SchPipeId, CFE_SB_Default_Qos, 1);
        if (iStatus != CFE_SUCCESS)
        {
            (void) CFE_EVS_SendEvent(PE_SUBSCRIBE_ERR_EID, CFE_EVS_ERROR,
					 "CMD Pipe failed to subscribe to PX4_VEHICLE_LAND_DETECTED_MID. (0x%08lX)",
					 iStatus);
            goto PE_InitPipe_Exit_Tag;
        }
        iStatus = CFE_SB_SubscribeEx(PX4_ACTUATOR_ARMED_MID, SchPipeId, CFE_SB_Default_Qos, 1);
        if (iStatus != CFE_SUCCESS)
        {
            (void) CFE_EVS_SendEvent(PE_SUBSCRIBE_ERR_EID, CFE_EVS_ERROR,
					 "CMD Pipe failed to subscribe to PX4_ACTUATOR_ARMED_MID. (0x%08lX)",
					 iStatus);
            goto PE_InitPipe_Exit_Tag;
        }
        iStatus = CFE_SB_SubscribeEx(PX4_VEHICLE_ATTITUDE_MID, SchPipeId, CFE_SB_Default_Qos, 1);
        if (iStatus != CFE_SUCCESS)
        {
            (void) CFE_EVS_SendEvent(PE_SUBSCRIBE_ERR_EID, CFE_EVS_ERROR,
					 "CMD Pipe failed to subscribe to PX4_VEHICLE_ATTITUDE_MID. (0x%08lX)",
					 iStatus);
            goto PE_InitPipe_Exit_Tag;
        }
        iStatus = CFE_SB_SubscribeEx(PX4_VEHICLE_CONTROL_MODE_MID, SchPipeId, CFE_SB_Default_Qos, 1);
        if (iStatus != CFE_SUCCESS)
        {
            (void) CFE_EVS_SendEvent(PE_SUBSCRIBE_ERR_EID, CFE_EVS_ERROR,
					 "CMD Pipe failed to subscribe to PX4_VEHICLE_CONTROL_MODE_MID. (0x%08lX)",
					 iStatus);
            goto PE_InitPipe_Exit_Tag;
        }
        iStatus = CFE_SB_SubscribeEx(PX4_SENSOR_COMBINED_MID, SchPipeId, CFE_SB_Default_Qos, 1);
        if (iStatus != CFE_SUCCESS)
        {
            (void) CFE_EVS_SendEvent(PE_SUBSCRIBE_ERR_EID, CFE_EVS_ERROR,
					 "CMD Pipe failed to subscribe to PX4_SENSOR_COMBINED_MID. (0x%08lX)",
					 iStatus);
            goto PE_InitPipe_Exit_Tag;
        }
        iStatus = CFE_SB_SubscribeEx(PX4_VEHICLE_ATTITUDE_SETPOINT_MID, SchPipeId, CFE_SB_Default_Qos, 1);
        if (iStatus != CFE_SUCCESS)
        {
            (void) CFE_EVS_SendEvent(PE_SUBSCRIBE_ERR_EID, CFE_EVS_ERROR,
					 "CMD Pipe failed to subscribe to PX4_VEHICLE_ATTITUDE_SETPOINT_MID. (0x%08lX)",
					 iStatus);
            goto PE_InitPipe_Exit_Tag;
        }
        iStatus = CFE_SB_SubscribeEx(PX4_MANUAL_CONTROL_SETPOINT_MID, SchPipeId, CFE_SB_Default_Qos, 1);
        if (iStatus != CFE_SUCCESS)
        {
            (void) CFE_EVS_SendEvent(PE_SUBSCRIBE_ERR_EID, CFE_EVS_ERROR,
					 "CMD Pipe failed to subscribe to PX4_MANUAL_CONTROL_SETPOINT_MID. (0x%08lX)",
					 iStatus);
            goto PE_InitPipe_Exit_Tag;
        }
    }
    else
    {
        (void) CFE_EVS_SendEvent(PE_PIPE_INIT_ERR_EID, CFE_EVS_ERROR,
			 "Failed to create SCH pipe (0x%08lX)",
			 iStatus);
        goto PE_InitPipe_Exit_Tag;
    }

    /* Init command pipe and subscribe to command messages */
    iStatus = CFE_SB_CreatePipe(&CmdPipeId,
    		PE_CMD_PIPE_DEPTH,
			PE_CMD_PIPE_NAME);
    if (iStatus == CFE_SUCCESS)
    {
        /* Subscribe to command messages */
        iStatus = CFE_SB_Subscribe(PE_CMD_MID, CmdPipeId);

        if (iStatus != CFE_SUCCESS)
        {
            (void) CFE_EVS_SendEvent(PE_SUBSCRIBE_ERR_EID, CFE_EVS_ERROR,
				 "CMD Pipe failed to subscribe to PE_CMD_MID. (0x%08lX)",
				 iStatus);
            goto PE_InitPipe_Exit_Tag;
        }
    }
    else
    {
        (void) CFE_EVS_SendEvent(PE_PIPE_INIT_ERR_EID, CFE_EVS_ERROR,
			 "Failed to create CMD pipe (0x%08lX)",
			 iStatus);
        goto PE_InitPipe_Exit_Tag;
    }

PE_InitPipe_Exit_Tag:
    return iStatus;
}
    

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Initialize Global Variables                                     */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void PE::InitData()
{
    /* Init housekeeping message. */
    CFE_SB_InitMsg(&HkTlm, PE_HK_TLM_MID, sizeof(HkTlm), TRUE);
    /* Init output messages */
    /* Vehicle local position message*/
	CFE_SB_InitMsg(&m_VehicleLocalPositionMsg,
			PX4_VEHICLE_LOCAL_POSITION_MID, sizeof(PX4_VehicleLocalPositionMsg_t), TRUE);
    /* Estimator status message */
    CFE_SB_InitMsg(&m_EstimatorStatusMsg,
			PX4_ESTIMATOR_STATUS_MID, sizeof(PX4_EstimatorStatusMsg_t), TRUE);
    /* Vehicle global position message */
    CFE_SB_InitMsg(&m_VehicleGlobalPositionMsg,
			PX4_VEHICLE_GLOBAL_POSITION_MID, sizeof(PX4_VehicleGlobalPositionMsg_t), TRUE);
    /* EKF2 innovations message  */
    CFE_SB_InitMsg(&m_Ekf2InnovationsMsg,
			PX4_EKF2_INNOVATIONS_MID, sizeof(PX4_Ekf2InnovationsMsg_t), TRUE);

	/* Set constants */
	DELAY_MAX           = 0.5f; // seconds
	HIST_STEP           = 0.05f; // 20 hz
	BIAS_MAX            = 1e-1f;
	HIST_LEN            = 10; // DELAY_MAX / HIST_STEP;
	N_DIST_SUBS         = 4;
	BETA_TABLE[0]       = 0.0f;
	BETA_TABLE[1]       = 8.82050518214f;
	BETA_TABLE[2]       = 12.094592431f;
	BETA_TABLE[3]       = 13.9876612368f;
	BETA_TABLE[4]       = 16.0875642296f;
	BETA_TABLE[5]       = 17.8797700658f;
	BETA_TABLE[6]       = 19.6465647819f;
	EST_STDDEV_XY_VALID = 2.0f;
	EST_STDDEV_Z_VALID  = 2.0f;
	EST_STDDEV_TZ_VALID = 2.0f;
	P_MAX               = 1.0e6f;
	LAND_RATE           = 10.0f;
	LOW_PASS_CUTOFF     = 5.0f;

    /* Params */
    memset(&m_Params, 0, sizeof(m_Params));
	UpdateLocalParams();

	/* Timestamps */
	m_Timestamp             = PX4LIB_GetPX4TimeUs();
	m_TimeLastBaro          = 0;
	m_TimeLastGps           = 0;
	m_TimeLastLand          = 0;
    m_Timestamp_Hist        = 0;
    m_TimestampLastBaro     = 0;

    /* Timeouts */
    m_BaroTimeout           = true;
    m_GpsTimeout            = true;
    m_LandTimeout           = true;

    /* Faults */
    m_BaroFault             = false;
    m_GpsFault              = false;
    m_LandFault             = false;

    /* Validity */
    m_XyEstValid            = false;
    m_ZEstValid             = false;
    m_TzEstValid            = false;

	/* Reference altitudes */
	m_AltOrigin             = 0.0f;
	m_BaroAltOrigin         = 0.0f;
	m_GpsAltOrigin          = 0.0f;

	/* Status */
	m_EstimatorInitialized  = FALSE;
	m_BaroInitialized 		= FALSE;
	m_GpsInitialized  		= FALSE;
	m_LandInitialized  		= FALSE;
	m_ReceivedGps           = FALSE;
	m_LastArmedState        = FALSE;
	m_AltOriginInitialized  = FALSE;
    m_ParamsUpdated         = FALSE;

    /* Matrix/Vector Zero */
    m_RotationMat.Zero();
    m_Euler.Zero();
    
    /* Stats Zero */
    m_BaroStats.reset();
    m_GpsStats.reset();
    
	/* State Space */
	m_StateVec.Zero();
	m_InputVec.Zero();
	InitStateSpace();

	/* Map */
	m_MapRef.init_done      = false;

    /* Initialize delay blocks */
    m_XDelay.Initialize();
    m_TDelay.Initialize();

}


void PE::initStateCov()
{
	m_StateCov.Zero();

	/* Initialize to twice valid condition */
	m_StateCov[X_x][X_x] = 2 * EST_STDDEV_XY_VALID * EST_STDDEV_XY_VALID;
	m_StateCov[X_y][X_y] = 2 * EST_STDDEV_XY_VALID * EST_STDDEV_XY_VALID;
	m_StateCov[X_z][X_z] = 2 * EST_STDDEV_Z_VALID * EST_STDDEV_Z_VALID;
	m_StateCov[X_vx][X_vx] = 2 * m_Params.VXY_PUB_THRESH * m_Params.VXY_PUB_THRESH;
	m_StateCov[X_vy][X_vy] = 2 * m_Params.VXY_PUB_THRESH * m_Params.VXY_PUB_THRESH;

	/* Use vxy threshold for vz init as well */
	m_StateCov[X_vz][X_vz] = 2 * m_Params.VXY_PUB_THRESH * m_Params.VXY_PUB_THRESH;

	/* Initialize bias uncertainty to small values to keep them stable */
	m_StateCov[X_bx][X_bx] = 1e-6;
	m_StateCov[X_by][X_by] = 1e-6;
	m_StateCov[X_bz][X_bz] = 1e-6;
	m_StateCov[X_tz][X_tz] = 2 * EST_STDDEV_TZ_VALID * EST_STDDEV_TZ_VALID;
}


void PE::InitStateSpace()
{
	initStateCov();

	/* Derivative of position is velocity */
	m_DynamicsMat.Zero();
	m_DynamicsMat[X_x][X_vx] = 1;
	m_DynamicsMat[X_y][X_vy] = 1;
	m_DynamicsMat[X_z][X_vz] = 1;

	/* Input matrix */
	m_InputMat.Zero();
	m_InputMat[X_vx][U_ax] = 1;
	m_InputMat[X_vy][U_ay] = 1;
	m_InputMat[X_vz][U_az] = 1;

	/* Update components that depend on current state */
	updateStateSpace();
	updateStateSpaceParams();
}


void PE::updateStateSpace()
{
	/* Derivative of velocity is accelerometer acceleration
	 * (in input matrix) - bias (in body frame) */
	m_DynamicsMat[X_vx][X_bx] = -m_RotationMat[0][0];
	m_DynamicsMat[X_vx][X_by] = -m_RotationMat[0][1];
	m_DynamicsMat[X_vx][X_bz] = -m_RotationMat[0][2];

	m_DynamicsMat[X_vy][X_bx] = -m_RotationMat[1][0];
	m_DynamicsMat[X_vy][X_by] = -m_RotationMat[1][1];
	m_DynamicsMat[X_vy][X_bz] = -m_RotationMat[1][2];

	m_DynamicsMat[X_vz][X_bx] = -m_RotationMat[2][0];
	m_DynamicsMat[X_vz][X_by] = -m_RotationMat[2][1];
	m_DynamicsMat[X_vz][X_bz] = -m_RotationMat[2][2];
}


void PE::updateStateSpaceParams()
{
	/* Input noise covariance matrix */
	m_InputCov.Zero();
	m_InputCov[U_ax][U_ax] = m_Params.ACCEL_XY_STDDEV * m_Params.ACCEL_XY_STDDEV;
	m_InputCov[U_ay][U_ay] = m_Params.ACCEL_XY_STDDEV * m_Params.ACCEL_XY_STDDEV;
	m_InputCov[U_az][U_az] = m_Params.ACCEL_Z_STDDEV * m_Params.ACCEL_Z_STDDEV;

	/* Process noise power matrix */
	m_NoiseCov.Zero();
	float pn_p_sq = m_Params.PN_P_NOISE_DENSITY * m_Params.PN_P_NOISE_DENSITY;
	float pn_v_sq = m_Params.PN_V_NOISE_DENSITY * m_Params.PN_V_NOISE_DENSITY;
	m_NoiseCov[X_x][X_x] = pn_p_sq;
	m_NoiseCov[X_y][X_y] = pn_p_sq;
	m_NoiseCov[X_z][X_z] = pn_p_sq;
	m_NoiseCov[X_vx][X_vx] = pn_v_sq;
	m_NoiseCov[X_vy][X_vy] = pn_v_sq;
	m_NoiseCov[X_vz][X_vz] = pn_v_sq;

	/* Technically the noise is in the body frame,
	* but the components are all the same, so
	* ignoring for now */
	float pn_b_sq = m_Params.PN_B_NOISE_DENSITY * m_Params.PN_B_NOISE_DENSITY;
	m_NoiseCov[X_bx][X_bx] = pn_b_sq;
	m_NoiseCov[X_by][X_by] = pn_b_sq;
	m_NoiseCov[X_bz][X_bz] = pn_b_sq;

	/* Terrain random walk noise ((m/s)/sqrt(hz)), scales with velocity */
	float pn_t_noise_density = m_Params.PN_T_NOISE_DENSITY +
		(m_Params.T_MAX_GRADE / 100.0f) * sqrtf(m_StateVec[X_vx] * 
        m_StateVec[X_vx] + m_StateVec[X_vy] * m_StateVec[X_vy]);
	m_NoiseCov[X_tz][X_tz] = pn_t_noise_density * pn_t_noise_density;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* PE initialization                                              */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 PE::InitApp()
{
    int32  iStatus   = CFE_SUCCESS;
    int8   hasEvents = 0;

    iStatus = InitEvent();
    if (iStatus != CFE_SUCCESS)
    {
        (void) CFE_ES_WriteToSysLog("PE - Failed to init events (0x%08lX)\n", iStatus);
        goto PE_InitApp_Exit_Tag;
    }
    else
    {
        hasEvents = 1;
    }

    iStatus = OS_MutSemCreate(&ConfigMutex, PE_PARAMS_MUTEX, 0);
    if (iStatus != CFE_SUCCESS)
    {
        (void) CFE_EVS_SendEvent(PE_MUTEX_ERR_EID, CFE_EVS_ERROR,
                                 "PE - mutex creation failed (0x%08lX)", iStatus);
        goto PE_InitApp_Exit_Tag;
    }

    iStatus = InitPipe();
    if (iStatus != CFE_SUCCESS)
    {
        goto PE_InitApp_Exit_Tag;
    }


    iStatus = InitConfigTbl();
    if (iStatus != CFE_SUCCESS)
    {
        goto PE_InitApp_Exit_Tag;
    }

    InitData();

PE_InitApp_Exit_Tag:
    if (iStatus == CFE_SUCCESS)
    {
        (void) CFE_EVS_SendEvent(PE_INIT_INF_EID, CFE_EVS_INFORMATION,
                                 "Initialized.  Version %d.%d.%d.%d",
								 PE_MAJOR_VERSION,
								 PE_MINOR_VERSION,
								 PE_REVISION,
								 PE_MISSION_REV);
    }
    else
    {
        if (hasEvents == 1)
        {
            (void) CFE_ES_WriteToSysLog("PE - Application failed to initialize\n");
        }
    }

    return iStatus;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Receive and Process Messages                                    */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int32 PE::RcvSchPipeMsg(int32 iBlocking)
{
    int32           iStatus=CFE_SUCCESS;
    uint64          baroTimestamp = 0;
    CFE_SB_Msg_t*   MsgPtr=NULL;
    CFE_SB_MsgId_t  MsgId;

    /* Stop Performance Log entry */
    CFE_ES_PerfLogExit(PE_MAIN_TASK_PERF_ID);

    /* Wait for WakeUp messages from scheduler */
    iStatus = CFE_SB_RcvMsg(&MsgPtr, SchPipeId, iBlocking);

    /* Start Performance Log entry */
    CFE_ES_PerfLogEntry(PE_MAIN_TASK_PERF_ID);

    if (iStatus == CFE_SUCCESS)
    {
        MsgId = CFE_SB_GetMsgId(MsgPtr);
        switch (MsgId)
        {
        	/* Cyclic op - 250hz */
            case PE_WAKEUP_MID:
                Update();
                break;

            case PE_SEND_HK_MID:
                ProcessCmdPipe();
                ReportHousekeeping();
                break;

            case PX4_VEHICLE_GPS_POSITION_MID:
                memcpy(&m_VehicleGpsPositionMsg, MsgPtr, sizeof(m_VehicleGpsPositionMsg));
                if(m_GpsTimeout)
                {
                	gpsInit();
                }
                else
                {
                	gpsCorrect();
                }

                break;

            case PX4_VEHICLE_STATUS_MID:
                memcpy(&m_VehicleStatusMsg, MsgPtr, sizeof(m_VehicleStatusMsg));
                break;

            case PX4_VEHICLE_LAND_DETECTED_MID:
                memcpy(&m_VehicleLandDetectedMsg, MsgPtr, sizeof(m_VehicleLandDetectedMsg));
                if(m_LandTimeout)
                {
                	landInit();
                }
                else
                {
                	landCorrect();
                }

                break;

            case PX4_ACTUATOR_ARMED_MID:
                memcpy(&m_ActuatorArmedMsg, MsgPtr, sizeof(m_ActuatorArmedMsg));
                break;

            case PX4_VEHICLE_ATTITUDE_MID:
                memcpy(&m_VehicleAttitudeMsg, MsgPtr, sizeof(m_VehicleAttitudeMsg));
                break;

            case PX4_VEHICLE_CONTROL_MODE_MID:
                memcpy(&m_VehicleControlModeMsg, MsgPtr, sizeof(m_VehicleControlModeMsg));
                break;

            case PX4_SENSOR_COMBINED_MID:
                memcpy(&m_SensorCombinedMsg, MsgPtr, sizeof(m_SensorCombinedMsg));
                /* If baro is valid */
                if(m_SensorCombinedMsg.BaroTimestampRelative != PX4_RELATIVE_TIMESTAMP_INVALID)
                {
                    /* Get the baro timestamp from the sensor combined timestamp
                     * (which is the gyro timestamp) plus the baro relative timestamp.
                     * Baro relative is the difference between the gyro and baro 
                     * when received by the sensors application. If baro is fresh.
                     */
                    if((m_SensorCombinedMsg.Timestamp + 
                       m_SensorCombinedMsg.BaroTimestampRelative) 
                       != m_TimeLastBaro)
                    {
                        if(m_BaroTimeout)
                        {
                            baroInit();
                        }
                        else
                        {
                            baroCorrect();
                        }
                        /* Save the last valid timestamp */
                        m_TimeLastBaro = m_SensorCombinedMsg.Timestamp + m_SensorCombinedMsg.BaroTimestampRelative;
                    }
                }
                break;

            case PX4_VEHICLE_ATTITUDE_SETPOINT_MID:
                memcpy(&m_VehicleAttitudeSetpointMsg, MsgPtr, sizeof(m_VehicleAttitudeSetpointMsg));
                break;

            case PX4_MANUAL_CONTROL_SETPOINT_MID:// TODO: verify needed
                memcpy(&m_ManualControlSetpointMsg, MsgPtr, sizeof(m_ManualControlSetpointMsg));
                break;

            default:
                (void) CFE_EVS_SendEvent(PE_MSGID_ERR_EID, CFE_EVS_ERROR,
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
        (void) CFE_EVS_SendEvent(PE_RCVMSG_ERR_EID, CFE_EVS_ERROR,
			  "SCH pipe read error (0x%08lX).", iStatus);
    }

    return iStatus;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Process Incoming Commands                                       */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void PE::ProcessCmdPipe()
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
                case PE_CMD_MID:
                    ProcessAppCmds(CmdMsgPtr);
                    break;

                default:
                    /* Bump the command error counter for an unknown command.
                     * (This should only occur if it was subscribed to with this
                     *  pipe, but not handled in this switch-case.) */
                    HkTlm.usCmdErrCnt++;
                    (void) CFE_EVS_SendEvent(PE_MSGID_ERR_EID, CFE_EVS_ERROR,
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
            (void) CFE_EVS_SendEvent(PE_RCVMSG_ERR_EID, CFE_EVS_ERROR,
                  "CMD pipe read error (0x%08lX)", iStatus);
            break;
        }
    }
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Process PE Commands                                            */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void PE::ProcessAppCmds(CFE_SB_Msg_t* MsgPtr)
{
    uint32  uiCmdCode=0;

    if (MsgPtr != NULL)
    {
        uiCmdCode = CFE_SB_GetCmdCode(MsgPtr);
        switch (uiCmdCode)
        {
            case PE_NOOP_CC:
                HkTlm.usCmdCnt++;
                (void) CFE_EVS_SendEvent(PE_CMD_NOOP_EID, CFE_EVS_INFORMATION,
					"Recvd NOOP. Version %d.%d.%d.%d",
					PE_MAJOR_VERSION,
					PE_MINOR_VERSION,
					PE_REVISION,
					PE_MISSION_REV);
                break;

            case PE_RESET_CC:
                HkTlm.usCmdCnt = 0;
                HkTlm.usCmdErrCnt = 0;
                break;

            default:
                HkTlm.usCmdErrCnt++;
                (void) CFE_EVS_SendEvent(PE_CC_ERR_EID, CFE_EVS_ERROR,
                                  "Recvd invalid command code (%u)", (unsigned int)uiCmdCode);
                break;
        }
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Send PE Housekeeping                                           */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void PE::ReportHousekeeping()
{
	HkTlm.EstimatorInitialized = m_EstimatorInitialized;
	HkTlm.XyEstValid = m_XyEstValid;
	HkTlm.ZEstValid = m_ZEstValid;
	HkTlm.TzEstValid = m_TzEstValid;
	HkTlm.Timestamp = m_Timestamp;
	HkTlm.TimeLastBaro = m_TimeLastBaro;
	HkTlm.TimeLastGps = m_TimeLastGps;
	HkTlm.TimeLastLand = m_TimeLastLand;
	HkTlm.BaroTimeout = m_BaroTimeout;
	HkTlm.GpsTimeout = m_GpsTimeout;
	HkTlm.LandTimeout = m_LandTimeout;
	HkTlm.BaroFault = m_BaroFault;
	HkTlm.GpsFault = m_GpsFault;
	HkTlm.LandFault = m_LandFault;
	HkTlm.AltOrigin = m_AltOrigin;
	HkTlm.AltOriginInitialized = m_AltOriginInitialized;
	HkTlm.BaroAltOrigin = m_BaroAltOrigin;
	HkTlm.GpsAltOrigin = m_GpsAltOrigin;

    CFE_SB_TimeStampMsg((CFE_SB_Msg_t*)&HkTlm);
    CFE_SB_SendMsg((CFE_SB_Msg_t*)&HkTlm);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Publish Output Data                                             */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void PE::SendVehicleLocalPositionMsg()
{
	float alt 			 = 0;
	float vxy_stddev     = 0;
	float epv 			 = 0;
	float eph			 = 0;
	float eph_thresh	 = 3.0f;
	float epv_thresh     = 3.0f;
	bool data_valid = false;

	vxy_stddev = sqrtf(m_StateCov[X_vx][X_vx] + m_StateCov[X_vy][X_vy]);
	epv = sqrt(m_StateCov[X_z][X_z]);
	eph = sqrt(m_StateCov[X_x][X_x] + m_StateCov[X_y][X_y]);

	if (vxy_stddev < m_Params.VXY_PUB_THRESH)
	{
		if (eph > eph_thresh)
		{
			eph = eph_thresh;
		}

		if (epv > epv_thresh)
		{
			epv = epv_thresh;
		}
	}

	if(	isfinite(m_XLowPass[X_vx]) &&
		isfinite(m_XLowPass[X_vy]) &&
		isfinite(m_XLowPass[X_vz]) &&
		isfinite(m_XLowPass[X_x]) &&
		isfinite(m_XLowPass[X_y]) &&
		isfinite(m_XLowPass[X_z]))
	{
		data_valid = true;
	}

	if(data_valid)
	{
		m_VehicleLocalPositionMsg.Timestamp = m_Timestamp;
		m_VehicleLocalPositionMsg.XY_Valid = m_XyEstValid;
		m_VehicleLocalPositionMsg.Z_Valid = m_ZEstValid;
		m_VehicleLocalPositionMsg.V_XY_Valid = m_XyEstValid;
		m_VehicleLocalPositionMsg.V_Z_Valid = m_ZEstValid;
		m_VehicleLocalPositionMsg.X = m_XLowPass[X_x];
		m_VehicleLocalPositionMsg.Y = m_XLowPass[X_y];
		m_VehicleLocalPositionMsg.Z = m_XLowPass[X_z]; // todo check if this should be agl
		m_VehicleLocalPositionMsg.VX = m_XLowPass[X_vx];
		m_VehicleLocalPositionMsg.VY = m_XLowPass[X_vy];
		m_VehicleLocalPositionMsg.VZ = m_XLowPass[X_vz];
		m_VehicleLocalPositionMsg.Yaw = m_Euler[2];
		m_VehicleLocalPositionMsg.XY_Global = m_XyEstValid;
		m_VehicleLocalPositionMsg.Z_Global = !m_BaroTimeout;
		m_VehicleLocalPositionMsg.RefTimestamp = m_Timestamp;
		m_VehicleLocalPositionMsg.RefLat = m_MapRef.lat_rad * 180/M_PI;
		m_VehicleLocalPositionMsg.RefLon = m_MapRef.lon_rad * 180/M_PI;
		m_VehicleLocalPositionMsg.RefAlt = m_AltOrigin;
		//m_VehicleLocalPositionMsg.DistBottom = ; //TODO agl low pass
		m_VehicleLocalPositionMsg.DistBottomRate = m_XLowPass[X_vz];
		m_VehicleLocalPositionMsg.SurfaceBottomTimestamp = m_Timestamp;
		m_VehicleLocalPositionMsg.DistBottomValid = m_ZEstValid;
		m_VehicleLocalPositionMsg.EpH = eph;
		m_VehicleLocalPositionMsg.EpV = epv;

		CFE_SB_TimeStampMsg((CFE_SB_Msg_t*)&m_VehicleLocalPositionMsg);
		CFE_SB_SendMsg((CFE_SB_Msg_t*)&m_VehicleLocalPositionMsg);
	}
	else
	{
		(void) CFE_EVS_SendEvent(PE_LOCAL_POS_MSG_ERR_EID, CFE_EVS_ERROR,
								 "Local position message data invalid");
	}
}

void PE::SendVehicleGlobalPositionMsg()
{
	double lat			 = 0;
	double lon 		  	 = 0;
	float alt 			 = 0;
	float vxy_stddev     = 0;
	float epv 			 = 0;
	float eph			 = 0;
	float eph_thresh	 = 3.0f;
	float epv_thresh     = 3.0f;
	bool data_valid = false;

	map_projection_reproject(&m_MapRef, m_XLowPass[X_x], m_XLowPass[X_y], &lat, &lon);

	alt = -m_XLowPass[X_z] + m_AltOrigin;
	vxy_stddev = sqrtf(m_StateCov[X_vx][X_vx] + m_StateCov[X_vy][X_vy]);
	epv = sqrt(m_StateCov[X_z][X_z]);
	eph = sqrt(m_StateCov[X_x][X_x] + m_StateCov[X_y][X_y]);

	if (vxy_stddev < m_Params.VXY_PUB_THRESH)
	{
		if (eph > eph_thresh)
		{
			eph = eph_thresh;
		}

		if (epv > epv_thresh)
		{
			epv = epv_thresh;
		}
	}

	if( isfinite(lat) &&
		isfinite(lon) &&
		isfinite(alt) &&
		isfinite(m_XLowPass[X_vx]) &&
		isfinite(m_XLowPass[X_vy]) &&
		isfinite(m_XLowPass[X_vz]))
	{
		data_valid = true;
	}

	if(data_valid)
	{
		m_VehicleGlobalPositionMsg.Timestamp = m_Timestamp;
		m_VehicleGlobalPositionMsg.TimeUtcUsec = m_VehicleGpsPositionMsg.TimeUtcUsec;
		m_VehicleGlobalPositionMsg.Lat = lat;
		m_VehicleGlobalPositionMsg.Lon = lon;
		m_VehicleGlobalPositionMsg.Alt = alt;
		m_VehicleGlobalPositionMsg.VelN = m_XLowPass[X_vx];
		m_VehicleGlobalPositionMsg.VelE = m_XLowPass[X_vy];
		m_VehicleGlobalPositionMsg.VelD = m_XLowPass[X_vz];
		m_VehicleGlobalPositionMsg.Yaw = m_Euler[2];
		m_VehicleGlobalPositionMsg.EpH = eph;
		m_VehicleGlobalPositionMsg.EpV = epv;
		m_VehicleGlobalPositionMsg.TerrainAlt = m_AltOrigin - m_XLowPass[X_tz];
		m_VehicleGlobalPositionMsg.TerrainAltValid = m_TzEstValid;
		m_VehicleGlobalPositionMsg.DeadReckoning = !m_XyEstValid;
		m_VehicleGlobalPositionMsg.PressureAlt = m_SensorCombinedMsg.BaroAlt;

		CFE_SB_TimeStampMsg((CFE_SB_Msg_t*)&m_VehicleGlobalPositionMsg);
		CFE_SB_SendMsg((CFE_SB_Msg_t*)&m_VehicleGlobalPositionMsg);
	}
	else
	{
		(void) CFE_EVS_SendEvent(PE_GLOBAL_POS_MSG_ERR_EID, CFE_EVS_ERROR,
								 "Global position message data invalid");
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Verify Command Length                                           */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
boolean PE::VerifyCmdLength(CFE_SB_Msg_t* MsgPtr,
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

            (void) CFE_EVS_SendEvent(PE_MSGLEN_ERR_EID, CFE_EVS_ERROR,
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
/* PE Application C style main entry point.                       */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
extern "C" void PE_AppMain()
{
    oPE.AppMain();
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* PE Application C++ style main entry point.                     */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void PE::AppMain()
{
    /* Register the application with Executive Services */
    uiRunStatus = CFE_ES_APP_RUN;

    int32 iStatus = CFE_ES_RegisterApp();
    if (iStatus != CFE_SUCCESS)
    {
        (void) CFE_ES_WriteToSysLog("PE - Failed to register the app (0x%08lX)\n", iStatus);
    }

    /* Start Performance Log entry */
    CFE_ES_PerfLogEntry(PE_MAIN_TASK_PERF_ID);

    /* Perform application initializations */
    if (iStatus == CFE_SUCCESS)
    {
        iStatus = InitApp();
    }

    if (iStatus == CFE_SUCCESS)
    {
        /* Do not perform performance monitoring on startup sync */
        CFE_ES_PerfLogExit(PE_MAIN_TASK_PERF_ID);
        CFE_ES_WaitForStartupSync(PE_STARTUP_TIMEOUT_MSEC);
        CFE_ES_PerfLogEntry(PE_MAIN_TASK_PERF_ID);
    }
    else
    {
        uiRunStatus = CFE_ES_APP_ERROR;
    }

    /* Application main loop */
    while (CFE_ES_RunLoop(&uiRunStatus) == TRUE)
    {
        RcvSchPipeMsg(PE_SCH_PIPE_PEND_TIME);

        iStatus = AcquireConfigPointers();
        if(iStatus != CFE_SUCCESS)
        {
            /* We apparently tried to load a new table but failed.  Terminate the application. */
            uiRunStatus = CFE_ES_APP_ERROR;
        }
    }

    /* Stop Performance Log entry */
    CFE_ES_PerfLogExit(PE_MAIN_TASK_PERF_ID);

    /* Exit the application */
    CFE_ES_ExitApp(uiRunStatus);
}


void PE::CheckTimeouts()
{
	baroCheckTimeout();
	gpsCheckTimeout();
	landCheckTimeout();
}


void PE::Update()
{
	/* Update timestamps */
	float dt = 0.0f;
	uint64 newTimestamp = PX4LIB_GetPX4TimeUs();
	dt = (newTimestamp - m_Timestamp) / 1.0e6f;
	m_Timestamp = newTimestamp;
	CheckTimeouts();

	/* Check if params are updated */
	if(m_ParamsUpdated)
	{
		updateStateSpaceParams();
		m_ParamsUpdated = false;
	}

	/* Check if xy is valid */
	bool VxyStdDevValid = false;
	if (fmax(m_StateCov[X_vx][X_vx], m_StateCov[X_vy][X_vy]) <
		m_Params.VXY_PUB_THRESH * m_Params.VXY_PUB_THRESH)
	{
		VxyStdDevValid = true;
	}

	/* Update current validity if needed */
	if(m_XyEstValid)
	{
		if(!VxyStdDevValid && m_GpsTimeout) // TODO: Should this really be AND?
		{
			m_XyEstValid = false;
		}
	}
	else
	{
		if(VxyStdDevValid && !m_GpsTimeout)
		{
			m_XyEstValid = true;
		}
	}

	/* Check if z is valid */
	bool ZStdDevValid = false;
	if(sqrtf(m_StateCov[X_z][X_z]) < m_Params.Z_PUB_THRESH)
	{
		ZStdDevValid = true;
	}

	/* Update current validity if needed */
	if(m_ZEstValid)
	{
		if(!ZStdDevValid && m_BaroTimeout) // TODO: Should this really be AND?
		{
			m_ZEstValid = false;
		}
	}
	else
	{
		if(ZStdDevValid && !m_BaroTimeout)
		{
			m_ZEstValid = true;
		}
	}

	/* Check if terrain is valid */
	bool TzStdDevValid = false;
	if(sqrtf(m_StateCov[X_tz][X_tz]) < m_Params.Z_PUB_THRESH)
	{
		TzStdDevValid = true;
	}

	/* Update current validity if needed */
	if(m_TzEstValid)
	{
		if(!TzStdDevValid)
		{
			m_TzEstValid = false;
		}
	}
	else
	{
		if(TzStdDevValid)
		{
			m_TzEstValid = true;
		}
	}

	/* Initialize map projection to INIT_ORIGIN_LAT, INIT_ORIGIN_LON if we don't
	 * have lat, lon data and allowing fake origin */
	if (!m_MapRef.init_done && m_XyEstValid && m_Params.FAKE_ORIGIN)
	{
		map_projection_init(&m_MapRef,
							m_Params.INIT_ORIGIN_LAT,
							m_Params.INIT_ORIGIN_LON,
							m_Timestamp); //TODO: Verify correct time

		(void) CFE_EVS_SendEvent(PE_GPS_OK_INF_EID, CFE_EVS_INFORMATION,
								 "GPS fake origin init. Lat: %6.2f Lon: %6.2f Alt: %5.1f m",
								 m_Params.INIT_ORIGIN_LAT,
								 m_Params.INIT_ORIGIN_LON,
								 double(m_AltOrigin));
	}

	/* Check if state vector needs to be reinitialized  */
	bool ReinitStateVec = false;
	for (int i = 0; i < n_x; i++)
	{
		if (!isfinite(m_StateVec[i]))
		{
			ReinitStateVec = true;
			(void) CFE_EVS_SendEvent(PE_ESTIMATOR_ERR_EID, CFE_EVS_INFORMATION,
									 "Reinitializing state vector. Index (%i) not finite", i);
			break;
		}
	}

	if (ReinitStateVec)
	{
		m_StateVec.Zero();
		/* TODO: Decide if we want to reinit sensors here. PX4 didn't */
	}

	/* Force state covariance symmetry and reinitialize matrix if necessary */
	bool ReinitStateCov = false;
	for (int i = 0; i < n_x; i++)
	{
		for (int j = 0; j <= i; j++)
		{
			if (!isfinite(m_StateCov[i][j]))
			{
				if(Initialized())
				{
					(void) CFE_EVS_SendEvent(PE_ESTIMATOR_ERR_EID, CFE_EVS_ERROR,
										 "Reinitializing state covariance. Index (%i, %i) not finite", i, j);
				}
				ReinitStateCov = true;
				break;
			}
			if (i == j)
			{
				/* Ensure diagonal elements are positive */
				if (m_StateCov[i][i] <= 0)
				{
					if(Initialized())
					{
						(void) CFE_EVS_SendEvent(PE_ESTIMATOR_ERR_EID, CFE_EVS_ERROR,
												 "Reinitializing state covariance. Index (%i, %i) negative", i, j);
					}
					ReinitStateCov = true;
					break;
				}
			}
			else
			{
				/* Copy element from upper triangle to force symmetry */
				m_StateCov[j][i] = m_StateCov[i][j];
			}
		}
	}

	if (ReinitStateCov)
	{
		initStateCov();
	}

	Predict(dt);

	/* Publish updated data if initialized */
	if(m_AltOriginInitialized)
	{
		if(!m_EstimatorInitialized)
		{
			m_EstimatorInitialized = true;
			(void) CFE_EVS_SendEvent(PE_ESTIMATOR_INF_EID, CFE_EVS_INFORMATION,
									 "Estimator initialized");
		}

		SendVehicleLocalPositionMsg();

		if(m_XyEstValid && (m_MapRef.init_done || m_Params.FAKE_ORIGIN))
		{
			SendVehicleGlobalPositionMsg();
		}
		else
		{
			//OS_printf("XyEst invalid\n");
		}
	}

    /* Propagate delayed state, no matter what if state is frozen, 
     * delayed state still needs to be propagated with frozen state.
     */
    float dt_hist = 1.0e-6f * (m_Timestamp - m_Timestamp_Hist);

    if (m_Timestamp_Hist == 0 || (dt_hist > HIST_STEP)) 
    {
        m_TDelay.Update(m_Timestamp);
        m_XDelay.Update(m_StateVec);
        m_Timestamp_Hist = m_Timestamp;
    }
}

bool PE::Initialized(void)
{
	return m_EstimatorInitialized && m_BaroInitialized && m_GpsInitialized && m_LandInitialized;
}


void PE::Predict(float dt)
{
	/* Get acceleration */
	math::Quaternion q(m_VehicleAttitudeMsg.Q[0],
					   m_VehicleAttitudeMsg.Q[1],
					   m_VehicleAttitudeMsg.Q[2],
					   m_VehicleAttitudeMsg.Q[3]);

	m_RotationMat = math::Dcm(q);
	m_Euler = math::Euler(m_RotationMat);
	math::Vector3F a(m_SensorCombinedMsg.Acc[0], 
                     m_SensorCombinedMsg.Acc[1], 
                     m_SensorCombinedMsg.Acc[2]);

	/* Note: bias is removed in dynamics function */
	m_InputVec = m_RotationMat * a;
	m_InputVec[U_az] += 9.81f; // add g

	/* Update state space based on new states */
	updateStateSpace();

	/* Continuous time Kalman filter prediction. Integrate runge kutta 4th order.
	 * https://en.wikipedia.org/wiki/Runge%E2%80%93Kutta_methods */
	math::Vector10F k1, k2, k3, k4;
	k1 = dynamics(m_StateVec, m_InputVec);
	k2 = dynamics(m_StateVec + k1 * dt / 2, m_InputVec);
	k3 = dynamics(m_StateVec + k2 * dt / 2, m_InputVec);
	k4 = dynamics(m_StateVec + k3 * dt, m_InputVec);
	math::Vector10F dx = (k1 + k2 * 2 + k3 * 2 + k4) * (dt / 6);

	/* Don't integrate position if no valid xy data */
	if (!m_XyEstValid)
	{
		dx[X_x]  = 0;
		dx[X_vx] = 0;
		dx[X_y]  = 0;
		dx[X_vy] = 0;
	}

	/* Don't integrate z if no valid z data */
	if (!m_ZEstValid)
	{
		dx[X_z] = 0;
	}

	/* Don't integrate tz if no valid tz data */
	if (!m_TzEstValid)
	{
		dx[X_tz] = 0;
	}

	/* Saturate bias */
	float bx = dx[X_bx] + m_StateVec[X_bx];
	float by = dx[X_by] + m_StateVec[X_by];
	float bz = dx[X_bz] + m_StateVec[X_bz];

	if (::abs(bx) > BIAS_MAX)
	{
		bx = BIAS_MAX * bx / ::abs(bx);
		dx[X_bx] = bx - m_StateVec[X_bx];
	}

	if (::abs(by) > BIAS_MAX)
	{
		by = BIAS_MAX * by / ::abs(by);
		dx[X_by] = by - m_StateVec[X_by];
	}

	if (::abs(bz) > BIAS_MAX)
	{
		bz = BIAS_MAX * bz / ::abs(bz);
		dx[X_bz] = bz - m_StateVec[X_bz];
	}

	/* Propagate */
	m_StateVec += dx;
	math::Matrix10F10 dP = (m_DynamicsMat * m_StateCov + m_StateCov * 
            m_DynamicsMat.Transpose() + m_InputMat * m_InputCov * 
            m_InputMat.Transpose() + m_NoiseCov) * dt;

	/* Covariance propagation logic */
	for (int i = 0; i < n_x; i++) 
    {
		if (m_StateCov[i][i] > P_MAX)
		{
			/* If diagonal element greater than max stop propagating */
			dP[i][i] = 0;

			for (int j = 0; j < n_x; j++)
			{
				dP[i][j] = 0;
				dP[j][i] = 0;
			}
		}
	}

	/* Update state */
	m_StateCov += dP;
	m_XLowPass.Update(m_StateVec, dt, LOW_PASS_CUTOFF);
}


math::Vector10F PE::dynamics(const math::Vector10F &x, const math::Vector3F &u)
{
	return (m_DynamicsMat * x) + (m_InputMat * u);
}


void PE::UpdateLocalParams()
{
	/* Lock the mutex */
	OS_MutSemTake(ConfigMutex);

	/* Update all locally stored params with the up to date value in the table */
	m_Params.FUSION             = ConfigTblPtr->FUSION;
	m_Params.VXY_PUB_THRESH     = ConfigTblPtr->VXY_PUB_THRESH;
	m_Params.Z_PUB_THRESH       = ConfigTblPtr->Z_PUB_THRESH;
	m_Params.ACCEL_XY_STDDEV    = ConfigTblPtr->ACCEL_XY_STDDEV;
	m_Params.ACCEL_Z_STDDEV     = ConfigTblPtr->ACCEL_Z_STDDEV;
	m_Params.BARO_STDDEV        = ConfigTblPtr->BARO_STDDEV;
	m_Params.GPS_DELAY          = ConfigTblPtr->GPS_DELAY;
	m_Params.GPS_XY_STDDEV      = ConfigTblPtr->GPS_XY_STDDEV;
	m_Params.GPS_Z_STDDEV       = ConfigTblPtr->GPS_Z_STDDEV;
	m_Params.GPS_VXY_STDDEV     = ConfigTblPtr->GPS_VXY_STDDEV;
	m_Params.GPS_VZ_STDDEV      = ConfigTblPtr->GPS_VZ_STDDEV;
	m_Params.GPS_EPH_MAX        = ConfigTblPtr->GPS_EPH_MAX;
	m_Params.GPS_EPV_MAX        = ConfigTblPtr->GPS_EPV_MAX;
	m_Params.LAND_Z_STDDEV      = ConfigTblPtr->LAND_Z_STDDEV;
	m_Params.LAND_VXY_STDDEV    = ConfigTblPtr->LAND_VXY_STDDEV;
	m_Params.PN_P_NOISE_DENSITY = ConfigTblPtr->PN_P_NOISE_DENSITY;
	m_Params.PN_V_NOISE_DENSITY = ConfigTblPtr->PN_V_NOISE_DENSITY;
	m_Params.PN_B_NOISE_DENSITY = ConfigTblPtr->PN_B_NOISE_DENSITY;
	m_Params.PN_T_NOISE_DENSITY = ConfigTblPtr->PN_T_NOISE_DENSITY;
	m_Params.T_MAX_GRADE        = ConfigTblPtr->T_MAX_GRADE;
	m_Params.FAKE_ORIGIN        = ConfigTblPtr->FAKE_ORIGIN;
	m_Params.INIT_ORIGIN_LAT    = ConfigTblPtr->INIT_ORIGIN_LAT;
	m_Params.INIT_ORIGIN_LON    = ConfigTblPtr->INIT_ORIGIN_LON;

	/* Unlock the mutex */
	OS_MutSemGive(ConfigMutex);
}


int PE::getDelayPeriods(float delay, uint8 *periods)
{
    float t_delay = 0;
    uint8 i_hist = 0;

    for(i_hist = 1; i_hist < HIST_LEN; i_hist++)
    {
        t_delay = 1.0e-6f * (m_Timestamp - m_TDelay.Get(i_hist));
        if(t_delay > delay)
        {
            break;
        }
    }

    *periods = i_hist;

    if(t_delay > DELAY_MAX)
    {
    	if(Initialized())
		{
			(void) CFE_EVS_SendEvent(PE_ESTIMATOR_ERR_EID, CFE_EVS_INFORMATION,
					"Delayed data old: %8.4f", t_delay);
		}
        return -1;
    }
    
    return CFE_SUCCESS;
}




/************************/
/*  End of File Comment */
/************************/
