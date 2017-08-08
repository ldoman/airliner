#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "ea_custom.h"
#include "ea_platform_cfg.h"
#include "ea_app.h"
#include "ea_msg.h"

void EA_StartAppCustom()
{
	int32 Status = -1;
	Status = CFE_ES_RegisterChildTask();

	if (Status == CFE_SUCCESS)
	{
		/*
		** Create child process to execute app
		*/
		pid_t pid = fork();

		/*
		** Child process
		*/
		if (pid == 0)
		{
			char *argv[] = {EA_AppData.ChildData.AppInterpreter, EA_AppData.ChildData.AppScript, NULL};
			if(execvp(EA_AppData.ChildData.AppInterpreter, argv) == -1)
			{
				CFE_EVS_SendEvent(EA_CMD_ERR_EID, CFE_EVS_ERROR,
									"Error starting external application");
			}
			exit(0);
		}
		/*
		** Failed Fork
		*/
		else if (pid == -1)
		{
			EA_AppData.HkTlm.usCmdErrCnt++;
			CFE_EVS_SendEvent(EA_CMD_ERR_EID, CFE_EVS_ERROR,
								"Error starting new process");
		}
		/*
		** Parent process
		*/
		else
		{
			EA_AppData.HkTlm.usCmdCnt++;
			CFE_EVS_SendEvent(EA_INF_APP_START, CFE_EVS_INFORMATION,
								"External application started");
			strncpy(EA_AppData.HkTlm.ActiveApp, EA_AppData.ChildData.AppScript, OS_MAX_PATH_LEN);
			EA_AppData.HkTlm.ActiveAppPID = pid;
			waitpid(pid, &EA_AppData.HkTlm.LastAppStatus, 0);
			EA_AppData.HkTlm.LastAppStatus = EA_AppData.HkTlm.LastAppStatus>>8;
			EA_AppData.HkTlm.ActiveAppPID = 0;
			EA_AppData.HkTlm.ActiveAppUtil = 0;
			strncpy(EA_AppData.HkTlm.LastAppRun, EA_AppData.HkTlm.ActiveApp, OS_MAX_PATH_LEN);
			memset(EA_AppData.HkTlm.ActiveApp, '\0', OS_MAX_PATH_LEN);
			memset(EA_AppData.ChildData.AppInterpreter, '\0', OS_MAX_PATH_LEN);
			memset(EA_AppData.ChildData.AppScript, '\0', OS_MAX_PATH_LEN);

		}
	}/*end if register child task*/
    else
    {
        /* Can't send event or write to syslog because this task isn't registered with the cFE. */
        OS_printf("StartApp Child Task Registration failed!\n");
    }

	EA_AppData.ChildAppTaskID = 0;
	EA_AppData.ChildAppTaskInUse = FALSE;
	CFE_ES_ExitChildTask();

	return;
}

void EA_TermAppCustom(CFE_SB_Msg_t* MsgPtr)
{
	uint16 ExpectedLength = sizeof(EA_NoArgCmd_t);
	int kill_status = -2;

	/*
	** Verify command packet length
	*/
	if(EA_VerifyCmdLength(MsgPtr, ExpectedLength))
	{
		/*
		** Ensure app is currently running
		*/
		if(EA_AppData.HkTlm.ActiveAppPID != 0)
		{
			CFE_ES_DeleteChildTask(EA_AppData.ChildAppTaskID);
			kill_status = kill(EA_AppData.HkTlm.ActiveAppPID, SIGKILL);

			/*
			** Check kill call return code
			*/
			if(kill_status == 0)
			{
				EA_AppData.HkTlm.usCmdCnt++;
				EA_AppData.HkTlm.ActiveAppPID = 0;
				EA_AppData.HkTlm.ActiveAppUtil = 0;
				strncpy(EA_AppData.HkTlm.LastAppRun, EA_AppData.HkTlm.ActiveApp, OS_MAX_PATH_LEN);
				EA_AppData.HkTlm.LastAppStatus = -1; // TODO: Add meaningful number to this
				memset(EA_AppData.HkTlm.ActiveApp, '\0', OS_MAX_PATH_LEN);
				EA_AppData.ChildAppTaskInUse = FALSE;
			}
			else
			{
				EA_AppData.HkTlm.usCmdErrCnt++;
				CFE_EVS_SendEvent(EA_CMD_ERR_EID, CFE_EVS_ERROR,
								"Unable to terminate application. Errno %i", kill_status);
			}
		}
		else
		{
			EA_AppData.HkTlm.usCmdErrCnt++;
			CFE_EVS_SendEvent(EA_CMD_ERR_EID, CFE_EVS_ERROR,
							"Attempted to terminate app while none executing");
		}
	}

	return;
}

void EA_PerfmonCustom(int32 pid)
{
	EA_ProcData_t proc_data = EA_ParsePidUtil(pid);
	int dp = proc_data.p_time - EA_AppData.ProcData.p_time;
	int dt = proc_data.total_time - EA_AppData.ProcData.total_time;
	float util = 100 * EA_APP_NUM_THREADS * (float)dp/ (float)dt;
	EA_AppData.ProcData.p_time = proc_data.p_time;
	EA_AppData.ProcData.total_time = proc_data.total_time;
	EA_AppData.HkTlm.ActiveAppUtil = util;

	return;
}

