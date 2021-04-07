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
#include "ILITek_Device.h"
#include "ILITek_CMDDefine.h"
#include "ILITek_Protocol.h"
#include "API/ILITek_Upgrade.h"
#include "ILITek_Main.h"

int PanelInfor_V6()
{
	int ret=0;
	uint8_t Wbuff[64] = {0}, Rbuff[64] = {0};

	Wbuff[0] = (uint8_t)ILITEK_TP_CMD_GET_RESOLUTION;
	ret = TransferData(Wbuff, 1, Rbuff, 0xF,1000);
	ptl.x_max = ((uint32_t)Rbuff[1]) * 256 + Rbuff[0];
	ptl.y_max = ((uint32_t)Rbuff[3]) * 256 + Rbuff[2];
	ptl.x_ch = ((uint32_t)Rbuff[5]) * 256 + Rbuff[4];
	ptl.y_ch = ((uint32_t)Rbuff[7]) * 256 + Rbuff[6];
	if (inConnectStyle == _ConnectStyle_I2C_)
		ptl.key_num = Rbuff[9];
	ptl.ic_num = Rbuff[10];
	ptl.mode_num = Rbuff[11];

	if (ptl.ver > PROTOCOL_V6_0_2) {
		ptl.block_num = Rbuff[14];
		PRINTF("%s, max_x=%u, max_y=%u, xch=%u, ych=%u, IC Number:%d, Key Number:%d, Block Number:%d, Support Mode:%d, ret=%d\n",
				__func__, ptl.x_max,ptl.y_max, ptl.x_ch, ptl.y_ch, ptl.ic_num, ptl.key_num, ptl.block_num, ptl.mode_num, ret);
	} else {
		PRINTF("%s, max_x=%u, max_y=%u, xch=%u, ych=%u, IC Number:%d, Key Number:%d, Support Mode:%d ret=%d\n", __func__, ptl.x_max,
				ptl.y_max, ptl.x_ch, ptl.y_ch, ptl.ic_num, ptl.key_num, ptl.mode_num, ret);
	}

	if (ptl.key_num > 0)
		ret = GetKeyInfor_V6(ptl.key_num);

	return ret;
}

int GetKeyInfor_V6(int key_num) {
	int ret = 0;
	uint8_t Wbuff[64] = {0}, *Rbuff;

	int r_len = key_num * ILITEK_KEYINFO_FORMAT_LENGTH + ILITEK_KEYINFO_V6_HEADER;
	Rbuff = (uint8_t *)calloc(r_len, sizeof(uint8_t));
	Wbuff[0] = (unsigned char)ILITEK_TP_CMD_GET_KEY_INFORMATION;
	ret = TransferData(Wbuff, 1, Rbuff, r_len, 1000);
	ptl.key_mode = Rbuff[0];
	PRINTF("%s, key mode:%d, ret=%d\n", __func__, ptl.key_mode, ret);
	free(Rbuff);
	return ret;
}

int SetDataLength_V6(uint32_t data_len)
{
	int ret=0;
	uint8_t Wbuff[64] = {0}, Rbuff[64] = {0};

	Wbuff[0] = (uint8_t)ILITEK_TP_CMD_SET_DATA_LENGTH;
	Wbuff[1] = (uint8_t)(data_len & 0xFF);
	Wbuff[2] = (uint8_t)(data_len >> 8);
	ret = TransferData(Wbuff, 3, Rbuff, 0, 1000);

	PRINTF("%s, Set data length:%d, ret=%u\n", __func__, data_len, ret);
	return ret;
}
int viSwitchMode_V6(int mode)
{
	int ret = _FAIL;
	uint8_t Wbuff[64] = {0}, Rbuff[64] = {0};

	Wbuff[0]=(uint8_t)ILITEK_TP_CMD_SWITCH_MODE;
	Wbuff[1] = (uint8_t)0x5A;
	Wbuff[2] = (uint8_t)0xA5;
	Wbuff[3] = mode;
	if(TransferData(Wbuff, 4, Rbuff, 0, 1000) < 0) {
		return _FAIL;
	}

	else {
		ret = CheckBusy(20, 100, NO_NEED);
		if(ret != _SUCCESS)
			return _FAIL;
	}

	if(GetFWMode() < 0)
		return _FAIL;
	if(ptl.mode[2] == mode) {
		PRINTF("Set mode Success\n");
	}
	else {
		PRINTF("Set mode Error, mode:%d, buf:0x%x 0x%x 0x%x\n", ptl.mode[2], ptl.mode[0], ptl.mode[1], ptl.mode[2]);
	}
	return _SUCCESS;
}

