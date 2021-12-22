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

PROTOCOL_DATA ptl;

int software_reset()
{
	int ret = _FAIL;
	uint8_t Wbuff[64] = {0}, Rbuff[64] = {0};

	if (no_sw_reset)
		return _SUCCESS;

	/* Do not software reset for I2C-HID interface */
	if (inConnectStyle == _ConnectStyle_I2CHID_)
		return _SUCCESS;

	Wbuff[0] = (uint8_t)ILITEK_TP_CMD_SOFTWARE_RESET;
	if (inProtocolStyle == _Protocol_V3_) {
		ret = TransferData(Wbuff, 1, Rbuff, 0, 0);
		usleep(300000);
	} else {
		ret = TransferData(Wbuff, 1, Rbuff, 0, 0);
		usleep(1000000);
	}
	if (ret == _FAIL)
		return _FAIL;

	LD_MSG("[%s] ret: %d\n", __func__, ret);

	return _SUCCESS;
}

int GetCoreVersion()
{
	int ret = 0;
	int count = 0;
	uint8_t Wbuff[64] = {0}, Rbuff[64] = {0};

	Wbuff[0] = (uint8_t)ILITEK_TP_CMD_GET_INTERNAL_VERSION;
	ret=TransferData(Wbuff, 1, Rbuff, 7, 1000);
	for(count = 0; count < 7; count++)
		CoreVersion[count] = Rbuff[count];

	LD_MSG("CoreVersion=0x%X.0x%X.0x%X.0x%X\n", CoreVersion[0], CoreVersion[1], CoreVersion[2], CoreVersion[3]);

	return ret;
}

int GetFWVersion()
{
	int ret=0;

	uint8_t Wbuff[64] = {0}, Rbuff[64] = {0};

	Wbuff[0] = (uint8_t)ILITEK_TP_CMD_GET_FIRMWARE_VERSION;
	ret = TransferData(Wbuff, 1, Rbuff, 8, 1000);

	memcpy(FWVersion, Rbuff, 8);
	LD_MSG("%s, firmware version: 0x%02X.0x%02X.0x%02X.0x%02X.0x%02X.0x%02X.0x%02X.0x%02X, ret=%u\n",
		__func__, FWVersion[0], FWVersion[1], FWVersion[2],
		FWVersion[3], FWVersion[4], FWVersion[5], FWVersion[6],
		FWVersion[7], ret);

	/* Chromebook script used, should be checked with Joe */
	LD_MSG("fw version: [%02X%02X.%02X%02X.%02X%02X.%02X%02X]\n",
			FWVersion[0], FWVersion[1], FWVersion[2], FWVersion[3],
			FWVersion[4], FWVersion[5], FWVersion[6], FWVersion[7]);
	return ret;
}

int GetProtocol()
{
	int ret=0;
	uint8_t Wbuff[64] = {0}, Rbuff[64] = {0};

	Wbuff[0]=(uint8_t)ILITEK_TP_CMD_GET_PROTOCOL_VERSION;
	ret=TransferData(Wbuff, 1, Rbuff, 3,1000);
	memcpy(ProtocolVersion, Rbuff, 3);

	ptl.ver = (Rbuff[0] << 16) + (Rbuff[1] << 8) + Rbuff[2];
	LD_MSG("%s, ProtocolVersion: %x.%x.%x, 0x%x, ret=%u\n", __func__, ProtocolVersion[0], ProtocolVersion[1], ProtocolVersion[2], ptl.ver, ret);

	/* Chromebook script used, should be checked with Joe */
	LD_MSG("ptl version: [%02X.%02X]\n", ProtocolVersion[0], ProtocolVersion[1]);

	return ret;
}

