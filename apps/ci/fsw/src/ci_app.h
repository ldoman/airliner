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

#ifndef CI_APP_H
#define CI_APP_H

/************************************************************************
** Pragmas
*************************************************************************/

/************************************************************************
** Includes
*************************************************************************/
#include "cfe.h"
#include "pb_lib.h"

#include "ci_platform_cfg.h"
#include "ci_mission_cfg.h"
#include "ci_private_ids.h"
#include "ci_private_types.h"
#include "ci_perfids.h"
#include "ci_msgids.h"
#include "ci_msg.h"
#include "ci_events.h"
#include "ci_config_utils.h"
#include "ci_cds_utils.h"
#include "ci_tbldefs.h"
#include "ci_custom.h"

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************
** Local Defines
*************************************************************************/
#define CI_MAX_CMD_INGEST           		(CFE_SB_MAX_SB_MSG_SIZE)
#define CI_LISTENER_TASK_NAME  				"CI_LISTENER"
#define CI_LISTENER_TASK_STACK_SIZE			160000
#define CI_LISTENER_TASK_PRIORITY			109
#define CI_SERIAL_LISTENER_TASK_NAME  		"CI_SERIAL_LISTENER"
#define CI_SERIAL_LISTENER_TASK_STACK_SIZE	160000
#define CI_SERIAL_LISTENER_TASK_PRIORITY	112
#define CI_CFG_TBL_MUTEX_NAME 				"CI_CFG_TBL_MUTEX"
#define CI_TIME_TBL_MUTEX_NAME 				"CI_TIME_TBL_MUTEX"

/************************************************************************
** Local Structure Definitions
*************************************************************************/

/**
**  \brief CI Operational Data Structure
*/
typedef enum
{
	BHV_OPTIMISTIC,
	BHV_PESSIMISTIC,
} CI_BEHAVIOR;

/**
**  \brief CI Operational Data Structure
*/
typedef struct
{
    /** \brief CFE Event Table */
    CFE_EVS_BinFilter_t  EventTbl[CI_EVT_CNT];

    /**\brief Scheduling Pipe ID */
    CFE_SB_PipeId_t  SchPipeId;

    /** \brief Command Pipe ID */
    CFE_SB_PipeId_t  CmdPipeId;

    /** \brief Data Pipe ID */
    CFE_SB_PipeId_t  DataPipeId;

    /* Task-related */

    /** \brief Task Run Status */
    uint32  uiRunStatus;

    /* Config table-related */

    /** \brief Config Table Handle */
    CFE_TBL_Handle_t  ConfigTblHdl;

    /** \brief Config Table Pointer */
    CI_ConfigTblEntry_t*  ConfigTblPtr;

    /** \brief Timeout Table Handle */
    CFE_TBL_Handle_t  TimeoutTblHdl;

    /** \brief Timeout Table */
    CI_TimeoutTblEntry_t  TimeoutTbl;

    /* Critical Data Storage (CDS) table-related */

    /** \brief CDS Table Handle */
    CFE_ES_CDSHandle_t  CdsTblHdl;

    /** \brief CDS Table data */
    CI_CdsTbl_t  CdsTbl;

    /* Inputs/Outputs */

    /** \brief Input Data from I/O or other apps */
    CI_InData_t   InData;

    /** \brief Output Data published at the end of cycle */
    CI_OutData_t  OutData;

    /** \brief Housekeeping Telemetry for downlink */
    CI_HkTlm_t  HkTlm;

    /** \brief Mutex for CI config table */
	uint32          ConfigTblMutex;

	/** \brief Mutex for CI timeout table */
	uint32          TimeoutTblMutex;

    /** \brief ID of listener child task */
    uint32          ListenerTaskID;

    /** \brief ID of serialized listener child task */
	uint32          SerialListenerTaskID;

    /** \brief Buffer for child task cmd ingest */
    uint8           IngestBuffer[CI_MAX_CMD_INGEST];

    /** \brief Buffer for serialized child task cmd ingest */
    uint8           SerialIngestBuffer[CI_MAX_CMD_INGEST];

    /** \brief Run flag for ingest loop */
    boolean			IngestActive;

    /** \brief Behavior for unknown commands to CI */
    CI_BEHAVIOR		IngestBehavior;

} CI_AppData_t;

/************************************************************************
** External Global Variables
*************************************************************************/

/************************************************************************
** Global Variables
*************************************************************************/

/************************************************************************
** Local Variables
*************************************************************************/

