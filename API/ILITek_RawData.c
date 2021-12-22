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

#ifndef _ILITEK_RAWDATA_C_
#define _ILITEK_RAWDATA_C_

/* Includes of headers ------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include "ILITek_RawData.h"
#include "ILITek_SensorTest.h"
#include "../ILITek_CMDDefine.h"
#include "../ILITek_Device.h"
#include "../ILITek_Protocol.h"
#include "../ILITek_Main.h"

/* Private define ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#ifndef VERSION_MACRO
#define VERSION_MACRO(a, b, c) (((a) << 16) + ((b) << 8) + (c))
#endif
/* Private variables ---------------------------------------------------------*/
FILE *result_file;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

int viRunBGData_3X(int inFrames)
{
	int ret = _SUCCESS;
	ret = viRunCDCType_3X("BG", inFrames);
	return ret;
}

int viRunBGData_6X(int inFrames)
{
	int ret = _SUCCESS;

	ret = viRunCDCType_6X("BG", inFrames);
	return ret;
}

int viRunCDCType_3X(const char *type, int inFrames) {
	int ret = _SUCCESS;
	int inCounts = 0;
	unsigned int CHX = 0, CHY = 0, len = 0;
	uint8_t mctype = 0;
	uint8_t sctype = 0;
	uint8_t mc_offset = 0;
	uint8_t sc_offset = 0;
	uint8_t keytype = 0;
	int report[300][300];
	int max = 0, min = 0xFFFF;
	int reportkey[3][50];
	uint8_t row = 0;

	LD_MSG("Type:%s\n", type);
	if (!strcmp(type, "DAC_P")) {
		mctype = TEST_MODE_V3_MC_DAC_P;
		sctype = TEST_MODE_V3_SC_DAC_P;
		mc_offset = 19;
		sc_offset = 18;
	} else if (!strcmp(type, "DAC_N")) {
		mctype = TEST_MODE_V3_MC_DAC_N;
		sctype = TEST_MODE_V3_SC_DAC_N;
		mc_offset = 17;
		sc_offset = 16;
	} else if (!strcmp(type, "Raw")) {
		mctype = TEST_MODE_V3_MC_RAW;
		sctype = TEST_MODE_V3_SC_RAW;
		keytype = TEST_MODE_V3_KEY_RAW;
		mc_offset = 0;
		sc_offset = 3;
	} else if (!strcmp(type, "BG")) {
		mctype = TEST_MODE_V3_MC_BG;
		sctype = TEST_MODE_V3_SC_BG;
		keytype = TEST_MODE_V3_KEY_BG;
		mc_offset = 1;
		sc_offset = 4;
	} else if (!strcmp(type, "SE")) {
		mctype = TEST_MODE_V3_MC_SINGNAL;
		sctype = TEST_MODE_V3_SC_SINGNAL;
		keytype = TEST_MODE_V3_KEY_SINGNEL;
		mc_offset = 2;
		sc_offset = 5;
	} else if (!strcmp(type, "AN")) {
		mctype = TEST_MODE_V3_ALL_NODE;
		sctype = 0;
	} else {
		LD_ERR("Type no support,%s\n", type);
		return _FAIL;
	}

	do {
		ret = EnterTestMode(100);
		if (ret < 0)
			break;

		for (inCounts = 0; inCounts < inFrames; inCounts++) {
			len = ptl.x_ch * ptl.y_ch;
			row = ptl.x_ch;
			ret = viGetCDCData_3X(mctype, len, mc_offset, TEST_MODE_V3_Y_DRIVEN, row);
			if (ret < 0)
				break;

			clrscr();
			LD_MSG("%s Datas: %d/%d Frames\n", type, inCounts + 1, inFrames);
			for (CHY = 0; CHY < ptl.y_ch; CHY++) {
				LD_MSG("Y_%2dCH:", CHY);
				for (CHX = 0; CHX < ptl.x_ch; CHX++) {
					LD_MSG("%4d,", uiTestDatas[CHY][CHX]);
					report[CHY][CHX] = uiTestDatas[CHY][CHX];
					if (uiTestDatas[CHY][CHX] > max)
						max = uiTestDatas[CHY][CHX];
					if (uiTestDatas[CHY][CHX] < min)
						min = uiTestDatas[CHY][CHX];
				}
				LD_MSG("\n");
			}

			if (sctype) {
				len = ptl.x_ch + ptl.y_ch;
				row = ptl.x_ch;
				ret = viGetCDCData_3X(sctype, len, sc_offset, TEST_MODE_V3_Y_DRIVEN, row);
				if (ret < 0)
					break;

				LD_MSG("\nSelf X:");
				for (CHX = 0; CHX < ptl.x_ch; CHX++) {
					LD_MSG("%4d,", uiTestDatas[0][CHX]);
					report[ptl.y_ch][CHX] = uiTestDatas[0][CHX];
				}
				LD_MSG("\nSelf Y:");
				for (CHY = 0; CHY < ptl.y_ch; CHY++) {
					LD_MSG("%4d,", uiTestDatas[1+CHY/ptl.x_ch][CHY%ptl.x_ch]);
					report[CHY][ptl.x_ch] = uiTestDatas[1+CHY/ptl.x_ch][CHY%ptl.x_ch];
				}
				LD_MSG("\n");
			}

			if (ptl.key_mode == ILITEK_HW_KEY_MODE && keytype) {
				len = ptl.key_num;
				row = ptl.key_num;
				ret = viGetCDCData_3X(keytype, len, mc_offset,TEST_MODE_V3_KEY_DATA, row);
				if (ret < 0)
					break;

				LD_MSG("\nKey data:");
				for (CHX = 0; CHX < len; CHX++) {
					LD_MSG("%4d,", uiTestDatas[0][CHX]);
					reportkey[0][CHX] = uiTestDatas[0][CHX];
				}
				LD_MSG("\n");

			}

			viWriteCDCReport(inCounts+1, report, max, min, reportkey);
		}
	} while (0);

	ExitTestMode(100);
	return ret;
}

