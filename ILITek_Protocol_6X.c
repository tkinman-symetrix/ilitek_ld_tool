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

int GetFWVersion_BL()
{
	int ret;

	uint8_t Wbuff[64], Rbuff[64];

	Wbuff[0] = (uint8_t)ILITEK_TP_CMD_GET_FIRMWARE_VERSION;
	ret = TransferData(Wbuff, 1, Rbuff, 8, 1000);

	memcpy(FWVersion, Rbuff, 8);
	LD_MSG("[%s] ver: 0x%02X.0x%02X.0x%02X.0x%02X.0x%02X.0x%02X.0x%02X.0x%02X\n",
		__func__, Rbuff[0], Rbuff[1], Rbuff[2], Rbuff[3],
		Rbuff[4], Rbuff[5], Rbuff[6], Rbuff[7]);

	return ret;
}


int GetCRC_V6()
{
	int error;
	int i;
	uint8_t Wbuff[64] = {0}, Rbuff[64] = {0};
	uint32_t CRC = 0;

	if (ptl.ic_num > 32) {
		LD_ERR("[%s] unexpected IC number: %d\n", __func__, ptl.ic_num);
		return _FAIL;
	}

	Wbuff[0] = ILITEK_TP_GET_AP_CRC;
	error = TransferData(Wbuff, 1, Rbuff, 2 * ptl.ic_num, 1000);

	CRC = get_le16(Rbuff);
	LD_MSG("[FW CRC] Master: 0x%X", CRC);

	for (i = 1; i < ptl.ic_num; i++) {
		CRC = get_le16(Rbuff + 2 * i);
		LD_MSG(", Slave[%d]: 0x%X", i, CRC);
	}
	LD_MSG("\n");

	return error;
}

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
		LD_MSG("%s, max_x=%u, max_y=%u, xch=%u, ych=%u, IC Number:%d, Key Number:%d, Block Number:%d, Support Mode:%d, ret=%d\n",
				__func__, ptl.x_max,ptl.y_max, ptl.x_ch, ptl.y_ch, ptl.ic_num, ptl.key_num, ptl.block_num, ptl.mode_num, ret);
	} else {
		LD_MSG("%s, max_x=%u, max_y=%u, xch=%u, ych=%u, IC Number:%d, Key Number:%d, Support Mode:%d ret=%d\n", __func__, ptl.x_max,
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
	LD_MSG("%s, key mode:%d, ret=%d\n", __func__, ptl.key_mode, ret);
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

	return ret;
}
int viSwitchMode_V6(int mode)
{
	uint8_t Wbuff[64] = {0};

	Wbuff[0]=(uint8_t)ILITEK_TP_CMD_SWITCH_MODE;
	Wbuff[1] = (uint8_t)0x5A;
	Wbuff[2] = (uint8_t)0xA5;
	Wbuff[3] = mode;

	if (write_and_wait_ack(Wbuff, 4, 5000, 20, 100, SYSTEM_BUSY) < 0)
		return _FAIL;

	if (GetFWMode() < 0)
		return _FAIL;
	if (ptl.mode[2] == mode)
		LD_MSG("Set mode Success\n");
	else
		LD_ERR("Set mode Error, mode:%d, buf:0x%x 0x%x 0x%x\n", ptl.mode[2], ptl.mode[0], ptl.mode[1], ptl.mode[2]);

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

		if (write_and_wait_ack(Wbuff, 8, 5000, 50, 50, SYSTEM_BUSY) < 0)
			return _FAIL;
	}

	Wbuff[1] = 1;
	if (TransferData(Wbuff, 2, Rbuff, 2, 1000) < 0)
		return _FAIL;
	crc = Rbuff[0]+(Rbuff[1] << 8);
	//LD_MSG("%s, Block CRC=0x%x,ret=%u\n", __func__, crc, ret);
	return crc;
}

