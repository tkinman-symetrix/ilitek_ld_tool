/******************** (C) COPYRIGHT 2019 ILI Technology Corp. ********************
 * File Name :   IliTek_Main.c
 * Description   :   Main function
 *
 ********************************************************************************
 *History:
 *   Version        Date           Author            Description
 *   --------------------------------------------------------------------------
 *      1.0       2019/02/15          Randy           Initial version
 *******************************************************************************/

#ifndef _ILITEK_MAIN_C_
#define _ILITEK_MAIN_C_

/* Includes of headers ------------------------------------------------------*/
#include "ILITek_Protocol.h"
#include "ILITek_CMDDefine.h"
#include "ILITek_Device.h"
#include "ILITek_DebugTool_3X.h"
#include "ILITek_Main.h"
#include "API/ILITek_Frequency.h"
#include "API/ILITek_RawData.h"
#include "API/ILITek_SensorTest.h"
#include "API/ILITek_Upgrade.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>
//struct timeval tv;
//struct timezone tz;
//__time_t basetime;
/* Private define ------------------------------------------------------------*/
#define FUNC_CMD_LEN 	20
#define FUNC_STR_LEN 	100
#define FUNC_NUM 		20 // size of Function
#define INTERFACE_NUM	2
/* Private macro ------------------------------------------------------------*/
#define LF	0x0A
#define	CR	0x0D
#define CHK_DELAY(x) ((*(x)=='D') && (*(x+1)=='e') && (*(x+2)=='l') && (*(x+3)=='a') && (*(x+4)=='y'))
#define CHK_I2C(x) ((*(x)=='I') && (*(x+1)=='2') && (*(x+2)=='C'))
#define CHK_USB(x) ((*(x)=='U') && (*(x+1)=='S') && (*(x+2)=='B'))
#define MAX_SCRIPT_CMD_SIZE	512


#define DEBUG_DESC		"Show ILITEK debug use"
//daemon function format	Interface, Protocol, Device, Addr, Ctrl_para1, Ctrl_para2, Ctrl_para3
#define DEBUG_USB	{ "USB", "V3/V6", "null", "null", "Debug Para", "", "" }
#define	DEBUG_I2C	{ "I2C", "V3/V6", "/dev/ilitek_ctrl", "41", "Debug Para", "", "" }

#define PANEL_INFOR_DESC	"Show the sensor panel information"
//daemon function format	Interface, Protocol, Device, Addr, Ctrl_para1, Ctrl_para2, Ctrl_para3
#define	PANEL_INFOR_USB	{ "USB", "V3/V6", "null", "null", "", "", "" }
#define	PANEL_INFOR_I2C	{ "I2C", "V3/V6", "/dev/ilitek_ctrl", "41", "", "", "" }

#define RAW_DATA_DESC		"Get the sensor raw data"
//daemon function format	Interface, Protocol, Device, Addr, Ctrl_para1, Ctrl_para2, Ctrl_para3
#define	RAW_DATA_USB	{ "USB", "V3/V6", "null", "null", "Frames", "", "" }
#define	RAW_DATA_I2C	{ "I2C", "V3/V6", "/dev/ilitek_ctrl", "41", "Frames", "", "" }

#define BG_RAW_DATA_DESC	"Get the sensor delta C data (BG - Raw)"
//daemon function format	Interface, Protocol, Device, Addr, Ctrl_para1, Ctrl_para2, Ctrl_para3
#define	BG_RAW_DATA_USB	{ "USB", "V3/V6", "null", "null", "Frames", "", "" }
#define	BG_RAW_DATA_I2C	{ "I2C", "V3/V6", "/dev/ilitek_ctrl", "41", "Frames", "", "" }

#define BG_DATA_DESC		"Get thhe sensor background data"
//daemon function format	Interface, Protocol, Device, Addr, Ctrl_para1, Ctrl_para2, Ctrl_para3
#define	BG_DATA_USB	{ "USB", "V3/V6", "null", "null", "Frames", "", "" }
#define	BG_DATA_I2C	{ "I2C", "V3/V6", "/dev/ilitek_ctrl", "41", "Frames", "", "" }

#define SENSOR_TEST_DESC	"Run MP sensor test"
//daemon function format	Interface, Protocol, Device, Addr, Ctrl_para1, Ctrl_para2, Ctrl_para3
#define	SENSOR_TEST_USB	{ "USB", "V3/V6", "null", "null", "Functions", "[Profile path]", "" }
#define	SENSOR_TEST_I2C	{ "I2C", "V3/V6", "/dev/ilitek_ctrl", "41", "Functions", "[Profile path]", "" }

#define FREQ_DESC		"Get different frequency data"
//daemon function format	Interface, Protocol, Device, Addr, Ctrl_para1, Ctrl_para2, Ctrl_para3
#define	FREQ_USB	{ "USB", "V3/V6", "null", "null", "Start Value", "End Value", "Step" }
#define	FREQ_I2C	{ "I2C", "V3/V6", "/dev/ilitek_ctrl", "41", "Start Value", "End Value", "Step" }

#define FW_UPDATE_DESC		"Run FW update"
//daemon function format	Interface, Protocol, Device, Addr, Ctrl_para1, Ctrl_para2, Ctrl_para3
#define	FW_UPDATE_USB	{ "USB", "V3/V6", "null", "null", "Hex Path", "[Version]", "" }
#define	FW_UPDATE_I2C	{ "I2C",  "V3/V6", "/dev/ilitek_ctrl", "41", "Hex Path", "[Version]", "" }