/************************************************************************
** Local Function Prototypes
*************************************************************************/

/************************************************************************/
/** \brief CFS Command Ingest Task (CI) application entry point
**
**  \par Description
**       CFS Command Ingest Task application entry point.  This function
**       performs app initialization, then waits for the cFE ES Startup
**       Sync, then executes the RPR main processing loop.
**
**  \par Assumptions, External Events, and Notes:
**       If there is an unrecoverable failure during initialization the
**       main loop is never executed and the application will exit.
**
*************************************************************************/
void  CI_AppMain(void);

/************************************************************************/
/** \brief Initialize the CFS Command Ingest (CI) application
**
**  \par Description
**       Command Ingest application initialization routine. This
**       function performs all the required startup steps to
**       initialize (or restore from CDS) CI data structures and get
**       the application registered with the cFE services so it can
**       begin to receive command messages and send events.
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \returns
**  \retcode #CFE_SUCCESS  \retdesc \copydoc CFE_SUCCESS    \endcode
**  \retstmt Return codes from #CFE_ES_RegisterApp          \endcode
**  \retstmt Return codes from #CI_InitEvent               \endcode
**  \retstmt Return codes from #CI_InitPipe                \endcode
**  \retstmt Return codes from #CI_InitData                \endcode
**  \retstmt Return codes from #CI_InitTbls          	   \endcode
**  \retstmt Return codes from #CI_InitCdsTbl              \endcode
**  \retstmt Return codes from #OS_TaskInstallDeleteHandler \endcode
**  \endreturns
**
*************************************************************************/
int32  CI_InitApp(void);

/************************************************************************/
/** \brief Initialize Event Services and Event tables
**
**  \par Description
**       This function performs the steps required to setup
**       cFE Event Services for use by the CI application.
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \returns
**  \retcode #CFE_SUCCESS  \retdesc \copydoc CFE_SUCCESS \endcode
**  \retstmt Return codes from #CFE_EVS_Register  \endcode
**  \endreturns
**
*************************************************************************/
int32  CI_InitEvent(void);

/************************************************************************/
/** \brief Initialize global variables used by CI application
**
**  \par Description
**       This function performs the steps required to initialize
**       the CI application data.
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \returns
**  \retcode #CFE_SUCCESS  \retdesc \copydoc CFE_SUCCESS \endcode
**  \retstmt Return codes from #CFE_EVS_Register  \endcode
**  \endreturns
**
*************************************************************************/
int32  CI_InitData(void);

/************************************************************************/
/** \brief Initialize message pipes
**
**  \par Description
**       This function performs the steps required to setup
**       initialize the cFE Software Bus message pipes and subscribe to
**       messages for the CI application.
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \returns
**  \retcode #CFE_SUCCESS  \retdesc \copydoc CFE_SUCCESS \endcode
**  \retstmt Return codes from #CFE_SB_CreatePipe        \endcode
**  \retstmt Return codes from #CFE_SB_SubscribeEx       \endcode
**  \retstmt Return codes from #CFE_SB_Subscribe         \endcode
**  \endreturns
**
*************************************************************************/
int32  CI_InitPipe(void);

/************************************************************************/
/** \brief Command Ingest Task (CI) cleanup prior to exit
**
**  \par Description
**       This function handles any necessary cleanup prior
**       to application exit.
**
**  \par Assumptions, External Events, and Notes:
**       None
**
*************************************************************************/
void  CI_CleanupCallback(void);

/************************************************************************/
/** \brief Receive and process messages
**
**  \par Description
**       This function receives and processes messages
**       for the CI application
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \param [in]   iBlocking    A #CFE_SB_PEND_FOREVER, #CFE_SB_POLL or
**                             millisecond timeout
**
**  \returns
**  \retcode #CFE_SUCCESS  \retdesc \copydoc CFE_SUCCESS \endcode
**  \retstmt Return codes from #CFE_SB_RcvMsg            \endcode
**  \endreturns
**
*************************************************************************/
int32  CI_RcvMsg(int32 iBlocking);


/************************************************************************/
/** \brief Command Ingest Task incoming data processing
**
**  \par Description
**       This function processes incoming data subscribed
**       by CI application
**
**  \par Assumptions, External Events, and Notes:
**       None
**
*************************************************************************/
void  CI_ProcessNewData(void);

