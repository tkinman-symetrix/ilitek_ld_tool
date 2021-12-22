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

#ifndef _ILITEK_SENSORTEST_C_
#define _ILITEK_SENSORTEST_C_

/* Includes of headers ------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../ILITek_CMDDefine.h"
#include "../ILITek_Device.h"
#include "../ILITek_Protocol.h"
#include "ILITek_SensorTest.h"
#include "ILITek_RawData.h"
#include "ILITek_Upgrade.h"
#include "../ILITek_Main.h"

static __time_t basetime;
static struct timeval tv;
static struct timezone tz;

/* Private define ------------------------------------------------------------*/
void InitialSensorTestV3Parameter()
{
	ST.LogPath = NULL;
	ST.UseNewFlow = -1;
	ST.Open_Threshold = 20;
	ST.Open_RX_Delta_Threshold = 13;
	ST.Open_TX_Delta_Threshold = 13;
	ST.Open_DCRangeMax = 113;
	ST.Open_DCRangeMin = 87;

	ST.Uniformity.uiBD_Top_Ratio = 12;
	ST.Uniformity.uiBD_Bottom_Ratio = 12;
	ST.Uniformity.uiBD_L_Ratio = 12;
	ST.Uniformity.uiBD_R_Ratio = 12;
	ST.Uniformity.uiVA_Ratio_X_diff = 13;
	ST.Uniformity.uiVA_Ratio_Y_diff = 13;
	ST.Uniformity.uiBD_VA_L_Ratio_Max = 115;
	ST.Uniformity.uiBD_VA_L_Ratio_Min = 85;
	ST.Uniformity.uiBD_VA_R_Ratio_Max = 115;
	ST.Uniformity.uiBD_VA_R_Ratio_Min = 85;
	ST.Uniformity.uiBD_VA_Top_Ratio_Max = 115;
	ST.Uniformity.uiBD_VA_Top_Ratio_Min = 85;
	ST.Uniformity.uiBD_VA_Bottom_Ratio_Max = 115;
	ST.Uniformity.uiBD_VA_Bottom_Ratio_Min = 85;
	ST.Uniformity.uiPanelLeftTopULimit = 113;
	ST.Uniformity.uiPanelLeftTopLLimit = 87;
	ST.Uniformity.uiPanelLeftBottomULimit = 113;
	ST.Uniformity.uiPanelLeftBottomLLimit = 87;
	ST.Uniformity.uiPanelRightTopULimit = 113;
	ST.Uniformity.uiPanelRightTopLLimit = 87;
	ST.Uniformity.uiPanelRightBottomULimit = 113;
	ST.Uniformity.uiPanelRightBottomLLimit = 87;

	ST.Short.Threshold = -1;

	ST.Open_Threshold = -1;
	ST.Open_RX_Delta_Threshold = -1;
	ST.Open_TX_Delta_Threshold = -1;
	ST.Open_RX_Continue_Fail_Tolerance = -1;
	ST.Open_TX_Continue_Fail_Tolerance = -1;
	ST.Open_TX_Aver_Diff = -1;

	ST.Self_Maximum = -1;
	ST.Self_Minimum = -1;
	ST.Self_P2P = -1;
	ST.Self_P2P_Edge = -1;
	ST.Self_Frame_Count = -1;

	ST.DAC_SC_P_Maximum = -1;
	ST.DAC_SC_P_Minimum = -1;
	ST.DAC_SC_N_Maximum = -1;
	ST.DAC_SC_N_Minimum = -1;
	ST.DAC_MC_P_Maximum = -1;
	ST.DAC_MC_P_Minimum = -1;
	ST.DAC_MC_N_Maximum = -1;
	ST.DAC_MC_N_Minimum = -1;

	ST.AllNode_Maximum = -1;
	ST.AllNode_Minimum = -1;
	ST.AllNode_Delta_Threshold = -1;
	ST.AllNode_Panel_Tolerance = -1;
	ST.AllNode_TX_Tolerance = -1;

}

void InitialSensorTestV6Parameter()
{
	ST.LogPath = NULL;
	//Short test
	ST.Short.Threshold = _FAIL;
	ST.Short.FrameCount = _FAIL;
	ST.Short.dump1 = _FAIL;
	ST.Short.dump2 = _FAIL;
	ST.Short.posidleL = _FAIL;
	ST.Short.posidleH = _FAIL;
	ST.Short.vref_v = _FAIL;
	//Open test
	ST.Open_Threshold = _FAIL;
	ST.Open_FrameCount = _FAIL;
	ST.Open_TX_Aver_Diff = _FAIL;
	ST.Open_RX_Delta_Threshold = _FAIL;
	ST.Open_RX_Continue_Fail_Tolerance = _FAIL;
	//Uniformity
	ST.Uniformity.FrameCount = _FAIL;
	ST.Uniformity.Max_Threshold = _FAIL;
	ST.Uniformity.Up_FailCount = _FAIL;
	ST.Uniformity.Min_Threshold = _FAIL;
	ST.Uniformity.Low_FailCount = _FAIL;
	ST.Uniformity.Win1_Threshold = _FAIL;
	ST.Uniformity.Win1_FailCount = _FAIL;
	ST.Uniformity.Win2_Threshold = _FAIL;
	ST.Uniformity.Win2_FailCount = _FAIL;
}
/* Private typedef -----------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/*8738=65535*4/30(dump)    216=60(cfb)*3.6V */

#define IMPEDANCE_MACRO(fout, dump1, dump2, vref) (( vref * 8738 * abs(dump2-dump1)) /(216 * fout))
/* Private variables ---------------------------------------------------------*/
SensorTest_Criteria ST;
char g_szConfigPath[INI_MAX_PATH];
unsigned char IniPath[256] = {0};
struct tm *timeinfo;
char fileName[256];
static char tmpstr[1024];
short int NewVerFlag = 0;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
int GetIniKeyInt(const char *title, const char *key, char *filename)
{
	int temp = 0;
	char *tmpbuffer = GetIniKeyString(title, key, filename);

	if (!tmpbuffer) {
		LD_ERR("Get [%s] %s no find value\n", title, key);
		return _FAIL;
	} else {
		//return atoi(tmpbuffer);
		if(strncmp(tmpbuffer, "0x", 2) == 0 || strncmp(tmpbuffer, "0X", 2) == 0)
			sscanf(tmpbuffer,"%x", &temp);
		else
			sscanf(tmpbuffer,"%d", &temp);
		LD_MSG("[%s] %s=%d\n", title, key, temp);
		return temp;
	}
}

char *GetIniKeyString(const char *title, const char *key, char *filename)
{
	FILE *fp;
	unsigned char *szLine;
	int rtnval;
	int i = 0;
	int flag = 0;
	char *tmp;
	char *splitResult;
	unsigned short isUni;
	bool MultiByteEnable = false;
	bool FindDataByte = false;
	int buteCount = 0;
	size_t ret;

	szLine = (unsigned char *)calloc(8192, sizeof(unsigned char));
	if (!szLine)
		return NULL;

	if ((fp = fopen(filename, ("rb"))) == NULL) {
		LD_ERR("have no such file :%s\n", filename);
		goto err_free;
	}

	ret = fread(&(isUni), 1, 2, fp);
	if (ret && isUni == (unsigned short)0xBBEF) {
		rtnval = fgetc(fp);
	} else if (ret && (isUni == (unsigned short)0xFFFE ||
		   isUni == (unsigned short)0xFEFF ||
		   isUni == (unsigned short)0xFEBB)) {
		MultiByteEnable = true;
	} else {
		szLine[1] = isUni >> 8;
		szLine[0] = isUni;
		i = 2;
	}

	while (!feof(fp))
	{
		rtnval = fgetc(fp);
		buteCount++;
		if (MultiByteEnable)
		{
			if (rtnval != 0x00)
			{
				FindDataByte = true;
			}
			else
				continue;
		}
		if (rtnval == EOF)
		{
			break;
		}
		else
		{
			szLine[i++] = rtnval;
			if (FindDataByte == true)
			{
				FindDataByte = false;
			}
		}
		if (rtnval == '\n')
		{
#ifndef WIN32
			i--;
#endif
			szLine[--i] = '\0';
			i = 0;
			tmp = strchr((char *)szLine, (int)'=');
			if ((tmp != NULL) && (flag == 1))
			{
				splitResult = strtok((char *)szLine,"=");
				//if (strstr(szLine, key) != NULL)
				if (splitResult != NULL && strstr(splitResult, key) != NULL)
				{
					if ('#' == szLine[0])
					{
					}
					else if ('/' == szLine[0] && '/' == szLine[1])
					{
					}
					else
					{
						strcpy(tmpstr, tmp + 1);
						free(szLine);
						fclose(fp);
						return tmpstr;
					}
				}
			}
			else
			{
				strcpy(tmpstr, "[");
				strcat(tmpstr, title);
				strcat(tmpstr, "]");
				if (strncmp(tmpstr, (char *)szLine, strlen(tmpstr)) == 0)
				{
					flag = 1;
				}
			}
		}
	}

	fclose(fp);
err_free:
	free(szLine);
	return NULL;
}

void check_use_default_set(void) {
	//uniformity
	if (ST.Uniformity.Max_Threshold < 0)
		ST.Uniformity.Max_Threshold = _SensorTest_Uniformaity_Max_Threshold_V6_;
	if (ST.Uniformity.Min_Threshold < 0)
		ST.Uniformity.Min_Threshold = _SensorTest_Uniformaity_Min_Threshold_V6_;
	if (ST.Uniformity.FrameCount < 0)
		ST.Uniformity.FrameCount =    _SensorTest_Uniformity_FrameCount_V6_;
	if (ST.Uniformity.Win1_Threshold < 0)
		ST.Uniformity.Win1_Threshold =_SensorTest_Uniformity_Win1_Threshold_V6_;
	if (ST.Uniformity.Win1_FailCount < 0)
		ST.Uniformity.Win1_FailCount =_SensorTest_Uniformity_Win1_FailCount_V6_;
	if (ST.Uniformity.Win2_Threshold < 0)
		ST.Uniformity.Win2_Threshold =_SensorTest_Uniformity_Win2_Threshold_V6_;
	if (ST.Uniformity.Win2_FailCount < 0)
		ST.Uniformity.Win2_FailCount =_SensorTest_Uniformity_Win2_FailCount_V6_;
	if (ST.Uniformity.Up_FailCount < 0)
		ST.Uniformity.Up_FailCount = _SensorTest_Uniformity_RawData_Tolerance;

	//open
	if (ST.Open_Threshold < 0)
		ST.Open_Threshold = _SensorTest_Open_Threshold_V6_;
	if (ST.Open_FrameCount < 0)
		ST.Open_FrameCount = _SensorTest_Open_FrameCount_V6_;
	if (ST.Open_RX_Continue_Fail_Tolerance < 0)
		ST.Open_RX_Continue_Fail_Tolerance = _SensorTestOpenRXTolerance_;
	//short
	if (ST.Short.Threshold < 0)
		ST.Short.Threshold = _SensorTest_Short_Thresshold_V6_;
	if (ST.Short.FrameCount < 0)
		ST.Short.FrameCount = _SensorTest_Short_FrameCount_V6_;
	if (ST.Short.dump1 < 0)
		ST.Short.dump1 = _SensorTest_Short_Dump1_V6_;
	if (ST.Short.dump2 < 0)
		ST.Short.dump2 = _SensorTest_Short_Dump2_V6_;
	if (ST.Short.vref_v < 0)
		ST.Short.vref_v = _SensorTest_Short_Vref_V6_;
	if (ST.Short.posidleL < 0)
		ST.Short.posidleL = _SensorTest_Short_posidleL_V6_;
	if (ST.Short.posidleH < 0)
		ST.Short.posidleH = _SensorTest_Short_posidleH_V6_;
}

bool check_ini_section(const char *title, char *filename)
{
	FILE *fp;
	char szLine[1024];
	int rtnval;
	int i = 0;
	bool status = false;
	unsigned short isUni;
	bool MultiByteEnable = false;
	bool FindDataByte = false;
	int buteCount = 0;
	size_t ret;

	if ((fp = fopen(filename, ("rb"))) == NULL)
	{
		LD_ERR("have no such file :%s\n",filename);
		return false;
	}
	ret = fread(&(isUni), 1, 2, fp);

	if (ret && isUni == (unsigned short)0xBBEF) {
		rtnval = fgetc(fp);
	} else if (ret && (isUni == (unsigned short)0xFFFE ||
				isUni == (unsigned short)0xFEFF || isUni == (unsigned short)0xFEBB)) {
		MultiByteEnable = true;
	} else {
		szLine[1] = isUni >> 8;
		szLine[0] = isUni;
		i = 2;
	}

	while (!feof(fp))
	{
		rtnval = fgetc(fp);
		buteCount++;

		if (MultiByteEnable)
		{
			if (rtnval != 0x00)
			{
				FindDataByte = true;
			}
			else
				continue;
		}
		if (rtnval == EOF)
		{
			break;
		}
		else
		{
			szLine[i++] = rtnval;
			if (FindDataByte == true)
			{
				FindDataByte = false;
			}
		}
		if (rtnval == '\n')
		{
#ifndef WIN32
			i--;
#endif
			szLine[--i] = '\0';
			i = 0;
			//tmp = strchr(szLine, '=');
			{
				strcpy(tmpstr, "[");
				strcat(tmpstr, title);
				strcat(tmpstr, "]");
				if (strncmp(tmpstr, szLine, strlen(tmpstr)) == 0)
				{
					LD_MSG("dat file exist[%s]\n", title);
					status = true;
				}
			}
		}
	}
	fclose(fp);
	return status;
}

char *GetIniSectionString(const char *title, char *tmp_str, char *filename)
{
	FILE *fp;
	//char szLine[1024];
	uint8_t *szLine;
	int rtnval;
	int i = 0;
	int flag = 0;
	char *splitResult;

	if ((fp = fopen(filename, "r")) == NULL)
		LD_ERR("have no such file :%s\n",filename);
	szLine = (uint8_t *)malloc(4096);
	while (!feof(fp))
	{
		rtnval = fgetc(fp);
		szLine[i++] = rtnval;
		if (szLine[0] == 0xEF && szLine[1] == 0xBB && szLine[2] == 0xBF)
			i = 0;
		if (rtnval == '\n')
		{
#ifndef WIN32
			i--;
#endif
			szLine[--i] = '\0';
			i = 0;
			if (flag == 1) {
				splitResult = strtok((char *)szLine,"\n");
				//if (strstr(szLine, key) != NULL)
				if (splitResult != NULL)
				{
					if ('#' == szLine[0])
					{
					}
					else if ('/' == szLine[0] && '/' == szLine[1])
					{
					}
					else if ('[' == szLine[0])
					{
						fclose(fp);
						return tmp_str;
					}
					else
					{
						strcat(tmp_str, (const char *)szLine);
						//LD_MSG("%s\n", tmp_str);
					}
				}
			}
			else
			{
				strcpy(tmpstr, "[");
				strcat(tmpstr, title);
				strcat(tmpstr, "]");
				if (strncmp(tmpstr, (const char *)szLine, strlen(tmpstr)) == 0)
					flag = 1;
			}
		}
	}
	free(szLine);
	fclose(fp);
	return NULL;
}

void vfReadBenchMarkValue(const char *title, int BenchMarkBuf[][_MaxChanelNum_], char *filename)
{
	unsigned char ucloopCh_X;
	unsigned char ucloopCh_Y;
	unsigned char ucIndex;
	unsigned int iCount;
	char key[10];
	char *strValue;
	char strData[10];
	for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch; ucloopCh_Y++)
	{
		sprintf(key, "%d", ucloopCh_Y);
		strValue = GetIniKeyString(title, key, filename);
		if (strcmp("", strValue) == 0)
			return;
		ucloopCh_X = 0;
		ucIndex = 0;
		memset(strData,0,sizeof(strData));
		for(iCount = 0; iCount < strlen(strValue); iCount++)
		{
			if(strValue[iCount] == ',')
			{
				strData[ucIndex] = '\0';
				BenchMarkBuf[ucloopCh_X][ucloopCh_Y] = atoi(strData);
				ucIndex = 0;
				memset(strData,0,sizeof(strData));
				ucloopCh_X++;
				if(ucloopCh_X >= ptl.x_ch)
					break;
			}
			else
			{
				strData[ucIndex] = strValue[iCount];
				ucIndex++;
				if(ucIndex >= 10)
					return;
			}
		}
	}
	LD_MSG("\nBenchMark_[%s]\n", title);
	for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch; ucloopCh_Y++)
	{
		for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch; ucloopCh_X++)
		{
			LD_MSG("%5d,", BenchMarkBuf[ucloopCh_X][ucloopCh_Y]);
		}
		LD_MSG("\n");
	}
	strValue = NULL;
}

void vfReadBenchMarkValue_V6(const char *title, char *filename, SensorTest_BenBenchmark_Node **data, int x, int y)
{
	unsigned char ucloopCh_X;
	unsigned char ucloopCh_Y;
	int iCount;
	char *strValue;
	int filesize = get_file_size(filename);
	char *strData;

	strValue = (char *)calloc(filesize, sizeof(char));
	GetIniSectionString(title, strValue, filename);
	strData = strtok(strValue, ";");
	iCount = 0;
	while (strData != NULL)
	{
		sscanf(strData,"%d,%d,%d,%d", &data[iCount%x][iCount/x].ini.data,
				&data[iCount%x][iCount/x].ini.max,
				&data[iCount%x][iCount/x].ini.min,
				&data[iCount%x][iCount/x].ini.type);
		// LD_MSG("[%d]%s,%d,%d,%d,%d,%d\n", iCount, strData, data[iCount%x][iCount/x].ini.data, data[iCount%x][iCount/x].ini.data,data[iCount%x][iCount/x].ini.max,
		//                                 data[iCount%x][iCount/x].ini.min, data[iCount%x][iCount/x].ini.type);
		iCount++;
		strData = strtok( NULL, ";");
	}

	LD_MSG("\n[%s]\nData ,", title);
	for (ucloopCh_X = 0; ucloopCh_X < x; ucloopCh_X++)
		LD_MSG("X_%3d,", ucloopCh_X);
	for (ucloopCh_Y = 0; ucloopCh_Y < y; ucloopCh_Y++) {
		LD_MSG("\nY_%3d,", ucloopCh_Y);
		for (ucloopCh_X = 0; ucloopCh_X < x; ucloopCh_X++) {
			LD_MSG("%5d,", data[ucloopCh_X][ucloopCh_Y].ini.data);
		}
	}
	LD_MSG("\n[%s]\n Max ,", title);
	for (ucloopCh_X = 0; ucloopCh_X < x; ucloopCh_X++)
		LD_MSG("X_%3d,", ucloopCh_X);
	for (ucloopCh_Y = 0; ucloopCh_Y < y; ucloopCh_Y++) {
		LD_MSG("\nY_%3d,", ucloopCh_Y);
		for (ucloopCh_X = 0; ucloopCh_X < x; ucloopCh_X++) {
			LD_MSG("%5d,", data[ucloopCh_X][ucloopCh_Y].ini.max);
		}
	}
	LD_MSG("\n[%s]\n Min ,", title);
	for (ucloopCh_X = 0; ucloopCh_X < x; ucloopCh_X++)
		LD_MSG("X_%3d,", ucloopCh_X);
	for (ucloopCh_Y = 0; ucloopCh_Y < y; ucloopCh_Y++) {
		LD_MSG("\nY_%3d,", ucloopCh_Y);
		for (ucloopCh_X = 0; ucloopCh_X < x; ucloopCh_X++) {
			LD_MSG("%5d,", data[ucloopCh_X][ucloopCh_Y].ini.min);
		}
	}
	LD_MSG("\n");
	free(strValue);
}

void vfSaveFWVerTestLog_V3(FILE *fp)
{
	if ((ucSensorTestResult & FWVERTION_TEST) == FWVERTION_TEST)
		fprintf(fp, "[FW_Verify]                      ,NG ,\n\n");
	else
		fprintf(fp, "[FW_Verify]                      ,OK ,\n\n");

	fprintf(fp, "      (Spec.)                    ,\n");
	fprintf(fp, "      Path                       ,%s,\n", ST.hexfile);
	fprintf(fp, "      FW_Ver                     ,%02X.%02X.%02X.%02X.%02X.%02X.%02X.%02X,\n",
		FWVersion[0], FWVersion[1], FWVersion[2], FWVersion[3],
		FWVersion[4], FWVersion[5], FWVersion[6], FWVersion[7]);
	fprintf(fp, " \n\n\n\n");
}

void vfSaveSelfTestLog(FILE *fp)
{
	unsigned char ucloopCh_X;
	unsigned char ucloopCh_Y;

	if ((ucSensorTestResult & SELF_TEST) == SELF_TEST)
		fprintf(fp, "[Self_Cap_Test]           ,NG ,\n");
	else
		fprintf(fp, "[Self_Cap_Test]           ,OK ,\n");

	ST.Self_Maximum = GetIniKeyInt("TestItem", "SelfCapTest_Maximum", g_szConfigPath);
	ST.Self_Minimum = GetIniKeyInt("TestItem", "SelfCapTest_Minimum", g_szConfigPath);
	ST.Self_P2P = GetIniKeyInt("TestItem", "SelfCapTest_P2P", g_szConfigPath);
	ST.Self_P2P_Edge = GetIniKeyInt("TestItem", "SelfCapTest_P2P_Edge", g_szConfigPath);
	ST.Self_Frame_Count = GetIniKeyInt("TestItem", "SelfCapTest_Frame", g_szConfigPath);
	fprintf(fp, "   Maximum_______________,%d\n", ST.Self_Maximum);
	fprintf(fp, "   Minimum_______________,%d\n", ST.Self_Minimum);
	fprintf(fp, "   P2P___________________,%d\n", ST.Self_P2P);
	fprintf(fp, "   P2P Edge______________,%d\n", ST.Self_P2P_Edge);
	fprintf(fp, "   Frame_Cnt_____________,%d\n\n", ST.Self_Frame_Count);
	fprintf(fp, "   X_0000,");
	for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch; ucloopCh_X++)
	{
		fprintf(fp, "  CH%02d,", ucloopCh_X);

	}
	fprintf(fp, "\n   X_0001,");

	for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch; ucloopCh_X++)
	{
		fprintf(fp, "%4d  ,", ST.self_xdaltc[ucloopCh_X]);

	}
	fprintf(fp, "\n");

	fprintf(fp, "   X_0000,");

	for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch; ucloopCh_Y++)
	{
		fprintf(fp, "  CH%02d,", ucloopCh_Y);

	}
	fprintf(fp, "\n   X_0001,");

	for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch; ucloopCh_Y++)
	{
		fprintf(fp, "%4d  ,", ST.self_ydaltc[ucloopCh_Y]);

	}
	fprintf(fp, "\n");

}

void vfSaveShortTestLog_V3(FILE *fp)
{
	unsigned char ucloopCh_X;
	unsigned char ucloopCh_Y;
	unsigned int ucloop;
	int key_ych = 0;

	if ((ucSensorTestResult & MICROSHORT_TEST) == MICROSHORT_TEST)
		fprintf(fp, "[Short_Test]                     ,NG ,\n\n");
	else
		fprintf(fp, "[Short_Test]                     ,OK ,\n\n");

	fprintf(fp, "      (Spec.)                    ,\n");
	fprintf(fp, "      Frame_Count                ,%d,\n", ST.Short.FrameCount);
	fprintf(fp, "      Max_Threshold              ,%d,\n", ST.Short.Threshold);
	fprintf(fp, "      KeyTX_Threshold            ,%d,\n", ST.Short.keyTx_thr);
	fprintf(fp, "      KeyRX_Threshold            ,%d,\n\n\n", ST.Short.keyRx_thr);

	fprintf(fp, "      Normal        ,");
	for (ucloopCh_X = 1; ucloopCh_X <= ptl.x_ch; ucloopCh_X++)
		fprintf(fp, "(X_%d%-*c,", ucloopCh_X, 4 - count_digit(ucloopCh_X), ')');
	if (ptl.key_mode == ILITEK_HW_KEY_MODE) {
		for (ucloop = 1; ucloop <= ptl.key_num; ucloop++)
			fprintf(fp, "(KeyX_%u),", ucloop);
	}

	fprintf(fp, "\n       X_SLK        ,");
	for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch + ptl.key_num; ucloopCh_X++)
		fprintf(fp, "%-7d,", ST.short_daltc[0][ucloopCh_X]);

	fprintf(fp, "\n        X_LK        ,");
	for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch + ptl.key_num; ucloopCh_X++)
		fprintf(fp, "%-7d,", ST.short_daltc[2][ucloopCh_X]);

	fprintf(fp, "\n      X_DIFF        ,");
	for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch + ptl.key_num; ucloopCh_X++)
		fprintf(fp, "%-7d,", abs(ST.short_daltc[2][ucloopCh_X] - ST.short_daltc[0][ucloopCh_X]));

	fprintf(fp, "\n                    ,\n                    ,");
	for (ucloopCh_Y = 1; ucloopCh_Y <= ptl.y_ch; ucloopCh_Y++)
		fprintf(fp, "(Y_%d%-*c,", ucloopCh_Y, 4 - count_digit(ucloopCh_Y), ')');
	if (ptl.key_mode == ILITEK_HW_KEY_MODE) {
		fprintf(fp, "(KeyY_1),");
		key_ych = 1;
	}

	fprintf(fp, "\n       Y_SLK        ,");
	for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch + key_ych; ucloopCh_Y++)
		fprintf(fp, "%-7d,", ST.short_daltc[1][ucloopCh_Y]);

	fprintf(fp, "\n        Y_LK        ,");
	for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch + key_ych; ucloopCh_Y++)
		fprintf(fp, "%-7d,", ST.short_daltc[3][ucloopCh_Y]);

	fprintf(fp, "\n      Y_DIFF        ,");
	for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch + key_ych; ucloopCh_Y++)
		fprintf(fp, "%-7d,", abs(ST.short_daltc[3][ucloopCh_Y] - ST.short_daltc[1][ucloopCh_Y]));
	fprintf(fp, "\n\n");
}

void vfSaveFWVerTestLog_V6(FILE *fp)
{
	int i = 0;

	if ((ucSensorTestResult & FWVERTION_TEST) == FWVERTION_TEST)
		fprintf(fp, "[FW_Verify]                      ,NG ,\n");
	else
		fprintf(fp, "[FW_Verify]                      ,OK ,\n");

	if (ptl.ver <= PROTOCOL_V6_0_2) {
		fprintf(fp, "The protocol no support\n");
		return;
	}

	fprintf(fp, "\n");
	fprintf(fp, "      (Spec.)                    ,\n");
	fprintf(fp, "      Path                       ,%s,\n", ST.hexfile);

	fprintf(fp, "      Master_CRC                 ,");
	for (i = 0; i < ST.block_num; i++)
		fprintf(fp, "%X,", ST.master_crc[i]);
	fprintf(fp, "\n");

	fprintf(fp, "      Slave_CRC                  ,");
	if (!ST.slave_num) {
		fprintf(fp, "0,");
	} else {
		for (i = 0; i < ST.slave_num; i++)
			fprintf(fp, "%X,", ST.slave_crc[i]);
	}
	fprintf(fp, "\n");

	fprintf(fp, "      Block                      ,%d,\n", ST.block_num);
	fprintf(fp, "      Slave_number               ,%d,\n", ST.slave_num);
	fprintf(fp, "      FW_Ver                     ,%02X.%02X.%02X.%02X.%02X.%02X.%02X.%02X,\n",
		ST.fw_ver[0], ST.fw_ver[1], ST.fw_ver[2], ST.fw_ver[3],
		ST.fw_ver[4], ST.fw_ver[5], ST.fw_ver[6], ST.fw_ver[7]);

	fprintf(fp, "\n\n\n\n");
}

void vfSaveOpenTestLog_V6(FILE *fp)
{
	unsigned int ucloopCh_X;
	unsigned int ucloopCh_Y;
	unsigned int ucloop;

	if ((ucSensorTestResult & OPEN_TEST) == OPEN_TEST)
		fprintf(fp, "[Open_Test]                      ,NG ,\n\n");
	else
		fprintf(fp, "[Open_Test]                      ,OK ,\n\n");

	fprintf(fp, "      (Spec.)                    ,\n");
	fprintf(fp, "      Frame_Count                ,%d,\n", ST.Open_FrameCount);
	fprintf(fp, "      Min_Threshold              ,%d,\n", ST.Open_Threshold);
	//fprintf(fp, "   TX_Average_Diff_Gap        ,%d\n", ST.Open_TX_Aver_Diff);
	//fprintf(fp, "   RX_Diff_Gap                ,%d\n", ST.Open_RX_Delta_Threshold);
	//fprintf(fp, "   RX_Diff_Gap_Tolerance      ,%d\n", ST.Open_RX_Continue_Fail_Tolerance);
	fprintf(fp, "      Key_Threshold              ,%d,\n\n\n", ST.Open.key_thr);

	fprintf(fp, "      Normal        ,");

	for (ucloopCh_X = 1; ucloopCh_X <= ptl.x_ch; ucloopCh_X++)
		fprintf(fp, "(X_%d%-*c,", ucloopCh_X, 4 - count_digit(ucloopCh_X), ')');
	fprintf(fp, "\n");

	for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch; ucloopCh_Y++) {
		fprintf(fp, "      Y_%-3d         ,", ucloopCh_Y + 1);

		for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch; ucloopCh_X++) {
			if (ST.v6_open_daltc[ucloopCh_X][ucloopCh_Y].status)
				fprintf(fp, "*%-6d,", ST.v6_open_daltc[ucloopCh_X][ucloopCh_Y].data);
			else
				fprintf(fp, "%-7d,", ST.v6_open_daltc[ucloopCh_X][ucloopCh_Y].data);
		}
		/*
		if (ST.Tx_Avdiff_daltc[ucloopCh_Y].status == NODE_FAIL)
			fprintf(fp, "*%6d,\n", ST.Tx_Avdiff_daltc[ucloopCh_Y].data);
		else
			fprintf(fp, "%7d \n", ST.Tx_Avdiff_daltc[ucloopCh_Y].data);
		*/
		fprintf(fp, "\n");
	}
	if (ptl.key_mode == ILITEK_HW_KEY_MODE) {
		fprintf(fp, "      Key_1         ,");
		for (ucloop = 0; ucloop < ptl.key_num; ucloop++) {
			if (ST.Open.key_daltc[ucloop].status == NODE_FAIL)
				fprintf(fp, "*%-5d ,", ST.Open.key_daltc[ucloop].data);
			else
				fprintf(fp, "%-6d ,", ST.Open.key_daltc[ucloop].data);
		}
		fprintf(fp, "\n");
	}
	if (ST.Open_RX_Delta_Threshold > 0) {
		fprintf(fp, "\n      RX_Diff       ,");

		for (ucloopCh_X = 1; ucloopCh_X <= ptl.x_ch; ucloopCh_X++)
			fprintf(fp, "(X_%d%-*c,", ucloopCh_X, 4 - count_digit(ucloopCh_X), ')');
		fprintf(fp, "\n");
		for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch; ucloopCh_Y++) {
			fprintf(fp, "      Y_%-3d         ,", ucloopCh_Y + 1);

			for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch; ucloopCh_X++) {
				if(ST.open_Rx_diff[ucloopCh_X][ucloopCh_Y].status)
					fprintf(fp, "*%-5d ,", ST.open_Rx_diff[ucloopCh_X][ucloopCh_Y].data);
				else
					fprintf(fp, "%-6d ,", ST.open_Rx_diff[ucloopCh_X][ucloopCh_Y].data);
			}
			fprintf(fp, "\n");
		}
	}

	fprintf(fp, "\n\n");
}

void vfSaveMircoOpenTestLog_V6(FILE *fp)
{
	unsigned int ucloopCh_X;
	unsigned int ucloopCh_Y;
	int i;

	if ((ucSensorTestResult & MIRCO_OPEN_TEST) == MIRCO_OPEN_TEST)
		fprintf(fp, "[MicroOpen_Test]                 ,NG ,\n");
	else
		fprintf(fp, "[MicroOpen_Test]                 ,OK ,\n");

	fprintf(fp, "\n      (Spec.)                    ,\n");

	if (ST.MOpen.RxDeltaEn == ENABLE_TEST) {
		fprintf(fp, "      RX_Delta        ,");
		for (ucloopCh_X = 1; ucloopCh_X < ptl.x_ch; ucloopCh_X++)
			fprintf(fp, "(D_%d%-*c,", ucloopCh_X, 5 - count_digit(ucloopCh_X), ')');
		fprintf(fp, "\n");
		for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch; ucloopCh_Y++) {
			fprintf(fp, "      Y_%-3d           ,", ucloopCh_Y + 1);
			for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch - 1; ucloopCh_X++)
				fprintf(fp, "%-8d,", ST.MOpen.rx_diff[ucloopCh_X][ucloopCh_Y].ini.max);
			fprintf(fp, "\n");
		}
	}
	if (ST.MOpen.TxAvgEn == ENABLE_TEST) {
		fprintf(fp, "      TX_Avg_Delta    ,(Avg_D)\n");
		for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch - 1; ucloopCh_Y++)
			fprintf(fp, "      D_%-3d           ,%d\n", ucloopCh_Y + 1, ST.MOpen.tx_avg[0][ucloopCh_Y].ini.max);
	}
	fprintf(fp, "\n\n\n");

	if (ST.MOpen.RxDeltaEn == ENABLE_TEST) {
		fprintf(fp, "      (RX_Delta)                 ,FailCounts:%d,", ST.MOpen.rx_diff_fail_cnt);
		for (ucloopCh_Y = 0, i = 0; ucloopCh_Y < ptl.y_ch; ucloopCh_Y++) {
			for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch - 1; ucloopCh_X++) {
				if (ST.MOpen.rx_diff[ucloopCh_X][ucloopCh_Y].raw.status) {
					i++;
					if (i%5 == 1)
						fprintf(fp, "\n      ");
					fprintf(fp, "(%03d,%03d) Diff:%03d, ",
						ucloopCh_X + 1, ucloopCh_Y + 1,
						ST.MOpen.rx_diff[ucloopCh_X][ucloopCh_Y].raw.data);
				}
			}
		}
	}
	fprintf(fp, "\n\n");
	if (ST.MOpen.TxAvgEn == ENABLE_TEST) {
		fprintf(fp, "      (TX_Avg_Delta)             ,FailChannels:%d,", ST.MOpen.tx_avg_fail_cnt);
		for (ucloopCh_Y = 0, i = 0; ucloopCh_Y < ptl.y_ch - 1; ucloopCh_Y++) {
			if (ST.MOpen.tx_avg[0][ucloopCh_Y].raw.status) {
				i++;
				if (i%5 == 1)
					fprintf(fp, "\n      ");
				fprintf(fp, "(Y_%03d Y_%03d) Diff:%03d, ",
					ucloopCh_Y + 1, ucloopCh_Y + 2,
					ST.MOpen.tx_avg[0][ucloopCh_Y].raw.data);
			}
		}
	}
	fprintf(fp, "\n\n");

	if (ST.MOpen.RxDeltaEn == ENABLE_TEST) {
		fprintf(fp, "\n      RX_Delta        ,");
		for (ucloopCh_X = 1; ucloopCh_X < ptl.x_ch; ucloopCh_X++)
			fprintf(fp, "(D_%d%-*c,", ucloopCh_X, 5 - count_digit(ucloopCh_X), ')');
		fprintf(fp, "\n");
		for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch; ucloopCh_Y++) {
			fprintf(fp, "      Y_%-3d           ,", ucloopCh_Y + 1);
			for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch - 1; ucloopCh_X++) {
				if (ST.MOpen.rx_diff[ucloopCh_X][ucloopCh_Y].raw.status)
					fprintf(fp, "*%-7d,", ST.MOpen.rx_diff[ucloopCh_X][ucloopCh_Y].raw.data);
				else
					fprintf(fp, "%-8d,", ST.MOpen.rx_diff[ucloopCh_X][ucloopCh_Y].raw.data);
			}
			fprintf(fp, "\n");
		}
	}
	fprintf(fp, "\n");

	if (ST.MOpen.TxAvgEn == ENABLE_TEST) {
		fprintf(fp, "      TX_Avg_Delta    ,(Avg_D),\n");
		for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch - 1; ucloopCh_Y++) {
			if (ST.MOpen.tx_avg[0][ucloopCh_Y].raw.status)
				fprintf(fp, "      D_%-3d           ,*%-6d,\n", ucloopCh_Y+1, ST.MOpen.tx_avg[0][ucloopCh_Y].raw.data);
			else
				fprintf(fp, "      D_%-3d           ,%-7d,\n", ucloopCh_Y+1, ST.MOpen.tx_avg[0][ucloopCh_Y].raw.data);
		}
	}
	fprintf(fp, "\n\n");

	fprintf(fp, "      Frame_1       ,");
	for (ucloopCh_X = 1; ucloopCh_X <= ptl.x_ch; ucloopCh_X++)
		fprintf(fp, "(X_%d%-*c,", ucloopCh_X, 4 - count_digit(ucloopCh_X), ')');
	fprintf(fp, "(Y_Avg),\n");

	for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch; ucloopCh_Y++) {
		fprintf(fp, "      Y_%-3d         ,", ucloopCh_Y + 1);
		for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch; ucloopCh_X++) {
			if (ST.v6_open_daltc[ucloopCh_X][ucloopCh_Y].status)
				fprintf(fp, "*%-6d,", ST.v6_open_daltc[ucloopCh_X][ucloopCh_Y].data);
			else
				fprintf(fp, "%-7d,", ST.v6_open_daltc[ucloopCh_X][ucloopCh_Y].data);
		}
		if (ST.Tx_Avdiff_daltc[ucloopCh_Y].status == NODE_FAIL)
			fprintf(fp, "*%-6d,\n", ST.Tx_Avdiff_daltc[ucloopCh_Y].data);
		else
			fprintf(fp, "%-7d,\n", ST.Tx_Avdiff_daltc[ucloopCh_Y].data);
	}
	fprintf(fp, "\n");
}

