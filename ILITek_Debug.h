/* SPDX-License-Identifier: GPL-2.0 */
/*
 * ILITEK Linux Daemon Tool
 *
 * Copyright (c) 2021 Luca Hsu <luca_hsu@ilitek.com>
 * Copyright (c) 2021 Joe Hung <joe_hung@ilitek.com>
 *
 * The code could be used by anyone for any purpose,
 * and could perform firmware update for ILITEK's touch IC.
 */

#ifndef INC_ILITEK_DEBUG_H_
#define INC_ILITEK_DEBUG_H_

#include <signal.h>

#define MAX_PAYLOAD	64
#define SAVE_DEBUGLOG

struct Debug_Handle {
	int error;
        unsigned int packet_num;

        char timebuf[64];
        const char *debug_dir_name;
        char filename[128];
        FILE *file;
        unsigned int file_idx;

	pthread_t get_msg_t;
        struct Netlink_Handle nl;
        char nl_buf[2 * MAX_PAYLOAD];
};

extern void vfRunDebug_3X(char *cStyle,int iRun);
extern int Debug_Main(void);

#endif /* INC_ILITEK_DEBUGTOOL_3X_H_ */