int GetProtocol_in_BL()
{
	int ret = 0;
	uint8_t Wbuff[64] = {0}, Rbuff[64] = {0};

	Wbuff[0] = (uint8_t)ILITEK_TP_CMD_GET_PROTOCOL_VERSION;

	if (inProtocolStyle == _Protocol_V3_) {
    		ret = TransferData(Wbuff, 1, Rbuff, 2,1000);
		ptl.bl_ver = (Rbuff[0] << 8) + Rbuff[1];
	} else if (inProtocolStyle == _Protocol_V6_) {
		ret = TransferData(Wbuff, 1, Rbuff, 3,1000);
		ptl.bl_ver = (Rbuff[0] << 16) + (Rbuff[1] << 8) + Rbuff[2];
	}

	LD_MSG("[%s] BL vers: 0x%X\n", __func__, ptl.bl_ver);

	return ret;
}

int GetCRC_V3()
{
	int error;
	uint8_t Wbuff[64] = {0}, Rbuff[64] = {0};
	uint32_t AP_CRC = 0, DATA_CRC = 0;

	Wbuff[0] = ILITEK_TP_GET_AP_CRC;
	if (ptl.ic == 0x2312 || ptl.ic == 0x2315) {
		error = TransferData(Wbuff, 1, Rbuff, 4, 5000);
		AP_CRC = get_le16(Rbuff + 2) << 16 | get_le16(Rbuff);
	} else {
		error = TransferData(Wbuff, 1, Rbuff, 2, 5000);
		AP_CRC = get_le16(Rbuff);
	}

	Wbuff[0] = ILITEK_TP_GET_DATA_CRC;
	error = TransferData(Wbuff, 1, Rbuff, 4, 1000);
	DATA_CRC = get_le16(Rbuff + 2) << 16 | get_le16(Rbuff);

	LD_MSG("[Check Code] AP: 0x%X, Data: 0x%X\n", AP_CRC, DATA_CRC);

	return error;
}

int PanelInfor_V3()
{
	int ret=0;
	uint8_t Wbuff[64] = {0}, Rbuff[64] = {0};

	Wbuff[0] = (uint8_t)ILITEK_TP_CMD_GET_RESOLUTION;
	ret = TransferData(Wbuff, 1, Rbuff, 15,1000);
	ptl.x_max = ((unsigned int)Rbuff[1]) * 256 + Rbuff[0];
	ptl.y_max = ((unsigned int)Rbuff[3]) * 256 + Rbuff[2];
	ptl.x_ch = Rbuff[4];
	ptl.y_ch = Rbuff[5];
	if (inConnectStyle == _ConnectStyle_I2C_)
		ptl.key_num = Rbuff[8];
	LD_MSG("%s, max_x=%u, max_y=%u, xch=%u, ych=%u, Key Number:%u, ret=%u\n", __func__, ptl.x_max, ptl.y_max, ptl.x_ch, ptl.y_ch, ptl.key_num, ret);
	if (ptl.key_num > 0) {
		if (Rbuff[10] == 0xFF && Rbuff[11] == 0xFF && Rbuff[12] == 0xFF && Rbuff[13])
			ptl.key_mode = ILITEK_HW_KEY_MODE;
		LD_MSG("%s, key mode:%d\n", __func__, ptl.key_mode);
	}
	return ret;
}

int GetKeyInfor_V3(int key_num) {
	int ret = 0;
	uint8_t Wbuff[64] = {0}, *Rbuff;

	int r_len = key_num * ILITEK_KEYINFO_FORMAT_LENGTH + ILITEK_KEYINFO_V3_HEADER;
	Rbuff = (uint8_t *)calloc(r_len, sizeof(uint8_t));
	Wbuff[0] = (unsigned char)ILITEK_TP_CMD_GET_KEY_INFORMATION;
	ret = TransferData(Wbuff, 1, Rbuff, r_len, 1000);
	free(Rbuff);
	return ret;
}

int GetFWMode()
{
	uint8_t Wbuff[64] = {0}, Rbuff[64] = {0};

	Wbuff[0]=(uint8_t)ILITEK_TP_CMD_SWITCH_MODE;
	if (TransferData(Wbuff, 1, Rbuff, 3, 1000) < 0)
		return _FAIL;

	memcpy(ptl.mode, Rbuff, 3);
	if (Rbuff[2] == 0xFF)
		LD_ERR("%s, FW Mode: no support\n", __func__);
	else
		LD_MSG("%s, FW Mode: 0x%02X\n", __func__, ptl.mode[2]);
	return _SUCCESS;
}

