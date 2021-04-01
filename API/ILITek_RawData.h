
/******************** (C) COPYRIGHT 2019 ILI Technology Corp. ********************
 * File Name :   IliTek_RawData.h
 * Description   :   Header for IliTek_RawData.c file
 *
 ********************************************************************************
 *History:
 *   Version        Date           Author            Description
 *   --------------------------------------------------------------------------
 *      1.0       2019/02/15          Randy           Initial version
 *******************************************************************************/

#ifndef _ILITEK_RAWDATA_H_
#define _ILITEK_RAWDATA_H_

/* Includes of headers ------------------------------------------------------*/
/* Extern define ------------------------------------------------------------*/
#define TEST_MODE_V3_MC_RAW                 0x1
#define TEST_MODE_V3_MC_BG                  0x2
#define TEST_MODE_V3_MC_SINGNAL             0x3
#define TEST_MODE_V3_SC_RAW                 0x4
#define TEST_MODE_V3_SC_BG                  0x5
#define TEST_MODE_V3_SC_SINGNAL             0x6
#define TEST_MODE_V3_KEY_RAW                0x12
#define TEST_MODE_V3_KEY_BG                 0x13
#define TEST_MODE_V3_KEY_SINGNEL            0x14
#define TEST_MODE_V3_MC_DAC_P               0x20
#define TEST_MODE_V3_MC_DAC_N               0x21
#define TEST_MODE_V3_SC_DAC_P               0x22
#define TEST_MODE_V3_SC_DAC_N               0x23
#define TEST_MODE_V3_Y_DRIVEN               0xE6
#define TEST_MODE_V3_X_DRIVEN               0xE7
#define TEST_MODE_V3_KEY_DATA               0xE8
#define TEST_MODE_V3_ALLNODE_WITH_BK        0x0
#define TEST_MODE_V3_ALLNODE_WITHOUT_BK     0x1
//protocol v6
#define TEST_MODE_V6_MC_RAW_BK              0x1
#define TEST_MODE_V6_MC_RAW_NBK             0x2
#define TEST_MODE_V6_MC_BG_BK               0x3
#define TEST_MODE_V6_MC_SE_BK               0x4
#define TEST_MODE_V6_MC_DAC_P               0x5
#define TEST_MODE_V6_MC_DAC_N               0x6
#define TEST_MODE_V6_SC_RAW_BK              0x9
#define TEST_MODE_V6_SC_RAW_NBK             0xA
#define TEST_MODE_V6_SC_BG_BK               0xB
#define TEST_MODE_V6_SC_SE_BK               0xC
#define TEST_MODE_V6_SC_DAC_P               0xD
#define TEST_MODE_V6_SC_DAC_N               0xE
#define TEST_MODE_V6_ICON_RAW_BK            0x11
#define TEST_MODE_V6_ICON_RAW_NBK           0x12
#define TEST_MODE_V6_ICON_BG_BK             0x13
#define TEST_MODE_V6_ICON_SE_BK             0x14
#define TEST_MODE_OPEN_KEY                  0x18
#define TEST_MODE_V6_OPEN                   0x19
#define TEST_MODE_V6_SHORT_RX               0x1A
#define TEST_MODE_V6_SHORT_TX               0x1B
#define TEST_MODE_V6_SHORT_KEY              0x1C
#define TEST_MODE_V6_SET_FREQ               0x20
#define TEST_MODE_V6_KEY_MC_RAW_BK          0x21
#define TEST_MODE_V6_KEY_MC_RAW_NBK         0x22
#define TEST_MODE_V6_KEY_MC_BG_BK           0x23
#define TEST_MODE_V6_KEY_MC_SE_BK           0x24
#define TEST_MODE_V6_KEY_MC_DAC_P           0x25
#define TEST_MODE_V6_KEY_MC_DAC_N           0x26
#define TEST_MODE_V6_KEY_SC_RAW_BK          0x29
#define TEST_MODE_V6_KEY_SC_RAW_NBK         0x2A
#define TEST_MODE_V6_KEY_SC_BG_BK           0x2B
#define TEST_MODE_V6_KEY_SC_SE_BK           0x2C
#define TEST_MODE_V6_KEY_SC_DAC_P           0x2D
#define TEST_MODE_V6_KEY_SC_DAC_N           0x2E
#define RAW_DATA_TRANSGER_V6_LENGTH         1024         //ILI2130/2131/2132 has 1024 byte length limit
/* Extern typedef -----------------------------------------------------------*/
/* Extern macro -------------------------------------------------------------*/
/* Extern variables ---------------------------------------------------------*/
/* Extern function prototypes -----------------------------------------------*/
/* Extern functions ---------------------------------------------------------*/

int viRunBGData(int inFrames);
int viRunCDCData(int inFrames);
int viRunCDCType_3X(const char *type, int inFrames);
int viRunCDCType_6X(const char *type, int inFrames);
int viInitRawData_3Para_3X(unsigned char ucCMDInit, unsigned char ucCMDMode, unsigned char ucCMDCounts);
int viGetRawData_3X(unsigned char ucCMD, unsigned char unStyle, int inTotalCounts, unsigned char ucDataFormat, unsigned char ucLineLenth);
int viRunBGMinusCDCData(int inFrames);
int viRunBGMinusCDCData_3X(int inFrames);
int viInitRawData_6X(unsigned char cmd, int delay_count);
int viGetRawData_6X(unsigned int d_len);
extern int viRunCreateBenchMark_6X(int argc, char *argv[]);
extern int viRunCDCType(char *argv[]);
extern int viInitRawData_3X(unsigned char ucCMDInit, unsigned char ucCMDMode);
extern int viGetCDCData_3X(unsigned char type, unsigned int len, unsigned char offset, unsigned char driven, unsigned char row);
extern int viGetCDCData_6X(unsigned char type, unsigned int len);
int viCreateCDCReportFile(const char *type);
int viWriteCDCReport(int count, int report[][300], int max, int min, int report_key[][50]);
#endif