EA_ProcData_t EA_ParsePidUtil(int32 pid)
{
	EA_ProcData_t procData;

	unsigned long long int utime = 0;
	unsigned long long int ntime = 0;
	unsigned long long int stime = 0;
	unsigned long long int itime = 0;
	unsigned long long int io_wait = 0;
	unsigned long long int irq = 0;
	unsigned long long int soft_irq = 0;
	unsigned long long int steal = 0;
	unsigned long long int guest = 0;
	unsigned long long int guest_nice = 0;
	unsigned long long int sys_time, idle_time, total_time, virt_time;

	FILE* proc_stat = fopen("/proc/stat", "r");
	if (proc_stat == NULL) {
		OS_printf("Unable to open stat");
		//return NULL;
	}
	char buf[1024];
	char* data = fgets(buf, sizeof(buf) - 1, proc_stat);
	if (data == NULL)
	{
		OS_printf("Unable to read stat");
	}
	fclose(proc_stat);
	sscanf(buf, "cpu  %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu",
			&utime, &ntime, &stime, &itime, &io_wait, &irq, &soft_irq, &steal, &guest, &guest_nice);
	utime = utime - guest;
	ntime = ntime - guest_nice;
	idle_time = itime + io_wait;
	sys_time = stime + irq + soft_irq;
	virt_time = guest + guest_nice;
	total_time = utime + ntime + sys_time + idle_time + steal + virt_time;
	procData.total_time = total_time;
	//OS_printf("Total time: %llu\n", total_time);

	char path[64];
	snprintf(path, sizeof(path), "/proc/%i/stat", pid);
	FILE* pid_stat = fopen(path, "r");
	if (pid_stat == NULL) {
		OS_printf("Unable to open pid_stat");
	}

	memset(buf, '\0', 1024);
	data = fgets(buf, sizeof(buf) - 1, pid_stat);
	if (data == NULL)
	{
		OS_printf("Unable to read pid_stat");
	}

	int utime_ln = 13;
	int stime_ln = 14;
	int count_ndx = 0;
	for (char *tok = strtok(data," "); tok != NULL; tok = strtok(NULL, " "))
	{
		if (count_ndx == utime_ln)
		{
			utime = atol(tok);
		}
		if (count_ndx == stime_ln)
		{
			stime = atol(tok);
		}
		count_ndx++;
	}
	fclose(pid_stat);

	procData.p_time = utime + stime;
//	OS_printf("Utime: %llu\n", utime);
//	OS_printf("Stime: %llu\n", stime);


	return(procData);
}

uint8 EA_CalibrateTop(int32 pid)
{
	FILE *fp;
	char output[1024];
	char top_cmd_calibrate[80];
	char *cpu_perc = "%CPU";

	/* Generate top calibration command for PID */
	snprintf(top_cmd_calibrate, sizeof(top_cmd_calibrate), "/usr/bin/top -b -n 1 -p %i | grep %%CPU", pid);

	fp = popen(top_cmd_calibrate, "r");
	if (fp == NULL) {
		OS_printf("Failed to run command top\n" );
		//exit(1);
	}

	int util_ndx = -1;
	int count_ndx = 0;
	/* Read the CPU% column header and get it's index in top output */
	if (fgets(output, sizeof(output)-1, fp) != NULL)
	{
		for (char *tok = strtok(output," "); tok != NULL; tok = strtok(NULL, " "))
		{
			if (strcmp(tok, "%CPU") == 0)
			{
				util_ndx = count_ndx;
				break;
			}
			count_ndx++;
		}
	}
	else
	{
		// Calibration failed
		OS_printf("Calibration faild\n");
	}
	pclose(fp);

	return(util_ndx);
}

uint8 EA_GetPidUtil(int32 pid, uint8 util_ndx)
{
	FILE *fp;
	char output[1024];
	char top_cmd[80];
	int util = -1;
	int TOP_READ_LINE = 1;
	/* Generate top command for PID */
	snprintf(top_cmd, sizeof(top_cmd), "/usr/bin/top -p %i -d 1 -b -n 2 | grep %i", pid, pid);

	fp = popen(top_cmd, "r");
	if (fp == NULL) {
		printf("Failed to run command\n" );
		exit(1);
	}

	int output_line = 0;
	int count_ndx = 0;
	/* Read the output a line at a time - output it. */
	while (fgets(output, sizeof(output)-1, fp) != NULL) {
		if (output_line == TOP_READ_LINE)
		{
			for (char *tok = strtok(output," "); tok != NULL; tok = strtok(NULL, " "))
			{
				if (count_ndx == util_ndx)
				{
					util = atoi(tok);
				}
				count_ndx++;
			}
		}
		output_line++;
	}

	OS_printf("Util: %i\n", util);

	/* close */
	pclose(fp);

	return 0;
}