#define CONSOLE_DESC		"Run ILITEK CMD"
//daemon function format	Interface, Write len, Read len, Write Data
#define	CONSOLE_USB	{ "USB", "Write len", "Read len", "Write Data", "", "", "" }
#define	CONSOLE_I2C	{ "I2C", "Write len", "Read len", "Write Data", "", "", "" }

#define SCRIPT_DESC		"Run script CMD"
//daemon function format	Interface, Protocol, Device, Addr, Ctrl_para1, Ctrl_para2, Ctrl_para3
#define	SCRIPT_USB	{ "USB", "null", "null", "null", "[Script path]", "", "" }
#define	SCRIPT_I2C	{ "I2C", "V3/V6", "/dev/ilitek_ctrl", "41", "[Script path]", "", "" }

#define CTRL_MODE_DESC		"Control FW mode"
//daemon function forma		Interface, Protocol, Device, Addr, Ctrl_para1, Ctrl_para2, Ctrl_para3
#define	CTRL_MODE_USB	{ "USB", "V3/V6", "null", "null", "set mode", "", "" }
#define	CTRL_MODE_I2C	{ "I2C", "V3/V6", "/dev/ilitek_ctrl", "41", "set mode", "", "" }

#define CDC_DESC		"Run CDC"
//daemon function format	Interface, Protocol, Device, Addr, Ctrl_para1, Ctrl_para2, Ctrl_para3
#define	CTRL_CDC_USB	{ "USB", "V3/V6", "null", "null", "set CDC type", "Frames", "" }
#define	CTRL_CDC_I2C	{ "I2C", "V3/V6", "/dev/ilitek_ctrl", "41", "set CDC type", "Frames", "" }

#define EXC_DESC		"Run monitor extension (USB only)"
//daemon function format	Interface, Protocol, Device, Addr, Ctrl_para1, Ctrl_para2, Ctrl_para3
#define	EXC_USB		{ "USB", "null","null", "null", "[Script path]","","" }
#define EXC_I2C		{ "I2C","Write len","Read len","Data","","","" }

#define COU_DESC		"Run monitor copy (USB only)"
//daemon function format	Interface, Protocol, Device, Addr, Ctrl_para1, Ctrl_para2, Ctrl_para3
#define	COU_USB		{ "USB", "V3", "null", "null", "", "", "" }
#define	COU_I2C		{ "I2C", "V3", "/dev/ilitek_ctrl", "41", "", "", "" }

#define FCU_DESC		"Show FW status (USB only)"
//daemon function format	Interface, Protocol, Device, Addr, Ctrl_para1, Ctrl_para2, Ctrl_para3
#define	FCU_USB		{ "USB", "V3", "null", "null", "", "", "" }
#define	FCU_I2C		{ "I2C", "V3", "/dev/ilitek_ctrl", "41", "", "", "" }

#define SRU_DESC		"Run FW soft reset (USB only)"
//daemon function format	Interface, Protocol, Device, Addr, Ctrl_para1, Ctrl_para2, Ctrl_para3
#define	SRU_USB		{ "USB", "V3", "null", "null", "", "", "" }
#define	SRU_I2C		{ "I2C", "V3", "/dev/ilitek_ctrl", "41", "", "", "" }

#define STU_DESC		"Run FW test mode (USB only)"
//daemon function format	Interface, Protocol, Device, Addr, Ctrl_para1, Ctrl_para2, Ctrl_para3
#define	STU_USB		{ "USB", "V3", "null", "null", "", "", "" }
#define	STU_I2C		{ "I2C", "V3", "/dev/ilitek_ctrl", "41", "", "", "" }

#define REMOTE_DESC		"Show Remote use"

#define READFLASH_DESC		"Show Flash data"
//daemon function format	connect IP

/* Private function prototypes -----------------------------------------------*/
int Func_Debug(int argc, char *argv[]);
int Func_PanelInfo(int argc, char *argv[]);
int Func_RawData(int argc, char *argv[]);
int Func_BGRawData(int argc, char *argv[]);
int Func_BGData(int argc, char *argv[]);
int Func_SensorTest(int argc, char *argv[]);
int Func_Frequency(int argc, char *argv[]);
int Func_FWUpgrade(int argc, char *argv[]);
int Func_Console(int argc, char *argv[]);
int Func_Script(int argc, char *argv[]);
int Func_Remote(int argc, char *argv[]);
int Func_ReadFlash(int argc, char *argv[]);
int Func_CtrlMode(int argc, char *argv[]);
int Func_CDC(int argc, char *argv[]);
int Func_Exu(int argc, char *argv[]);
int Func_Cou(int argc, char *argv[]);
int Func_Fcu(int argc, char *argv[]);
int Func_Sru(int argc, char *argv[]);
int Func_Stu(int argc, char *argv[]);

/* Private typedef -----------------------------------------------------------*/

typedef int Function_t(int argc, char *argv[]);

typedef struct
{
	char Interface[FUNC_CMD_LEN];
	char Protocol[FUNC_CMD_LEN];
	char Device[FUNC_CMD_LEN];
	char Addr[FUNC_CMD_LEN];
	char Ctrl_para1[FUNC_CMD_LEN];
	char Ctrl_para2[FUNC_CMD_LEN];
	char Ctrl_para3[FUNC_CMD_LEN];
}S_PARA;