int viRunCDCType_6X(const char *type, int inFrames) {
	int ret = _SUCCESS;
	int inCounts = 0;
	unsigned int CHX = 0, CHY = 0;
	unsigned int d_len = 0;   //d_len: data total length.
	uint8_t mctype = 0, sctype = 0, mckeytype = 0, sckeytype = 0;
	int report[300][300];
	int reportkey[3][50]; //Lego support max key number is 50, data:50 self x:50 self y:1
	int max = 0, min = 0xFFFF;

	LD_MSG("Type:%s\n", type);
	if (!strcmp(type, "DAC_P")) {
		mctype = TEST_MODE_V6_MC_DAC_P;
		sctype = TEST_MODE_V6_SC_DAC_P;
		mckeytype = TEST_MODE_V6_KEY_MC_DAC_P;
		sckeytype = TEST_MODE_V6_KEY_SC_DAC_P;
	} else if (!strcmp(type, "DAC_N")) {
		mctype = TEST_MODE_V6_MC_DAC_N;
		sctype = TEST_MODE_V6_SC_DAC_N;
		mckeytype = TEST_MODE_V6_KEY_MC_DAC_N;
		sckeytype = TEST_MODE_V6_KEY_SC_DAC_N;
	} else if (!strcmp(type, "Raw_BK")) {
		mctype = TEST_MODE_V6_MC_RAW_BK;
		sctype = TEST_MODE_V6_SC_RAW_BK;
		mckeytype = TEST_MODE_V6_KEY_MC_RAW_BK;
		sckeytype = TEST_MODE_V6_KEY_SC_RAW_BK;
	} else if (!strcmp(type, "Raw_NBK")) {
		mctype = TEST_MODE_V6_MC_RAW_NBK;
		sctype = TEST_MODE_V6_SC_RAW_NBK;
		mckeytype = TEST_MODE_V6_KEY_MC_RAW_NBK;
		sckeytype = TEST_MODE_V6_KEY_SC_RAW_NBK;
	} else if (!strcmp(type, "BG")) {
		mctype = TEST_MODE_V6_MC_BG_BK;
		sctype = TEST_MODE_V6_SC_BG_BK;
		mckeytype = TEST_MODE_V6_KEY_MC_BG_BK;
		sckeytype = TEST_MODE_V6_KEY_SC_BG_BK;
	} else if (!strcmp(type, "SE")) {
		mctype = TEST_MODE_V6_MC_SE_BK;
		sctype = TEST_MODE_V6_SC_SE_BK;
		mckeytype = TEST_MODE_V6_KEY_MC_SE_BK;
		sckeytype = TEST_MODE_V6_KEY_SC_SE_BK;
	} else {
		LD_ERR("Type no support,%s\n", type);
		return _FAIL;
	}

	do {
		ret = ModeCtrl_V6(ENTER_SUSPEND_MODE, ENABLE_ENGINEER, 100);
		if (ret < 0)
			break;

		ret = ModeCtrl_V6(ENTER_TEST_MODE, DISABLE_ENGINEER, 100);
		if (ret < 0)
			break;

		LD_MSG("x_ch: %d, y_ch: %d\n", ptl.x_ch, ptl.y_ch);
		for (inCounts = 0; inCounts < inFrames; inCounts++) {
			ret = viInitRawData_6X(mctype, 10);
			if (ret < 0) {
				LD_ERR("Error! Init %x Failed!\n", mctype);
				break;
			}

			d_len = ptl.x_ch * ptl.y_ch;
			ret = viGetRawData_6X(d_len, NULL);
			if (ret < 0) {
				LD_ERR("Error! Get %x Failed!\n", mctype);
				break;
			}

			clrscr();
			LD_MSG("%s Datas: %d/%d Frames\n", type, inCounts, inFrames);
			for (CHY = 0; CHY < ptl.y_ch; CHY++) {
				LD_MSG("Y_%2dCH:", CHY);
				for (CHX = 0; CHX < ptl.x_ch; CHX++) {
					LD_MSG("%4d,", uiTestDatas[CHY][CHX]);
					report[CHY][CHX] = uiTestDatas[CHY][CHX];
					if (uiTestDatas[CHY][CHX] > max)
						max = uiTestDatas[CHY][CHX];
					if (uiTestDatas[CHY][CHX] < min)
						min = uiTestDatas[CHY][CHX];
				}
				LD_MSG("\n");
			}

			ret = viInitRawData_6X(sctype, 10);
			if (ret < 0) {
				LD_ERR("Error! Init %x Failed!\n", sctype);
				break;
			}

			d_len = ptl.x_ch + ptl.y_ch;
			ret = viGetRawData_6X(d_len, NULL);
			if (ret < 0) {
				LD_ERR("Error! Get %x Failed!\n", sctype);
				break;
			}

			LD_MSG("\nSelf X:");
			for (CHX = 0; CHX < ptl.x_ch; CHX++) {
				LD_MSG("%4d,", uiTestDatas[0][CHX]);
				report[ptl.y_ch][CHX] = uiTestDatas[0][CHX];
			}

			LD_MSG("\nSelf Y:");
			for (CHY = 0; CHY < ptl.y_ch; CHY++) {
				LD_MSG("%4d,", uiTestDatas[1+CHY/ptl.x_ch][CHY%ptl.x_ch]);
				report[CHY][ptl.x_ch] = uiTestDatas[1+CHY/ptl.x_ch][CHY%ptl.x_ch];
			}
			LD_MSG("\n");

			if (ptl.key_mode == ILITEK_HW_KEY_MODE) {
				ret = viInitRawData_6X(mckeytype, 10);
				if (ret < 0) {
					LD_ERR("Error! Init %x Failed!\n", mckeytype);
					break;
				}

				d_len = ptl.key_num;
				ret = viGetRawData_6X(d_len, NULL);
				if (ret < 0) {
					LD_ERR("Error! Get %x Failed!\n", mckeytype);
					break;
				}

				LD_MSG("\nKey data:");
				for (CHX = 0; CHX < d_len; CHX++) {
					LD_MSG("%4d,", uiTestDatas[0][CHX]);
					reportkey[0][CHX] = uiTestDatas[0][CHX];
				}
				ret = viInitRawData_6X(sckeytype, 10);
				if (ret < 0) {
					LD_ERR("Error! Init %x Failed!\n", sckeytype);
					break;
				}
				d_len = ptl.key_num + 1; //add 1 for self y ch
				ret = viGetRawData_6X(d_len, NULL);
				if (ret < 0) {
					LD_ERR("Error! Get %x Failed!\n", sckeytype);
					break;
				}

				LD_MSG("\nSelf X:");
				for (CHX = 0; CHX < ptl.key_num; CHX++) {
					LD_MSG("%4d,", uiTestDatas[0][CHX]);
					reportkey[1][CHX] = uiTestDatas[0][CHX];
				}
				LD_MSG(" Self Y: %4d\n", uiTestDatas[0][ptl.key_num]);
				reportkey[2][0] = uiTestDatas[0][ptl.key_num];
			}

			viWriteCDCReport(inCounts+1, report, max, min, reportkey);
		}
	} while (0);

	ModeCtrl_V6(ENTER_NORMAL_MODE, DISABLE_ENGINEER, 100);
	return ret;
}