void SaveUniformitytoCSV_V6(const char *name, FILE *fp, SensorTest_BenBenchmark_Node **data, int x, int y, int type) {
	int loopCh_X = 0, loopCh_Y = 0, status = 0;

	fprintf(fp, "\n%s,", name);
	for (loopCh_X = 1; loopCh_X <= x; loopCh_X++)
	{
		fprintf(fp, "(X_%03d),", loopCh_X);
	}
	fprintf(fp, "\n");
	for (loopCh_Y = 0; loopCh_Y < y; loopCh_Y++)
	{
		fprintf(fp, "      Y_%03d         ,", loopCh_Y + 1);
		for (loopCh_X = 0; loopCh_X < x; loopCh_X++)
		{
			if(type == NODE_STATUS)
				status = data[loopCh_X][loopCh_Y].raw.status;
			else if (type == NODE_MAX_STATUS)
				status = data[loopCh_X][loopCh_Y].raw.max_st;
			else if (type == NODE_MIN_STATUS)
				status = data[loopCh_X][loopCh_Y].raw.min_st;
			if(status == NODE_FAIL)
				fprintf(fp, "*%6d,", data[loopCh_X][loopCh_Y].raw.data);
			else
				fprintf(fp, "%7d,", data[loopCh_X][loopCh_Y].raw.data);
		}
		fprintf(fp, "\n");
	}
}

int count_digit(long long n)
{
	int count = 0;

	while (n != 0) {
		n = n / 10;
		++count;
	}

	return count;
}

void vfSaveUniformityTestLog_V6(FILE *fp)
{
	unsigned int loopCh_X;
	unsigned int loopCh_Y;

	if ((ucSensorTestResult & UNIFORMITY_TEST) == UNIFORMITY_TEST)
		fprintf(fp, "[Uniformity_Test]                ,NG ,\n\n");
	else
		fprintf(fp, "[Uniformity_Test]                ,OK ,\n\n");
	fprintf(fp, "      (Spec.)                    ,\n");

	if (ST.PFVer >= PROFILE_V1_0_2_0) {
		if (ST.Uniformity.En_allraw) {
			fprintf(fp, "      RawData_Max     ,");
			for (loopCh_X = 1; loopCh_X <= ptl.x_ch; loopCh_X++)
				fprintf(fp, "(X_%d%-*c,", loopCh_X, 5 - count_digit(loopCh_X), ')');
			fprintf(fp, "\n");
			for (loopCh_Y = 0; loopCh_Y < ptl.y_ch; loopCh_Y++) {
				fprintf(fp, "      Y_%-3d           ,", loopCh_Y + 1);
				for (loopCh_X = 0; loopCh_X < ptl.x_ch; loopCh_X++)
					fprintf(fp, "%-8d,", ST.Uniformity.allraw[loopCh_X][loopCh_Y].ini.max);
				fprintf(fp, "\n");
			}

			fprintf(fp, "      RawData_Min     ,");
			for (loopCh_X = 1; loopCh_X <= ptl.x_ch; loopCh_X++)
				fprintf(fp, "(X_%d%-*c,", loopCh_X, 5 - count_digit(loopCh_X), ')');
			fprintf(fp, "\n");
			for (loopCh_Y = 0; loopCh_Y < ptl.y_ch; loopCh_Y++) {
				fprintf(fp, "      Y_%-3d           ,", loopCh_Y + 1);
				for (loopCh_X = 0; loopCh_X < ptl.x_ch; loopCh_X++)
					fprintf(fp, "%-8d,", ST.Uniformity.allraw[loopCh_X][loopCh_Y].ini.min);
				fprintf(fp, "\n");
			}
		}
		if (ST.Uniformity.En_allwin1) {
			fprintf(fp, "      Win1_Max        ,");
			for (loopCh_X = 0; loopCh_X < ptl.x_ch; loopCh_X++)
				fprintf(fp, "(X_%d%-*c,", loopCh_X, 5 - count_digit(loopCh_X), ')');
			fprintf(fp, "\n");
			for (loopCh_Y = 0; loopCh_Y < ptl.y_ch - 1; loopCh_Y++) {
				fprintf(fp, "      Y_%-3d           ,", loopCh_Y + 1);
				for (loopCh_X = 0; loopCh_X < ptl.x_ch; loopCh_X++)
					fprintf(fp, "%-8d,", ST.Uniformity.allwin1[loopCh_X][loopCh_Y].ini.max);
				fprintf(fp, "\n");
			}
		}
		if (ST.Uniformity.En_allwin2) {
			fprintf(fp, "      Win2_Max        ,");
			for (loopCh_X = 1; loopCh_X <= ptl.x_ch; loopCh_X++)
				fprintf(fp, "(X_%d%-*c,", loopCh_X, 5 - count_digit(loopCh_X), ')');
			fprintf(fp, "\n");
			for (loopCh_Y = 0; loopCh_Y < ptl.y_ch - 1; loopCh_Y++) {
				fprintf(fp, "      Y_%-3d           ,", loopCh_Y + 1);
				for (loopCh_X = 0; loopCh_X < ptl.x_ch - 1; loopCh_X++)
					fprintf(fp, "%-8d,", ST.Uniformity.allwin2[loopCh_X][loopCh_Y].ini.max);
				fprintf(fp, "\n");
			}
		}

		fprintf(fp, "\n\n\n");

		if (ST.Uniformity.En_allraw) {
			fprintf(fp, "      (Uniformity_RawData_Max)   ,");
			fprintf(fp, "        %s, (FailPoints:%5d %s %5d),\n",
				(ST.Uniformity.RawMaxPass) ? "OK" : "NG",
				ST.Uniformity.RawMaxFailCount,
				(ST.Uniformity.RawMaxPass) ? "<=" : ">",
				ST.Uniformity.Up_FailCount);

			fprintf(fp, "      (Uniformity_RawData_Min)   ,");
			fprintf(fp, "        %s, (FailPoints:%5d %s %5d),\n",
				(ST.Uniformity.RawMinPass) ? "OK" : "NG",
				ST.Uniformity.RawMinFailCount,
				(ST.Uniformity.RawMinPass) ? "<=" : ">",
				ST.Uniformity.Low_FailCount);
		} else {
			fprintf(fp, "      (Uniformity_RawData_Max)   ,  Not Test,\n");
			fprintf(fp, "      (Uniformity_RawData_Min)   ,  Not Test,\n");
		}

		if (ST.Uniformity.En_allwin1) {
			fprintf(fp, "      (Uniformity_Win1_Max)      ,");
			fprintf(fp, "        %s, (FailPoints:%5d %s %5d),\n",
				(ST.Uniformity.Win1Pass) ? "OK" : "NG",
				ST.Uniformity.Win1FailCount,
				(ST.Uniformity.Win1Pass) ? "<=" : ">",
				ST.Uniformity.Win1_FailCount);
		} else {
			fprintf(fp, "      (Uniformity_Win1_Max)      ,  Not Test,\n");
		}

		if (ST.Uniformity.En_allwin2) {
			fprintf(fp, "      (Uniformity_Win2_Max)      ,");
			fprintf(fp, "        %s, (FailPoints:%5d %s %5d),\n",
				(ST.Uniformity.Win2Pass) ? "OK" : "NG",
				ST.Uniformity.Win2FailCount,
				(ST.Uniformity.Win2Pass) ? "<=" : ">",
				ST.Uniformity.Win2_FailCount);
		} else {
			fprintf(fp, "      (Uniformity_Win2_Max)      ,  Not Test,\n");
		}

		if (ST.Uniformity.En_allraw) {
			fprintf(fp, "\n");
			fprintf(fp, "      RawData_Max     ,");
			for (loopCh_X = 1; loopCh_X <= ptl.x_ch; loopCh_X++)
				fprintf(fp, "(X_%d%-*c,", loopCh_X, 5 - count_digit(loopCh_X), ')');
			fprintf(fp, "\n");
			for (loopCh_Y = 0; loopCh_Y < ptl.y_ch; loopCh_Y++) {
				fprintf(fp, "      Y_%-3d           ,", loopCh_Y + 1);
				for (loopCh_X = 0; loopCh_X < ptl.x_ch; loopCh_X++) {
					if (ST.Uniformity.allraw[loopCh_X][loopCh_Y].raw.max_st == NODE_FAIL)
						fprintf(fp, "*%-7d,", ST.Uniformity.allraw[loopCh_X][loopCh_Y].raw.data);
					else
						fprintf(fp, "%-8d,", ST.Uniformity.allraw[loopCh_X][loopCh_Y].raw.data);
				}
				fprintf(fp, "\n");
			}
			fprintf(fp, "\n");
			fprintf(fp, "      RawData_Min     ,");
			for (loopCh_X = 1; loopCh_X <= ptl.x_ch; loopCh_X++)
				fprintf(fp, "(X_%d%-*c,", loopCh_X, 5 - count_digit(loopCh_X), ')');
			fprintf(fp, "\n");
			for (loopCh_Y = 0; loopCh_Y < ptl.y_ch; loopCh_Y++) {
				fprintf(fp, "      Y_%-3d           ,", loopCh_Y + 1);
				for (loopCh_X = 0; loopCh_X < ptl.x_ch; loopCh_X++) {
					if (ST.Uniformity.allraw[loopCh_X][loopCh_Y].raw.min_st == NODE_FAIL)
						fprintf(fp, "*%-7d,", ST.Uniformity.allraw[loopCh_X][loopCh_Y].raw.data);
					else
						fprintf(fp, "%-8d,", ST.Uniformity.allraw[loopCh_X][loopCh_Y].raw.data);
				}
				fprintf(fp, "\n");
			}
		}

		if (ST.Uniformity.En_allwin1) {
			fprintf(fp, "\n");
			fprintf(fp, "      Win1_Max        ,");
			for (loopCh_X = 0; loopCh_X < ptl.x_ch; loopCh_X++)
				fprintf(fp, "(X_%d%-*c,", loopCh_X, 5 - count_digit(loopCh_X), ')');
			fprintf(fp, "\n");
			for (loopCh_Y = 0; loopCh_Y < ptl.y_ch - 1; loopCh_Y++) {
				fprintf(fp, "      Y_%-3d           ,", loopCh_Y + 1);
				for (loopCh_X = 0; loopCh_X < ptl.x_ch; loopCh_X++) {
					if (ST.Uniformity.allwin1[loopCh_X][loopCh_Y].raw.max_st == NODE_FAIL)
						fprintf(fp, "*%-7d,", ST.Uniformity.allwin1[loopCh_X][loopCh_Y].raw.data);
					else
						fprintf(fp, "%-8d,", ST.Uniformity.allwin1[loopCh_X][loopCh_Y].raw.data);
				}
				fprintf(fp, "\n");
			}
		}

		if (ST.Uniformity.En_allwin2) {
			fprintf(fp, "\n");
			fprintf(fp, "      Win2_Max        ,");
			for (loopCh_X = 1; loopCh_X <= ptl.x_ch; loopCh_X++)
				fprintf(fp, "(X_%d%-*c,", loopCh_X, 5 - count_digit(loopCh_X), ')');
			fprintf(fp, "\n");
			for (loopCh_Y = 0; loopCh_Y < ptl.y_ch - 1; loopCh_Y++) {
				fprintf(fp, "      Y_%-3d           ,", loopCh_Y + 1);
				for (loopCh_X = 0; loopCh_X < ptl.x_ch - 1; loopCh_X++) {
					if (ST.Uniformity.allwin2[loopCh_X][loopCh_Y].raw.max_st == NODE_FAIL)
						fprintf(fp, "*%-7d,", ST.Uniformity.allwin2[loopCh_X][loopCh_Y].raw.data);
					else
						fprintf(fp, "%-8d,", ST.Uniformity.allwin2[loopCh_X][loopCh_Y].raw.data);
				}
				fprintf(fp, "\n");
			}
		}
	} else {
		/*
		fprintf(fp, "   Max_Threshold           ,%d\n", ST.Uniformity.Max_Threshold);
		fprintf(fp, "   Up_FailCount            ,%d\n", ST.Uniformity.Up_FailCount);
		fprintf(fp, "   Min_Threshold           ,%d\n", ST.Uniformity.Min_Threshold);
		fprintf(fp, "   Low_FailCount           ,%d\n", ST.Uniformity.Low_FailCount);
		fprintf(fp, "   Win1_Threshold          ,%d\n", ST.Uniformity.Win1_Threshold);
		fprintf(fp, "   Win1_FailCount          ,%d\n", ST.Uniformity.Win1_FailCount);
		fprintf(fp, "   Win2_Threshold          ,%d\n", ST.Uniformity.Win2_Threshold);
		fprintf(fp, "   Win2_FailCount          ,%d\n", ST.Uniformity.Win2_FailCount);
		*/

		fprintf(fp, "\n      Upper_Limit   ,");
		for (loopCh_X = 1; loopCh_X <= ptl.x_ch; loopCh_X++)
			fprintf(fp, "(X_%03d),", loopCh_X);
		fprintf(fp, "\n");
		for (loopCh_Y = 0; loopCh_Y < ptl.y_ch; loopCh_Y++){
			fprintf(fp, "      Y_%03d         ,", loopCh_Y);
			for (loopCh_X = 0; loopCh_X < ptl.x_ch; loopCh_X++) {
				if (ST.v6_unifor_daltc[loopCh_X][loopCh_Y].max_st)
					fprintf(fp, "*%6d,", ST.v6_unifor_daltc[loopCh_X][loopCh_Y].data);
				else
					fprintf(fp, "%7d,", ST.v6_unifor_daltc[loopCh_X][loopCh_Y].data);
			}
			fprintf(fp, "\n");
		}

		fprintf(fp, "\n      Lower_Limit   ,");
		for (loopCh_X = 1; loopCh_X <= ptl.x_ch; loopCh_X++)
			fprintf(fp, "(X_%03d),", loopCh_X);
		fprintf(fp, "\n");
		for (loopCh_Y = 0; loopCh_Y < ptl.y_ch; loopCh_Y++) {
			fprintf(fp, "      Y_%03d         ,", loopCh_Y);
			for (loopCh_X = 0; loopCh_X < ptl.x_ch; loopCh_X++) {
				if (ST.v6_unifor_daltc[loopCh_X][loopCh_Y].min_st)
					fprintf(fp, "*%6d,", ST.v6_unifor_daltc[loopCh_X][loopCh_Y].data);
				else
					fprintf(fp, "%7d,", ST.v6_unifor_daltc[loopCh_X][loopCh_Y].data);
			}
			fprintf(fp, "\n");
		}

		fprintf(fp, "\n      Win1          ,");
		for (loopCh_X = 1; loopCh_X <= ptl.x_ch; loopCh_X++)
			fprintf(fp, "(X_%03d),", loopCh_X);
		fprintf(fp, "\n");
		for (loopCh_Y = 0; loopCh_Y < ptl.y_ch - 1; loopCh_Y++) {
			fprintf(fp, "      Y_%03d         ,", loopCh_Y);
			for (loopCh_X = 0; loopCh_X < ptl.x_ch; loopCh_X++) {
				if (ST.v6_unifor_win1[loopCh_X][loopCh_Y].status)
					fprintf(fp, "*%6d,", ST.v6_unifor_win1[loopCh_X][loopCh_Y].data);
				else
					fprintf(fp, "%7d,", ST.v6_unifor_win1[loopCh_X][loopCh_Y].data);
			}
			fprintf(fp, "\n");
		}

		fprintf(fp, "\n      Win2          ,");
		for (loopCh_X = 1; loopCh_X <= ptl.x_ch; loopCh_X++)
			fprintf(fp, "(X_%03d),", loopCh_X);
		fprintf(fp, "\n");
		for (loopCh_Y = 0; loopCh_Y < ptl.y_ch - 1; loopCh_Y++) {
			fprintf(fp, "      Y_%03d         ,", loopCh_Y);
			for (loopCh_X = 0; loopCh_X < ptl.x_ch - 1; loopCh_X++) {
				if (ST.v6_unifor_win2[loopCh_X][loopCh_Y].status)
					fprintf(fp, "*%6d,", ST.v6_unifor_win2[loopCh_X][loopCh_Y].data);
				else
					fprintf(fp, "%7d,", ST.v6_unifor_win2[loopCh_X][loopCh_Y].data);
			}
			fprintf(fp, "\n");
		}

		if(ST.Uniformity.En_bench == ENABLE_TEST)
			SaveUniformitytoCSV_V6("      Benchmark     ", fp, ST.Uniformity.bench, ptl.x_ch, ptl.y_ch, NODE_STATUS);
		if(ST.Uniformity.En_allraw == ENABLE_TEST) {
			SaveUniformitytoCSV_V6("      ANode_Raw_Up  ", fp, ST.Uniformity.allraw, ptl.x_ch, ptl.y_ch, NODE_MAX_STATUS);
			SaveUniformitytoCSV_V6("      ANode_Raw_Up  ", fp, ST.Uniformity.allraw, ptl.x_ch, ptl.y_ch, NODE_MIN_STATUS);
		}
		if(ST.Uniformity.En_allwin1 == ENABLE_TEST) {
			SaveUniformitytoCSV_V6("      ANode_Win1_Up ", fp, ST.Uniformity.allwin1, ptl.x_ch, ptl.y_ch - 1, NODE_MAX_STATUS);
			SaveUniformitytoCSV_V6("      ANode_Win1_Low", fp, ST.Uniformity.allwin1, ptl.x_ch, ptl.y_ch - 1, NODE_MIN_STATUS);
		}
		if (ST.Uniformity.En_allwin2 == ENABLE_TEST) {
			SaveUniformitytoCSV_V6("      ANode_Win2_Up ", fp, ST.Uniformity.allwin2, ptl.x_ch - 1, ptl.y_ch - 1, NODE_MAX_STATUS);
			SaveUniformitytoCSV_V6("      ANode_Win2_Low", fp, ST.Uniformity.allwin2, ptl.x_ch - 1, ptl.y_ch - 1, NODE_MIN_STATUS);
		}
	}

	fprintf(fp, "\n\n");
}

void vfSaveShortTestLog_V6(FILE *fp)
{
	unsigned int ucloopCh_X;
	unsigned int ucloopCh_Y;
	unsigned int ucloop;

	if ((ucSensorTestResult & MICROSHORT_TEST) == MICROSHORT_TEST)
		fprintf(fp, "[Short_Test]                     ,NG ,\n\n");
	else
		fprintf(fp, "[Short_Test]                     ,OK ,\n\n");

	fprintf(fp, "      (Spec.)                    ,\n");
	fprintf(fp, "      Frame_Count                ,%d,\n", ST.Short.FrameCount);
	fprintf(fp, "      Max_Threshold              ,%d,\n", ST.Short.Threshold);
	fprintf(fp, "      KeyTX_Threshold            ,%d,\n", ST.Short.keyTx_thr);
	fprintf(fp, "      KeyRX_Threshold            ,%d,\n\n\n", ST.Short.keyRx_thr);
	fprintf(fp, "      Normal        ,");

	for (ucloopCh_X = 1; ucloopCh_X <= ptl.x_ch; ucloopCh_X++)
		fprintf(fp, "(X_%d%-*c,", ucloopCh_X, 4 - count_digit(ucloopCh_X), ')');
	if (ptl.key_mode == ILITEK_HW_KEY_MODE) {
		for (ucloop = 1; ucloop <= ptl.key_num; ucloop++)
			fprintf(fp, "(KeyX_%d),", ucloop);
	}
	fprintf(fp, "\n       X_SLK        ,");

	for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch; ucloopCh_X++) {
		if(ST.v6_short_daltc[0][ucloopCh_X].status == NODE_FAIL)
			fprintf(fp, "*%-6d,", ST.v6_short_daltc[0][ucloopCh_X].data);
		else
			fprintf(fp, "%-7d,", ST.v6_short_daltc[0][ucloopCh_X].data);

	}
	if (ptl.key_mode == ILITEK_HW_KEY_MODE) {
		for (ucloop = 0; ucloop < ptl.key_num; ucloop++) {
			if(ST.Short.key_daltc[ucloop].status == NODE_FAIL)
				fprintf(fp, "*%-6d,", ST.Short.key_daltc[ucloop].data);
			else
				fprintf(fp, "%-7d,", ST.Short.key_daltc[ucloop].data);
		}
	}

	fprintf(fp, "\n                    ,\n");
	fprintf(fp, "                    ,");
	for (ucloopCh_Y = 1; ucloopCh_Y <= ptl.y_ch; ucloopCh_Y++)
		fprintf(fp, "(Y_%d%-*c,", ucloopCh_Y, 4 - count_digit(ucloopCh_Y), ')');
	if (ptl.key_mode == ILITEK_HW_KEY_MODE)
		fprintf(fp, "(KeyY_1),");
	fprintf(fp, "\n       Y_SLK        ,");

	for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch; ucloopCh_Y++) {
		if (ST.v6_short_daltc[1][ucloopCh_Y].status == NODE_FAIL)
			fprintf(fp, "*%-6d,", ST.v6_short_daltc[1][ucloopCh_Y].data);
		else
			fprintf(fp, "%-7d,", ST.v6_short_daltc[1][ucloopCh_Y].data);

	}
	if (ptl.key_mode == ILITEK_HW_KEY_MODE) {
		if (ST.Short.key_daltc[ptl.key_num].status == NODE_FAIL)
			fprintf(fp, "*%-6d,", ST.Short.key_daltc[ptl.key_num].data);
		else
			fprintf(fp, "%-7d,", ST.Short.key_daltc[ptl.key_num].data);
	}
	fprintf(fp, "\n                    ,\n\n");
}
void vfSaveUniformityTestLog(FILE *fp)
{
	unsigned char loopCh_X;
	unsigned char loopCh_Y;

	if ((ucSensorTestResult & UNIFORMITY_TEST) == UNIFORMITY_TEST)
		fprintf(fp, "[Uniformity_Test]                ,NG ,\n\n");
	else
		fprintf(fp, "[Uniformity_Test]                ,OK ,\n\n");
	fprintf(fp, "      (Spec.)                    ,\n");

	if (!ST.useINI) {
		fprintf(fp, "BD_VA_L_Ratio_Min :      %3d%%,  BD_VA_L_Ratio_Max :       %3d%%\n", ST.Uniformity.uiBD_VA_L_Ratio_Min,ST.Uniformity.uiBD_VA_L_Ratio_Max);
		fprintf(fp, "BD_VA_R_Ratio_Min :      %3d%%,  BD_VA_R_Ratio_Max :       %3d%%\n", ST.Uniformity.uiBD_VA_R_Ratio_Min,ST.Uniformity.uiBD_VA_R_Ratio_Max);
		fprintf(fp, "BD_VA_Top_Ratio_Min :     %3d%%,  BD_VA_Top_Ratio_Max :     %3d%%\n", ST.Uniformity.uiBD_VA_Top_Ratio_Min,ST.Uniformity.uiBD_VA_Top_Ratio_Max);
		fprintf(fp, "BD_VA_Bottom_Ratio_Min : %3d%%,  BD_VA_Bottom_Ratio_Max :  %d%%\n\n", ST.Uniformity.uiBD_VA_Bottom_Ratio_Min,ST.Uniformity.uiBD_VA_Bottom_Ratio_Max);
		fprintf(fp, "BD_Top_XDiff_Caculate_Value :        0,  BD_Top_Ratio :     %3d%%\n", ST.Uniformity.uiBD_Top_Ratio);
		fprintf(fp, "BD_Bottom_XDiff_Caculate_Value :     0,  BD_Bottom_Ratio :  %3d%%\n", ST.Uniformity.uiBD_Bottom_Ratio);
		fprintf(fp, "BD_Left_YDiff_Caculate_Value :       0,  BD_L_Ratio :        %3d%%\n", ST.Uniformity.uiBD_L_Ratio);
		fprintf(fp, "BD_Right_YDiff_Caculate_Value:       0,  BD_R_Ratio :        %3d%%\n\n", ST.Uniformity.uiBD_R_Ratio);
		fprintf(fp, "VA_XDiff_Caculate_Value :            0,  VA_Ratio_X_diff :  %3d%%\n", ST.Uniformity.uiVA_Ratio_X_diff);
		fprintf(fp, "VA_YDiff_Caculate_Value :            0,  VA_Ratio_Y_diff :  %3d%%\n", ST.Uniformity.uiVA_Ratio_Y_diff);
		fprintf(fp, "PanelRange_LeftTop :         0 -    0,  PanelLeftTopLLimit:     %3d%%,  PanelLeftTopULimit:     %3d%%\n", ST.Uniformity.uiPanelLeftTopLLimit,ST.Uniformity.uiPanelLeftTopULimit);
		fprintf(fp, "PanelRange_RightTop :        0 -    0,  PanelRightTopLLimit:    %3d%%,  PanelRightTopULimit:    %3d%%\n", ST.Uniformity.uiPanelRightTopLLimit,ST.Uniformity.uiPanelRightTopULimit);
		fprintf(fp, "PanelRange_LeftBottom :      0 -    0,  PanelLeftBottomLLimit:  %3d%%,  PanelLeftBottomULimit:  %3d%%\n", ST.Uniformity.uiPanelLeftBottomLLimit,ST.Uniformity.uiPanelLeftBottomULimit);
		fprintf(fp, "PanelRange_RightBottom :     0 -    0,  PanelRightBottomLLimit: %3d%%,  PanelRightBottomULimit: %3d%%\n\n", ST.Uniformity.uiPanelRightBottomLLimit,ST.Uniformity.uiPanelRightBottomULimit);

		fprintf(fp, "   Y_Driven_Bench_____,");
		for (loopCh_X = 0; loopCh_X < ptl.x_ch; loopCh_X++)
			fprintf(fp, "   X%02d  ,", loopCh_X);
		fprintf(fp, "\n");

		for (loopCh_Y = 0; loopCh_Y < ptl.y_ch; loopCh_Y++) {
			fprintf(fp, "Y_Driven_Bench__Y%02d,", loopCh_Y);

			for (loopCh_X = 0; loopCh_X < ptl.x_ch; loopCh_X++)
				fprintf(fp, "%5d   ,", ST.BenchMark.iUniformityBenchMark[loopCh_X][loopCh_Y]);
			fprintf(fp, "\n");
		}
		fprintf(fp, "\n");


		fprintf(fp, "   Y_Driven_Data______,");
		for (loopCh_X = 0; loopCh_X < ptl.x_ch; loopCh_X++)
			fprintf(fp, "   X%02d  ,", loopCh_X);
		fprintf(fp, "\n");

		for (loopCh_Y = 0; loopCh_Y < ptl.y_ch; loopCh_Y++) {
			fprintf(fp, "Y_Driven_Data___Y%02d,", loopCh_Y);

			for (loopCh_X = 0; loopCh_X < ptl.x_ch; loopCh_X++)
				fprintf(fp, "%5d   ,", ST.unifor_daltc[loopCh_X][loopCh_Y]);
			fprintf(fp, "\n");
		}
		fprintf(fp, "\n");

		return;
	}

	if (ST.Uniformity.En_allraw) {
		fprintf(fp, "      RawData_Max     ,");
		for (loopCh_X = 1; loopCh_X <= ptl.x_ch; loopCh_X++)
			fprintf(fp, "(X_%d%-*c,", loopCh_X, 5 - count_digit(loopCh_X), ')');
		fprintf(fp, "\n");
		for (loopCh_Y = 0; loopCh_Y < ptl.y_ch; loopCh_Y++) {
			fprintf(fp, "      Y_%-3d           ,", loopCh_Y + 1);
			for (loopCh_X = 0; loopCh_X < ptl.x_ch; loopCh_X++)
				fprintf(fp, "%-8d,", ST.Uniformity.allraw[loopCh_X][loopCh_Y].ini.max);
			fprintf(fp, "\n");
		}

		fprintf(fp, "      RawData_Min     ,");
		for (loopCh_X = 1; loopCh_X <= ptl.x_ch; loopCh_X++)
			fprintf(fp, "(X_%d%-*c,", loopCh_X, 5 - count_digit(loopCh_X), ')');
		fprintf(fp, "\n");
		for (loopCh_Y = 0; loopCh_Y < ptl.y_ch; loopCh_Y++) {
			fprintf(fp, "      Y_%-3d           ,", loopCh_Y + 1);
			for (loopCh_X = 0; loopCh_X < ptl.x_ch; loopCh_X++)
				fprintf(fp, "%-8d,", ST.Uniformity.allraw[loopCh_X][loopCh_Y].ini.min);
			fprintf(fp, "\n");
		}
	}
	if (ST.Uniformity.En_allwin1) {
		fprintf(fp, "      Win1_Max        ,");
		for (loopCh_X = 0; loopCh_X < ptl.x_ch; loopCh_X++)
			fprintf(fp, "(X_%d%-*c,", loopCh_X, 5 - count_digit(loopCh_X), ')');
		fprintf(fp, "\n");
		for (loopCh_Y = 0; loopCh_Y < ptl.y_ch - 1; loopCh_Y++) {
			fprintf(fp, "      Y_%-3d           ,", loopCh_Y + 1);
			for (loopCh_X = 0; loopCh_X < ptl.x_ch; loopCh_X++)
				fprintf(fp, "%-8d,", ST.Uniformity.allwin1[loopCh_X][loopCh_Y].ini.max);
			fprintf(fp, "\n");
		}
	}
	if (ST.Uniformity.En_allwin2) {
		fprintf(fp, "      Win2_Max        ,");
		for (loopCh_X = 1; loopCh_X <= ptl.x_ch; loopCh_X++)
			fprintf(fp, "(X_%d%-*c,", loopCh_X, 5 - count_digit(loopCh_X), ')');
		fprintf(fp, "\n");
		for (loopCh_Y = 0; loopCh_Y < ptl.y_ch - 1; loopCh_Y++) {
			fprintf(fp, "      Y_%-3d           ,", loopCh_Y + 1);
			for (loopCh_X = 0; loopCh_X < ptl.x_ch - 1; loopCh_X++)
				fprintf(fp, "%-8d,", ST.Uniformity.allwin2[loopCh_X][loopCh_Y].ini.max);
			fprintf(fp, "\n");
		}
	}

	fprintf(fp, "\n\n\n");

	if (ST.Uniformity.En_allraw) {
		fprintf(fp, "      (Uniformity_RawData_Max)   ,");
		if (ST.Uniformity.RawMaxPass) {
			fprintf(fp, "        OK, (FailPoints:%5d <= %5d),\n",
				ST.Uniformity.RawMaxFailCount,
				ST.Uniformity.Up_FailCount);
		} else {
			fprintf(fp, "        NG, (FailPoints:%5d > %5d),\n",
				ST.Uniformity.RawMaxFailCount,
				ST.Uniformity.Up_FailCount);
		}
		fprintf(fp, "      (Uniformity_RawData_Min)   ,");
		if (ST.Uniformity.RawMinPass) {
			fprintf(fp, "        OK, (FailPoints:%5d <= %5d),\n",
				ST.Uniformity.RawMinFailCount,
				ST.Uniformity.Low_FailCount);
		} else {
			fprintf(fp, "        NG, (FailPoints:%5d > %5d),\n",
				ST.Uniformity.RawMinFailCount,
				ST.Uniformity.Low_FailCount);
		}
	} else {
		fprintf(fp, "      (Uniformity_RawData_Max)   ,  Not Test,\n");
		fprintf(fp, "      (Uniformity_RawData_Min)   ,  Not Test,\n");
	}

	if (ST.Uniformity.En_allwin1) {
		fprintf(fp, "      (Uniformity_Win1_Max)      ,");
		if (ST.Uniformity.Win1Pass) {
			fprintf(fp, "        OK, (FailPoints:%5d <= %5d),\n",
				ST.Uniformity.Win1FailCount,
				ST.Uniformity.Win1_FailCount);
		} else {
			fprintf(fp, "        NG, (FailPoints:%5d > %5d),\n",
				ST.Uniformity.Win1FailCount,
				ST.Uniformity.Win1_FailCount);
		}
	} else {
		fprintf(fp, "      (Uniformity_Win1_Max)      ,  Not Test,\n");
	}

	if (ST.Uniformity.En_allwin2) {
		fprintf(fp, "      (Uniformity_Win2_Max)      ,");
		if (ST.Uniformity.Win2Pass) {
			fprintf(fp, "        OK, (FailPoints:%5d <= %5d),\n",
				ST.Uniformity.Win2FailCount,
				ST.Uniformity.Win2_FailCount);
		} else {
			fprintf(fp, "        NG, (FailPoints:%5d > %5d),\n",
				ST.Uniformity.Win2FailCount,
				ST.Uniformity.Win2_FailCount);
		}
	} else {
		fprintf(fp, "      (Uniformity_Win2_Max)      ,  Not Test,\n");
	}

	if (ST.Uniformity.En_allraw) {
		fprintf(fp, "\n");
		fprintf(fp, "      RawData_Max     ,");
		for (loopCh_X = 1; loopCh_X <= ptl.x_ch; loopCh_X++)
			fprintf(fp, "(X_%d%-*c,", loopCh_X, 5 - count_digit(loopCh_X), ')');
		fprintf(fp, "\n");
		for (loopCh_Y = 0; loopCh_Y < ptl.y_ch; loopCh_Y++) {
			fprintf(fp, "      Y_%-3d           ,", loopCh_Y + 1);
			for (loopCh_X = 0; loopCh_X < ptl.x_ch; loopCh_X++)
				fprintf(fp, "%-8d,", ST.unifor_daltc[loopCh_X][loopCh_Y]);
			fprintf(fp, "\n");
		}
		fprintf(fp, "\n");
		fprintf(fp, "      RawData_Min     ,");
		for (loopCh_X = 1; loopCh_X <= ptl.x_ch; loopCh_X++)
			fprintf(fp, "(X_%d%-*c,", loopCh_X, 5 - count_digit(loopCh_X), ')');
		fprintf(fp, "\n");
		for (loopCh_Y = 0; loopCh_Y < ptl.y_ch; loopCh_Y++) {
			fprintf(fp, "      Y_%-3d           ,", loopCh_Y + 1);
			for (loopCh_X = 0; loopCh_X < ptl.x_ch; loopCh_X++)
				fprintf(fp, "%-8d,", ST.unifor_daltc[loopCh_X][loopCh_Y]);
			fprintf(fp, "\n");
		}
	}

	if (ST.Uniformity.En_allwin1) {
		fprintf(fp, "\n");
		fprintf(fp, "      Win1_Max        ,");
		for (loopCh_X = 0; loopCh_X < ptl.x_ch; loopCh_X++)
			fprintf(fp, "(X_%d%-*c,", loopCh_X, 5 - count_digit(loopCh_X), ')');
		fprintf(fp, "\n");
		for (loopCh_Y = 0; loopCh_Y < ptl.y_ch - 1; loopCh_Y++) {
			fprintf(fp, "      Y_%-3d           ,", loopCh_Y + 1);
			for (loopCh_X = 0; loopCh_X < ptl.x_ch; loopCh_X++)
				fprintf(fp, "%-8d,", ST.unifor_daltc[loopCh_X][loopCh_Y]);
			fprintf(fp, "\n");
		}
	}

	if (ST.Uniformity.En_allwin2) {
		fprintf(fp, "\n");
		fprintf(fp, "      Win2_Max        ,");
		for (loopCh_X = 1; loopCh_X <= ptl.x_ch; loopCh_X++)
			fprintf(fp, "(X_%d%-*c,", loopCh_X, 5 - count_digit(loopCh_X), ')');
		fprintf(fp, "\n");
		for (loopCh_Y = 0; loopCh_Y < ptl.y_ch - 1; loopCh_Y++) {
			fprintf(fp, "      Y_%-3d           ,", loopCh_Y + 1);
			for (loopCh_X = 0; loopCh_X < ptl.x_ch - 1; loopCh_X++)
				fprintf(fp, "%-8d,", ST.unifor_daltc[loopCh_X][loopCh_Y]);
			fprintf(fp, "\n");
		}
	}
	fprintf(fp, "\n\n");
}