typedef struct
{
	char FuncStrs[FUNC_CMD_LEN];
	Function_t *pFuncPoint;
	char FuncDesc[FUNC_STR_LEN];
	S_PARA Para[INTERFACE_NUM];
} S_FUNC_MAP;

S_FUNC_MAP au8FuncStrs[] =
{
	{"Debug",		    Func_Debug, 		DEBUG_DESC,				{DEBUG_USB, DEBUG_I2C},},
	{"PanelInfor",	    Func_PanelInfo, 	PANEL_INFOR_DESC,		{PANEL_INFOR_USB, PANEL_INFOR_I2C},},
	{"RawData", 	    Func_RawData,		RAW_DATA_DESC,			{RAW_DATA_USB, RAW_DATA_I2C},},
	{"BG-RawData",	    Func_BGRawData, 	BG_RAW_DATA_DESC,		{BG_RAW_DATA_USB, BG_RAW_DATA_I2C},},
	{"BGData",		    Func_BGData,		BG_DATA_DESC,			{BG_DATA_USB, BG_DATA_I2C},},
	{"SensorTest",	    Func_SensorTest,	SENSOR_TEST_DESC,		{SENSOR_TEST_USB, SENSOR_TEST_I2C},},
	{"Frequency",	    Func_Frequency, 	FREQ_DESC,				{FREQ_USB, FREQ_I2C},},
	{"FWUpgrade",	    Func_FWUpgrade, 	FW_UPDATE_DESC,			{FW_UPDATE_USB, FW_UPDATE_I2C},},
	{"Console", 	    Func_Console,		CONSOLE_DESC,			{CONSOLE_USB, CONSOLE_I2C},},
	{"Script",		    Func_Script,		SCRIPT_DESC,			{SCRIPT_USB, SCRIPT_I2C},},
	{"ControlMode" ,    Func_CtrlMode,	 	CTRL_MODE_DESC,			{CTRL_MODE_USB, CTRL_MODE_I2C},},
	{"CDC" ,   		 	Func_CDC,		 	CDC_DESC,				{CTRL_MODE_USB, CTRL_MODE_I2C},},
	{"-exu",		    Func_Exu,			EXC_DESC,				{EXC_USB, EXC_I2C},},
	{"-cou",		    Func_Cou,			COU_DESC,				{COU_USB, COU_I2C},},
	{"-fcu",		    Func_Fcu,			FCU_DESC,				{FCU_USB, FCU_I2C},},
	{"-sru",		    Func_Sru,			SRU_DESC,				{SRU_USB, SRU_I2C},},
	{"-stu",		    Func_Stu,			STU_DESC,				{STU_USB, STU_I2C},},
	{"Remote", 		    Func_Remote,		REMOTE_DESC,			{}},
	{"ReadFlash", 		Func_ReadFlash,		READFLASH_DESC,			{}},
};

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
int Func_Debug(int argc, char *argv[])
{
	return _UNKNOWN;
}

int Func_PanelInfo(int argc, char *argv[])
{
	int ret = _FAIL;

	if ((inConnectStyle == _ConnectStyle_I2C_ && argc >= 6) ||
			(inConnectStyle == _ConnectStyle_USB_ && argc >= 4) ||
			inConnectStyle == _ConnectStyle_I2CHID_)
		ret = viGetPanelInfor();
	return ret;
}

int Func_RawData(int argc, char *argv[])
{
	int ret = _FAIL;

	if (argc >= 7)
		ret = viRunCDCData(atoi(argv[6]));
	return ret;
}

int Func_BGRawData(int argc, char *argv[])
{
	int ret = _FAIL;

	if (argc >= 7)
		ret = viRunBGMinusCDCData(atoi(argv[6]));
	return ret;
}

int Func_BGData(int argc, char *argv[])
{
	int ret = _FAIL;

	if (argc >= 7)
		ret = viRunBGData(atoi(argv[6]));

	return ret;
}

int Func_CDC(int argc, char *argv[])
{
	int ret = _FAIL;

	if (argc >= 7)
		ret = viRunCDCType(argv);
	return ret;
}

int Func_SensorTest(int argc, char *argv[])
{
	int ret = _FAIL;
	int inFunctions = 0;

	if (argc >= 7)
	{
		inFunctions = atoi(argv[6]);
		if (inProtocolStyle == _Protocol_V6_)
		{
			InitialSensorTestV6Parameter();
		}
		else if (inProtocolStyle == _Protocol_V3_)
		{
			InitialSensorTestV3Parameter();
		}
		if (argc == 8 && strcmp(argv[7], "null") != 0)
		{
			memset(IniPath, 0, sizeof(IniPath));
			strcpy((char *)IniPath, argv[7]);
		}
		ret = viRunSensorTest(inFunctions);
	}

	return ret;
}