int viSwitchMode_V3(int mode)
{
	int ret = _SUCCESS;
	uint8_t Wbuff[64] = {0}, Rbuff[64] = {0};

	Wbuff[0]=(uint8_t)ILITEK_TP_CMD_SWITCH_MODE;
	Wbuff[1] = (uint8_t)0x55;
	Wbuff[2] = (uint8_t)0xAA;
	Wbuff[3] = mode;
	ret = TransferData(Wbuff, 4, Rbuff, 0, 1000);
	if(ret != _FAIL) {
		ret = CheckBusy(20, 100, NO_NEED);
		if(ret != _SUCCESS)
			return _FAIL;
	}
	else {
		return _FAIL;
	}
	ret = GetFWMode();
	if(ptl.mode[2] == mode) {
		LD_MSG("Set mode Success\n");
	}
	else {
		LD_ERR("Set mode Error, mode:%d, buf:0x%x 0x%x 0x%x\n", ptl.mode[2], ptl.mode[0], ptl.mode[1], ptl.mode[2]);
	}
	return ret;
}



uint32_t RawDataInfor(void)
{
	int ret = _FAIL;
	uint8_t Wbuff[64] = {0}, Rbuff[64] = {0};

	Wbuff[0]=(uint8_t)ILITEK_TP_CMD_GET_RAW_DATA_INFOR;
	ret=TransferData(Wbuff, 1, Rbuff, 4, 1000);

	LD_MSG("%s, 0x%x, 0x%x, 0x%x, 0x%x, ret=%u\n", __func__, Rbuff[0], Rbuff[1], Rbuff[2], Rbuff[3], ret);
	if(ret < 0)
		return 0;
	return Rbuff[0] + (Rbuff[1] << 8) + (Rbuff[2] << 16) + (Rbuff[3] << 24);
}


int monitor_extend()
{
	int ret = _FAIL;

	buff[4] = 0xD4;
	buff[5] = 0x00;

	ret = TransferData(buff, 2, NULL, 0, 1000);
	LD_MSG("%s, ret=%u\n", __func__, ret);
	if(ret != _FAIL)
		return _SUCCESS;
	else
		return _FAIL;
}

int monitor_Copy()
{
	int ret = _FAIL;

	buff[0] = 0xD4;
	buff[1] = 0x01;

	ret = TransferData(buff, 2, NULL, 0, 1000);
	LD_MSG("%s, ret=%u\n", __func__, ret);
	if(ret != _FAIL)
		return _SUCCESS;
	else
		return _FAIL;
}

int check_status()
{
	int ret = _FAIL;

	buff[0] = 0xFC;

	ret = TransferData(buff, 1, NULL, 0, 1000);
	LD_MSG("%s, ret=%u\n", __func__, ret);
	if(ret != _FAIL)
		return _SUCCESS;
	else
		return _FAIL;
}

int switch_testmode(uint8_t *para_m, uint8_t *para_f)
{
	int ret = _FAIL;

	buff[0] = 0xD5;
	buff[1] = hex_2_dec((char *)para_m, 2);
	buff[2] = hex_2_dec((char *)para_f, 2);
	ret = TransferData(buff, 3, NULL, 0, 1000);
	LD_MSG("%s, ret=%u\n", __func__, ret);
	if(ret != _FAIL)
		return _SUCCESS;
	else
		return _FAIL;
}

int GetICMode()
{
	int ret = _FAIL;
	uint8_t Wbuff[64] = {0}, Rbuff[64] = {0};

	Wbuff[0] = (uint8_t)ILITEK_TP_CMD_READ_OP_MODE;

	ret = TransferData(Wbuff, 1, Rbuff, 1, 1000);

	ICMode = Rbuff[0];
	LD_MSG("%s, Mode:0x%X, %s mode\n", __func__, ICMode, (ICMode == 0x55) ? "BL" : "AP");
	if(ret < 0)
		return _FAIL;
	return _SUCCESS;
}

int SetProgramKey()
{
	int ret = _FAIL;

	if (inProtocolStyle == _Protocol_V3_)
		ret = SetProgramKey_V3();
	else if(inProtocolStyle == _Protocol_V6_)
		ret = SetProgramKey_V6();

	return ret;
}

