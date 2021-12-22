// SPDX-License-Identifier: GPL-2.0
/*
 * ILITEK Linux Daemon Tool
 *
 * Copyright (c) 2021 Luca Hsu <luca_hsu@ilitek.com>
 * Copyright (c) 2021 Joe Hung <joe_hung@ilitek.com>
 *
 * The code could be used by anyone for any purpose,
 * and could perform firmware update for ILITEK's touch IC.
 */
#include "ILITek_CMDDefine.h"
#include "ILITek_Device.h"
#include "ILITek_Debug.h"

static volatile sig_atomic_t sig_terminate_thread = 0;
static volatile sig_atomic_t sig_stop_show = 0;
static volatile sig_atomic_t sig_stop_time = 0;

void *Debug_do_GetMsg(void *data)
{
	struct Debug_Handle *debug = (struct Debug_Handle *)data;
	time_t rawtime;
	struct tm *timeinfo;
	struct timeval tv;
	uint8_t end_index;
	char dbg_log[256];

	debug->error = netlink_connect(&debug->nl, "Daemon_Debug_Start",
				       MAX_PAYLOAD, 1000);
	if (debug->error < 0)
		goto exit_pthread;

	i2c_read_data_enable(true);

#ifdef SAVE_DEBUGLOG
	debug->debug_dir_name = "Debug";
	debug->file_idx = 0;
	debug->packet_num = 0;

	if (access(debug->debug_dir_name, F_OK) < 0) {
		if (mkdir(debug->debug_dir_name, 0777)) {
			LD_ERR("create dir: %s failed, err: %d\n",
				debug->debug_dir_name, errno);
			debug->error = errno;
			goto disconnect_netlink;
		}
	}

	time(&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(debug->timebuf, sizeof(debug->timebuf),
		 "%Y%m%d_%H%M%S", timeinfo);
	memset(debug->filename, 0, sizeof(debug->filename));
	sprintf(debug->filename, "%s/%s_%03u.csv", debug->debug_dir_name,
		debug->timebuf, debug->file_idx);

	debug->file = fopen(debug->filename, "w");
	if (!debug->file) {
		LD_ERR("fopen file: %s failed, err: %d\n",
			debug->filename, errno);
		debug->error = errno;
		goto disconnect_netlink;
	}
#endif

	while (!sig_terminate_thread) {

		if (netlink_recv(&debug->nl, debug->nl_buf) < 0)
			continue;

		if ((uint8_t)debug->nl_buf[0] != 0xDB || debug->nl_buf[1] < 2)
			continue;

		gettimeofday(&tv, NULL);

		if (sig_stop_show)
			continue;

		end_index = debug->nl_buf[1];
		debug->nl_buf[end_index] = '\0';
		memset(dbg_log, 0, sizeof(dbg_log));

		if (!sig_stop_time)
			sprintf(dbg_log, "[%lu.%lu] ", tv.tv_sec, tv.tv_usec);
		sprintf(dbg_log + strlen(dbg_log), "%s", debug->nl_buf + 2);

		LD_MSG("%s", dbg_log);

		fflush(stdout);

#ifdef SAVE_DEBUGLOG
		if (debug->packet_num >= 300000) {
			debug->file_idx++;
			debug->packet_num = 0;

			if (debug->file)
				fclose(debug->file);

			memset(debug->filename, 0,
			       sizeof(debug->filename));
			sprintf(debug->filename, "%s/%s_%03u.csv",
				debug->debug_dir_name, debug->timebuf,
				debug->file_idx);
			debug->file = fopen(debug->filename, "w");
			if (!debug->file) {
				LD_ERR("fopen file: %s failed, err: %d\n",
					debug->filename, errno);
				debug->error = errno;
				goto disconnect_netlink;
			}
		}

		fprintf(debug->file, "%s", dbg_log);
		fflush(debug->file);

		debug->packet_num++;
#endif
	}

	LD_MSG("Debug file path: %s\n", debug->filename);
	if (debug->file)
		fclose(debug->file);

disconnect_netlink:
	i2c_read_data_enable(false);
	netlink_disconnect(&debug->nl, "Daemon_Debug_End");
exit_pthread:
	pthread_exit(NULL);
}

void terminate_act_cb(int signal, siginfo_t *siginfo, void *context)
{
	UNUSED(siginfo);
	UNUSED(context);

	sig_terminate_thread = 1;
	LD_MSG("\n[%s] signal: %d\n", __func__, signal);
}

void stop_act_cb(int signal, siginfo_t *siginfo, void *context)
{
	UNUSED(siginfo);
	UNUSED(context);

	sig_stop_show = (sig_stop_show) ? 0 : 1;

	LD_MSG("\n[%s] signal: %d\n", __func__, signal);
}

void signal_act_cb(int signal, siginfo_t *siginfo, void *context)
{
	UNUSED(siginfo);
	UNUSED(context);

	sig_stop_time = (sig_stop_time) ? 0 : 1;

	LD_MSG("\n[%s] signal: %d\n", __func__, signal);
}

int Debug_Main()
{
	struct sigaction terminate_act, stop_act, signal_act;
	struct Debug_Handle debug;

	memset(&terminate_act, 0, sizeof(stop_act));
  	terminate_act.sa_sigaction  = terminate_act_cb;
	terminate_act.sa_flags = SA_SIGINFO;

	if (sigaction(SIGINT, &terminate_act, NULL) < 0) {
		LD_ERR("sigaction failed\n");
		return _FAIL;
	}

	memset(&stop_act, 0, sizeof(stop_act));
  	stop_act.sa_sigaction  = stop_act_cb;
	stop_act.sa_flags = SA_SIGINFO;

	if (sigaction(SIGTSTP, &stop_act, NULL) < 0) {
		LD_ERR("sigaction failed\n");
		return _FAIL;
	}

	memset(&signal_act, 0, sizeof(signal_act));
  	signal_act.sa_sigaction  = signal_act_cb;
	signal_act.sa_flags = SA_SIGINFO;

	if (sigaction(SIGQUIT, &signal_act, NULL) < 0) {
		LD_ERR("sigaction failed\n");
		return _FAIL;
	}

	memset(&debug, 0, sizeof(debug));
	pthread_create(&debug.get_msg_t, NULL, Debug_do_GetMsg, &debug);
	pthread_join(debug.get_msg_t, NULL);

	return 0;
}