int Func_Frequency(int argc, char *argv[])
{
	int ret = _FAIL;

	if (argc >= 8)
	{
		PRINTF("%s,%d,start:%d,end:%d,step:%d\n", __func__, __LINE__, atoi(argv[6]), atoi(argv[7]), atoi(argv[8]));
		ret = viRunFre(argv);
	}
	return ret;
}
extern int ilitek_fd;
int Func_FWUpgrade(int argc, char *argv[])
{
	char *Version = NULL;
	unsigned char *filename;
	int ret = _FAIL;

	filename = (unsigned char *)malloc(UPGRAD_FILE_PATH_SIZE);
	memset(filename, 0x0, UPGRAD_FILE_PATH_SIZE);
	switch_irq(0);
	if (argc >= 7)
	{
		if (argc >= 8)
		{
			Version = argv[7];
		}
		PRINTF("Hex filename:%s\n", argv[6]);
		strcat((char *)filename, argv[6]);
		PRINTF("Hex filename:%s\n", filename);
		ret = viRunFiremwareUpgrade(filename, Version);
	}
	free(filename);
	switch_irq(1);
	return ret;

}

int Func_Console(int argc, char *argv[])
{
	int ret = _FAIL;

	if (argc >= 6)
		ret = viConsoleData(argv);
	return ret;
}

int Func_Script(int argc, char *argv[])
{
	int ret = _FAIL;

	if (argc >= 6)
		ret = viScript(argv);
	return ret;
}

int Func_Remote(int argc, char *argv[])
{
	int ret = _FAIL;

	if (argc >= 7)
		ret = viRemote(argv);
	return ret;
}

int Func_ReadFlash(int argc, char *argv[])
{
	int ret = _FAIL;
	int start_addr = 0;
	int read_len = 0;

	//start_addr = atoi(argv[6]);
	sscanf(argv[6], "%x", &start_addr);
	//read_len = atoi(argv[7]);
	sscanf(argv[7], "%x", &read_len);
	PRINTF("Read Flash, Start Address:0x%x, End Address:0x%x, Read Lenght:%d\n",
			start_addr, start_addr + read_len - 1, read_len);
	if (inProtocolStyle == _Protocol_V6_) {
		PRINTF("Read Flash(V6)\n");
		if (GetFlashData_V6(start_addr, read_len, argv[8]) == _SUCCESS) {
			ret = _SUCCESS;
			PRINTF("Read Flash SUCCESS\n");
		} else {
			PRINTF("Read Flash FAIL\n");
		}
	} else if (inProtocolStyle == _Protocol_V3_) {
		PRINTF("Read Flash(V3)\n");
		if (GetFlashData_V3(start_addr, read_len, argv[8]) == _SUCCESS) {
			ret = _SUCCESS;
			PRINTF("Read Flash SUCCESS\n");
		} else {
			PRINTF("Read Flash FAIL\n");
		}
	}

	return ret;
}

int Func_CtrlMode(int argc, char *argv[])
{
	int ret = _FAIL;
	int mode = atoi(argv[6]);

	if (argc >= 6 && mode < 10 && mode >= 0)
		ret = viSwitchMode(mode);
	return ret;
}

int Func_Exu(int argc, char *argv[])
{
	return monitor_extend();
}

int Func_Cou(int argc, char *argv[])
{
	return monitor_Copy();
}

int Func_Fcu(int argc, char *argv[])
{
	return check_status();
}

int Func_Sru(int argc, char *argv[])
{
	return software_reset();

}
int Func_Stu(int argc, char *argv[])
{
	return switch_testmode((uint8_t *)argv[2], (uint8_t *)argv[3]);
}

int PrintInfor(char *argv[])
{
	unsigned char u8ID = 0;
	int ret = _FAIL;

	if (strcmp(argv[1], "-v") == 0) {
		PRINTF(TOOL_VERSION);
		ret = _SUCCESS;
	} else if (strcmp(argv[1], "-h") == 0 ||
			strcmp(argv[1], "--help") == 0 ||
			strcmp(argv[1], "-help") == 0) {
		PRINTF("%-20s %s\n", "Test Function", "Function Descriotion");
		for ( u8ID = 0; u8ID < FUNC_NUM; u8ID++ )
			PRINTF("%-20s %s\n", au8FuncStrs[u8ID].FuncStrs, au8FuncStrs[u8ID].FuncDesc);
		ret = _SUCCESS;
	} else {
		ret = _FAIL;
	}
	return ret;
}

int viGetPanelInfor_V3()
{
	int ret = _FAIL;

	if (GetProtocol() != _FAIL &&
			GetKernelVer() != _FAIL &&
			GetFWVersion() != _FAIL &&
			GetICMode() != _FAIL &&
			GetCoreVersion() != _FAIL &&
			GetFWMode() != _FAIL &&
			PanelInfor_V3() != _FAIL)
		ret = _SUCCESS;

	/* return success if it's BL mode */
	if (ICMode == 0x55)
		return _SUCCESS;
	return ret;
}

int viGetPanelInfor_V6()
{
	int ret = _FAIL;

	if (GetProtocol() != _FAIL &&
			GetKernelVer() != _FAIL &&
			GetFWVersion() != _FAIL &&
			GetICMode() != _FAIL &&
			GetCoreVersion() != _FAIL &&
			GetFWMode() != _FAIL &&
			PanelInfor_V6() != _FAIL)
		ret = _SUCCESS;

	/* return success if it's BL mode */
	if (ICMode == 0x55)
		return _SUCCESS;

	return ret;
}

int viEnterTestMode()
{
	int ret = _FAIL;

	if (inProtocolStyle == _Protocol_V3_)
		ret = EnterTestMode();
	else if (inProtocolStyle == _Protocol_V6_)
		ret = ModeCtrl_V6(ENTER_SUSPEND_MODE, ENABLE_ENGINEER);
	return ret;
}