int viRunCDCData_3X(int inFrames)
{
	int ret = _SUCCESS;

	ret = viRunCDCType_3X("Raw", inFrames);
	return ret;
}

int viRunCDCData_6X(int inFrames)
{
	int ret = _SUCCESS;

	ret = viRunCDCType_6X("Raw_NBK", inFrames);
	return ret;
}
int viRunBGMinusCDCData_3X(int inFrames)
{
	int ret = _SUCCESS;

	ret = viRunCDCType_3X("SE", inFrames);
	return ret;
}

int viRunBGMinusCDCData_6X(int inFrames)
{
	int ret = _SUCCESS;

	ret = viRunCDCType_6X("SE", inFrames);
	return ret;
}

int viInitRawData_3X(unsigned char ucCMDInit, unsigned char ucCMDMode)
{
	int ret = 0;
	uint8_t Wbuff[64] = {0};

	Wbuff[0] = 0xF3;
	Wbuff[1] = ucCMDInit;
	Wbuff[2] = 0x00;
	Wbuff[3] = ucCMDMode;
	ret = TransferData(Wbuff, 4, NULL, 0, 1000);
	if (ret < 0)
		return _FAIL;
	usleep(200000);
	ret = CheckBusy(100, 50, NO_NEED);
	return ret;
}