int SetProgramKey_V3()
{
	int ret = _FAIL;
	uint8_t Wbuff[64] = {0}, Rbuff[64] = {0};

	Wbuff[0]=(uint8_t)ILITEK_TP_CMD_WRITE_ENABLE;
	Wbuff[1]=0x5A;
	Wbuff[2]=0xA5;
	ret = TransferData(Wbuff, 3, Rbuff, 0, 1000);
	LD_MSG("%s, ret=%u\n", __func__, ret);
	if(ret < 0)
		return _FAIL;
	return _SUCCESS;
}

int ChangeTOBL()
{
	int ret = _FAIL;

	uint8_t Wbuff[64] = {0}, Rbuff[64] = {0};

	Wbuff[0] = (uint8_t)ILITEK_TP_CMD_SET_BL_MODE;

	ret = TransferData(Wbuff, 1, Rbuff, 0, 1000);

	LD_MSG("%s, ret=%d\n", __func__, ret);

	return (ret < 0) ? _FAIL : _SUCCESS;
}

int EnterTestMode(int delay_ms)
{
	buff[0]=0xF2;
	buff[1]=0x01;

	if (TransferData(buff, 2, NULL, 0, 1000) < 0) {
		LD_ERR("%s, EnterTestMode Failed\n", __func__);
		return _FAIL;
	}

	if (delay_ms > 0)
		usleep(delay_ms * 1000);

	return _SUCCESS;
}

int ExitTestMode(int delay_ms)
{
	buff[0]=0xF2;
	buff[1]=0x00;

	if (TransferData(buff, 2, NULL, 0, 1000) < 0) {
		LD_ERR("%s, ExitTestMode Failed\n", __func__);
		return _FAIL;
	}

	if (delay_ms > 0)
		usleep(delay_ms * 1000);

	return _SUCCESS;
}

int ChangeTOAP()
{
	int ret = 0;
	uint8_t Wbuff[64] = {0}, Rbuff[64] = {0};

	Wbuff[0]=(uint8_t)ILITEK_TP_CMD_SET_AP_MODE;
	ret = TransferData(Wbuff, 1, Rbuff, 0, 1000);
	LD_MSG("%s, ret=%u\n", __func__, ret);
	if(ret < 0)
		return _FAIL;
	return _SUCCESS;
}

int GetKernelVer()
{
	int ret=0;
	char product_id[30];
	uint8_t Wbuff[64] = {0}, Rbuff[64] = {0};

	Wbuff[0] = (uint8_t)ILITEK_TP_CMD_GET_MCU_KERNEL_VER;
	ret = TransferData(Wbuff, 1, Rbuff, 32, 1000);
	memcpy(KernelVersion, Rbuff, 5);
	ptl.ic = Rbuff[0] + (Rbuff[1] << 8);
	LD_MSG("%s, mcu kernel version: %.2X.%.2X.%.2X.%.2X.%.2X, 0x%04x ret=%u\n",
		__func__, KernelVersion[0], KernelVersion[1], KernelVersion[2],
		KernelVersion[3], KernelVersion[4], ptl.ic, ret);

	/* Chromebook script used, should be checked with Joe */
	memset(product_id, 0, sizeof(product_id));
	memcpy(product_id, Rbuff+6, 26);
	LD_MSG("product id: [%04X], module: [%s]\n", ptl.ic, product_id);
	return ret;
}

int GetKernelVer_in_BL()
{
	int ret = 0;
	uint8_t Wbuff[64] = {0}, Rbuff[64] = {0};

	Wbuff[0]=(uint8_t)ILITEK_TP_CMD_GET_MCU_KERNEL_VER;
    	ret = TransferData(Wbuff, 1, Rbuff, 6, 1000);

	ptl.ic = Rbuff[0] + (Rbuff[1] << 8);
	upg.df_start_addr = (Rbuff[2] << 16) + (Rbuff[3] << 8) + Rbuff[4];

	return ret;
}