int viExitTestMode()
{
	int ret = _FAIL;

	if (inProtocolStyle == _Protocol_V3_)
		ret = ExitTestMode();
	else if (inProtocolStyle == _Protocol_V6_)
		ret = ModeCtrl_V6(ENTER_NORMAL_MODE, DISABLE_ENGINEER);
	return ret;
}

int viGetPanelInfor()
{
	int ret = _FAIL;

	PRINTF(TOOL_VERSION);
	if (inProtocolStyle == _Protocol_V3_)
		ret = viGetPanelInfor_V3();
	else if (inProtocolStyle == _Protocol_V6_)
		ret = viGetPanelInfor_V6();
	return ret;
}

int ChangeToBootloader()
{
	int ret = _SUCCESS;
	int count;

	for (count = 0; count < 5; count++) {
		//read current op mode
		ret = GetICMode();
		if (ICMode != OP_MODE_BOOTLOADER) {
			//set op mode as bootloader if current op mode is not boorloader
			ret = SetProgramKey();
			usleep(20000);

			//send changing bootloader command
			ret = ChangeTOBL();

			if (inConnectStyle == _ConnectStyle_I2CHID_)
				usleep(1300000 + count * 100000);
			else
				usleep(210000 + count * 20000);

			if (inConnectStyle != _ConnectStyle_I2C_ &&
					inConnectStyle != _ConnectStyle_I2CHID_)
				break;
		} else {
			PRINTF("%s, current op mode is bootloader mode, can update firmware(%u/5)\n", __func__, count);
			break;
		}
	}

	//check again
	if (inConnectStyle != _ConnectStyle_I2C_ &&
			inConnectStyle != _ConnectStyle_I2CHID_) {
		CloseDevice();
		usleep(500000);
		if (inProtocolStyle == _Protocol_V6_)
			sleep(1);
		for (count = 1; count < 13; count++) {
			if (InitDevice() != _FAIL) {
				if (is_usb_hid_old_bl == 1)
					break;

				//read current op mode
				ret = GetICMode();
				if (ICMode != OP_MODE_BOOTLOADER) {
					PRINTF("%s, currently in AP mode, wait 5 sec, after change to bootloader mode, 0x%X, %u\n",
							__func__, ICMode, count);
					usleep(5000000);
					if (count == 12)
						return _FAIL;
				} else {
					PRINTF("%s, check again, currently bootloader mode, can update firmware, 0x%X, ret=%u\n",
							__func__, ICMode, ret);
					break;
				}
			} else {
				PRINTF("%s, currently in AP mode, wait 5 sec, after change to bootloader mode, %u\n",
						__func__, count);
				usleep(5000000);
				if (count == 12)
					return _FAIL;
			}
		}
	}
	return _SUCCESS;
}

int ChangeToAPMode()
{
	int ret = _FAIL;
	int count;

	for (count = 0; count < 5; count++) {
		ret = SetProgramKey();
		usleep(20000);

		//send changing bootloader command
		ret = ChangeTOAP();
		usleep(10000);

		usleep(500000 + count * 100000);
		//read current op mode
		if (inConnectStyle != _ConnectStyle_I2C_)
			break;
		ret = GetICMode();
		if (ICMode == OP_MODE_APPLICATION) {
			PRINTF("%s, current op mode is AP mode(%u/5), 0x%X, ret=%u\n", __func__, count, ICMode, ret);
			return _SUCCESS;
		}
	}

	//check again
	if (inConnectStyle != _ConnectStyle_I2C_) {
		CloseDevice();
		usleep(500000);
		for (count = 1; count < 13; count++) {
			if (InitDevice() != _FAIL) {
				//read current op mode
				ret = GetICMode();
				if (ICMode == OP_MODE_APPLICATION) {
					PRINTF("%s, upgrade firmware finish\n", __func__);
					return _SUCCESS;
				} else {
					PRINTF("%s, currently in BL mode, wait 5 sec, after change to AP mode, %u\n",
							__func__, count);
					usleep(5000000);

					if (count == 12) {
						PRINTF("%s, upgrade firmware failed\n", __func__);
						return _FAIL;
					}
				}
			} else {
				PRINTF("%s, currently in BL mode, wait 5 sec, after change to AP mode, %u\n",
						__func__, count);
				usleep(5000000);

				if (count == 12) {
					PRINTF("%s, upgrade firmware failed\n", __func__);
					return _FAIL;
				}
			}
		}
	}
	return _FAIL;
}

unsigned int get_file_size(char *filename)
{
	unsigned int size;
	FILE *file = fopen(filename, "r");

	fseek(file, 0, SEEK_END);
	size = ftell(file);
	fclose(file);
	return size;
}

int DearlWithFunctions(int argc, char *argv[])
{
	unsigned char u8ID = 0;
	int ret = _FAIL;

	PRINTF("Para:%s\n", argv[2]);

	for ( u8ID = 0; u8ID < FUNC_NUM; u8ID++ ) {
		if ( strcmp(argv[1], au8FuncStrs[u8ID].FuncStrs) == 0 ) {
			ret = au8FuncStrs[u8ID].pFuncPoint(argc, argv);
			if (ret == _FAIL)
				PRINTF("Error! %s Failed!!\n", au8FuncStrs[u8ID].FuncStrs);
			else if (ret == _UNKNOWN)
				PRINTF(" %s not yet been down!!\n", au8FuncStrs[u8ID].FuncStrs);
			else
				PRINTF("%s, Success!!\n", au8FuncStrs[u8ID].FuncStrs);
		}
	}
	return ret;
}