uint32_t GetICBlockCrcNum(uint32_t block, uint32_t type)
{
	uint32_t crc = 0;
	uint8_t Wbuff[64], Rbuff[64];

	Wbuff[0] = ILITEK_TP_CMD_GET_BLOCK_CRC_FOR_NUM;

	if (type) {
		Wbuff[1] = 0;
		Wbuff[2] = block;

		if (write_and_wait_ack(Wbuff, 3, 5000, 50, 50, SYSTEM_BUSY) < 0)
			return _FAIL;
	}

	Wbuff[1] = 1;
	if (TransferData(Wbuff, 2, Rbuff, 2, 1000) < 0)
		return _FAIL;
	crc = get_le16(Rbuff);
	LD_MSG("[%s] Block:%u CRC=0x%x\n", __func__, block, crc);
	return crc;
}

int WriteFlashEnable_BL1_8(uint32_t start,uint32_t end)
{
	uint8_t Wbuff[64] = {0}, Rbuff[64] = {0};

	Wbuff[0] = (unsigned char)ILITEK_TP_CMD_WRITE_FLASH_ENABLE;
	Wbuff[1] = 0x5A;
	Wbuff[2] = 0xA5;
	Wbuff[3] = start & 0xFF;
	Wbuff[4] = (start >> 8) & 0xFF;
	Wbuff[5] = start >> 16;
	Wbuff[6] = end & 0xFF;
	Wbuff[7] = (end >> 8) & 0xFF;
	Wbuff[8] = end >> 16;

	if (TransferData(Wbuff, 9, Rbuff, 0, 1000) < 0)
		return _FAIL;

	return _SUCCESS;
}

int WriteSlaveFlashEnable_BL1_8(uint32_t start,uint32_t end)
{
	int ret = _FAIL;
	uint8_t Wbuff[64] = {0}, Rbuff[64] = {0};

	Wbuff[0] = (unsigned char)ILITEK_TP_CMD_WRITE_FLASH_ENABLE;
	Wbuff[1] = 0x5A;
	Wbuff[2] = 0xA5;
	Wbuff[3] = start & 0xFF;
	Wbuff[4] = (start >> 8) & 0xFF;
	Wbuff[5] = start >> 16;
	Wbuff[6] = end & 0xFF;
	Wbuff[7] = (end >> 8) & 0xFF;
	Wbuff[8] = end >> 16;

	LD_MSG("Please wait updating...\n");

	if (inConnectStyle==_ConnectStyle_I2C_) {
		ret = TransferData(Wbuff, 9, Rbuff, 0, 1000);
		sleep(20);
	} else {
		ret = TransferData(Wbuff, 9, Rbuff, 1, 15000);
		sleep(5);
		InitDevice();
	}
	if(ret < 0)
		return _FAIL;
	return _SUCCESS;
}

int CtrlParameter_V6(uint8_t fun, uint8_t ctrl, uint8_t type, uint8_t *RData, int WLen, int RLen)
{
	uint8_t *Wbuff, *Rbuff;
	int rlen = 0, wlen = 0, rCount = 0, wCount = 0;
	int ret = 0;

	if (ptl.ver >= PROTOCOL_V6_0_4) {
		rlen = BYTE_2K;
		wlen = BYTE_2K;
	} else {
		rlen = BYTE_1K;
		wlen = BYTE_2K;
	}
	Wbuff = (uint8_t *)calloc(BYTE_2K + 7, sizeof(uint8_t));
	Rbuff = (uint8_t *)calloc(BYTE_2K + 7 , sizeof(uint8_t));

	if (!Wbuff || !Rbuff)
		return _FAIL;

	Wbuff[0] = (unsigned char)ILITEK_TP_CMD_PARAMETER_V6;
	Wbuff[1] = fun;
	Wbuff[2] = ctrl;
	Wbuff[3] = type;
	do {
		if (WLen < wCount + wlen)
			wlen = WLen;
		if (RLen < rCount + rlen)
			rlen = RLen;
		LD_MSG("rlen=%d, wlen=%d, rCount=%d\n", rlen, wlen, rCount);

		if (rlen == 0) {
			if (write_and_wait_ack(Wbuff, wlen + 4, 5000, 1000, 100,
					       SYSTEM_BUSY|INITIAL_BUSY) < 0) {
				ret = _FAIL;
				goto err_free;
			}
		} else {
			if (TransferData(Wbuff, wlen + 4, Rbuff, rlen, 1000) < 0) {
				ret = _FAIL;
				goto err_free;
			}
			memcpy(RData + rCount, Rbuff, rlen);
		}

		rCount += rlen;
	} while (rCount < RLen);

err_free:
	free(Wbuff);
	free(Rbuff);
	return ret;
}