int WriteDataFlashKey(unsigned int df_end_addr,unsigned int df_check)
{
	uint8_t Wbuff[64] = {0}, Rbuff[64] = {0};

	Wbuff[0] = (uint8_t)ILITEK_TP_CMD_WRITE_ENABLE;//0xC4
	Wbuff[1] = 0x5A;
	Wbuff[2] = 0xA5;
	Wbuff[3] = 0x01;//data flash
	Wbuff[4] = df_end_addr >> 16;
	Wbuff[5] = (df_end_addr >> 8) & 0xFF;
	Wbuff[6] = (df_end_addr) & 0xFF;
	Wbuff[7] = df_check >> 16;
	Wbuff[8] = (df_check >> 8) & 0xFF;
	Wbuff[9] = df_check & 0xFF;
	if(TransferData(Wbuff, 10, Rbuff, 0, 1000) < 0)
		return _FAIL;
	return _SUCCESS;
}

int WriteAPCodeKey(unsigned int ap_end_addr,unsigned int ap_check)
{
	int ret = _FAIL;
	uint8_t Wbuff[64] = {0}, Rbuff[64] = {0};

	Wbuff[0] = (uint8_t)ILITEK_TP_CMD_WRITE_ENABLE;//0xC4
	Wbuff[1] = 0x5A;
	Wbuff[2] = 0xA5;
	Wbuff[3] = 0x00;//AP Code
	Wbuff[4] = ap_end_addr >> 16;
	Wbuff[5] = (ap_end_addr >> 8) & 0xFF;
	Wbuff[6] = (ap_end_addr) & 0xFF;
	Wbuff[7] = ap_check >> 16;
	Wbuff[8] = (ap_check >> 8) & 0xFF;
	Wbuff[9] = ap_check & 0xFF;
	ret = TransferData(Wbuff, 10, Rbuff, 0, 1000);

	LD_MSG("%s, ret=%u\n", __func__, ret);
	if(ret < 0)
		return _FAIL;
	return _SUCCESS;
}

int EraseDataFlash()
{
	int ret=0;
	uint8_t Wbuff[64] = {0xFF};

	if (ProtocolVersion[0] == 0x01 && ProtocolVersion[1] == 0x07) {
		memset(Wbuff, 0xFF,33);
		ret = WriteDataFlashKey(0x1F01F, 0xFF*32);
		usleep(10000);
		Wbuff[0] = (unsigned char)ILITEK_TP_CMD_WRITE_DATA;
		ret = TransferData(Wbuff, 33, NULL, 0, 1000);
	} else {
		ret = SetProgramKey();
		usleep(100000);
		Wbuff[0] = 0x63;
		Wbuff[1] = 0x02;
		ret=TransferData(Wbuff, 2, NULL, 0, 1000);
	}

	LD_MSG("%s, ret=%u\n", __func__, ret);
	if(ret < 0)
		return _FAIL;
	return _SUCCESS;
}

int CheckBusy(int count, int delay, int type) {
	if (inProtocolStyle == _Protocol_V3_)
		return CheckBusy_3X(count, delay);
	else if (inProtocolStyle == _Protocol_V6_)
		return CheckBusy_6X(count, delay, type);
	return _FAIL;
}

int CheckBusy_3X(int count, int delay)
{
	int busyState=0;
	uint8_t Wbuff[64] = {0}, Rbuff[64] = {0};
	do
	{
		Wbuff[0]=0x80;
		TransferData(Wbuff, 1, Rbuff, 1, 1000);
		busyState=Rbuff[0];
		if(busyState!=0x50)
			usleep(delay * 1000);
		count--;
	}
	while(count>0 && busyState!=0x50);
	if(busyState == 0x50)
		return _SUCCESS;
	LD_MSG("%s, FW is busy, ret=%u\n", __func__, busyState);
	return _FAIL;
}