unsigned char chartohex(unsigned char *str)
{
	unsigned int temp;

	sscanf((char *)str, "%x", &temp);
	PRINTF("the temp is %x\n", temp);
	return temp;
}

int viConsoleData(char *argv[])
{
	int count = 0;
	int inWlen = 0, inRlen = 0;
	uint8_t Wbuff[64] = {0}, Rbuff[64] = {0};
	int offset = 0;

	if (inConnectStyle == _ConnectStyle_I2CHID_)
		offset = 1;
	inWlen = atoi(argv[3 + offset]);
	inRlen = atoi(argv[4 + offset]);


	for (count = 0; count < inWlen; count++)
		Wbuff[count] = (unsigned char)chartohex((unsigned char *)argv[5 + count + offset]);

	if (TransferData(Wbuff, inWlen, Rbuff, inRlen, 1000) < 0)
		return _FAIL;

	PRINTF("%s, Return data: ", __func__);
	for (count = 0; count < inRlen; count++) {
		tempbuff[count] = Rbuff[count];
		PRINTF("%.2X.", tempbuff[count]);
	}
	return _SUCCESS;
}

int viScript(char *argv[])
{
	FILE *fp;
	unsigned char i = 0;
	unsigned int u16_delay_time = 1;
	unsigned char u8_chk_buf[MAX_SCRIPT_CMD_SIZE];
	unsigned char u8_chk_cnt = 0;
	int inWlen = 0, inRlen = 0;

	PRINTF("Script filename:%s\n", argv[6]);

	fp = fopen(argv[6], "r");
	if (fp == NULL) {
		PRINTF("%s, cannot open %s file\n", __func__, argv[6]);
		return _FAIL;
	}

	while (!feof(fp)) {
		if (fgets((char *)u8_chk_buf, MAX_SCRIPT_CMD_SIZE, fp)!=NULL) {
			if (CHK_I2C(u8_chk_buf)) {
				sscanf((char *)u8_chk_buf, "I2C %d %d", &inWlen, &inRlen);
				/* Parser write data */
				for (i=0; i<strlen((char *)u8_chk_buf); i++) {
					if (*(u8_chk_buf+i)==' ') {
						if (u8_chk_cnt++ < 2)
							continue;
						else
							sscanf((char *)(u8_chk_buf+i), "%2X", (int *)&buff[u8_chk_cnt-3]);
					}
				}
				if (TransferData(buff, inWlen, buff, inRlen, 1000) < 0)
					return _FAIL;
				PRINTF("%s, Return data: ", __func__);
				for (i=0; i<inRlen; i++) {
					tempbuff[i] = buff[i];
					PRINTF("%.2X.", tempbuff[i]);
				}
			} else if (CHK_DELAY(u8_chk_buf)) {
				u16_delay_time = 1;
				sscanf((char *)u8_chk_buf, "Delay %d", &u16_delay_time);
				usleep(u16_delay_time*1000);
			}
		}
	}

	fclose(fp);

	return _SUCCESS;
}

int help_chk(char *argv[])
{
	unsigned char u8ID = 0;
	unsigned char u8i=0;
	int ret = _FAIL;

	if (strcmp(argv[2], "-h") == 0 ||
			strcmp(argv[2], "--help") == 0 ||
			strcmp(argv[2], "-help") == 0) {
		for ( u8ID = 0; u8ID < FUNC_NUM; u8ID++ ) {
			if (strcmp(argv[1], au8FuncStrs[u8ID].FuncStrs) == 0) {
				PRINTF("%-16s|%-16s|%-16s|%-16s|%-16s|%-16s|%-16s|%-16s|\n",
						"----------------", "----------------", "----------------", "----------------",
						"----------------", "----------------", "----------------", "----------------");
				PRINTF("%-16s|%-16s|%-16s|%-16s|%-16s|%-16s|%-16s|%-16s|\n",
						"Function", "Interface", "Protocol", "Device", "I2C address",
						"Ctrl param1", "Ctrl param2", "Ctrl param3");
				PRINTF("%-16s|%-16s|%-16s|%-16s|%-16s|%-16s|%-16s|%-16s|\n",
						"----------------", "----------------", "----------------", "----------------",
						"----------------", "----------------", "----------------", "----------------");
				for (u8i=0;u8i<INTERFACE_NUM;u8i++) {
					PRINTF("%-16s|%-16s|%-16s|%-16s|%-16s|%-16s|%-16s|%-16s|\n",
							au8FuncStrs[u8ID].FuncStrs, au8FuncStrs[u8ID].Para[u8i].Interface,
							au8FuncStrs[u8ID].Para[u8i].Protocol, au8FuncStrs[u8ID].Para[u8i].Device,
							au8FuncStrs[u8ID].Para[u8i].Addr, au8FuncStrs[u8ID].Para[u8i].Ctrl_para1,
							au8FuncStrs[u8ID].Para[u8i].Ctrl_para2, au8FuncStrs[u8ID].Para[u8i].Ctrl_para3);
					PRINTF("%-16s|%-16s|%-16s|%-16s|%-16s|%-16s|%-16s|%-16s|\n",
							"----------------", "----------------", "----------------", "----------------",
							"----------------", "----------------", "----------------", "----------------");
				}
				ret = _SUCCESS;
				break;
			}
		}
	}
	return ret;
}