uint32_t GetICBlockCrcAddr(uint32_t start, uint32_t end, uint32_t type)
{
	uint32_t crc = 0;
	uint8_t Wbuff[64] = {0}, Rbuff[64] = {0};
	Wbuff[0] = ILITEK_TP_CMD_GET_BLOCK_CRC_FOR_ADDR;
	if (type) {
		Wbuff[1] = 0;
		Wbuff[2] = start;
		Wbuff[3] = (start >> 8) & 0xFF;
		Wbuff[4] = (start >> 16) & 0xFF;
		Wbuff[5] = end & 0xFF;
		Wbuff[6] = (end >> 8) & 0xFF;
		Wbuff[7] = (end >> 16) & 0xFF;
		if (inConnectStyle == _ConnectStyle_I2C_) {
			if(TransferData(Wbuff, 8, Rbuff, 0, 1000) < 0)
				return _FAIL;

			if (CheckBusy(50, 50, SYSTEM_BUSY) < 0) {
				PRINTF("%s, Last: CheckBusy Failed\n", __func__);
				return _FAIL;
			}
		} else if (inConnectStyle == _ConnectStyle_I2CHID_) {
			if (TransferData(Wbuff, 8, Rbuff, 0, 1000) < 0)
				return _FAIL;
			if (viWaitAck(Wbuff[0], 1500000) < 0)
				return _FAIL;
		} else {
			if (TransferData(Wbuff, 8, Rbuff, 1, 1000) < 0) {
				PRINTF("%s, Last: Check IC Ack Failed\n", __func__);
				return _FAIL;
			}
		}
		//return _SUCCESS;
	}
	Wbuff[1] = 1;
	if (TransferData(Wbuff, 2, Rbuff, 2, 1000) < 0)
		return _FAIL;
	crc = Rbuff[0]+(Rbuff[1] << 8);
	//PRINTF("%s, Block CRC=0x%x,ret=%u\n", __func__, crc, ret);
	return crc;
}

uint32_t GetICBlockCrcNum(uint32_t block, uint32_t type)
{
	uint32_t crc = 0;
	uint8_t Wbuff[64] = {0}, Rbuff[64] = {0};

	Wbuff[0] = ILITEK_TP_CMD_GET_BLOCK_CRC_FOR_NUM;
	if(type) {
		Wbuff[1] = 0;
		Wbuff[2] = block;
		if (inConnectStyle == _ConnectStyle_I2C_) {
			if(TransferData(Wbuff, 3, Rbuff, 0, 1000) < 0)
				return _FAIL;

			if (CheckBusy(50, 50, SYSTEM_BUSY) < 0)
			{
				PRINTF("%s, Last: CheckBusy Failed\n", __func__);
				return _FAIL;
			}
		} else if (inConnectStyle == _ConnectStyle_I2CHID_) {
			if(TransferData(Wbuff, 3, Rbuff, 0, 1000) < 0)
				return _FAIL;
			if (viWaitAck(Wbuff[0], 1500000) < 0)
				return _FAIL;
		} else {
			if(TransferData(Wbuff, 3, Rbuff, 1, 1000) < 0) {
				PRINTF("%s, Last: Check IC Ack Failed\n", __func__);
				return _FAIL;
			}
		}
		//return _SUCCESS;
	}
	Wbuff[1] = 1;
	if(TransferData(Wbuff, 2, Rbuff, 2, 1000) < 0)
		return _FAIL;
	crc = Rbuff[0]+(Rbuff[1] << 8);
	PRINTF("%s, Block CRC=0x%x\n", __func__, crc);
	return crc;
}

