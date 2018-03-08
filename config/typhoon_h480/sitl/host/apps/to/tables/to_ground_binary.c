/************************************************************************
** Pragmas
*************************************************************************/

/************************************************************************
** Includes
*************************************************************************/
#include "cfe_tbl_filedef.h"
#include "to_tbldefs.h"
#include "msg_ids.h"

/************************************************************************
** Local Defines
*************************************************************************/

/************************************************************************
** Local Structure Definitions
*************************************************************************/

/************************************************************************
** External Global Variables
*************************************************************************/

/************************************************************************
** Global Variables
*************************************************************************/

#define TO_PQUEUE_SINGLE_PASS_IDX		        0
#define TO_PQUEUE_HIGH_OPS_RSRVD_IDX	        1
#define TO_PQUEUE_HIGH_IDX                      2
#define TO_PQUEUE_MEDIUM_IDX                    3
#define TO_PQUEUE_LOW_IDX                       4

/**
**  \brief Default TO config table data
*/
TO_ChannelTbl_t TO_ConfigTbl =
{
	/* Table ID */
	1,
	TO_OUTPUT_TYPE_BINARY,
	{
		/* Message Flows */
		/* Ground Queues */
	    {AMC_HK_TLM_MID,			      1,	TO_PQUEUE_HIGH_IDX},
	    {MAC_HK_TLM_MID,			      1,	TO_PQUEUE_HIGH_IDX},
	    {MPC_HK_TLM_MID,			      1,	TO_PQUEUE_HIGH_IDX},
	    {ULR_HK_TLM_MID,			      1,	TO_PQUEUE_HIGH_IDX},
	    {RGBLED_HK_TLM_MID,			      1,	TO_PQUEUE_HIGH_IDX},
	    {GPS_HK_TLM_MID,			      1,	TO_PQUEUE_HIGH_IDX},
	    {SENS_HK_TLM_MID,			      1,	TO_PQUEUE_HIGH_IDX},
	    {LD_HK_TLM_MID,			      1,	TO_PQUEUE_HIGH_IDX},
	    {NAV_HK_TLM_MID,			      1,	TO_PQUEUE_HIGH_IDX},
	    {RCIN_HK_TLM_MID,			      1,	TO_PQUEUE_HIGH_IDX},
	    {VM_HK_TLM_MID,			      1,	TO_PQUEUE_HIGH_IDX},
	    {BAT_HK_TLM_MID,			      1,	TO_PQUEUE_HIGH_IDX},
	    {PE_HK_TLM_MID,			      1,	TO_PQUEUE_HIGH_IDX},
	    {HMC5883_HK_TLM_MID,		      1,	TO_PQUEUE_HIGH_IDX},
	    {HMC5883_DIAG_TLM_MID,		      1,	TO_PQUEUE_HIGH_IDX},
        {MS5611_HK_TLM_MID,			      1,	TO_PQUEUE_HIGH_IDX},
        {MS5611_DIAG_TLM_MID,                     1,        TO_PQUEUE_HIGH_IDX},
        {MPU9250_HK_TLM_MID,			      1,	TO_PQUEUE_HIGH_IDX},
        {MPU9250_DIAG_TLM_MID,                     1,        TO_PQUEUE_HIGH_IDX},
        {TO_HK_TLM_MID,			      1,	TO_PQUEUE_HIGH_IDX},
        {TO_DATA_TYPE_MID,			      1,	TO_PQUEUE_HIGH_IDX},
        {EA_HK_TLM_MID,			      1,	TO_PQUEUE_HIGH_IDX},
        {VC_HK_TLM_MID,			      1,	TO_PQUEUE_HIGH_IDX},
        {TO_DIAG_TLM_MID,			      1,	TO_PQUEUE_HIGH_IDX},
        {GPS_HK_TLM_MID,			      1,        TO_PQUEUE_HIGH_IDX},
        {MAVLINK_HK_TLM_MID,                      1,        TO_PQUEUE_HIGH_IDX},
        {CFE_ES_HK_TLM_MID,			      1,	TO_PQUEUE_MEDIUM_IDX},
        {CFE_EVS_HK_TLM_MID,		      1,	TO_PQUEUE_MEDIUM_IDX},
	    {CFE_SB_HK_TLM_MID,			      1,	TO_PQUEUE_MEDIUM_IDX},
	    {CFE_TBL_HK_TLM_MID,		      1,	TO_PQUEUE_MEDIUM_IDX},
	    {CFE_TIME_HK_TLM_MID,		      1,	TO_PQUEUE_MEDIUM_IDX},
	    {CFE_TIME_DIAG_TLM_MID,		      1,	TO_PQUEUE_MEDIUM_IDX},
	    {CFE_EVS_EVENT_MSG_MID,		      32,       TO_PQUEUE_MEDIUM_IDX},
	    {CFE_SB_STATS_TLM_MID,		      1,	TO_PQUEUE_MEDIUM_IDX},
	    {CFE_ES_APP_TLM_MID,		      1,	TO_PQUEUE_MEDIUM_IDX},
	    {CFE_TBL_REG_TLM_MID,		      1,	TO_PQUEUE_MEDIUM_IDX},
	    {CFE_SB_ONESUB_TLM_MID,		      1,	TO_PQUEUE_MEDIUM_IDX},
	    {CFE_ES_SHELL_TLM_MID,		      32,       TO_PQUEUE_MEDIUM_IDX},
	    {CFE_ES_MEMSTATS_TLM_MID,	              1,	TO_PQUEUE_MEDIUM_IDX},
	    {CF_HK_TLM_MID,			      1,	TO_PQUEUE_MEDIUM_IDX},
	    {CF_TRANS_TLM_MID,			      1,	TO_PQUEUE_MEDIUM_IDX},
	    {CF_CONFIG_TLM_MID,			      1,	TO_PQUEUE_MEDIUM_IDX},
	    {CF_SPACE_TO_GND_PDU_MID,	             32,	TO_PQUEUE_MEDIUM_IDX},
	    {CS_HK_TLM_MID,			      1,	TO_PQUEUE_MEDIUM_IDX},
	    {DS_HK_TLM_MID,			      1,	TO_PQUEUE_MEDIUM_IDX},
	    {DS_DIAG_TLM_MID,			      1,	TO_PQUEUE_MEDIUM_IDX},
	    {FM_HK_TLM_MID,			      1,	TO_PQUEUE_MEDIUM_IDX},
	    {FM_FILE_INFO_TLM_MID,		      1,	TO_PQUEUE_MEDIUM_IDX},
	    {FM_DIR_LIST_TLM_MID,		      1,	TO_PQUEUE_MEDIUM_IDX},
	    {FM_OPEN_FILES_TLM_MID,		      1,	TO_PQUEUE_MEDIUM_IDX},
	    {FM_FREE_SPACE_TLM_MID,		      1,	TO_PQUEUE_MEDIUM_IDX},
	    {HK_HK_TLM_MID,			      1,	TO_PQUEUE_MEDIUM_IDX},
	    {HS_HK_TLM_MID,			      1,	TO_PQUEUE_MEDIUM_IDX},
	    {LC_HK_TLM_MID,			      1,	TO_PQUEUE_MEDIUM_IDX},
	    {MD_HK_TLM_MID,			      1,	TO_PQUEUE_MEDIUM_IDX},
	    {MM_HK_TLM_MID,			      1,	TO_PQUEUE_MEDIUM_IDX},
	    {SCH_HK_TLM_MID,			      1,	TO_PQUEUE_MEDIUM_IDX},
	    {SCH_DIAG_TLM_MID,			      1,	TO_PQUEUE_MEDIUM_IDX},
	    {CI_HK_TLM_MID,			      1,	TO_PQUEUE_MEDIUM_IDX},
	    {AE_HK_TLM_MID,			      1,	TO_PQUEUE_MEDIUM_IDX},
	},{
		/* Priority Queues */
		/* TO_PQUEUE_SINGLE_PASS_IDX */
		{TO_PQUEUE_ENA, 100, TO_PRIORITY_QUEUE_TYPE_SINGLE},
		/* TO_PQUEUE_HIGH_OPS_RSRVD_IDX */
		{TO_PQUEUE_ENA, 100, TO_PRIORITY_QUEUE_TYPE_FIFO},
		/* TO_PQUEUE_HIGH_IDX */
		{TO_PQUEUE_ENA, 100, TO_PRIORITY_QUEUE_TYPE_FIFO},
		/* TO_PQUEUE_MEDIUM_IDX */
		{TO_PQUEUE_ENA, 100, TO_PRIORITY_QUEUE_TYPE_FIFO},
		/* TO_PQUEUE_LOW_IDX */
		{TO_PQUEUE_ENA, 100, TO_PRIORITY_QUEUE_TYPE_FIFO}
	}
};

/************************************************************************
** Local Variables
*************************************************************************/

/************************************************************************
** Local Function Prototypes
*************************************************************************/

/************************************************************************
** Function Definitions
*************************************************************************/


CFE_TBL_FILEDEF(TO_ConfigTbl, TO.GRND_BIN_CFG, TO ground config table, to_grnd_bin.tbl )

/************************/
/*  End of File Comment */
/************************/