void vfSaveOpenTestLog_NewFlow(FILE *fp)
{
	unsigned char ucloopCh_X;
	unsigned char ucloopCh_Y;

	if ((ucSensorTestResult & OPEN_TEST) == OPEN_TEST)
	{
		fprintf(fp, "[Open_Test]              ,NG ,\n");
	}
	else
	{
		fprintf(fp, "[Open_Test]              ,OK ,\n");
	}

	fprintf(fp, "   Y_Driven_Data___(20V)\n");

	fprintf(fp, "   Y_Driven_Data______,");

	for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch; ucloopCh_X++)
	{
		fprintf(fp, "   X%02d  ,", ucloopCh_X);

	}
	fprintf(fp, "\n");


	for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch; ucloopCh_Y++)
	{
		fprintf(fp, "   Y_Driven_Data___Y%02d,", ucloopCh_Y);

		for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch; ucloopCh_X++)
		{
			fprintf(fp, "%5d   ,", ST.open_20V_daltc[ucloopCh_X][ucloopCh_Y]);

		}
		fprintf(fp, "\n");

	}

	//=============================6V==========================================
	fprintf(fp, "\n");

	fprintf(fp, "Y_Driven_Data___(6V)\n");

	fprintf(fp, "Y_Driven_Data______,");

	for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch; ucloopCh_X++)
	{
		fprintf(fp, "   X%02d  ,", ucloopCh_X);

	}
	fprintf(fp, "\n");

	for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch; ucloopCh_Y++)
	{
		fprintf(fp, "Y_Driven_Data___Y%02d,", ucloopCh_Y);

		for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch; ucloopCh_X++)
		{
			fprintf(fp, "%5d   ,", ST.open_6V_daltc[ucloopCh_X][ucloopCh_Y]);
		}
		fprintf(fp, "\n");

	}
	fprintf(fp, "\n");

	//20190314 add by tigers
	//Y_Driven_Data___(Diff_20V-6V)
	fprintf(fp, "Y_Driven_Data___(Diff_20V-6V)\n");

	fprintf(fp, "Y_Driven_Data______,");

	for(ucloopCh_X = 0; ucloopCh_X < ptl.x_ch; ucloopCh_X++)
	{
		fprintf(fp, "   X%02d  ,", ucloopCh_X);
	}
	fprintf(fp, "\n");

	for(ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch; ucloopCh_Y++)
	{
		fprintf(fp, "Y_Driven_Data___Y%02d,", ucloopCh_Y);

		for(ucloopCh_X = 0; ucloopCh_X < ptl.x_ch; ucloopCh_X++)
		{
			fprintf(fp, "%5d   ,", ST.open_20_6V_daltc[ucloopCh_X][ucloopCh_Y]);

		}
		fprintf(fp, "\n");

	}
	fprintf(fp, "\n");

	//Y_Driven_Data___(XDiff_20V-6V)
	fprintf(fp, "Y_Driven_Data___(XDiff_20V-6V)\n");

	fprintf(fp, "Y_Driven_Data______,");

	for(ucloopCh_X = 0; ucloopCh_X < ptl.x_ch - 1; ucloopCh_X++)
	{
		fprintf(fp, "   X%02d  ,", ucloopCh_X);
	}
	fprintf(fp, "\n");

	for(ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch; ucloopCh_Y++)
	{
		fprintf(fp, "Y_Driven_Data___Y%02d,", ucloopCh_Y);

		for(ucloopCh_X = 1; ucloopCh_X < ptl.x_ch; ucloopCh_X++)
		{
			fprintf(fp, "%5d   ,", ST.open_Rx_diff[ucloopCh_X][ucloopCh_Y].data);
		}
		fprintf(fp, "\n");
	}
	fprintf(fp, "\n");

	//Y_Driven_Data___(YDiff_20V-6V)
	fprintf(fp, "Y_Driven_Data___(YDiff_20V-6V)\n");

	fprintf(fp, "Y_Driven_Data______,");

	for(ucloopCh_X = 0; ucloopCh_X < ptl.x_ch; ucloopCh_X++)
	{
		fprintf(fp, "   X%02d  ,", ucloopCh_X);
	}
	fprintf(fp, "\n");
	for(ucloopCh_Y = 1; ucloopCh_Y < ptl.y_ch; ucloopCh_Y++)
	{
		fprintf(fp, "Y_Driven_Data___Y%02d,", (ucloopCh_Y - 1));
		for(ucloopCh_X = 0; ucloopCh_X < ptl.x_ch; ucloopCh_X++)
		{
			fprintf(fp, "%5d   ,", ST.open_Tx_diff_new[ucloopCh_X][ucloopCh_Y]);
		}
		fprintf(fp, "\n");
	}
	fprintf(fp, "\n");

	//Y_Driven_Data___(DC_Range_20V)
	fprintf(fp, "Y_Driven_Data___(DC_Range_20V)\n");

	fprintf(fp, "Y_Driven_Data______,");

	for(ucloopCh_X = 0; ucloopCh_X < ptl.x_ch; ucloopCh_X++)
	{
		fprintf(fp, "   X%02d  ,", ucloopCh_X);
	}
	fprintf(fp, "\n");

	for(ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch; ucloopCh_Y++)
	{
		fprintf(fp, "Y_Driven_Data___Y%02d,", ucloopCh_Y);

		for(ucloopCh_X = 0; ucloopCh_X < ptl.x_ch; ucloopCh_X++)
		{
			fprintf(fp, "%5d   ,", ST.uiYDC_Range[ucloopCh_X][ucloopCh_Y]);
		}
		fprintf(fp, "\n");
	}
	fprintf(fp, "\n");
}

void vfSaveMircoOpenTestLog_V3(FILE *fp)
{
	unsigned int ucloopCh_X;
	unsigned int ucloopCh_Y;
	int i;

	if ((ucSensorTestResult & MIRCO_OPEN_TEST) == MIRCO_OPEN_TEST)
		fprintf(fp, "[MicroOpen_Test]                 ,NG ,\n\n");
	else
		fprintf(fp, "[MicroOpen_Test]                 ,OK ,\n\n");

	fprintf(fp, "      (Spec.)                    ,\n");
	if (ST.MOpen.RxDeltaEn == ENABLE_TEST) {
		fprintf(fp, "      RX_Delta        ,");
		for (ucloopCh_X = 1; ucloopCh_X < ptl.x_ch; ucloopCh_X++)
			fprintf(fp, "(D_%d%-*c,", ucloopCh_X, 5 - count_digit(ucloopCh_X), ')');
		fprintf(fp, "\n");
		for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch; ucloopCh_Y++) {
			fprintf(fp, "      Y_%-3d           ,", ucloopCh_Y + 1);
			for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch - 1; ucloopCh_X++)
				fprintf(fp, "%-8d,", ST.MOpen.rx_diff[ucloopCh_X][ucloopCh_Y].ini.max);
			fprintf(fp, "\n");
		}
	}
	if (ST.MOpen.TxAvgEn == ENABLE_TEST) {
		fprintf(fp, "      TX_Avg_Delta    ,(Avg_D)\n");
		for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch - 1; ucloopCh_Y++)
			fprintf(fp, "      D_%-3d           ,%d\n", ucloopCh_Y + 1, ST.MOpen.tx_avg[0][ucloopCh_Y].ini.max);
	}
	fprintf(fp, "\n\n\n");

	if (ST.MOpen.RxDeltaEn == ENABLE_TEST) {
		fprintf(fp, "      (RX_Delta)                 ,FailCounts:%d,", ST.MOpen.rx_diff_fail_cnt);
		for (ucloopCh_Y = 0, i = 0; ucloopCh_Y < ptl.y_ch; ucloopCh_Y++) {
			for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch - 1; ucloopCh_X++) {
				if (ST.MOpen.rx_diff[ucloopCh_X][ucloopCh_Y].raw.status) {
					i++;
					if (i%5 == 1)
						fprintf(fp, "\n      ");
					fprintf(fp, "(%03d,%03d) Diff:%03d, ",
						ucloopCh_X + 1, ucloopCh_Y + 1,
						ST.MOpen.rx_diff[ucloopCh_X][ucloopCh_Y].raw.data);
				}
			}
		}
	}
	fprintf(fp, "\n\n");
	if (ST.MOpen.TxAvgEn == ENABLE_TEST) {
		fprintf(fp, "      (TX_Avg_Delta)             ,FailChannels:%d,", ST.MOpen.tx_avg_fail_cnt);
		for (ucloopCh_Y = 0, i = 0; ucloopCh_Y < ptl.y_ch - 1; ucloopCh_Y++) {
			if (ST.MOpen.tx_avg[0][ucloopCh_Y].raw.status) {
				i++;
				if (i%5 == 1)
					fprintf(fp, "\n      ");
				fprintf(fp, "(Y_%03d Y_%03d) Diff:%03d, ",
					ucloopCh_Y + 1, ucloopCh_Y + 2,
					ST.MOpen.tx_avg[0][ucloopCh_Y].raw.data);
			}
		}
	}
	fprintf(fp, "\n\n");

	if (ST.MOpen.RxDeltaEn == ENABLE_TEST) {
		fprintf(fp, "\n      RX_Delta        ,");
		for (ucloopCh_X = 1; ucloopCh_X < ptl.x_ch; ucloopCh_X++)
			fprintf(fp, "(D_%d%-*c,", ucloopCh_X, 5 - count_digit(ucloopCh_X), ')');
		fprintf(fp, "\n");
		for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch; ucloopCh_Y++) {
			fprintf(fp, "      Y_%-3d           ,", ucloopCh_Y + 1);
			for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch - 1; ucloopCh_X++) {
				if (ST.MOpen.rx_diff[ucloopCh_X][ucloopCh_Y].raw.status)
					fprintf(fp, "*%-7d,", ST.MOpen.rx_diff[ucloopCh_X][ucloopCh_Y].raw.data);
				else
					fprintf(fp, "%-8d,", ST.MOpen.rx_diff[ucloopCh_X][ucloopCh_Y].raw.data);
			}
			fprintf(fp, "\n");
		}
	}
	fprintf(fp, "\n");

	if (ST.MOpen.TxAvgEn == ENABLE_TEST) {
		fprintf(fp, "      TX_Avg_Delta    ,(Avg_D),\n");
		for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch - 1; ucloopCh_Y++) {
			if (ST.MOpen.tx_avg[0][ucloopCh_Y].raw.status)
				fprintf(fp, "      D_%-3d           ,*%-6d,\n", ucloopCh_Y+1, ST.MOpen.tx_avg[0][ucloopCh_Y].raw.data);
			else
				fprintf(fp, "      D_%-3d           ,%-7d,\n", ucloopCh_Y+1, ST.MOpen.tx_avg[0][ucloopCh_Y].raw.data);
		}
	}
	fprintf(fp, "\n\n");

	fprintf(fp, "      Frame_1       ,");
	for (ucloopCh_X = 1; ucloopCh_X <= ptl.x_ch; ucloopCh_X++)
		fprintf(fp, "(X_%d%-*c,", ucloopCh_X, 4 - count_digit(ucloopCh_X), ')');
	fprintf(fp, "(Y_Avg),\n");

	for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch; ucloopCh_Y++) {
		fprintf(fp, "      Y_%-3d         ,", ucloopCh_Y + 1);
		for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch; ucloopCh_X++)
			fprintf(fp, "%-7d,", ST.open_daltc[ucloopCh_X][ucloopCh_Y]);

		if (ST.Tx_Avdiff_daltc[ucloopCh_Y].status == NODE_FAIL)
			fprintf(fp, "*%-6d,\n", ST.Tx_Avdiff_daltc[ucloopCh_Y].data);
		else
			fprintf(fp, "%-7d,\n", ST.Tx_Avdiff_daltc[ucloopCh_Y].data);
	}
	fprintf(fp, "\n");
}

void vfSaveOpenTestLog_V3(FILE *fp)
{
	unsigned char ucloopCh_X;
	unsigned char ucloopCh_Y;
	unsigned char ucloop;

	if ((ucSensorTestResult & OPEN_TEST) == OPEN_TEST)
		fprintf(fp, "[Open_Test]                      ,NG ,\n\n");
	else
		fprintf(fp, "[Open_Test]                      ,OK ,\n\n");

	fprintf(fp, "      (Spec.)                    ,\n");
	fprintf(fp, "      Frame_Count                ,%d,\n", ST.Open_FrameCount);
	fprintf(fp, "      Min_Threshold              ,%d,\n", ST.Open_Threshold);
	fprintf(fp, "      Key_Threshold              ,%d,\n\n\n", ST.Open.key_thr);

	fprintf(fp, "      Normal        ,");

	for (ucloopCh_X = 1; ucloopCh_X <= ptl.x_ch; ucloopCh_X++)
		fprintf(fp, "(X_%d%-*c,", ucloopCh_X, 4 - count_digit(ucloopCh_X), ')');
	fprintf(fp, "\n");

	for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch; ucloopCh_Y++) {
		fprintf(fp, "      Y_%-3d         ,", ucloopCh_Y + 1);

		for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch; ucloopCh_X++) {
			if (ST.open_daltc[ucloopCh_X][ucloopCh_Y] < ST.Open_Threshold)
			    	fprintf(fp, "*%-6d,", ST.open_daltc[ucloopCh_X][ucloopCh_Y]);
			else
				fprintf(fp, "%-7d,", ST.open_daltc[ucloopCh_X][ucloopCh_Y]);
		}
		fprintf(fp, "\n");
	}
	if (ptl.key_mode == ILITEK_HW_KEY_MODE) {
		fprintf(fp, "      Key_1         ,");
		for (ucloop = 0; ucloop < ptl.key_num; ucloop++) {
			if (ST.Open.key_daltc[ucloop].data < ST.Open.key_thr)
				fprintf(fp, "*%-6d,", ST.Open.key_daltc[ucloop].data);
			else
				fprintf(fp, "%-7d,", ST.Open.key_daltc[ucloop].data);
		}
		fprintf(fp, "\n");
	}

	if (!ST.useINI) {
		fprintf(fp, "\n");
		fprintf(fp, "   Open_Data_Delta_RX____,");
		for (ucloopCh_X = 1; ucloopCh_X < ptl.x_ch; ucloopCh_X++)
			fprintf(fp, "(X_%d%-*c,", ucloopCh_X, 4 - count_digit(ucloopCh_X), ')');
		fprintf(fp, "\n");

		for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch; ucloopCh_Y++) {
			fprintf(fp, "      Delta_RX_Y_%-3d    ,", ucloopCh_Y + 1);
			for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch - 1; ucloopCh_X++)
				fprintf(fp, "%-7d,", ST.open_Rx_diff[ucloopCh_X][ucloopCh_Y].data);
			fprintf(fp, "\n");
		}
		fprintf(fp, "\n");

		fprintf(fp, "   Open_Data_Delta_TX____,");
		for (ucloopCh_X = 1; ucloopCh_X <= ptl.x_ch; ucloopCh_X++)
			fprintf(fp, "(X_%d%-*c,", ucloopCh_X, 4 - count_digit(ucloopCh_X), ')');
		fprintf(fp, "\n");

		for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch - 1; ucloopCh_Y++) {
			fprintf(fp, "      Delta_TX_Y_%-3d    ,", ucloopCh_Y + 1);
			for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch; ucloopCh_X++)
				fprintf(fp, "%-7d,", ST.open_Tx_diff[ucloopCh_X][ucloopCh_Y].data);
			fprintf(fp, "\n");
		}
		fprintf(fp, "\n");
	}

	fprintf(fp, "\n\n");
}

void vfSaveAllNodeTestLog(FILE *fp)
{
	unsigned char loopCh_X;
	unsigned char loopCh_Y;

 	if (!ST.useINI) {
		if ((ucSensorTestResult & ALL_NODE_TEST) == ALL_NODE_TEST)
			fprintf(fp, "[All_Node_Test]          ,NG ,\n");
		else
			fprintf(fp, "[All_Node_Test]          ,OK ,\n");

		fprintf(fp, "   All_Node_Maximum______________,%d\n", ST.AllNode_Maximum);
		fprintf(fp, "   All_Node_Minimum______________,%d\n", ST.AllNode_Minimum);
		fprintf(fp, "   All_Node_Delta_Threshold______,%d\n", ST.AllNode_Delta_Threshold);
		fprintf(fp, "   All_Node_Panel_Tolerance______,%d\n", ST.AllNode_Panel_Tolerance);
		fprintf(fp, "   All_Node_TX_Tolerance_________,%d\n\n", ST.AllNode_TX_Tolerance);

		fprintf(fp, "   Data_____________________,");
		for (loopCh_X = 0; loopCh_X < ptl.x_ch; loopCh_X++)
			fprintf(fp, "  Rx%02d  ,", loopCh_X);
		fprintf(fp, "\n");
		for (loopCh_Y = 0; loopCh_Y < ptl.y_ch; loopCh_Y++) {
			fprintf(fp, "   Data_________________Tx%02d,", loopCh_Y);
			for (loopCh_X = 0; loopCh_X < ptl.x_ch; loopCh_X++)
				fprintf(fp, "%5d   ,", ST.all_daltc[loopCh_X][loopCh_Y]);
			fprintf(fp, "\n");
		}
		fprintf(fp, "\n\n");

		fprintf(fp, "   Delta_threshold_Data_____,");
		for (loopCh_X = 0; loopCh_X < ptl.x_ch - 1; loopCh_X++)
			fprintf(fp, "  Rx%02d  ,", loopCh_X);
		fprintf(fp, "\n");
		for (loopCh_Y = 0; loopCh_Y < ptl.y_ch - 1; loopCh_Y++) {
			fprintf(fp, "   Delta_threshold_Data_Tx%02d,", loopCh_Y);
			for (loopCh_X = 0; loopCh_X < ptl.x_ch - 1; loopCh_X++)
				fprintf(fp, "%5d   ,", ST.all_w2_data[loopCh_X][loopCh_Y]);
			fprintf(fp, "\n");
		}
		fprintf(fp, "\n");

		return;
	}

	if ((ucSensorTestResult & ALL_NODE_TEST) == ALL_NODE_TEST)
		fprintf(fp, "[Uniformity_Test]                ,NG ,\n\n");
	else
		fprintf(fp, "[Uniformity_Test]                ,OK ,\n\n");
	fprintf(fp, "      (Spec.)                    ,\n");

	if (ST.Uniformity.En_allraw) {
		fprintf(fp, "      RawData_Max     ,");
		for (loopCh_X = 1; loopCh_X <= ptl.x_ch; loopCh_X++)
			fprintf(fp, "(X_%d%-*c,", loopCh_X, 5 - count_digit(loopCh_X), ')');
		fprintf(fp, "\n");
		for (loopCh_Y = 0; loopCh_Y < ptl.y_ch; loopCh_Y++) {
			fprintf(fp, "      Y_%-3d           ,", loopCh_Y + 1);
			for (loopCh_X = 0; loopCh_X < ptl.x_ch; loopCh_X++)
				fprintf(fp, "%-8d,", ST.Uniformity.allraw[loopCh_X][loopCh_Y].ini.max);
			fprintf(fp, "\n");
		}

		fprintf(fp, "      RawData_Min     ,");
		for (loopCh_X = 1; loopCh_X <= ptl.x_ch; loopCh_X++)
			fprintf(fp, "(X_%d%-*c,", loopCh_X, 5 - count_digit(loopCh_X), ')');
		fprintf(fp, "\n");
		for (loopCh_Y = 0; loopCh_Y < ptl.y_ch; loopCh_Y++) {
			fprintf(fp, "      Y_%-3d           ,", loopCh_Y + 1);
			for (loopCh_X = 0; loopCh_X < ptl.x_ch; loopCh_X++)
				fprintf(fp, "%-8d,", ST.Uniformity.allraw[loopCh_X][loopCh_Y].ini.min);
			fprintf(fp, "\n");
		}
	}
	if (ST.Uniformity.En_allwin1) {
		fprintf(fp, "      Win1_Max        ,");
		for (loopCh_X = 0; loopCh_X < ptl.x_ch; loopCh_X++)
			fprintf(fp, "(X_%d%-*c,", loopCh_X, 5 - count_digit(loopCh_X), ')');
		fprintf(fp, "\n");
		for (loopCh_Y = 0; loopCh_Y < ptl.y_ch - 1; loopCh_Y++) {
			fprintf(fp, "      Y_%-3d           ,", loopCh_Y + 1);
			for (loopCh_X = 0; loopCh_X < ptl.x_ch; loopCh_X++)
				fprintf(fp, "%-8d,", ST.Uniformity.allwin1[loopCh_X][loopCh_Y].ini.max);
			fprintf(fp, "\n");
		}
	}
	if (ST.Uniformity.En_allwin2) {
		fprintf(fp, "      Win2_Max        ,");
		for (loopCh_X = 1; loopCh_X <= ptl.x_ch; loopCh_X++)
			fprintf(fp, "(X_%d%-*c,", loopCh_X, 5 - count_digit(loopCh_X), ')');
		fprintf(fp, "\n");
		for (loopCh_Y = 0; loopCh_Y < ptl.y_ch - 1; loopCh_Y++) {
			fprintf(fp, "      Y_%-3d           ,", loopCh_Y + 1);
			for (loopCh_X = 0; loopCh_X < ptl.x_ch - 1; loopCh_X++)
				fprintf(fp, "%-8d,", ST.Uniformity.allwin2[loopCh_X][loopCh_Y].ini.max);
			fprintf(fp, "\n");
		}
	}

	fprintf(fp, "\n\n\n");

	if (ST.Uniformity.En_allraw) {
		fprintf(fp, "      (Uniformity_RawData_Max)   ,");
		fprintf(fp, "        %s, (FailPoints:%5d %s %5d),\n",
			(ST.Uniformity.RawMaxPass) ? "OK" : "NG",
			ST.Uniformity.RawMaxFailCount,
			(ST.Uniformity.RawMaxPass) ? "<=" : ">",
			ST.Uniformity.Up_FailCount);

		fprintf(fp, "      (Uniformity_RawData_Min)   ,");
		fprintf(fp, "        %s, (FailPoints:%5d %s %5d),\n",
			(ST.Uniformity.RawMinPass) ? "OK" : "NG",
			ST.Uniformity.RawMinFailCount,
			(ST.Uniformity.RawMinPass) ? "<=" : ">",
			ST.Uniformity.Low_FailCount);
	} else {
		fprintf(fp, "      (Uniformity_RawData_Max)   ,  Not Test,\n");
		fprintf(fp, "      (Uniformity_RawData_Min)   ,  Not Test,\n");
	}

	if (ST.Uniformity.En_allwin1) {
		fprintf(fp, "      (Uniformity_Win1_Max)      ,");
		fprintf(fp, "        %s, (FailPoints:%5d %s %5d),\n",
			(ST.Uniformity.Win1Pass) ? "OK" : "NG",
			ST.Uniformity.Win1FailCount,
			(ST.Uniformity.Win1Pass) ? "<=" : ">",
			ST.Uniformity.Win1_FailCount);
	} else {
		fprintf(fp, "      (Uniformity_Win1_Max)      ,  Not Test,\n");
	}

	if (ST.Uniformity.En_allwin2) {
		fprintf(fp, "      (Uniformity_Win2_Max)      ,");
		fprintf(fp, "        %s, (FailPoints:%5d %s %5d),\n",
			(ST.Uniformity.Win2Pass) ? "OK" : "NG",
			ST.Uniformity.Win2FailCount,
			(ST.Uniformity.Win2Pass) ? "<=" : ">",
			ST.Uniformity.Win2_FailCount);
	} else {
		fprintf(fp, "      (Uniformity_Win2_Max)      ,  Not Test,\n");
	}

	if (ST.Uniformity.En_allraw) {
		fprintf(fp, "\n");
		fprintf(fp, "      RawData_Max     ,");
		for (loopCh_X = 1; loopCh_X <= ptl.x_ch; loopCh_X++)
			fprintf(fp, "(X_%d%-*c,", loopCh_X, 5 - count_digit(loopCh_X), ')');
		fprintf(fp, "\n");
		for (loopCh_Y = 0; loopCh_Y < ptl.y_ch; loopCh_Y++) {
			fprintf(fp, "      Y_%-3d           ,", loopCh_Y + 1);
			for (loopCh_X = 0; loopCh_X < ptl.x_ch; loopCh_X++) {
				if (ST.Uniformity.allraw[loopCh_X][loopCh_Y].raw.max_st == NODE_FAIL)
					fprintf(fp, "*%-7d,", ST.all_daltc[loopCh_X][loopCh_Y]);
				else
					fprintf(fp, "%-8d,", ST.all_daltc[loopCh_X][loopCh_Y]);
			}
			fprintf(fp, "\n");
		}
		fprintf(fp, "\n");
		fprintf(fp, "      RawData_Min     ,");
		for (loopCh_X = 1; loopCh_X <= ptl.x_ch; loopCh_X++)
			fprintf(fp, "(X_%d%-*c,", loopCh_X, 5 - count_digit(loopCh_X), ')');
		fprintf(fp, "\n");
		for (loopCh_Y = 0; loopCh_Y < ptl.y_ch; loopCh_Y++) {
			fprintf(fp, "      Y_%-3d           ,", loopCh_Y + 1);
			for (loopCh_X = 0; loopCh_X < ptl.x_ch; loopCh_X++) {
				if (ST.Uniformity.allraw[loopCh_X][loopCh_Y].raw.min_st == NODE_FAIL)
					fprintf(fp, "*%-7d,", ST.all_daltc[loopCh_X][loopCh_Y]);
				else
					fprintf(fp, "%-8d,", ST.all_daltc[loopCh_X][loopCh_Y]);
			}
			fprintf(fp, "\n");
		}
	}

	if (ST.Uniformity.En_allwin1) {
		fprintf(fp, "\n");
		fprintf(fp, "      Win1_Max        ,");
		for (loopCh_X = 0; loopCh_X < ptl.x_ch; loopCh_X++)
			fprintf(fp, "(X_%d%-*c,", loopCh_X, 5 - count_digit(loopCh_X), ')');
		fprintf(fp, "\n");
		for (loopCh_Y = 0; loopCh_Y < ptl.y_ch - 1; loopCh_Y++) {
			fprintf(fp, "      Y_%-3d           ,", loopCh_Y + 1);
			for (loopCh_X = 0; loopCh_X < ptl.x_ch; loopCh_X++) {
				if (ST.Uniformity.allwin1[loopCh_X][loopCh_Y].raw.max_st == NODE_FAIL)
					fprintf(fp, "*%-7d,", ST.all_w1_data[loopCh_X][loopCh_Y]);
				else
					fprintf(fp, "%-8d,", ST.all_w1_data[loopCh_X][loopCh_Y]);
			}
			fprintf(fp, "\n");
		}
	}

	if (ST.Uniformity.En_allwin2) {
		fprintf(fp, "\n");
		fprintf(fp, "      Win2_Max        ,");
		for (loopCh_X = 1; loopCh_X <= ptl.x_ch; loopCh_X++)
			fprintf(fp, "(X_%d%-*c,", loopCh_X, 5 - count_digit(loopCh_X), ')');
		fprintf(fp, "\n");
		for (loopCh_Y = 0; loopCh_Y < ptl.y_ch - 1; loopCh_Y++) {
			fprintf(fp, "      Y_%-3d           ,", loopCh_Y + 1);
			for (loopCh_X = 0; loopCh_X < ptl.x_ch - 1; loopCh_X++) {
				if (ST.Uniformity.allwin2[loopCh_X][loopCh_Y].raw.max_st == NODE_FAIL)
					fprintf(fp, "*%-7d,", ST.all_w2_data[loopCh_X][loopCh_Y]);
				else
					fprintf(fp, "%-8d,", ST.all_w2_data[loopCh_X][loopCh_Y]);
			}
			fprintf(fp, "\n");
		}
	}
	fprintf(fp, "\n\n");
}

void vfSaveDACTestLog(FILE *fp)
{
	unsigned char ucloopCh_X;
	unsigned char ucloopCh_Y;

	if ((ucSensorTestResult & ALL_NODE_TEST) == ALL_NODE_TEST)
	{
		fprintf(fp, "[DAC_Test]               ,NG ,\n");
	}
	else
	{
		fprintf(fp, "[DAC_Test]               ,OK ,\n");
	}

	fprintf(fp, "   SC_P_Max______________,%d\n", ST.DAC_SC_P_Maximum);

	fprintf(fp, "   SC_P_Min______________,%d\n", ST.DAC_SC_P_Minimum);

	fprintf(fp, "   MC_P_Max______________,%d\n", ST.DAC_SC_N_Maximum);

	fprintf(fp, "   MC_P_Min______________,%d\n", ST.DAC_SC_N_Minimum);

	fprintf(fp, "   SC_N_Max______________,%d\n", ST.DAC_MC_P_Maximum);

	fprintf(fp, "   SC_N_Min______________,%d\n", ST.DAC_MC_P_Minimum);

	fprintf(fp, "   MC_N_Max______________,%d\n", ST.DAC_MC_N_Maximum);

	fprintf(fp, "   MC_N_Min______________,%d\n", ST.DAC_MC_N_Minimum);

	//DAC Positive data
	fprintf(fp, "\n   DAC_SC_P_X____,");

	for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch; ucloopCh_X++)
	{
		fprintf(fp, "   X%02d ,", ucloopCh_X);
	}
	fprintf(fp, "\n   DAC_SC_P_X_000,");

	for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch; ucloopCh_X++)
	{
		fprintf(fp, "   %3d ,", ST.dac_sc_p[ucloopCh_X]);
	}
	fprintf(fp, "\n   DAC_SC_P_Y____,");

	for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch; ucloopCh_Y++)
	{
		fprintf(fp, "   Y%02d ,", ucloopCh_Y);
	}
	fprintf(fp, "\n   DAC_SC_P_Y_000,");

	for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch; ucloopCh_Y++)
	{
		fprintf(fp, "   %3d ,", ST.dac_sc_p[ucloopCh_Y+ptl.x_ch]);

	}

	fprintf(fp, "\n\n   DAC_MC_P______,");

	for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch; ucloopCh_X++)
	{
		fprintf(fp, "   X%02d ,", ucloopCh_X);

	}
	fprintf(fp, "\n");

	for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch; ucloopCh_Y++)
	{
		fprintf(fp, "   DAC_MC_P___%03d,", ucloopCh_Y);

		for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch; ucloopCh_X++)
		{
			fprintf(fp,"   %3d ,", ST.dac_mc_p[ucloopCh_X][ucloopCh_Y]);
		}
		fprintf(fp, "\n");
	}
	//DAC Negative data
	fprintf(fp, "\n\n   DAC_SC_N_X____,");

	for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch; ucloopCh_X++)
	{
		fprintf(fp, "   X%02d ,", ucloopCh_X);

	}
	fprintf(fp, "\n   DAC_SC_N_X_000,");

	for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch; ucloopCh_X++)
	{
		fprintf(fp, "   %3d ,", ST.dac_sc_n[ucloopCh_X]);

	}
	fprintf(fp, "\n   DAC_SC_N_Y____,");

	for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch; ucloopCh_Y++)
	{
		fprintf(fp, "   Y%02d ,", ucloopCh_Y);

	}
	fprintf(fp, "\n   DAC_SC_N_Y_000,");

	for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch; ucloopCh_Y++)
	{
		fprintf(fp, "   %3d ,", ST.dac_sc_n[ucloopCh_Y+ptl.x_ch]);

	}

	fprintf(fp, "\n\n   DAC_MC_N______,");

	for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch; ucloopCh_X++)
	{
		fprintf(fp, "   X%02d ,", ucloopCh_X);

	}
	fprintf(fp, "\n");

	for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch; ucloopCh_Y++)
	{
		fprintf(fp, "   DAC_MC_N___%03d,", ucloopCh_Y);

		for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch; ucloopCh_X++)
		{
			fprintf(fp,"   %3d ,", ST.dac_mc_n[ucloopCh_X][ucloopCh_Y]);
		}
		fprintf(fp, "\n");

	}
	fprintf(fp, "\n");
}
void vfConverDataFormat()
{
	unsigned char ucloopCh_X;
	unsigned char ucloopCh_Y;
	short int temp[_MaxChanelNum_][_MaxChanelNum_];

	for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch; ucloopCh_Y++)
	{
		for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch; ucloopCh_X++)
		{
			temp[ucloopCh_Y][ucloopCh_X] = uiTestDatas[ucloopCh_Y][ucloopCh_X];
		}
	}
	for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch; ucloopCh_Y++)
	{
		for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch; ucloopCh_X++)
		{
			uiTestDatas[ucloopCh_X][ucloopCh_Y] = temp[ucloopCh_Y][ucloopCh_X];
		}
	}
}