int WriteFlashEnable_BL1_8(uint32_t start,uint32_t end)
{
	uint8_t Wbuff[64] = {0}, Rbuff[64] = {0};

	Wbuff[0] = (unsigned char)ILITEK_TP_CMD_WRITE_FLASH_ENABLE;//0xCC
	Wbuff[1] = 0x5A;
	Wbuff[2] = 0xA5;
	Wbuff[3] = start & 0xFF;
	Wbuff[4] = (start >> 8) & 0xFF;
	Wbuff[5] = start >> 16;
	Wbuff[6] = end & 0xFF;
	Wbuff[7] = (end >> 8) & 0xFF;
	Wbuff[8] = end >> 16;

	if(TransferData(Wbuff, 9, Rbuff, 0, 1000) < 0)
		return _FAIL;
	return _SUCCESS;
}

int WriteSlaveFlashEnable_BL1_8(uint32_t start,uint32_t end)
{
	int ret = _FAIL;
	uint8_t Wbuff[64] = {0}, Rbuff[64] = {0};

	Wbuff[0] = (unsigned char)ILITEK_TP_CMD_WRITE_FLASH_ENABLE;//0xCC
	Wbuff[1] = 0x5A;
	Wbuff[2] = 0xA5;
	Wbuff[3] = start & 0xFF;
	Wbuff[4] = (start >> 8) & 0xFF;
	Wbuff[5] = start >> 16;
	Wbuff[6] = end & 0xFF;
	Wbuff[7] = (end >> 8) & 0xFF;
	Wbuff[8] = end >> 16;
	if(inConnectStyle==_ConnectStyle_I2C_) {
		ret=TransferData(Wbuff, 9, Rbuff, 0, 1000);
		PRINTF("Please wait updating...\n");
		sleep(20);
	}
	else {
		PRINTF("Please wait updating...\n");
		ret=TransferData(Wbuff, 9, Rbuff, 1, 15000000);
		sleep(5);
		InitDevice();
	}
	if(ret < 0)
		return _FAIL;
	return _SUCCESS;
}

int SetProgramKey_V6()
{
	uint8_t Wbuff[64] = {0}, Rbuff[64] = {0};

	Wbuff[0]=(uint8_t)ILITEK_TP_CMD_WRITE_FLASH_ENABLE;
	Wbuff[1]=0x5A;
	Wbuff[2]=0xA5;
	if(TransferData(Wbuff, 3, Rbuff, 0, 1000) < 0)
		return _FAIL;
	return _SUCCESS;
}

int ModeCtrl_V6(uint8_t mode, uint8_t engineer)
{
	int ret = _FAIL;
	uint8_t Wbuff[64] = {0}, Rbuff[64] = {0};

	Wbuff[0] = (uint8_t)ILITEK_TP_CMD_SET_MODE_CONTORL;
	Wbuff[1] = mode;
	Wbuff[2] = engineer; //daemon no need engineer mode
	ret = TransferData(Wbuff, 3, Rbuff, 0, 1000);
	usleep(100000);
	switch(mode) {
	case ENTER_NORMAL_MODE:
		PRINTF("Change to Normal mode:");
		break;
	case ENTER_DEBUG_MODE:
		PRINTF("Change to Debug mode:");
		break;
	case ENTER_SUSPEND_MODE:
		PRINTF("Change to Suspend mode:");
		break;
	case ENTER_TEST_MODE:
		PRINTF("Change to Test mode:");
		break;
	}
	usleep(200000);
	if(ret < _SUCCESS) {
		PRINTF("Fail\n");
		return _FAIL;
	}
	PRINTF("Success\n");
	return _SUCCESS;
}

int ModeCtrl_V6_nowait(uint8_t mode, uint8_t engineer)
{
	uint8_t Wbuff[64] = {0};

	Wbuff[0] = (uint8_t)ILITEK_TP_CMD_SET_MODE_CONTORL;
	Wbuff[1] = mode;
	Wbuff[2] = engineer; //daemon no need engineer mode

	if (TransferData(Wbuff, 3, NULL, 0, 1000) < _SUCCESS)
		return _FAIL;
	return _SUCCESS;
}