unsigned int GetCodeCheckSum(uint8_t ucType)
{
	unsigned int uiCheckSum=0;
	uint8_t Wbuff[64] = {0}, Rbuff[64] = {0};

	Wbuff[0]=ILITEK_TP_GET_AP_CRC;
	if(ucType!=0)
	{
		TransferData(Wbuff, 1, Rbuff, 1, 1000);
		uiCheckSum=Rbuff[0];
	}
	else
	{
		TransferData(Wbuff, 1, Rbuff, 4, 1000);
		uiCheckSum=Rbuff[0]+(Rbuff[1] * 256)+(Rbuff[2] * 256 * 256)+(Rbuff[3] * 256 *256 * 256);
	}
	//LD_MSG("%s, Check Sum=%u,ret=%u\n", __func__, uiCheckSum,ret);
	return uiCheckSum;
}

uint32_t SetFlashAddress(uint32_t addr)
{
	uint8_t Wbuff[64] = {0};

	Wbuff[0] = ILITEK_TP_CMD_SET_FLASH_ADDRESS;
	Wbuff[3] = addr & 0xFF;
	Wbuff[2] = addr >> 8;
	Wbuff[1] = addr >> 16;
	if(TransferData(Wbuff, 4, NULL, 0, 1000) < 0)
		return _FAIL;
	return _SUCCESS;
}

uint32_t ReadFlash_V3(uint8_t *buff, uint32_t len)
{
	int ret = _SUCCESS;
	uint8_t Wbuff[64] = {0};

	Wbuff[0] = ILITEK_TP_CMD_GET_FLASH;

	ret = TransferData(Wbuff, 1, buff, len, 5000);
	if(ret < 0)
		return _FAIL;
	return _SUCCESS;
}

uint32_t SaveFlashFile(uint8_t *buff, uint32_t start, uint32_t len, char *path) {
	struct tm *timeinfo;
	time_t datetime;
	char timebuf[60],fileName[256];
	FILE *result_file = NULL;
	unsigned int  i = 0, count = 0;
	bool f_exist = false;

	time ( &datetime);
	timeinfo = localtime (&datetime);
	strftime(timebuf,60,"%Y%m%d_%I%M%S",timeinfo);
	sprintf(fileName,"/%s",path);
	if(access(fileName, F_OK) == 0) {
		sprintf(fileName,"%s/flash_%s.txt",fileName, timebuf);
		result_file = fopen(fileName, "w");
		LD_MSG("Read flash result (.csv) path =>%s\n", fileName);
		f_exist = true;
	}
	else {
		LD_ERR("Path is no exist, %s\n", fileName);
	}
	for(i = start; i < start+len; i++, count++)
	{
		if(i % 0x10 == 0) {
			LD_MSG("\n0x%06X :", i);
			if(f_exist)
				fprintf(result_file, "\n0x%06X :", i);
		}
		LD_MSG(" %02X", buff[i]);
		if(f_exist)
			fprintf(result_file, " %02X", buff[i]);
	}
	LD_MSG("\n");
	if(f_exist) {
		fprintf(result_file, "\n");
		fclose(result_file);
	}
	return _SUCCESS;
}

int32_t GetFlashData_V3(uint32_t start, uint32_t len, char *path) {
	uint8_t *Rbuff = NULL, *buff = NULL;
	int t_len = 32, count = 0;
	unsigned int addr = 0;

	if (ChangeToBootloader() == _FAIL)
		return _FAIL;
	buff = (uint8_t *)calloc(t_len, sizeof(uint8_t));
	Rbuff = (uint8_t *)calloc(len + t_len, sizeof(uint8_t));
	GetFWVersion();
	GetProtocol();
	if((ptl.ver&0xFFFF00) == 0x10700 && FWVersion[3] < 3) {
		LD_ERR("This BL no support function\n");
		goto nosupport;
	}
	for(addr = start; addr < start + len; count+=t_len) {
		SetFlashAddress(addr);
		usleep(5000);
		ReadFlash_V3(buff, t_len);
		usleep(5000);
		memcpy(Rbuff+count, buff, t_len);
		addr += t_len;
	}
	SaveFlashFile(Rbuff, start, len, path);
nosupport:
	free(buff);
	free(Rbuff);

	if (ChangeToAPMode() == _FAIL)
	{
		LD_ERR("%s, Change to ap mode failed\n", __func__);
		return _FAIL;
	}
	return _SUCCESS;
}