int viRunUniformityTest_3X()
{
	int ret = _SUCCESS;
	int inBD_TopAvg,inBD_BottomAvg,inBD_RightAvg,inBD_Left_Avg,inVAAvg;
	int inIndexCounts;
	int inSum;
	int inDeltaValue1;
	unsigned int ucloopCh_X;
	unsigned int ucloopCh_Y;

	LD_MSG("\n");
	LD_MSG("Uniformity Test Criteria:\n");
	LD_MSG("BD_Top_Ratio = %d\n",ST.Uniformity.uiBD_Top_Ratio);
	LD_MSG("BD_Bottom_Ratio = %d\n",ST.Uniformity.uiBD_Bottom_Ratio);
	LD_MSG("BD_L_Ratio = %d\n",ST.Uniformity.uiBD_L_Ratio);
	LD_MSG("BD_R_Ratio = %d\n",ST.Uniformity.uiBD_R_Ratio);
	LD_MSG("VA_Ratio_X_diff = %d\n",ST.Uniformity.uiVA_Ratio_X_diff);
	LD_MSG("VA_Ratio_Y_diff = %d\n",ST.Uniformity.uiVA_Ratio_Y_diff);
	LD_MSG("BD_VA_L_Ratio_Max = %d\n",ST.Uniformity.uiBD_VA_L_Ratio_Max);
	LD_MSG("BD_VA_L_Ratio_Min = %d\n",ST.Uniformity.uiBD_VA_L_Ratio_Min);
	LD_MSG("BD_VA_R_Ratio_Max = %d\n",ST.Uniformity.uiBD_VA_R_Ratio_Max);
	LD_MSG("BD_VA_R_Ratio_Min = %d\n",ST.Uniformity.uiBD_VA_R_Ratio_Min);
	LD_MSG("BD_VA_Top_Ratio_Max = %d\n",ST.Uniformity.uiBD_VA_Top_Ratio_Max);
	LD_MSG("BD_VA_Top_Ratio_Min = %d\n",ST.Uniformity.uiBD_VA_Top_Ratio_Min);
	LD_MSG("BD_VA_Bottom_Ratio_Max = %d\n",ST.Uniformity.uiBD_VA_Bottom_Ratio_Max);
	LD_MSG("BD_VA_Bottom_Ratio_Min = %d\n",ST.Uniformity.uiBD_VA_Bottom_Ratio_Min);
	LD_MSG("PanelLeftTopULimit = %d\n",ST.Uniformity.uiPanelLeftTopULimit);
	LD_MSG("PanelLeftTopLLimit = %d\n",ST.Uniformity.uiPanelLeftTopLLimit);
	LD_MSG("PanelLeftBottomULimit = %d\n",ST.Uniformity.uiPanelLeftBottomULimit);
	LD_MSG("PanelLeftBottomLLimit = %d\n",ST.Uniformity.uiPanelLeftBottomLLimit);
	LD_MSG("PanelRightTopULimit = %d\n",ST.Uniformity.uiPanelRightTopULimit);
	LD_MSG("PanelRightTopLLimit = %d\n",ST.Uniformity.uiPanelRightTopLLimit);
	LD_MSG("PanelRightBottomULimit = %d\n",ST.Uniformity.uiPanelRightBottomULimit);
	LD_MSG("PanelRightBottomLLimit = %d\n\n",ST.Uniformity.uiPanelRightBottomLLimit);


	switch (KernelVersion[2]) {
		case RDValue_MCUKernel_CDCVersion_8bit2315_:
		case RDValue_MCUKernel_CDCVersion_16bit2510_:
			if (viInitRawData_3Para_3X(0x0C, 0xE6, 0x14) != _FAIL) {
				if (viGetRawData_3X(0xE6, _FastMode_, ptl.x_ch * ptl.y_ch,
				    _DataFormat_16_Bit_, ptl.x_ch, NULL) != _FAIL) {
					vfConverDataFormat();
					ret = _SUCCESS;
				} else {
					LD_ERR("Error! Get RawData Failed!\n");
					ret = _FAIL;
				}
			} else {
				LD_ERR("Error! Init RawData Failed!\n");
				ret = _FAIL;
			}
			break;
	}

	if (ret == _FAIL)
		return ret;

	LD_MSG("Uniformity Datas: \n");
	for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch; ucloopCh_Y++) {
		LD_MSG("Y_%2dCH:", ucloopCh_Y);
		for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch; ucloopCh_X++) {
			ST.unifor_daltc[ucloopCh_X][ucloopCh_Y] = uiTestDatas[ucloopCh_X][ucloopCh_Y] - ST.OffsetValue;
			LD_MSG("%4d,", ST.unifor_daltc[ucloopCh_X][ucloopCh_Y]);
		}
		LD_MSG("\n");
	}

	inIndexCounts = 0;
	inSum = 0;
	for (ucloopCh_X = 1; ucloopCh_X < ptl.x_ch - 1; ucloopCh_X++) {
		inSum += ST.BenchMark.iUniformityBenchMark[ucloopCh_X][0];
		inIndexCounts++;
	}
	inBD_TopAvg = inSum / inIndexCounts;

	//------------Caculate inBD_BottomAvg 2315 New-------------------------
	inIndexCounts = 0;
	inSum = 0;
	for (ucloopCh_X = 1; ucloopCh_X < ptl.x_ch - 1; ucloopCh_X++) {
		inSum += ST.BenchMark.iUniformityBenchMark[ucloopCh_X][ptl.y_ch - 1];
		inIndexCounts++;
	}
	inBD_BottomAvg = inSum / inIndexCounts;

	//------------Caculate inBD_RightAvg 2315 New-------------------------
	inIndexCounts = 0;
	inSum = 0;
	for (ucloopCh_Y = 1; ucloopCh_Y < ptl.y_ch - 1; ucloopCh_Y++) {
		inSum += ST.BenchMark.iUniformityBenchMark[ptl.x_ch - 1][ucloopCh_Y];
		inIndexCounts++;
	}
	inBD_RightAvg = inSum / inIndexCounts;

	//------------Caculate inBD_LeftAvg 2315 New-------------------------
	inIndexCounts = 0;
	inSum = 0;
	for (ucloopCh_Y = 1; ucloopCh_Y < ptl.y_ch - 1; ucloopCh_Y++) {
		inSum += ST.BenchMark.iUniformityBenchMark[0][ucloopCh_Y];
		inIndexCounts++;
	}
	inBD_Left_Avg = inSum / inIndexCounts;


	//-------Caculate inVAAvg  2315 New -----------------------------------
	inSum = 0;
	inIndexCounts = 0;
	for (ucloopCh_X = 1; ucloopCh_X < ptl.x_ch - 1; ucloopCh_X++) {
		for (ucloopCh_Y = 1; ucloopCh_Y < ptl.y_ch - 1; ucloopCh_Y++) {
			inSum += ST.BenchMark.iUniformityBenchMark[ucloopCh_X][ucloopCh_Y];
			inIndexCounts++;
		}
	}
	inVAAvg = inSum / inIndexCounts;

	//-------Check BD_Top --------------------------------------------------
	//str_2315Thresh.inBD_Top_XDiff = (int)(inBD_TopAvg * ST.Uniformity.uiBD_Top_Ratio * 0.01);

	for (ucloopCh_X = 2; ucloopCh_X < ptl.x_ch - 1; ucloopCh_X++) {
		inDeltaValue1 = abs(ST.unifor_daltc[ucloopCh_X][0] - ST.unifor_daltc[ucloopCh_X - 1][0]);
		if (inDeltaValue1 > (int)(inBD_TopAvg * ST.Uniformity.uiBD_Top_Ratio * 0.01)) {
			ucSensorTestResult |= UNIFORMITY_TEST;
			LD_ERR("[Joe] failed, [%d, %d], abs(%d-%d) = inDeltaValue1: %d > (int)(inBD_TopAvg: %d * ST.Uniformity.uiBD_Top_Ratio: %d * 0.01): %d",
				ucloopCh_X, 0, ST.unifor_daltc[ucloopCh_X][0], ST.unifor_daltc[ucloopCh_X - 1][0],
				inDeltaValue1, inBD_TopAvg, ST.Uniformity.uiBD_Top_Ratio, (int)(inBD_TopAvg * ST.Uniformity.uiBD_Top_Ratio * 0.01));
			return _FAIL;
		}
	}

	//-------Check BD_Bottom --------------------------------------------------
	//str_2315Thresh.inBD_BOttom_XDiff = (int)(inBD_BottomAvg * ST.Uniformity.uiBD_Bottom_Ratio * 0.01);
	for (ucloopCh_X = 2; ucloopCh_X < ptl.x_ch - 1; ucloopCh_X++) {
		inDeltaValue1 = abs(ST.unifor_daltc[ucloopCh_X][ptl.y_ch - 1] - ST.unifor_daltc[ucloopCh_X - 1][ptl.y_ch - 1]);

		if (inDeltaValue1 > (int)(inBD_BottomAvg * ST.Uniformity.uiBD_Bottom_Ratio * 0.01)) {
			ucSensorTestResult |= UNIFORMITY_TEST;
			LD_ERR("[Joe] failed, ucloopCh_X: %d, ucloopCh_Y: %d", ucloopCh_X, ptl.y_ch - 1);
			return _FAIL;
		}
	}

	//------------Check inBD_RightAvg 2315 New-------------------------
	//str_2315Thresh.inBD_Right_YDiff = (int)(inBD_RightAvg * ST.Uniformity.uiBD_R_Ratio * 0.01);

	for (ucloopCh_Y = 2; ucloopCh_Y < ptl.y_ch - 1; ucloopCh_Y++) {
		inDeltaValue1 = abs(ST.unifor_daltc[ptl.x_ch - 1][ucloopCh_Y] - ST.unifor_daltc[ptl.x_ch - 1][ucloopCh_Y - 1]);

		if (inDeltaValue1 > (int)(inBD_RightAvg * ST.Uniformity.uiBD_R_Ratio * 0.01)) {
			LD_ERR("[Joe] failed, [%d, %d], abs(%d-%d) = inDeltaValue1: %d > (int)(inBD_RightAvg: %d * ST.Uniformity.uiBD_R_Ratio: %d * 0.01): %d",
				ptl.x_ch - 1, ucloopCh_Y, ST.unifor_daltc[ptl.x_ch - 1][ucloopCh_Y], ST.unifor_daltc[ptl.x_ch - 1][ucloopCh_Y - 1],
				inDeltaValue1, inBD_RightAvg, ST.Uniformity.uiBD_R_Ratio, (int)(inBD_RightAvg * ST.Uniformity.uiBD_R_Ratio * 0.01));
			ucSensorTestResult |= UNIFORMITY_TEST;
			return _FAIL;
		}
	}

	//------------Check inBD_LeftAvg 2315 New-------------------------
	//str_2315Thresh.inBD_Left_YDiff = (int)(inBD_Left_Avg * ST.Uniformity.uiBD_L_Ratio * 0.01);

	for (ucloopCh_Y = 2; ucloopCh_Y < ptl.y_ch - 1; ucloopCh_Y++) {
		inDeltaValue1 = abs(ST.unifor_daltc[0][ucloopCh_Y] - ST.unifor_daltc[0][ucloopCh_Y - 1]);

		if (inDeltaValue1 > (int)(inBD_Left_Avg * ST.Uniformity.uiBD_L_Ratio * 0.01)) {
			ucSensorTestResult |= UNIFORMITY_TEST;
			LD_ERR("[Joe] failed, ucloopCh_X: %d, ucloopCh_Y: %d", 0, ucloopCh_Y);
			return _FAIL;
		}
	}


	//-------Check inVAAvg Y_Diff 2315 New -----------------------------------
	//str_2315Thresh.inVA_YDiff = (int)(inVAAvg * ST.Uniformity.uiVA_Ratio_Y_diff * 0.01);
	for (ucloopCh_X = 1; ucloopCh_X < ptl.x_ch - 1; ucloopCh_X++) {

		for (ucloopCh_Y = 2; ucloopCh_Y < ptl.y_ch - 1; ucloopCh_Y++) {
			inDeltaValue1 = abs(ST.unifor_daltc[ucloopCh_X][ucloopCh_Y] - ST.unifor_daltc[ucloopCh_X][ucloopCh_Y - 1]);

			if (inDeltaValue1 > (int)(inVAAvg * ST.Uniformity.uiVA_Ratio_Y_diff * 0.01)) {
				ucSensorTestResult |= UNIFORMITY_TEST;
				LD_ERR("[Joe] failed, ucloopCh_X: %d, ucloopCh_Y: %d", ucloopCh_X, ucloopCh_Y);
				return _FAIL;
			}
		}

	}

	//-------Check inVAAvg X_Diff 2315 New -----------------------------------
	//str_2315Thresh.inVA_XDiff =  (int)(inVAAvg * ST.Uniformity.uiVA_Ratio_X_diff * 0.01);
	for (ucloopCh_Y = 1; ucloopCh_Y < ptl.y_ch- 1; ucloopCh_Y++) {
		for (ucloopCh_X = 2; ucloopCh_X < ptl.x_ch - 1; ucloopCh_X++) {
			inDeltaValue1 = abs(ST.unifor_daltc[ucloopCh_X][ucloopCh_Y] - ST.unifor_daltc[ucloopCh_X - 1][ucloopCh_Y]);

			if (inDeltaValue1 > (int)(inVAAvg * ST.Uniformity.uiVA_Ratio_X_diff * 0.01)) {
				ucSensorTestResult |= UNIFORMITY_TEST;
				LD_ERR("[Joe] failed, ucloopCh_X: %d, ucloopCh_Y: %d, inDeltaValue1: %d > (inVAAvg: %d * ST.Uniformity.uiVA_Ratio_X_diff: %d * 0.01): %d",
					ucloopCh_X, ucloopCh_Y, inDeltaValue1, inVAAvg, ST.Uniformity.uiVA_Ratio_X_diff, (int)(inVAAvg * ST.Uniformity.uiVA_Ratio_X_diff * 0.01));
				return _FAIL;
			}
		}
	}

	//-------Check BD/VA_Top --------------------------------------------------
	for (ucloopCh_X = 1; ucloopCh_X < ptl.x_ch - 1; ucloopCh_X++) {
		inDeltaValue1 = ((1.0 * ST.unifor_daltc[ucloopCh_X][0] / ST.unifor_daltc[ucloopCh_X][1]) * 100);

		if (inDeltaValue1 > ST.Uniformity.uiBD_VA_Top_Ratio_Max ||
		    inDeltaValue1 < ST.Uniformity.uiBD_VA_Top_Ratio_Min) {
			ucSensorTestResult |= UNIFORMITY_TEST;
			LD_ERR("[Joe] failed, ucloopCh_X: %d, ucloopCh_Y: %d", ucloopCh_X, 0);
			return _FAIL;
		}
	}

	//-------Check BD/VA_Bottom --------------------------------------------------
	for (ucloopCh_X = 1; ucloopCh_X < ptl.x_ch - 1; ucloopCh_X++) {
		inDeltaValue1 = (int)((1.0 * ST.unifor_daltc[ucloopCh_X][ptl.y_ch - 1] / ST.unifor_daltc[ucloopCh_X][ptl.y_ch - 2]) * 100);

		if (inDeltaValue1 > ST.Uniformity.uiBD_VA_Bottom_Ratio_Max ||
		    inDeltaValue1 < ST.Uniformity.uiBD_VA_Bottom_Ratio_Min) {
			ucSensorTestResult |= UNIFORMITY_TEST;
			LD_ERR("[Joe] failed, ucloopCh_X: %d, ucloopCh_Y: %d", ucloopCh_X, ptl.y_ch - 1);
			return _FAIL;
		}
	}

	//------------Check BD/VA Right ------------------------------------------------
	for (ucloopCh_Y = 1; ucloopCh_Y < ptl.y_ch - 1; ucloopCh_Y++) {
		inDeltaValue1 = (int)((1.0 * ST.unifor_daltc[ptl.x_ch - 1][ucloopCh_Y] / ST.unifor_daltc[ptl.x_ch - 2][ucloopCh_Y]) * 100);

		if (inDeltaValue1 > ST.Uniformity.uiBD_VA_R_Ratio_Max ||
		    inDeltaValue1 < ST.Uniformity.uiBD_VA_R_Ratio_Min) {
			ucSensorTestResult |= UNIFORMITY_TEST;
			LD_ERR("[Joe] failed, ucloopCh_X: %d, ucloopCh_Y: %d", ptl.x_ch - 1, ucloopCh_Y);
			return _FAIL;
		}
	}

	//------------Check BD/VA Left ------------------------------------------------
	for (ucloopCh_Y = 1; ucloopCh_Y < ptl.y_ch - 1; ucloopCh_Y++) {
		inDeltaValue1 = (int)((1.0 * ST.unifor_daltc[0][ucloopCh_Y] / ST.unifor_daltc[1][ucloopCh_Y]) * 100);

		if (inDeltaValue1 > ST.Uniformity.uiBD_VA_L_Ratio_Max ||
		    inDeltaValue1 < ST.Uniformity.uiBD_VA_L_Ratio_Min) {
			ucSensorTestResult |= UNIFORMITY_TEST;
			LD_ERR("[Joe] failed, ucloopCh_X: %d, ucloopCh_Y: %d", 0, ucloopCh_Y);
			return _FAIL;
		}
	}

	//-----------Check Corner----------------------------------------------------------
	//str_2315Thresh.inCorner_LT_Max = (int)(ST.BenchMark.iUniformityBenchMark[0][0] * ST.Uniformity.uiPanelLeftTopULimit * 0.01);
	//str_2315Thresh.inCorner_LT_Min = (int)(ST.BenchMark.iUniformityBenchMark[0][0] * ST.Uniformity.uiPanelLeftTopLLimit * 0.01);
	if (ST.unifor_daltc[0][0] > (int)(ST.BenchMark.iUniformityBenchMark[0][0] * ST.Uniformity.uiPanelLeftTopULimit * 0.01) ||
	    ST.unifor_daltc[0][0] < (int)(ST.BenchMark.iUniformityBenchMark[0][0] * ST.Uniformity.uiPanelLeftTopLLimit * 0.01)) {
		ucSensorTestResult |= UNIFORMITY_TEST;
		LD_ERR("[Joe] failed, ucloopCh_X: %d, ucloopCh_Y: %d", 0, 0);
		return _FAIL;
	}

	//str_2315Thresh.inCorner_LB_Max = (int)(ST.BenchMark.iUniformityBenchMark[0][ptl.y_ch - 1] * ST.Uniformity.uiPanelLeftBottomULimit * 0.01);
	//str_2315Thresh.inCorner_LB_Min = (int)(ST.BenchMark.iUniformityBenchMark[0][ptl.y_ch - 1] * ST.Uniformity.uiPanelLeftBottomLLimit * 0.01);
	if (ST.unifor_daltc[0][ptl.y_ch - 1] > (int)(ST.BenchMark.iUniformityBenchMark[0][ptl.y_ch - 1] * ST.Uniformity.uiPanelLeftBottomULimit * 0.01) ||
	    ST.unifor_daltc[0][ptl.y_ch - 1] < (int)(ST.BenchMark.iUniformityBenchMark[0][ptl.y_ch - 1] * ST.Uniformity.uiPanelLeftBottomLLimit * 0.01)) {
		ucSensorTestResult |= UNIFORMITY_TEST;
		LD_ERR("[Joe] failed, ucloopCh_X: %d, ucloopCh_Y: %d", 0, ptl.y_ch - 1);
		return _FAIL;
	}

	// str_2315Thresh.inCorner_RT_Max = (int)(ST.BenchMark.iUniformityBenchMark[ptl.x_ch - 1][0] * ST.Uniformity.uiPanelRightTopULimit * 0.01);
	// str_2315Thresh.inCorner_RT_Min = (int)(ST.BenchMark.iUniformityBenchMark[ptl.x_ch - 1][0] * ST.Uniformity.uiPanelRightTopLLimit * 0.01);
	if (ST.unifor_daltc[ptl.x_ch - 1][0] > (int)(ST.BenchMark.iUniformityBenchMark[ptl.x_ch - 1][0] * ST.Uniformity.uiPanelRightTopULimit * 0.01) ||
	    ST.unifor_daltc[ptl.x_ch - 1][0] < (int)(ST.BenchMark.iUniformityBenchMark[ptl.x_ch - 1][0] * ST.Uniformity.uiPanelRightTopLLimit * 0.01)) {
		ucSensorTestResult |= UNIFORMITY_TEST;
		LD_ERR("[Joe] failed, ucloopCh_X: %d, ucloopCh_Y: %d", ptl.x_ch - 1, 0);
		return _FAIL;
	}

	//str_2315Thresh.inCornew_RB_Max = (int)(ST.BenchMark.iUniformityBenchMark[ptl.x_ch - 1][ptl.y_ch - 1] * ST.Uniformity.uiPanelRightBottomULimit * 0.01);
	//str_2315Thresh.inCornew_RB_Min = (int)(ST.BenchMark.iUniformityBenchMark[ptl.x_ch - 1][ptl.y_ch - 1] * ST.Uniformity.uiPanelRightBottomLLimit * 0.01);
	if (ST.unifor_daltc[ptl.x_ch - 1][ptl.y_ch - 1] > (int)(ST.BenchMark.iUniformityBenchMark[ptl.x_ch - 1][ptl.y_ch - 1] * ST.Uniformity.uiPanelRightBottomULimit * 0.01) ||
	    ST.unifor_daltc[ptl.x_ch - 1][ptl.y_ch - 1] < (int)(ST.BenchMark.iUniformityBenchMark[ptl.x_ch - 1][ptl.y_ch - 1] * ST.Uniformity.uiPanelRightBottomLLimit * 0.01)) {
		ucSensorTestResult |= UNIFORMITY_TEST;
		LD_ERR("[Joe] failed, ucloopCh_X: %d, ucloopCh_Y: %d", ptl.x_ch - 1, ptl.y_ch - 1);
		return _FAIL;
	}

	return ret;
}

int NodeTest_V3(const char *name, short int **delac, SensorTest_BenBenchmark_Node **data,
		int X_MAX, int Y_MAX, int fail_count_limit, int *fail_count,
		bool check_max) {
	int ret;
	int CHX = 0, CHY = 0;

	for (CHY = 0; CHY < Y_MAX; CHY++) {
		for (CHX = 0; CHX < X_MAX; CHX++) {
			data[CHX][CHY].raw.data = delac[CHX][CHY];

			if (check_max && data[CHX][CHY].raw.data >
			    data[CHX][CHY].ini.max) {
				data[CHX][CHY].raw.status = NODE_FAIL;
				data[CHX][CHY].raw.max_st = NODE_FAIL;
				*fail_count += 1;
			} else if (!check_max && data[CHX][CHY].raw.data <
				   data[CHX][CHY].ini.min) {
				data[CHX][CHY].raw.status = NODE_FAIL;
				data[CHX][CHY].raw.min_st = NODE_FAIL;
				*fail_count += 1;
			}
		}
	}
	ret = (*fail_count > fail_count_limit) ? _FAIL : _SUCCESS;
	LD_MSG("[%s][%s] failcount: %d, threshold: %d\n",
		__func__, name, *fail_count, fail_count_limit);
	return ret;
}

int viRunAllNodeTest_3X()
{
	int ret = 1;
	unsigned int CHX = 0, CHY = 0;
	unsigned char ucPass = 1;
	int PanelFailCount = 0;
	int TXFailCount = 0;
	int TXisFail = 0;
	unsigned char u8DataInfor = _DataFormat_16_Bit_;

	switch (KernelVersion[2]) {
	case RDValue_MCUKernel_CDCVersion_8bit2315_:
		if (ST.AllNode_Maximum == -1)
			ST.AllNode_Maximum = _2315SensorTestAllNodeMaximum_;
		if (ST.AllNode_Minimum == -1)
			ST.AllNode_Minimum = _2315SensorTestAllNodeMinimum_;
		if (ST.AllNode_Delta_Threshold == -1)
			ST.AllNode_Delta_Threshold = _2315SensorTestAllNodeDeltaThreshold_;
		if (ST.AllNode_Panel_Tolerance == -1)
			ST.AllNode_Panel_Tolerance = _2315SensorTestAllNodePanelTolerance_;
		if (ST.AllNode_TX_Tolerance == -1)
			ST.AllNode_TX_Tolerance = _2315SensorTestAllNodeTXTolerance_;
		break;
	case RDValue_MCUKernel_CDCVersion_16bit2510_:
		if (ST.AllNode_Maximum == -1)
			ST.AllNode_Maximum = _2510SensorTestAllNodeMaximum_;
		if (ST.AllNode_Minimum == -1)
			ST.AllNode_Minimum = _2510SensorTestAllNodeMinimum_;
		if (ST.AllNode_Delta_Threshold == -1)
			ST.AllNode_Delta_Threshold = _2510SensorTestAllNodeDeltaThreshold_;
		if (ST.AllNode_Panel_Tolerance == -1)
			ST.AllNode_Panel_Tolerance = _2510SensorTestAllNodePanelTolerance_;
		if (ST.AllNode_TX_Tolerance == -1)
			ST.AllNode_TX_Tolerance = _2510SensorTestAllNodeTXTolerance_;
		break;
	}

	LD_MSG("\n");
	LD_MSG("All Node Test Criteria:\n");
	LD_MSG("Maximum = %d\n",ST.AllNode_Maximum);
	LD_MSG("Minimum = %d\n",ST.AllNode_Minimum);
	LD_MSG("Delta Threshold = %d\n",ST.AllNode_Delta_Threshold);
	LD_MSG("Panel Tolerance = %d\n",ST.AllNode_Panel_Tolerance);
	LD_MSG("TX Tolerance = %d\n\n",ST.AllNode_TX_Tolerance);

	switch (KernelVersion[2]) {
	case RDValue_MCUKernel_CDCVersion_8bit2315_:
	case RDValue_MCUKernel_CDCVersion_16bit2510_:
		if (ptl.ver < PROTOCOL_V3_4_0)
			ret = viInitRawData_3X(TEST_MODE_V3_ALL_NODE,
						TEST_MODE_V3_Y_DRIVEN);
		else
			ret = viInitRawData_3Para_3X(TEST_MODE_V3_ALL_NODE,
							TEST_MODE_V3_Y_DRIVEN,
							TEST_MODE_V3_ALLNODE_WITH_BK);
		if (ret != _FAIL) {
			ret = RawDataInfor();
			u8DataInfor = (ret & (1 << 26)) ? _DataFormat_16_Bit_: _DataFormat_8_Bit_;
			if (viGetRawData_3X(0xE6, _FastMode_, ptl.x_ch * ptl.y_ch,
					    u8DataInfor, ptl.x_ch, NULL) != _FAIL) {
				ret = _SUCCESS;
			} else {
				LD_ERR("Error! Get RawData Failed!\n");
				ret = _FAIL;
			}
		} else {
			LD_ERR("Error! Init RawData Failed!\n");
			ret = _FAIL;
		}
		break;
	}

	if (ret == _FAIL)
		return ret;

	LD_MSG("All Node Datas:\n       ");

	for (CHX = 0; CHX < ptl.x_ch; CHX++)
		LD_MSG("X_%02d,", CHX);
	LD_MSG("\n");
	for (CHY = 0; CHY < ptl.y_ch; CHY++) {
		LD_MSG("Y_%2dCH:", CHY);
		for (CHX = 0; CHX < ptl.x_ch; CHX++) {
			LD_MSG("%4d,", uiTestDatas[CHY][CHX]);
			ST.all_daltc[CHX][CHY] = uiTestDatas[CHY][CHX];

			if (ST.all_daltc[CHX][CHY] > ST.AllNode_Maximum ||
				ST.all_daltc[CHX][CHY] < ST.AllNode_Minimum)
				ucPass = 0;
		}
		LD_MSG("\n");
	}
	LD_MSG("\n");

	LD_MSG("Win1:\n       ");
	for (CHX = 0; CHX < ptl.x_ch - 1; CHX++)
		LD_MSG("X_%02d,", CHX);
	LD_MSG("\n");
	for (CHY = 1; CHY < ptl.y_ch; CHY++) {
		LD_MSG("Y_%2dCH:", CHY - 1);
		for (CHX = 0; CHX < ptl.x_ch; CHX++) {
			ST.all_w1_data[CHX][CHY-1] = abs(uiTestDatas[CHY][CHX] - uiTestDatas[CHY - 1][CHX]);
			LD_MSG("%4d,", ST.all_w1_data[CHX][CHY-1]);
		}
		LD_MSG("\n");
	}
	LD_MSG("\n");

	LD_MSG("Win2:\n       ");
	for (CHX = 0; CHX < ptl.x_ch - 1; CHX++)
		LD_MSG("X_%02d,", CHX);
	LD_MSG("\n");
	for (CHY = 1; CHY < ptl.y_ch; CHY++) {
		LD_MSG("Y_%2dCH:", CHY - 1);
		TXFailCount = 0;
		for (CHX = 1; CHX < ptl.x_ch; CHX++) {
			ST.all_w2_data[CHX-1][CHY-1] = abs((uiTestDatas[CHY][CHX] - uiTestDatas[CHY - 1][CHX]) -
					(uiTestDatas[CHY][CHX - 1] - uiTestDatas[CHY - 1][CHX - 1]));
			LD_MSG("%4d,", ST.all_w2_data[CHX-1][CHY-1]);
			if (ST.all_w2_data[CHX-1][CHY-1] > ST.AllNode_Delta_Threshold) {
				++TXFailCount;
				++PanelFailCount;
			}
		}
		if (TXFailCount >= ST.AllNode_TX_Tolerance) {
			TXisFail = 1;
			PRINTFTEST("\n");
		}
		LD_MSG("\n");
	}

	if (TXisFail == 1 && PanelFailCount >= ST.AllNode_Panel_Tolerance) {
		ucPass = 0;
		PRINTFTEST("\n");
	}

	if (ST.useINI) {
		ucPass = 1;
		ST.Uniformity.RawMaxFailCount = 0;
		ST.Uniformity.RawMinFailCount = 0;
		ST.Uniformity.Win1FailCount = 0;
		ST.Uniformity.Win2FailCount = 0;
		ST.Uniformity.RawMaxPass = true;
		ST.Uniformity.RawMinPass = true;
		ST.Uniformity.Win1Pass = true;
		ST.Uniformity.Win2Pass = true;

		if (ST.Uniformity.En_allraw == ENABLE_TEST &&
		    NodeTest_V3("Raw Max", ST.all_daltc, ST.Uniformity.allraw,
				ptl.x_ch, ptl.y_ch, ST.Uniformity.Up_FailCount,
				&ST.Uniformity.RawMaxFailCount, true) == _FAIL) {
			ST.Uniformity.RawMaxPass = false;
			ucPass = 0;
		}

		if (ST.Uniformity.En_allraw == ENABLE_TEST &&
		    NodeTest_V3("Raw Min", ST.all_daltc, ST.Uniformity.allraw,
				ptl.x_ch, ptl.y_ch, ST.Uniformity.Low_FailCount,
				&ST.Uniformity.RawMinFailCount, false) == _FAIL) {
			ST.Uniformity.RawMinPass = false;
			ucPass = 0;
		}

		if (ST.Uniformity.En_allwin1 == ENABLE_TEST &&
		    NodeTest_V3("Win1", ST.all_w1_data, ST.Uniformity.allwin1,
				ptl.x_ch, ptl.y_ch - 1, ST.Uniformity.Win1_FailCount,
				&ST.Uniformity.Win1FailCount, true) == _FAIL) {
			ST.Uniformity.Win1Pass = false;
			ucPass = 0;
		}

		if (ST.Uniformity.En_allwin2 == ENABLE_TEST &&
		    NodeTest_V3("Win2", ST.all_w2_data, ST.Uniformity.allwin2,
				ptl.x_ch - 1, ptl.y_ch - 1, ST.Uniformity.Win2_FailCount,
				&ST.Uniformity.Win2FailCount, true) == _FAIL) {
			ST.Uniformity.Win2Pass = false;
			ucPass = 0;
		}
	}

	if (ucPass == 0)
		ucSensorTestResult |= ALL_NODE_TEST;

	return ret;
}

int viRunOpenTest_3X_NewFlow()
{
	int ret = _SUCCESS;
	int inDeltaValue1;
	int inDiffAvg;
	int inIndexCounts;
	int inSum;
	int inTempCheckValue;
	int xdiff_fail_count = 0, ydiff_fail_count = 0;
	unsigned int ucloopCh_X;
	unsigned int ucloopCh_Y;
	LD_MSG("\n");
	LD_MSG("Open Test Criteria:\n");
	LD_MSG("Open Test 20V-6V Threshold = %d\n",ST.Open_Threshold);
	LD_MSG("Open Test 20V-6V X Diff Threshold = %d\n",ST.Open_RX_Delta_Threshold);
	LD_MSG("Open Test 20V-6V X Diff Fail count = %d\n",ST.Open_RX_Continue_Fail_Tolerance);
	LD_MSG("Open Test 20V-6V Y Diff Threshold = %d\n",ST.Open_TX_Delta_Threshold);
	LD_MSG("Open Test 20V-6V Y Diff Fail count = %d\n",ST.Open_TX_Continue_Fail_Tolerance);
	LD_MSG("Open_DCRangeMax = %d\n",ST.Open_DCRangeMax);
	LD_MSG("Open_DCRangeMin = %d\n\n",ST.Open_DCRangeMin);

	switch (KernelVersion[2])
	{
		case RDValue_MCUKernel_CDCVersion_8bit2315_:
		case RDValue_MCUKernel_CDCVersion_16bit2510_:
			{
				if (viInitRawData_3Para_3X(0x0C, 0xE6, 0x06) != _FAIL)
				{
					if (viGetRawData_3X(0xE6, _FastMode_, ptl.x_ch * ptl.y_ch, _DataFormat_16_Bit_, ptl.x_ch, NULL) != _FAIL)
					{
						vfConverDataFormat();
						for(ucloopCh_X = 0;ucloopCh_X < _MaxChanelNum_;ucloopCh_X++)
						{
							for(ucloopCh_Y = 0;ucloopCh_Y < _MaxChanelNum_;ucloopCh_Y++)
							{
								uiTestDatas_1[ucloopCh_X][ucloopCh_Y] = uiTestDatas[ucloopCh_X][ucloopCh_Y];
							}
						}
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

				if (viInitRawData_3Para_3X(0x0C, 0xE6, 0x14) != _FAIL)
				{
					if (viGetRawData_3X(0xE6, _FastMode_, ptl.x_ch * ptl.y_ch, _DataFormat_16_Bit_, ptl.x_ch, NULL) != _FAIL)
					{
						vfConverDataFormat();
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
				break;
			}
	}
	if (ret != _FAIL)
	{
		LD_MSG("Open Datas_20V: \n");
		for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch; ucloopCh_Y++)
		{
			LD_MSG("Y_%2dCH:", ucloopCh_Y);
			for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch; ucloopCh_X++)
			{
				ST.open_20V_daltc[ucloopCh_X][ucloopCh_Y] = uiTestDatas[ucloopCh_X][ucloopCh_Y];
				LD_MSG("%4d,", uiTestDatas[ucloopCh_X][ucloopCh_Y]);
			}
			LD_MSG("\n");
		}

		LD_MSG("Open Datas_6V: \n");
		for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch; ucloopCh_Y++)
		{
			LD_MSG("Y_%2dCH:", ucloopCh_Y);
			for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch; ucloopCh_X++)
			{
				LD_MSG("%4d,", uiTestDatas_1[ucloopCh_X][ucloopCh_Y]);
				ST.open_6V_daltc[ucloopCh_X][ucloopCh_Y] = uiTestDatas_1[ucloopCh_X][ucloopCh_Y];
			}
			LD_MSG("\n");
		}
		//-------Caculate inDiffAvg  20V - 6V ---------------------------
		inSum = 0;
		inIndexCounts = 0;
		for (ucloopCh_X = 0;ucloopCh_X < ptl.x_ch;ucloopCh_X++)
		{
			for (ucloopCh_Y = 0;ucloopCh_Y < ptl.y_ch;ucloopCh_Y++)
			{
				inSum +=(ST.BenchMark.iOpenBenchMark_0[ucloopCh_X][ucloopCh_Y] - ST.BenchMark.iOpenBenchMark_1[ucloopCh_X][ucloopCh_Y]);
				inIndexCounts++;
			}
		}
		inDiffAvg = (int)(1.0 * inSum / inIndexCounts);

		//inDiffThreshold = (int)(inDiffAvg * ST.Open_DiffGolden * 0.01);
		//-------Caculate X_DiffAvg 20V - 6V ---------------------------
		inSum = 0;
		inIndexCounts = 0;
		for (ucloopCh_Y = 0;ucloopCh_Y < ptl.y_ch;ucloopCh_Y++)
		{
			for (ucloopCh_X = 1;ucloopCh_X < ptl.x_ch;ucloopCh_X++)
			{
				inSum +=abs((ST.BenchMark.iOpenBenchMark_0[ucloopCh_X][ucloopCh_Y] - ST.BenchMark.iOpenBenchMark_1[ucloopCh_X][ucloopCh_Y]) -
						(ST.BenchMark.iOpenBenchMark_0[ucloopCh_X-1][ucloopCh_Y] - ST.BenchMark.iOpenBenchMark_1[ucloopCh_X-1][ucloopCh_Y]));
				inIndexCounts++;
			}
		}
		//inX_DiffAvg = (int)(1.0 * inSum / inIndexCounts);

		//inX_DiffThreshold = (int)(inDiffAvg * ST.Open_XDiffThreshold * 0.01);
		//-------Caculate Y_DiffAvg 20V - 6V ---------------------------
		inSum = 0;
		inIndexCounts = 0;
		for (ucloopCh_X = 0;ucloopCh_X < ptl.x_ch;ucloopCh_X++)
		{
			for (ucloopCh_Y = 1;ucloopCh_Y < ptl.y_ch;ucloopCh_Y++)
			{
				inSum +=abs((ST.BenchMark.iOpenBenchMark_0[ucloopCh_X][ucloopCh_Y ] - ST.BenchMark.iOpenBenchMark_1[ucloopCh_X][ucloopCh_Y]) -
						(ST.BenchMark.iOpenBenchMark_0[ucloopCh_X][ucloopCh_Y - 1] - ST.BenchMark.iOpenBenchMark_1[ucloopCh_X][ucloopCh_Y - 1]));
				inIndexCounts++;
			}
		}
		//inY_DiffAvg = (int)(1.0 * inSum / inIndexCounts);
		//inY_DiffThreshold = (int)(inDiffAvg * ST.Open_YDiffThreshold * 0.01);

		//Check DiffThreshold   Threshold0--------------------------------------
		for (ucloopCh_X = 0;ucloopCh_X < ptl.x_ch;ucloopCh_X++)
		{
			for (ucloopCh_Y = 0;ucloopCh_Y < ptl.y_ch;ucloopCh_Y++)
			{
				inDeltaValue1 = (uiTestDatas[ucloopCh_X][ucloopCh_Y] - uiTestDatas_1[ucloopCh_X][ucloopCh_Y]);
				//20190314 add by tigers
				ST.open_20_6V_daltc[ucloopCh_X][ucloopCh_Y] = inDeltaValue1;
				//20190314 end
				//if(inDeltaValue1 < (int)(inDiffAvg * ST.Open_DiffGolden * 0.01))
				if(inDeltaValue1 < (int)(inDiffAvg * ST.Open_Threshold * 0.01))
				{
					ucSensorTestResult |= OPEN_TEST;
					//LD_MSG("%s,%d, ucSensorTestResult = %d\n", __func__, __LINE__, ucSensorTestResult);
				}
			}
		}
		LD_MSG("Open Test 20V-6V Threshold = %d\n",ST.Open_Threshold);
		LD_MSG("Open Test 20V-6V X Diff Threshold = %d\n",ST.Open_RX_Delta_Threshold);
		LD_MSG("Open Test 20V-6V X Diff Fail count = %d\n",ST.Open_RX_Continue_Fail_Tolerance);
		LD_MSG("Open Test 20V-6V Y Diff Threshold = %d\n",ST.Open_TX_Delta_Threshold);
		LD_MSG("Open Test 20V-6V X Diff Fail count = %d\n",ST.Open_TX_Continue_Fail_Tolerance);
		LD_MSG("Open_DCRangeMax = %d\n",ST.Open_DCRangeMax);
		LD_MSG("Open_DCRangeMin = %d\n\n",ST.Open_DCRangeMin);
		//-------Check X_DiffAvg 20V - 6V ---------------------------
		for (ucloopCh_Y = 0;ucloopCh_Y < ptl.y_ch;ucloopCh_Y++)
		{
			for (ucloopCh_X = 1;ucloopCh_X < ptl.x_ch;ucloopCh_X++)
			{
				//			if(SpecialWindow.ucOpenXDiff[ucloopCh_X][ucloopCh_Y] !=0
				//			  || SpecialWindow.ucOpenXDiff[ucloopCh_X - 1][ucloopCh_Y] !=0)
				//			{
				//			   inTempCheckValue = 0;
				//			   inTempCheckValue = max(SpecialWindow.ucOpenXDiff[ucloopCh_X - 1][ucloopCh_Y],SpecialWindow.ucOpenXDiff[ucloopCh_X][ucloopCh_Y]);
				//			}
				//			else
				{
					//inTempCheckValue = ST.Open_XDiffThreshold;
					inTempCheckValue = ST.Open_RX_Delta_Threshold;
				}
				inDeltaValue1=abs((uiTestDatas[ucloopCh_X][ucloopCh_Y] - uiTestDatas_1[ucloopCh_X][ucloopCh_Y]) -
						(uiTestDatas[ucloopCh_X-1][ucloopCh_Y] - uiTestDatas_1[ucloopCh_X-1][ucloopCh_Y]));
				//20190314 add by tigers
				ST.open_Rx_diff[ucloopCh_X][ucloopCh_Y].data = inDeltaValue1;
				//20190314 end

				if(inDeltaValue1 > (int)(inDiffAvg * inTempCheckValue * 0.01))
				{
					xdiff_fail_count++;
					if(ST.Open_RX_Continue_Fail_Tolerance <= xdiff_fail_count)
					{
						ucSensorTestResult |= OPEN_TEST;
						//LD_MSG("%s,%d, ucSensorTestResult = %d\n", __func__, __LINE__, ucSensorTestResult);
					}
				}
			}
		}

		//-------Check Y_DiffAvg 20V - 6V ---------------------------

		for (ucloopCh_X = 0;ucloopCh_X < ptl.x_ch;ucloopCh_X++)
		{
			for (ucloopCh_Y = 1;ucloopCh_Y < ptl.y_ch;ucloopCh_Y++)
			{
				//			 if(SpecialWindow.ucOpenYDiff[ucloopCh_X][ucloopCh_Y - 1] !=0
				//			  || SpecialWindow.ucOpenYDiff[ucloopCh_X][ucloopCh_Y] !=0)
				//			  {
				//				 inTempCheckValue = 0;
				//				 inTempCheckValue = max(SpecialWindow.ucOpenYDiff[ucloopCh_X][ucloopCh_Y - 1],SpecialWindow.ucOpenYDiff[ucloopCh_X][ucloopCh_Y]);
				//			  }
				//			  else
				{
					//inTempCheckValue = ST.Open_YDiffThreshold;
					inTempCheckValue = ST.Open_TX_Delta_Threshold;
				}
				inDeltaValue1 =abs((uiTestDatas[ucloopCh_X][ucloopCh_Y ] - uiTestDatas_1[ucloopCh_X][ucloopCh_Y]) -
						(uiTestDatas[ucloopCh_X][ucloopCh_Y - 1] - uiTestDatas_1[ucloopCh_X][ucloopCh_Y - 1]));
				//20190314 add by tigers
				ST.open_Tx_diff_new[ucloopCh_X][ucloopCh_Y] = inDeltaValue1;
				//20190314 end
				if(inDeltaValue1 > (int)(inDiffAvg * inTempCheckValue * 0.01))
				{
					ydiff_fail_count++;
					if(ST.Open_TX_Continue_Fail_Tolerance <= ydiff_fail_count)
					{
						ucSensorTestResult |= OPEN_TEST;
						//LD_MSG("%s,%d, ucSensorTestResult = %d\n", __func__, __LINE__, ucSensorTestResult);
					}
				}
			}
		}

		//---------Check DC Range----------------------------------------------------------
		for (ucloopCh_X = 0;ucloopCh_X < ptl.x_ch;ucloopCh_X++)
		{
			for (ucloopCh_Y = 0;ucloopCh_Y < ptl.y_ch;ucloopCh_Y++)
			{
				//  inDeltaValue1 = (uiTestDatas[ucloopCh_X][ucloopCh_Y] - uiTestDatas_1[ucloopCh_X][ucloopCh_Y]);
				//20190314 add by tigers
				ST.uiYDC_Range[ucloopCh_X][ucloopCh_Y] = uiTestDatas[ucloopCh_X][ucloopCh_Y] - ST.OffsetValue;
				//20190314 end
				if(ST.uiYDC_Range[ucloopCh_X][ucloopCh_Y] > (int)(ST.BenchMark.iOpenBenchMark_0[ucloopCh_X][ucloopCh_Y ] * ST.Open_DCRangeMax * 0.01)
						|| ST.uiYDC_Range[ucloopCh_X][ucloopCh_Y] < (int)(ST.BenchMark.iOpenBenchMark_0[ucloopCh_X][ucloopCh_Y ] * ST.Open_DCRangeMin * 0.01))
				{
					//  LD_MSG("%s,%d, ucSensorTestResult = %d,data[%d][%d] = %d, golden[%d][%d] = %d,%d,%d\n", __func__, __LINE__, ucSensorTestResult, ucloopCh_X, ucloopCh_Y
					//  , ST.uiYDC_Range[ucloopCh_X][ucloopCh_Y], ucloopCh_X, ucloopCh_Y, ST.BenchMark.iOpenBenchMark_0[ucloopCh_X][ucloopCh_Y],
					//  (int)(ST.BenchMark.iOpenBenchMark_0[ucloopCh_X][ucloopCh_Y ] * ST.Open_DCRangeMax * 0.01),
					//  (int)(ST.BenchMark.iOpenBenchMark_0[ucloopCh_X][ucloopCh_Y ] * ST.Open_DCRangeMin * 0.01));
					ucSensorTestResult |= OPEN_TEST;
				}
			}
		}
		//vfSaveOpenTestLog_NewFlow(ST.LogPath);
	}
	return ret;

}