int GetSlaveICMode_V6(int number)
{
	int i=0;
	uint8_t Wbuff[64] = {0}, Rbuff[64] = {0};

	Wbuff[0] = (uint8_t)ILITEK_TP_CMD_READ_OP_MODE;
	if(TransferData(Wbuff, 1, Rbuff, number * 2, 1000) < 0)
		return _FAIL;
	for(i = 0; i < number; i++){
		upg.ic[i].mode = Rbuff[i*2];
		PRINTF("IC[%d] mode: 0x%x\n", i, upg.ic[i].mode);
	}
	return _SUCCESS;
}

uint32_t GetAPCRC(int number)
{
	int i=0;
	uint8_t Wbuff[64] = {0}, Rbuff[64] = {0};

	Wbuff[0] = ILITEK_TP_GET_AP_CRC;
	if(TransferData(Wbuff, 1, Rbuff, number * 2, 1000) < 0)
		return _FAIL;
	for(i = 0; i < number; i++){
		upg.ic[i].crc = Rbuff[i*2] + (Rbuff[i*2+1] << 8);
		PRINTF("IC[%d] CRC: 0x%x\n", i, upg.ic[i].crc);
	}
	return _SUCCESS;
}

int SetAccessSlave(int number, uint8_t type)
{
	uint8_t Wbuff[64] = {0}, Rbuff[64] = {0};

	Wbuff[0] = (uint8_t)ILITEK_TP_CMD_ACCESS_SLAVE;
	Wbuff[1] = 0x3;
	Wbuff[2] = type;

	if (inConnectStyle == _ConnectStyle_I2C_) {
		if(TransferData(Wbuff, 3, Rbuff, 0, 1000) < 0)
			return _FAIL;
		if (CheckBusy(10, 100, SYSTEM_BUSY) < 0)
		{
			PRINTF("%s, Last: CheckBusy Failed\n", __func__);
			return _FAIL;
		}
	} else if (inConnectStyle == _ConnectStyle_I2CHID_) {
		if(TransferData(Wbuff, 3, Rbuff, 0, 1000) < 0)
			return _FAIL;
		if (viWaitAck(Wbuff[0], 1500000) < 0)
			return _FAIL;
	} else {
		if(TransferData(Wbuff, 3, Rbuff, 1, 1000) < 0) {
			PRINTF("%s, Last: Check IC Ack Failed\n", __func__);
			return _FAIL;
		}
	}
	return _SUCCESS;
}

int CheckBusy_6X(int count, int delay, int type)
{
	int busyState=0;
	uint8_t Wbuff[64] = {0}, Rbuff[64] = {0};
	do
	{
		Wbuff[0]=ILITEK_TP_CMD_GET_SYSTEM_BUSY;
		TransferData(Wbuff, 1, Rbuff, 1, 1000);
		busyState=Rbuff[0] & (SYSTEM_RETRY + type);
		if(busyState!=SYSTEM_RETRY)
			usleep(delay * 1000);
		count--;
	}
	while(count>0 && busyState!=SYSTEM_RETRY);
	if(busyState == SYSTEM_RETRY)
		return _SUCCESS;
	PRINTF("%s, FW is busy, ret=%u\n", __func__, Rbuff[0]);
	return _FAIL;
}

int SetFsInfo(uint16_t mc_sine_start, uint16_t mc_sine_end, uint8_t mc_sine_step,
		uint16_t mc_swcap_start, uint16_t mc_swcap_end, uint8_t mc_swcap_step,
		uint16_t sc_swcap_start, uint16_t sc_swcap_end, uint8_t sc_swcap_step,
		uint16_t frame_num, uint8_t scan_data)
{
	int ret = _FAIL;
	uint8_t Wbuff[64] = {0}, Rbuff[64] = {0};

	Wbuff[0] = (uint8_t)ILITEK_TP_CMD_SET_FS_INFO;
	Wbuff[1] = mc_sine_start & 0xFF;
	Wbuff[2] = mc_sine_start >> 8;
	Wbuff[3] = mc_sine_end & 0xFF;
	Wbuff[4] = mc_sine_end >> 8;
	Wbuff[5] = mc_sine_step;
	Wbuff[6] = mc_swcap_start & 0xFF;
	Wbuff[7] = mc_swcap_start >> 8;
	Wbuff[8] = mc_swcap_end & 0xFF;
	Wbuff[9] = mc_swcap_end >> 8;
	Wbuff[10] = mc_swcap_step;
	Wbuff[11] = sc_swcap_start & 0xFF;
	Wbuff[12] = sc_swcap_start >> 8;
	Wbuff[13] = sc_swcap_end & 0xFF;
	Wbuff[14] = sc_swcap_end >> 8;
	Wbuff[15] = sc_swcap_step;
	PRINTF("ptl.ver=0x%x protocol:0x%x\n", ptl.ver, PROTOCOL_V6_0_2);
	if(ptl.ver < PROTOCOL_V6_0_2) {
		ret = TransferData(Wbuff, 16, Rbuff, 0, 10000);
	}
	else {
		Wbuff[16] = (uint8_t)(frame_num & 0xFF);
		Wbuff[17] = (frame_num & 0xFF00) >> 8;
		Wbuff[18] = scan_data;
		ret = TransferData(Wbuff, 19, Rbuff, 0, 10000);
	}
	if(ret < 0)
		return _FAIL;
	return _SUCCESS;
}

