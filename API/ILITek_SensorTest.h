
/******************** (C) COPYRIGHT 2019 ILI Technology Corp. ********************
 * File Name :   IliTek_SensorTest.h
 * Description   :   Header for IliTek_SensorTest.c file
 *
 ********************************************************************************
 *History:
 *   Version        Date           Author            Description
 *   --------------------------------------------------------------------------
 *      1.0       2019/02/15          Randy           Initial version
 *******************************************************************************/

#ifndef _ILITEK_SENSORTEST_H_
#define _ILITEK_SENSORTEST_H_

/* Includes of headers ------------------------------------------------------*/
#include "../ILITek_Device.h"
#include <stdbool.h>
/* Extern define ------------------------------------------------------------*/
#define INI_MAX_PATH    260
#define DIVIDE_10M      110
#define ENABLE_TEST     1
#define DISABLE_TEST    0
#define NODE_PASS       false
#define NODE_FAIL       true
#define NODE_STATUS         0
#define NODE_MAX_STATUS     1
#define NODE_MIN_STATUS     2
#define PROFILE_FORMAT_DAT  ".dat"
#define PROFILE_FORMAT_INI  ".ini"
#define PROFILE_V1_0_2_0    0x1000200
#define PROFILE_V1_0_3_0    0x1000300
/* Extern typedef -----------------------------------------------------------*/
typedef struct _SensorTest_Node_
{
	int data;
	bool status;
	bool min_st;    //min status
	bool max_st;    //max status
}SensorTest_Node;

typedef struct _SensorTest_Short_
{
	int Threshold;
	int FrameCount;
	int8_t dump1;
	int8_t dump2;
	double vref_v;      //vref value
	int8_t vref_s;     //verf set to ic command
	int8_t posidleL;
	int8_t posidleH;
	int keyTx_thr;
	int keyRx_thr;
	SensorTest_Node *key_daltc;
}SensorTest_Short;

typedef struct _SensorTest_BenBenchmark_Ini_
{
	int data;
	int max;
	int min;
	int type;
}SensorTest_BenBenchmark_Ini;

typedef struct _SensorTest_BenBenchmark_Node_
{
	SensorTest_BenBenchmark_Ini ini;
	SensorTest_Node raw;
}SensorTest_BenBenchmark_Node;

typedef struct _SensorTest_Open_
{
	SensorTest_BenBenchmark_Node *tx_avg;
	SensorTest_BenBenchmark_Node **rx_diff;
	int freq;
	int gain;
	int key_thr;
	SensorTest_Node *key_daltc;
} SensorTest_Open;
typedef struct _SensorTest_MircoOpen_
{
	int En;                                             //enable
	int FrameCount;
	int RxDeltaEn;                                      //Rx Delta enable
	int TxAvgEn;                                        //TX Avg Delta Enable
	SensorTest_BenBenchmark_Node **tx_avg;
	SensorTest_BenBenchmark_Node **rx_diff;
	int TxAvgThr;                                       //TX_Avg_Delta_Threshold
	int TxAvgCorner;                                    //TX Avg Delta Threshold AvoidCorner
	int RxToles;                                        //RX_Delta_Threshold_Tolerance
	int RxDiffThr;                                      //RX_Delta_Threshold
}SensorTest_MircoOpen;

typedef struct _SensorTest_Unifromity_
{
	int uiBD_Top_Ratio;
	int uiBD_Bottom_Ratio;
	int uiBD_L_Ratio;
	int uiBD_R_Ratio;
	int uiVA_Ratio_X_diff;
	int uiVA_Ratio_Y_diff;
	int uiBD_VA_L_Ratio_Max;
	int uiBD_VA_L_Ratio_Min;
	int uiBD_VA_R_Ratio_Max;
	int uiBD_VA_R_Ratio_Min;
	int uiBD_VA_Top_Ratio_Max;
	int uiBD_VA_Top_Ratio_Min;
	int uiBD_VA_Bottom_Ratio_Max;
	int uiBD_VA_Bottom_Ratio_Min;
	int uiPanelLeftTopULimit;
	int uiPanelLeftTopLLimit;
	int uiPanelLeftBottomULimit;
	int uiPanelLeftBottomLLimit;
	int uiPanelRightTopULimit;
	int uiPanelRightTopLLimit;
	int uiPanelRightBottomULimit;
	int uiPanelRightBottomLLimit;

	int Max_Threshold;
	int Up_FailCount;
	int Min_Threshold;
	int Low_FailCount;
	int Win2_Threshold;
	int Win2_FailCount;
	int Win1_Threshold;
	int Win1_FailCount;
	int FrameCount;
	int En_bench;
	int En_allraw;
	int En_allwin1;
	int En_allwin2;
	SensorTest_BenBenchmark_Node **bench;
	SensorTest_BenBenchmark_Node **allraw;
	SensorTest_BenBenchmark_Node **allwin1;
	SensorTest_BenBenchmark_Node **allwin2;
} SensorTest_Unifromity;