int OpenTestKeyThreshold() {
	int ret = _SUCCESS;
	unsigned int count = 0;

	LD_MSG("Key_Threshold = %d\n\n", ST.Open.key_thr);
	LD_MSG("Open Key Datas\n       ");
	for (count = 0; count < ptl.key_num; count++)
		LD_MSG("%6d,", count + 1);
	if(viGetCDCData_6X(TEST_MODE_OPEN_KEY, ptl.key_num) < 0)
		return _FAIL;
	LD_MSG("\n  data:");
	for(count = 0; count < ptl.key_num; count++) {
		ST.Open.key_daltc[count].data = uiTestDatas[count/ptl.x_ch][count%ptl.x_ch];
		if(ST.Open.key_daltc[count].data < ST.Open.key_thr) {
			LD_MSG("*%5d,", ST.Open.key_daltc[count].data);
			ST.Open.key_daltc[count].status = NODE_FAIL;
			ret = _FAIL;
		}
		else {
			LD_MSG("%6d,", ST.Open.key_daltc[count].data);
			ST.Open.key_daltc[count].status = NODE_PASS;
		}
	}
	LD_MSG("\n");

	return ret;
}

int OpenTestThreshold() {
	unsigned int CHX = 0, CHY = 0;
	int ret = _SUCCESS;

	LD_MSG("Open Datas: \n       ");
	for (CHX = 0; CHX < ptl.x_ch; CHX++) {
		LD_MSG(" %3dCH,", CHX);
	}
	LD_MSG("\n");
	for (CHY = 0; CHY < ptl.y_ch; CHY++)
	{
		LD_MSG("Y_%2dCH:", CHY);
		for (CHX = 0; CHX < ptl.x_ch; CHX++)
		{
			ST.v6_open_daltc[CHX][CHY].data = uiTestDatas[CHY][CHX];
			if (ST.v6_open_daltc[CHX][CHY].data < ST.Open_Threshold)
			{
				ST.v6_open_daltc[CHX][CHY].status = NODE_FAIL;
				LD_MSG("*%5d,", ST.v6_open_daltc[CHX][CHY].data);
				ret = _FAIL;
			}
			else {
				ST.v6_open_daltc[CHX][CHY].status = NODE_PASS;
				LD_MSG("%6d,", ST.v6_open_daltc[CHX][CHY].data);
			}
		}
		LD_MSG("\n");
	}

	return ret;
}

void OpenTestTxDiff() {
	unsigned int CHX = 0, CHY = 0;
	int diff = 0;

	LD_MSG("Tx Diff: \n       ");
	for (CHX = 0; CHX < ptl.x_ch - 1; CHX++)
		LD_MSG(" %3dCH,", CHX);
	LD_MSG("\n");
	for (CHY = 0; CHY < ptl.y_ch - 1; CHY++) {
		LD_MSG("Y_%2dCH:", CHY);
		for (CHX = 0; CHX < ptl.x_ch; CHX++) {
			diff = abs(uiTestDatas[CHY][CHX] - uiTestDatas[CHY + 1][CHX]);
			ST.open_Tx_diff[CHX][CHY].data = diff;
			LD_MSG("%5d,", ST.open_Tx_diff[CHX][CHY].data);
		}
		LD_MSG("\n");
	}
	LD_MSG("\n");
}

int OpenTestRxDiff() {
	unsigned int CHX = 0, CHY = 0;
	int ret = _SUCCESS;
	int threshold = 0;
	int diff = 0;
	int Tolerance = 0;

	ST.MOpen.rx_diff_fail_cnt = 0;

	LD_MSG("Rx Diff: \n       ");
	for (CHX = 0; CHX < ptl.x_ch - 1; CHX++)
		LD_MSG(" %3dCH,", CHX);
	LD_MSG("\n");
	for (CHY = 0; CHY < ptl.y_ch; CHY++) {
		LD_MSG("Y_%2dCH:", CHY);
		for (CHX = 1; CHX < ptl.x_ch; CHX++) {
			diff = abs(uiTestDatas[CHY][CHX] - uiTestDatas[CHY][CHX - 1]);

			if (ST.MOpen.RxDeltaEn == ENABLE_TEST) {
				threshold = ST.MOpen.rx_diff[CHX - 1][CHY].ini.max;
				ST.MOpen.rx_diff[CHX - 1][CHY].raw.data = diff;
				Tolerance = ST.MOpen.RxToles;
				if (diff > threshold) {
					++ST.MOpen.rx_diff_fail_cnt;
					ST.MOpen.rx_diff[CHX - 1][CHY].raw.status = NODE_FAIL;
					LD_MSG("*%5d,", diff);
				} else {
					ST.MOpen.rx_diff[CHX - 1][CHY].raw.status = NODE_PASS;
					LD_MSG("%6d,", diff);
				}
				if (ST.MOpen.rx_diff_fail_cnt > Tolerance)
					ret = _FAIL;
			} else {
				ST.open_Rx_diff[CHX - 1][CHY].data = diff;
				if (ST.open_Rx_diff[CHX - 1][CHY].data > ST.Open_RX_Delta_Threshold) {
					++ST.MOpen.rx_diff_fail_cnt;
					ST.open_Rx_diff[CHX - 1][CHY].status = NODE_FAIL;
					LD_MSG("*%5d,", ST.open_Rx_diff[CHX - 1][CHY].data);
				} else {
					ST.open_Rx_diff[CHX - 1][CHY].status = NODE_PASS;
					LD_MSG("%6d,", ST.open_Rx_diff[CHX - 1][CHY].data);
				}
				if (ST.MOpen.rx_diff_fail_cnt > ST.Open_RX_Continue_Fail_Tolerance)
					ret = _FAIL;
			}
		}
		LD_MSG("\n");
	}

	return ret;
}

int OpenTestTxAverageTest() {
	unsigned int CHX = 0, CHY = 0;
	int ret = _SUCCESS;
	int count = 0;
	int threshold = 0;
	int channelAvg = 0;

	for (CHY = 0; CHY < ptl.y_ch; CHY++) {
		count = 0;
		ST.Tx_Avdiff_daltc[CHY].data = 0;
		for (CHX = 0; CHX < ptl.x_ch; CHX++) {
			if (ST.Open_TX_Aver_Diff_Gap_Corner) {
				//No corner calculations
				if (CHY == 0 && (CHX == 0 || CHX == ptl.x_ch - 1)) {
					//LD_MSG("No corner calculations:[%d][%d]\n", CHX,CHY);
					continue;
				}
				if (CHY == ptl.y_ch - 1 && (CHX == 0 || CHX == ptl.x_ch - 1)) {
					//LD_MSG("No corner calculations:[%d][%d]\n", CHX,CHY);
					continue;
				}
			}
			count++;
			ST.Tx_Avdiff_daltc[CHY].data += uiTestDatas[CHY][CHX];
		}
		//LD_MSG("avdiff[%d] total:%d\n", CHY, ST.Tx_Avdiff_daltc[CHY].data);
		ST.Tx_Avdiff_daltc[CHY].data /= count;
		//LD_MSG("avdiff[%d] %d\n", CHY, ST.Tx_Avdiff_daltc[CHY].data);
	}

	ST.MOpen.tx_avg_fail_cnt = 0;
	for (CHY = 1; CHY < ptl.y_ch; CHY++) {
		if (ST.MOpen.TxAvgEn == ENABLE_TEST) {
			threshold = ST.MOpen.tx_avg[0][CHY - 1].ini.max;
			ST.MOpen.tx_avg[0][CHY - 1].raw.data = abs(ST.Tx_Avdiff_daltc[CHY].data - ST.Tx_Avdiff_daltc[CHY - 1].data);
			channelAvg = ST.MOpen.tx_avg[0][CHY - 1].raw.data;
			if (channelAvg > threshold) {
				ST.MOpen.tx_avg[0][CHY - 1].raw.status = NODE_FAIL;
				ret = _FAIL;
				ST.MOpen.tx_avg_fail_cnt++;
			}
			//LD_MSG("[%d]thr:%d, data:%d\n", CHY - 1, threshold, channelAvg);
		} else {
			threshold = ST.Open_TX_Aver_Diff;
			channelAvg = abs(ST.Tx_Avdiff_daltc[CHY].data - ST.Tx_Avdiff_daltc[CHY - 1].data);
			if (channelAvg > threshold) {
				ST.Tx_Avdiff_daltc[CHY].status = NODE_FAIL;
				ST.Tx_Avdiff_daltc[CHY - 1].status = NODE_FAIL;
				ret = _FAIL;
			}
		}
	}
	return ret;
}

int viRunOpenTest_3X()
{
	int ret = _FAIL;;
	unsigned int inCounts = 0;
	unsigned int CHX = 0, CHY = 0;
	int TXFailCount = 0;
	bool TxAverageStatus = true;
	bool RxDiffStatus = true;

	if (ST.Open_Threshold == -1)
		ST.Open_Threshold = _SensorTestOpenThreshold_;
	if (ST.Open_RX_Delta_Threshold == -1)
		ST.Open_RX_Delta_Threshold = _SensorTestOpenDeltaRX_;
	if (ST.Open_TX_Delta_Threshold == -1)
		ST.Open_TX_Delta_Threshold = _SensorTestOpenDeltaTX_;
	if (ST.Open_RX_Continue_Fail_Tolerance == -1)
		ST.Open_RX_Continue_Fail_Tolerance = _SensorTestOpenRXTolerance_;
	if (ST.Open_TX_Continue_Fail_Tolerance == -1)
		ST.Open_TX_Continue_Fail_Tolerance = _SensorTestOpenTXTolerance_;
	if (ST.Open_TX_Aver_Diff == -1)
		ST.Open_TX_Aver_Diff = _SensorTestOpenTxAverDiff_;

	LD_MSG("\n");
	LD_MSG("Open Test Criteria:\n");
	LD_MSG("Threshold = %d\n",ST.Open_Threshold);
	LD_MSG("RX Delta Threshold = %d\n",ST.Open_RX_Delta_Threshold);
	LD_MSG("TX Delta Threshold = %d\n\n",ST.Open_TX_Delta_Threshold);
	LD_MSG("Open_RX_Continue_Fail_Tolerance = %d\n\n",ST.Open_RX_Continue_Fail_Tolerance);
	LD_MSG("Open_TX_Continue_Fail_Tolerance = %d\n\n",ST.Open_TX_Continue_Fail_Tolerance);
	LD_MSG("TX_Average_Diff_Gap = %d\n\n",ST.Open_TX_Aver_Diff);
	if (ptl.key_mode == ILITEK_HW_KEY_MODE)
		LD_MSG("Key_Threshold = %d\n\n", ST.Open.key_thr);

	switch (KernelVersion[2]) {
	case RDValue_MCUKernel_CDCVersion_10bit2301_:
	case RDValue_MCUKernel_CDCVersion_08bit2301_:
		if (viInitRawData_3X(0x0C, 0xE6) != _FAIL) {
			if (viGetRawData_3X(0xE6, _FastMode_, ptl.x_ch * ptl.y_ch + ptl.key_num,
					    _DataFormat_8_Bit_, ptl.x_ch, NULL) != _FAIL) {
				ret = _SUCCESS;
			} else {
				LD_ERR("Error! Get RawData Failed!\n");
				ret = _FAIL;
			}
		} else {
			LD_ERR("Error! Init RawData Failed!\n");
			ret = _FAIL;
		}
		break;
	case RDValue_MCUKernel_CDCVersion_8bit2315_:
	case RDValue_MCUKernel_CDCVersion_16bit2510_:
		if (viInitRawData_3X(0x0C, 0xE6) != _FAIL) {
			if (viGetRawData_3X(0xE6, _FastMode_, ptl.x_ch * ptl.y_ch + ptl.key_num,
					    _DataFormat_16_Bit_, ptl.x_ch, NULL) != _FAIL) {
				ret = _SUCCESS;
			} else {
				LD_ERR("Error! Get RawData Failed!\n");
				ret = _FAIL;
			}
		} else {
			LD_ERR("Error! Init RawData Failed!\n");
			ret = _FAIL;
		}
		break;
	}

	if (ret == _FAIL)
		return ret;

	if (!ST.MOpen.En && OpenTestTxAverageTest() == _FAIL)
		TxAverageStatus = false;

	LD_MSG("Open Datas: \n       ");
	for (CHX = 0; CHX < ptl.x_ch; CHX++)
		LD_MSG(" %3dCH,", CHX);
	LD_MSG("Average,\n");
	for (CHY = 0; CHY < ptl.y_ch; CHY++) {
		LD_MSG("Y_%2dCH:", CHY);
		for (CHX = 0; CHX < ptl.x_ch; CHX++) {
			LD_MSG("%6d,", uiTestDatas[CHY][CHX]);
			ST.open_daltc[CHX][CHY] = uiTestDatas[CHY][CHX];
			if (uiTestDatas[CHY][CHX] < ST.Open_Threshold)
				ret = _FAIL;
		}
		LD_MSG("%7d,", ST.Tx_Avdiff_daltc[CHY].data);
		LD_MSG("\n");
	}

	for (CHY = 1; CHY < ptl.y_ch; CHY++) {
		for (CHX = 0; CHX < ptl.x_ch; CHX++) {
			if (abs(uiTestDatas[CHY][CHX] - uiTestDatas[CHY - 1][CHX]) > ST.Open_TX_Delta_Threshold)
				++TXFailCount;
			else
				TXFailCount = 0;

			if (TXFailCount >= ST.Open_TX_Continue_Fail_Tolerance)
				ret = _FAIL;
		}
	}
	if (ptl.key_mode == ILITEK_HW_KEY_MODE) {
		LD_MSG("Open Key Datas\n       ");
		for (inCounts = 0; inCounts < ptl.key_num; inCounts++)
			LD_MSG("%6d,", inCounts + 1);
		LD_MSG("\n  data:");
		for (inCounts = 0; inCounts < ptl.key_num; inCounts++) {
			ST.Open.key_daltc[inCounts].data = uiTestDatas[ptl.y_ch][inCounts];
			if (ST.Open.key_daltc[inCounts].data < ST.Open.key_thr) {
				LD_MSG("*%5d,", ST.Open.key_daltc[inCounts].data);
				ST.Open.key_daltc[inCounts].status = NODE_FAIL;
				ret = _FAIL;
			} else {
				LD_MSG("%6d,", ST.Open.key_daltc[inCounts].data);
				ST.Open.key_daltc[inCounts].status = NODE_PASS;
			}
		}
		LD_MSG("\n");
	}

	if (!ST.MOpen.En && OpenTestRxDiff() == _FAIL)
		RxDiffStatus = false;

	if (!ST.useINI)
		OpenTestTxDiff();

	LD_MSG("TxAverageStatus: %s, RxDiffStatus: %s, ret: %s\n",
		(TxAverageStatus == false) ? "NG" : "OK",
		(RxDiffStatus == false) ? "NG" : "OK",
		(ret == _FAIL) ? "NG" : "OK");

	if (TxAverageStatus == false || RxDiffStatus == false)
		ret = _FAIL;

	return ret;
}