uint32_t SetShortInfo(uint8_t dump1,uint8_t dump2, uint8_t verf, uint8_t posidleL, uint8_t posidleH)
{
	int ret=0;
	uint8_t Wbuff[64] = {0};

	Wbuff[0] = ILITEK_TP_CMD_SET_SHORT_INFO;
	Wbuff[1] = dump1;
	Wbuff[2] = dump2;
	Wbuff[3] = verf;
	Wbuff[4] = posidleL;
	Wbuff[5] = posidleH;
	ret = TransferData(Wbuff, 6, NULL, 0, 1000);
	return ret;
}

uint32_t SetOpenInfo(uint8_t frep_L,uint8_t frep_H, uint8_t gain)
{
	int ret=0;
	uint8_t Wbuff[64] = {0};

	Wbuff[0] = ILITEK_TP_CMD_SET_OPEN_INFO;
	Wbuff[1] = frep_L;
	Wbuff[2] = frep_H;
	Wbuff[3] = gain;
	ret = TransferData(Wbuff, 4, NULL, 0, 1000);
	return ret;
}

uint32_t SetFlashAddress_V6(uint32_t addr)
{
	int ret = _SUCCESS;
	uint8_t Wbuff[64] = {0};

	Wbuff[0] = ILITEK_TP_CMD_SET_FLASH_ADDRESS;
	Wbuff[1] = addr & 0xFF;
	Wbuff[2] = addr >> 8;
	Wbuff[3] = addr >> 16;
	ret = TransferData(Wbuff, 4, NULL, 0, 1000);
	if(ret < 0)
		return _FAIL;
	return _SUCCESS;
}

uint32_t ReadFlash_V6(uint8_t type, uint8_t *buff, uint32_t len)
{
	int ret = _SUCCESS;
	uint8_t Wbuff[64] = {0};

	Wbuff[0] = ILITEK_TP_CMD_GET_FLASH;
	Wbuff[1] = type;
	if(type == FLASH_PREPARE)
		ret = TransferData(Wbuff, 2, NULL, 0, 100000);
	else
	{
		ret = TransferData(Wbuff, 2, buff, len, 1000);
	}
	if(ret < 0)
		return _FAIL;
	return _SUCCESS;
}

int32_t GetFlashData_V6(uint32_t start, uint32_t len, char *path) {
	uint8_t *Rbuff = NULL, *buff = NULL;
	int t_len = 4096, remain_len = 0;
	unsigned int addr = 0;

	if (ChangeToBootloader() == _FAIL)
		return _FAIL;
	buff = (uint8_t *)calloc(t_len, sizeof(uint8_t));
	Rbuff = (uint8_t *)calloc(ILITEK_DEFAULT_I2C_MAX_FIRMWARE_SIZE, sizeof(uint8_t));

	for(addr = start; addr < start + len;) {
		remain_len = start + len - addr;
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
		ReadFlash_V6(FLASH_READY, buff, t_len);
		memcpy(Rbuff+addr, buff, t_len);
		addr += t_len;
	}
	SaveFlashFile(Rbuff, start, len, path);
	free(buff);
	free(Rbuff);

	if (ChangeToAPMode() == _FAIL)
	{
		PRINTF("%s, Change to ap mode failed\n", __func__);
		return _FAIL;
	}
	return _SUCCESS;
}