int viInitRawData_6X(unsigned char cmd, int delay_count)
{
	uint8_t Wbuff[64] = {0};

	Wbuff[0] = ILITEK_TP_CMD_SET_CDC_INITOAL_V6;
	Wbuff[1] = cmd;

	if (write_and_wait_ack(Wbuff, 2, 5000, 1000, delay_count,
			       SYSTEM_BUSY | INITIAL_BUSY) < 0)
		return _FAIL;

	return _SUCCESS;
}

int viInitRawData_3Para_3X(unsigned char ucCMDInit, unsigned char ucCMDMode, unsigned char ucCMDCounts)
{
	int ret = 0;
	uint8_t Wbuff[64] = {0}, Rbuff[64] = {0};

	Wbuff[0] = 0xF3;
	Wbuff[1] = ucCMDInit;
	Wbuff[2] = 0x00;
	Wbuff[3] = ucCMDMode;
	Wbuff[4] = ucCMDCounts;
	ret = TransferData(Wbuff, 5, Rbuff, 0, 1000);

	usleep(1000000);
	if (ret != _FAIL)
	{
		ret = CheckBusy(300, 10, NO_NEED);
	}
	return ret;
}

int viGetRawData_3X(unsigned char ucCMD, unsigned char unStyle,
		    int d_len, unsigned char ucDataFormat,
		    unsigned int rowLen, uint8_t *buf)
{
	int rlen, wlen = 1;
	int idx = 0;
	int need, got = 0;
	int ret = 0;
	short int *p_s16Data;
	char *p_c8Data;
	uint8_t Wbuff[64] = {0}, Rbuff[64] = {0};
	int copy_len = 0, offset = 0;
	bool writen = false;
	char tmpbuf[64];

	need = (ucDataFormat == _DataFormat_16_Bit_) ? d_len * 2 : d_len;

	Wbuff[0] = ucCMD;

	do {
		if (inConnectStyle == _ConnectStyle_I2C_)
			rlen = get_min(need + 2, 32);
		else
			rlen = get_min(need + 2, 60);

		if (unStyle == _FastMode_)
			wlen = (writen == false) ? 1 : 0;

		if (inConnectStyle == _ConnectStyle_I2CHID_ &&
		    unStyle == _FastMode_ && writen == true) {
			ret = read_report(tmpbuf, 64, 1000);
			memcpy(Rbuff, tmpbuf + 4, rlen);
		} else {
			ret = TransferData(Wbuff, wlen, Rbuff, rlen, 1000);
		}
		if (ret < 0) {
			LD_ERR("Error! Read Data error, wlen: %d\n", wlen);
			return _FAIL;
		}

		writen = true;

		if (buf) {
			copy_len = rlen - 2;
			memcpy(buf + offset, Rbuff + 2, copy_len);
			offset += copy_len;
			continue;
		}

		for (idx = 0; idx < rlen - 2 && got < d_len; got++) {
			if (ucDataFormat == _DataFormat_16_Bit_) {
				p_s16Data = (short int *)(&Rbuff[2]);
				uiTestDatas[got / rowLen][got % rowLen] =
					p_s16Data[idx / 2];
				idx += 2;
				continue;
 			}

			if (ucSignedDatas == 1) {
				p_c8Data = (char *)(&Rbuff[2]);
				uiTestDatas[got / rowLen][got % rowLen] =
					p_c8Data[idx];
			} else {
				uiTestDatas[got / rowLen][got % rowLen] =
					Rbuff[idx + 2];
			}
			idx++;
		}
	} while ((need -= (rlen - 2)) > 0);

	return 0;
}