/************************************************************************/
/** \brief Command Ingest Task incoming command processing
**
**  \par Description
**       This function processes incoming commands subscribed
**       by CI application
**
**  \par Assumptions, External Events, and Notes:
**       None
**
*************************************************************************/
void  CI_ProcessNewCmds(void);

/************************************************************************/
/** \brief Command Ingest Task application commands
**
**  \par Description
**       This function processes command messages
**       specific to the CI application
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \param [in]   MsgPtr       A #CFE_SB_Msg_t pointer that
**                             references the software bus message
**
*************************************************************************/
void  CI_ProcessNewAppCmds(CFE_SB_Msg_t* MsgPtr);


/************************************************************************/
/** \brief Sends CI housekeeping message
**
**  \par Description
**       This function sends the housekeeping message
**
**  \par Assumptions, External Events, and Notes:
**       None
**
*************************************************************************/
void  CI_ReportHousekeeping(void);

/************************************************************************/
/** \brief Sends CI output data
**
**  \par Description
**       This function publishes the CI application output data.
**
**  \par Assumptions, External Events, and Notes:
**       None
**
*************************************************************************/
void  CI_SendOutData(void);

/************************************************************************/
/** \brief Verify Command Length
**
**  \par Description
**       This function verifies the command message length.
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \param [in]   MsgPtr        A #CFE_SB_Msg_t pointer that
**                              references the software bus message
**  \param [in]   usExpectedLen The expected length of the message
**
**  \returns
**  TRUE if the message length matches expectations, FALSE if it does not.
**  \endreturns
**
*************************************************************************/
boolean  CI_VerifyCmdLength(CFE_SB_Msg_t* MsgPtr, uint16 usExpectedLen);

/************************************************************************/
/** \brief Init Listener Task
**
**  \par Description
**       This function create a CFs child task for the command
**       ingest listener.
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \returns
**	\retcode #CFE_SUCCESS  \retdesc \copydoc CFE_SUCCESS \endcode
**  \retstmt Return codes from #CFE_ES_CreateChildTask            \endcode
**
*************************************************************************/
int32  CI_InitListenerTask(void);

/************************************************************************/
/** \brief Listener Task Main
**
**  \par Description
**       This function opens a socket and ingests all cmds for
**       CI to process before publishing them to the software bus.
**
**  \par Assumptions, External Events, and Notes:
**       None
**
*************************************************************************/
void CI_ListenerTaskMain(void);

/************************************************************************/
/** \brief Serialized Listener Task Main
**
**  \par Description
**       This function opens a socket and ingests all serialized cmds for
**       CI to process before publishing them to the software bus.
**
**  \par Assumptions, External Events, and Notes:
**       None
**
*************************************************************************/
void CI_SerializedListenerTaskMain(void);


/************************************************************************/
/** \brief Process Ingest Command
**
**  \par Description
**       This function contains the shared logic for processing a cmd
**       used by both the serialized and nonserialized ingest loops.
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \param [in]   CmdMsgPtr     A #CFE_SB_Msg_t pointer that
**                              references the software bus message
**  \param [in]   MsgSize 		The size of the message from the
**  							ingest buffer
**
*************************************************************************/
void CI_ProcessIngestCmd(CFE_SB_MsgPtr_t CmdMsgPtr, uint32 MsgSize);

/************************************************************************/
/** \brief Validate Command
**
**  \par Description
**       This function validates several parameters of the command.
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \param [in]   MsgPtr        A #CFE_SB_Msg_t pointer that
**                              references the software bus message
**  \param [in]   MsgSize 		The size of the message from the
**  							ingest buffer
**
**  \returns
**  TRUE if the command is valid, FALSE if it is not.
**  \endreturns
**
*************************************************************************/
boolean CI_ValidateCmd(CFE_SB_Msg_t* MsgPtr, uint32 MsgSize);

/************************************************************************/
/** \brief Validate Serialized Command
**
**  \par Description
**       This function validates several parameters of a serialized command.
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \param [in]   MsgPtr        A #CFE_SB_Msg_t pointer that
**                              references the software bus message
**
**  \returns
**  TRUE if the command is valid, FALSE if it is not.
**  \endreturns
**
*************************************************************************/
boolean CI_ValidateSerialCmd(CFE_SB_Msg_t* MsgPtr);


/************************************************************************/
/** \brief Get Command Authorization
**
**  \par Description
**       This function verifies if a command is authorized to execute
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \param [in]   MsgPtr        A #CFE_SB_Msg_t pointer that
**                              references the software bus message
**
**  \returns
**  TRUE if the command is authorized, FALSE if it is not.
**  \endreturns
**
*************************************************************************/
boolean CI_GetCmdAuthorized(CFE_SB_Msg_t* MsgPtr);

