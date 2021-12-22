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

#ifndef INC_ILITEK_MP_RESULT_H_
#define INC_ILITEK_MP_RESULT_H_

#include <stdint.h>
#define FLASH_SENSORTEST_START_ADDR_V6     0x3E000
#define MP_RESULT_LENGHT                    1000

struct MP_RESULT_DATA {
    uint8_t week;
    uint8_t year;
    uint8_t fw_ver[8];
    uint8_t M_name[19];
    uint8_t bar[28];
    uint8_t station;
};
extern struct MP_RESULT_DATA bbox[10];
extern int viGetMpResult();
extern int viGetMpResult_V3();
extern int viGetMpResult_V6();
int32_t BL_Read_MpResult_V6(uint8_t *data);
#endif