int viGetRawData_6X(unsigned int d_len, uint8_t *buf)
{
	unsigned int rlen = 0;
	unsigned int idx = 0;
	int need = d_len;
	unsigned int got = 0, t_len = RAW_DATA_TRANSGER_V6_LENGTH;
	int ret = 0;
	uint8_t Wbuff[64] = {0}, Rbuff[4096];
	unsigned int header = 6;
	int start = 5;
	int id_len = 0;
	int copy_len = 0;

	if (inConnectStyle==_ConnectStyle_USB_ ||
	    inConnectStyle==_ConnectStyle_I2CHID_)
		id_len = 1;

	header = header + id_len;
	start = start + id_len;
	SetDataLength_V6(t_len);

	if (inConnectStyle == _ConnectStyle_I2C_)
		t_len = (d_len * 2 + header < t_len) ? d_len * 2 : t_len;

	do {
		rlen = t_len + header;

		Wbuff[0] = ILITEK_TP_CMD_GET_CDC_DATA_V6;

		if (write_and_wait_ack(Wbuff, 1, 10000, 300, 10, NO_NEED) < 0)
			return _FAIL;

		if (inConnectStyle == _ConnectStyle_I2C_)
			ret = TransferData(NULL, 0, Rbuff, rlen, 1000);
		else
			ret = TransferData(NULL, 0, Rbuff, t_len, 1000);

		if (ret < 0) {
			LD_ERR("Error! Read Data error \n");
			return ret;
		}

		got = get_le16(Rbuff + id_len + 1) +
		      get_le16(Rbuff + id_len + 3) * ptl.x_ch;

		if (got + need > d_len) {
			LD_ERR("FW get lenght error, got: %u, need: %d, d_len: %u\n",
				got, need, d_len);
			break;
		}

		if (buf) {
			copy_len = get_min(2 * (d_len - got), t_len);
			memcpy(buf + 2 * got, Rbuff + start, copy_len);
			got += (copy_len / 2);
			continue;
	     	}

		for (idx = start; idx < rlen - 1 && got < d_len;
		     idx += 2, got++) {
			uiTestDatas[got / ptl.x_ch][got % ptl.x_ch] =
				get_le16(Rbuff + idx);
		}
	} while ((need = d_len - got) > 0);

	return 0;
}

int viRunBGData(int inFrames)
{
	int ret = _SUCCESS;
	if(viGetPanelInfor() == _FAIL)
		return _FAIL;
	ret = viCreateCDCReportFile("BG");
	if (inProtocolStyle == _Protocol_V3_)
	{
		ret = viRunBGData_3X(inFrames);
	}
	else if (inProtocolStyle == _Protocol_V6_)
	{
		ret = viRunBGData_6X(inFrames);
	}
	if(result_file != NULL)
		fclose(result_file);
	return ret;
}
int viRunBGMinusCDCData(int inFrames)
{
	int ret = _SUCCESS;

	if(viGetPanelInfor() == _FAIL)
		return _FAIL;
	viCreateCDCReportFile("SE");
	if (inProtocolStyle == _Protocol_V3_)
	{
		ret = viRunBGMinusCDCData_3X(inFrames);
	}
	else if (inProtocolStyle == _Protocol_V6_)
	{
		ret = viRunBGMinusCDCData_6X(inFrames);
	}
	if(result_file != NULL)
		fclose(result_file);
	return ret;
}