int SetProgramKey_V6()
{
	uint8_t Wbuff[64], Rbuff[64];

	Wbuff[0] = (uint8_t)ILITEK_TP_CMD_WRITE_FLASH_ENABLE;
	Wbuff[1] = 0x5A;
	Wbuff[2] = 0xA5;
	Wbuff[3] = LEGO_AP_START_ADDRESS & 0xFF;
	Wbuff[4] = (LEGO_AP_START_ADDRESS >> 8) & 0xFF;
	Wbuff[5] = LEGO_AP_START_ADDRESS >> 16;
	Wbuff[6] = LEGO_AP_START_ADDRESS & 0xFF;
	Wbuff[7] = (LEGO_AP_START_ADDRESS >> 8) & 0xFF;
	Wbuff[8] = LEGO_AP_START_ADDRESS >> 16;

	if (TransferData(Wbuff, 9, Rbuff, 0, 1000) < 0)
		return _FAIL;
	return _SUCCESS;
}

int ModeCtrl_V6(uint8_t mode, uint8_t engineer, int delay_ms)
{
	int ret = _FAIL;
	uint8_t Wbuff[64], Rbuff[64];

	switch (mode) {
	case ENTER_NORMAL_MODE:
		LD_MSG("Change to Normal mode:");
		break;
	case ENTER_DEBUG_MODE:
		LD_MSG("Change to Debug mode:");
		break;
	case ENTER_SUSPEND_MODE:
		LD_MSG("Change to Suspend mode:");
		break;
	case ENTER_TEST_MODE:
		LD_MSG("Change to Test mode:");
		break;
	}

	Wbuff[0] = (uint8_t)ILITEK_TP_CMD_SET_MODE_CONTORL;
	Wbuff[1] = mode;
	Wbuff[2] = engineer; //daemon no need engineer mode
	ret = TransferData(Wbuff, 3, Rbuff, 0, 1000);

	if (delay_ms > 0)
		usleep(delay_ms * 1000);

	if (ret < _SUCCESS) {
		LD_ERR("Fail\n");
		return _FAIL;
	}

	LD_MSG("Success\n");
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
		LD_MSG("IC[%d] mode: 0x%x\n", i, upg.ic[i].mode);
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
		LD_MSG("IC[%d] CRC: 0x%x\n", i, upg.ic[i].crc);
	}
	return _SUCCESS;
}

int SetAccessSlave(int number, uint8_t type)
{
	uint8_t Wbuff[64] = {0};

	UNUSED(number);

	Wbuff[0] = (uint8_t)ILITEK_TP_CMD_ACCESS_SLAVE;
	Wbuff[1] = 0x3;
	Wbuff[2] = type;

	if (write_and_wait_ack(Wbuff, 3, 5000, 10, 100, SYSTEM_BUSY) < 0)
		return _FAIL;

	return _SUCCESS;
}

int CheckBusy_6X(int count, int delay, int type)
{
	int busyState=0;
	uint8_t Wbuff[64] = {0}, Rbuff[64] = {0};

	do {
		Wbuff[0] = ILITEK_TP_CMD_GET_SYSTEM_BUSY;
		TransferData(Wbuff, 1, Rbuff, 1, 1000);
		busyState = Rbuff[0] & (SYSTEM_RETRY + type);
		if (busyState != SYSTEM_RETRY)
			usleep(delay * 1000);
		count--;
	} while(count > 0 && busyState != SYSTEM_RETRY);
	if(busyState == SYSTEM_RETRY)
		return _SUCCESS;
	LD_ERR("%s, FW is busy, ret=%u\n", __func__, Rbuff[0]);
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
	LD_MSG("ptl.ver=0x%x protocol:0x%x\n", ptl.ver, PROTOCOL_V6_0_2);
	if (ptl.ver < PROTOCOL_V6_0_2) {
		ret = TransferData(Wbuff, 16, Rbuff, 0, 10000);
	} else {
		Wbuff[16] = (uint8_t)(frame_num & 0xFF);
		Wbuff[17] = (frame_num & 0xFF00) >> 8;
		Wbuff[18] = scan_data;
		ret = TransferData(Wbuff, 19, Rbuff, 0, 10000);
	}
	if(ret < 0)
		return _FAIL;
	return _SUCCESS;
}