int viRunMircoOpenTest_3X()
{
	int ret = _SUCCESS;
	unsigned int CHY = 0, CHX = 0;
	int TxAverageStatus = _SUCCESS, RxDiffStatus = _SUCCESS;

	LD_MSG("\nMirco Open Test Criteria:\n");
	LD_MSG("RX_Delta_En=%s\n", ST.MOpen.RxDeltaEn ? "True" : "False");
	LD_MSG("TX_Avg_Delta_En=%s\n", ST.MOpen.TxAvgEn ? "True" : "False");
	LD_MSG("TX_Avg_Delta_Threshold = %d\n",ST.MOpen.TxAvgThr);
	LD_MSG("TX_Avg_Delta_Threshold_AvoidCorner=%s\n", ST.MOpen.TxAvgCorner ? "True" : "False");
	LD_MSG("RX_Delta_Threshold_Tolerance = %d\n",ST.MOpen.RxToles);


	if(!ST.Open_test) {
		switch (KernelVersion[2])
		{
			case RDValue_MCUKernel_CDCVersion_10bit2301_:
			case RDValue_MCUKernel_CDCVersion_08bit2301_:
			{
				if (viInitRawData_3X(0x0C, 0xE6) != _FAIL)
				{
					if (viGetRawData_3X(0xE6, _FastMode_, ptl.x_ch * ptl.y_ch + ptl.key_num, _DataFormat_8_Bit_, ptl.x_ch, NULL) != _FAIL)
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
				break;
			}
			case RDValue_MCUKernel_CDCVersion_8bit2315_:
			case RDValue_MCUKernel_CDCVersion_16bit2510_:
			{
				if (viInitRawData_3X(0x0C, 0xE6) != _FAIL)
				{
					if (viGetRawData_3X(0xE6, _FastMode_, ptl.x_ch * ptl.y_ch + ptl.key_num, _DataFormat_16_Bit_, ptl.x_ch, NULL) != _FAIL)
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
				break;
			}
		}
	}
	if (OpenTestTxAverageTest() == _FAIL)
		TxAverageStatus = _FAIL;

	LD_MSG("%s, Tx Average: %s\n", __func__, (TxAverageStatus == _SUCCESS)? "PASS" : "FAIL");
	LD_MSG("  Delta,(Avg_D)\n");
	for (CHY = 1; CHY < ptl.y_ch; CHY++)
	{
		if(ST.MOpen.tx_avg[0][CHY].raw.status == NODE_FAIL)
			LD_MSG("Y_%3dCH,*%5d\n", CHY, ST.MOpen.tx_avg[0][CHY - 1].raw.data);
		else
			LD_MSG("Y_%3dCH,%6d\n", CHY, ST.MOpen.tx_avg[0][CHY - 1].raw.data);
	}
	LD_MSG("\nMirco Open Datas: \n       ");
	for (CHX = 0; CHX < ptl.x_ch; CHX++) {
		LD_MSG(" %3dCH,", CHX);
	}
	LD_MSG("\n");
	for (CHY = 0; CHY < ptl.y_ch; CHY++)
	{
		LD_MSG("Y_%2dCH:", CHY);
		for (CHX = 0; CHX < ptl.x_ch; CHX++)
		{
			if(!ST.Open_test)
				ST.open_daltc[CHX][CHY] = uiTestDatas[CHY][CHX];
			LD_MSG("%6d,", ST.open_daltc[CHX][CHY]);
		}
		//LD_MSG("%7d,", ST.Tx_Avdiff_daltc[CHY].data);
		LD_MSG("\n");
	}

	if (OpenTestRxDiff() == _FAIL)
		RxDiffStatus = _FAIL;
	LD_MSG("%s, Rx Diff: %s\n", __func__, (RxDiffStatus == _SUCCESS)? "PASS" : "FAIL");
	if (TxAverageStatus == _FAIL || RxDiffStatus == _FAIL)
		ret = _FAIL;

	return ret;
}

int viRunShortTest_6X()
{
	int ret = _SUCCESS;
	unsigned int ucLineLenth = 0;
	unsigned int ucIndex = 0;

	if (ST.Short.Threshold == -1)
	{
		ST.Short.Threshold = _SensorTest_Short_Thresshold_V6_;
	}
	LD_MSG("\n");
	LD_MSG("Short Test Criteria:\n");
	LD_MSG("Threshold = %d\n",ST.Short.Threshold);
	LD_MSG("Dump1=0x%x\n", ST.Short.dump1);
	LD_MSG("Dump2=0x%x\n", ST.Short.dump2);
	LD_MSG("Vref=%0.1f\n", ST.Short.vref_v);
	LD_MSG("Postidle_L=0x%x\n", ST.Short.posidleL);

	ucLineLenth = ptl.x_ch;
	ModeCtrl_V6(ENTER_TEST_MODE, DISABLE_ENGINEER, 100);
	if (SetShortInfo(ST.Short.dump1, ST.Short.dump2, ST.Short.vref_s, ST.Short.posidleL, ST.Short.posidleH) < 0) {
		LD_ERR("Error! Set Short info Failed!\n");
		return _FAIL;
	}
	if(viGetCDCData_6X(TEST_MODE_V6_SHORT_RX, ucLineLenth) < 0)
		return _FAIL;

	for (ucIndex = 0; ucIndex < ptl.x_ch; ucIndex++)
	{
		ST.v6_short_daltc[0][ucIndex].data = uiTestDatas[ucIndex/ptl.x_ch][ucIndex%ptl.x_ch];
	}

	ucLineLenth = ptl.y_ch;
	if(viGetCDCData_6X(TEST_MODE_V6_SHORT_TX, ucLineLenth) < 0)
		return _FAIL;

	for (ucIndex = 0; ucIndex < ptl.y_ch; ucIndex++)
	{
		ST.v6_short_daltc[1][ucIndex].data = uiTestDatas[ucIndex/ptl.x_ch][ucIndex%ptl.x_ch];
	}

	if(ptl.key_mode == ILITEK_HW_KEY_MODE) {
		LD_MSG("KeyRX_Threshold=%d\n", ST.Short.keyRx_thr);
		LD_MSG("KeyTX_Threshold=%d\n", ST.Short.keyTx_thr);
		ucLineLenth = ptl.key_num + 1;
		if(viGetCDCData_6X(TEST_MODE_V6_SHORT_KEY, ucLineLenth) < 0)
			return _FAIL;

		for (ucIndex = 0; ucIndex < ucLineLenth; ucIndex++)
		{
			ST.Short.key_daltc[ucIndex].data = uiTestDatas[ucIndex/ptl.x_ch][ucIndex%ptl.x_ch];
		}
		LD_MSG("Short Key Datas:\n       ");
		for (ucIndex = 0; ucIndex < ptl.key_num; ucIndex++)
			LD_MSG("%6d,", ucIndex + 1);
		LD_MSG("Self X:\n       ");
		for (ucIndex = 0; ucIndex < ptl.key_num; ucIndex++) {
			if(ST.Short.key_daltc[ucIndex].data > ST.Short.keyRx_thr) {
				ret = _FAIL;
				LD_MSG("*%5d,", ST.Short.key_daltc[ucIndex].data);
				ST.Short.key_daltc[ucIndex].status = NODE_FAIL;
			}
			else {
				LD_MSG("%6d,", ST.Short.key_daltc[ucIndex].data);
				ST.Short.key_daltc[ucIndex].status = NODE_PASS;
			}

		}
		if(ST.Short.key_daltc[ptl.key_num].data > ST.Short.keyTx_thr) {
			ret = _FAIL;
			ST.Short.key_daltc[ptl.key_num].status = NODE_FAIL;
			LD_MSG("\nSelf Y:*%5d,\n:", ST.Short.key_daltc[ptl.key_num].data);
		}
		else {
			ST.Short.key_daltc[ptl.key_num].status = NODE_PASS;
			LD_MSG("\nSelf Y:%6d,\n", ST.Short.key_daltc[ptl.key_num].data);
		}
	}
	LD_MSG("\nShort Datas:\n");
	LD_MSG("X_CH:");
	for (ucIndex = 0; ucIndex < ptl.x_ch; ucIndex++)
	{
		if(ST.v6_short_daltc[0][ucIndex].data > ST.Short.Threshold) {
			ST.v6_short_daltc[0][ucIndex].status = NODE_FAIL;
			LD_MSG(" *%4d,", ST.v6_short_daltc[0][ucIndex].data);
			ret = _FAIL;
		}
		else {
			ST.v6_short_daltc[0][ucIndex].status = NODE_PASS;
			LD_MSG("  %4d,", ST.v6_short_daltc[0][ucIndex].data);
		}
	}
	LD_MSG("\n");
	LD_MSG("Y_CH:");
	for (ucIndex = 0; ucIndex < ptl.y_ch; ucIndex++)
	{
		if(ST.v6_short_daltc[1][ucIndex].data > ST.Short.Threshold) {
			ST.v6_short_daltc[1][ucIndex].status = NODE_FAIL;
			LD_MSG(" *%4d,", ST.v6_short_daltc[1][ucIndex].data);
			ret = _FAIL;
		}
		else {
			LD_MSG("  %4d,", ST.v6_short_daltc[1][ucIndex].data);
			ST.v6_short_daltc[1][ucIndex].status = NODE_PASS;
		}
	}
	LD_MSG("\nImpedance value:\n   X:");
	for (ucIndex = 0; ucIndex < ptl.x_ch; ucIndex++)
		LD_MSG("%3d_CH,", ucIndex);
	LD_MSG("\n     ");
	for (ucIndex = 0; ucIndex < ptl.x_ch; ucIndex++) {
		if(ST.v6_short_daltc[0][ucIndex].data < DIVIDE_10M)
			LD_MSG("   10M,");
		else {
			LD_MSG("%.3lf", IMPEDANCE_MACRO(ST.v6_short_daltc[0][ucIndex].data, ST.Short.dump1, ST.Short.dump2,ST.Short.vref_v));
			LD_MSG("M,");
		}
	}
	LD_MSG("\n   Y:");
	for (ucIndex = 0; ucIndex < ptl.y_ch; ucIndex++)
		LD_MSG("%3d_CH,", ucIndex);
	LD_MSG("\n     ");
	for (ucIndex = 0; ucIndex < ptl.y_ch; ucIndex++) {
		if(ST.v6_short_daltc[1][ucIndex].data < DIVIDE_10M)
			LD_MSG("   10M,");
		else {
			LD_MSG("%.3lf", IMPEDANCE_MACRO(ST.v6_short_daltc[1][ucIndex].data, ST.Short.dump1, ST.Short.dump2,ST.Short.vref_v));
			LD_MSG("M,");
		}
	}
	ModeCtrl_V6(ENTER_NORMAL_MODE, DISABLE_ENGINEER, 1000);
	return ret;
}

int viRunOpenTest()
{
	int ret = 1;

	if (inProtocolStyle == _Protocol_V3_) {
		if (ST.UseNewFlow == 1)
			ret = viRunOpenTest_3X_NewFlow();
		else
			ret = viRunOpenTest_3X();
	} else if (inProtocolStyle == _Protocol_V6_) {
		ret = viRunOpenTest_6X();
	}
	return ret;
}

int viRunMircoOpenTest()
{
	int ret = 1;
	if (inProtocolStyle == _Protocol_V3_)
	{
		ret = viRunMircoOpenTest_3X();
	}
	else if (inProtocolStyle == _Protocol_V6_)
	{
		ret = viRunMircoOpenTest_6X();
	}
	return ret;
}

int viRunSelfTest_3X()
{
	int ret = _SUCCESS;
	int inCounts = 0;
	unsigned int CHX = 0, CHY = 0;
	unsigned char ucPass = 1;
	switch (KernelVersion[2])
	{
		case RDValue_MCUKernel_CDCVersion_8bit2315_:
			if (ST.Self_Maximum == -1)
				ST.Self_Maximum = _2315SensorTestSelfMaximum_;
			if (ST.Self_Minimum == -1)
				ST.Self_Minimum = _2315SensorTestSelfMinimum_;
			if (ST.Self_P2P == -1)
				ST.Self_P2P = _2315SensorTestSelfP2P_;
			if (ST.Self_P2P_Edge == -1)
				ST.Self_P2P_Edge = _2315SensorTestSelfP2PEdge_;
			if (ST.Self_Frame_Count == -1)
				ST.Self_Frame_Count = _2315SensorTestSelfFramCnt_;
			break;
		case RDValue_MCUKernel_CDCVersion_16bit2510_:
			if (ST.Self_Maximum == -1)
				ST.Self_Maximum = _2510SensorTestSelfMaximum_;
			if (ST.Self_Minimum == -1)
				ST.Self_Minimum = _2510SensorTestSelfMinimum_;
			if (ST.Self_P2P == -1)
				ST.Self_P2P = _2510SensorTestSelfP2P_;
			if (ST.Self_P2P_Edge == -1)
				ST.Self_P2P_Edge = _2510SensorTestSelfP2PEdge_;
			if (ST.Self_Frame_Count == -1)
				ST.Self_Frame_Count = _2510SensorTestSelfFramCnt_;
			break;
	}

	LD_MSG("\n");
	LD_MSG("Self Test Criteria:\n");
	LD_MSG("Maximum = %d\n",ST.Self_Maximum);
	LD_MSG("Minimum = %d\n",ST.Self_Minimum);
	LD_MSG("P2P = %d\n",ST.Self_P2P);
	LD_MSG("P2P Edge = %d\n",ST.Self_P2P_Edge);
	LD_MSG("Frame Count = %d\n\n",ST.Self_Frame_Count);


	if (viInitRawData_3Para_3X(0x0d, 0xE6, ST.Self_Frame_Count) != _FAIL)
	{
		if (viGetRawData_3X(0xE6, _FastMode_, ptl.x_ch + ptl.y_ch + 4, _DataFormat_16_Bit_, ptl.x_ch + ptl.y_ch + 4, NULL) != _FAIL)
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

	if (ret != _FAIL)
	{
		LD_MSG("Self Datas CHX: ");
		for (CHX = 0; CHX < ptl.x_ch; CHX++)
		{
			LD_MSG("%4d,", uiTestDatas[0][CHX]);
			ST.self_xdaltc[CHX] = uiTestDatas[0][CHX];
			if (CHX == 0 || CHX == (ptl.x_ch - 1))
			{
				if (uiTestDatas[0][CHX] > ST.Self_P2P_Edge)
				{
					ucPass = 0;
				}
			}
			else
			{
				if (uiTestDatas[0][CHX] > ST.Self_P2P)
				{
					ucPass = 0;
				}
			}
		}
		LD_MSG("\n");

		LD_MSG("Self Datas CHY: ");
		for (CHY = ptl.x_ch, inCounts = 0; CHY < (ptl.y_ch + ptl.x_ch); CHY++, inCounts++)
		{
			LD_MSG("%4d,", uiTestDatas[0][CHY]);
			ST.self_ydaltc[inCounts] = uiTestDatas[0][CHY];
			if (CHY == ptl.x_ch || CHY == (ptl.y_ch + ptl.x_ch - 1))
			{
				if (uiTestDatas[0][CHY] > ST.Self_P2P_Edge)
				{
					ucPass = 0;
				}
			}
			else
			{
				if (uiTestDatas[0][CHY] > ST.Self_P2P)
				{
					ucPass = 0;
				}
			}
		}
		LD_MSG("\n");
		LD_MSG("CHX Maximum Value: ");
		LD_MSG("%4d,\n", uiTestDatas[0][ptl.y_ch + ptl.x_ch + 0]);
		LD_MSG("CHX Minimum Value: ");
		LD_MSG("%4d,\n", uiTestDatas[0][ptl.y_ch + ptl.x_ch + 1]);
		LD_MSG("CHY Maximum Value: ");
		LD_MSG("%4d,\n", uiTestDatas[0][ptl.y_ch + ptl.x_ch + 2]);
		LD_MSG("CHY Minimum Value: ");
		LD_MSG("%4d,\n", uiTestDatas[0][ptl.y_ch + ptl.x_ch + 3]);
		LD_MSG("\n\n");
		if (uiTestDatas[0][ptl.y_ch + ptl.x_ch + 0] > ST.Self_Maximum || uiTestDatas[0][ptl.y_ch + ptl.x_ch + 2] > ST.Self_Maximum)
		{
			ucPass = 0;
		}

		if (uiTestDatas[0][ptl.y_ch + ptl.x_ch + 1] < ST.Self_Minimum || uiTestDatas[0][ptl.y_ch + ptl.x_ch + 3] < ST.Self_Minimum)
		{
			ucPass = 0;
		}

		if (ucPass == 1)
		{
			//LD_MSG("Success! Open Test : Pass\n");
			//LD_MSG("Success! Open Test : Pass\n");
		}
		else
		{
			ucSensorTestResult |= SELF_TEST;
			ret = _FAIL;
			//LD_ERR("Failed! Open Test : NG\n");
			//LD_ERR("Failed! Open Test : NG\n");
		}
	}
	return ret;
}

int viRunDACTest_3X()
{
	int ret = _FAIL;
	unsigned int CHX = 0, CHY = 0;
	unsigned char ucPass = 1;

	if (ST.DAC_SC_P_Maximum == -1)
		ST.DAC_SC_P_Maximum = _SensorTestDACSPMax_;
	if (ST.DAC_SC_P_Minimum == -1)
		ST.DAC_SC_P_Minimum = _SensorTestDACSPMin_;
	if (ST.DAC_SC_N_Maximum == -1)
		ST.DAC_SC_N_Maximum = _SensorTestDACSNMax_;
	if (ST.DAC_SC_N_Minimum == -1)
		ST.DAC_SC_N_Minimum = _SensorTestDACSNMin_;
	if (ST.DAC_MC_P_Maximum == -1)
		ST.DAC_MC_P_Maximum = _SensorTestDACMPMax_;
	if (ST.DAC_MC_P_Minimum == -1)
		ST.DAC_MC_P_Minimum = _SensorTestDACMPMin_;
	if (ST.DAC_MC_N_Maximum == -1)
		ST.DAC_MC_N_Maximum = _SensorTestDACMNMax_;
	if (ST.DAC_MC_N_Minimum == -1)
		ST.DAC_MC_N_Minimum = _SensorTestDACMNMin_;
	LD_MSG("\n");
	LD_MSG("DAC Test Criteria:\n");
	LD_MSG("Self P DAC Maximum = %d\n",ST.DAC_SC_P_Maximum);
	LD_MSG("Self P DAC Minimum = %d\n",ST.DAC_SC_P_Minimum);
	LD_MSG("Self N DACMaximum = %d\n",ST.DAC_SC_N_Maximum);
	LD_MSG("Self N DACMinimum = %d\n",ST.DAC_SC_N_Minimum);
	LD_MSG("Mutual P DAC Maximum = %d\n",ST.DAC_MC_P_Maximum);
	LD_MSG("Mutual P DAC Minimum = %d\n",ST.DAC_MC_P_Minimum);
	LD_MSG("Mutual N DAC Maximum = %d\n",ST.DAC_MC_N_Maximum);
	LD_MSG("Mutual N DAC Minimum = %d\n\n",ST.DAC_MC_N_Minimum);
	ucSignedDatas = 1;

	//----------------------------Key_DAC_SC_P_2510_ = 0x22 ---------------------------------------------
	if (viInitRawData_3X(0x22, 0xE6) != _FAIL)
	{
		if (viGetRawData_3X(0xE6, _FastMode_, ptl.x_ch + ptl.y_ch, _DataFormat_8_Bit_, ptl.x_ch + ptl.y_ch, NULL) != _FAIL)
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

	if (ret != _FAIL)
	{
		LD_MSG("DAC_SC_P Datas CHX: \n");
		for (CHX = 0; CHX < ptl.x_ch; CHX++)
		{
			ST.dac_sc_p[CHX] = uiTestDatas[0][CHX];
			LD_MSG("%4d,", uiTestDatas[0][CHX]);
			if (uiTestDatas[0][CHX] < ST.DAC_SC_P_Minimum || uiTestDatas[0][CHX] > ST.DAC_SC_P_Maximum)
			{
				ucPass = 0;
			}
		}
		LD_MSG("\n");

		LD_MSG("DAC_SC_P Datas CHY: \n");
		for (CHY = ptl.x_ch; CHY < (ptl.y_ch + ptl.x_ch); CHY++)
		{
			ST.dac_sc_p[CHY] = uiTestDatas[0][CHY];
			LD_MSG("%4d,", uiTestDatas[0][CHY]);
			if (uiTestDatas[0][CHY] < ST.DAC_SC_P_Minimum || uiTestDatas[0][CHY] > ST.DAC_SC_P_Maximum)
			{
				ucPass = 0;
			}
		}
		LD_MSG("\n\n");

		if (ucPass == 1)
		{
			//LD_MSG("Success! Open Test : Pass\n");
			//LD_MSG("Success! Open Test : Pass\n");
		}
		else
		{
			ucSensorTestResult |= DAC_TEST;
			ret = _FAIL;
			//LD_ERR("Failed! Open Test : NG\n");
			//LD_ERR("Failed! Open Test : NG\n");
		}
	}

	//----------------------------Key_DAC_MC_P_2510_ = 0x20 ---------------------------------------------
	if (viInitRawData_3X(0x20, 0xE6) != _FAIL)
	{
		if (viGetRawData_3X(0xE6, _FastMode_, ptl.x_ch * ptl.y_ch, _DataFormat_8_Bit_, ptl.x_ch, NULL) != _FAIL)
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

	if (ret != _FAIL)
	{
		LD_MSG("DAC_MC_P Datas CHX: \n");
		for (CHY = 0; CHY < ptl.y_ch; CHY++)
		{
			LD_MSG("Y_%2dCH:", CHY);
			for (CHX = 0; CHX < ptl.x_ch; CHX++)
			{
				LD_MSG("%4d,", uiTestDatas[CHY][CHX]);
				ST.dac_mc_p[CHX][CHY] = uiTestDatas[CHY][CHX];
				if (uiTestDatas[0][CHX] < ST.DAC_MC_P_Minimum || uiTestDatas[0][CHX] > ST.DAC_MC_P_Maximum)
				{
					ucPass = 0;
				}
			}
			LD_MSG("\n\n");
		}
		LD_MSG("\n\n");

		if (ucPass == 1)
		{
			//LD_MSG("Success! Open Test : Pass\n");
			//LD_MSG("Success! Open Test : Pass\n");
		}
		else
		{
			ucSensorTestResult |= DAC_TEST;
			ret = _FAIL;
			//LD_ERR("Failed! Open Test : NG\n");
			//LD_ERR("Failed! Open Test : NG\n");
		}
	}

	//----------------------------Key_DAC_SC_N_2510_ = 0x23 ---------------------------------------------
	if (viInitRawData_3X(0x23, 0xE6) != _FAIL)
	{
		if (viGetRawData_3X(0xE6, _FastMode_, ptl.x_ch + ptl.y_ch, _DataFormat_8_Bit_, ptl.x_ch + ptl.y_ch, NULL) != _FAIL)
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

	if (ret != _FAIL)
	{
		LD_MSG("DAC_SC_N Datas CHX: \n");
		for (CHX = 0; CHX < ptl.x_ch; CHX++)
		{
			ST.dac_sc_n[CHX] = uiTestDatas[0][CHX];
			LD_MSG("%4d,", uiTestDatas[0][CHX]);
			if (uiTestDatas[0][CHX] < ST.DAC_SC_N_Minimum || uiTestDatas[0][CHX] > ST.DAC_SC_N_Maximum)
			{
				ucPass = 0;
			}
		}
		LD_MSG("\n");

		LD_MSG("DAC_SC_N Datas CHY: \n");
		for (CHY = ptl.x_ch; CHY < (ptl.y_ch + ptl.x_ch); CHY++)
		{
			ST.dac_sc_n[CHY] = uiTestDatas[0][CHY];
			LD_MSG("%4d,", uiTestDatas[0][CHY]);
			if (uiTestDatas[0][CHY] < ST.DAC_SC_N_Minimum || uiTestDatas[0][CHY] > ST.DAC_SC_N_Maximum)
			{
				ucPass = 0;
			}
		}
		LD_MSG("\n\n");

		if (ucPass == 1)
		{
			//LD_MSG("Success! Open Test : Pass\n");
			//LD_MSG("Success! Open Test : Pass\n");
		}
		else
		{
			ucSensorTestResult |= DAC_TEST;
			ret = _FAIL;
			//LD_ERR("Failed! Open Test : NG\n");
			//LD_ERR("Failed! Open Test : NG\n");
		}
	}

	//----------------------------Key_DAC_MC_N_2510_ = 0x21 ---------------------------------------------
	if (viInitRawData_3X(0x21, 0xE6) != _FAIL)
	{
		if (viGetRawData_3X(0xE6, _FastMode_, ptl.x_ch * ptl.y_ch, _DataFormat_8_Bit_, ptl.x_ch, NULL) != _FAIL)
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

	if (ret != _FAIL)
	{
		LD_MSG("DAC_MC_N Datas CHX: \n");
		for (CHY = 0; CHY < ptl.y_ch; CHY++)
		{
			LD_MSG("Y_%2dCH:", CHY);
			for (CHX = 0; CHX < ptl.x_ch; CHX++)
			{
				ST.dac_mc_n[CHX][CHY] = uiTestDatas[CHY][CHX];
				LD_MSG("%4d,", uiTestDatas[CHY][CHX]);
				if (uiTestDatas[0][CHX] < ST.DAC_MC_N_Minimum || uiTestDatas[0][CHX] > ST.DAC_MC_N_Maximum)
				{
					ucPass = 0;
				}
			}
			LD_MSG("\n\n");
		}
		LD_MSG("\n\n");

		if (ucPass == 1)
		{
		}
		else
		{
			ucSensorTestResult |= DAC_TEST;
			ret = _FAIL;
		}
	}
	ucSignedDatas = 0;
	return ret;
}

int viRunShortTest_3X()
{
	int ret = _SUCCESS;
	int inCounts = 0;
	unsigned char ucPass = 1;
	unsigned char ucInitCMD = 0;
	unsigned char ucReadCMD = 0;
	unsigned char ucLineLenth = 0;
	unsigned char ucIndex = 0;
	int key_ych = 0;

	if (ST.Short.Threshold == -1)
	{
		ST.Short.Threshold = _SensorTestShortThreshold_;
	}
	LD_MSG("\n");
	LD_MSG("Short Test Criteria:\n");
	LD_MSG("Threshold = %d\n",ST.Short.Threshold);
	if(ptl.key_mode == ILITEK_HW_KEY_MODE) {
		LD_MSG("KeyRX_Threshold = %d\n",ST.Short.keyRx_thr);
		LD_MSG("KeyTsX_Threshold = %d\n",ST.Short.keyTx_thr);
		key_ych++;
	}

	for (inCounts = 0; inCounts < 4; inCounts++)
	{
		switch (inCounts)
		{
			case 0:
				ucInitCMD = 0x09;
				ucReadCMD = 0xe0;
				ucLineLenth = ptl.x_ch + ptl.key_num;
				break;
			case 1:
				ucInitCMD = 0x09;
				ucReadCMD = 0xe1;
				ucLineLenth = ptl.y_ch + key_ych;
				break;
			case 2:
				ucInitCMD = 0x0a;
				ucReadCMD = 0xe0;
				ucLineLenth = ptl.x_ch + ptl.key_num;
				break;
			case 3:
				ucInitCMD = 0x0a;
				ucReadCMD = 0xe1;
				ucLineLenth = ptl.y_ch + key_ych;
				break;
		}
		if (viInitRawData_3X(ucInitCMD, ucReadCMD) != _FAIL)
		{
			if (viGetRawData_3X(ucReadCMD, _SlowMode_, ucLineLenth, _DataFormat_8_Bit_, ucLineLenth, NULL) != _FAIL)
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

		for (ucIndex = 0; ucIndex < ucLineLenth; ucIndex++)
		{
			ST.short_daltc[inCounts][ucIndex] = uiTestDatas[0][ucIndex];
		}
	}

	if (ret != _FAIL)
	{
		LD_MSG("\n");
		LD_MSG("Short Datas:");
		LD_MSG("\n");
		LD_MSG("X_CH_SLK: ");
		for (ucIndex = 0; ucIndex < ptl.x_ch; ucIndex++)
		{
			LD_MSG("%4d,", ST.short_daltc[0][ucIndex]);
		}
		LD_MSG("\n");
		LD_MSG("X_CH__LK: ");
		for (ucIndex = 0; ucIndex < ptl.x_ch; ucIndex++)
		{
			LD_MSG("%4d,", ST.short_daltc[2][ucIndex]);
			if (((ST.short_daltc[0][ucIndex] >= ST.short_daltc[2][ucIndex]) &&
						((ST.short_daltc[0][ucIndex] - ST.short_daltc[2][ucIndex]) > ST.Short.Threshold)) ||
					((ST.short_daltc[0][ucIndex] < ST.short_daltc[2][ucIndex]) &&
					 ((ST.short_daltc[2][ucIndex] - ST.short_daltc[0][ucIndex]) > ST.Short.Threshold)))
			{
				ucPass = 0;
			}
		}

		LD_MSG("\n");
		LD_MSG("Y_CH_SLK: ");
		for (ucIndex = 0; ucIndex < ptl.y_ch; ucIndex++)
		{
			LD_MSG("%4d,", ST.short_daltc[1][ucIndex]);
		}

		LD_MSG("\n");
		LD_MSG("Y_CH__LK: ");
		for (ucIndex = 0; ucIndex < ptl.y_ch; ucIndex++)
		{
			LD_MSG("%4d,", ST.short_daltc[3][ucIndex]);
			if (((ST.short_daltc[1][ucIndex] >= ST.short_daltc[3][ucIndex]) &&
						((ST.short_daltc[1][ucIndex] - ST.short_daltc[3][ucIndex]) > ST.Short.Threshold)) ||
					((ST.short_daltc[1][ucIndex] < ST.short_daltc[3][ucIndex]) &&
					 ((ST.short_daltc[3][ucIndex] - ST.short_daltc[1][ucIndex]) > ST.Short.Threshold)))
			{
				ucPass = 0;
			}
		}
		if (ptl.key_mode == ILITEK_HW_KEY_MODE) {
			LD_MSG("\n");
			LD_MSG("KEY X_CH_SLK: ");
			for (ucIndex = ptl.x_ch; ucIndex < ptl.x_ch + ptl.key_num; ucIndex++)
			{
				LD_MSG("%4d,", ST.short_daltc[0][ucIndex]);
			}
			LD_MSG("\n");
			LD_MSG("KEY X_CH__LK: ");
			for (ucIndex = ptl.x_ch; ucIndex < ptl.x_ch + ptl.key_num; ucIndex++)
			{
				LD_MSG("%4d,", ST.short_daltc[2][ucIndex]);
				if (((ST.short_daltc[0][ucIndex] >= ST.short_daltc[2][ucIndex]) &&
							((ST.short_daltc[0][ucIndex] - ST.short_daltc[2][ucIndex]) > ST.Short.keyRx_thr)) ||
						((ST.short_daltc[0][ucIndex] < ST.short_daltc[2][ucIndex]) &&
						 ((ST.short_daltc[2][ucIndex] - ST.short_daltc[0][ucIndex]) > ST.Short.keyRx_thr)))
				{
					ucPass = 0;
				}
			}

			LD_MSG("\n");
			LD_MSG("KEY Y_CH_SLK: ");
			for (ucIndex = ptl.y_ch; ucIndex < ptl.y_ch + key_ych; ucIndex++)
			{
				LD_MSG("%4d,", ST.short_daltc[1][ucIndex]);
			}

			LD_MSG("\n");
			LD_MSG("KEY Y_CH__LK: ");
			for (ucIndex = ptl.y_ch; ucIndex < ptl.y_ch + key_ych; ucIndex++)
			{
				LD_MSG("%4d,", ST.short_daltc[3][ucIndex]);
				if (((ST.short_daltc[1][ucIndex] >= ST.short_daltc[3][ucIndex]) &&
							((ST.short_daltc[1][ucIndex] - ST.short_daltc[3][ucIndex]) > ST.Short.keyTx_thr)) ||
						((ST.short_daltc[1][ucIndex] < ST.short_daltc[3][ucIndex]) &&
						 ((ST.short_daltc[3][ucIndex] - ST.short_daltc[1][ucIndex]) > ST.Short.keyTx_thr)))
				{
					ucPass = 0;
				}
			}
		}

		LD_MSG("\n\n");
		if (ucPass == 1)
		{
			//	LD_MSG("Success! Short Test : Pass\n");
			//	LD_MSG("Success! Short Test : Pass\n");
		}
		else
		{
			ucSensorTestResult |= MICROSHORT_TEST;
			ret = _FAIL;
			//	LD_ERR("Failed! Short Test : NG\n");
			//	LD_ERR("Failed! Short Test : NG\n");
		}
	}
	return ret;
}

int viRunOpenTest_6X()
{
	int ret = _SUCCESS;
	int trans_num = 0;
	int TxAverageStatus = _SUCCESS, RxDiffStatus = _SUCCESS, ThresholdStatus = _SUCCESS;

	LD_MSG("\nOpen Test Criteria:\n");
	trans_num = ptl.x_ch * ptl.y_ch;
	LD_MSG("Threshold = %d, total = %d\n", ST.Open_Threshold, trans_num);
	LD_MSG("TX_Average_Diff_Gap = %d\n", ST.Open_TX_Aver_Diff);
	LD_MSG("RX Delta Threshold = %d\n", ST.Open_RX_Delta_Threshold);
	LD_MSG("Open_RX_Continue_Fail_Tolerance = %d\n", ST.Open_RX_Continue_Fail_Tolerance);
	LD_MSG("Gain = %d\n", ST.Open.gain);
	LD_MSG("Frequency = %d\n\n", ST.Open.freq);

	ModeCtrl_V6(ENTER_TEST_MODE, DISABLE_ENGINEER, 100);

	if (SetOpenInfo(ST.Open.freq & 0xFF, (ST.Open.freq >> 8) & 0xFF, ST.Open.gain) < 0) {
		LD_ERR("Error! Set Open info Failed!\n");
		return _FAIL;
	}
	if (viGetCDCData_6X(TEST_MODE_V6_OPEN, trans_num) < 0)
		return _FAIL;
	if (ST.Open_TX_Aver_Diff > 0)
		if(OpenTestTxAverageTest() == _FAIL)
			TxAverageStatus = _FAIL;

	LD_MSG("%s, Tx Average: %s\n", __func__, (TxAverageStatus == _SUCCESS)? "PASS" : "FAIL");
	if (OpenTestThreshold() == _FAIL)
		ThresholdStatus = _FAIL;

	LD_MSG("%s, Threshold: %s\n", __func__, (ThresholdStatus == _SUCCESS)? "PASS" : "FAIL");
	if (ST.Open_RX_Delta_Threshold > 0) {
		if(OpenTestRxDiff() == _FAIL)
			RxDiffStatus = _FAIL;
	}
	LD_MSG("%s, Rx Diff: %s\n", __func__, (RxDiffStatus == _SUCCESS)? "PASS" : "FAIL");
	if (ptl.key_mode == ILITEK_HW_KEY_MODE) {
		if(OpenTestKeyThreshold() == _FAIL)
			ret = _FAIL;
	}
	if (TxAverageStatus == _FAIL || RxDiffStatus == _FAIL || ThresholdStatus == _FAIL) {
		ret = _FAIL;
	}

	ModeCtrl_V6(ENTER_NORMAL_MODE, DISABLE_ENGINEER, 100);

	return ret;
}

int viRunMircoOpenTest_6X()
{
	int ret = _SUCCESS;
	unsigned int CHY = 0, CHX = 0;
	int trans_num = 0;
	int TxAverageStatus = _SUCCESS, RxDiffStatus = _SUCCESS;

	LD_MSG("\nMirco Open Test Criteria:\n");
	trans_num = ptl.x_ch * ptl.y_ch;
	LD_MSG("RX_Delta_En=%s\n", ST.MOpen.RxDeltaEn ? "True" : "False");
	LD_MSG("TX_Avg_Delta_En=%s\n", ST.MOpen.TxAvgEn ? "True" : "False");
	LD_MSG("TX_Avg_Delta_Threshold = %d\n",ST.MOpen.TxAvgThr);
	LD_MSG("TX_Avg_Delta_Threshold_AvoidCorner=%s\n", ST.MOpen.TxAvgCorner ? "True" : "False");
	LD_MSG("RX_Delta_Threshold_Tolerance = %d\n",ST.MOpen.RxToles);


	if(!ST.Open_test) {
		ModeCtrl_V6(ENTER_TEST_MODE, DISABLE_ENGINEER, 100);

		if (viInitRawData_6X(TEST_MODE_V6_OPEN, 10) != _FAIL)
		{
			if (viGetRawData_6X(trans_num, NULL) != _FAIL)
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
	}

	if (OpenTestTxAverageTest() == _FAIL && ST.MOpen.TxAvgEn == ENABLE_TEST)
		TxAverageStatus = _FAIL;

	LD_MSG("%s, Tx Average: %s\n", __func__, (TxAverageStatus == _SUCCESS)? "PASS" : "FAIL");
	LD_MSG("  Delta,(Avg_D)\n");
	for (CHY = 1; CHY < ptl.y_ch; CHY++)
	{
		if(ST.MOpen.tx_avg[0][CHY].raw.status == NODE_FAIL)
			LD_MSG("Y_%3dCH,*%5d\n", CHY, ST.MOpen.tx_avg[0][CHY - 1].raw.data);
		else
			LD_MSG("Y_%3dCH,%6d\n", CHY, ST.MOpen.tx_avg[0][CHY - 1].raw.data);
	}
	LD_MSG("\nMirco Open Datas: \n       ");
	for (CHX = 0; CHX < ptl.x_ch; CHX++) {
		LD_MSG(" %3dCH,", CHX);
	}
	LD_MSG("\n");
	for (CHY = 0; CHY < ptl.y_ch; CHY++) {
		LD_MSG("Y_%2dCH:", CHY);
		for (CHX = 0; CHX < ptl.x_ch; CHX++) {
			if(!ST.Open_test)
				ST.v6_open_daltc[CHX][CHY].data = uiTestDatas[CHY][CHX];
			LD_MSG("%6d,", ST.v6_open_daltc[CHX][CHY].data);

		}
		LD_MSG("\n");
	}

	if (OpenTestRxDiff() == _FAIL && ST.MOpen.RxDeltaEn == ENABLE_TEST)
		RxDiffStatus = _FAIL;
	LD_MSG("%s, Rx Diff: %s\n", __func__, (RxDiffStatus == _SUCCESS)? "PASS" : "FAIL");

	if (TxAverageStatus == _FAIL || RxDiffStatus == _FAIL)
		ret = _FAIL;

	ModeCtrl_V6(ENTER_NORMAL_MODE, DISABLE_ENGINEER, 100);

	return ret;
}

int NodeTest_V6(const char *name, SensorTest_Node **delac, SensorTest_BenBenchmark_Node **data,
		int X_MAX, int Y_MAX, int fail_count_limit, int *fail_count,
		bool check_max)
{
	int ret = _SUCCESS;
	int CHX = 0, CHY = 0;

	LD_MSG("%s_Up: \n        ", name);
	for (CHX = 0; CHX < X_MAX; CHX++)
		LD_MSG(" X_%3d,", CHX);
	for (CHY = 0; CHY < Y_MAX; CHY++) {
		LD_MSG("\nY_%3dCH:", CHY);
		for (CHX = 0; CHX < X_MAX; CHX++)
			LD_MSG(" %5d,", data[CHX][CHY].ini.max);
	}

	LD_MSG("\n\n%s_Low: \n        ", name);
	for (CHX = 0; CHX < X_MAX; CHX++)
		LD_MSG(" X_%3d,", CHX);
	for (CHY = 0; CHY < Y_MAX; CHY++) {
		LD_MSG("\nY_%3dCH:", CHY);
		for (CHX = 0; CHX < X_MAX; CHX++)
			LD_MSG(" %5d,", data[CHX][CHY].ini.min);
	}

	LD_MSG("\n\n%s Datas: \n        ", name);
	for (CHX = 0; CHX < X_MAX; CHX++)
		LD_MSG(" X_%3d,", CHX);
	for (CHY = 0; CHY < Y_MAX; CHY++) {
		LD_MSG("\nY_%3dCH:", CHY);
		for (CHX = 0; CHX < X_MAX; CHX++) {
			data[CHX][CHY].raw.data = delac[CHX][CHY].data;

			if (data[CHX][CHY].raw.data > data[CHX][CHY].ini.max &&
			    check_max) {
				data[CHX][CHY].raw.max_st = NODE_FAIL;
				data[CHX][CHY].raw.status = NODE_FAIL;
				*fail_count += 1;
			} else if (data[CHX][CHY].raw.data <
				   data[CHX][CHY].ini.min && !check_max) {
				data[CHX][CHY].raw.min_st = NODE_FAIL;
				data[CHX][CHY].raw.status = NODE_FAIL;
				*fail_count += 1;
			}

			if (data[CHX][CHY].raw.status)
				LD_MSG("*%5d,", data[CHX][CHY].raw.data);
			else
				LD_MSG(" %5d,", data[CHX][CHY].raw.data);
		}
	}
	if (*fail_count > fail_count_limit)
		ret = _FAIL;
	LD_MSG("\n%s=%s\n", name, ret ? "FAIL":"PASS");
	return ret;
}

int viRunUniformityTest_6X()
{
	int ret = _SUCCESS;
	unsigned int CHY = 0, CHX = 0;
	int trans_num = 0;
	int BenchMaxFailCount = 0;
	int BenchMinFailCount = 0;

	ST.Uniformity.RawMaxFailCount = 0;
	ST.Uniformity.RawMinFailCount = 0;
	ST.Uniformity.Win1FailCount = 0;
	ST.Uniformity.Win2FailCount = 0;
	ST.Uniformity.RawMaxPass = true;
	ST.Uniformity.RawMinPass = true;
	ST.Uniformity.Win1Pass = true;
	ST.Uniformity.Win2Pass = true;

	LD_MSG("Uniformity Test Criteria:\n");
	trans_num = ptl.x_ch * ptl.y_ch;
	LD_MSG("Max_Threshold = %d\n", ST.Uniformity.Max_Threshold);
	LD_MSG("Up_FailCount = %d\n", ST.Uniformity.Up_FailCount);
	LD_MSG("Min_Threshold = %d\n", ST.Uniformity.Min_Threshold);
	LD_MSG("Low_FailCount = %d\n", ST.Uniformity.Low_FailCount);
	LD_MSG("Win1_Threshold = %d\n", ST.Uniformity.Win1_Threshold);
	LD_MSG("Win1_FailCount = %d\n", ST.Uniformity.Win1_FailCount);
	LD_MSG("Win2_Threshold = %d\n", ST.Uniformity.Win2_Threshold);
	LD_MSG("Win2_FailCount = %d\n", ST.Uniformity.Win2_FailCount);

	ModeCtrl_V6(ENTER_TEST_MODE, DISABLE_ENGINEER, 100);

	if (viGetCDCData_6X(TEST_MODE_V6_MC_RAW_NBK, trans_num) < 0)
		return _FAIL;

	if (ST.PFVer >= PROFILE_V1_0_2_0 &&
	    ST.Uniformity.En_allraw == false) {
		LD_MSG("Uniformity RawData\n Datas: \n        no test\n");
	} else {
		LD_MSG("Uniformity\n Datas: \n        ");
		for (CHX = 0; CHX < ptl.x_ch; CHX++)
			LD_MSG(" X_%3d,", CHX);
		for (CHY = 0; CHY < ptl.y_ch; CHY++) {
			LD_MSG("\nY_%3dCH:", CHY);
			for (CHX = 0; CHX < ptl.x_ch; CHX++) {
				ST.v6_unifor_daltc[CHX][CHY].data = uiTestDatas[CHY][CHX];
				ST.v6_unifor_daltc[CHX][CHY].status = NODE_PASS;
				ST.v6_unifor_daltc[CHX][CHY].max_st = NODE_PASS;
				ST.v6_unifor_daltc[CHX][CHY].min_st = NODE_PASS;

				//profile 1.0.2.0 version above use all node test
				if (ST.v6_unifor_daltc[CHX][CHY].data > ST.Uniformity.Max_Threshold ||
				    ST.v6_unifor_daltc[CHX][CHY].data < ST.Uniformity.Min_Threshold) {
					ST.v6_unifor_daltc[CHX][CHY].status = NODE_FAIL;
					if (ST.v6_unifor_daltc[CHX][CHY].data > ST.Uniformity.Max_Threshold) {
						ST.Uniformity.RawMaxFailCount++;
						ST.v6_unifor_daltc[CHX][CHY].max_st = NODE_FAIL;
					}
					if (ST.v6_unifor_daltc[CHX][CHY].data < ST.Uniformity.Min_Threshold) {
						ST.Uniformity.RawMinFailCount++;
						ST.v6_unifor_daltc[CHX][CHY].min_st = NODE_FAIL;
					}
				}
				if (ST.v6_unifor_daltc[CHX][CHY].status == NODE_FAIL)
					LD_MSG("*%5d,", ST.v6_unifor_daltc[CHX][CHY].data);
				else
					LD_MSG(" %5d,", ST.v6_unifor_daltc[CHX][CHY].data);
			}
		}
		if (ST.Uniformity.RawMaxFailCount > ST.Uniformity.Up_FailCount)
			ST.Uniformity.RawMaxPass = false;
		if (ST.Uniformity.RawMinFailCount > ST.Uniformity.Low_FailCount)
			ST.Uniformity.RawMinPass = false;
		if (!ST.Uniformity.RawMaxPass || !ST.Uniformity.RawMinPass)
			ret = _FAIL;
	}

	if (ST.PFVer >= PROFILE_V1_0_2_0 &&
	    ST.Uniformity.En_allwin1 == false) {
		LD_MSG("\n\nUniformity RawData Win1\n Datas: \n        no test\n");
	} else {
		LD_MSG("\n\nUniformity Win_1 Datas: \n        ");
		for (CHX = 0; CHX < ptl.x_ch; CHX++)
			LD_MSG(" X_%3d,", CHX);
		for (CHY = 1; CHY < ptl.y_ch; CHY++) {
			LD_MSG("\nY_%3dCH:", CHY - 1);
			for (CHX = 0; CHX < ptl.x_ch; CHX++) {
				ST.v6_unifor_win1[CHX][CHY-1].data = abs(uiTestDatas[CHY][CHX] - uiTestDatas[CHY - 1][CHX]);
				//profile 1.0.2.0 version above use all node test
				if (ST.v6_unifor_win1[CHX][CHY-1].data > ST.Uniformity.Win1_Threshold) {
					ST.Uniformity.Win1FailCount++;
					ST.v6_unifor_win1[CHX][CHY-1].status = NODE_FAIL;
					LD_MSG("*%5d,", ST.v6_unifor_win1[CHX][CHY-1].data);
				} else {
					ST.v6_unifor_win1[CHX][CHY-1].status = NODE_PASS;
					LD_MSG(" %5d,", ST.v6_unifor_win1[CHX][CHY-1].data);
				}
			}
		}
		if (ST.Uniformity.Win1FailCount >= ST.Uniformity.Win1_FailCount) {
			ST.Uniformity.Win1Pass = false;
			ret = _FAIL;
		}
	}

	if (ST.PFVer >= PROFILE_V1_0_2_0 &&
	    ST.Uniformity.En_allwin2 == false) {
		LD_MSG("Uniformity RawData Win1\n Datas: \n        no test\n");
	} else {
		LD_MSG("\n\nUniformity Win_2 Datas: \n        ");
		for (CHX = 0; CHX < ptl.x_ch - 1; CHX++)
			LD_MSG(" X_%3d,", CHX);
		LD_MSG("\n");
		for (CHY = 1; CHY < ptl.y_ch; CHY++) {
			LD_MSG("Y_%3dCH:", CHY - 1);
			for (CHX = 1; CHX < ptl.x_ch; CHX++) {
				ST.v6_unifor_win2[CHX-1][CHY-1].data = abs((uiTestDatas[CHY][CHX] - uiTestDatas[CHY - 1][CHX]) -
						(uiTestDatas[CHY][CHX - 1] - uiTestDatas[CHY - 1][CHX - 1]));
				//profile 1.0.2.0 version above use all node test
				if (ST.v6_unifor_win2[CHX-1][CHY-1].data > ST.Uniformity.Win2_Threshold) {
					ST.Uniformity.Win2FailCount++;
					ST.v6_unifor_win2[CHX-1][CHY-1].status = NODE_FAIL;
					LD_MSG("*%5d,",  ST.v6_unifor_win2[CHX-1][CHY-1].data);
				} else {
					ST.v6_unifor_win2[CHX-1][CHY-1].status = NODE_PASS;
					LD_MSG(" %5d,",  ST.v6_unifor_win2[CHX-1][CHY-1].data);
				}
			}
			LD_MSG("\n");
		}
		if (ST.Uniformity.Win2FailCount >= ST.Uniformity.Win2_FailCount) {
			ST.Uniformity.Win2Pass = false;
			ret = _FAIL;
		}
	}

	if (ST.Uniformity.En_bench == ENABLE_TEST) {
		if (NodeTest_V6("BenchMark MAX", ST.v6_unifor_daltc,
				ST.Uniformity.bench, ptl.x_ch, ptl.y_ch, 1,
				&BenchMaxFailCount, true) != _SUCCESS)
			ret = _FAIL;
		if (NodeTest_V6("BenchMark MIN", ST.v6_unifor_daltc,
				ST.Uniformity.bench, ptl.x_ch, ptl.y_ch, 1,
				&BenchMinFailCount, false) != _SUCCESS)
			ret = _FAIL;
	}

	if (ST.Uniformity.En_allraw == ENABLE_TEST) {
		ST.Uniformity.RawMaxPass = true;
		ST.Uniformity.RawMinPass = true;
		ST.Uniformity.RawMaxFailCount = 0;
		ST.Uniformity.RawMinFailCount = 0;
		if (NodeTest_V6("ANode_Raw MAX", ST.v6_unifor_daltc,
				ST.Uniformity.allraw, ptl.x_ch, ptl.y_ch,
				ST.Uniformity.Up_FailCount,
				&ST.Uniformity.RawMaxFailCount,
				true) != _SUCCESS) {
			ret = _FAIL;
			ST.Uniformity.RawMaxPass = false;
		}

		if (NodeTest_V6("ANode_Raw MIN", ST.v6_unifor_daltc,
				ST.Uniformity.allraw, ptl.x_ch, ptl.y_ch,
				ST.Uniformity.Low_FailCount,
				&ST.Uniformity.RawMinFailCount,
				false) != _SUCCESS) {
			ret = _FAIL;
			ST.Uniformity.RawMinPass = false;
		}
	}

	if (ST.Uniformity.En_allwin1 == ENABLE_TEST) {
		ST.Uniformity.Win1Pass = true;
		ST.Uniformity.Win1FailCount = 0;
		if (NodeTest_V6("ANode_Win1", ST.v6_unifor_win1,
				ST.Uniformity.allwin1, ptl.x_ch, ptl.y_ch - 1,
				ST.Uniformity.Win1_FailCount,
				&ST.Uniformity.Win1FailCount,
				true) != _SUCCESS) {
			ret = _FAIL;
			ST.Uniformity.Win1Pass = false;
		}
	}
	if (ST.Uniformity.En_allwin2 == ENABLE_TEST) {
		ST.Uniformity.Win2Pass = true;
		ST.Uniformity.Win2FailCount = 0;
		if (NodeTest_V6("ANode_Win2", ST.v6_unifor_win2,
				ST.Uniformity.allwin2, ptl.x_ch - 1,
				ptl.y_ch - 1, ST.Uniformity.Win2_FailCount,
				&ST.Uniformity.Win2FailCount,
				true) != _SUCCESS) {
			ret = _FAIL;
			ST.Uniformity.Win2Pass = false;
		}
	}

	ModeCtrl_V6(ENTER_NORMAL_MODE, DISABLE_ENGINEER, 100);

	return ret;
}

int viRunFWVerTest()
{
	if (inProtocolStyle == _Protocol_V3_)
		return viRunFWVerTest_3X();
	else if(inProtocolStyle == _Protocol_V6_)
		return viRunFWVerTest_6X();
	return _FAIL;
}

int viRunFWVerTest_3X()
{
	int i = 0;
	int ret = _SUCCESS;

	for(i = 0; i < 8; i++)
	{
		LD_MSG("FW version check:0x%x,0x%x\n", FWVersion[i], ST.fw_ver[i]);
		if(FWVersion[i] != ST.fw_ver[i])
		{
			ucSensorTestResult |= FWVERTION_TEST;
			ret = _FAIL;
		}
	}
	return ret;
}

int viRunFWVerTest_6X()
{
	int i = 0;

	if(ptl.ver <= PROTOCOL_V6_0_2) {
		LD_MSG("%s, The protocol no support\n", __func__);
		return _SUCCESS;
	}
	LD_MSG("FW Check Criteria:\n");
	if(ptl.block_num > 0 && ptl.block_num != ST.block_num) {
		LD_MSG("IC/INI Block number not match, IC:%d, INI:%d\n", ptl.block_num, ST.block_num);
		ST.block_num = ptl.block_num;
	}
	upg.blk = (struct BLOCK_DATA *)calloc(ST.block_num , sizeof(struct BLOCK_DATA));
	for(i = 0; i < 4; i++)
	{
		LD_MSG("FW version check:0x%x,0x%x\n", FWVersion[i], ST.fw_ver[i]);
		if(FWVersion[i] != ST.fw_ver[i])
		{
			return _FAIL;
		}
	}
	if(GetICMode()){
		if (ICMode == OP_MODE_APPLICATION) {
			ModeCtrl_V6(ENTER_SUSPEND_MODE, ENABLE_ENGINEER, 100);
		}
	}
	for(i = 0; i < ST.block_num; i++) {
		upg.blk[i].ic_crc = GetICBlockCrcNum(i, CRC_CALCULATION_FROM_IC);
		if(upg.blk[i].ic_crc != ST.master_crc[i]) {
			LD_MSG("Block:%d, IC CRC:0x%x, INI CRC:0x%x\n", i, upg.blk[i].ic_crc, ST.master_crc[i]);
			LD_ERR("CRC compare fail\n");
			return _FAIL;
		}
	}
	return _SUCCESS;
}

int viRunSelfTest()
{
	int ret = _SUCCESS;

	if (inProtocolStyle == _Protocol_V3_)
	{
		switch (KernelVersion[2])
		{
			case RDValue_MCUKernel_CDCVersion_8bit2315_:
			case RDValue_MCUKernel_CDCVersion_16bit2510_:
				{
					ret = viRunSelfTest_3X();
					break;
				}
			default:
				{
					LD_ERR("Error! SelfTest Function Don't Support This MCU !\n");
					ret =  _FAIL;
				}
		}
	}
	else if (inProtocolStyle == _Protocol_V6_)
	{

	}
	return ret;
}

int viRunDACTest()
{
	int ret = _SUCCESS;
	if (inProtocolStyle == _Protocol_V3_)
	{
		switch (KernelVersion[2])
		{
			case RDValue_MCUKernel_CDCVersion_8bit2315_:
			case RDValue_MCUKernel_CDCVersion_16bit2510_:
				{
					ret = viRunDACTest_3X();
					break;
				}
			default:
				{
					LD_ERR("Error! MultyTest Function Don't Support This MCU !\n");
					ret =  _FAIL;
				}
		}
	}
	else if (inProtocolStyle == _Protocol_V6_)
	{

	}
	return ret;
}

void vfPrintSensorTestResult_V3(int inFunctions)
{
	int error;
	char timebuf[60], logst[60];
	char result_file_name[256] = {0};
	FILE *result_file;
	time_t rawtime;

	if ((ucSensorTestResult & MICROSHORT_TEST) == MICROSHORT_TEST)
		LD_ERR("SensorTest: Short Test NG!\n");
	else if ((inFunctions & MICROSHORT_TEST) == MICROSHORT_TEST)
		LD_MSG("SensorTest: Short Test PASS!\n");

	if ((ucSensorTestResult & OPEN_TEST) == OPEN_TEST)
		LD_ERR("SensorTest: Open Test NG!\n");
	else if ((inFunctions & OPEN_TEST) == OPEN_TEST)
		LD_MSG("SensorTest: Open Test PASS!\n");
	if (ST.CreateGolden != 1) {
		if ((ucSensorTestResult & MIRCO_OPEN_TEST) == MIRCO_OPEN_TEST)
			LD_ERR("SensorTest: Mirco Open Test NG!\n");
		else if ((inFunctions & MIRCO_OPEN_TEST) == MIRCO_OPEN_TEST)
			LD_MSG("SensorTest: Mirco Open Test PASS!\n");
		if ((ucSensorTestResult & FWVERTION_TEST) == FWVERTION_TEST)
			LD_ERR("SensorTest: Check FW Version NG!\n");
		else if ((inFunctions & FWVERTION_TEST) == FWVERTION_TEST)
			LD_MSG("SensorTest: Check FW Version PASS!\n");
		if ((ucSensorTestResult & SELF_TEST) == SELF_TEST)
			LD_ERR("SensorTest: Self Test NG!\n");
		else if ((inFunctions & SELF_TEST) == SELF_TEST)
			LD_MSG("SensorTest: Self Test PASS!\n");

		if ((ucSensorTestResult & DAC_TEST) == DAC_TEST)
			LD_ERR("SensorTest: DAC Test NG!\n");
		else if ((inFunctions & DAC_TEST) == DAC_TEST)
			LD_MSG("SensorTest: DAC Test PASS!\n");

		if ((ucSensorTestResult & ALL_NODE_TEST) == ALL_NODE_TEST)
			LD_ERR("SensorTest: All Node Test NG!\n");
		else if ((inFunctions & ALL_NODE_TEST) == ALL_NODE_TEST)
			LD_MSG("SensorTest: All Node Test PASS!\n");

		if ((ucSensorTestResult & UNIFORMITY_TEST) == UNIFORMITY_TEST)
			LD_ERR("SensorTest: Uniformity Test NG!\n");
		else if ((inFunctions & UNIFORMITY_TEST) == UNIFORMITY_TEST)
			LD_MSG("SensorTest: Uniformity Test PASS!\n");
		if (!ucSensorTestResult)
			sprintf(logst, "PASS_");
		else
			sprintf(logst, "FAIL_");
	} else {
		if ((ucSensorTestResult & (MICROSHORT_TEST + OPEN_TEST)) > 0)
			sprintf(logst, "LOG_FAIL_");
		else
			sprintf(logst, "LOG_PASS_");
	}

	strftime(timebuf, 60, "%Y%m%d_%I%M%S", timeinfo);
	if (ST.LogPath == NULL || (ST.LogPath && !strlen(ST.LogPath)))
		ST.LogPath = (char *)".";
	sprintf(result_file_name,"%s/%s%s.csv", ST.LogPath, logst, timebuf);

	result_file = fopen(result_file_name, "w");
	if (!result_file)
		LD_ERR("fopen: %s failed\n", result_file_name);
	LD_MSG("test result (.csv) path => %s\n", result_file_name);

	fprintf(result_file, "===============================================================================\n");
	fprintf(result_file, "%s: Sensor Test\n", TOOL_VERSION);
	fprintf(result_file, "Confidentiality Notice:\n");
	fprintf(result_file, "Any information of this tool is confidential and privileged.\n");
	fprintf(result_file, "@2020 ILI TECHNOLOGY CORP. All Rights Reserved.\n");
	fprintf(result_file, "===============================================================================\n");
	fprintf(result_file, "Start_Testing_Time     ,%s", asctime(timeinfo));
	fprintf(result_file, "Bar_Code               ,\n");
	fprintf(result_file, "ProFile_Path           ,%s\n", g_szConfigPath);
	fprintf(result_file, "IC Type                ,ILI%x\n", ptl.ic);
	fprintf(result_file, "IC Channel             ,X:%d Y:%d\n", ptl.x_ch, ptl.y_ch);
	fprintf(result_file, "Test_Station           ,MODULE\n");
	fprintf(result_file, "ProFile_Version        ,%d.%d.%d.%d\n", ST.PFVer0, ST.PFVer1, ST.PFVer2, ST.PFVer3);
	fprintf(result_file, "ProFile_Date           ,%s\n", ST.PFDate);
	fprintf(result_file, "Report_Format_Version  ,%s\n", REPORT_VERSION);
	fprintf(result_file, "===============================================================================\n");

	if ((inFunctions & FWVERTION_TEST) == FWVERTION_TEST)
		vfSaveFWVerTestLog_V3(result_file);
	if ((inFunctions & MICROSHORT_TEST) == MICROSHORT_TEST)
		vfSaveShortTestLog_V3(result_file);
	if ((inFunctions & OPEN_TEST) == OPEN_TEST) {
		if (ST.UseNewFlow == 1)
			vfSaveOpenTestLog_NewFlow(result_file);
		else
			vfSaveOpenTestLog_V3(result_file);
	}
	if ((inFunctions & SELF_TEST) == SELF_TEST)
		vfSaveSelfTestLog(result_file);
	if ((inFunctions & DAC_TEST) == DAC_TEST)
		vfSaveDACTestLog(result_file);
	if ((inFunctions & ALL_NODE_TEST) == ALL_NODE_TEST)
		vfSaveAllNodeTestLog(result_file);
	if ((inFunctions & UNIFORMITY_TEST) == UNIFORMITY_TEST)
		vfSaveUniformityTestLog(result_file);
	if ((inFunctions & MIRCO_OPEN_TEST) == MIRCO_OPEN_TEST)
		vfSaveMircoOpenTestLog_V3(result_file);

	fprintf(result_file, "===============================================================================\n");
	fprintf(result_file, "Result_Summary           ,\n\n");
	if ((inFunctions & FWVERTION_TEST) == FWVERTION_TEST) {
		if ((ucSensorTestResult & FWVERTION_TEST) == FWVERTION_TEST)
			fprintf(result_file, "   {FW_Verify}                            ,NG ,\n");
		else
			fprintf(result_file, "   {FW_Verify}                            ,OK ,\n");
	}
	if ((inFunctions & MICROSHORT_TEST) == MICROSHORT_TEST) {
		if ((ucSensorTestResult & MICROSHORT_TEST) == MICROSHORT_TEST)
			fprintf(result_file, "   {Short_Test}                           ,NG ,\n");
		else
			fprintf(result_file, "   {Short_Test}                           ,OK ,\n");
	}
	if ((inFunctions & OPEN_TEST) == OPEN_TEST) {
		if ((ucSensorTestResult & OPEN_TEST) == OPEN_TEST)
			fprintf(result_file, "   {Open_Test}                            ,NG ,\n");
		else
			fprintf(result_file, "   {Open_Test}                            ,OK ,\n");
	}
	if ((inFunctions & ALL_NODE_TEST) == ALL_NODE_TEST) {
		if (ST.useINI) {
			if ((ucSensorTestResult & ALL_NODE_TEST) == ALL_NODE_TEST)
				fprintf(result_file, "   {Uniformity_Test}                      ,NG ,\n");
			else
				fprintf(result_file, "   {Uniformity_Test}                      ,OK ,\n");
		} else {
			if ((ucSensorTestResult & ALL_NODE_TEST) == ALL_NODE_TEST)
				fprintf(result_file, "   {All node Test}                        ,NG ,\n");
			else
				fprintf(result_file, "   {All node Test}                        ,OK ,\n");
		}
	}
	if ((inFunctions & UNIFORMITY_TEST) == UNIFORMITY_TEST) {
		if ((ucSensorTestResult & UNIFORMITY_TEST) == UNIFORMITY_TEST)
			fprintf(result_file, "   {Uniformity_Test}                      ,NG ,\n");
		else
			fprintf(result_file, "   {Uniformity_Test}                      ,OK ,\n");
	}
	if ((inFunctions & MIRCO_OPEN_TEST) == MIRCO_OPEN_TEST) {
		if ((ucSensorTestResult & MIRCO_OPEN_TEST) == MIRCO_OPEN_TEST)
			fprintf(result_file, "   {MicroOpen_Test}                       ,NG ,\n");
		else
			fprintf(result_file, "   {MicroOpen_Test}                       ,OK ,\n");
	}

	fprintf(result_file, "===============================================================================\n");
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	fprintf(result_file, "End_Testing_Time       ,%s", asctime(timeinfo));
	fprintf(result_file, "Total_Testing_Time     ,%ld.%ld sec\n", tv.tv_sec, tv.tv_usec);
	fprintf(result_file, "===============================================================================\n");

	error = remove(fileName);
	fclose(result_file);
	if (error < 0)
		return;
}

void vfPrintSensorTestResult_V6(int inFunctions)
{
	int error;
	char timebuf[60], logst[60];
	char result_file_name[256] = {0};
	FILE *result_file;
	time_t rawtime;

	if ((ucSensorTestResult & MICROSHORT_TEST) == MICROSHORT_TEST)
		LD_ERR("SensorTest: Short Test NG!\n");
	else if ((inFunctions & MICROSHORT_TEST) == MICROSHORT_TEST)
		LD_MSG("SensorTest: Short Test PASS!\n");

	if ((ucSensorTestResult & OPEN_TEST) == OPEN_TEST)
		LD_ERR("SensorTest: Open Test NG!\n");
	else if ((inFunctions & OPEN_TEST) == OPEN_TEST)
		LD_MSG("SensorTest: Open Test PASS!\n");
	if (ST.CreateGolden != 1) {
		if ((ucSensorTestResult & MIRCO_OPEN_TEST) == MIRCO_OPEN_TEST)
			LD_ERR("SensorTest: Mirco Open Test NG!\n");
		else if ((inFunctions & MIRCO_OPEN_TEST) == MIRCO_OPEN_TEST)
			LD_MSG("SensorTest: Mirco Open Test PASS!\n");
		if ((ucSensorTestResult & FWVERTION_TEST) == FWVERTION_TEST)
			LD_ERR("SensorTest: Check FW Version NG!\n");
		else if ((inFunctions & FWVERTION_TEST) == FWVERTION_TEST)
			LD_MSG("SensorTest: Check FW Version PASS!\n");
		if ((ucSensorTestResult & SELF_TEST) == SELF_TEST)
			LD_ERR("SensorTest: Self Test NG!\n");
		else if ((inFunctions & SELF_TEST) == SELF_TEST)
			LD_MSG("SensorTest: Self Test PASS!\n");

		if ((ucSensorTestResult & DAC_TEST) == DAC_TEST)
			LD_ERR("SensorTest: DAC Test NG!\n");
		else if ((inFunctions & DAC_TEST) == DAC_TEST)
			LD_MSG("SensorTest: DAC Test PASS!\n");

		if ((ucSensorTestResult & ALL_NODE_TEST) == ALL_NODE_TEST)
			LD_ERR("SensorTest: All Node Test NG!\n");
		else if ((inFunctions & ALL_NODE_TEST) == ALL_NODE_TEST)
			LD_MSG("SensorTest: All Node Test PASS!\n");

		if ((ucSensorTestResult & UNIFORMITY_TEST) == UNIFORMITY_TEST)
			LD_ERR("SensorTest: Uniformity Test NG!\n");
		else if ((inFunctions & UNIFORMITY_TEST) == UNIFORMITY_TEST)
			LD_MSG("SensorTest: Uniformity Test PASS!\n");
		if (!ucSensorTestResult)
			sprintf(logst, "PASS_");
		else
			sprintf(logst, "FAIL_");
	} else {
		if ((ucSensorTestResult & (MICROSHORT_TEST + OPEN_TEST)) > 0)
			sprintf(logst, "LOG_FAIL_");
		else
			sprintf(logst, "LOG_PASS_");
	}

	strftime(timebuf, 60, "%Y%m%d_%I%M%S", timeinfo);
	if (ST.LogPath == NULL || (ST.LogPath && !strlen(ST.LogPath)))
		ST.LogPath = (char *)".";

	sprintf(result_file_name, "%s/%s%s.csv", ST.LogPath, logst, timebuf);

	result_file = fopen(result_file_name, "w");
	if (!result_file) {
		LD_ERR("fopen: %s failed\n", result_file_name);
		return;
	}
	LD_MSG("test result (.csv) path => %s\n", result_file_name);

	fprintf(result_file, "===============================================================================\n");
	fprintf(result_file, "%s: Sensor Test\n", TOOL_VERSION);
	fprintf(result_file, "Confidentiality Notice:\n");
	fprintf(result_file, "Any information of this tool is confidential and privileged.\n");
	fprintf(result_file, "@2020 ILI TECHNOLOGY CORP. All Rights Reserved.\n");
	fprintf(result_file, "===============================================================================\n");
	fprintf(result_file, "Start_Testing_Time     ,%s", asctime(timeinfo));
	fprintf(result_file, "Bar_Code               ,\n");
	fprintf(result_file, "ProFile_Path           ,%s\n", g_szConfigPath);
	fprintf(result_file, "IC Type                ,ILI%x\n", ptl.ic);
	fprintf(result_file, "IC Channel             ,X:%d Y:%d\n", ptl.x_ch, ptl.y_ch);
	fprintf(result_file, "Test_Station           ,MODULE\n");
	fprintf(result_file, "ProFile_Version        ,%d.%d.%d.%d\n", ST.PFVer0, ST.PFVer1, ST.PFVer2, ST.PFVer3);
	fprintf(result_file, "ProFile_Date           ,%s\n", ST.PFDate);
	fprintf(result_file, "Report_Format_Version  ,%s\n", REPORT_VERSION);
	fprintf(result_file, "===============================================================================\n");

	if ((inFunctions & FWVERTION_TEST) == FWVERTION_TEST)
		vfSaveFWVerTestLog_V6(result_file);
	if ((inFunctions & MICROSHORT_TEST) == MICROSHORT_TEST)
		vfSaveShortTestLog_V6(result_file);
	if ((inFunctions & OPEN_TEST) == OPEN_TEST)
		vfSaveOpenTestLog_V6(result_file);
	if ((inFunctions & UNIFORMITY_TEST) == UNIFORMITY_TEST)
		vfSaveUniformityTestLog_V6(result_file);
	if ((inFunctions & MIRCO_OPEN_TEST) == MIRCO_OPEN_TEST)
		vfSaveMircoOpenTestLog_V6(result_file);

	fprintf(result_file, "===============================================================================\n");
	fprintf(result_file, "Result_Summary           ,\n\n");
	if ((inFunctions & FWVERTION_TEST) == FWVERTION_TEST) {
		if ((ucSensorTestResult & FWVERTION_TEST) == FWVERTION_TEST)
			fprintf(result_file, "   {FW_Verify}                            ,NG ,\n");
		else
			fprintf(result_file, "   {FW_Verify}                            ,OK ,\n");
	}
	if ((inFunctions & MICROSHORT_TEST) == MICROSHORT_TEST) {
		if ((ucSensorTestResult & MICROSHORT_TEST) == MICROSHORT_TEST)
			fprintf(result_file, "   {Short_Test}                           ,NG ,\n");
		else
			fprintf(result_file, "   {Short_Test}                           ,OK ,\n");
	}
	if ((inFunctions & OPEN_TEST) == OPEN_TEST) {
		if ((ucSensorTestResult & OPEN_TEST) == OPEN_TEST)
			fprintf(result_file, "   {Open_Test}                            ,NG ,\n");
		else
			fprintf(result_file, "   {Open_Test}                            ,OK ,\n");
	}
	if ((inFunctions & UNIFORMITY_TEST) == UNIFORMITY_TEST) {
		if ((ucSensorTestResult & UNIFORMITY_TEST) == UNIFORMITY_TEST)
			fprintf(result_file, "   {Uniformity_Test}                      ,NG ,\n");
		else
			fprintf(result_file, "   {Uniformity_Test}                      ,OK ,\n");
	}
	if ((inFunctions & MIRCO_OPEN_TEST) == MIRCO_OPEN_TEST) {
		if ((ucSensorTestResult & MIRCO_OPEN_TEST) == MIRCO_OPEN_TEST)
			fprintf(result_file, "   {MicroOpen_Test}                       ,NG ,\n");
		else
			fprintf(result_file, "   {MicroOpen_Test}                       ,OK ,\n");
	}

	fprintf(result_file, "===============================================================================\n");
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	fprintf(result_file, "End_Testing_Time       ,%s", asctime(timeinfo));
	fprintf(result_file, "Total_Testing_Time     ,%ld.%ld sec\n", tv.tv_sec, tv.tv_usec);
	fprintf(result_file, "===============================================================================\n");

	error = remove(fileName);
	fclose(result_file);
	if (error < 0)
		return;
}

void vfPrintSensorTestResult(int inFunctions)
{
	LD_MSG("\n========================TestResult========================\n\n");
	if (inProtocolStyle == _Protocol_V3_)
		vfPrintSensorTestResult_V3(inFunctions);
	else if (inProtocolStyle == _Protocol_V6_)
		vfPrintSensorTestResult_V6(inFunctions);
	LD_MSG("\n==========================================================\n");
}

int viRunShortTest()
{
	int ret = 1;
	if (inProtocolStyle == _Protocol_V3_)
	{
		ret = viRunShortTest_3X();
	}
	else if (inProtocolStyle == _Protocol_V6_)
	{
		ret = viRunShortTest_6X();
	}
	return ret;
}

int viRunUniformityTest()
{
	int ret = _SUCCESS;

	if (inProtocolStyle == _Protocol_V3_ &&
	    (KernelVersion[2] == RDValue_MCUKernel_CDCVersion_8bit2315_ ||
	     KernelVersion[2] == RDValue_MCUKernel_CDCVersion_16bit2510_)) {
		if(ST.UseNewFlow == 1) {
			ret = viRunUniformityTest_3X();
		} else {
			LD_ERR("Error! UniformityTest Function Just Support When UseNewFlow = 1 !\n");
			ret = _FAIL;
		}
	} else if (inProtocolStyle == _Protocol_V6_) {
		ret = viRunUniformityTest_6X();
	} else {
		LD_ERR("Error! UniformityTest Function Don't Support This MCU !\n");
		ret = _FAIL;
	}

	return ret;
}

int viRunAllNodeTest()
{
	int ret = _SUCCESS;

	if (inProtocolStyle == _Protocol_V3_ && (KernelVersion[2] == RDValue_MCUKernel_CDCVersion_8bit2315_ || KernelVersion[2] == RDValue_MCUKernel_CDCVersion_16bit2510_))
	{
		ret = viRunAllNodeTest_3X();
	}
	else if (inProtocolStyle == _Protocol_V3_)
	{
		LD_ERR("Error! AllNodeTest Function Don't Support This MCU !\n");
		ret = _FAIL;
	}
	else if (inProtocolStyle == _Protocol_V6_)
	{

	}
	return ret;
}
int init_sentest_array()
{
	unsigned int i = 0;

	//set short array
	ST.short_daltc = (short int**)calloc(4, sizeof(short int*));
	for(i = 0; i < 4; i++) {
		if(i % 2 == 0)
			ST.short_daltc[i] = (short int*)calloc(ptl.x_ch, sizeof(short int));
		else
			ST.short_daltc[i] = (short int*)calloc(ptl.y_ch, sizeof(short int));
	}
	ST.v6_short_daltc = (SensorTest_Node**)calloc(2 , sizeof(SensorTest_Node*));
	ST.v6_short_daltc[0] = (SensorTest_Node*)calloc(ptl.x_ch , sizeof(SensorTest_Node));
	ST.v6_short_daltc[1] = (SensorTest_Node*)calloc(ptl.y_ch , sizeof(SensorTest_Node));
	if(ptl.key_mode == ILITEK_HW_KEY_MODE) {
		//Self x is key number + Self y is 1
		ST.Short.key_daltc = (SensorTest_Node*)calloc(ptl.key_num + 1 , sizeof(SensorTest_Node));
	}
	ST.v6_open_daltc = (SensorTest_Node**)calloc(ptl.x_ch, sizeof(SensorTest_Node*));
	for(i = 0; i < ptl.x_ch; i++)
		ST.v6_open_daltc[i] = (SensorTest_Node*)calloc(ptl.y_ch, sizeof(SensorTest_Node));
	ST.Open.key_daltc = (SensorTest_Node*)calloc(ptl.key_num, sizeof(SensorTest_Node));
	ST.v6_unifor_daltc = (SensorTest_Node**)calloc(ptl.x_ch, sizeof(SensorTest_Node*));
	for(i = 0; i < ptl.x_ch; i++)
		ST.v6_unifor_daltc[i] = (SensorTest_Node*)calloc(ptl.y_ch, sizeof(SensorTest_Node));
	ST.v6_unifor_win1 = (SensorTest_Node**)calloc(ptl.x_ch, sizeof(SensorTest_Node*));
	for(i = 0; i < ptl.x_ch; i++)
		ST.v6_unifor_win1[i] = (SensorTest_Node*)calloc(ptl.y_ch, sizeof(SensorTest_Node));
	ST.v6_unifor_win2 = (SensorTest_Node**)calloc(ptl.x_ch, sizeof(SensorTest_Node*));
	for(i = 0; i < ptl.x_ch; i++)
		ST.v6_unifor_win2[i] = (SensorTest_Node*)calloc(ptl.y_ch, sizeof(SensorTest_Node));
	ST.Uniformity.bench = (SensorTest_BenBenchmark_Node**)calloc(ptl.x_ch, sizeof(SensorTest_BenBenchmark_Node*));
	for(i = 0; i < ptl.x_ch; i++)
		ST.Uniformity.bench[i] = (SensorTest_BenBenchmark_Node*)calloc(ptl.y_ch, sizeof(SensorTest_BenBenchmark_Node));
	ST.Uniformity.allraw = (SensorTest_BenBenchmark_Node**)calloc(ptl.x_ch, sizeof(SensorTest_BenBenchmark_Node*));
	for(i = 0; i < ptl.x_ch; i++)
		ST.Uniformity.allraw[i] = (SensorTest_BenBenchmark_Node*)calloc(ptl.y_ch, sizeof(SensorTest_BenBenchmark_Node));
	ST.Uniformity.allwin1 = (SensorTest_BenBenchmark_Node**)calloc(ptl.x_ch, sizeof(SensorTest_BenBenchmark_Node*));
	for(i = 0; i < ptl.x_ch; i++)
		ST.Uniformity.allwin1[i] = (SensorTest_BenBenchmark_Node*)calloc((ptl.y_ch - 1), sizeof(SensorTest_BenBenchmark_Node));
	ST.Uniformity.allwin2 = (SensorTest_BenBenchmark_Node**)calloc((ptl.x_ch - 1), sizeof(SensorTest_BenBenchmark_Node*));
	for(i = 0; i < ptl.x_ch - 1; i++)
		ST.Uniformity.allwin2[i] = (SensorTest_BenBenchmark_Node*)calloc((ptl.y_ch - 1), sizeof(SensorTest_BenBenchmark_Node));
	//set open array

	ST.open_20V_daltc = (short int**)calloc(ptl.x_ch, sizeof(short int*));
	for(i = 0; i < ptl.x_ch; i++)
		ST.open_20V_daltc[i] = (short int*)calloc(ptl.y_ch, sizeof(short int));
	ST.open_6V_daltc = (short int**)calloc(ptl.x_ch, sizeof(short int*));
	for(i = 0; i < ptl.x_ch; i++)
		ST.open_6V_daltc[i] = (short int*)calloc(ptl.y_ch, sizeof(short int));
	ST.open_20_6V_daltc = (short int**)calloc(ptl.x_ch, sizeof(short int*));
	for(i = 0; i < ptl.x_ch; i++)
		ST.open_20_6V_daltc[i] = (short int*)calloc(ptl.y_ch, sizeof(short int));
	ST.open_Rx_diff = (SensorTest_Node**)calloc(ptl.x_ch, sizeof(SensorTest_Node*));
	for(i = 0; i < ptl.x_ch; i++)
		ST.open_Rx_diff[i] = (SensorTest_Node*)calloc(ptl.y_ch, sizeof(SensorTest_Node));
	ST.open_Tx_diff = (SensorTest_Node**)calloc(ptl.x_ch, sizeof(SensorTest_Node*));
	for(i = 0; i < ptl.x_ch; i++)
		ST.open_Tx_diff[i] = (SensorTest_Node*)calloc(ptl.y_ch, sizeof(SensorTest_Node));
	ST.open_Tx_diff_new = (short int**)calloc(ptl.x_ch, sizeof(short int*));
	for(i = 0; i < ptl.x_ch; i++)
		ST.open_Tx_diff_new[i] = (short int*)calloc(ptl.y_ch, sizeof(short int));
	ST.uiYDC_Range = (short int**)calloc(ptl.x_ch, sizeof(short int*));
	for(i = 0; i < ptl.x_ch; i++)
		ST.uiYDC_Range[i] = (short int*)calloc(ptl.y_ch, sizeof(short int));

	ST.open_daltc = (short int**)calloc(ptl.x_ch, sizeof(short int*));
	for(i = 0; i < ptl.x_ch; i++)
		ST.open_daltc[i] = (short int*)calloc(ptl.y_ch, sizeof(short int));
	ST.open_Rx_daltc = (short int**)calloc(ptl.x_ch, sizeof(short int*));
	for(i = 0; i < ptl.x_ch; i++)
		ST.open_Rx_daltc[i] = (short int*)calloc(ptl.y_ch, sizeof(short int));
	ST.open_Tx_daltc = (short int**)calloc(ptl.x_ch, sizeof(short int*));
	for(i = 0; i < ptl.x_ch; i++)
		ST.open_Tx_daltc[i] = (short int*)calloc(ptl.y_ch, sizeof(short int));
	ST.Tx_Avdiff_daltc = (SensorTest_Node*)calloc(ptl.y_ch, sizeof(SensorTest_Node));
	//Mirco Open
	//ST.MOpen.tx_avg = (SensorTest_BenBenchmark_Node**)calloc((ptl.y_ch - 1)* sizeof(SensorTest_BenBenchmark_Node*));
	ST.MOpen.tx_avg = (SensorTest_BenBenchmark_Node**)calloc(1, sizeof(SensorTest_BenBenchmark_Node*));
	//for(i = 0; i < ptl.y_ch; i++)
	ST.MOpen.tx_avg[0] = (SensorTest_BenBenchmark_Node*)calloc((ptl.y_ch - 1), sizeof(SensorTest_BenBenchmark_Node));
	ST.MOpen.rx_diff = (SensorTest_BenBenchmark_Node**)calloc(ptl.x_ch, sizeof(SensorTest_BenBenchmark_Node*));
	for(i = 0; i < ptl.x_ch; i++)
		ST.MOpen.rx_diff[i] = (SensorTest_BenBenchmark_Node*)calloc(ptl.y_ch, sizeof(SensorTest_BenBenchmark_Node));

	//set uniformity
	ST.unifor_daltc = (short int**)calloc(ptl.x_ch, sizeof(short int*));
	for(i = 0; i < ptl.x_ch; i++)
		ST.unifor_daltc[i] = (short int*)calloc(ptl.y_ch, sizeof(short int));
	//set self
	ST.self_xdaltc = (short int*)calloc(ptl.x_ch, sizeof(short int));
	ST.self_ydaltc = (short int*)calloc(ptl.y_ch, sizeof(short int));
	//all node
	ST.all_daltc = (short int**)calloc(ptl.x_ch, sizeof(short int*));
	for(i = 0; i < ptl.x_ch; i++)
		ST.all_daltc[i] = (short int*)calloc(ptl.y_ch, sizeof(short int));
	ST.all_w1_data = (short int**)calloc(ptl.x_ch, sizeof(short int*));
	for(i = 0; i < ptl.x_ch; i++)
		ST.all_w1_data[i] = (short int*)calloc(ptl.y_ch, sizeof(short int));
	ST.all_w2_data = (short int**)calloc(ptl.x_ch, sizeof(short int*));
	for(i = 0; i < ptl.x_ch; i++)
		ST.all_w2_data[i] = (short int*)calloc(ptl.y_ch, sizeof(short int));
	//DAC
	ST.dac_sc_p = (short int*)calloc((ptl.x_ch + ptl.y_ch), sizeof(short int));
	ST.dac_sc_n = (short int*)calloc((ptl.x_ch + ptl.y_ch), sizeof(short int));
	ST.dac_mc_p = (short int**)calloc(ptl.x_ch, sizeof(short int*));
	for(i = 0; i < ptl.x_ch; i++)
		ST.dac_mc_p[i] = (short int*)calloc(ptl.y_ch, sizeof(short int));
	ST.dac_mc_n = (short int**)calloc(ptl.x_ch, sizeof(short int*));
	for(i = 0; i < ptl.x_ch; i++)
		ST.dac_mc_n[i] = (short int*)calloc(ptl.y_ch, sizeof(short int));
	//FW check
	ST.LogPath = (char *)calloc(1024, sizeof(char));

	return _SUCCESS;
}
int viRunSensorTest(int inFunctions)
{
	int ret = 0;

	switch_irq(0);
	if (inProtocolStyle == _Protocol_V3_)
		ret = viRunSensorTest_V3(inFunctions);
	else if(inProtocolStyle == _Protocol_V6_)
		ret = viRunSensorTest_V6(inFunctions);
	switch_irq(1);

	if (ret == _SUCCESS)
		print_PASS_or_FAIL(true);
	else
		print_PASS_or_FAIL(false);

	return ret;
}
int viRunSensorTest_V3(int inFunctions)
{
	int ret = _SUCCESS;
	ucSensorTestResult = 0;
	time_t rawtime;

	usleep(100000);

	time(&rawtime);
	timeinfo = localtime(&rawtime);
	gettimeofday(&tv , &tz);
	basetime = tv.tv_sec;

	if (viSetTestMode(true, 100) != _FAIL &&
	    viGetPanelInfor() != _FAIL && inFunctions > 0) {
		if (ICMode == 0x55) {
			ret = _FAIL;
			goto END;
		}

		init_sentest_array();
		//read profile set sensor
		if (strlen((char *)IniPath) != 0)
			inFunctions = ReadST();

		usleep(200000);
		if ((inFunctions & FWVERTION_TEST) == FWVERTION_TEST && _FAIL == viRunFWVerTest())
		{
			ucSensorTestResult |= FWVERTION_TEST;
			ret = _FAIL;
			LD_ERR("Error! Get FW Failed!!\n");
		}
		if ((inFunctions & MICROSHORT_TEST) == 1 && _FAIL == viRunShortTest())
		{
			ret = _FAIL;
			ucSensorTestResult |= MICROSHORT_TEST;
			LD_ERR("Error! Get ShortDatas Failed!!\n");
		}

		usleep(200000);
		if ((inFunctions & OPEN_TEST) == 2 && _FAIL == viRunOpenTest())
		{
			ret = _FAIL;
			ucSensorTestResult |= OPEN_TEST;
			LD_ERR("Error! Get OpenDatas Failed!!\n");
		}
		if ((inFunctions & MIRCO_OPEN_TEST) == MIRCO_OPEN_TEST && _FAIL == viRunMircoOpenTest())
		{
			ucSensorTestResult |= MIRCO_OPEN_TEST;
			ret = _FAIL;
		}
		usleep(200000);
		if ((inFunctions & SELF_TEST) == 4 && _FAIL == viRunSelfTest())
		{
			ret = _FAIL;
			ucSensorTestResult |= SELF_TEST;
			LD_ERR("Error! Get SelfDatas Failed!!\n");
		}

		usleep(200000);
		if ((inFunctions & DAC_TEST) == 8 && _FAIL == viRunDACTest())
		{
			ret = _FAIL;
			ucSensorTestResult |= DAC_TEST;
			LD_ERR("Error! Get DACDatas Failed!!\n");
		}

		usleep(200000);
		if ((inFunctions & ALL_NODE_TEST) == 0x10 && _FAIL == viRunAllNodeTest())
		{
			ret = _FAIL;
			ucSensorTestResult |= ALL_NODE_TEST;
			LD_ERR("Error! Get AllNodeDatas Failed!!\n");
		}

		usleep(200000);
		if ((inFunctions & UNIFORMITY_TEST) == 0x20 && _FAIL == viRunUniformityTest())
		{
			ret = _FAIL;
			ucSensorTestResult |= UNIFORMITY_TEST;
			LD_ERR("Error! Get UniformityDatas Failed!!\n");
		}

		gettimeofday(&tv , &tz);
		tv.tv_sec -= basetime;

		//----------------Print Result---------------
		vfPrintSensorTestResult(inFunctions);
	}
	else
	{
		LD_ERR("Error! Get Base Infor Failed!!\n");
		ret = _FAIL;
	}

END:
	viSetTestMode(false, 100);
	return ret;
}

int viRunSensorTest_V6(int inFunctions)
{
	int ret = _SUCCESS;
	ucSensorTestResult = 0;
	time_t rawtime;

	usleep(100000);
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	gettimeofday(&tv , &tz);
	basetime = tv.tv_sec;

	if (viSetTestMode(true, 100) != _FAIL &&
	    viGetPanelInfor() != _FAIL && inFunctions > 0) {
		if (ICMode == 0x55) {
			ret = _FAIL;
			goto END;
		}

		init_sentest_array();

		if (strlen((char *)IniPath) != 0)
			inFunctions = ReadST();

		check_use_default_set();
		if (inFunctions == _FAIL) {
			ret = _FAIL;
			goto END;
		}
		if ((inFunctions & FWVERTION_TEST) == FWVERTION_TEST && _FAIL == viRunFWVerTest())
		{
			ucSensorTestResult |= FWVERTION_TEST;
			ret = _FAIL;
		}
		if ((inFunctions & MICROSHORT_TEST) == 1 && _FAIL == viRunShortTest())
		{
			ucSensorTestResult |= MICROSHORT_TEST;
			ret = _FAIL;
		}
		if ((inFunctions & OPEN_TEST) == OPEN_TEST && _FAIL == viRunOpenTest())
		{
			ucSensorTestResult |= OPEN_TEST;
			ret = _FAIL;
		}
		if ((inFunctions & MIRCO_OPEN_TEST) == MIRCO_OPEN_TEST && _FAIL == viRunMircoOpenTest())
		{
			ucSensorTestResult |= MIRCO_OPEN_TEST;
			ret = _FAIL;
		}
		if ((inFunctions & SELF_TEST) == SELF_TEST && _FAIL == viRunSelfTest())
		{
			ucSensorTestResult |= SELF_TEST;
			ret = _FAIL;
		}
		if ((inFunctions & DAC_TEST) == DAC_TEST && _FAIL == viRunDACTest())
		{
			ucSensorTestResult |= DAC_TEST;
			ret = _FAIL;
		}
		if ((inFunctions & ALL_NODE_TEST) == ALL_NODE_TEST && _FAIL == viRunAllNodeTest())
		{
			ucSensorTestResult |= ALL_NODE_TEST;
			ret = _FAIL;
		}
		if ((inFunctions & UNIFORMITY_TEST) == UNIFORMITY_TEST && _FAIL == viRunUniformityTest())
		{
			ucSensorTestResult |= UNIFORMITY_TEST;
			ret = _FAIL;
		}

		gettimeofday(&tv, &tz);
		tv.tv_sec -= basetime;
		//----------------Print Result---------------
		vfPrintSensorTestResult(inFunctions);
	}
	else
	{
		LD_ERR("Error! Get Base Infor Failed!!\n");
		ret = _FAIL;
	}
END:
	viSetTestMode(false, 100);
	return ret;
}

int ReadST()
{
	if (inProtocolStyle == _Protocol_V3_)
		return ReadST_V3();
	else if (inProtocolStyle == _Protocol_V6_)
		return ReadST_V6();

	return _FAIL;
}

int ReadST_V3()
{
	int i;
	char buf[INI_MAX_PATH], *tmp;
	int inFunctions = 0;
	int ret;

	ST.useINI = false;

	//read ini test  Criteria
	memset(buf, 0, sizeof(buf));
	memset(g_szConfigPath, 0, sizeof(g_szConfigPath));
	//LD_MSG("=>Get sensor test criteria\n");
	strcpy(g_szConfigPath, (char *)IniPath);
	if (strstr((char *)IniPath, PROFILE_FORMAT_DAT) != NULL) {
        	ST.profile = PROFILE_DAT;
		LD_MSG("Profile file path %s\n", g_szConfigPath);
		LD_MSG("ini path=%s\n",IniPath);

		ret = GetIniKeyInt("Profile", "Version", g_szConfigPath);
		if (ret != _FAIL) {
			ST.PFVer3 = atoi(&tmpstr[0]);
			ST.PFVer2 = atoi(&tmpstr[2]);
			ST.PFVer1 = atoi(&tmpstr[4]);
			ST.PFVer0 = atoi(&tmpstr[6]);
		}

		ST.UseNewFlow = GetIniKeyInt("Profile", "UseNewMPFlow", g_szConfigPath);
		LD_MSG("%s,%d,UseNewFlow = %d\n", __func__, __LINE__, ST.UseNewFlow);
		ST.CreateGolden = GetIniKeyInt("Profile", "CreateGolden", g_szConfigPath);
		LD_MSG("%s,%d,CreateGolden = %d\n", __func__, __LINE__, ST.CreateGolden);
		ST.OffsetValue = GetIniKeyInt("Profile", "OffsetValue", g_szConfigPath);
		LD_MSG("%s,%d,OffsetValue = %d\n", __func__, __LINE__, ST.OffsetValue);

		tmp = GetIniKeyString("Profile", "LogPath", g_szConfigPath);
		if (tmp != NULL) {
			strcpy(ST.LogPath, tmp);
			if (access(ST.LogPath, F_OK | W_OK) == 0) {
				LD_MSG("LogPath:%s\n", ST.LogPath);
			} else {
				ST.LogPath = getcwd(NULL, 0);
				LD_ERR("[INI]LogPath no directory, use defaule path:%s\n", ST.LogPath);
			}
		}

		if (inProtocolStyle != _Protocol_V3_)
			ST.UseNewFlow = 0;

		ST.fw_ver[0] = GetIniKeyInt("PannelInformation", "FWVersion0", g_szConfigPath);
		ST.fw_ver[1] = GetIniKeyInt("PannelInformation", "FWVersion1", g_szConfigPath);
		ST.fw_ver[2] = GetIniKeyInt("PannelInformation", "FWVersion2", g_szConfigPath);
		ST.fw_ver[3] = GetIniKeyInt("PannelInformation", "FWVersion3", g_szConfigPath);
		ST.fw_ver[4] = GetIniKeyInt("PannelInformation", "FWVersion4", g_szConfigPath);
		ST.fw_ver[5] = GetIniKeyInt("PannelInformation", "FWVersion5", g_szConfigPath);
		ST.fw_ver[6] = GetIniKeyInt("PannelInformation", "FWVersion6", g_szConfigPath);
		ST.fw_ver[7] = GetIniKeyInt("PannelInformation", "FWVersion7", g_szConfigPath);
		LD_MSG("FWVersion:");
		for(i = 0; i < 8; i++)
			LD_MSG("%2x.",ST.fw_ver[i]);
		LD_MSG("\n");
		ST.dat_format = check_ini_section("TestItem", g_szConfigPath);
		LD_MSG("%s,%d,dat_format = %d\n", __func__, __LINE__, ST.dat_format);
		ST.FWVersion_test = GetIniKeyInt("TestItem", "FWVersion", g_szConfigPath);
		if (ST.FWVersion_test == 1)
			inFunctions += FWVERTION_TEST;

		//--------------------------profile OpenShort test --------------------------//
		ST.Short.Threshold = GetIniKeyInt("TestItem", "TraceLoading_Window", g_szConfigPath);
		//ST. = GetIniKeyInt("TestItem", "TraceLoading_RefData_Key", g_szConfigPath); //profile exist, but daemon not use.
		ST.OpenShort_test = GetIniKeyInt("TestItem", "OpenShort", g_szConfigPath);
		if (ST.OpenShort_test == 1)
			inFunctions += (OPEN_TEST) + (MICROSHORT_TEST);

		ST.Open_Threshold = GetIniKeyInt("TestItem", "OpenShort_Threshold", g_szConfigPath);
		LD_MSG("%s,%d,Open_Threshold = %d\n", __func__, __LINE__, ST.Open_Threshold);
		ST.Open_RX_Delta_Threshold = GetIniKeyInt("TestItem", "OpenShort_Threshold2", g_szConfigPath);
		LD_MSG("%s,%d,Open_RX_Delta_Threshold = %d\n", __func__, __LINE__, ST.Open_RX_Delta_Threshold);
		ST.Open_RX_Continue_Fail_Tolerance = GetIniKeyInt("TestItem", "OpenShort_Threshold2_con", g_szConfigPath);
		LD_MSG("%s,%d,OpenShort_Threshold2_con(Open_RX_Continue_Fail_Tolerance) = %d\n", __func__, __LINE__, ST.Open_RX_Continue_Fail_Tolerance);
		ST.Open_TX_Continue_Fail_Tolerance = GetIniKeyInt("TestItem", "OpenShort_Threshold3_con", g_szConfigPath);
		LD_MSG("%s,%d,OpenShort_Threshold3_con(Open_TX_Continue_Fail_Tolerance) = %d\n", __func__, __LINE__, ST.Open_TX_Continue_Fail_Tolerance);
		if (ST.UseNewFlow == 1) {
			ST.Open_DCRangeMax = GetIniKeyInt("TestItem", "OpenShort_Threshold3", g_szConfigPath);
			LD_MSG("%s,%d,OpenShort_Threshold3(Open_DCRangeMax) = %d\n", __func__, __LINE__, ST.Open_DCRangeMax);
			ST.Open_TX_Delta_Threshold = GetIniKeyInt("TestItem", "OpenShort_Threshold4", g_szConfigPath);
			LD_MSG("%s,%d,OpenShort_Threshold4(Open_TX_Delta_Threshold) = %d\n", __func__, __LINE__, ST.Open_TX_Delta_Threshold);
		} else {
			ST.Open_TX_Delta_Threshold = GetIniKeyInt("TestItem", "OpenShort_Threshold3", g_szConfigPath);
			LD_MSG("%s,%d,OpenShort_Threshold3(Open_TX_Delta_Threshold) = %d\n", __func__, __LINE__, ST.Open_TX_Delta_Threshold);
		}
		ST.Open_DCRangeMin = GetIniKeyInt("TestItem", "OpenShort_Threshold5", g_szConfigPath);

		//--------------------------profile Self test --------------------------//
		ST.SelfCapTest_test = GetIniKeyInt("TestItem", "SelfCapTest", g_szConfigPath);
		if (ST.SelfCapTest_test == 1)
			inFunctions += SELF_TEST;
		ST.Self_Maximum = GetIniKeyInt("TestItem", "SelfCapTest_Maximum", g_szConfigPath);
		ST.Self_Minimum = GetIniKeyInt("TestItem", "SelfCapTest_Minimum", g_szConfigPath);
		ST.Self_P2P = GetIniKeyInt("TestItem", "SelfCapTest_P2P", g_szConfigPath);
		ST.Self_P2P_Edge = GetIniKeyInt("TestItem", "SelfCapTest_P2P_Edge", g_szConfigPath);
		ST.Self_Frame_Count = GetIniKeyInt("TestItem", "SelfCapTest_Frame", g_szConfigPath);

		//--------------------------profile Allnode test --------------------------//
		ST.AllNode_test = GetIniKeyInt("TestItem", "AllNode", g_szConfigPath);
		if (ST.AllNode_test == 1)
			inFunctions += ALL_NODE_TEST;
		ST.AllNode_Delta_Threshold = GetIniKeyInt("TestItem", "AllNode_Window", g_szConfigPath);
		ST.AllNode_Panel_Tolerance = GetIniKeyInt("TestItem", "AllNode_Window2", g_szConfigPath);
		ST.AllNode_TX_Tolerance = GetIniKeyInt("TestItem", "AllNode_Window3", g_szConfigPath);
		ST.AllNode_Maximum = GetIniKeyInt("TestItem", "AllNode_XMax", g_szConfigPath);
		ST.AllNode_Minimum = GetIniKeyInt("TestItem", "AllNode_XMin", g_szConfigPath);
		if (ST.dat_format == true) {
			//--------------------------profile Uniformity test --------------------------//
			ST.Uniformity_test = GetIniKeyInt("TestItem", "UniformityTest", g_szConfigPath);
			if (ST.Uniformity_test == 1)
				inFunctions += UNIFORMITY_TEST;
			ST.Uniformity.uiBD_Top_Ratio = GetIniKeyInt("TestItem","BD_Top_Ratio(%)",g_szConfigPath);
			ST.Uniformity.uiBD_Bottom_Ratio = GetIniKeyInt("TestItem","BD_Bottom_Ratio(%)",g_szConfigPath);
			ST.Uniformity.uiBD_L_Ratio = GetIniKeyInt("TestItem","BD_L_Ratio(%)",g_szConfigPath);
			ST.Uniformity.uiBD_R_Ratio = GetIniKeyInt("TestItem","BD_R_Ratio(%)",g_szConfigPath);
			ST.Uniformity.uiVA_Ratio_X_diff = GetIniKeyInt("TestItem","VA_Ratio_X_diff(%)",g_szConfigPath);
			ST.Uniformity.uiVA_Ratio_Y_diff = GetIniKeyInt("TestItem","VA_Ratio_Y_diff(%)",g_szConfigPath);
			ST.Uniformity.uiBD_VA_L_Ratio_Max = GetIniKeyInt("TestItem","BD_VA_L_Ratio_Max(%)",g_szConfigPath);
			ST.Uniformity.uiBD_VA_L_Ratio_Min = GetIniKeyInt("TestItem","BD_VA_L_Ratio_Min(%)",g_szConfigPath);
			ST.Uniformity.uiBD_VA_R_Ratio_Max = GetIniKeyInt("TestItem","BD_VA_R_Ratio_Max(%)",g_szConfigPath);
			ST.Uniformity.uiBD_VA_R_Ratio_Min = GetIniKeyInt("TestItem","BD_VA_R_Ratio_Min(%)",g_szConfigPath);
			ST.Uniformity.uiBD_VA_Top_Ratio_Max = GetIniKeyInt("TestItem","BD_VA_Top_Ratio_Max(%)",g_szConfigPath);
			ST.Uniformity.uiBD_VA_Top_Ratio_Min = GetIniKeyInt("TestItem","BD_VA_Top_Ratio_Min(%)",g_szConfigPath);
			ST.Uniformity.uiBD_VA_Bottom_Ratio_Max = GetIniKeyInt("TestItem","BD_VA_Bottom_Ratio_Max(%)",g_szConfigPath);
			ST.Uniformity.uiBD_VA_Bottom_Ratio_Min = GetIniKeyInt("TestItem","BD_VA_Bottom_Ratio_Min(%)",g_szConfigPath);
			ST.Uniformity.uiPanelLeftTopULimit = GetIniKeyInt("TestItem","PanelLeftTopULimit(%)",g_szConfigPath);
			ST.Uniformity.uiPanelLeftTopLLimit = GetIniKeyInt("TestItem","PanelLeftTopLLimit(%)",g_szConfigPath);
			ST.Uniformity.uiPanelLeftBottomULimit = GetIniKeyInt("TestItem","PanelLeftBottomULimit(%)",g_szConfigPath);
			ST.Uniformity.uiPanelLeftBottomLLimit = GetIniKeyInt("TestItem","PanelLeftBottomLLimit(%)",g_szConfigPath);
			ST.Uniformity.uiPanelRightTopULimit = GetIniKeyInt("TestItem","PanelRightTopULimit(%)",g_szConfigPath);
			ST.Uniformity.uiPanelRightTopLLimit = GetIniKeyInt("TestItem","PanelRightTopLLimit(%)",g_szConfigPath);
			ST.Uniformity.uiPanelRightBottomULimit = GetIniKeyInt("TestItem","PanelRightBottomULimit(%)",g_szConfigPath);
			ST.Uniformity.uiPanelRightBottomLLimit = GetIniKeyInt("TestItem","PanelRightBottomLLimit(%)",g_szConfigPath);
		}
		//--------------------------profile DAC test --------------------------//
		ST.DAC_test = GetIniKeyInt("TestItem", "DACTest", g_szConfigPath);
		if (ST.DAC_test == 1)
			inFunctions += DAC_TEST;
		ST.DAC_SC_P_Maximum = GetIniKeyInt("TestItem", "DACTest_SC_P_Max", g_szConfigPath);
		ST.DAC_SC_P_Minimum = GetIniKeyInt("TestItem", "DACTest_SC_P_Min", g_szConfigPath);
		ST.DAC_SC_N_Maximum = GetIniKeyInt("TestItem", "DACTest_SC_N_Max", g_szConfigPath);
		ST.DAC_SC_N_Minimum = GetIniKeyInt("TestItem", "DACTest_SC_N_Min", g_szConfigPath);
		ST.DAC_MC_P_Maximum = GetIniKeyInt("TestItem", "DACTest_MC_P_Max", g_szConfigPath);
		ST.DAC_MC_P_Minimum = GetIniKeyInt("TestItem", "DACTest_MC_P_Min", g_szConfigPath);
		ST.DAC_MC_N_Maximum = GetIniKeyInt("TestItem", "DACTest_MC_N_Max", g_szConfigPath);
		ST.DAC_MC_N_Minimum = GetIniKeyInt("TestItem", "DACTest_MC_N_Min", g_szConfigPath);
		if (ST.UseNewFlow == 1) {
			vfReadBenchMarkValue("YDriven",ST.BenchMark.iUniformityBenchMark,g_szConfigPath);
			vfReadBenchMarkValue("YDriven_Open",ST.BenchMark.iOpenBenchMark_0,g_szConfigPath);
			vfReadBenchMarkValue("YDriven_Open1",ST.BenchMark.iOpenBenchMark_1,g_szConfigPath);
		}
		LD_MSG("Profile Version:%d.%d.%d.%d\n",
			ST.PFVer3, ST.PFVer2, ST.PFVer1, ST.PFVer0);

		if (ST.PFVer0 != 0)
			NewVerFlag = 1;

		LD_MSG("NewVerFlag State:%d\n",NewVerFlag);
		LD_MSG("inFunctions State:%d\n",inFunctions);
	} else if (strstr((char *)IniPath, PROFILE_FORMAT_INI) != NULL) {
		ST.useINI = true;
		inFunctions = ReadST_V6();

		/*
		 * For V3, Uniformity from INI should run Allnode test
		 */
		if ((inFunctions & UNIFORMITY_TEST) == UNIFORMITY_TEST) {
			inFunctions -= UNIFORMITY_TEST;
			inFunctions += ALL_NODE_TEST;
		}
	} else {
		LD_ERR("Profile fail, %s\n", g_szConfigPath);
	}
	return inFunctions;
}

double GetVerfMapping(uint8_t *tmp) {
	if(strcmp((char *)tmp,"0.3") == 0 || strcmp((char *)tmp," 0.3") == 0) {
		LD_MSG("0.3\n");
		ST.Short.vref_s = 0xB;
		return 0.3;
	}
	else if(strcmp((char *)tmp,"0.4") == 0 || strcmp((char *)tmp," 0.4") == 0) {
		LD_MSG("0.4\n");
		ST.Short.vref_s = 0xC;
		return 0.4;
	}
	else if(strcmp((char *)tmp,"0.5") == 0 || strcmp((char *)tmp," 0.5") == 0) {
		LD_MSG("0.5\n");
		ST.Short.vref_s = 0xD;
		return 0.5;
	}
	else if(strcmp((char *)tmp,"0.6") == 0 || strcmp((char *)tmp," 0.6") == 0) {
		LD_MSG("0.6\n");
		ST.Short.vref_s = 0xE;
		return 0.6;
	}
	else if(strcmp((char *)tmp,"0.7") == 0 || strcmp((char *)tmp," 0.7") == 0) {
		LD_MSG("0.7\n");
		ST.Short.vref_s = 0xF;
		return 0.7;
	}
	else {
		LD_MSG("0xFF\n");
		ST.Short.vref_s = 0xB;
		return _FAIL;
	}
}

void print_PASS_or_FAIL(bool pass)
{
	LD_MSG("\n");
	if (pass) {
		LD_MSG("@@@@@@@@        @@           @@@@       @@@@\n");
		LD_MSG("@@@@@@@@       @@@@         @@@@@@     @@@@@@\n");
		LD_MSG("@@     @@     @@  @@       @@    @@   @@    @@\n");
		LD_MSG("@@     @@    @@    @@     @@    @@   @@    @@\n");
		LD_MSG("@@@@@@@@@   @@@@@@@@@@    @@         @@\n");
		LD_MSG("@@@@@@@    @@@@@@@@@@@@    @@@@@@     @@@@@@\n");
		LD_MSG("@@         @@        @@     @@@@@@     @@@@@@\n");
		LD_MSG("@@         @@        @@          @@         @@\n");
		LD_MSG("@@         @@        @@   @@    @@   @@    @@\n");
		LD_MSG("@@         @@        @@    @@@@@@     @@@@@@\n");
	} else {
		LD_MSG("@@@@@@@@@       @@        @@@@@@@@    @@\n");
		LD_MSG("@@@@@@@@@      @@@@       @@@@@@@@    @@\n");
		LD_MSG("@@            @@  @@         @@       @@\n");
		LD_MSG("@@           @@    @@        @@       @@\n");
		LD_MSG("@@@@@@@@@   @@@@@@@@@@       @@       @@\n");
		LD_MSG("@@@@@@@@@  @@@@@@@@@@@@      @@       @@\n");
		LD_MSG("@@         @@        @@      @@       @@\n");
		LD_MSG("@@         @@        @@      @@       @@\n");
		LD_MSG("@@         @@        @@   @@@@@@@@    @@@@@@@@@\n");
		LD_MSG("@@         @@        @@   @@@@@@@@    @@@@@@@@@\n");
	}
	LD_MSG("\n");
}

int ReadST_V6()
{
	int i;
	char buf[INI_MAX_PATH], *tmp, section_fw[INI_MAX_PATH];
	int inFunctions = 0;

	ST.profile = PROFILE_INI;
	tmp = (char *)calloc(1024, sizeof(uint8_t));
	//read ini test  Criteria
	memset(buf, 0, sizeof(buf));
	memset(g_szConfigPath, 0, sizeof(g_szConfigPath));
	//LD_MSG("=>Get sensor test criteria\n");
	strcpy(g_szConfigPath, (char *)IniPath);
	LD_MSG("Profile file path %s\n", g_szConfigPath);
	LD_MSG("ini path=%s\n",IniPath);

	//[System]
	tmp = GetIniKeyString("System", "Save_Date", g_szConfigPath);
	strcpy(ST.PFDate, tmp);
	LD_MSG("Profile Save_Date: %s\n", ST.PFDate);
	tmp = GetIniKeyString("System", "ProfileVersion", g_szConfigPath);
	sscanf(tmp, "%d.%d.%d.%d", &ST.PFVer0, &ST.PFVer1, &ST.PFVer2, &ST.PFVer3);
	LD_MSG("Profile Version:%s, %d.%d.%d.%d\n", tmp, ST.PFVer0, ST.PFVer1, ST.PFVer2, ST.PFVer3);
	ST.PFVer = (ST.PFVer0 << 24) + (ST.PFVer1 << 16) + (ST.PFVer2 << 8) + ST.PFVer3;
	LD_MSG("Profile Version:0x%x\n", ST.PFVer);
	//[Panel_Info]
	ST.x_ch = GetIniKeyInt("Panel_Info", "XChannel", g_szConfigPath);
	ST.y_ch = GetIniKeyInt("Panel_Info", "YChannel", g_szConfigPath);
	if(ST.x_ch != ptl.x_ch || ST.y_ch != ptl.y_ch) {
		LD_MSG("IC channel X_CH:%d Y_CH:%d, Profile channel X_CH:%d Y_CH:%d\n", ptl.x_ch, ptl.y_ch, ST.x_ch,  ST.y_ch);
		LD_ERR("Channel not consistent\n");
		return _FAIL;
	}

	//[Report]
	tmp = GetIniKeyString("Report", "Path", g_szConfigPath);
	if (tmp) {
		strcpy(ST.LogPath, tmp);
		if (!access(ST.LogPath, F_OK | W_OK)) {
			LD_MSG("LogPath:%s\n", ST.LogPath);
		} else if (mkdir(ST.LogPath, 0777)) {
			ST.LogPath = getcwd(NULL, 0);
			LD_ERR("[INI] mkdir LogPath failed: %d, use defaule path:%s\n",
				errno, ST.LogPath);
		}
	}

	//[FW_Upgrade]
	if(ST.PFVer < 0x1000105)
		strcpy(section_fw, "FW_Upgrade");
	else
		strcpy(section_fw, "FW_Verify");

	tmp = GetIniKeyString(section_fw, "Enable", g_szConfigPath);


	if(strcmp(tmp, "True") == 0) {
		ST.FWVersion_test = 1;
		inFunctions += FWVERTION_TEST;
		LD_MSG("FWVersion_test=%d,tmp:%s\n", ST.FWVersion_test, tmp);
		tmp = GetIniKeyString(section_fw, "FW_Ver", g_szConfigPath);
		if(ST.PFVer < PROFILE_V1_0_2_0) {
			sscanf(tmp, "V%x.%x.%x.%x", &ST.fw_ver[0], &ST.fw_ver[1], &ST.fw_ver[2], &ST.fw_ver[3]);
			LD_MSG("FW Version:%s, %x.%x.%x.%x\n", tmp,ST.fw_ver[0], ST.fw_ver[1], ST.fw_ver[2], ST.fw_ver[3]);
		}
		else {
			sscanf(tmp, "%x.%x.%x.%x.%x.%x.%x.%x", &ST.fw_ver[0], &ST.fw_ver[1], &ST.fw_ver[2], &ST.fw_ver[3]
					, &ST.fw_ver[4], &ST.fw_ver[5], &ST.fw_ver[6], &ST.fw_ver[7]);
			LD_MSG("FW Version:%s, %x.%x.%x.%x.%x.%x.%x.%x\n", tmp, ST.fw_ver[0], ST.fw_ver[1], ST.fw_ver[2]
					, ST.fw_ver[3], ST.fw_ver[4], ST.fw_ver[5], ST.fw_ver[6], ST.fw_ver[7]);
		}
		ST.block_num = GetIniKeyInt(section_fw, "Block", g_szConfigPath);
		LD_MSG("ST.block_num=%d\n", ST.block_num);
		if(ST.block_num > 0) {
			tmp = GetIniKeyString(section_fw, "Master_CRC", g_szConfigPath);
			ST.master_crc = (unsigned short *)calloc(ST.block_num, sizeof(unsigned short));
			char *Tbuf = strtok(tmp, ",");
			i = 0;
			while (Tbuf != NULL)
			{
				sscanf(Tbuf,"%x", (unsigned int *)&ST.master_crc[i]);
				LD_MSG("INI CRC[%d]:0x%x\n", i, ST.master_crc[i]);
				i++;
				Tbuf = strtok( NULL, ",");
			}
		}
		ST.slave_num = GetIniKeyInt(section_fw, "Slave_number", g_szConfigPath);
		if(ST.slave_num > 0) {
			tmp = GetIniKeyString(section_fw, "Slave_CRC", g_szConfigPath);
			ST.slave_crc = (unsigned short *)calloc(ST.slave_num, sizeof(unsigned short));
			char *Sbuf = strtok(tmp, ",");
			i = 0;
			while (Sbuf != NULL)
			{
				sscanf(Sbuf,"%x", (unsigned int *)&ST.slave_crc[i]);
				i++;
				Sbuf = strtok( NULL, ",");
			}
		}
		tmp = GetIniKeyString(section_fw, "Path", g_szConfigPath);
		ST.hexfile = (char *)calloc(1024, sizeof(char));
		strcpy(ST.hexfile,tmp);
	}
	//[Short_Test]
	tmp = GetIniKeyString("Short_Test", "Enable", g_szConfigPath);
	if(strcmp(tmp, "True") == 0) {
		ST.Short_test = ENABLE_TEST;
		inFunctions += MICROSHORT_TEST;
		LD_MSG("Short_test=%d,tmp:%s\n", ST.Short_test, tmp);
		ST.Short.Threshold = GetIniKeyInt("Short_Test", "Max_Threshold", g_szConfigPath);
		ST.Short.FrameCount = GetIniKeyInt("Short_Test", "Frame_Count", g_szConfigPath);
		ST.Short.dump1 = GetIniKeyInt("Short_Test", "Dump1", g_szConfigPath);
		ST.Short.dump2 = GetIniKeyInt("Short_Test", "Dump2", g_szConfigPath);
		ST.Short.posidleL = GetIniKeyInt("Short_Test", "Short_PostIdle_L", g_szConfigPath);
		ST.Short.posidleH = GetIniKeyInt("Short_Test", "Short_PostIdle_H", g_szConfigPath);
		ST.Short.vref_v = GetVerfMapping((uint8_t *)GetIniKeyString("Short_Test", "VrefL", g_szConfigPath));
		ST.Short.keyTx_thr = GetIniKeyInt("Short_Test", "KeyTX_Threshold", g_szConfigPath);
		ST.Short.keyRx_thr = GetIniKeyInt("Short_Test", "KeyRX_Threshold", g_szConfigPath);
	}
	LD_MSG("Short_Test=%s\n", ST.Short_test ? "True" : "False");
	//[Open_Test]
	tmp = GetIniKeyString("Open_Test", "Enable", g_szConfigPath);
	if (strcmp(tmp, "True") == 0) {
		ST.Open_test = ENABLE_TEST;
		LD_MSG("Open_test=%d,tmp:%s\n", ST.Open_test, tmp);
		inFunctions += OPEN_TEST;
		ST.Open_Threshold = GetIniKeyInt("Open_Test", "Min_Threshold", g_szConfigPath);
		ST.Open.key_thr = GetIniKeyInt("Open_Test", "Key_Threshold", g_szConfigPath);
		ST.Open_FrameCount = GetIniKeyInt("Open_Test", "Frame_Count", g_szConfigPath);
		ST.Open_TX_Aver_Diff = GetIniKeyInt("Open_Test", "TX_Average_Diff_Gap", g_szConfigPath);
		tmp = GetIniKeyString("Open_Test", "TX_Average_Diff_Gap_AvoidCorner", g_szConfigPath);
		if (tmp && strcmp(tmp, "True") == 0)
			ST.Open_TX_Aver_Diff_Gap_Corner = true;
		LD_MSG("TX_Average_Diff_Gap_AvoidCorner=%s\n", ST.Open_TX_Aver_Diff_Gap_Corner ? "True" : "False");
		ST.Open_RX_Delta_Threshold = GetIniKeyInt("Open_Test", "RX_Diff_Gap", g_szConfigPath);
		ST.Open_RX_Continue_Fail_Tolerance = GetIniKeyInt("Open_Test", "RX_Diff_Gap_Tolerance", g_szConfigPath);
		ST.Open.freq = GetIniKeyInt("Open_Test", "Frequency", g_szConfigPath);
		if(ST.Open.freq < 0)
			ST.Open.freq = 100;
		ST.Open.gain = GetIniKeyInt("Open_Test", "Gain", g_szConfigPath);
		if(ST.Open.gain < 0)
			ST.Open.gain = 0;
	}
	LD_MSG("Open_Test=%s\n", ST.Open_test ? "True" : "False");
	if(ST.PFVer >= PROFILE_V1_0_3_0) {
		ST.Open_TX_Aver_Diff = -1;
		ST.Open_RX_Delta_Threshold = -1;
	}
	//[Mirco Open Test]
	tmp = GetIniKeyString("MicroOpen_Test", "Enable", g_szConfigPath);
	if(strcmp(tmp, "True") == 0) {
		ST.MOpen.En = ENABLE_TEST;
		inFunctions += MIRCO_OPEN_TEST;
		ST.MOpen.FrameCount = GetIniKeyInt("MicroOpen_Test", "Frame_Count", g_szConfigPath);
		ST.MOpen.RxDiffThr = GetIniKeyInt("MicroOpen_Test", "RX_Delta_Threshold", g_szConfigPath);
		tmp = GetIniKeyString("MicroOpen_Test", "RX_Delta_En", g_szConfigPath);
		if(strcmp(tmp, "True") == 0) {
			ST.MOpen.RxDeltaEn = ENABLE_TEST;
			vfReadBenchMarkValue_V6("RX_Delta", g_szConfigPath, ST.MOpen.rx_diff, ptl.x_ch - 1, ptl.y_ch);
		}
		LD_MSG("RX_Delta_En=%s,%d\n", ST.MOpen.RxDeltaEn ? "True" : "False" , ST.MOpen.RxDeltaEn);
		tmp = GetIniKeyString("MicroOpen_Test", "TX_Avg_Delta_En", g_szConfigPath);
		if(strcmp(tmp, "True") == 0) {
			ST.MOpen.TxAvgEn = ENABLE_TEST;
			vfReadBenchMarkValue_V6("TX_Avg_Delta", g_szConfigPath, ST.MOpen.tx_avg, 1, ptl.y_ch - 1);
			ST.MOpen.TxAvgThr = GetIniKeyInt("MicroOpen_Test", "TX_Avg_Delta_Threshold", g_szConfigPath);
			tmp = GetIniKeyString("MicroOpen_Test", "TX_Avg_Delta_Threshold_AvoidCorner", g_szConfigPath);
			if(strcmp(tmp, "True") == 0)
				ST.MOpen.TxAvgCorner = ENABLE_TEST;
			LD_MSG("TX_Avg_Delta_Threshold_AvoidCorner=%s\n", ST.MOpen.TxAvgCorner ? "True" : "False");
		}
		LD_MSG("TX_Avg_Delta_En=%s\n", ST.MOpen.TxAvgEn ? "True" : "False");
		ST.MOpen.RxToles = GetIniKeyInt("MicroOpen_Test", "RX_Delta_Threshold_Tolerance", g_szConfigPath);
	}
	LD_MSG("MicroOpen_Test=%s\n", ST.MOpen.En ? "True" : "False");
	//[Uniformity_Test]
	tmp = GetIniKeyString("Uniformity_Test", "Enable", g_szConfigPath);
	if(strcmp(tmp, "True") == 0) {
		ST.Uniformity_test = ENABLE_TEST;
		inFunctions += UNIFORMITY_TEST;
		LD_MSG("Uniformity_test=%d,tmp:%s\n", ST.Uniformity_test, tmp);
		ST.Uniformity.FrameCount = GetIniKeyInt("Uniformity_Test", "Frame_Count", g_szConfigPath);
		if(ST.PFVer >= PROFILE_V1_0_2_0) {
			ST.Uniformity.Max_Threshold = GetIniKeyInt("Uniformity_Test", "Uniformity_RawData_Max_Threshold", g_szConfigPath);
			ST.Uniformity.Up_FailCount = GetIniKeyInt("Uniformity_Test", "Uniformity_RawData_Max_Threshold_Tolerance", g_szConfigPath);
			ST.Uniformity.Min_Threshold = GetIniKeyInt("Uniformity_Test", "Uniformity_RawData_Min_Threshold", g_szConfigPath);
			ST.Uniformity.Low_FailCount = GetIniKeyInt("Uniformity_Test", "Uniformity_RawData_Min_Threshold_Tolerance", g_szConfigPath);
			ST.Uniformity.Win1_Threshold = GetIniKeyInt("Uniformity_Test", "Uniformity_Win1_Max_Threshold", g_szConfigPath);
			ST.Uniformity.Win1_FailCount = GetIniKeyInt("Uniformity_Test", "Uniformity_Win1_Max_Threshold_Tolerance", g_szConfigPath);
			ST.Uniformity.Win2_Threshold = GetIniKeyInt("Uniformity_Test", "Uniformity_Win2_Max_Threshold", g_szConfigPath);
			ST.Uniformity.Win2_FailCount = GetIniKeyInt("Uniformity_Test", "Uniformity_Win2_Max_Threshold_Tolerance", g_szConfigPath);
			tmp = GetIniKeyString("Uniformity_Test", "Benchmark_Enable", g_szConfigPath);
			if (tmp && strcmp(tmp, "True") == 0) {
				ST.Uniformity.En_bench = ENABLE_TEST;
				vfReadBenchMarkValue_V6("Uniformity_Benchmark", g_szConfigPath, ST.Uniformity.bench, ptl.x_ch, ptl.y_ch);
			}
			tmp = GetIniKeyString("Uniformity_Test", "Uniformity_RawData_En", g_szConfigPath);
			if (tmp && strcmp(tmp, "True") == 0) {
				ST.Uniformity.En_allraw = ENABLE_TEST;
				vfReadBenchMarkValue_V6("Uniformity_RawData", g_szConfigPath, ST.Uniformity.allraw, ptl.x_ch, ptl.y_ch);
			}
			tmp = GetIniKeyString("Uniformity_Test", "Uniformity_Win1_En", g_szConfigPath);
			if (tmp && strcmp(tmp, "True") == 0) {
				ST.Uniformity.En_allwin1 = ENABLE_TEST;
				vfReadBenchMarkValue_V6("Uniformity_Win1", g_szConfigPath, ST.Uniformity.allwin1, ptl.x_ch, ptl.y_ch - 1);
			}
			tmp = GetIniKeyString("Uniformity_Test", "Uniformity_Win2_En", g_szConfigPath);
			if (tmp && strcmp(tmp, "True") == 0) {
				ST.Uniformity.En_allwin2 = ENABLE_TEST;
				vfReadBenchMarkValue_V6("Uniformity_Win2", g_szConfigPath, ST.Uniformity.allwin2, ptl.x_ch - 1, ptl.y_ch - 1);
			}
		}
		else {
			ST.Uniformity.Max_Threshold = GetIniKeyInt("Uniformity_Test", "Max_Threshold", g_szConfigPath);
			ST.Uniformity.Up_FailCount = GetIniKeyInt("Uniformity_Test", "Up_FailCount", g_szConfigPath);
			ST.Uniformity.Min_Threshold = GetIniKeyInt("Uniformity_Test", "Min_Threshold", g_szConfigPath);
			ST.Uniformity.Low_FailCount = GetIniKeyInt("Uniformity_Test", "Low_FailCount", g_szConfigPath);
			ST.Uniformity.FrameCount = GetIniKeyInt("Uniformity_Test", "Frame_Count", g_szConfigPath);
			ST.Uniformity.Win1_Threshold = GetIniKeyInt("Uniformity_Test", "Win1_Threshold", g_szConfigPath);
			ST.Uniformity.Win1_FailCount = GetIniKeyInt("Uniformity_Test", "Win1_FailCount", g_szConfigPath);
			ST.Uniformity.Win2_Threshold = GetIniKeyInt("Uniformity_Test", "Win2_Threshold", g_szConfigPath);
			ST.Uniformity.Win2_FailCount = GetIniKeyInt("Uniformity_Test", "Win2_FailCount", g_szConfigPath);
			tmp = GetIniKeyString("Uniformity_Test", "Benchmark_Enable", g_szConfigPath);
			if (tmp && strcmp(tmp, "True") == 0) {
				ST.Uniformity.En_bench = ENABLE_TEST;
				vfReadBenchMarkValue_V6("Uniformity_Benchmark", g_szConfigPath, ST.Uniformity.bench, ptl.x_ch, ptl.y_ch);
			}
			tmp = GetIniKeyString("Uniformity_Test", "AllNode_RawData_En", g_szConfigPath);
			if (tmp && strcmp(tmp, "True") == 0) {
				ST.Uniformity.En_allraw = ENABLE_TEST;
				vfReadBenchMarkValue_V6("AllNode_RawData", g_szConfigPath, ST.Uniformity.allraw, ptl.x_ch, ptl.y_ch);
			}
			tmp = GetIniKeyString("Uniformity_Test", "AllNode_Win1_En", g_szConfigPath);
			if (tmp && strcmp(tmp, "True") == 0) {
				ST.Uniformity.En_allwin1 = ENABLE_TEST;
				vfReadBenchMarkValue_V6("AllNode_Win1", g_szConfigPath, ST.Uniformity.allwin1, ptl.x_ch, ptl.y_ch - 1);
			}
			tmp = GetIniKeyString("Uniformity_Test", "AllNode_Win2_En", g_szConfigPath);
			if (tmp && strcmp(tmp, "True") == 0) {
				ST.Uniformity.En_allwin2 = ENABLE_TEST;
				vfReadBenchMarkValue_V6("AllNode_Win2", g_szConfigPath, ST.Uniformity.allwin2, ptl.x_ch - 1, ptl.y_ch - 1);
			}
		}

	}
	//free(tmp);
	return inFunctions;
}
#endif