int viRunCDCData(int inFrames)
{
	int ret = _SUCCESS;

	if(viGetPanelInfor() == _FAIL)
		return _FAIL;
	viCreateCDCReportFile("Raw");
	if (inProtocolStyle == _Protocol_V3_)
	{
		ret = viRunCDCData_3X(inFrames);
	}
	else if (inProtocolStyle == _Protocol_V6_)
	{
		ret = viRunCDCData_6X(inFrames);
	}
	if(result_file != NULL)
		fclose(result_file);
	return ret;
}

int viRunCDCType(char *argv[]) {
	int ret = _SUCCESS;
	int inFrames = atoi(argv[7]);
	char *type = argv[6];
	if (viGetPanelInfor() == _FAIL)
		return _FAIL;

	if (ICMode == 0x55)
		return _FAIL;

	viCreateCDCReportFile(type);
	if (inProtocolStyle == _Protocol_V3_)
	{
		ret = viRunCDCType_3X(type, inFrames);
	}
	else if (inProtocolStyle == _Protocol_V6_)
	{
		ret = viRunCDCType_6X(type, inFrames);
	}
	if(result_file != NULL)
		fclose(result_file);
	return ret;
}

int viCreateCDCReportFile(const char *type)
{
	int ret = _SUCCESS;
	time_t rawtime;
	char timebuf[60];
	struct tm *timeinfo;
	char cdc_type[256];
	char interface[4];
	int data_format = 16;
	char result_file_name[256] = {0};

	time(&rawtime);
	timeinfo = localtime (&rawtime);
	if (access("Record",0) == _FAIL) {
		if(mkdir("Record",0777)) {
			LD_ERR("creat Record file bag failed!!!");
			return _FAIL;
		}
	}
	strftime(timebuf,60,"%Y_%m_%d_%I_%M_%S",timeinfo);
	//LD_MSG(" %s %s\n", ST.LogPath, tmp);
	sprintf(result_file_name,"Record/%s.csv", timebuf);
	LD_MSG("Record file:%s\n", result_file_name);
	result_file = fopen(result_file_name, "w");
	if(strcmp(type, "DAC_P") == 0) {
		strcpy(cdc_type, "TEST_MODE_DAC_P");
	} else if(strcmp(type, "DAC_N") == 0) {
		strcpy(cdc_type, "TEST_MODE_DAC_N");
	} else if(strcmp(type, "Raw_BK") == 0) {
		strcpy(cdc_type, "TEST_MODE_RAW_BK");
	} else if(strcmp(type, "Raw_NBK") == 0) {
		strcpy(cdc_type, "TEST_MODE_RAW_NBK");
	} else if(strcmp(type, "Raw") == 0) {
		strcpy(cdc_type, "TEST_MODE_RAW_BK");
	}
	else if(strcmp(type, "BG") == 0) {
		if(inProtocolStyle == _Protocol_V6_)
			strcpy(cdc_type, "TEST_MODE_BG_BK");
		else
			strcpy(cdc_type, "TEST_MODE_BG");
	} else if(strcmp(type, "SE") == 0) {
		if(inProtocolStyle == _Protocol_V6_)
			strcpy(cdc_type, "TEST_MODE_SE_BK");
		else
			strcpy(cdc_type, "TEST_MODE_SE");
	}
	if(inConnectStyle == _ConnectStyle_USB_)
		strcpy(interface, "HID");
	else
		strcpy(interface, "I2C");
	fprintf(result_file, "%s\n", TOOL_VERSION);
	fprintf(result_file, "===================================================================\n");
	fprintf(result_file, "Sensing Method  =%s\n", cdc_type);
	fprintf(result_file, "IC Type         =ILI%x\n", ptl.ic);
	if (inProtocolStyle == _Protocol_V3_)
		fprintf(result_file, "Protocol Type   =V3_%s\n", interface);
	else if (inProtocolStyle == _Protocol_V6_)
		fprintf(result_file, "Protocol Type   =V6_%s\n", interface);
	else
		fprintf(result_file, "Protocol Type   =V3_%s\n", interface);
	fprintf(result_file, "Data Format     =%d_Bits\n", data_format);
	fprintf(result_file, "CDC Type        =CDC\n");
	fprintf(result_file, "MCU Type        =%x\n", ptl.ic);
	fprintf(result_file, "X Channel       =%d\n", ptl.x_ch);
	fprintf(result_file, "Y Channel       =%d\n", ptl.y_ch);
	fprintf(result_file, "Over 8 Bits     =True\n");
	if(ptl.key_mode == ILITEK_HW_KEY_MODE) {
		fprintf(result_file, "Support HW Key  =True\n");
		fprintf(result_file, "Key Mode        =HW_Key_2\n");
		fprintf(result_file, "Key Amounts     =%d\n", ptl.key_num);
	}
	else
		fprintf(result_file, "Support HW Key  =False\n");
	return ret;
}