int SetShortInfo(uint8_t dump1,uint8_t dump2, uint8_t verf, uint8_t posidleL, uint8_t posidleH)
{
	uint8_t Wbuff[64] = {0};

	Wbuff[0] = ILITEK_TP_CMD_SET_SHORT_INFO;
	Wbuff[1] = dump1;
	Wbuff[2] = dump2;
	Wbuff[3] = verf;
	Wbuff[4] = posidleL;
	Wbuff[5] = posidleH;

	return TransferData(Wbuff, 6, NULL, 0, 1000);
}

int SetOpenInfo(uint8_t frep_L,uint8_t frep_H, uint8_t gain)
{
	uint8_t Wbuff[64] = {0};

	Wbuff[0] = ILITEK_TP_CMD_SET_OPEN_INFO;
	Wbuff[1] = frep_L;
	Wbuff[2] = frep_H;
	Wbuff[3] = gain;

	return TransferData(Wbuff, 4, NULL, 0, 1000);
}

int SetFlashAddress_V6(uint32_t addr)
{
	uint8_t Wbuff[64] = {0};

	Wbuff[0] = ILITEK_TP_CMD_SET_FLASH_ADDRESS;
	Wbuff[1] = addr & 0xFF;
	Wbuff[2] = addr >> 8;
	Wbuff[3] = addr >> 16;

	return TransferData(Wbuff, 4, NULL, 0, 1000);
}

int ReadFlash_V6(uint8_t type, uint8_t *buff, uint32_t len)
{
	int ret;
	uint8_t Wbuff[64], Rbuff[8192];

	Wbuff[0] = ILITEK_TP_CMD_GET_FLASH;
	Wbuff[1] = type;
	if (type == FLASH_PREPARE) {
		ret = TransferData(Wbuff, 2, NULL, 0, 1000);
		usleep(100000);
	} else {
		if (inConnectStyle == _ConnectStyle_I2C_)
			ret = TransferData(Wbuff, 2, Rbuff, len, 1000);
		else
			ret = TransferData(NULL, 0, Rbuff, len, 1000);

		if (inConnectStyle == _ConnectStyle_I2CHID_)
			memcpy(buff, Rbuff + 5, len);
		else
			memcpy(buff, Rbuff, len);
	}

	LD_MSG("[%s] type: %u, len: %u\n", __func__, type, len);

	return (ret < 0) ? _FAIL : _SUCCESS;
}

int GetFlashData_V6(uint32_t start, uint32_t len, char *path)
{
	uint8_t *Rbuff = NULL, *buff = NULL;
	int t_len, remain_len = 0;
	unsigned int addr = 0;

	if (ChangeToBootloader() == _FAIL)
		return _FAIL;
	buff = (uint8_t *)calloc(ILITEK_DEFAULT_I2C_MAX_FIRMWARE_SIZE, sizeof(uint8_t));
	Rbuff = (uint8_t *)calloc(ILITEK_DEFAULT_I2C_MAX_FIRMWARE_SIZE, sizeof(uint8_t));

	for (addr = start; addr < start + len;) {
		remain_len = start + len - addr;
		if (remain_len <= BYTE_64)
			t_len = BYTE_64;
		else if (remain_len > BYTE_64 && remain_len <= BYTE_256)
			t_len = BYTE_256;
		else if (remain_len > BYTE_256 && remain_len <= BYTE_1K)
			t_len = BYTE_1K;
		else
			t_len = BYTE_2K;

		SetDataLength_V6(t_len);
		SetFlashAddress_V6(addr);
		ReadFlash_V6(FLASH_PREPARE, NULL, 0);
		ReadFlash_V6(FLASH_READY, buff, t_len);
		memcpy(Rbuff + addr, buff, t_len);
		addr += t_len;
	}
	SaveFlashFile(Rbuff, start, len, path);
	free(buff);
	free(Rbuff);

	if (ChangeToAPMode() == _FAIL) {
		LD_ERR("%s, Change to ap mode failed\n", __func__);
		return _FAIL;
	}

	return _SUCCESS;
}