/************************************************************************/
/** \brief Log Command
**
**  \par Description
**       This function logs execution of the command
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \param [in]   MsgPtr        A #CFE_SB_Msg_t pointer that
**                              references the software bus message
**
*************************************************************************/
void CI_LogCmd(CFE_SB_Msg_t* MsgPtr);

/************************************************************************/
/** \brief Process Timeouts
**
**  \par Description
**       This function decrements all authorized commands
**       timeout counters and resets their state to unauthorized
**       when it hits zero.
**
**  \par Assumptions, External Events, and Notes:
**       None
**
*************************************************************************/
void CI_ProcessTimeouts(void);

/************************************************************************/
/** \brief Command Authorize
**
**  \par Description
**       This function authorizes a 2-step command.
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \param [in]   MsgPtr        A #CFE_SB_Msg_t pointer that
**                              references the software bus message
**
*************************************************************************/
void CI_CmdAuthorize(CFE_SB_Msg_t* MsgPtr);

/************************************************************************/
/** \brief Command Deauthorize
**
**  \par Description
**       This function deauthorizes a 2-step command.
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \param [in]   MsgPtr        A #CFE_SB_Msg_t pointer that
**                              references the software bus message
**
*************************************************************************/
void CI_CmdDeauthorize(CFE_SB_Msg_t* MsgPtr);

/************************************************************************/
/** \brief Get Registered Command
**
**  \par Description
**       This function searches for and returns a command
**       from the config table
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \param [in]   msgID        A #CFE_SB_MsgId_t that specifies the
**  						   message ID if of the command
**
**  \param [in]   cmdCode      A #uint16 that specifies the command code
**
**  \returns
**  #CI_CmdData_t if the command is registerd, NULL if it is not.
**  \endreturns
**
*************************************************************************/
CI_CmdData_t *CI_GetRegisterdCmd(CFE_SB_MsgId_t msgID, uint16 cmdCode);

/************************************************************************/
/** \brief Get Registered Command Index
**
**  \par Description
**       This function searches for a cmd and returns an index
**       of the config table containing that cmd
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \param [in]   msgID        A #CFE_SB_MsgId_t that specifies the
**  						   message ID if of the command
**
**  \param [in]   cmdCode      A #uint16 that specifies the command code
**
**  \returns
**  #uint32 if the command is registerd, -1 if it is not.
**  \endreturns
**
*************************************************************************/
uint32 CI_GetRegisterdCmdIdx(CFE_SB_MsgId_t msgID, uint16 cmdCode);

/************************************************************************/
/** \brief Register Command
**
**  \par Description
**       This function adds a command to the registration table.
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \param [in]   MsgPtr        A #CFE_SB_Msg_t pointer that
**                              references the software bus message
**
*************************************************************************/
void CI_CmdRegister(CFE_SB_Msg_t* MsgPtr);

/************************************************************************/
/** \brief Deregister Command
**
**  \par Description
**       This function removes a command from the registration table.
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \param [in]   MsgPtr        A #CFE_SB_Msg_t pointer that
**                              references the software bus message
**
*************************************************************************/
void CI_CmdDeregister(CFE_SB_Msg_t* MsgPtr);

/************************************************************************/
/** \brief Update Command Register
**
**  \par Description
**       This function updates a command in the registration table.
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \param [in]   MsgPtr        A #CFE_SB_Msg_t pointer that
**                              references the software bus message
**
*************************************************************************/
void CI_UpdateCmdReg(CFE_SB_Msg_t* MsgPtr);

/************************************************************************/
/** \brief Deserialize Message
**
**  \par Description
**       This function receives a SB message pointer that has a payload
**       serialized with protobuf and deserializes that payload to return
**       a valid message pointer CFS can understand.
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \param [in]   MsgPtr        A #CFE_SB_Msg_t pointer that
**                              references the software bus message
**
**  \returns
**  #uint32 The size of the new #CFE_SB_Msg_t
**  \endreturns
*************************************************************************/
uint32 CI_DeserializeMsg(CFE_SB_MsgPtr_t CmdMsgPtr);

#ifdef __cplusplus
}
#endif 

#endif /* CI_APP_H */

/************************/
/*  End of File Comment */
/************************/