void viWriteCDCReport(int count, int report[][300], int max, int min, int report_key[][50]) {
	unsigned int CHX = 0, CHY = 0;

	fprintf(result_file, "===================================================================\n");
	fprintf(result_file, "No                    = %d\n", count);
	fprintf(result_file, "Max                   = %d\n", max);
	fprintf(result_file, "Min                   = %d\n", min);
	if(ptl.key_mode == ILITEK_HW_KEY_MODE) {
		fprintf(result_file, "Key Data\n       ,");
		for(CHX = 1; CHX <= ptl.key_num; CHX++)
			fprintf(result_file, "    Key%d,", CHX);
		fprintf(result_file, "    \nKeyDrv1,");
		for(CHX = 0; CHX < ptl.key_num; CHX++)
			fprintf(result_file, "   %5d,", report_key[0][CHX]);
		if (inProtocolStyle == _Protocol_V6_) {
			fprintf(result_file, "   %5d,\n       ,", report_key[2][0]);
			for(CHX = 0; CHX < ptl.key_num; CHX++)
				fprintf(result_file, "   %5d,", report_key[1][CHX]);
			fprintf(result_file, "    \n");
		}
	}
	fprintf(result_file, "Driven Data\n    ,");
	for(CHX = 1; CHX <= ptl.x_ch; CHX++)
		fprintf(result_file, " X%03d,", CHX);
	fprintf(result_file, "    \n");
	for(CHY = 0; CHY <= ptl.y_ch; CHY++) {
		if(CHY != ptl.y_ch)
			fprintf(result_file, "Y%03d,", CHY + 1);
		else
			fprintf(result_file, "    ,");
		for(CHX = 0; CHX <= ptl.x_ch; CHX++) {
			if(CHY == ptl.y_ch && CHX == ptl.x_ch)
				break;
			fprintf(result_file, "%5d,", report[CHY][CHX]);
		}
		fprintf(result_file, "\n");
	}
}

int viGetCDCData_6X(unsigned char type, unsigned int len) {
	int ret = _FAIL;
	if (viInitRawData_6X(type, 10) != _FAIL)
	{
		if (viGetRawData_6X(len, NULL) != _FAIL)
		{
			ret = _SUCCESS;
		}
		else
		{
			LD_ERR("Error! Get RawData Failed!\n");
		}
	}
	else
	{
		LD_ERR("Error! Init RawData Failed!\n");
	}
	return ret;
}

int viGetCDCData_3X(unsigned char type, unsigned int len, uint8_t offset, uint8_t driven, uint8_t row) {
	int ret = _SUCCESS;
	uint8_t u8DataInfor;

	LD_MSG("len=%d\n", len);
	ret = RawDataInfor();
	u8DataInfor = (ret & (1 << offset)) ? _DataFormat_16_Bit_: _DataFormat_8_Bit_;
	if (viInitRawData_3X(type, driven) != _FAIL)
	{
		if (viGetRawData_3X(driven, _FastMode_, len, u8DataInfor, row, NULL) != _FAIL)
		{
			ret = _SUCCESS;
		}
		else
		{
			LD_ERR("Error! Get RawData Failed!\n");
			ret = _FAIL;
		}
	}
	else
	{
		LD_ERR("Error! Init RawData Failed!\n");
		ret = _FAIL;
	}
	return ret;
}

#endif