int readn(int fd, void *vptr, size_t n)
{
	size_t nleft = n;
	ssize_t nread = 0;
	unsigned char *ptr = (unsigned char *)vptr;

	while (nleft > 0) {
		nread = read(fd, ptr, nleft);
		if (nread == -1) {
			if (EINTR == errno)
				nread = 0;
			else
				return _FAIL;
		} else if (nread == 0) {
			break;
		}
		nleft -= nread;
		ptr += nread;
	}
	return n - nleft;
}

ssize_t recv_peek(int sockfd, void *buf, size_t len)
{
	int ret = 0;
	while (1) {
		ret = recv(sockfd, buf, len, MSG_PEEK);
		if (ret == -1 && errno == EINTR)
			continue;
		else
			break;
	}
	return ret;
}

ssize_t readline(int sockfd, void *buf, size_t maxline)
{
	int ret;
	int nread;
	char *bufp = (char *)buf;
	int nleft = maxline;
	int count = 0;
	int i;

	while (1) {
		ret = recv_peek(sockfd, bufp, nleft);
		if (ret < 0)
			return ret;
		else if (ret == 0)
			return ret;

		nread = ret;

		for (i = 0; i < nread; i++) {
			if (bufp[i] == '\n') {
				ret = readn(sockfd, bufp, i + 1);
				if (ret != i + 1)
					exit(EXIT_FAILURE);

				bufp[i+1] = '\0';

				return ret + count;
			}
		}

		if (nread > nleft)
			exit(EXIT_FAILURE);
		nleft -= nread;
		ret = readn(sockfd, bufp, nread);
		if (ret != nread)
			exit(EXIT_FAILURE);

		bufp += nread;
		count += nread;
	}
	return _FAIL;
}

int viRemote(char *argv[])
{
	int ret = _SUCCESS, count = 20;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	char buffer[1024], cmd_split_buffer[1024];
	int i = 0;
	int each_self = 0;
	int buff_data[32768];
	char str_to_server[32768];

	memset(&server_addr, 0, sizeof(server_addr));

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(argv[6]);
	server_addr.sin_port = htons(17385);

	//connect timeout 20s
	for (i = 0; i < count; i++) {
		ret = connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
		if (ret == _SUCCESS) {
			PRINTF("Connect to Server Success\n");
			inRemote_Flag = 1;
			break;
		} else {
			PRINTF("Connect to Server Fail, ret = %d\n", ret);
			if (i == count - 1)
				return _FAIL;
			sleep(1);
		}
	}

	while (1) {
		memset(buffer, '\0', sizeof(buffer));
		//read(sockfd, buffer, sizeof(buffer) - 1);
		readline(sockfd, buffer, sizeof(buffer));
		PRINTF("Len = %zu, Read data form server = %s\n", sizeof(buffer), buffer);

		memset(cmd_split_buffer, 0, sizeof(cmd_split_buffer));
		strcpy(cmd_split_buffer, buffer);

		if (strstr(buffer, ":3") != NULL) {
			//split cmd
			int cmd_split[64];
			int cmd_split_index = 0;
			char *p = cmd_split_buffer;
			char *key_point;

			while (p) {
				while ((key_point = strsep(&p,":,")) != NULL) {
					//printf("%s\n", key_point);
					cmd_split[cmd_split_index] = atoi(key_point);
					cmd_split_index++;
				}
			}

			//debug
			PRINTF("cmd_split_index = %d\n", cmd_split_index);
			for (i = 0; i < cmd_split_index; i++)
				PRINTF("cmd_split[%d] = %d\n", i, cmd_split[i]);

			//cmd set to buff
			for (i = 0; i < cmd_split_index; i++) {
				//cmd_split[0] is null, I don't know why.....
				buff[i] = cmd_split[i + 1];
			}

			if (strstr(buffer, ":3,163,1,60") != NULL) {
				printf("Cond1 = %s\n", buffer);

				//write/read data to tp
				int each_first = 1;
				int total_count = ptl.x_ch * ptl.y_ch * 2;

				if (each_self == 1) {
					total_count = (ptl.x_ch + ptl.y_ch) * 2;
					each_self = 0;
				}

				memset(buff_data, 0, sizeof(buff_data));

				int index = 0;

				do
				{
					if (inConnectStyle != _ConnectStyle_I2C_) {
						if (each_first == 1) {
							ret = TransferData(buff, cmd_split_index - 2, buff, cmd_split[4], 1000);
							each_first = 0;
						} else {
							ret = TransferData(NULL, 0, buff, cmd_split[4], 1000);
						}
					}

					//save data
					for (i = 0; i < 64; i++) {
						//printf("buff[%d] = %d\n", i, buff[i]);
						buff_data[index++] = buff[i];
					}

					//03,A3,E6,3C,indexX,indexY,xxxxxxxxxxxxx, data 58 count
					total_count = total_count - 58;
					//printf("total_count = %d\n", total_count);
				} while ((total_count) > 0);

				//write data to server
				//printf("index = %d\n", index);
				//for (i = 0; i <= index; i++)
				//{
				//	printf("buff_data[%d] = %d\n", i, buff_data[i]);
				//}

				memset(str_to_server, 0, sizeof(str_to_server));

				char temp[64];
				memset(temp, 0, sizeof(temp));

				strcat(str_to_server, ":");

				for (i = 0; i < index; i++) {
					if (i == index - 1) {
						sprintf(temp, "%d:", buff_data[i]);
						strcat(str_to_server, temp);

						printf("Write data to server = %s\n\n", str_to_server);
						ret = write(sockfd, str_to_server, sizeof(str_to_server));
						//printf("ret1 = %d\n", ret);
					} else {
						sprintf(temp, "%d,", buff_data[i]);
						strcat(str_to_server, temp);
					}
				}
			} else {
				printf("Cond2 = %s\n", buffer);

				//for do self raw data..
				if (strstr(buffer, ":3,163,4,0,243,6") != NULL)
					each_self = 1;

				//write/read data to tp
				if (inConnectStyle != _ConnectStyle_I2C_) {
					if (cmd_split[4] > 0) {
						ret = TransferData(buff, cmd_split_index - 2, buff, cmd_split[4], 1000);
					} else {
						ret = TransferData(buff, cmd_split_index - 2, NULL, 0, 1000);
						printf("\n");
					}
				}

				//for get ptl.x_ch, ptl.y_ch
				if (strstr(buffer, ":3,163,1,10,32") != NULL) {
					ptl.x_ch = buff[4 + 4];
					ptl.y_ch = buff[5 + 4];
				}

				//write data to server
				memset(str_to_server, 0, sizeof(str_to_server));
				if (cmd_split[4] > 0) {
					char temp[64];
					memset(temp, 0, sizeof(temp));

					strcat(str_to_server, ":");

					for (i = 0; i < cmd_split[4] + 4; i++) {
						if (i == cmd_split[4] + 4 - 1) {
							sprintf(temp, "%d:", buff[i]);
							strcat(str_to_server, temp);
						} else {
							sprintf(temp, "%d,", buff[i]);
							strcat(str_to_server, temp);
						}
					}

					printf("Write data to server = %s\n\n", str_to_server);
					ret = write(sockfd, str_to_server, sizeof(str_to_server));
					//printf("ret2 = %d\n", ret);
				}
			}
		} else {
			printf("%s, wrong data\n", buffer);
		}
	}

	close(sockfd);
	return _SUCCESS;
}