typedef struct _SensorTest_BenchMark_
{
	int iOpenBenchMark_0[_MaxChanelNum_][_MaxChanelNum_];
	int iOpenBenchMark_1[_MaxChanelNum_][_MaxChanelNum_];
	int iUniformityBenchMark[_MaxChanelNum_][_MaxChanelNum_];
} SensorTest_BenchMark;


typedef struct _SensorTest_Criteria_
{
	// V3
	char *LogPath;
	int UseNewFlow;
	int Open_Threshold;
	int Open_RX_Delta_Threshold;
	int Open_TX_Delta_Threshold;
	int Open_RX_Continue_Fail_Tolerance;
	int Open_TX_Continue_Fail_Tolerance;
	int Open_DCRangeMax;
	int Open_DCRangeMin;
	int Open_FrameCount;
	int Open_TX_Aver_Diff;
	bool Open_TX_Aver_Diff_Gap_Corner;

	int Self_Maximum;
	int Self_Minimum;
	int Self_P2P;
	int Self_P2P_Edge;
	int Self_Frame_Count;

	int DAC_SC_P_Maximum;
	int DAC_SC_P_Minimum;
	int DAC_SC_N_Maximum;
	int DAC_SC_N_Minimum;
	int DAC_MC_P_Maximum;
	int DAC_MC_P_Minimum;
	int DAC_MC_N_Maximum;
	int DAC_MC_N_Minimum;

	int AllNode_Maximum;
	int AllNode_Minimum;
	int AllNode_Delta_Threshold;
	int AllNode_Panel_Tolerance;
	int AllNode_TX_Tolerance;

	SensorTest_Unifromity Uniformity;
	SensorTest_BenchMark BenchMark;
	SensorTest_Short Short;
	SensorTest_MircoOpen MOpen;
	//Profile Version
	int PFVer0;
	int PFVer1;
	int PFVer2;
	int PFVer3;
	int PFVer;

	int OffsetValue;
	bool dat_format;    //true:section [TestItem]
	int CreateGolden;

	//daltc array
	short int **open_20V_daltc;
	short int **open_6V_daltc;
	short int **open_20_6V_daltc;
	short int **open_Tx_diff;
	SensorTest_Node **open_Rx_diff;

	short int **open_daltc;
	short int **open_Tx_daltc;
	short int **open_Rx_daltc;

	short int **all_daltc;
	short int **all_w1_data;
	short int **all_w2_data;

	short int *dac_sc_p;
	short int *dac_sc_n;
	short int **dac_mc_p;
	short int **dac_mc_n;

	short int **uiYDC_Range;
	short int **short_daltc;

	short int **unifor_daltc;
	short int *self_xdaltc;
	short int *self_ydaltc;

	unsigned int fw_ver[8];
	//test Item
	int FWVersion_test;
	int OpenShort_test;
	int SelfCapTest_test;
	int AllNode_test;
	int Uniformity_test;
	int DAC_test;
	int Short_test;
	int Open_test;
	int BenchMark_test;     //1:BenchMark_test 0:Threshold

	//for v6
	SensorTest_Node **v6_short_daltc;
	SensorTest_Node **v6_open_daltc;
	SensorTest_Node **v6_unifor_daltc;
	SensorTest_Node **v6_unifor_win1;
	SensorTest_Node **v6_unifor_win2;
	SensorTest_Node *Tx_Avdiff_daltc;   //V3/V6 share
	SensorTest_Open Open;
	int block_num;
	int slave_num;
	unsigned short *master_crc;
	unsigned short *slave_crc;
	char *hexfile;
	unsigned int x_ch;
	unsigned int y_ch;
} SensorTest_Criteria;

/* Extern macro -------------------------------------------------------------*/
/* Extern variables ---------------------------------------------------------*/
extern unsigned char IniPath[256];
extern SensorTest_Criteria ST;
extern void InitialSensorTestV6Parameter();
extern void InitialSensorTestV3Parameter();
/* Extern function prototypes -----------------------------------------------*/
/* Extern functions ---------------------------------------------------------*/

int viRunSensorTest(int inFunctions);
int viInitRawData_3X(unsigned char ucCMDInit, unsigned char ucCMDMode);
char *GetIniKeyString(const char *title, const char *key, char *filename);
int init_sentest_array();
int ReadST_V6();
int ReadST_V3();
int viExitTestMode();
int ReadST();
int viRunSensorTest_V6(int inFunctions);
int viRunSensorTest_V3(int inFunctions);
uint32_t GetICBlockCrcNum(uint32_t block, uint32_t type);
int viRunFWVerTest_6X();
int viRunFWVerTest_3X();
uint32_t SetOpenInfo(uint8_t frep_L,uint8_t frep_H, uint8_t gain);
int viRunMircoOpenTest_6X();
int viRunMircoOpenTest_3X();
int viRunOpenTest_6X();
#endif

