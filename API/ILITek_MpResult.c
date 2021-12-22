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
#ifndef _ILITEK_MP_RESULT_C_
#define _ILITEK_MP_RESULT_C_

#include <stdio.h>
#include <stdlib.h>
#include "ILITek_RawData.h"
#include "ILITek_MpResult.h"
#include "../ILITek_Main.h"
#include "../ILITek_Protocol.h"
#include "../ILITek_CMDDefine.h"
#include "../ILITek_Device.h"

struct MP_RESULT_DATA bbox[10];

int viGetMpResult()
{
	int ret = _FAIL;

	if (inProtocolStyle == _Protocol_V3_)
		ret = viGetMpResult_V3();
	else if (inProtocolStyle == _Protocol_V6_)
		ret = viGetMpResult_V6();
	return ret;
}

int viGetMpResult_V3() {
    int ret = _SUCCESS;
    return ret;
}

void testResult(const char *testname, uint8_t data, uint8_t shift) {

    // LD_MSG("data=0x%x, shift = %d, mask = %x\n", data, shift, ((1 << shift) + (1 << shift + 1)));
    // LD_MSG("%x,ret = %x\n", data & ((1 << shift) + (1 << shift + 1)) , data & ((1 << shift) + (1 << shift + 1)) >> shift);
    if((data & ((1 << shift) + (1 << (shift + 1)))) >> shift == 1)
        LD_MSG("%s Test Result: PASS\n", testname);
    else if ((data & ((1 << shift) + (1 << (shift + 1)))) >> shift == 2)
        LD_ERR("%s Test Result: NG\n", testname);
    else
        LD_ERR("%s Test Result: N/A\n", testname);
}

int32_t BL_Read_MpResult_V6(uint8_t *data) {
	uint8_t *buff = NULL;
	int t_len = BYTE_1K, remain_len = 0;
	unsigned int addr = 0;

	if (ChangeToBootloader() == _FAIL)
		return _FAIL;
	buff = (uint8_t *)calloc(t_len, sizeof(uint8_t));

	for(addr = FLASH_SENSORTEST_START_ADDR_V6; addr < FLASH_SENSORTEST_START_ADDR_V6 + MP_RESULT_LENGHT;) {
		remain_len = FLASH_SENSORTEST_START_ADDR_V6 + MP_RESULT_LENGHT - addr;
		if(remain_len <= BYTE_64)
			t_len = BYTE_64;
		else if (remain_len > BYTE_64 && remain_len <= BYTE_256)
			t_len = BYTE_256;
		else if (remain_len > BYTE_256 && remain_len <= BYTE_1K)
			t_len = BYTE_1K;
		else if (remain_len > BYTE_1K && remain_len <= BYTE_2K)
			t_len = BYTE_2K;
		else if (remain_len > BYTE_2K && remain_len <= BYTE_4K)
			t_len = BYTE_4K;

		SetDataLength_V6(t_len);
		SetFlashAddress_V6(addr);
		ReadFlash_V6(FLASH_PREPARE, NULL, 0);
		ReadFlash_V6(FLASH_READY, buff, MP_RESULT_LENGHT);
		memcpy(data, buff, MP_RESULT_LENGHT);
		addr += t_len;
	}

	free(buff);
	if (ChangeToAPMode() == _FAIL)
	{
		LD_MSG("%s, Change to ap mode failed\n", __func__);
		return _FAIL;
	}
	return _SUCCESS;
}

int viGetMpResult_V6() {
    int ret = _SUCCESS, RLen = 0, i = 0, j = 0, header = 0;
    uint8_t *RData;
    int station = 0;

    if (GetICMode() < 0)
        return _FAIL;
    RData = (uint8_t *)calloc(1008, sizeof(uint8_t));
    if (ICMode == OP_MODE_BOOTLOADER) {
        BL_Read_MpResult_V6(RData);
        header = 0;
        RLen = MP_RESULT_LENGHT;
    } else {
        if (inConnectStyle == _ConnectStyle_I2C_)
            header = 5;
        else
            header = 6;
        RLen = 1003 + header; //data:1000 header:5 crc:2 checksum:1

        ret = CtrlParameter_V6(0x0, 0x4, 0x10, RData, 0, 0);
        if (ret < 0)
            goto END;
        ret = CtrlParameter_V6(0x1, 0x4, 0x10, RData, 0, RLen);
        if (ret < 0)
            goto END;
    }

    for (i = header; i < RLen; i = i+100, station++) {
        LD_MSG("***Station%d***\n", station + 1);

        if ((i - header)%100 == 0 && RData[i] > 52) {
            LD_MSG("No Test\n");
            continue;
        } else {
            bbox[station].week = RData[i];
            LD_MSG("Week of year : %d\n", bbox[station].week);
            bbox[station].year = RData[i+1];
            LD_MSG("Year : 20%d\n", bbox[station].year);
            memcpy(bbox[station].fw_ver, RData+(i+2), 8);
            LD_MSG("Firmware Version :");
            for(j = 0; j < 8; j++)
                LD_MSG("0x%x.",bbox[station].fw_ver[j]);
	    memset(bbox[station].M_name, 0, sizeof(bbox[station].M_name));
            memcpy(bbox[station].M_name, RData+(i+10), 19);
            LD_MSG("\nModule Name : %s\n", bbox[station].M_name);
            testResult("Short", RData[i+29], 0);
            testResult("Open", RData[i+29], 2);
            testResult("Self Cap", RData[i+29], 4);
            testResult("Uniformity", RData[i+29], 6);
            testResult("DAC", RData[i+30], 0);
            testResult("Key Raw", RData[i+30], 2);
            testResult("Final", RData[i+30], 4);
            testResult("Painting", RData[i+30], 6);
            testResult("MicroOpen", RData[i+31], 0);
            LD_MSG("Station : %d\n", RData[i+99]);
        }
    }
END:
    free(RData);
    return ret;
}
#endif