int viSwitchMode(int mode)
{
	int ret = _FAIL;

	PRINTF(TOOL_VERSION);
	if (inProtocolStyle == _Protocol_V3_)
		ret = viSwitchMode_V3(mode);
	else if (inProtocolStyle == _Protocol_V6_)
		ret = viSwitchMode_V6(mode);
	return ret;
}

int main(int argc, char *argv[])
{
	int ret = _FAIL;
	unsigned char u8i=0;
	// basetime = tv.tv_sec;
	// gettimeofday(&tv , &tz);
	// basetime = tv.tv_sec;

	// gettimeofday(&tv , &tz);
	// printf("main start time:%lu.%d\n", tv.tv_sec - basetime, tv.tv_usec);
	if (PrintInfor(argv) == _SUCCESS) {
		ret = _SUCCESS;
	} else if (help_chk(argv) == _SUCCESS) {
		ret = _SUCCESS;
	} else if (strcmp(argv[1], "Create") == 0) {
		PRINTF("%s,%d\n", __func__, __LINE__);
		SetConnectStyle(argv);
		PRINTF("%s,%d\n", __func__, __LINE__);
		InitDevice();
		PRINTF("%s,%d\n", __func__, __LINE__);
		if (argc >= 8) {
			PRINTF("%s,%d\n", __func__, __LINE__);
			viRunCreateBenchMark_6X(argc, argv);
			PRINTF("%s,%d\n", __func__, __LINE__);
		} else {
			PRINTF("Error! argv Error\n");
		}
		CloseDevice();
	} else if (SetConnectStyle(argv) == _SUCCESS) {
		if (InitDevice() == _SUCCESS) {
			switch_irq(0);
			if (strcmp(argv[1], "Console") != 0)
				viEnterTestMode();

			if (DearlWithFunctions(argc, argv) == _FAIL)
				ret = _FAIL;
			else
				ret = _SUCCESS;

			if (strcmp(argv[1], "Console") != 0) {
				viExitTestMode();
				if (inConnectStyle != _ConnectStyle_I2CHID_)
					software_reset();
			}

			switch_irq(1);
			CloseDevice();
		} else {
			PRINTF("InitDevice Error\n");
		}
	} else if (strcmp(argv[1], "Debug") == 0)
	{
		if (argc >= 7)
			vfRunDebug_3X(argv[2], atoi(argv[6]));
		else
			PRINTF("Error! argv Error\n");
	} else {
		PRINTF("argv Error: \"");
		for (u8i=0;u8i<argc;u8i++)
			PRINTF("%s ", argv[u8i]);
		PRINTF("\"\n");
	}

	PRINTF("main ret = %d\n", ret);
	// gettimeofday(&tv , &tz);
	// printf("main end time:%lu.%d\n", tv.tv_sec - basetime, tv.tv_usec);
	return ret;
}

#endif
