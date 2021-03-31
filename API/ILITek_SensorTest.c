
/******************** (C) COPYRIGHT 2019 ILI Technology Corp. ********************
* File Name :   IliTek_SensorTest.c
* Description   :   SensorTest function
*
********************************************************************************
*History:
*   Version        Date           Author            Description
*   --------------------------------------------------------------------------
*      1.0       2019/02/15          Randy           Initial version
*******************************************************************************/

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
int GetIniKeyInt(char *title, char *key, char *filename)
{
    int temp = 0;
    char *tmpbuffer = GetIniKeyString(title, key, filename);
    if (strcmp("", tmpbuffer) == 0)
    {
        PRINTF("Get [%s] %s no find value\n", title, key);
        return _FAIL;
    }
    else
    {
        //return atoi(tmpbuffer);
        if(strncmp(tmpbuffer, "0x", 2) == 0 || strncmp(tmpbuffer, "0X", 2) == 0)
            sscanf(tmpbuffer,"%x", &temp);
        else
            sscanf(tmpbuffer,"%d", &temp);
        PRINTF("[%s] %s=%d\n", title, key, temp);
        return temp;
    }
}
char *GetIniKeyString(char *title, char *key, char *filename)
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

    szLine = calloc(8192, sizeof(unsigned char));
    if ((fp = fopen(filename, ("rb"))) == NULL)
    {
        PRINTF("have no such file :%s\n",filename);
        return "";
    }
    fread(&(isUni), 1, 2, fp);
    if(isUni == (unsigned short)0xBBEF){
        //PRINTF("%s,%d,UTF-8 have Bom\n", __func__, __LINE__);
        //? ç‚ºbom?¯ä??‹byteï¼Œæ?ä»¥è?å¤šè?ä¸€?‹byte
        rtnval = fgetc(fp);
    }
    else if (isUni == (unsigned short)0xFFFE || isUni == (unsigned short)0xFEFF || isUni == (unsigned short)0xFEBB) {
        MultiByteEnable = true;
        //PRINTF("%s,%d, Format is Unicode\n", __func__, __LINE__);
    }
    else {
        //PRINTF("%s,%d, Format is UTF-8\n", __func__, __LINE__);
        szLine[1] = isUni >> 8;
        szLine[0] = isUni;
        i = 2;
    }
    while (!feof(fp))
    {
        rtnval = fgetc(fp);
        buteCount++;
        //?¤æ–·Unicode data?¨high/Low byte
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
            tmp = strchr(szLine, '=');
            if ((tmp != NULL) && (flag == 1))
            {
                splitResult = strtok(szLine,"=");
                //if (strstr(szLine, key) != NULL)
                if (splitResult != NULL && strstr(splitResult, key) != NULL)
                {
                    //?ï¿½ï¿½???ï¿½æ–¤??
                    if ('#' == szLine[0])
                    {
                    }
                    else if ('/' == szLine[0] && '/' == szLine[1])
                    {
                    }
                    else
                    {
                        //?ï¿½ï¿½??ï¿½key????ï¿½çµ¨
                        strcpy(tmpstr, tmp + 1);
                        free(szLine);
                        fclose(fp);
                        //PRINTF(tmpstr);
                        //PRINTF("%c",tmpstr[0]);

                        //     PRINTF("\n");
                        return tmpstr;
                    }
                }
            }
            else
            {
                strcpy(tmpstr, "[");
                strcat(tmpstr, title);
                strcat(tmpstr, "]");
                if (strncmp(tmpstr, szLine, strlen(tmpstr)) == 0)
                {
                    //?ï¿½æ–¤?ï¿½ï¿½?çµ«itle
                    flag = 1;
                }
            }
        }
    }
    free(szLine);
    fclose(fp);
    return "";
}

void check_use_default_set(void) {
    //uniformity
    if(ST.Uniformity.Max_Threshold < 0)
        ST.Uniformity.Max_Threshold = _SensorTest_Uniformaity_Max_Threshold_V6_;
    if(ST.Uniformity.Min_Threshold < 0)
        ST.Uniformity.Min_Threshold = _SensorTest_Uniformaity_Min_Threshold_V6_;
    if(ST.Uniformity.FrameCount < 0)
        ST.Uniformity.FrameCount =    _SensorTest_Uniformity_FrameCount_V6_;
    if(ST.Uniformity.Win1_Threshold < 0)
        ST.Uniformity.Win1_Threshold =_SensorTest_Uniformity_Win1_Threshold_V6_;
    if(ST.Uniformity.Win1_FailCount < 0)
        ST.Uniformity.Win1_FailCount =_SensorTest_Uniformity_Win1_FailCount_V6_;
    if(ST.Uniformity.Win2_Threshold < 0)
        ST.Uniformity.Win2_Threshold =_SensorTest_Uniformity_Win2_Threshold_V6_;
    if(ST.Uniformity.Win2_FailCount < 0)
        ST.Uniformity.Win2_FailCount =_SensorTest_Uniformity_Win2_FailCount_V6_;
    if(ST.Uniformity.Up_FailCount < 0)
        ST.Uniformity.Up_FailCount = _SensorTest_Uniformity_RawData_Tolerance;
    
    //open
    if(ST.Open_Threshold < 0)
        ST.Open_Threshold = _SensorTest_Open_Threshold_V6_;
    if(ST.Open_FrameCount < 0)
        ST.Open_FrameCount = _SensorTest_Open_FrameCount_V6_;
    if(ST.Open_TX_Aver_Diff < 0)
        ST.Open_TX_Aver_Diff = _SensorTestOpenTxAverDiff_V6_;
    if (ST.Open_RX_Delta_Threshold < 0)
        ST.Open_RX_Delta_Threshold = _SensorTestOpenDeltaRX_;
    if (ST.Open_RX_Continue_Fail_Tolerance < 0)
        ST.Open_RX_Continue_Fail_Tolerance = _SensorTestOpenRXTolerance_;
    //short
    if(ST.Short.Threshold < 0)
        ST.Short.Threshold = _SensorTest_Short_Thresshold_V6_;
    if(ST.Short.FrameCount < 0)
        ST.Short.FrameCount = _SensorTest_Short_FrameCount_V6_;
    if(ST.Short.dump1 < 0)
        ST.Short.dump1 = _SensorTest_Short_Dump1_V6_;
    if(ST.Short.dump2 < 0)
        ST.Short.dump2 = _SensorTest_Short_Dump2_V6_;
    if(ST.Short.vref_v < 0) {
        ST.Short.vref_v = _SensorTest_Short_Vref_V6_;
    }
    if(ST.Short.posidleL < 0)
        ST.Short.posidleL = _SensorTest_Short_posidleL_V6_;
    if(ST.Short.posidleH < 0)
        ST.Short.posidleH = _SensorTest_Short_posidleH_V6_;
}

bool check_ini_section(char *title, char *filename)
{
    FILE *fp;
    char szLine[1024];
    int rtnval;
    int i = 0;
    char *tmp;
    bool status = false;
    unsigned short isUni;
    bool MultiByteEnable = false;
    bool FindDataByte = false;
    int buteCount = 0;

    if ((fp = fopen(filename, ("rb"))) == NULL)
    {
        PRINTF("have no such file :%s\n",filename);
        return "";
    }
    fread(&(isUni), 1, 2, fp);
    if(isUni == (unsigned short)0xBBEF){
        //PRINTF("%s,%d,UTF-8 have Bom\n", __func__, __LINE__);
        //? ç‚ºbom?¯ä??‹byteï¼Œæ?ä»¥è?å¤šè?ä¸€?‹byte
        rtnval = fgetc(fp);
    }
    else if (isUni == (unsigned short)0xFFFE || isUni == (unsigned short)0xFEFF || isUni == (unsigned short)0xFEBB) {
        MultiByteEnable = true;
        //PRINTF("%s,%d, Format is Unicode\n", __func__, __LINE__);
    }
    else {
        //PRINTF("%s,%d, Format is UTF-8\n", __func__, __LINE__);
        szLine[1] = isUni >> 8;
        szLine[0] = isUni;
        i = 2;
    }
    while (!feof(fp))
    {
        rtnval = fgetc(fp);
        buteCount++;
        //?¤æ–·Unicode data?¨high/Low byte
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
            tmp = strchr(szLine, '=');
            {
                strcpy(tmpstr, "[");
                strcat(tmpstr, title);
                strcat(tmpstr, "]");
                if (strncmp(tmpstr, szLine, strlen(tmpstr)) == 0)
                {
                    PRINTF("dat file exist[%s]\n", title);
                    status = true;
                }
            }
        }
    }
    fclose(fp);
    return status;
}

char *GetIniSectionString(char *title, char *tmp_str, char *filename)
{
    FILE *fp;
    //char szLine[1024];
    char *szLine;
    int rtnval;
    int i = 0;
    int flag = 0;
    char *splitResult;
    
    if ((fp = fopen(filename, ("r"))) == NULL)
    {
        PRINTF("have no such file :%s\n",filename);
    }
    szLine = malloc(4096);
    while (!feof(fp))
    {
        rtnval = fgetc(fp);
        szLine[i++] = rtnval;
        if(szLine[0] == 0xEF && szLine[1] == 0xBB && szLine[2] == 0xBF){
            //PRINTF("%s,%d,UTF-8 have Bom\n", __func__, __LINE__);
            i = 0;
        }    
        if (rtnval == '\n')
        {
#ifndef WIN32
            i--;
#endif
            szLine[--i] = '\0';
            i = 0;
            if ((flag == 1))
            {
                splitResult = strtok(szLine,"\n");
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
                        strcat(tmp_str, szLine);
                        //printf("%s\n", tmp_str);
                    }
                }
            }
            else
            {
                strcpy(tmpstr, "[");
                strcat(tmpstr, title);
                strcat(tmpstr, "]");
                if (strncmp(tmpstr, szLine, strlen(tmpstr)) == 0)
                {
                    flag = 1;
                }
            }
        }
    }
    free(szLine);
    fclose(fp);
    return "";
}

void vfReadBenchMarkValue(char *title,int BenchMarkBuf[][_MaxChanelNum_], char *filename)
{
    unsigned char ucloopCh_X;
    unsigned char ucloopCh_Y;
    unsigned char ucIndex;
    int iCount;
    char key[10];
    char *strValue;
    char strData[10];
    for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch; ucloopCh_Y++)
    {
        sprintf(key, "%d", ucloopCh_Y);
        strValue = GetIniKeyString(title, key, filename);
        if (strcmp("", strValue) == 0)
        {
            return;
        }
        ucloopCh_X = 0;
        ucIndex = 0;
        memset(strData,0,sizeof(strData));
        for(iCount = 0; iCount < strlen(strValue); iCount++)
        {
            if(strValue[iCount] == ',')
            {
                strData[ucIndex] = '\0';
                BenchMarkBuf[ucloopCh_X][ucloopCh_Y] = atoi(strData) + ST.OffsetValue;
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
    PRINTF("\nBenchMark_[%s]\n", title);
    for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch; ucloopCh_Y++)
    {
        for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch; ucloopCh_X++)
        {
            PRINTF("%5d,", BenchMarkBuf[ucloopCh_X][ucloopCh_Y]);
        }
        PRINTF("\n");
    }
    strValue = NULL;
}

void vfReadBenchMarkValue_V6(char *title, char *filename, SensorTest_BenBenchmark_Node **data, int x, int y)
{
    unsigned char ucloopCh_X;
    unsigned char ucloopCh_Y;
    int iCount;
    char *strValue;
    int filesize = get_file_size(filename);
    char *strData;

    strValue = calloc(filesize, sizeof(char));
    GetIniSectionString(title, strValue, filename);
    strData = strtok(strValue, ";");
    iCount = 0;
    while (strData != NULL)
    {
        sscanf(strData,"%d,%d,%d,%d", &data[iCount%x][iCount/x].ini.data,
        &data[iCount%x][iCount/x].ini.max,
        &data[iCount%x][iCount/x].ini.min,
        &data[iCount%x][iCount/x].ini.type);
        // printf("[%d]%s,%d,%d,%d,%d,%d\n", iCount, strData, data[iCount%x][iCount/x].ini.data, data[iCount%x][iCount/x].ini.data,data[iCount%x][iCount/x].ini.max,
        //                                 data[iCount%x][iCount/x].ini.min, data[iCount%x][iCount/x].ini.type);
        iCount++;
        strData = strtok( NULL, ";");
    }

    PRINTF("\n[%s]\nData ,", title);
    for (ucloopCh_X = 0; ucloopCh_X < x; ucloopCh_X++)
        PRINTF("X_%3d,", ucloopCh_X);
    for (ucloopCh_Y = 0; ucloopCh_Y < y; ucloopCh_Y++) {
        PRINTF("\nY_%3d,", ucloopCh_Y);
        for (ucloopCh_X = 0; ucloopCh_X < x; ucloopCh_X++) {
            PRINTF("%5d,", data[ucloopCh_X][ucloopCh_Y].ini.data);
        }
    }
    PRINTF("\n[%s]\n Max ,", title);
    for (ucloopCh_X = 0; ucloopCh_X < x; ucloopCh_X++)
        PRINTF("X_%3d,", ucloopCh_X);
    for (ucloopCh_Y = 0; ucloopCh_Y < y; ucloopCh_Y++) {
        PRINTF("\nY_%3d,", ucloopCh_Y);
        for (ucloopCh_X = 0; ucloopCh_X < x; ucloopCh_X++) {
            PRINTF("%5d,", data[ucloopCh_X][ucloopCh_Y].ini.max);
        }
    }
    PRINTF("\n[%s]\n Min ,", title);
    for (ucloopCh_X = 0; ucloopCh_X < x; ucloopCh_X++)
        PRINTF("X_%3d,", ucloopCh_X);
    for (ucloopCh_Y = 0; ucloopCh_Y < y; ucloopCh_Y++) {
        PRINTF("\nY_%3d,", ucloopCh_Y);
        for (ucloopCh_X = 0; ucloopCh_X < x; ucloopCh_X++) {
            PRINTF("%5d,", data[ucloopCh_X][ucloopCh_Y].ini.min);
        }
    }
    PRINTF("\n");
    free(strValue);
}

void vfSaveFWVerTestLog_V3(FILE *fp)
{
    int i = 0;

    if ((ucSensorTestResult & FWVERTION_TEST) == FWVERTION_TEST)
        fprintf(fp, "[Firmware_Version]       ,NG ,Version,V");
    else
    {
        fprintf(fp, "[Firmware_Version]       ,OK ,Version,V");
    }
    for(i = 0; i < 8; i++)
    {
        fprintf(fp, "%2x.", FWVersion[i]);
    }
    fprintf(fp, ",\n");
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
    int key_ych = 0;
    
    if ((ucSensorTestResult & MICROSHORT_TEST) == MICROSHORT_TEST)
        fprintf(fp, "[MicroShort_Test]        ,NG,\n");
    else
    {
        fprintf(fp, "[MicroShort_Test]        ,OK,\n");
    }

    fprintf(fp, "MicroShort_Test_Window_1,%d\n", ST.Short.Threshold);

    fprintf(fp, "MicroShort_Test_Window_2,%d\n", ST.Short.Threshold);
    if (ptl.key_mode == ILITEK_HW_KEY_MODE) {
        fprintf(fp, "KeyRX_Threshold,%d\n", ST.Short.keyRx_thr);
        fprintf(fp, "KeyTX_Threshold,%d\n", ST.Short.keyTx_thr);
        key_ych++;
    }

    fprintf(fp, "      X_Data_M_00,");

    for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch; ucloopCh_X++)
        fprintf(fp, " CH%02d,", ucloopCh_X);
    for (ucloopCh_X = ptl.x_ch; ucloopCh_X < ptl.x_ch + ptl.key_num; ucloopCh_X++)
        fprintf(fp, "(KeyX%d),", ucloopCh_X);

        fprintf(fp, "\n      X_Data_M_01,");

    for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch; ucloopCh_X++)
        fprintf(fp, "%3d  ,", ST.short_daltc[0][ucloopCh_X]);
    for (ucloopCh_X = ptl.x_ch; ucloopCh_X < ptl.x_ch + ptl.key_num; ucloopCh_X++)
        fprintf(fp, "%3d  ,", ST.short_daltc[0][ucloopCh_X]);

    fprintf(fp, "\n      X_Data_M_02,");

    for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch; ucloopCh_X++)
        fprintf(fp, "%3d  ,", ST.short_daltc[2][ucloopCh_X]);
    for (ucloopCh_X = ptl.x_ch; ucloopCh_X < ptl.x_ch + ptl.key_num; ucloopCh_X++)
        fprintf(fp, "%3d  ,", ST.short_daltc[2][ucloopCh_X]);


    fprintf(fp, "\n        X_M_Delta,");

    for (ucloopCh_X = ptl.x_ch; ucloopCh_X < ptl.x_ch + ptl.key_num; ucloopCh_X++)
    {
        fprintf(fp, "%3d  ,", abs(ST.short_daltc[2][ucloopCh_X] - ST.short_daltc[0][ucloopCh_X]));

    }
    fprintf(fp, "\n\n");


    fprintf(fp, "       _Data_M_00,");

    for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch; ucloopCh_Y++)
        fprintf(fp, " CH%02d,", ucloopCh_Y);
    if (ptl.key_mode == ILITEK_HW_KEY_MODE)
        fprintf(fp, "(KeyY_1),");

    fprintf(fp, "\n       _Data_M_01,");

    for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch + key_ych; ucloopCh_Y++)
        fprintf(fp, "%3d  ,", ST.short_daltc[1][ucloopCh_Y]);

    fprintf(fp, "\n       _Data_M_02,");

    for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch + key_ych; ucloopCh_Y++)
        fprintf(fp, "%3d  ,", ST.short_daltc[3][ucloopCh_Y]);

    fprintf(fp, "\n        Y_M_Delta,");

    for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch + key_ych; ucloopCh_Y++)
        fprintf(fp, "%3d  ,", abs(ST.short_daltc[3][ucloopCh_Y] - ST.short_daltc[1][ucloopCh_Y]));

    fprintf(fp, "\n");

}

void vfSaveFWVerTestLog_V6(FILE *fp)
{
    int i = 0;

    if ((ucSensorTestResult & FWVERTION_TEST) == FWVERTION_TEST)
        fprintf(fp, "[FW_Verify]                           ,NG ,\n");
    else
    {
        fprintf(fp, "[FW_Verify]                           ,OK ,\n");
    }

    if(ptl.ver <= PROTOCOL_V6_0_2) {
        fprintf(fp, "The protocol no support\n");
        return;
    }

    fprintf(fp, "\n      (Spec.),\n");

    fprintf(fp, "      Path                    ,%s\n",ST.hexfile);

    fprintf(fp, "      FW Ver.                 ,V%x.%x.%x.%x\n",ST.fw_ver[0],ST.fw_ver[1],ST.fw_ver[2],ST.fw_ver[3]);

    fprintf(fp, "      Master CRC              ,");

    for(i = 0; i < ST.block_num; i++) {
        fprintf(fp, "%X,", ST.master_crc[i]);

    }
    fprintf(fp, "\n");

    fprintf(fp, "      Slave CRC               ,");

    for(i = 0; i < ST.slave_num; i++) {
        fprintf(fp, "%X,", ST.slave_crc[i]);

    }
    fprintf(fp, "\n");

}

void vfSaveOpenTestLog_V6(FILE *fp)
{
	unsigned int ucloopCh_X;
	unsigned int ucloopCh_Y;
    unsigned int ucloop;

    if ((ucSensorTestResult & OPEN_TEST) == OPEN_TEST)
    {
        fprintf(fp, "[Open Test]                            ,NG ,\n");
    }
    else
    {
        fprintf(fp, "[Open_Test]                            ,OK ,\n");
    }

    fprintf(fp, "\n      (Spec.),\n");

    fprintf(fp, "   Frame_Count                ,%d\n", ST.Open_FrameCount);

    fprintf(fp, "   Min_Threshold              ,%d\n", ST.Open_Threshold);

    fprintf(fp, "   TX_Average_Diff_Gap        ,%d\n", ST.Open_TX_Aver_Diff); 

    fprintf(fp, "   RX_Diff_Gap                ,%d\n", ST.Open_RX_Delta_Threshold); 

    fprintf(fp, "   RX_Diff_Gap_Tolerance      ,%d\n", ST.Open_RX_Continue_Fail_Tolerance); 

    fprintf(fp, "   Key_Threshold      ,%d\n", ST.Open.key_thr); 

    fprintf(fp, "\n      Normal        ,");

    for (ucloopCh_X = 1; ucloopCh_X <= ptl.x_ch; ucloopCh_X++)
    {
        fprintf(fp, "(X_%03d),", ucloopCh_X);

    }
    fprintf(fp, "Average,\n");


    for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch; ucloopCh_Y++)
    {
        fprintf(fp, "      Y_%03d         ,", ucloopCh_Y + 1);

        for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch; ucloopCh_X++)
        {
            if(ST.v6_open_daltc[ucloopCh_X][ucloopCh_Y].status)
                fprintf(fp, "*%5d ,", ST.v6_open_daltc[ucloopCh_X][ucloopCh_Y].data);
            else
                fprintf(fp, "%6d ,", ST.v6_open_daltc[ucloopCh_X][ucloopCh_Y].data);
        }
        if(ST.Tx_Avdiff_daltc[ucloopCh_Y].status == NODE_FAIL) {
            fprintf(fp, "*%5d ,\n", ST.Tx_Avdiff_daltc[ucloopCh_Y].data);
        }
        else {
            fprintf(fp, "%6d ,\n", ST.Tx_Avdiff_daltc[ucloopCh_Y].data);
        }
    }
    if (ptl.key_mode == ILITEK_HW_KEY_MODE) {
        fprintf(fp, "      Key_1         ,");
        for (ucloop = 0; ucloop < ptl.key_num; ucloop++) {
            if(ST.Open.key_daltc[ucloop].status == NODE_FAIL)
                fprintf(fp, "*%5d ,", ST.Open.key_daltc[ucloop].data);
            else
                fprintf(fp, "%6d ,", ST.Open.key_daltc[ucloop].data);
        }
        fprintf(fp, "\n");
    }
    if(ST.Open_RX_Delta_Threshold > 0) {
        fprintf(fp, "\n      RX_Diff       ,");

        for (ucloopCh_X = 1; ucloopCh_X <= ptl.x_ch; ucloopCh_X++)
        {
            fprintf(fp, "(X_%03d),", ucloopCh_X);

        }
        fprintf(fp, "\n");
        for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch; ucloopCh_Y++)
        {
            fprintf(fp, "      Y_%03d         ,", ucloopCh_Y + 1);

            for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch; ucloopCh_X++)
            {
                if(ST.open_Rx_diff[ucloopCh_X][ucloopCh_Y].status)
                    fprintf(fp, "*%5d ,", ST.open_Rx_diff[ucloopCh_X][ucloopCh_Y].data);
                else
                    fprintf(fp, "%6d ,", ST.open_Rx_diff[ucloopCh_X][ucloopCh_Y].data);
            }
            fprintf(fp, "\n");
        }
    }
}

void vfSaveMircoOpenTestLog_V6(FILE *fp)
{
	unsigned int ucloopCh_X;
	unsigned int ucloopCh_Y;

    if ((ucSensorTestResult & MIRCO_OPEN_TEST) == MIRCO_OPEN_TEST)
    {
        fprintf(fp, "[MicroOpen_Test]                 ,NG ,\n");
    }
    else
    {
        fprintf(fp, "[MicroOpen_Test]                 ,OK ,\n");
    }
    fprintf(fp, "\n      (Spec.)                    ,\n");
    fprintf(fp, "      Frame_Count                ,%d\n", ST.MOpen.FrameCount);
    fprintf(fp, "      TX_Avg_Delta_Threshold     ,%d\n", ST.MOpen.TxAvgThr);
    fprintf(fp, "      RX_Delta_Threshold         ,%d\n", ST.MOpen.RxDiffThr); 
    fprintf(fp, "      RX_Delta_Threshold_Tolerance,%d\n", ST.MOpen.RxToles);
    
    if(ST.MOpen.RxDeltaEn == ENABLE_TEST) {
        fprintf(fp, "\n      RX_Delta      ,");
        for (ucloopCh_X = 1; ucloopCh_X < ptl.x_ch; ucloopCh_X++)
        {
            fprintf(fp, "(D_%03d),", ucloopCh_X);

        }
        fprintf(fp, "\n");
        for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch; ucloopCh_Y++)
        {
            fprintf(fp, "      Y_%03d         ,", ucloopCh_Y + 1);

            for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch - 1; ucloopCh_X++)
            {
                if(ST.MOpen.rx_diff[ucloopCh_X][ucloopCh_Y].raw.status)
                    fprintf(fp, "*%5d ,", ST.MOpen.rx_diff[ucloopCh_X][ucloopCh_Y].raw.data);
                else
                    fprintf(fp, "%6d ,", ST.MOpen.rx_diff[ucloopCh_X][ucloopCh_Y].raw.data);
            }
            fprintf(fp, "\n");
        }
    }
    if(ST.MOpen.TxAvgEn == ENABLE_TEST) {
        fprintf(fp, "\n\n      TX_Avg_Delta  ,(Avg_D),\n");
        for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch - 1; ucloopCh_Y++)
        {
            if(ST.MOpen.tx_avg[0][ucloopCh_Y].raw.status)
                fprintf(fp, "      D_%03d           ,*%5d ,\n", ucloopCh_Y+1, ST.MOpen.tx_avg[0][ucloopCh_Y].raw.data);
            else
                fprintf(fp, "      D_%03d           ,%6d ,\n", ucloopCh_Y+1, ST.MOpen.tx_avg[0][ucloopCh_Y].raw.data);
        } 
    }

    fprintf(fp, "\n      Frame_1       ,");
    for (ucloopCh_X = 1; ucloopCh_X <= ptl.x_ch; ucloopCh_X++)
    {
        fprintf(fp, "(X_%03d),", ucloopCh_X);

    }
    fprintf(fp, "(Y_Avg),\n");

    for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch; ucloopCh_Y++)
    {
        fprintf(fp, "      Y_%03d         ,", ucloopCh_Y + 1);
        for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch; ucloopCh_X++)
        {
            if(ST.v6_open_daltc[ucloopCh_X][ucloopCh_Y].status)
                fprintf(fp, "*%5d ,", ST.v6_open_daltc[ucloopCh_X][ucloopCh_Y].data);
            else
                fprintf(fp, "%6d ,", ST.v6_open_daltc[ucloopCh_X][ucloopCh_Y].data);
        }
        if(ST.Tx_Avdiff_daltc[ucloopCh_Y].status == NODE_FAIL) {
            fprintf(fp, "*%5d ,\n", ST.Tx_Avdiff_daltc[ucloopCh_Y].data);
        }
        else {
            fprintf(fp, "%6d ,\n", ST.Tx_Avdiff_daltc[ucloopCh_Y].data);
        }
    }

}

void SaveUniformitytoCSV_V6(char *name, FILE *fp, SensorTest_BenBenchmark_Node **data, int x, int y, int type) {
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

void vfSaveUniformityTestLog_V6(FILE *fp)
{
    unsigned int loopCh_X;
    unsigned int loopCh_Y;

    if ((ucSensorTestResult & UNIFORMITY_TEST) == UNIFORMITY_TEST)
    {
        fprintf(fp, "[Uniformity Test]                      ,NG ,\n");
    }
    else
    {
        fprintf(fp, "[Uniformity_Test]                      ,OK ,\n");
    }
    fprintf(fp, "\n      (Spec.),\n");
    fprintf(fp, "   Frame Count             ,%d\n", ST.Uniformity.FrameCount);
    if(ST.PFVer >= PROFILE_V1_0_2_0) {
        fprintf(fp, "   Uniformity_RawData_Max_Threshold,%d\n", ST.Uniformity.Max_Threshold);
        fprintf(fp, "   Uniformity_RawData_Max_Threshold_Tolerance,%d\n", ST.Uniformity.Up_FailCount);
        fprintf(fp, "   Uniformity_RawData_Min_Threshold,%d\n", ST.Uniformity.Min_Threshold);
        fprintf(fp, "   Uniformity_RawData_Min_Threshold_Tolerance,%d\n", ST.Uniformity.Low_FailCount);
        fprintf(fp, "   Uniformity_Win1_Max_Threshold,%d\n", ST.Uniformity.Win1_Threshold);
        fprintf(fp, "   Uniformity_Win1_Max_Threshold_Tolerance,%d\n", ST.Uniformity.Win1_FailCount);
        fprintf(fp, "   Uniformity_Win2_Max_Threshold,%d\n", ST.Uniformity.Win2_Threshold);
        fprintf(fp, "   Uniformity_Win2_Max_Threshold_Tolerance,%d\n", ST.Uniformity.Win2_FailCount);
        if(ST.Uniformity.En_allraw) {
            fprintf(fp, "\n      RawData_Max   ,");
            for (loopCh_X = 1; loopCh_X <= ptl.x_ch; loopCh_X++)
            {
                fprintf(fp, "(X_%03d),", loopCh_X);
            }
            fprintf(fp, "\n");
            for (loopCh_Y = 0; loopCh_Y < ptl.y_ch; loopCh_Y++)
            {
                fprintf(fp, "      Y_%03d         ,", loopCh_Y + 1);
                for (loopCh_X = 0; loopCh_X < ptl.x_ch; loopCh_X++)
                {
                    if(ST.v6_unifor_daltc[loopCh_X][loopCh_Y].max_st)
                        fprintf(fp, "*%6d,", ST.v6_unifor_daltc[loopCh_X][loopCh_Y].data);
                    else
                        fprintf(fp, "%7d,", ST.v6_unifor_daltc[loopCh_X][loopCh_Y].data);
                }
                fprintf(fp, "\n");
            }
            //SaveRawDatatoCSV_V6("Lower_Limit", FILE *fp,  ST.v6_unifor_daltc, ptl.x_ch, ptl.y_ch, NODE_MIN_STATUS);
            fprintf(fp, "\n      RawData_Min   ,");
            for (loopCh_X = 1; loopCh_X <= ptl.x_ch; loopCh_X++)
            {
                fprintf(fp, "(X_%03d),", loopCh_X);
            }
            fprintf(fp, "\n");
            for (loopCh_Y = 0; loopCh_Y < ptl.y_ch; loopCh_Y++)
            {
                fprintf(fp, "      Y_%03d         ,", loopCh_Y + 1);
                for (loopCh_X = 0; loopCh_X < ptl.x_ch; loopCh_X++)
                {
                    if(ST.v6_unifor_daltc[loopCh_X][loopCh_Y].min_st)
                        fprintf(fp, "*%6d,", ST.v6_unifor_daltc[loopCh_X][loopCh_Y].data);
                    else
                        fprintf(fp, "%7d,", ST.v6_unifor_daltc[loopCh_X][loopCh_Y].data);
                }
                fprintf(fp, "\n");
            }
        }

        if(ST.Uniformity.En_allwin1) {
            fprintf(fp, "\n      Win1_Max      ,");

            for (loopCh_X = 0; loopCh_X < ptl.x_ch; loopCh_X++)
            {
                fprintf(fp, "(X_%03d),", loopCh_X);

            }
            fprintf(fp, "\n");

            for (loopCh_Y = 0; loopCh_Y < ptl.y_ch - 1; loopCh_Y++)
            {
                fprintf(fp, "      Y_%03d         ,", loopCh_Y + 1);

                for (loopCh_X = 0; loopCh_X < ptl.x_ch; loopCh_X++)
                {
                    if(ST.v6_unifor_win1[loopCh_X][loopCh_Y].status)
                        fprintf(fp, "*%6d,", ST.v6_unifor_win1[loopCh_X][loopCh_Y].data);
                    else
                        fprintf(fp, "%7d,", ST.v6_unifor_win1[loopCh_X][loopCh_Y].data);
                }
                fprintf(fp, "\n");

            }
        }

        if(ST.Uniformity.En_allwin2) {
            fprintf(fp, "\n      Win2_Max      ,");

            for (loopCh_X = 1; loopCh_X <= ptl.x_ch; loopCh_X++)
            {
                fprintf(fp, "(X_%03d),", loopCh_X);

            }
            fprintf(fp, "\n");

            for (loopCh_Y = 0; loopCh_Y < ptl.y_ch - 1; loopCh_Y++)
            {
                fprintf(fp, "      Y_%03d         ,", loopCh_Y + 1);

                for (loopCh_X = 0; loopCh_X < ptl.x_ch - 1; loopCh_X++)
                {
                    if(ST.v6_unifor_win2[loopCh_X][loopCh_Y].status)
                        fprintf(fp, "*%6d,", ST.v6_unifor_win2[loopCh_X][loopCh_Y].data);
                    else
                        fprintf(fp, "%7d,", ST.v6_unifor_win2[loopCh_X][loopCh_Y].data);
            
                }
                fprintf(fp, "\n");
            }
        }
    }
    else {
        fprintf(fp, "   Max_Threshold           ,%d\n", ST.Uniformity.Max_Threshold);
        fprintf(fp, "   Up_FailCount            ,%d\n", ST.Uniformity.Up_FailCount);
        fprintf(fp, "   Min_Threshold           ,%d\n", ST.Uniformity.Min_Threshold);
        fprintf(fp, "   Low_FailCount           ,%d\n", ST.Uniformity.Low_FailCount);
        fprintf(fp, "   Win1_Threshold          ,%d\n", ST.Uniformity.Win1_Threshold);
        fprintf(fp, "   Win1_FailCount          ,%d\n", ST.Uniformity.Win1_FailCount);
        fprintf(fp, "   Win2_Threshold          ,%d\n", ST.Uniformity.Win2_Threshold);
        fprintf(fp, "   Win2_FailCount          ,%d\n", ST.Uniformity.Win2_FailCount);
        //SaveUniformitytoCSV_V6("Upper_Limit", FILE *fp,  ST.v6_unifor_daltc, ptl.x_ch, ptl.y_ch, NODE_MAX_STATUS);
        fprintf(fp, "\n      Upper_Limit   ,");
        for (loopCh_X = 1; loopCh_X <= ptl.x_ch; loopCh_X++)
        {
            fprintf(fp, "(X_%03d),", loopCh_X);
        }
        fprintf(fp, "\n");
        for (loopCh_Y = 0; loopCh_Y < ptl.y_ch; loopCh_Y++)
        {
            fprintf(fp, "      Y_%03d         ,", loopCh_Y);
            for (loopCh_X = 0; loopCh_X < ptl.x_ch; loopCh_X++)
            {
                if(ST.v6_unifor_daltc[loopCh_X][loopCh_Y].max_st)
                    fprintf(fp, "*%6d,", ST.v6_unifor_daltc[loopCh_X][loopCh_Y].data);
                else
                    fprintf(fp, "%7d,", ST.v6_unifor_daltc[loopCh_X][loopCh_Y].data);
            }
            fprintf(fp, "\n");
        }
        //SaveRawDatatoCSV_V6("Lower_Limit", FILE *fp,  ST.v6_unifor_daltc, ptl.x_ch, ptl.y_ch, NODE_MIN_STATUS);
        fprintf(fp, "\n      Lower_Limit   ,");
        for (loopCh_X = 1; loopCh_X <= ptl.x_ch; loopCh_X++)
        {
            fprintf(fp, "(X_%03d),", loopCh_X);
        }
        fprintf(fp, "\n");
        for (loopCh_Y = 0; loopCh_Y < ptl.y_ch; loopCh_Y++)
        {
            fprintf(fp, "      Y_%03d         ,", loopCh_Y);
            for (loopCh_X = 0; loopCh_X < ptl.x_ch; loopCh_X++)
            {
                if(ST.v6_unifor_daltc[loopCh_X][loopCh_Y].min_st)
                    fprintf(fp, "*%6d,", ST.v6_unifor_daltc[loopCh_X][loopCh_Y].data);
                else
                    fprintf(fp, "%7d,", ST.v6_unifor_daltc[loopCh_X][loopCh_Y].data);
            }
            fprintf(fp, "\n");
        }

        fprintf(fp, "\n      Win1          ,");

        for (loopCh_X = 1; loopCh_X <= ptl.x_ch; loopCh_X++)
        {
            fprintf(fp, "(X_%03d),", loopCh_X);

        }
        fprintf(fp, "\n");

        for (loopCh_Y = 0; loopCh_Y < ptl.y_ch - 1; loopCh_Y++)
        {
            fprintf(fp, "      Y_%03d         ,", loopCh_Y);

            for (loopCh_X = 0; loopCh_X < ptl.x_ch; loopCh_X++)
            {
                if(ST.v6_unifor_win1[loopCh_X][loopCh_Y].status)
                    fprintf(fp, "*%6d,", ST.v6_unifor_win1[loopCh_X][loopCh_Y].data);
                else
                    fprintf(fp, "%7d,", ST.v6_unifor_win1[loopCh_X][loopCh_Y].data);
            }
            fprintf(fp, "\n");

        }

        fprintf(fp, "\n      Win2          ,");

        for (loopCh_X = 1; loopCh_X <= ptl.x_ch; loopCh_X++)
        {
            fprintf(fp, "(X_%03d),", loopCh_X);

        }
        fprintf(fp, "\n");

        for (loopCh_Y = 0; loopCh_Y < ptl.y_ch - 1; loopCh_Y++)
        {
            fprintf(fp, "      Y_%03d         ,", loopCh_Y);

            for (loopCh_X = 0; loopCh_X < ptl.x_ch - 1; loopCh_X++)
            {
                if(ST.v6_unifor_win2[loopCh_X][loopCh_Y].status)
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
}

void vfSaveShortTestLog_V6(FILE *fp)
{
    unsigned int ucloopCh_X;
    unsigned int ucloopCh_Y;
    unsigned int ucloop;

    if ((ucSensorTestResult & MICROSHORT_TEST) == MICROSHORT_TEST)
        fprintf(fp, "\n[Short Test]                           ,NG ,\n\n");
    else
    {
        fprintf(fp, "\n[Short_Test]                           ,OK ,\n\n");
    }

    fprintf(fp, "      (Spec.),\n");

    fprintf(fp, "   Frame_Count             ,%d\n", ST.Short.FrameCount);

    fprintf(fp, "   Max_Threshold           ,%d\n", ST.Short.Threshold);

    fprintf(fp, "   Dump_1                  ,%d\n", ST.Short.dump1);

    fprintf(fp, "   Dump_2                  ,%d\n", ST.Short.dump2);

    fprintf(fp, "   KeyTX_Threshold         ,%d\n", ST.Short.keyTx_thr);

    fprintf(fp, "   KeyRX_Threshold         ,%d\n", ST.Short.keyRx_thr);

    fprintf(fp, "\n      Normal        ,");

    for (ucloopCh_X = 1; ucloopCh_X <= ptl.x_ch; ucloopCh_X++)
    {
        fprintf(fp, "(X_%03d),", ucloopCh_X);

    }
    if (ptl.key_mode == ILITEK_HW_KEY_MODE) {
        for (ucloop = 1; ucloop <= ptl.key_num; ucloop++)
            fprintf(fp, "(KeyX_%d),", ucloop);
    }
    fprintf(fp, "\n       X_SLK        ,");

    for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch; ucloopCh_X++)
    {
        if(ST.v6_short_daltc[0][ucloopCh_X].status == NODE_FAIL)
            fprintf(fp, "*%6d,", ST.v6_short_daltc[0][ucloopCh_X].data);
        else
            fprintf(fp, "%7d,", ST.v6_short_daltc[0][ucloopCh_X].data);

    }
    if (ptl.key_mode == ILITEK_HW_KEY_MODE) {
        for (ucloop = 0; ucloop < ptl.key_num; ucloop++) {
            if(ST.Short.key_daltc[ucloop].status == NODE_FAIL)
                fprintf(fp, "*%6d,", ST.Short.key_daltc[ucloop].data);
            else
                fprintf(fp, "%7d,", ST.Short.key_daltc[ucloop].data);
        }
    }
    // fprintf(fp, "\n      X_Resistance  ,");

    // for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch; ucloopCh_X++)
    // {
    //     if(ST.v6_short_daltc[0][ucloopCh_X].data < DIVIDE_10M)
    //         fprintf(fp, "%d.00M ,", 10);
    //     else
    //         fprintf(fp, "%5.2lfM ,", IMPEDANCE_MACRO(ST.v6_short_daltc[0][ucloopCh_X].data, ST.Short.dump1, ST.Short.dump2,ST.Short.vref_v));

    // }
    fprintf(fp, "\n                    ,");
    for (ucloopCh_Y = 1; ucloopCh_Y <= ptl.y_ch; ucloopCh_Y++)
    {
        fprintf(fp, "(Y_%03d),", ucloopCh_Y);
    }
    if (ptl.key_mode == ILITEK_HW_KEY_MODE) {
        fprintf(fp, "(KeyY_1),");
    }
    fprintf(fp, "\n       Y_SLK        ,");

    for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch; ucloopCh_Y++)
    {
        if(ST.v6_short_daltc[1][ucloopCh_Y].status == NODE_FAIL)
            fprintf(fp, "*%6d,", ST.v6_short_daltc[1][ucloopCh_Y].data);
        else
            fprintf(fp, "%7d,", ST.v6_short_daltc[1][ucloopCh_Y].data);

    }
    if (ptl.key_mode == ILITEK_HW_KEY_MODE) {
        if(ST.Short.key_daltc[ptl.key_num].status == NODE_FAIL)
            fprintf(fp, "*%6d,", ST.Short.key_daltc[ptl.key_num].data);
        else
            fprintf(fp, "%7d,", ST.Short.key_daltc[ptl.key_num].data);
    }
    // fprintf(fp, "\n      Y_Resistance  ,");

    // for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch; ucloopCh_Y++)
    // {
    //     if(ST.v6_short_daltc[1][ucloopCh_Y].data < DIVIDE_10M)
    //         fprintf(fp, "%d.00M ,", 10);
    //     else
    //         fprintf(fp, "%5.2lfM ,", IMPEDANCE_MACRO(ST.v6_short_daltc[1][ucloopCh_Y].data, ST.Short.dump1, ST.Short.dump2,ST.Short.vref_v));

    // }
    fprintf(fp, "\n\n");
}
void vfSaveUniformityTestLog(FILE *fp)
{
    unsigned char ucloopCh_X;
    unsigned char ucloopCh_Y;

    if ((ucSensorTestResult & UNIFORMITY_TEST) == UNIFORMITY_TEST)
        fprintf(fp, "[Uniformity_Test]        ,NG,\n");
    else
    {
        fprintf(fp, "[Uniformity_Test]        ,OK,\n");
    }

    fprintf(fp, "BD_VA_L_Ratio_Min :      %3d%%,  BD_VA_L_Ratio_Max :       %3d%%\n"
    ,ST.Uniformity.uiBD_VA_L_Ratio_Min,ST.Uniformity.uiBD_VA_L_Ratio_Max);

    fprintf(fp, "BD_VA_R_Ratio_Min :      %3d%%,  BD_VA_R_Ratio_Max :       %3d%%\n"
    ,ST.Uniformity.uiBD_VA_R_Ratio_Min,ST.Uniformity.uiBD_VA_R_Ratio_Max);

    fprintf(fp, "BD_VA_Top_Ratio_Min :     %3d%%,  BD_VA_Top_Ratio_Max :     %3d%%\n"
    ,ST.Uniformity.uiBD_VA_Top_Ratio_Min,ST.Uniformity.uiBD_VA_Top_Ratio_Max);

    fprintf(fp, "BD_VA_Bottom_Ratio_Min : %3d%%,  BD_VA_Bottom_Ratio_Max :  %d%%\n\n"
    ,ST.Uniformity.uiBD_VA_Bottom_Ratio_Min,ST.Uniformity.uiBD_VA_Bottom_Ratio_Max);

    fprintf(fp, "BD_Top_XDiff_Caculate_Value :        0,  BD_Top_Ratio :     %3d%%\n",ST.Uniformity.uiBD_Top_Ratio);

    fprintf(fp, "BD_Bottom_XDiff_Caculate_Value :     0,  BD_Bottom_Ratio :  %3d%%\n",ST.Uniformity.uiBD_Bottom_Ratio);

    fprintf(fp, "BD_Left_YDiff_Caculate_Value :       0,  BD_L_Ratio :        %3d%%\n",ST.Uniformity.uiBD_L_Ratio);

    fprintf(fp, "BD_Right_YDiff_Caculate_Value:       0,  BD_R_Ratio :        %3d%%\n\n",ST.Uniformity.uiBD_R_Ratio);

    fprintf(fp, "VA_XDiff_Caculate_Value :            0,  VA_Ratio_X_diff :  %3d%%\n",ST.Uniformity.uiVA_Ratio_X_diff);

    fprintf(fp, "VA_YDiff_Caculate_Value :            0,  VA_Ratio_Y_diff :  %3d%%\n",ST.Uniformity.uiVA_Ratio_Y_diff);

    fprintf(fp, "PanelRange_LeftTop :         0 -    0,  PanelLeftTopLLimit:     %3d%%,  PanelLeftTopULimit:     %3d%%\n"
    ,ST.Uniformity.uiPanelLeftTopLLimit,ST.Uniformity.uiPanelLeftTopULimit);

    fprintf(fp, "PanelRange_RightTop :        0 -    0,  PanelRightTopLLimit:    %3d%%,  PanelRightTopULimit:    %3d%%\n"
    ,ST.Uniformity.uiPanelRightTopLLimit,ST.Uniformity.uiPanelRightTopULimit);

    fprintf(fp, "PanelRange_LeftBottom :      0 -    0,  PanelLeftBottomLLimit:  %3d%%,  PanelLeftBottomULimit:  %3d%%\n"
    ,ST.Uniformity.uiPanelLeftBottomLLimit,ST.Uniformity.uiPanelLeftBottomULimit);

    fprintf(fp, "PanelRange_RightBottom :     0 -    0,  PanelRightBottomLLimit: %3d%%,  PanelRightBottomULimit: %3d%%\n\n"
    ,ST.Uniformity.uiPanelRightBottomLLimit,ST.Uniformity.uiPanelRightBottomULimit);

    fprintf(fp, "   Y_Driven_Bench_____,");

    for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch; ucloopCh_X++)
    {
        fprintf(fp, "   X%02d  ,", ucloopCh_X);

    }
    fprintf(fp, "\n");


    for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch; ucloopCh_Y++)
    {
        fprintf(fp, "Y_Driven_Bench__Y%02d,", ucloopCh_Y);

        for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch; ucloopCh_X++)
        {
            fprintf(fp, "%5d   ,", ST.BenchMark.iUniformityBenchMark[ucloopCh_X][ucloopCh_Y]);
    
        }
        fprintf(fp, "\n");

    }
    fprintf(fp, "\n");


    fprintf(fp, "   Y_Driven_Data______,");

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
            fprintf(fp, "%5d   ,", ST.unifor_daltc[ucloopCh_X][ucloopCh_Y]);
    
        }
        fprintf(fp, "\n");

    }
    fprintf(fp, "\n");

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
			fprintf(fp, "%5d   ,", ST.open_Tx_diff[ucloopCh_X][ucloopCh_Y]);
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

void vfSaveOpenTestLog_V3(FILE *fp)
{
	unsigned char ucloopCh_X;
	unsigned char ucloopCh_Y;
    unsigned char ucloop;

    if ((ucSensorTestResult & OPEN_TEST) == OPEN_TEST)
    {
        fprintf(fp, "[Open_Test]              ,NG ,\n");
    }
    else
    {
        fprintf(fp, "[Open_Test]              ,OK ,\n");
    }

    fprintf(fp, "   Min_Threshold_______,%d\n", ST.Open_Threshold);
    if(ST.Open_RX_Delta_Threshold != 0)
        fprintf(fp, "   Open_Delta_RX_______,%d\n", ST.Open_RX_Delta_Threshold);
    if(ST.Open_TX_Delta_Threshold != 0)
        fprintf(fp, "   Open_Delta_TX_______,%d\n", ST.Open_TX_Delta_Threshold);
    if(ST.Open_TX_Aver_Diff != 0)
        fprintf(fp, "   TX_Average_Diff_Gap_,%d\n", ST.Open_TX_Aver_Diff);

    fprintf(fp, "TX_Average_Diff_Gap_AvoidCorner,%s\n", ST.Open_TX_Aver_Diff_Gap_Corner ? "True" : "False");

    fprintf(fp, "   TX_Average_Diff_Gap_,%d\n", ST.Open_TX_Aver_Diff);

    fprintf(fp, "   Open_Data____,");
    if (ptl.key_mode == ILITEK_HW_KEY_MODE) 
        fprintf(fp, "   Key_Threshold,%d\n", ST.Open.key_thr);

    for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch; ucloopCh_X++)
    {
        fprintf(fp, "   X%02d  ,", ucloopCh_X);

    }
    fprintf(fp, " Average,\n");

    for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch; ucloopCh_Y++)
    {
        fprintf(fp, "   Open_Data_Y%02d,", ucloopCh_Y);

        for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch; ucloopCh_X++)
        {
            fprintf(fp, "%5d   ,", ST.open_daltc[ucloopCh_X][ucloopCh_Y]);

        }
        fprintf(fp, "%5d   ,\n", ST.Tx_Avdiff_daltc[ucloopCh_Y].data);
    }
    if (ptl.key_mode == ILITEK_HW_KEY_MODE) {
        fprintf(fp,"Key_1         ,");
        for (ucloop = 0 ; ucloop < ptl.key_num; ucloop++)
            fprintf(fp, "%5d  ,", ST.Open.key_daltc[ucloop].data);
    }
    fprintf(fp, "\n");
}

void vfSaveAllNodeTestLog(FILE *fp)
{
	unsigned char ucloopCh_X;
	unsigned char ucloopCh_Y;

    if ((ucSensorTestResult & ALL_NODE_TEST) == ALL_NODE_TEST)
    {
        fprintf(fp, "[All_Node_Test]          ,NG ,\n");
    }
    else
    {
        fprintf(fp, "[All_Node_Test]          ,OK ,\n");
    }

    fprintf(fp, "   All_Node_Maximum______________,%d\n", ST.AllNode_Maximum);

    fprintf(fp, "   All_Node_Minimum______________,%d\n", ST.AllNode_Minimum);

    fprintf(fp, "   All_Node_Delta_Threshold______,%d\n", ST.AllNode_Delta_Threshold);

    fprintf(fp, "   All_Node_Panel_Tolerance______,%d\n", ST.AllNode_Panel_Tolerance);

    fprintf(fp, "   All_Node_TX_Tolerance_________,%d\n", ST.AllNode_TX_Tolerance);


   fprintf(fp, "\n   Data_____________________,");

   for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch; ucloopCh_X++)
   {
	   fprintf(fp, "  Rx%02d  ,", ucloopCh_X);

   }
   fprintf(fp, "\n");


   for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch; ucloopCh_Y++)
   {
	    fprintf(fp, "   Data_________________Tx%02d,", ucloopCh_Y);

		for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch; ucloopCh_X++)
		{
			fprintf(fp, "%5d   ,", ST.all_daltc[ucloopCh_X][ucloopCh_Y]);

		}
		fprintf(fp, "\n");

	}
    fprintf(fp, "\n");


   fprintf(fp, "\n   Delta_threshold_Data_____,");

   for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch - 1; ucloopCh_X++)
   {
	   fprintf(fp, "  Rx%02d  ,", ucloopCh_X);
   }
   fprintf(fp, "\n");

   for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch - 1; ucloopCh_Y++)
   {
	    fprintf(fp, "   Delta_threshold_Data_Tx%02d,", ucloopCh_Y);

		for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch - 1; ucloopCh_X++)
		{
			fprintf(fp, "%5d   ,", ST.all_w2_data[ucloopCh_X][ucloopCh_Y]);
		}
		fprintf(fp, "\n");
	}
    fprintf(fp, "\n");

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
void vfConverDataFormat()  //X Channel ï¿½ï¿½ Y Channelï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½SensorTestï¿½á¹¹ï¿½ï¿½Í¬
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
    int ucloopCh_X;
    int ucloopCh_Y;

    PRINTF("\n");
    PRINTF("Uniformity Test Criteria:\n");
    PRINTF("BD_Top_Ratio = %d\n",ST.Uniformity.uiBD_Top_Ratio);
    PRINTF("BD_Bottom_Ratio = %d\n",ST.Uniformity.uiBD_Bottom_Ratio);
    PRINTF("BD_L_Ratio = %d\n",ST.Uniformity.uiBD_L_Ratio);
    PRINTF("BD_R_Ratio = %d\n",ST.Uniformity.uiBD_R_Ratio);
    PRINTF("VA_Ratio_X_diff = %d\n",ST.Uniformity.uiVA_Ratio_X_diff);
    PRINTF("VA_Ratio_Y_diff = %d\n",ST.Uniformity.uiVA_Ratio_Y_diff);
    PRINTF("BD_VA_L_Ratio_Max = %d\n",ST.Uniformity.uiBD_VA_L_Ratio_Max);
    PRINTF("BD_VA_L_Ratio_Min = %d\n",ST.Uniformity.uiBD_VA_L_Ratio_Min);
    PRINTF("BD_VA_R_Ratio_Max = %d\n",ST.Uniformity.uiBD_VA_R_Ratio_Max);
    PRINTF("BD_VA_R_Ratio_Min = %d\n",ST.Uniformity.uiBD_VA_R_Ratio_Min);
    PRINTF("BD_VA_Top_Ratio_Max = %d\n",ST.Uniformity.uiBD_VA_Top_Ratio_Max);
    PRINTF("BD_VA_Top_Ratio_Min = %d\n",ST.Uniformity.uiBD_VA_Top_Ratio_Min);
    PRINTF("BD_VA_Bottom_Ratio_Max = %d\n",ST.Uniformity.uiBD_VA_Bottom_Ratio_Max);
    PRINTF("BD_VA_Bottom_Ratio_Min = %d\n",ST.Uniformity.uiBD_VA_Bottom_Ratio_Min);
    PRINTF("PanelLeftTopULimit = %d\n",ST.Uniformity.uiPanelLeftTopULimit);
    PRINTF("PanelLeftTopLLimit = %d\n",ST.Uniformity.uiPanelLeftTopLLimit);
    PRINTF("PanelLeftBottomULimit = %d\n",ST.Uniformity.uiPanelLeftBottomULimit);
    PRINTF("PanelLeftBottomLLimit = %d\n",ST.Uniformity.uiPanelLeftBottomLLimit);
    PRINTF("PanelRightTopULimit = %d\n",ST.Uniformity.uiPanelRightTopULimit);
    PRINTF("PanelRightTopLLimit = %d\n",ST.Uniformity.uiPanelRightTopLLimit);
    PRINTF("PanelRightBottomULimit = %d\n",ST.Uniformity.uiPanelRightBottomULimit);
    PRINTF("PanelRightBottomLLimit = %d\n\n",ST.Uniformity.uiPanelRightBottomLLimit);


    switch (KernelVersion[2])
    {
        case RDValue_MCUKernel_CDCVersion_8bit2315_:
        case RDValue_MCUKernel_CDCVersion_16bit2510_:
        {
            if (viInitRawData_3Para_3X(0x0C, 0xE6, 0x14) != _FAIL)
            {
                if (viGetRawData_3X(0xE6, _FastMode_, ptl.x_ch * ptl.y_ch, _DataFormat_16_Bit_, ptl.x_ch) != _FAIL)
                {
                    ret = _SUCCESS;
                }
                else
                {
                    PRINTF("Error! Get RawData Failed!\n");
                    ret = _FAIL;
                }
            }
            else
            {
                PRINTF("Error! Init RawData Failed!\n");
            }
                ret = _FAIL;
            break;
        }
    }

    if (ret != _FAIL)
    {
        PRINTF("Uniformity Datas: \n");
        for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch; ucloopCh_Y++)
        {
            PRINTF("Y_%2dCH:", ucloopCh_Y);
            for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch; ucloopCh_X++)
            {
                ST.unifor_daltc[ucloopCh_X][ucloopCh_Y] = uiTestDatas[ucloopCh_X][ucloopCh_Y];
                PRINTF("%4d,", ST.unifor_daltc[ucloopCh_X][ucloopCh_Y]);
            }
            PRINTF("\n");
        }

        vfConverDataFormat();
        inIndexCounts = 0;
        inSum = 0;
        for (ucloopCh_X = 1; ucloopCh_X < ptl.x_ch - 1; ucloopCh_X++)
        {
            inSum += ST.BenchMark.iUniformityBenchMark[ucloopCh_X][0];
            inIndexCounts++;
        }
        inBD_TopAvg = inSum / inIndexCounts;

        //------------Caculate inBD_BottomAvg 2315 New-------------------------
        inIndexCounts = 0;
        inSum = 0;
        for (ucloopCh_X = 1; ucloopCh_X < ptl.x_ch - 1; ucloopCh_X++)
        {
            inSum += ST.BenchMark.iUniformityBenchMark[ucloopCh_X][ptl.y_ch - 1];
            inIndexCounts++;
        }
        inBD_BottomAvg = inSum / inIndexCounts;

        //------------Caculate inBD_RightAvg 2315 New-------------------------
        inIndexCounts = 0;
        inSum = 0;
        for (ucloopCh_Y = 1; ucloopCh_Y < ptl.y_ch - 1; ucloopCh_Y++)
        {
            inSum += ST.BenchMark.iUniformityBenchMark[ptl.x_ch - 1][ucloopCh_Y];
            inIndexCounts++;
        }
        inBD_RightAvg = inSum / inIndexCounts;

        //------------Caculate inBD_LeftAvg 2315 New-------------------------
        inIndexCounts = 0;
        inSum = 0;
        for (ucloopCh_Y = 1; ucloopCh_Y < ptl.y_ch - 1; ucloopCh_Y++)
        {
            inSum += ST.BenchMark.iUniformityBenchMark[0][ucloopCh_Y];
            inIndexCounts++;
        }
        inBD_Left_Avg = inSum / inIndexCounts;


        //-------Caculate inVAAvg  2315 New -----------------------------------
        inSum = 0;
        inIndexCounts = 0;
        for (ucloopCh_X = 1; ucloopCh_X < ptl.x_ch - 1; ucloopCh_X++)
        {
            for (ucloopCh_Y = 1; ucloopCh_Y < ptl.y_ch - 1; ucloopCh_Y++)
            {
                inSum +=ST.BenchMark.iUniformityBenchMark[ucloopCh_X][ucloopCh_Y];
                inIndexCounts++;
            }
        }
        inVAAvg = inSum / inIndexCounts;

        //-------Check BD_Top --------------------------------------------------
        //str_2315Thresh.inBD_Top_XDiff = (int)(inBD_TopAvg * ST.Uniformity.uiBD_Top_Ratio * 0.01);

        for (ucloopCh_X = 2; ucloopCh_X < ptl.x_ch - 1; ucloopCh_X++)
        {
            inDeltaValue1= abs(uiTestDatas[ucloopCh_X][0] - uiTestDatas[ucloopCh_X - 1][0]);
            if(inDeltaValue1 > (int)(inBD_TopAvg * ST.Uniformity.uiBD_Top_Ratio * 0.01))
            {
                ucSensorTestResult |= UNIFORMITY_TEST;
                return ret;
            }
        }

        //-------Check BD_Bottom --------------------------------------------------
        //str_2315Thresh.inBD_BOttom_XDiff = (int)(inBD_BottomAvg * ST.Uniformity.uiBD_Bottom_Ratio * 0.01);

        for (ucloopCh_X = 2; ucloopCh_X < ptl.x_ch - 1; ucloopCh_X++)
        {
            inDeltaValue1= abs(uiTestDatas[ucloopCh_X][ptl.y_ch - 1] - uiTestDatas[ucloopCh_X - 1][ptl.y_ch - 1]);

            if(inDeltaValue1 > (int)(inBD_BottomAvg * ST.Uniformity.uiBD_Bottom_Ratio * 0.01))
            {
                ucSensorTestResult |= UNIFORMITY_TEST;
                return ret;
            }
        }

        //------------Check inBD_RightAvg 2315 New-------------------------
        //str_2315Thresh.inBD_Right_YDiff = (int)(inBD_RightAvg * ST.Uniformity.uiBD_R_Ratio * 0.01);

        for (ucloopCh_Y = 2; ucloopCh_Y < ptl.y_ch - 1; ucloopCh_Y++)
        {
            inDeltaValue1= abs(uiTestDatas[ptl.x_ch - 1][ucloopCh_Y] - uiTestDatas[ptl.x_ch - 1][ucloopCh_Y - 1]);

            if(inDeltaValue1 > (int)(inBD_RightAvg * ST.Uniformity.uiBD_R_Ratio * 0.01))
            {
                ucSensorTestResult |= UNIFORMITY_TEST;
                return ret;
            }
        }

        //------------Check inBD_LeftAvg 2315 New-------------------------
        //str_2315Thresh.inBD_Left_YDiff = (int)(inBD_Left_Avg * ST.Uniformity.uiBD_L_Ratio * 0.01);

        for (ucloopCh_Y = 2; ucloopCh_Y < ptl.y_ch - 1; ucloopCh_Y++)
        {
            inDeltaValue1= abs(uiTestDatas[0][ucloopCh_Y] - uiTestDatas[0][ucloopCh_Y - 1]);

            if(inDeltaValue1 > (int)(inBD_Left_Avg * ST.Uniformity.uiBD_L_Ratio * 0.01))
            {
                ucSensorTestResult |= UNIFORMITY_TEST;
                return ret;
            }
        }


        //-------Check inVAAvg Y_Diff 2315 New -----------------------------------
        //str_2315Thresh.inVA_YDiff = (int)(inVAAvg * ST.Uniformity.uiVA_Ratio_Y_diff * 0.01);

        for (ucloopCh_X = 1; ucloopCh_X < ptl.x_ch - 1; ucloopCh_X++)
        {

            for (ucloopCh_Y = 2; ucloopCh_Y < ptl.y_ch - 1; ucloopCh_Y++)
            {
                inDeltaValue1= abs(uiTestDatas[ucloopCh_X][ucloopCh_Y] - uiTestDatas[ucloopCh_X][ucloopCh_Y - 1]);

                if(inDeltaValue1 > (int)(inVAAvg * ST.Uniformity.uiVA_Ratio_Y_diff * 0.01))
                {
                    ucSensorTestResult |= UNIFORMITY_TEST;
                    return ret;
                }
            }

        }

        //-------Check inVAAvg X_Diff 2315 New -----------------------------------
        //str_2315Thresh.inVA_XDiff =  (int)(inVAAvg * ST.Uniformity.uiVA_Ratio_X_diff * 0.01);

        for (ucloopCh_Y = 1; ucloopCh_Y < ptl.y_ch- 1; ucloopCh_Y++)
        {
            for (ucloopCh_X = 2; ucloopCh_X < ptl.x_ch - 1; ucloopCh_X++)
            {
                inDeltaValue1= abs(uiTestDatas[ucloopCh_X][ucloopCh_Y] - uiTestDatas[ucloopCh_X - 1][ucloopCh_Y]);

                if(inDeltaValue1 > (int)(inVAAvg * ST.Uniformity.uiVA_Ratio_X_diff * 0.01))
                {
                    ucSensorTestResult |= UNIFORMITY_TEST;
                    return ret;
                }
            }

        }

        //-------Check BD/VA_Top --------------------------------------------------

        for (ucloopCh_X = 1; ucloopCh_X < ptl.x_ch - 1; ucloopCh_X++)
        {
            inDeltaValue1= ((1.0 * uiTestDatas[ucloopCh_X][0] / uiTestDatas[ucloopCh_X][1]) * 100);

            if(inDeltaValue1 > ST.Uniformity.uiBD_VA_Top_Ratio_Max ||
                    inDeltaValue1 < ST.Uniformity.uiBD_VA_Top_Ratio_Min)
            {
                ucSensorTestResult |= UNIFORMITY_TEST;
                return ret;
            }
        }

        //-------Check BD/VA_Bottom --------------------------------------------------

        for (ucloopCh_X = 1; ucloopCh_X < ptl.x_ch - 1; ucloopCh_X++)
        {
            inDeltaValue1= (int)((1.0 * uiTestDatas[ucloopCh_X][ptl.y_ch - 1] / uiTestDatas[ucloopCh_X][ptl.y_ch - 2]) * 100);

            if(inDeltaValue1 > ST.Uniformity.uiBD_VA_Bottom_Ratio_Max ||
                    inDeltaValue1 < ST.Uniformity.uiBD_VA_Bottom_Ratio_Min)
            {
                ucSensorTestResult |= UNIFORMITY_TEST;
                return ret;
            }
        }

        //------------Check BD/VA Right ------------------------------------------------

        for (ucloopCh_Y = 1; ucloopCh_Y < ptl.y_ch - 1; ucloopCh_Y++)
        {
            inDeltaValue1= (int)((1.0 * uiTestDatas[ptl.x_ch - 1][ucloopCh_Y] / uiTestDatas[ptl.x_ch - 2][ucloopCh_Y]) * 100);

            if(inDeltaValue1 > ST.Uniformity.uiBD_VA_R_Ratio_Max ||
                    inDeltaValue1 < ST.Uniformity.uiBD_VA_R_Ratio_Min)
            {
                ucSensorTestResult |= UNIFORMITY_TEST;
                return ret;
            }
        }

        //------------Check BD/VA Left ------------------------------------------------

        for (ucloopCh_Y = 1; ucloopCh_Y < ptl.y_ch - 1; ucloopCh_Y++)
        {
            inDeltaValue1= (int)((1.0 * uiTestDatas[0][ucloopCh_Y] / uiTestDatas[1][ucloopCh_Y]) * 100);

            if(inDeltaValue1 > ST.Uniformity.uiBD_VA_L_Ratio_Max ||
                    inDeltaValue1 < ST.Uniformity.uiBD_VA_L_Ratio_Min)
            {
                ucSensorTestResult |= UNIFORMITY_TEST;
                return ret;
            }
        }

        //-----------Check Corner----------------------------------------------------------
        //str_2315Thresh.inCorner_LT_Max = (int)(ST.BenchMark.iUniformityBenchMark[0][0] * ST.Uniformity.uiPanelLeftTopULimit * 0.01);
        //str_2315Thresh.inCorner_LT_Min = (int)(ST.BenchMark.iUniformityBenchMark[0][0] * ST.Uniformity.uiPanelLeftTopLLimit * 0.01);

        if(uiTestDatas[0][0] > (int)(ST.BenchMark.iUniformityBenchMark[0][0] * ST.Uniformity.uiPanelLeftTopULimit * 0.01)
                ||uiTestDatas[0][0] < (int)(ST.BenchMark.iUniformityBenchMark[0][0] * ST.Uniformity.uiPanelLeftTopLLimit * 0.01))
        {
            ucSensorTestResult |= UNIFORMITY_TEST;
            return ret;
        }

        //str_2315Thresh.inCorner_LB_Max = (int)(ST.BenchMark.iUniformityBenchMark[0][ptl.y_ch - 1] * ST.Uniformity.uiPanelLeftBottomULimit * 0.01);
        //str_2315Thresh.inCorner_LB_Min = (int)(ST.BenchMark.iUniformityBenchMark[0][ptl.y_ch - 1] * ST.Uniformity.uiPanelLeftBottomLLimit * 0.01);

        if(uiTestDatas[0][ptl.y_ch - 1] > (int)(ST.BenchMark.iUniformityBenchMark[0][ptl.y_ch - 1] * ST.Uniformity.uiPanelLeftBottomULimit * 0.01)
                ||uiTestDatas[0][ptl.y_ch - 1] < (int)(ST.BenchMark.iUniformityBenchMark[0][ptl.y_ch - 1] * ST.Uniformity.uiPanelLeftBottomLLimit * 0.01))
        {
            ucSensorTestResult |= UNIFORMITY_TEST;
            return ret;
        }

        // str_2315Thresh.inCorner_RT_Max = (int)(ST.BenchMark.iUniformityBenchMark[ptl.x_ch - 1][0] * ST.Uniformity.uiPanelRightTopULimit * 0.01);
        // str_2315Thresh.inCorner_RT_Min = (int)(ST.BenchMark.iUniformityBenchMark[ptl.x_ch - 1][0] * ST.Uniformity.uiPanelRightTopLLimit * 0.01);

        if(uiTestDatas[ptl.x_ch - 1][0] > (int)(ST.BenchMark.iUniformityBenchMark[ptl.x_ch - 1][0] * ST.Uniformity.uiPanelRightTopULimit * 0.01)
                ||uiTestDatas[ptl.x_ch - 1][0] < (int)(ST.BenchMark.iUniformityBenchMark[ptl.x_ch - 1][0] * ST.Uniformity.uiPanelRightTopLLimit * 0.01))
        {
            ucSensorTestResult |= UNIFORMITY_TEST;
            return ret;
        }

        //str_2315Thresh.inCornew_RB_Max = (int)(ST.BenchMark.iUniformityBenchMark[ptl.x_ch - 1][ptl.y_ch - 1] * ST.Uniformity.uiPanelRightBottomULimit * 0.01);
        //str_2315Thresh.inCornew_RB_Min = (int)(ST.BenchMark.iUniformityBenchMark[ptl.x_ch - 1][ptl.y_ch - 1] * ST.Uniformity.uiPanelRightBottomLLimit * 0.01);

        if(uiTestDatas[ptl.x_ch - 1][ptl.y_ch - 1] > (int)(ST.BenchMark.iUniformityBenchMark[ptl.x_ch - 1][ptl.y_ch - 1] * ST.Uniformity.uiPanelRightBottomULimit * 0.01)
                ||uiTestDatas[ptl.x_ch - 1][ptl.y_ch - 1] < (int)(ST.BenchMark.iUniformityBenchMark[ptl.x_ch - 1][ptl.y_ch - 1] * ST.Uniformity.uiPanelRightBottomLLimit * 0.01))
        {
            ucSensorTestResult |= UNIFORMITY_TEST;
            return ret;
        }
    }
    return ret;
}

int viRunAllNodeTest_3X()
{
    int ret = 1;
    int CHX = 0, CHY = 0;
    unsigned char ucPass = 1;
    int PanelFailCount = 0;
    int TXFailCount = 0;
    int TXisFail = 0;
	unsigned char u8DataInfor = _DataFormat_16_Bit_;
    switch (KernelVersion[2])
    {
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
    printf("\n");
    PRINTF("All Node Test Criteria:\n");
    PRINTF("Maximum = %d\n",ST.AllNode_Maximum);
    PRINTF("Minimum = %d\n",ST.AllNode_Minimum);
    PRINTF("Delta Threshold = %d\n",ST.AllNode_Delta_Threshold);
    PRINTF("Panel Tolerance = %d\n",ST.AllNode_Panel_Tolerance);
    PRINTF("TX Tolerance = %d\n\n",ST.AllNode_TX_Tolerance);
    switch (KernelVersion[2])
    {
        case RDValue_MCUKernel_CDCVersion_8bit2315_:
        case RDValue_MCUKernel_CDCVersion_16bit2510_:
        {
            if(ptl.ver < PROTOCOL_V3_4_0)
                ret = viInitRawData_3X(0x0B, 0xE6);
            else
                ret = viInitRawData_3Para_3X(0x0B, 0xE6, 0x00);
            if (ret != _FAIL)
            {
                ret = RawDataInfor();
                u8DataInfor = (ret & (1 << 26)) ? _DataFormat_16_Bit_: _DataFormat_8_Bit_;
                if (viGetRawData_3X(0xE6, _FastMode_, ptl.x_ch * ptl.y_ch, u8DataInfor, ptl.x_ch) != _FAIL)
                {
                    ret = _SUCCESS;
                }
                else
                {
                    PRINTF("Error! Get RawData Failed!\n");
                    ret = _FAIL;
                }
            }
            else
            {
                PRINTF("Error! Init RawData Failed!\n");
                ret = _FAIL;
            }
            break;
        }
    }
    if (ret != _FAIL)
    {
        PRINTF("All Node Datas: \n       ");
        for (CHX = 0; CHX < ptl.x_ch; CHX++)
        {
            printf("X_%02d,", CHX);
        }
        printf("\n");
        for (CHY = 0; CHY < ptl.y_ch; CHY++)
        {
            PRINTF("Y_%2dCH:", CHY);
            for (CHX = 0; CHX < ptl.x_ch; CHX++)
            {
                printf("%4d,", uiTestDatas[CHY][CHX]);
                ST.all_daltc[CHX][CHY] = uiTestDatas[CHY][CHX];
                if (uiTestDatas[CHX][CHY] > ST.AllNode_Maximum || uiTestDatas[CHY][CHX] < ST.AllNode_Minimum)
                {
                    ucPass = 0;
                }
            }
            printf("\n");
        }
        printf("\n       ");
        for (CHX = 0; CHX < ptl.x_ch - 1; CHX++)
        {
            printf("X_%02d,", CHX);
        }
        printf("\n");
        for (CHY = 1; CHY < ptl.y_ch; CHY++)
        {
            PRINTF("Y_%2dCH:", CHY - 1);
            TXFailCount = 0;
            for (CHX = 1; CHX < ptl.x_ch; CHX++)
            {
                ST.all_w2_data[CHX-1][CHY-1] = abs((uiTestDatas[CHY][CHX] - uiTestDatas[CHY - 1][CHX]) -
                        (uiTestDatas[CHY][CHX - 1] - uiTestDatas[CHY - 1][CHX - 1]));
                printf("%4d,", ST.all_w2_data[CHX-1][CHY-1]);
                if (ST.all_w2_data[CHX-1][CHY-1] > ST.AllNode_Delta_Threshold)
                {
                    ++TXFailCount;
                    ++PanelFailCount;
                }
            }
            if (TXFailCount >= ST.AllNode_TX_Tolerance)
            {
                TXisFail = 1;
            }
            printf("\n");
        }
        if (TXisFail == 1 && PanelFailCount >= ST.AllNode_Panel_Tolerance)
        {
            ucPass = 0;
        }

        if (ucPass == 1)
        {
        }
        else
        {
            ucSensorTestResult |= ALL_NODE_TEST;
        }
    }
    return ret;
}

int viRunOpenTest_3X_NewFlow()
{
	int ret = _SUCCESS;
	int inDeltaValue1;
	int inDiffAvg,inX_DiffAvg,inY_DiffAvg;
	//int inDiffThreshold,inX_DiffThreshold,inY_DiffThreshold,
	int inIndexCounts;
	int inSum;
	int inTempCheckValue;
    int xdiff_fail_count = 0, ydiff_fail_count = 0;
	int ucloopCh_X;
	int ucloopCh_Y;
	printf("\n");
	PRINTF("Open Test Criteria:\n");
	PRINTF("Open Test 20V-6V Threshold = %d\n",ST.Open_Threshold);
	PRINTF("Open Test 20V-6V X Diff Threshold = %d\n",ST.Open_RX_Delta_Threshold);
    PRINTF("Open Test 20V-6V X Diff Fail count = %d\n",ST.Open_RX_Continue_Fail_Tolerance);
	PRINTF("Open Test 20V-6V Y Diff Threshold = %d\n",ST.Open_TX_Delta_Threshold);
    PRINTF("Open Test 20V-6V Y Diff Fail count = %d\n",ST.Open_TX_Continue_Fail_Tolerance);
	PRINTF("Open_DCRangeMax = %d\n",ST.Open_DCRangeMax);
	PRINTF("Open_DCRangeMin = %d\n\n",ST.Open_DCRangeMin);

	switch (KernelVersion[2])
	{
		case RDValue_MCUKernel_CDCVersion_8bit2315_:
		case RDValue_MCUKernel_CDCVersion_16bit2510_:
		{
			if (viInitRawData_3Para_3X(0x0C, 0xE6, 0x06) != _FAIL)
			{
				if (viGetRawData_3X(0xE6, _FastMode_, ptl.x_ch * ptl.y_ch, _DataFormat_16_Bit_, ptl.x_ch) != _FAIL)
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
					PRINTF("Error! Get RawData Failed!\n");
					ret = _FAIL;
				}
			}
			else
			{
				PRINTF("Error! Init RawData Failed!\n");
				ret = _FAIL;
			}

			if (viInitRawData_3Para_3X(0x0C, 0xE6, 0x14) != _FAIL)
			{
				if (viGetRawData_3X(0xE6, _FastMode_, ptl.x_ch * ptl.y_ch, _DataFormat_16_Bit_, ptl.x_ch) != _FAIL)
				{
					vfConverDataFormat();
                    ret = _SUCCESS;
				}
				else
				{
					PRINTF("Error! Get RawData Failed!\n");
					ret = _FAIL;
				}
			}
			else
			{
				PRINTF("Error! Init RawData Failed!\n");
				ret = _FAIL;
			}
			break;
		}
	}
	if (ret != _FAIL)
	{
		PRINTF("Open Datas_20V: \n");
		for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch; ucloopCh_Y++)
		{
			PRINTF("Y_%2dCH:", ucloopCh_Y);
			for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch; ucloopCh_X++)
			{
                ST.open_20V_daltc[ucloopCh_X][ucloopCh_Y] = uiTestDatas[ucloopCh_X][ucloopCh_Y];
				printf("%4d,", uiTestDatas[ucloopCh_X][ucloopCh_Y]);
			}
			printf("\n");
		}

		PRINTF("Open Datas_6V: \n");
		for (ucloopCh_Y = 0; ucloopCh_Y < ptl.y_ch; ucloopCh_Y++)
		{
			PRINTF("Y_%2dCH:", ucloopCh_Y);
			for (ucloopCh_X = 0; ucloopCh_X < ptl.x_ch; ucloopCh_X++)
			{
				printf("%4d,", uiTestDatas_1[ucloopCh_X][ucloopCh_Y]);
                ST.open_6V_daltc[ucloopCh_X][ucloopCh_Y] = uiTestDatas_1[ucloopCh_X][ucloopCh_Y];
			}
			printf("\n");
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
		inX_DiffAvg = (int)(1.0 * inSum / inIndexCounts);

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
		inY_DiffAvg = (int)(1.0 * inSum / inIndexCounts);
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
                    //PRINTF("%s,%d, ucSensorTestResult = %d\n", __func__, __LINE__, ucSensorTestResult);
				 }
			}
		}
	PRINTF("Open Test 20V-6V Threshold = %d\n",ST.Open_Threshold);
	PRINTF("Open Test 20V-6V X Diff Threshold = %d\n",ST.Open_RX_Delta_Threshold);
    PRINTF("Open Test 20V-6V X Diff Fail count = %d\n",ST.Open_RX_Continue_Fail_Tolerance);
	PRINTF("Open Test 20V-6V Y Diff Threshold = %d\n",ST.Open_TX_Delta_Threshold);
    PRINTF("Open Test 20V-6V X Diff Fail count = %d\n",ST.Open_TX_Continue_Fail_Tolerance);
	PRINTF("Open_DCRangeMax = %d\n",ST.Open_DCRangeMax);
	PRINTF("Open_DCRangeMin = %d\n\n",ST.Open_DCRangeMin);
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
                        //PRINTF("%s,%d, ucSensorTestResult = %d\n", __func__, __LINE__, ucSensorTestResult);
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
				ST.open_Tx_diff[ucloopCh_X][ucloopCh_Y] = inDeltaValue1;
				//20190314 end
				 if(inDeltaValue1 > (int)(inDiffAvg * inTempCheckValue * 0.01))
				 {
                    ydiff_fail_count++;
                    if(ST.Open_TX_Continue_Fail_Tolerance <= ydiff_fail_count)
                    {
                        ucSensorTestResult |= OPEN_TEST;
                        //PRINTF("%s,%d, ucSensorTestResult = %d\n", __func__, __LINE__, ucSensorTestResult);
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
				ST.uiYDC_Range[ucloopCh_X][ucloopCh_Y] = uiTestDatas[ucloopCh_X][ucloopCh_Y];
				//20190314 end
				 if(uiTestDatas[ucloopCh_X][ucloopCh_Y] > (int)(ST.BenchMark.iOpenBenchMark_0[ucloopCh_X][ucloopCh_Y ] * ST.Open_DCRangeMax * 0.01)
					|| uiTestDatas[ucloopCh_X][ucloopCh_Y] < (int)(ST.BenchMark.iOpenBenchMark_0[ucloopCh_X][ucloopCh_Y ] * ST.Open_DCRangeMin * 0.01))
				 {
                    //  PRINTF("%s,%d, ucSensorTestResult = %d,data[%d][%d] = %d, golden[%d][%d] = %d,%d,%d\n", __func__, __LINE__, ucSensorTestResult, ucloopCh_X, ucloopCh_Y
                    //  , uiTestDatas[ucloopCh_X][ucloopCh_Y], ucloopCh_X, ucloopCh_Y, ST.BenchMark.iOpenBenchMark_0[ucloopCh_X][ucloopCh_Y],
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
    int count = 0;

    PRINTF("Key_Threshold = %d\n\n", ST.Open.key_thr);
    PRINTF("Open Key Datas\n       ");
    for (count = 0; count < ptl.key_num; count++)
        PRINTF("%6d,", count + 1);
    if(viGetCDCData_6X(TEST_MODE_OPEN_KEY, ptl.key_num) < 0)
        return _FAIL;
    PRINTF("\n  data:");
    for(count = 0; count < ptl.key_num; count++) {
        ST.Open.key_daltc[count].data = uiTestDatas[count/ptl.x_ch][count%ptl.x_ch];
        if(ST.Open.key_daltc[count].data < ST.Open.key_thr) {
            PRINTF("*%5d,", ST.Open.key_daltc[count].data);
            ST.Open.key_daltc[count].status = NODE_FAIL;
            ret = _FAIL;
        }
        else {
            PRINTF("%6d,", ST.Open.key_daltc[count].data);
            ST.Open.key_daltc[count].status = NODE_PASS;
        }
    }
    PRINTF("\n");

    return ret;
}

int OpenTestThreshold() {
    int CHX = 0, CHY = 0;
    int ret = _SUCCESS;

    PRINTF("Open Datas: \n       ");
    for (CHX = 0; CHX < ptl.x_ch; CHX++) {
        PRINTF(" %3dCH,", CHX);
    }
    PRINTF("\n");
    for (CHY = 0; CHY < ptl.y_ch; CHY++)
    {
        PRINTF("Y_%2dCH:", CHY);
        for (CHX = 0; CHX < ptl.x_ch; CHX++)
        {
            ST.v6_open_daltc[CHX][CHY].data = uiTestDatas[CHY][CHX];
            if (ST.v6_open_daltc[CHX][CHY].data < ST.Open_Threshold)
            {
                ST.v6_open_daltc[CHX][CHY].status = NODE_FAIL;
                printf("*%5d,", ST.v6_open_daltc[CHX][CHY].data);
                ret = _FAIL;
            }
            else {
                ST.v6_open_daltc[CHX][CHY].status = NODE_PASS;
                printf("%6d,", ST.v6_open_daltc[CHX][CHY].data);
            }
        }
        printf("\n");
    }

    return ret;
}

int OpenTestRxDiff() {
    int CHX = 0, CHY = 0;
    int ret = _SUCCESS;
    int RXFailCount = 0;
    int threshold = 0;
    int diff = 0;
    int Tolerance = 0;

    PRINTF("Rx Diff: \n       ");
    for (CHX = 0; CHX < ptl.x_ch - 1; CHX++) {
        PRINTF(" %3dCH,", CHX);
    }
    PRINTF("\n");
    for (CHY = 0; CHY < ptl.y_ch; CHY++)
    {
        PRINTF("Y_%2dCH:", CHY);
        for (CHX = 1; CHX < ptl.x_ch; CHX++)
        {
            if(ST.MOpen.RxDeltaEn == ENABLE_TEST) {
                threshold = ST.MOpen.rx_diff[CHX - 1][CHY].ini.max;
                diff = abs(uiTestDatas[CHY][CHX] - uiTestDatas[CHY][CHX - 1]);
                ST.MOpen.rx_diff[CHX - 1][CHY].raw.data = diff;
                Tolerance = ST.MOpen.RxToles;
                if (diff > threshold)
                {
                    ++RXFailCount;
                    ST.MOpen.rx_diff[CHX - 1][CHY].raw.status = NODE_FAIL;
                    printf("*%5d,", diff);
                }
                else
                {
                    ST.MOpen.rx_diff[CHX - 1][CHY].raw.status = NODE_PASS;
                    printf("%6d,", diff);
                }
                if (RXFailCount >= Tolerance)
                {
                    ret = _FAIL;
                }
            }
            else {
                ST.open_Rx_diff[CHX - 1][CHY].data = abs(uiTestDatas[CHY][CHX] - uiTestDatas[CHY][CHX - 1]);
                if (ST.open_Rx_diff[CHX - 1][CHY].data > ST.Open_RX_Delta_Threshold)
                {
                    ++RXFailCount;
                    ST.open_Rx_diff[CHX - 1][CHY].status = NODE_FAIL;
                    printf("*%5d,", ST.open_Rx_diff[CHX - 1][CHY].data);
                }
                else
                {
                    ST.open_Rx_diff[CHX - 1][CHY].status = NODE_PASS;
                    printf("%6d,", ST.open_Rx_diff[CHX - 1][CHY].data);
                }
                if (RXFailCount >= ST.Open_RX_Continue_Fail_Tolerance)
                {
                    ret = _FAIL;
                }
            }
        }
        printf("\n");
    }

    return ret;
}

int OpenTestTxAverageTest() {
    int CHX = 0, CHY = 0;
    int ret = _SUCCESS;
    int count = 0;
    int threshold = 0;
    int channelAvg = 0;

    for (CHY = 0; CHY < ptl.y_ch; CHY++)
    {
        count = 0;
        ST.Tx_Avdiff_daltc[CHY].data = 0;
        for (CHX = 0; CHX < ptl.x_ch; CHX++)
        {
            if(ST.Open_TX_Aver_Diff_Gap_Corner) {
                //No corner calculations
                if(CHY == 0 && (CHX == 0 || CHX == ptl.x_ch - 1)) {
                    //PRINTF("No corner calculations:[%d][%d]\n", CHX,CHY);
                    continue;
                }
                if(CHY == ptl.y_ch - 1 && (CHX == 0 || CHX == ptl.x_ch - 1)) {
                    //PRINTF("No corner calculations:[%d][%d]\n", CHX,CHY);
                    continue;
                }
            }
            count++;
            ST.Tx_Avdiff_daltc[CHY].data = ST.Tx_Avdiff_daltc[CHY].data + uiTestDatas[CHY][CHX];
        }
        //PRINTF("avdiff[%d] total:%d\n", CHY, ST.Tx_Avdiff_daltc[CHY].data);
        ST.Tx_Avdiff_daltc[CHY].data = ST.Tx_Avdiff_daltc[CHY].data / count;
        //PRINTF("avdiff[%d] %d\n", CHY, ST.Tx_Avdiff_daltc[CHY].data);
    }

    for (CHY = 1; CHY < ptl.y_ch; CHY++)
    {
        if(ST.MOpen.TxAvgEn == ENABLE_TEST) {
            threshold = ST.MOpen.tx_avg[0][CHY - 1].ini.max;
            ST.MOpen.tx_avg[0][CHY - 1].raw.data = abs(ST.Tx_Avdiff_daltc[CHY].data - ST.Tx_Avdiff_daltc[CHY - 1].data);
            channelAvg = ST.MOpen.tx_avg[0][CHY - 1].raw.data;
            if(channelAvg > threshold) {
                ST.MOpen.tx_avg[0][CHY - 1].raw.status = NODE_FAIL;
                ret = _FAIL;
            }
            //PRINTF("[%d]thr:%d, data:%d\n", CHY - 1, threshold, channelAvg);
        }
        else {
            threshold = ST.Open_TX_Aver_Diff;
            channelAvg = abs(ST.Tx_Avdiff_daltc[CHY].data - ST.Tx_Avdiff_daltc[CHY - 1].data);
            if(channelAvg > threshold) {
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
    int inCounts = 0;
    int CHX = 0, CHY = 0;
    int TXFailCount = 0;
    bool TxAverageStatus = _SUCCESS;
    bool RxDiffStatus = _SUCCESS;

    if (ST.Open_Threshold == -1)
    {
        ST.Open_Threshold = _SensorTestOpenThreshold_;
    }
    if (ST.Open_RX_Delta_Threshold == -1)
    {
        ST.Open_RX_Delta_Threshold = _SensorTestOpenDeltaRX_;
    }
    if (ST.Open_TX_Delta_Threshold == -1)
    {
        ST.Open_TX_Delta_Threshold = _SensorTestOpenDeltaTX_;
    }
    if (ST.Open_RX_Continue_Fail_Tolerance == -1)
    {
        ST.Open_RX_Continue_Fail_Tolerance = _SensorTestOpenRXTolerance_;
    }
    if (ST.Open_TX_Continue_Fail_Tolerance == -1)
    {
        ST.Open_TX_Continue_Fail_Tolerance = _SensorTestOpenTXTolerance_;
    }
    if (ST.Open_TX_Aver_Diff == -1)
    {
        ST.Open_TX_Aver_Diff = _SensorTestOpenTxAverDiff_;
    }
    printf("\n");
    PRINTF("Open Test Criteria:\n");
    PRINTF("Threshold = %d\n",ST.Open_Threshold);
    PRINTF("RX Delta Threshold = %d\n",ST.Open_RX_Delta_Threshold);
    PRINTF("TX Delta Threshold = %d\n\n",ST.Open_TX_Delta_Threshold);
    PRINTF("Open_RX_Continue_Fail_Tolerance = %d\n\n",ST.Open_RX_Continue_Fail_Tolerance);
    PRINTF("Open_TX_Continue_Fail_Tolerance = %d\n\n",ST.Open_TX_Continue_Fail_Tolerance);
    PRINTF("TX_Average_Diff_Gap = %d\n\n",ST.Open_TX_Aver_Diff);
    if (ptl.key_mode == ILITEK_HW_KEY_MODE) {
        PRINTF("Key_Threshold = %d\n\n", ST.Open.key_thr);
    }
    switch (KernelVersion[2])
    {
        case RDValue_MCUKernel_CDCVersion_10bit2301_:
        case RDValue_MCUKernel_CDCVersion_08bit2301_:
        {
            if (viInitRawData_3X(0x0C, 0xE6) != _FAIL)
            {
                if (viGetRawData_3X(0xE6, _FastMode_, ptl.x_ch * ptl.y_ch + ptl.key_num, _DataFormat_8_Bit_, ptl.x_ch) != _FAIL)
                {
                    ret = _SUCCESS;
                }
                else
                {
                    PRINTF("Error! Get RawData Failed!\n");
                    ret = _FAIL;
                }
            }
            else
            {
                PRINTF("Error! Init RawData Failed!\n");
                ret = _FAIL;
            }
            break;
        }
        case RDValue_MCUKernel_CDCVersion_8bit2315_:
        case RDValue_MCUKernel_CDCVersion_16bit2510_:
        {
            if (viInitRawData_3X(0x0C, 0xE6) != _FAIL)
            {
                if (viGetRawData_3X(0xE6, _FastMode_, ptl.x_ch * ptl.y_ch + ptl.key_num, _DataFormat_16_Bit_, ptl.x_ch) != _FAIL)
                {
                    ret = _SUCCESS;
                }
                else
                {
                    PRINTF("Error! Get RawData Failed!\n");
                    ret = _FAIL;
                }
            }
            else
            {
                PRINTF("Error! Init RawData Failed!\n");
                ret = _FAIL;
            }
            break;
        }
    }

    if(OpenTestTxAverageTest() == _FAIL) {
        TxAverageStatus = _FAIL;
    }

    if (ret != _FAIL)
    {
        PRINTF("Open Datas: \n       ");
        for (CHX = 0; CHX < ptl.x_ch; CHX++) {
            PRINTF(" %3dCH,", CHX);
        }
        PRINTF("Average,\n");
        for (CHY = 0; CHY < ptl.y_ch; CHY++)
        {
            PRINTF("Y_%2dCH:", CHY);
            for (CHX = 0; CHX < ptl.x_ch; CHX++)
            {
                printf("%6d,", uiTestDatas[CHY][CHX]);
                ST.open_daltc[CHX][CHY] = uiTestDatas[CHY][CHX];
                if (uiTestDatas[CHY][CHX] < ST.Open_Threshold)
                {
                    ret = _FAIL;
                }
            }
            PRINTF("%7d,", ST.Tx_Avdiff_daltc[CHY].data);
            PRINTF("\n");
        }

        for (CHY = 1; CHY < ptl.y_ch; CHY++)
        {
            for (CHX = 0; CHX < ptl.x_ch; CHX++)
            {
                if (abs(uiTestDatas[CHY][CHX] - uiTestDatas[CHY - 1][CHX]) > ST.Open_TX_Delta_Threshold)
                {
                    ++TXFailCount;
                }
                else
                {
                    TXFailCount = 0;
                }
                if (TXFailCount >= ST.Open_TX_Continue_Fail_Tolerance)
                {
                    ret = _FAIL;
                }
            }
        }
        if (ptl.key_mode == ILITEK_HW_KEY_MODE) {
            PRINTF("Open Key Datas\n       ");
            for (inCounts = 0; inCounts < ptl.key_num; inCounts++)
                PRINTF("%6d,", inCounts + 1);
            PRINTF("\n  data:");
            for(inCounts = 0; inCounts < ptl.key_num; inCounts++) {
                ST.Open.key_daltc[inCounts].data = uiTestDatas[ptl.y_ch][inCounts];
                if(ST.Open.key_daltc[inCounts].data < ST.Open.key_thr) {
                    PRINTF("*%5d,", ST.Open.key_daltc[inCounts].data);
                    ST.Open.key_daltc[inCounts].status = NODE_FAIL;
                    ret = _FAIL;
                }
                else {
                    PRINTF("%6d,", ST.Open.key_daltc[inCounts].data);
                    ST.Open.key_daltc[inCounts].status = NODE_PASS;
                }
            }
            PRINTF("\n");
        }

        if(OpenTestRxDiff() == _FAIL) {
            RxDiffStatus = _FAIL;
        }

        if(TxAverageStatus == _FAIL || RxDiffStatus == _FAIL) {
            ret = _FAIL;
        }
    }
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
	printf("\n");
	PRINTF("Short Test Criteria:\n");
	PRINTF("Threshold = %d\n",ST.Short.Threshold);
    PRINTF("Dump1=0x%x\n", ST.Short.dump1);
    PRINTF("Dump2=0x%x\n", ST.Short.dump2);
    PRINTF("Vref=%0.1f\n", ST.Short.vref_v);
    PRINTF("Postidle_L=0x%x\n", ST.Short.posidleL);

    ucLineLenth = ptl.x_ch;
    ModeCtrl_V6(ENTER_TEST_MODE, DISABLE_ENGINEER);
    if(SetShortInfo(ST.Short.dump1, ST.Short.dump2, ST.Short.vref_s, ST.Short.posidleL, ST.Short.posidleH) < 0) {
        PRINTF("Error! Set Short info Failed!\n");
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
        PRINTF("KeyRX_Threshold=%d\n", ST.Short.keyRx_thr);
        PRINTF("KeyTX_Threshold=%d\n", ST.Short.keyTx_thr);
        ucLineLenth = ptl.key_num + 1;
        if(viGetCDCData_6X(TEST_MODE_V6_SHORT_KEY, ucLineLenth) < 0)
            return _FAIL;

        for (ucIndex = 0; ucIndex < ucLineLenth; ucIndex++)
        {
            ST.Short.key_daltc[ucIndex].data = uiTestDatas[ucIndex/ptl.x_ch][ucIndex%ptl.x_ch];
        }
        PRINTF("Short Key Datas:\n       ");
        for (ucIndex = 0; ucIndex < ptl.key_num; ucIndex++)
            PRINTF("%6d,", ucIndex + 1);
        PRINTF("Self X:\n       ");
        for (ucIndex = 0; ucIndex < ptl.key_num; ucIndex++) {
            if(ST.Short.key_daltc[ucIndex].data > ST.Short.keyRx_thr) {
                ret = _FAIL;
                PRINTF("*%5d,", ST.Short.key_daltc[ucIndex].data);
                ST.Short.key_daltc[ucIndex].status = NODE_FAIL;
            }
            else {
                PRINTF("%6d,", ST.Short.key_daltc[ucIndex].data);
                ST.Short.key_daltc[ucIndex].status = NODE_PASS;
            }

        }
        if(ST.Short.key_daltc[ptl.key_num].data > ST.Short.keyTx_thr) {
            ret = _FAIL;
            ST.Short.key_daltc[ptl.key_num].status = NODE_FAIL;
            PRINTF("\nSelf Y:*%5d,\n:", ST.Short.key_daltc[ptl.key_num].data);
        }
        else {
            ST.Short.key_daltc[ptl.key_num].status = NODE_PASS;
            PRINTF("\nSelf Y:%6d,\n", ST.Short.key_daltc[ptl.key_num].data);
        }
    }
    PRINTF("\nShort Datas:\n");
    PRINTF("X_CH:");
    for (ucIndex = 0; ucIndex < ptl.x_ch; ucIndex++)
    {
        if(ST.v6_short_daltc[0][ucIndex].data > ST.Short.Threshold) {
            ST.v6_short_daltc[0][ucIndex].status = NODE_FAIL;
            printf(" *%4d,", ST.v6_short_daltc[0][ucIndex].data);
            ret = _FAIL;
        }
        else {
            ST.v6_short_daltc[0][ucIndex].status = NODE_PASS;
            printf("  %4d,", ST.v6_short_daltc[0][ucIndex].data);
        }
    }
    printf("\n");
    PRINTF("Y_CH:");
    for (ucIndex = 0; ucIndex < ptl.y_ch; ucIndex++)
    {
        if(ST.v6_short_daltc[1][ucIndex].data > ST.Short.Threshold) {
            ST.v6_short_daltc[1][ucIndex].status = NODE_FAIL;
            printf(" *%4d,", ST.v6_short_daltc[1][ucIndex].data);
            ret = _FAIL;
        }
        else {
            printf("  %4d,", ST.v6_short_daltc[1][ucIndex].data);
            ST.v6_short_daltc[1][ucIndex].status = NODE_PASS;
        }
    }
    printf("\nImpedance value:\n   X:");
    for (ucIndex = 0; ucIndex < ptl.x_ch; ucIndex++)
        PRINTF("%3d_CH,", ucIndex);
    printf("\n     ");
    for (ucIndex = 0; ucIndex < ptl.x_ch; ucIndex++) {
        if(ST.v6_short_daltc[0][ucIndex].data < DIVIDE_10M)
            printf("   10M,");
        else {
            printf("%.3lf", IMPEDANCE_MACRO(ST.v6_short_daltc[0][ucIndex].data, ST.Short.dump1, ST.Short.dump2,ST.Short.vref_v));
            printf("M,");
        }
    }
    printf("\n   Y:");
    for (ucIndex = 0; ucIndex < ptl.y_ch; ucIndex++)
        PRINTF("%3d_CH,", ucIndex);
    printf("\n     ");
    for (ucIndex = 0; ucIndex < ptl.y_ch; ucIndex++) {
        if(ST.v6_short_daltc[1][ucIndex].data < DIVIDE_10M)
            printf("   10M,");
        else {
            printf("%.3lf", IMPEDANCE_MACRO(ST.v6_short_daltc[1][ucIndex].data, ST.Short.dump1, ST.Short.dump2,ST.Short.vref_v));
            printf("M,");
        }
    }
    ModeCtrl_V6(ENTER_NORMAL_MODE, DISABLE_ENGINEER);
	return ret;
}

int viRunOpenTest()
{
    int ret = 1;
    if (inProtocolStyle == _Protocol_V3_)
    {
        if(ST.UseNewFlow == 1)
        {
            ret = viRunOpenTest_3X_NewFlow();
        }
        else
        {
            ret = viRunOpenTest_3X();
        }
    }
    else if (inProtocolStyle == _Protocol_V6_)
    {
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
    int CHX = 0, CHY = 0;
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

    printf("\n");
    PRINTF("Self Test Criteria:\n");
    PRINTF("Maximum = %d\n",ST.Self_Maximum);
    PRINTF("Minimum = %d\n",ST.Self_Minimum);
    PRINTF("P2P = %d\n",ST.Self_P2P);
    PRINTF("P2P Edge = %d\n",ST.Self_P2P_Edge);
    PRINTF("Frame Count = %d\n\n",ST.Self_Frame_Count);


    if (viInitRawData_3Para_3X(0x0d, 0xE6, ST.Self_Frame_Count) != _FAIL)
    {
        if (viGetRawData_3X(0xE6, _FastMode_, ptl.x_ch + ptl.y_ch + 4, _DataFormat_16_Bit_, ptl.x_ch + ptl.y_ch + 4) != _FAIL)
        {
            ret = _SUCCESS;
        }
        else
        {
            PRINTF("Error! Get RawData Failed!\n");
            ret = _FAIL;
        }
    }
    else
    {
        PRINTF("Error! Init RawData Failed!\n");
        ret = _FAIL;
    }

    if (ret != _FAIL)
    {
        PRINTF("Self Datas CHX: ");
        for (CHX = 0; CHX < ptl.x_ch; CHX++)
        {
            printf("%4d,", uiTestDatas[0][CHX]);
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
        printf("\n");

        PRINTF("Self Datas CHY: ");
        for (CHY = ptl.x_ch, inCounts = 0; CHY < (ptl.y_ch + ptl.x_ch); CHY++, inCounts++)
        {
            printf("%4d,", uiTestDatas[0][CHY]);
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
        printf("\n");
        PRINTF("CHX Maximum Value: ");
        printf("%4d,\n", uiTestDatas[0][ptl.y_ch + ptl.x_ch + 0]);
        PRINTF("CHX Minimum Value: ");
        printf("%4d,\n", uiTestDatas[0][ptl.y_ch + ptl.x_ch + 1]);
        PRINTF("CHY Maximum Value: ");
        printf("%4d,\n", uiTestDatas[0][ptl.y_ch + ptl.x_ch + 2]);
        PRINTF("CHY Minimum Value: ");
        printf("%4d,\n", uiTestDatas[0][ptl.y_ch + ptl.x_ch + 3]);
        printf("\n\n");
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
            //PRINTF("Success! Open Test : Pass\n");
            //PRINTF("Success! Open Test : Pass\n");
        }
        else
        {
            ucSensorTestResult |= SELF_TEST;
            ret = _FAIL;
            //PRINTF("Failed! Open Test : NG\n");
            //PRINTF("Failed! Open Test : NG\n");
        }
    }
    return ret;
}

int viRunDACTest_3X()
{
    int ret = _FAIL;
    int CHX = 0, CHY = 0;
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
    printf("\n");
    PRINTF("DAC Test Criteria:\n");
    PRINTF("Self P DAC Maximum = %d\n",ST.DAC_SC_P_Maximum);
    PRINTF("Self P DAC Minimum = %d\n",ST.DAC_SC_P_Minimum);
    PRINTF("Self N DACMaximum = %d\n",ST.DAC_SC_N_Maximum);
    PRINTF("Self N DACMinimum = %d\n",ST.DAC_SC_N_Minimum);
    PRINTF("Mutual P DAC Maximum = %d\n",ST.DAC_MC_P_Maximum);
    PRINTF("Mutual P DAC Minimum = %d\n",ST.DAC_MC_P_Minimum);
    PRINTF("Mutual N DAC Maximum = %d\n",ST.DAC_MC_N_Maximum);
    PRINTF("Mutual N DAC Minimum = %d\n\n",ST.DAC_MC_N_Minimum);
    ucSignedDatas = 1;

    //----------------------------Key_DAC_SC_P_2510_ = 0x22 ---------------------------------------------
    if (viInitRawData_3X(0x22, 0xE6) != _FAIL)
    {
        if (viGetRawData_3X(0xE6, _FastMode_, ptl.x_ch + ptl.y_ch, _DataFormat_8_Bit_, ptl.x_ch + ptl.y_ch) != _FAIL)
        {
            ret = _SUCCESS;
        }
        else
        {
            PRINTF("Error! Get RawData Failed!\n");
            ret = _FAIL;
        }
    }
    else
    {
        PRINTF("Error! Init RawData Failed!\n");
        ret = _FAIL;
    }

    if (ret != _FAIL)
    {
        PRINTF("DAC_SC_P Datas CHX: \n");
        for (CHX = 0; CHX < ptl.x_ch; CHX++)
        {
            ST.dac_sc_p[CHX] = uiTestDatas[0][CHX];
            printf("%4d,", uiTestDatas[0][CHX]);
            if (uiTestDatas[0][CHX] < ST.DAC_SC_P_Minimum || uiTestDatas[0][CHX] > ST.DAC_SC_P_Maximum)
            {
                ucPass = 0;
            }
        }
        printf("\n");

        PRINTF("DAC_SC_P Datas CHY: \n");
        for (CHY = ptl.x_ch; CHY < (ptl.y_ch + ptl.x_ch); CHY++)
        {
            ST.dac_sc_p[CHY] = uiTestDatas[0][CHY];
            printf("%4d,", uiTestDatas[0][CHY]);
            if (uiTestDatas[0][CHY] < ST.DAC_SC_P_Minimum || uiTestDatas[0][CHY] > ST.DAC_SC_P_Maximum)
            {
                ucPass = 0;
            }
        }
        printf("\n\n");

        if (ucPass == 1)
        {
            //PRINTF("Success! Open Test : Pass\n");
            //PRINTF("Success! Open Test : Pass\n");
        }
        else
        {
            ucSensorTestResult |= DAC_TEST;
            ret = _FAIL;
            //PRINTF("Failed! Open Test : NG\n");
            //PRINTF("Failed! Open Test : NG\n");
        }
    }

    //----------------------------Key_DAC_MC_P_2510_ = 0x20 ---------------------------------------------
    if (viInitRawData_3X(0x20, 0xE6) != _FAIL)
    {
        if (viGetRawData_3X(0xE6, _FastMode_, ptl.x_ch * ptl.y_ch, _DataFormat_8_Bit_, ptl.x_ch) != _FAIL)
        {
            ret = _SUCCESS;
        }
        else
        {
            PRINTF("Error! Get RawData Failed!\n");
            ret = _FAIL;
        }
    }
    else
    {
        PRINTF("Error! Init RawData Failed!\n");
        ret = _FAIL;
    }

    if (ret != _FAIL)
    {
        PRINTF("DAC_MC_P Datas CHX: \n");
        for (CHY = 0; CHY < ptl.y_ch; CHY++)
        {
            PRINTF("Y_%2dCH:", CHY);
            for (CHX = 0; CHX < ptl.x_ch; CHX++)
            {
                printf("%4d,", uiTestDatas[CHY][CHX]);
                ST.dac_mc_p[CHX][CHY] = uiTestDatas[CHY][CHX];
                if (uiTestDatas[0][CHX] < ST.DAC_MC_P_Minimum || uiTestDatas[0][CHX] > ST.DAC_MC_P_Maximum)
                {
                    ucPass = 0;
                }
            }
            printf("\n\n");
        }
        printf("\n\n");

        if (ucPass == 1)
        {
            //PRINTF("Success! Open Test : Pass\n");
            //PRINTF("Success! Open Test : Pass\n");
        }
        else
        {
            ucSensorTestResult |= DAC_TEST;
            ret = _FAIL;
            //PRINTF("Failed! Open Test : NG\n");
            //PRINTF("Failed! Open Test : NG\n");
        }
    }

    //----------------------------Key_DAC_SC_N_2510_ = 0x23 ---------------------------------------------
    if (viInitRawData_3X(0x23, 0xE6) != _FAIL)
    {
        if (viGetRawData_3X(0xE6, _FastMode_, ptl.x_ch + ptl.y_ch, _DataFormat_8_Bit_, ptl.x_ch + ptl.y_ch) != _FAIL)
        {
            ret = _SUCCESS;
        }
        else
        {
            PRINTF("Error! Get RawData Failed!\n");
            ret = _FAIL;
        }
    }
    else
    {
        PRINTF("Error! Init RawData Failed!\n");
        ret = _FAIL;
    }

    if (ret != _FAIL)
    {
        PRINTF("DAC_SC_N Datas CHX: \n");
        for (CHX = 0; CHX < ptl.x_ch; CHX++)
        {
            ST.dac_sc_n[CHX] = uiTestDatas[0][CHX];
            printf("%4d,", uiTestDatas[0][CHX]);
            if (uiTestDatas[0][CHX] < ST.DAC_SC_N_Minimum || uiTestDatas[0][CHX] > ST.DAC_SC_N_Maximum)
            {
                ucPass = 0;
            }
        }
        printf("\n");

        PRINTF("DAC_SC_N Datas CHY: \n");
        for (CHY = ptl.x_ch; CHY < (ptl.y_ch + ptl.x_ch); CHY++)
        {
            ST.dac_sc_n[CHY] = uiTestDatas[0][CHY];
            printf("%4d,", uiTestDatas[0][CHY]);
            if (uiTestDatas[0][CHY] < ST.DAC_SC_N_Minimum || uiTestDatas[0][CHY] > ST.DAC_SC_N_Maximum)
            {
                ucPass = 0;
            }
        }
        printf("\n\n");

        if (ucPass == 1)
        {
            //PRINTF("Success! Open Test : Pass\n");
            //PRINTF("Success! Open Test : Pass\n");
        }
        else
        {
            ucSensorTestResult |= DAC_TEST;
            ret = _FAIL;
            //PRINTF("Failed! Open Test : NG\n");
            //PRINTF("Failed! Open Test : NG\n");
        }
    }

    //----------------------------Key_DAC_MC_N_2510_ = 0x21 ---------------------------------------------
    if (viInitRawData_3X(0x21, 0xE6) != _FAIL)
    {
        if (viGetRawData_3X(0xE6, _FastMode_, ptl.x_ch * ptl.y_ch, _DataFormat_8_Bit_, ptl.x_ch) != _FAIL)
        {
            ret = _SUCCESS;
        }
        else
        {
            PRINTF("Error! Get RawData Failed!\n");
            ret = _FAIL;
        }
    }
    else
    {
        PRINTF("Error! Init RawData Failed!\n");
        ret = _FAIL;
    }

    if (ret != _FAIL)
    {
        PRINTF("DAC_MC_N Datas CHX: \n");
        for (CHY = 0; CHY < ptl.y_ch; CHY++)
        {
            PRINTF("Y_%2dCH:", CHY);
            for (CHX = 0; CHX < ptl.x_ch; CHX++)
            {
                ST.dac_mc_n[CHX][CHY] = uiTestDatas[CHY][CHX];
                printf("%4d,", uiTestDatas[CHY][CHX]);
                if (uiTestDatas[0][CHX] < ST.DAC_MC_N_Minimum || uiTestDatas[0][CHX] > ST.DAC_MC_N_Maximum)
                {
                    ucPass = 0;
                }
            }
            printf("\n\n");
        }
        printf("\n\n");

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
	printf("\n");
	PRINTF("Short Test Criteria:\n");
	PRINTF("Threshold = %d\n",ST.Short.Threshold);
    if(ptl.key_mode == ILITEK_HW_KEY_MODE) {
        PRINTF("KeyRX_Threshold = %d\n",ST.Short.keyRx_thr);
        PRINTF("KeyTsX_Threshold = %d\n",ST.Short.keyTx_thr);
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
            if (viGetRawData_3X(ucReadCMD, _SlowMode_, ucLineLenth, _DataFormat_8_Bit_, ucLineLenth) != _FAIL)
            {
                ret = _SUCCESS;
            }
            else
            {
                PRINTF("Error! Get RawData Failed!\n");
                ret = _FAIL;
            }
        }
        else
        {
            PRINTF("Error! Init RawData Failed!\n");
            ret = _FAIL;
        }

        for (ucIndex = 0; ucIndex < ucLineLenth; ucIndex++)
        {
            ST.short_daltc[inCounts][ucIndex] = uiTestDatas[0][ucIndex];
        }
    }

	if (ret != _FAIL)
	{
		printf("\n");
		PRINTF("Short Datas:");
		printf("\n");
		PRINTF("X_CH_SLK: ");
		for (ucIndex = 0; ucIndex < ptl.x_ch; ucIndex++)
		{
			printf("%4d,", ST.short_daltc[0][ucIndex]);
		}
		printf("\n");
		PRINTF("X_CH__LK: ");
		for (ucIndex = 0; ucIndex < ptl.x_ch; ucIndex++)
		{
			printf("%4d,", ST.short_daltc[2][ucIndex]);
			if (((ST.short_daltc[0][ucIndex] >= ST.short_daltc[2][ucIndex]) &&
            ((ST.short_daltc[0][ucIndex] - ST.short_daltc[2][ucIndex]) > ST.Short.Threshold)) ||
			((ST.short_daltc[0][ucIndex] < ST.short_daltc[2][ucIndex]) &&
            ((ST.short_daltc[2][ucIndex] - ST.short_daltc[0][ucIndex]) > ST.Short.Threshold)))
			{
				ucPass = 0;
			}
		}

		printf("\n");
		PRINTF("Y_CH_SLK: ");
		for (ucIndex = 0; ucIndex < ptl.y_ch; ucIndex++)
		{
			printf("%4d,", ST.short_daltc[1][ucIndex]);
		}

		printf("\n");
		PRINTF("Y_CH__LK: ");
		for (ucIndex = 0; ucIndex < ptl.y_ch; ucIndex++)
		{
			printf("%4d,", ST.short_daltc[3][ucIndex]);
			if (((ST.short_daltc[1][ucIndex] >= ST.short_daltc[3][ucIndex]) &&
            ((ST.short_daltc[1][ucIndex] - ST.short_daltc[3][ucIndex]) > ST.Short.Threshold)) ||
			((ST.short_daltc[1][ucIndex] < ST.short_daltc[3][ucIndex]) &&
            ((ST.short_daltc[3][ucIndex] - ST.short_daltc[1][ucIndex]) > ST.Short.Threshold)))
			{
				ucPass = 0;
			}
		}
        if (ptl.key_mode == ILITEK_HW_KEY_MODE) {
            PRINTF("\n");
            PRINTF("KEY X_CH_SLK: ");
            for (ucIndex = ptl.x_ch; ucIndex < ptl.x_ch + ptl.key_num; ucIndex++)
            {
                printf("%4d,", ST.short_daltc[0][ucIndex]);
            }
            printf("\n");
            PRINTF("KEY X_CH__LK: ");
            for (ucIndex = ptl.x_ch; ucIndex < ptl.x_ch + ptl.key_num; ucIndex++)
            {
                printf("%4d,", ST.short_daltc[2][ucIndex]);
                if (((ST.short_daltc[0][ucIndex] >= ST.short_daltc[2][ucIndex]) &&
                ((ST.short_daltc[0][ucIndex] - ST.short_daltc[2][ucIndex]) > ST.Short.keyRx_thr)) ||
                ((ST.short_daltc[0][ucIndex] < ST.short_daltc[2][ucIndex]) &&
                ((ST.short_daltc[2][ucIndex] - ST.short_daltc[0][ucIndex]) > ST.Short.keyRx_thr)))
                {
                    ucPass = 0;
                }
            }

            printf("\n");
            PRINTF("KEY Y_CH_SLK: ");
            for (ucIndex = ptl.y_ch; ucIndex < ptl.y_ch + key_ych; ucIndex++)
            {
                printf("%4d,", ST.short_daltc[1][ucIndex]);
            }

            printf("\n");
            PRINTF("KEY Y_CH__LK: ");
            for (ucIndex = ptl.y_ch; ucIndex < ptl.y_ch + key_ych; ucIndex++)
            {
                printf("%4d,", ST.short_daltc[3][ucIndex]);
                if (((ST.short_daltc[1][ucIndex] >= ST.short_daltc[3][ucIndex]) &&
                ((ST.short_daltc[1][ucIndex] - ST.short_daltc[3][ucIndex]) > ST.Short.keyTx_thr)) ||
                ((ST.short_daltc[1][ucIndex] < ST.short_daltc[3][ucIndex]) &&
                ((ST.short_daltc[3][ucIndex] - ST.short_daltc[1][ucIndex]) > ST.Short.keyTx_thr)))
                {
                    ucPass = 0;
                }
            }
        }

		printf("\n\n");
		if (ucPass == 1)
		{
			//	PRINTF("Success! Short Test : Pass\n");
			//	PRINTF("Success! Short Test : Pass\n");
		}
		else
		{
			ucSensorTestResult |= MICROSHORT_TEST;
            ret = _FAIL;
			//	PRINTF("Failed! Short Test : NG\n");
			//	PRINTF("Failed! Short Test : NG\n");
		}
	}
	return ret;
}

int viRunOpenTest_6X()
{
	int ret = _SUCCESS;
	int trans_num = 0;
    int TxAverageStatus = _SUCCESS, RxDiffStatus = _SUCCESS, ThresholdStatus = _SUCCESS;

	PRINTF("\nOpen Test Criteria:\n");
    trans_num = ptl.x_ch * ptl.y_ch;
    PRINTF("Threshold = %d, total = %d\n",ST.Open_Threshold, trans_num);
    PRINTF("TX_Average_Diff_Gap = %d\n",ST.Open_TX_Aver_Diff);
    PRINTF("RX Delta Threshold = %d\n",ST.Open_RX_Delta_Threshold);
    PRINTF("Open_RX_Continue_Fail_Tolerance = %d\n",ST.Open_RX_Continue_Fail_Tolerance);
    PRINTF("Gain = %d\n", ST.Open.gain);
    PRINTF("Frequency = %d\n\n", ST.Open.freq);
    ModeCtrl_V6(ENTER_SUSPEND_MODE, ENABLE_ENGINEER);
    ModeCtrl_V6(ENTER_TEST_MODE, DISABLE_ENGINEER);
    if(SetOpenInfo(ST.Open.freq & 0xFF, (ST.Open.freq >> 8) & 0xFF, ST.Open.gain) < 0) {
        PRINTF("Error! Set Open info Failed!\n");
        return _FAIL;
    }
    if(viGetCDCData_6X(TEST_MODE_V6_OPEN, trans_num) < 0)
        return _FAIL;
    if(ST.Open_TX_Aver_Diff > 0)
        if(OpenTestTxAverageTest() == _FAIL) {
            TxAverageStatus = _FAIL;
        }
    if(OpenTestThreshold() == _FAIL) {
        ThresholdStatus = _FAIL;
    }
    if(ST.Open_RX_Delta_Threshold > 0) {
        if(OpenTestRxDiff() == _FAIL)
            RxDiffStatus = _FAIL;
    }
    if(ptl.key_mode == ILITEK_HW_KEY_MODE) {
        if(OpenTestKeyThreshold() == _FAIL)
            ret = _FAIL;
    }

    if(TxAverageStatus == _FAIL || RxDiffStatus == _FAIL || ThresholdStatus == _FAIL) {
        ret = _FAIL;
    }

	return ret;
}

int viRunMircoOpenTest_3X()
{

}
int viRunMircoOpenTest_6X()
{
	int ret = _SUCCESS;
	int CHY = 0, CHX = 0, trans_num = 0;
    int TxAverageStatus = _SUCCESS, RxDiffStatus = _SUCCESS;

	PRINTF("\nMirco Open Test Criteria:\n");
    trans_num = ptl.x_ch * ptl.y_ch;
    PRINTF("RX_Delta_En=%s\n", ST.MOpen.RxDeltaEn ? "True" : "False");
    PRINTF("TX_Avg_Delta_En=%s\n", ST.MOpen.TxAvgEn ? "True" : "False");
    PRINTF("TX_Avg_Delta_Threshold = %d\n",ST.MOpen.TxAvgThr);
    PRINTF("TX_Avg_Delta_Threshold_AvoidCorner=%s\n", ST.MOpen.TxAvgCorner ? "True" : "False");
    PRINTF("RX_Delta_Threshold_Tolerance = %d\n",ST.MOpen.RxToles);  
    

    if(!ST.Open_test) {
        ModeCtrl_V6(ENTER_TEST_MODE, DISABLE_ENGINEER);
    
        if (viInitRawData_6X(TEST_MODE_V6_OPEN, 10) != _FAIL)
        {
            if (viGetRawData_6X(trans_num) != _FAIL)
            {
                ret = _SUCCESS;
            }
            else
            {
                PRINTF("Error! Get RawData Failed!\n");
                ret = _FAIL;
            }
        }
        else
        {
            PRINTF("Error! Init RawData Failed!\n");
            ret = _FAIL;
        }
    }

    if(OpenTestTxAverageTest() == _FAIL) {
        TxAverageStatus = _FAIL;
    }
    PRINTF("  Delta,(Avg_D)\n");
    for (CHY = 1; CHY < ptl.y_ch; CHY++)
    {
        if(ST.MOpen.tx_avg[0][CHY].raw.status == NODE_FAIL) 
            PRINTF("Y_%3dCH,*%5d\n", CHY, ST.MOpen.tx_avg[0][CHY - 1].raw.data);
        else
            PRINTF("Y_%3dCH,%6d\n", CHY, ST.MOpen.tx_avg[0][CHY - 1].raw.data);
    }    
    PRINTF("\nMirco Open Datas: \n       ");
    for (CHX = 0; CHX < ptl.x_ch; CHX++) {
        PRINTF(" %3dCH,", CHX);
    }
    PRINTF("\n");
    for (CHY = 0; CHY < ptl.y_ch; CHY++)
    {
        PRINTF("Y_%2dCH:", CHY);
        for (CHX = 0; CHX < ptl.x_ch; CHX++)
        {
            if(!ST.Open_test)
                ST.v6_open_daltc[CHX][CHY].data = uiTestDatas[CHY][CHX];
            printf("%6d,", ST.v6_open_daltc[CHX][CHY].data);

        }
        //PRINTF("%7d,", ST.Tx_Avdiff_daltc[CHY].data);
        printf("\n");
    }

    if(OpenTestRxDiff() == _FAIL) 
        RxDiffStatus = _FAIL;

    if(TxAverageStatus == _FAIL || RxDiffStatus == _FAIL) {
        ret = _FAIL;
    }

	return ret;
}

int NodeTest_V6(char *name, SensorTest_Node **delac, SensorTest_BenBenchmark_Node **data,
                int x, int y, int max_fail_count, int min_fail_count) {
    int ret = _SUCCESS;
    int CHX = 0, CHY = 0;
    int MaxFailCount = 0, MinFailCount = 0;

    PRINTF("%s_Up: \n        ", name);
    for (CHX = 0; CHX < x; CHX++)
        PRINTF(" X_%3d,", CHX);
    for (CHY = 0; CHY < y; CHY++)
    {
        PRINTF("\nY_%3dCH:", CHY);
        for (CHX = 0; CHX < x; CHX++)
        {
            printf(" %5d,", data[CHX][CHY].ini.max);
        }
    }

    PRINTF("\n\n%s_Low: \n        ", name);
    for (CHX = 0; CHX < x; CHX++)
        PRINTF(" X_%3d,", CHX);
    for (CHY = 0; CHY < y; CHY++)
    {
        PRINTF("\nY_%3dCH:", CHY);
        for (CHX = 0; CHX < x; CHX++)
        {
            printf(" %5d,", data[CHX][CHY].ini.min);
        }
    }

    PRINTF("\n\n%s Datas: \n        ", name);
    for (CHX = 0; CHX < x; CHX++)
        PRINTF(" X_%3d,", CHX);
    for (CHY = 0; CHY < y; CHY++)
    {
        PRINTF("\nY_%3dCH:", CHY);
        for (CHX = 0; CHX < x; CHX++)
        {
            data[CHX][CHY].raw.data = delac[CHX][CHY].data;

            if (data[CHX][CHY].raw.data > data[CHX][CHY].ini.max ||
                data[CHX][CHY].raw.data < data[CHX][CHY].ini.min)
            {
                data[CHX][CHY].raw.status = NODE_FAIL;
                if (data[CHX][CHY].raw.data > data[CHX][CHY].ini.max) {
                    data[CHX][CHY].raw.max_st = NODE_FAIL;
                    MaxFailCount++;
                }

                if (data[CHX][CHY].raw.data < data[CHX][CHY].ini.min) {
                    data[CHX][CHY].raw.min_st = NODE_FAIL;
                    MinFailCount++;
                }
            }
            if(data[CHX][CHY].raw.status)
                printf("*%5d,", data[CHX][CHY].raw.data);
            else
                printf(" %5d,", data[CHX][CHY].raw.data);
        }
    }
    if(MaxFailCount > max_fail_count || MinFailCount > min_fail_count) {
        ret = _FAIL;
    }
    PRINTF("\n%s=%s\n", name, ret ? "FAIL":"PASS");
    return ret;
}

int viRunUniformityTest_6X()
{
	int ret = _SUCCESS;
	int CHY = 0, CHX = 0, trans_num = 0;
    int RawMaxFailCount = 0;
    int RawMinFailCount = 0;
    int Win1FailCount = 0;
    int Win2FailCount = 0;

	PRINTF("Uniformity Test Criteria:\n");
    trans_num = ptl.x_ch * ptl.y_ch;
    PRINTF("Max_Threshold = %d\n",ST.Uniformity.Max_Threshold);
    PRINTF("Up_FailCount = %d\n",ST.Uniformity.Up_FailCount);
    PRINTF("Min_Threshold = %d\n",ST.Uniformity.Min_Threshold);
    PRINTF("Low_FailCount = %d\n",ST.Uniformity.Low_FailCount);
    PRINTF("Win1_Threshold = %d\n",ST.Uniformity.Win1_Threshold);
    PRINTF("Win1_FailCount = %d\n",ST.Uniformity.Win1_FailCount);
    PRINTF("Win2_Threshold = %d\n",ST.Uniformity.Win2_Threshold);
    PRINTF("Win2_FailCount = %d\n",ST.Uniformity.Win2_FailCount);
    ModeCtrl_V6(ENTER_SUSPEND_MODE, ENABLE_ENGINEER);
    ModeCtrl_V6(ENTER_TEST_MODE, DISABLE_ENGINEER);
    if(viGetCDCData_6X(TEST_MODE_V6_MC_RAW_NBK, trans_num) < 0)
        return _FAIL;

    if(ST.PFVer >= PROFILE_V1_0_2_0 && ST.Uniformity.En_allraw == false) {
        PRINTF("Uniformity RawData\n Datas: \n        no test\n");
    }
    else {
        PRINTF("Uniformity\n Datas: \n        ");
        for (CHX = 0; CHX < ptl.x_ch; CHX++)
            PRINTF(" X_%3d,", CHX);
        for (CHY = 0; CHY < ptl.y_ch; CHY++)
        {
            PRINTF("\nY_%3dCH:", CHY);
            for (CHX = 0; CHX < ptl.x_ch; CHX++)
            {
                ST.v6_unifor_daltc[CHX][CHY].data = uiTestDatas[CHY][CHX];
                ST.v6_unifor_daltc[CHX][CHY].status = NODE_PASS;
                ST.v6_unifor_daltc[CHX][CHY].max_st = NODE_PASS;
                ST.v6_unifor_daltc[CHX][CHY].min_st = NODE_PASS;
                //profile 1.0.2.0 version above use all node test
                if ((ST.v6_unifor_daltc[CHX][CHY].data > ST.Uniformity.Max_Threshold || ST.v6_unifor_daltc[CHX][CHY].data < ST.Uniformity.Min_Threshold) 
                && (ST.PFVer < PROFILE_V1_0_2_0))
                {
                    ST.v6_unifor_daltc[CHX][CHY].status = NODE_FAIL;
                    if(ST.v6_unifor_daltc[CHX][CHY].data > ST.Uniformity.Max_Threshold) {
                        RawMaxFailCount++;
                        ST.v6_unifor_daltc[CHX][CHY].max_st = NODE_FAIL;
                    }
                    if(ST.v6_unifor_daltc[CHX][CHY].data < ST.Uniformity.Min_Threshold) {
                        RawMinFailCount++;
                        ST.v6_unifor_daltc[CHX][CHY].max_st = NODE_FAIL;
                    }
                }
                if(ST.v6_unifor_daltc[CHX][CHY].status == NODE_FAIL)
                    printf("*%5d,", ST.v6_unifor_daltc[CHX][CHY].data);
                else
                    printf(" %5d,", ST.v6_unifor_daltc[CHX][CHY].data);
            }
        }
        if(RawMaxFailCount > ST.Uniformity.Up_FailCount || RawMinFailCount > ST.Uniformity.Low_FailCount) {
            ret = _FAIL;
        }
    }
    if(ST.PFVer >= PROFILE_V1_0_2_0 && ST.Uniformity.En_allwin1 == false) {
        PRINTF("\n\nUniformity RawData Win1\n Datas: \n        no test\n");
    }
    else {
        PRINTF("\n\nUniformity Win_1 Datas: \n        ");
        for (CHX = 0; CHX < ptl.x_ch; CHX++)
            PRINTF(" X_%3d,", CHX);
        for (CHY = 1; CHY < ptl.y_ch; CHY++)
        {
            PRINTF("\nY_%3dCH:", CHY - 1);
            for (CHX = 0; CHX < ptl.x_ch; CHX++)
            {
                ST.v6_unifor_win1[CHX][CHY-1].data = abs(uiTestDatas[CHY][CHX] - uiTestDatas[CHY - 1][CHX]);
                //profile 1.0.2.0 version above use all node test
                if ((ST.v6_unifor_win1[CHX][CHY-1].data > ST.Uniformity.Win1_Threshold) && (ST.PFVer < PROFILE_V1_0_2_0))
                {
                    ++Win1FailCount;
                    ST.v6_unifor_win1[CHX][CHY-1].status = NODE_FAIL;
                    printf("*%5d,", ST.v6_unifor_win1[CHX][CHY-1].data);
                }
                else
                {
                    ST.v6_unifor_win1[CHX][CHY-1].status = NODE_PASS;
                    printf(" %5d,", ST.v6_unifor_win1[CHX][CHY-1].data);
                }
            }
        }
        if (Win1FailCount >= ST.Uniformity.Win1_FailCount)
        {
            ret = _FAIL;
        }
    }
    if(ST.PFVer >= PROFILE_V1_0_2_0 && ST.Uniformity.En_allwin2 == false) {
        PRINTF("Uniformity RawData Win1\n Datas: \n        no test\n");
    }
    else {
        PRINTF("\n\nUniformity Win_2 Datas: \n        ");
        for (CHX = 0; CHX < ptl.x_ch - 1; CHX++)
            PRINTF(" X_%3d,", CHX);
        PRINTF("\n");
        for (CHY = 1; CHY < ptl.y_ch; CHY++)
        {
            PRINTF("Y_%3dCH:", CHY - 1);
            for (CHX = 1; CHX < ptl.x_ch; CHX++)
            {
                ST.v6_unifor_win2[CHX-1][CHY-1].data = abs((uiTestDatas[CHY][CHX] - uiTestDatas[CHY - 1][CHX]) -
                        (uiTestDatas[CHY][CHX - 1] - uiTestDatas[CHY - 1][CHX - 1]));
                //profile 1.0.2.0 version above use all node test
                if ((ST.v6_unifor_win2[CHX-1][CHY-1].data > ST.Uniformity.Win2_Threshold) && (ST.PFVer < PROFILE_V1_0_2_0))
                {
                    ++Win2FailCount;
                    ST.v6_unifor_win2[CHX-1][CHY-1].status = NODE_FAIL;
                    printf("*%5d,",  ST.v6_unifor_win2[CHX-1][CHY-1].data);
                }
                else {
                    ST.v6_unifor_win2[CHX-1][CHY-1].status = NODE_PASS;
                    printf(" %5d,",  ST.v6_unifor_win2[CHX-1][CHY-1].data);
                }
            }
            printf("\n");
        }
        if (Win2FailCount >= ST.Uniformity.Win2_FailCount)
        {
            ret = _FAIL;
        }
    }

    if (ST.Uniformity.En_bench == ENABLE_TEST) {
        if(NodeTest_V6("BenchMark", ST.v6_unifor_daltc, ST.Uniformity.bench,ptl.x_ch, ptl.y_ch, 1, 1) != _SUCCESS)
            ret = _FAIL;
    }
    if (ST.Uniformity.En_allraw == ENABLE_TEST) {
        if(NodeTest_V6("ANode_Raw ", ST.v6_unifor_daltc, ST.Uniformity.allraw,ptl.x_ch, ptl.y_ch, ST.Uniformity.Up_FailCount, ST.Uniformity.Low_FailCount) != _SUCCESS)
            ret = _FAIL;
    }
    if (ST.Uniformity.En_allwin1 == ENABLE_TEST) {
        if(NodeTest_V6("ANode_Win1", ST.v6_unifor_win1, ST.Uniformity.allwin1,ptl.x_ch, ptl.y_ch - 1, ST.Uniformity.Win1_FailCount, 1) != _SUCCESS)
            ret = _FAIL;
    }
    if (ST.Uniformity.En_allwin2 == ENABLE_TEST) {
        if(NodeTest_V6("ANode_Win2", ST.v6_unifor_win2, ST.Uniformity.allwin2,ptl.x_ch - 1, ptl.y_ch - 1, ST.Uniformity.Win2_FailCount, 1) != _SUCCESS)
            ret = _FAIL;
    }
	return ret;
}

int viRunFWVerTest()
{
    if (inProtocolStyle == _Protocol_V3_)
        return viRunFWVerTest_3X();
    else if(inProtocolStyle == _Protocol_V6_)
        return viRunFWVerTest_6X();
}

int viRunFWVerTest_3X()
{
    int i = 0;
    int ret = _SUCCESS;

    for(i = 0; i < 8; i++)
    {
        PRINTF("FW version check:0x%x,0x%x\n", FWVersion[i], ST.fw_ver[i]);
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
        PRINTF("%s, The protocol no support\n", __func__);
        return _SUCCESS;
    }
    PRINTF("FW Check Criteria:\n");
    if(ptl.block_num > 0 && ptl.block_num != ST.block_num) {
        PRINTF("IC/INI Block number not match, IC:%d, INI:%d\n", ptl.block_num, ST.block_num);
        ST.block_num = ptl.block_num;
    }
    upg.blk = calloc(ST.block_num , sizeof(struct BLOCK_DATA));
    for(i = 0; i < 4; i++)
    {
        PRINTF("FW version check:0x%x,0x%x\n", FWVersion[i], ST.fw_ver[i]);
        if(FWVersion[i] != ST.fw_ver[i])
        {
            return _FAIL;
        }
    }
    if(GetICMode()){
        if (ICMode == OP_MODE_APPLICATION) {
            ModeCtrl_V6(ENTER_SUSPEND_MODE, ENABLE_ENGINEER);
        }
    }
    for(i = 0; i < ST.block_num; i++) {
        upg.blk[i].ic_crc = GetICBlockCrcNum(i, CRC_CALCULATION_FROM_IC);
        if(upg.blk[i].ic_crc != ST.master_crc[i]) {
            PRINTF("Block:%d, IC CRC:0x%x, INI CRC:0x%x\n", i, upg.blk[i].ic_crc, ST.master_crc[i]);
            PRINTF("CRC compare fail\n");
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
                PRINTF("Error! SelfTest Function Don't Support This MCU !\n");
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
                PRINTF("Error! MultyTest Function Don't Support This MCU !\n");
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
	int ret;
    char timebuf[60];
	char result_file_name[256] = {0};
	FILE *result_file;

    if ((ucSensorTestResult & MICROSHORT_TEST) == MICROSHORT_TEST)
        PRINTF("SensorTest: Short Test NG!\n");
    else if ((inFunctions & MICROSHORT_TEST) == MICROSHORT_TEST)
        PRINTF("SensorTest: Short Test PASS!\n");

    if ((ucSensorTestResult & OPEN_TEST) == OPEN_TEST)
        PRINTF("SensorTest: Open Test NG!\n");
    else if ((inFunctions & OPEN_TEST) == OPEN_TEST)
        PRINTF("SensorTest: Open Test PASS!\n");
    if(ST.CreateGolden != 1)
    {
        if ((ucSensorTestResult & FWVERTION_TEST) == FWVERTION_TEST)
            PRINTF("SensorTest: Check FW Version NG!\n");
        else if ((inFunctions & FWVERTION_TEST) == FWVERTION_TEST)
            PRINTF("SensorTest: Check FW Version PASS!\n");
        if ((ucSensorTestResult & SELF_TEST) == SELF_TEST)
            PRINTF("SensorTest: Self Test NG!\n");
        else if ((inFunctions & SELF_TEST) == SELF_TEST)
            PRINTF("SensorTest: Self Test PASS!\n");

        if ((ucSensorTestResult & DAC_TEST) == DAC_TEST)
            PRINTF("SensorTest: DAC Test NG!\n");
        else if ((inFunctions & DAC_TEST) == DAC_TEST)
            PRINTF("SensorTest: DAC Test PASS!\n");

        if ((ucSensorTestResult & ALL_NODE_TEST) == ALL_NODE_TEST)
            PRINTF("SensorTest: All Node Test NG!\n");
        else if ((inFunctions & ALL_NODE_TEST) == ALL_NODE_TEST)
            PRINTF("SensorTest: All Node Test PASS!\n");

        if ((ucSensorTestResult & UNIFORMITY_TEST) == UNIFORMITY_TEST)
            PRINTF("SensorTest: Uniformity Test NG!\n");
        else if ((inFunctions & UNIFORMITY_TEST) == UNIFORMITY_TEST)
            PRINTF("SensorTest: Uniformity Test PASS!\n");
        if (!ucSensorTestResult)
            strcat(result_file_name, "PASS_");
        else
            strcat(result_file_name, "FAIL_");
    }
    else
    {
        if ((ucSensorTestResult & (MICROSHORT_TEST + OPEN_TEST)) > 0)
        {
            strcat(result_file_name, "LOG_FAIL_");
        }
        else
        {
            strcat(result_file_name, "LOG_PASS_");
        }

    }


	strftime(timebuf,60,"%Y%m%d_%I%M%S",timeinfo);
	sprintf(fileName,"%s.csv",timebuf);
	strcat(result_file_name, fileName);

	result_file = fopen(result_file_name, "w");

	PRINTF("test result (.csv) path =>%s\n", result_file_name);
	fprintf(result_file, "===============================================================================\n");
	fprintf(result_file, TOOL_VERSION);
	fprintf(result_file, "ProFile_Path:,%s\n", g_szConfigPath);
	fprintf(result_file, "Start_Testing_Time :,%s", asctime(timeinfo));
	fprintf(result_file, "Sensor test result:\n");
    if ((inFunctions & FWVERTION_TEST) == FWVERTION_TEST)
        vfSaveFWVerTestLog_V3(result_file);
    if ((inFunctions & MICROSHORT_TEST) == MICROSHORT_TEST)
	    vfSaveShortTestLog_V3(result_file);
    if ((inFunctions & OPEN_TEST) == OPEN_TEST)
        if(ST.UseNewFlow == 1)
            vfSaveOpenTestLog_NewFlow(result_file);
        else
            vfSaveOpenTestLog_V3(result_file);
    if ((inFunctions & SELF_TEST) == SELF_TEST)
        vfSaveSelfTestLog(result_file);
    if ((inFunctions & DAC_TEST) == DAC_TEST)
        vfSaveDACTestLog(result_file);
    if ((inFunctions & ALL_NODE_TEST) == ALL_NODE_TEST)
        vfSaveAllNodeTestLog(result_file);
    if ((inFunctions & UNIFORMITY_TEST) == UNIFORMITY_TEST)
        vfSaveUniformityTestLog(result_file);
	ret = remove(fileName);
	fclose(result_file);

}
void vfPrintSensorTestResult_V6(int inFunctions)
{
	int ret;
	char tmp[2] = {"."};
    char timebuf[60],logst[60];
	char result_file_name[256] = {0};
	FILE *result_file;

    if ((ucSensorTestResult & MICROSHORT_TEST) == MICROSHORT_TEST)
        PRINTF("SensorTest: Short Test NG!\n");
    else if ((inFunctions & MICROSHORT_TEST) == MICROSHORT_TEST)
        PRINTF("SensorTest: Short Test PASS!\n");

    if ((ucSensorTestResult & OPEN_TEST) == OPEN_TEST)
        PRINTF("SensorTest: Open Test NG!\n");
    else if ((inFunctions & OPEN_TEST) == OPEN_TEST)
        PRINTF("SensorTest: Open Test PASS!\n");
    if(ST.CreateGolden != 1)
    {
        if ((ucSensorTestResult & MIRCO_OPEN_TEST) == MIRCO_OPEN_TEST)
            PRINTF("SensorTest: Mirco Open Test NG!\n");
        else if ((inFunctions & MIRCO_OPEN_TEST) == MIRCO_OPEN_TEST)
            PRINTF("SensorTest: Mirco Open Test PASS!\n");
        if ((ucSensorTestResult & FWVERTION_TEST) == FWVERTION_TEST)
            PRINTF("SensorTest: Check FW Version NG!\n");
        else if ((inFunctions & FWVERTION_TEST) == FWVERTION_TEST)
            PRINTF("SensorTest: Check FW Version PASS!\n");
        if ((ucSensorTestResult & SELF_TEST) == SELF_TEST)
            PRINTF("SensorTest: Self Test NG!\n");
        else if ((inFunctions & SELF_TEST) == SELF_TEST)
            PRINTF("SensorTest: Self Test PASS!\n");

        if ((ucSensorTestResult & DAC_TEST) == DAC_TEST)
            PRINTF("SensorTest: DAC Test NG!\n");
        else if ((inFunctions & DAC_TEST) == DAC_TEST)
            PRINTF("SensorTest: DAC Test PASS!\n");

        if ((ucSensorTestResult & ALL_NODE_TEST) == ALL_NODE_TEST)
            PRINTF("SensorTest: All Node Test NG!\n");
        else if ((inFunctions & ALL_NODE_TEST) == ALL_NODE_TEST)
            PRINTF("SensorTest: All Node Test PASS!\n");

        if ((ucSensorTestResult & UNIFORMITY_TEST) == UNIFORMITY_TEST)
            PRINTF("SensorTest: Uniformity Test NG!\n");
        else if ((inFunctions & UNIFORMITY_TEST) == UNIFORMITY_TEST)
            PRINTF("SensorTest: Uniformity Test PASS!\n");
        if (!ucSensorTestResult)
            sprintf(logst, "PASS_");
        else
            sprintf(logst, "FAIL_");
    }
    else
    {
        if ((ucSensorTestResult & (MICROSHORT_TEST + OPEN_TEST)) > 0)
        {
            sprintf(logst, "LOG_FAIL_");
        }
        else
        {
            sprintf(logst, "LOG_PASS_");
        }

    }

	strftime(timebuf,60,"%Y%m%d_%I%M%S",timeinfo);
    if(ST.LogPath == NULL) {
        strcpy(ST.LogPath, tmp);
    }
    //PRINTF(" %s %s\n", ST.LogPath, tmp);
	sprintf(result_file_name,"%s/%s%s.csv",ST.LogPath, logst, timebuf);
	//strcat(logst, fileName);
    //strcat(result_file_name, fileName);
	result_file = fopen(result_file_name, "w");
	PRINTF("test result (.csv) path =>%s\n", result_file_name);
	fprintf(result_file, "===============================================================================\n");
	fprintf(result_file, TOOL_VERSION);
    fprintf(result_file, REPORT_VERSION);
    fprintf(result_file, "ProFile_Version        ,%d.%d.%d.%d\n", ST.PFVer0, ST.PFVer1, ST.PFVer2, ST.PFVer3);
	fprintf(result_file, "ProFile_Path           ,%s\n", g_szConfigPath);
	fprintf(result_file, "Start_Testing_Time     ,%s\n", asctime(timeinfo));
	fprintf(result_file, "IC Type                ,ILI%x\n", ptl.ic);
    fprintf(result_file, "IC Channel             ,X:%d Y:%d\n", ptl.x_ch, ptl.y_ch);
	fprintf(result_file, "Sensor test result     ,\n");
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
	ret = remove(fileName);
	fclose(result_file);

}

void vfPrintSensorTestResult(int inFunctions)
{
    PRINTF("\n========================TestResult========================\n\n");
    if (inProtocolStyle == _Protocol_V3_)
    {
        vfPrintSensorTestResult_V3(inFunctions);
    }
    else if (inProtocolStyle == _Protocol_V6_)
    {
        vfPrintSensorTestResult_V6(inFunctions);
    }
    PRINTF("\n==========================================================\n");
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
    int ret = 1;

    if (inProtocolStyle == _Protocol_V3_ && (KernelVersion[2] == RDValue_MCUKernel_CDCVersion_8bit2315_ || KernelVersion[2] == RDValue_MCUKernel_CDCVersion_16bit2510_))
    {
        if(ST.UseNewFlow == 1)
        {
            ret = viRunUniformityTest_3X();
        }
        else
        {
            PRINTF("Error! UniformityTest Function Just Support When UseNewFlow = 1 !\n");
            ret = _FAIL;
        }
    }
    else if (inProtocolStyle == _Protocol_V6_)
    {
        ret = viRunUniformityTest_6X();
    }
    else
    {
        PRINTF("Error! UniformityTest Function Don't Support This MCU !\n");
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
        PRINTF("Error! AllNodeTest Function Don't Support This MCU !\n");
        ret = _FAIL;
    }
    else if (inProtocolStyle == _Protocol_V6_)
    {

    }
    return ret;
}
int init_sentest_array()
{
    int i = 0;
    //set short array
    ST.short_daltc = (short int**)calloc(4, sizeof(short int*));
    for(i = 0; i < 4; i++)
    {
        if(i % 2 == 0)
            ST.short_daltc[i] = (short int*)calloc(ptl.x_ch, sizeof(short int));
        else
        {
            ST.short_daltc[i] = (short int*)calloc(ptl.y_ch, sizeof(short int));
        }
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
        ST.open_Tx_diff = (short int**)calloc(ptl.x_ch, sizeof(short int*));
        for(i = 0; i < ptl.x_ch; i++)
            ST.open_Tx_diff[i] = (short int*)calloc(ptl.y_ch, sizeof(short int));
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
    ST.LogPath = calloc(1024, sizeof(char));
}
int viRunSensorTest(int inFunctions)
{
    int ret = 0;
    switch_irq(0);
    if (inProtocolStyle == _Protocol_V3_) {
        ret = viRunSensorTest_V3(inFunctions);
    }
    else if(inProtocolStyle == _Protocol_V6_) {
        ret = viRunSensorTest_V6(inFunctions);
    }
    switch_irq(1);
    return ret;
}
int viRunSensorTest_V3(int inFunctions)
{
    int ret = _SUCCESS;
    ucSensorTestResult = 0;
    time_t rawtime;
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    usleep(100000);
    if (viEnterTestMode() != _FAIL && viGetPanelInfor() != _FAIL && inFunctions > 0)
    {
        init_sentest_array();
        //read profile set sensor 
        if (strlen(IniPath) != 0)
        {
            inFunctions = ReadST();
        }
        usleep(200000);
        if ((inFunctions & FWVERTION_TEST) == FWVERTION_TEST && _FAIL == viRunFWVerTest())
        {
            ucSensorTestResult |= FWVERTION_TEST;
            ret = _FAIL;
            PRINTF("Error! Get FW Failed!!\n");
        }
        if ((inFunctions & MICROSHORT_TEST) == 1 && _FAIL == viRunShortTest())
        {
            ret = _FAIL;
            ucSensorTestResult |= MICROSHORT_TEST;
            PRINTF("Error! Get ShortDatas Failed!!\n");
        }

        usleep(200000);
        if ((inFunctions & OPEN_TEST) == 2 && _FAIL == viRunOpenTest())
        {
            ret = _FAIL;
            ucSensorTestResult |= OPEN_TEST;
            PRINTF("Error! Get OpenDatas Failed!!\n");
        }
        if ((inFunctions & MIRCO_OPEN_TEST) == 0x40 && _FAIL == viRunMircoOpenTest())
        {
            ucSensorTestResult |= MIRCO_OPEN_TEST;
            ret = _FAIL;
        }
        usleep(200000);
        if ((inFunctions & SELF_TEST) == 4 && _FAIL == viRunSelfTest())
        {
            ret = _FAIL;
            ucSensorTestResult |= SELF_TEST;
            PRINTF("Error! Get SelfDatas Failed!!\n");
        }

        usleep(200000);
        if ((inFunctions & DAC_TEST) == 8 && _FAIL == viRunDACTest())
        {
            ret = _FAIL;
            ucSensorTestResult |= DAC_TEST;
            PRINTF("Error! Get DACDatas Failed!!\n");
        }

        usleep(200000);
        if ((inFunctions & ALL_NODE_TEST) == 0x10 && _FAIL == viRunAllNodeTest())
        {
            ret = _FAIL;
            ucSensorTestResult |= ALL_NODE_TEST;
            PRINTF("Error! Get AllNodeDatas Failed!!\n");
        }

        usleep(200000);
        if ((inFunctions & UNIFORMITY_TEST) == 0x20 && _FAIL == viRunUniformityTest())
        {
            ret = _FAIL;
            ucSensorTestResult |= UNIFORMITY_TEST;
            PRINTF("Error! Get UniformityDatas Failed!!\n");
        }
        //----------------Print Result---------------
        vfPrintSensorTestResult(inFunctions);
    }
    else
    {
        PRINTF("Error! Get Base Infor Failed!!\n");
        ret = _FAIL;
    }

    viExitTestMode();
    return ret;
}

int viRunSensorTest_V6(int inFunctions)
{
    int ret = _SUCCESS;
    ucSensorTestResult = 0;
    time_t rawtime;
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );

    usleep(100000);
    if (viEnterTestMode() != _FAIL && viGetPanelInfor() != _FAIL && inFunctions > 0)
    {
        init_sentest_array();
        //read profile set sensor
        if (strlen(IniPath) != 0)
        {
            inFunctions = ReadST();
        }
        check_use_default_set();
        if(inFunctions == _FAIL) {
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
        //----------------Print Result---------------
        vfPrintSensorTestResult(inFunctions);
    }
    else
    {
        PRINTF("Error! Get Base Infor Failed!!\n");
        ret = _FAIL;
    }
END:
    viExitTestMode();
    return ret;
}
int ReadST()
{
    if (inProtocolStyle == _Protocol_V3_) {
        return ReadST_V3();
    }
    else if (inProtocolStyle == _Protocol_V6_) {
        return ReadST_V6();
    }
}
int ReadST_V3()
{
    //    int *ptr;
    int i;
    char buf[INI_MAX_PATH];
    int inFunctions = 0;

    //read ini test  Criteria
    memset(buf, 0, sizeof(buf));
    memset(g_szConfigPath, 0, sizeof(g_szConfigPath));
    //PRINTF("=>Get sensor test criteria\n");
    strcpy(g_szConfigPath, IniPath);
    if (strstr(IniPath, PROFILE_FORMAT_DAT) != NULL) {
        PRINTF("Profile file path %s\n", g_szConfigPath);
        PRINTF("ini path=%s\n",IniPath);

        GetIniKeyInt("Profile", "Version", g_szConfigPath);
        ST.PFVer3 = atoi(&tmpstr[0]);
        ST.PFVer2 = atoi(&tmpstr[2]);
        ST.PFVer1 = atoi(&tmpstr[4]);
        ST.PFVer0 = atoi(&tmpstr[6]);
        ST.UseNewFlow = GetIniKeyInt("Profile", "UseNewMPFlow", g_szConfigPath);
        PRINTF("%s,%d,UseNewFlow = %d\n", __func__, __LINE__, ST.UseNewFlow);
        ST.CreateGolden = GetIniKeyInt("Profile", "CreateGolden", g_szConfigPath);
        PRINTF("%s,%d,CreateGolden = %d\n", __func__, __LINE__, ST.CreateGolden);
        ST.OffsetValue = GetIniKeyInt("Profile", "OffsetValue", g_szConfigPath);
        PRINTF("%s,%d,OffsetValue = %d\n", __func__, __LINE__, ST.OffsetValue);
        ST.LogPath = GetIniKeyString("Profile", "LogPath", g_szConfigPath);
        if(inProtocolStyle != _Protocol_V3_)
            ST.UseNewFlow = 0;

        ST.fw_ver[0] = GetIniKeyInt("PannelInformation", "FWVersion0", g_szConfigPath);
        ST.fw_ver[1] = GetIniKeyInt("PannelInformation", "FWVersion1", g_szConfigPath);
        ST.fw_ver[2] = GetIniKeyInt("PannelInformation", "FWVersion2", g_szConfigPath);
        ST.fw_ver[3] = GetIniKeyInt("PannelInformation", "FWVersion3", g_szConfigPath);
        ST.fw_ver[4] = GetIniKeyInt("PannelInformation", "FWVersion4", g_szConfigPath);
        ST.fw_ver[5] = GetIniKeyInt("PannelInformation", "FWVersion5", g_szConfigPath);
        ST.fw_ver[6] = GetIniKeyInt("PannelInformation", "FWVersion6", g_szConfigPath);
        ST.fw_ver[7] = GetIniKeyInt("PannelInformation", "FWVersion7", g_szConfigPath);
        PRINTF("FWVersion:");
        for(i = 0; i < 8; i++)
            PRINTF("%2x.",ST.fw_ver[i]);
        PRINTF("\n");
        ST.dat_format = check_ini_section("TestItem", g_szConfigPath);
        PRINTF("%s,%d,dat_format = %d\n", __func__, __LINE__, ST.dat_format);
        ST.FWVersion_test = GetIniKeyInt("TestItem", "FWVersion", g_szConfigPath);
        if(ST.FWVersion_test == 1)
        {
            inFunctions += FWVERTION_TEST;
        }
        //--------------------------profile OpenShort test --------------------------//
        ST.Short.Threshold = GetIniKeyInt("TestItem", "TraceLoading_Window", g_szConfigPath);
        //ST. = GetIniKeyInt("TestItem", "TraceLoading_RefData_Key", g_szConfigPath); //profile exist, but daemon not use.
        ST.OpenShort_test = GetIniKeyInt("TestItem", "OpenShort", g_szConfigPath);
        if(ST.OpenShort_test == 1)
        {
            inFunctions += (OPEN_TEST) + (MICROSHORT_TEST);
        }
        ST.Open_Threshold = GetIniKeyInt("TestItem", "OpenShort_Threshold", g_szConfigPath);
        PRINTF("%s,%d,Open_Threshold = %d\n", __func__, __LINE__, ST.Open_Threshold);
        ST.Open_RX_Delta_Threshold = GetIniKeyInt("TestItem", "OpenShort_Threshold2", g_szConfigPath);
        PRINTF("%s,%d,Open_RX_Delta_Threshold = %d\n", __func__, __LINE__, ST.Open_RX_Delta_Threshold);
        ST.Open_RX_Continue_Fail_Tolerance = GetIniKeyInt("TestItem", "OpenShort_Threshold2_con", g_szConfigPath);
        PRINTF("%s,%d,OpenShort_Threshold2_con(Open_RX_Continue_Fail_Tolerance) = %d\n", __func__, __LINE__, ST.Open_RX_Continue_Fail_Tolerance);
        ST.Open_TX_Continue_Fail_Tolerance = GetIniKeyInt("TestItem", "OpenShort_Threshold3_con", g_szConfigPath);
        PRINTF("%s,%d,OpenShort_Threshold3_con(Open_TX_Continue_Fail_Tolerance) = %d\n", __func__, __LINE__, ST.Open_TX_Continue_Fail_Tolerance);
        if(ST.UseNewFlow == 1) {
            ST.Open_DCRangeMax = GetIniKeyInt("TestItem", "OpenShort_Threshold3", g_szConfigPath);
            PRINTF("%s,%d,OpenShort_Threshold3(Open_DCRangeMax) = %d\n", __func__, __LINE__, ST.Open_DCRangeMax);
            ST.Open_TX_Delta_Threshold = GetIniKeyInt("TestItem", "OpenShort_Threshold4", g_szConfigPath);
            PRINTF("%s,%d,OpenShort_Threshold4(Open_TX_Delta_Threshold) = %d\n", __func__, __LINE__, ST.Open_TX_Delta_Threshold);
        }
        else{
            ST.Open_TX_Delta_Threshold = GetIniKeyInt("TestItem", "OpenShort_Threshold3", g_szConfigPath);
            PRINTF("%s,%d,OpenShort_Threshold3(Open_TX_Delta_Threshold) = %d\n", __func__, __LINE__, ST.Open_TX_Delta_Threshold);
        }
        ST.Open_DCRangeMin = GetIniKeyInt("TestItem", "OpenShort_Threshold5", g_szConfigPath);
        //--------------------------profile Self test --------------------------//
        ST.SelfCapTest_test = GetIniKeyInt("TestItem", "SelfCapTest", g_szConfigPath);
        if(ST.SelfCapTest_test == 1)
        {
            inFunctions += SELF_TEST;
        }
        ST.Self_Maximum = GetIniKeyInt("TestItem", "SelfCapTest_Maximum", g_szConfigPath);
        ST.Self_Minimum = GetIniKeyInt("TestItem", "SelfCapTest_Minimum", g_szConfigPath);
        ST.Self_P2P = GetIniKeyInt("TestItem", "SelfCapTest_P2P", g_szConfigPath);
        ST.Self_P2P_Edge = GetIniKeyInt("TestItem", "SelfCapTest_P2P_Edge", g_szConfigPath);
        ST.Self_Frame_Count = GetIniKeyInt("TestItem", "SelfCapTest_Frame", g_szConfigPath);
        //--------------------------profile Allnode test --------------------------//
        ST.AllNode_test = GetIniKeyInt("TestItem", "AllNode", g_szConfigPath);
        if(ST.AllNode_test == 1) {
            inFunctions += ALL_NODE_TEST;
        }
        ST.AllNode_Delta_Threshold = GetIniKeyInt("TestItem", "AllNode_Window", g_szConfigPath);
        ST.AllNode_Panel_Tolerance = GetIniKeyInt("TestItem", "AllNode_Window2", g_szConfigPath);
        ST.AllNode_TX_Tolerance = GetIniKeyInt("TestItem", "AllNode_Window3", g_szConfigPath);
        ST.AllNode_Maximum = GetIniKeyInt("TestItem", "AllNode_XMax", g_szConfigPath);
        ST.AllNode_Minimum = GetIniKeyInt("TestItem", "AllNode_XMin", g_szConfigPath);
        if(ST.dat_format == true)
        {
            //--------------------------profile Uniformity test --------------------------//
            ST.Uniformity_test = GetIniKeyInt("TestItem","UniformityTest",g_szConfigPath);
            if(ST.Uniformity_test == 1)
            {
                inFunctions += UNIFORMITY_TEST;
            }
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
        if(ST.DAC_test == 1)
        {
            inFunctions += DAC_TEST;
        }
        ST.DAC_SC_P_Maximum = GetIniKeyInt("TestItem", "DACTest_SC_P_Max", g_szConfigPath);
        ST.DAC_SC_P_Minimum = GetIniKeyInt("TestItem", "DACTest_SC_P_Min", g_szConfigPath);
        ST.DAC_SC_N_Maximum = GetIniKeyInt("TestItem", "DACTest_SC_N_Max", g_szConfigPath);
        ST.DAC_SC_N_Minimum = GetIniKeyInt("TestItem", "DACTest_SC_N_Min", g_szConfigPath);
        ST.DAC_MC_P_Maximum = GetIniKeyInt("TestItem", "DACTest_MC_P_Max", g_szConfigPath);
        ST.DAC_MC_P_Minimum = GetIniKeyInt("TestItem", "DACTest_MC_P_Min", g_szConfigPath);
        ST.DAC_MC_N_Maximum = GetIniKeyInt("TestItem", "DACTest_MC_N_Max", g_szConfigPath);
        ST.DAC_MC_N_Minimum = GetIniKeyInt("TestItem", "DACTest_MC_N_Min", g_szConfigPath);
        if(ST.UseNewFlow == 1)
        {
            vfReadBenchMarkValue("YDriven",ST.BenchMark.iUniformityBenchMark,g_szConfigPath);
            vfReadBenchMarkValue("YDriven_Open",ST.BenchMark.iOpenBenchMark_0,g_szConfigPath);
            vfReadBenchMarkValue("YDriven_Open1",ST.BenchMark.iOpenBenchMark_1,g_szConfigPath);
        }
        PRINTF("Profile Version:%d.%d.%d.%d\n", ST.PFVer3,ST.PFVer2,ST.PFVer1,ST.PFVer0);

        if (ST.PFVer0 != 0)
        {
            NewVerFlag = 1;
        }

        PRINTF("NewVerFlag State:%d\n",NewVerFlag);
        PRINTF("inFunctions State:%d\n",inFunctions);
    }
    else if (strstr(IniPath, PROFILE_FORMAT_INI) != NULL) {
        inFunctions = ReadST_V6();
        //Because profile ini format Uniformity test is allnode test.
        if((inFunctions & UNIFORMITY_TEST) == UNIFORMITY_TEST) {
            inFunctions = inFunctions - UNIFORMITY_TEST + ALL_NODE_TEST;
            ST.AllNode_Delta_Threshold = ST.Uniformity.Min_Threshold;
            ST.AllNode_Panel_Tolerance = ST.Uniformity.Up_FailCount + ST.Uniformity.Low_FailCount;
            ST.AllNode_Maximum = ST.Uniformity.Max_Threshold;
        }
    }
    else {
        PRINTF("Profile fail, %s\n", g_szConfigPath);
    }
    return inFunctions;
}
double GetVerfMapping(uint8_t *tmp) {
    if(strcmp(tmp,"0.3") == 0 || strcmp(tmp," 0.3") == 0) {
        PRINTF("0.3\n");
        ST.Short.vref_s = 0xB;
        return 0.3;
    }
    else if(strcmp(tmp,"0.4") == 0 || strcmp(tmp," 0.4") == 0) {
        PRINTF("0.4\n");
        ST.Short.vref_s = 0xC;
        return 0.4;
    }
    else if(strcmp(tmp,"0.5") == 0 || strcmp(tmp," 0.5") == 0) {
        PRINTF("0.5\n");
        ST.Short.vref_s = 0xD;
        return 0.5;
    }
    else if(strcmp(tmp,"0.6") == 0 || strcmp(tmp," 0.6") == 0) {
        PRINTF("0.6\n");
        ST.Short.vref_s = 0xE;
        return 0.6;
    }
    else if(strcmp(tmp,"0.7") == 0 || strcmp(tmp," 0.7") == 0) {
        PRINTF("0.7\n");
        ST.Short.vref_s = 0xF;
        return 0.7;
    }
    else {
        PRINTF("0xFF\n");
        ST.Short.vref_s = 0xB;
        return _FAIL;
    }
}

int ReadST_V6()
{
    int i;
    char buf[INI_MAX_PATH], *tmp, section_fw[INI_MAX_PATH];
    int inFunctions = 0;

    tmp = calloc(1024, sizeof(uint8_t));
    //read ini test  Criteria
    memset(buf, 0, sizeof(buf));
    memset(g_szConfigPath, 0, sizeof(g_szConfigPath));
    //PRINTF("=>Get sensor test criteria\n");
    strcpy(g_szConfigPath, IniPath);
    PRINTF("Profile file path %s\n", g_szConfigPath);
    PRINTF("ini path=%s\n",IniPath);
    //[System]
    tmp = GetIniKeyString("System", "ProfileVersion", g_szConfigPath);
    sscanf(tmp, "%d.%d.%d.%d", &ST.PFVer0, &ST.PFVer1, &ST.PFVer2, &ST.PFVer3);
    printf("Profile Version:%s, %d.%d.%d.%d\n", tmp,ST.PFVer0, ST.PFVer1, ST.PFVer2, ST.PFVer3);
    ST.PFVer = (ST.PFVer0 << 24) + (ST.PFVer1 << 16) + (ST.PFVer2 << 8) + ST.PFVer3;
    PRINTF("Profile Version:0x%x\n", ST.PFVer);
    //[Panel_Info]
    ST.x_ch = GetIniKeyInt("Panel_Info", "XChannel", g_szConfigPath);
    ST.y_ch = GetIniKeyInt("Panel_Info", "YChannel", g_szConfigPath);
    if(ST.x_ch != ptl.x_ch || ST.y_ch != ptl.y_ch) {
        PRINTF("IC channel X_CH:%d Y_CH:%d, Profile channel X_CH:%d Y_CH:%d\n", ptl.x_ch, ptl.y_ch, ST.x_ch,  ST.y_ch);
        PRINTF("Channel not consistent\n");
        return _FAIL;
    }
    //[Report]
    tmp = GetIniKeyString("Report", "Path", g_szConfigPath);
    strcpy(ST.LogPath,tmp);
    if(access(ST.LogPath, F_OK | W_OK) == 0) {
        PRINTF("LogPath:%s\n", ST.LogPath);
    }
    else {
        ST.LogPath = getcwd(NULL, 0);
        PRINTF("[INI]LogPath no directory, use defaule path:%s\n", ST.LogPath);
    }
    //[FW_Upgrade]
    if(ST.PFVer < 0x1000105) {
        strcpy(section_fw, "FW_Upgrade");
    }
    else {
        strcpy(section_fw, "FW_Verify");
    }

    tmp = GetIniKeyString(section_fw, "Enable", g_szConfigPath);


    if(strcmp(tmp, "True") == 0) {
        ST.FWVersion_test = 1;
        inFunctions += FWVERTION_TEST;
        printf("FWVersion_test=%d,tmp:%s\n", ST.FWVersion_test, tmp);
        tmp = GetIniKeyString(section_fw, "FW_Ver", g_szConfigPath);
        if(ST.PFVer < PROFILE_V1_0_2_0) {
            sscanf(tmp, "V%x.%x.%x.%x", &ST.fw_ver[0], &ST.fw_ver[1], &ST.fw_ver[2], &ST.fw_ver[3]);
            printf("FW Version:%s, %x.%x.%x.%x\n", tmp,ST.fw_ver[0], ST.fw_ver[1], ST.fw_ver[2], ST.fw_ver[3]);
        }
        else {
            sscanf(tmp, "%x.%x.%x.%x.%x.%x.%x.%x", &ST.fw_ver[0], &ST.fw_ver[1], &ST.fw_ver[2], &ST.fw_ver[3]
                                                  , &ST.fw_ver[4], &ST.fw_ver[5], &ST.fw_ver[6], &ST.fw_ver[7]);
            printf("FW Version:%s, %x.%x.%x.%x.%x.%x.%x.%x\n", tmp, ST.fw_ver[0], ST.fw_ver[1], ST.fw_ver[2]
                                        , ST.fw_ver[3], ST.fw_ver[4], ST.fw_ver[5], ST.fw_ver[6], ST.fw_ver[7]);
        }
        ST.block_num = GetIniKeyInt(section_fw, "Block", g_szConfigPath);
        printf("ST.block_num=%d\n", ST.block_num);
        if(ST.block_num > 0) {
            tmp = GetIniKeyString(section_fw, "Master_CRC", g_szConfigPath);
            ST.master_crc = calloc(ST.block_num, sizeof(unsigned short));
            char *Tbuf = strtok(tmp, ",");
            i = 0;
            while (Tbuf != NULL)
            {
                sscanf(Tbuf,"%x", (unsigned int *)&ST.master_crc[i]);
                PRINTF("INI CRC[%d]:0x%x\n", i, ST.master_crc[i]);
                i++;
                Tbuf = strtok( NULL, ",");
            }
        }
        ST.slave_num = GetIniKeyInt(section_fw, "Slave_number", g_szConfigPath);
        if(ST.slave_num > 0) {
            tmp = GetIniKeyString(section_fw, "Slave_CRC", g_szConfigPath);
            ST.slave_crc = calloc(ST.slave_num, sizeof(unsigned short));
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
        ST.hexfile = calloc(1024, sizeof(char));
        strcpy(ST.hexfile,tmp);
    }
    //[Short_Test]
    tmp = GetIniKeyString("Short_Test", "Enable", g_szConfigPath);
    if(strcmp(tmp, "True") == 0) {
        ST.Short_test = ENABLE_TEST;
        inFunctions += MICROSHORT_TEST;
        printf("Short_test=%d,tmp:%s\n", ST.Short_test, tmp);
        ST.Short.Threshold = GetIniKeyInt("Short_Test", "Max_Threshold", g_szConfigPath);
        ST.Short.FrameCount = GetIniKeyInt("Short_Test", "Frame_Count", g_szConfigPath);
        ST.Short.dump1 = GetIniKeyInt("Short_Test", "Dump1", g_szConfigPath);
        ST.Short.dump2 = GetIniKeyInt("Short_Test", "Dump2", g_szConfigPath);
        ST.Short.posidleL = GetIniKeyInt("Short_Test", "Short_PostIdle_L", g_szConfigPath);
        ST.Short.posidleH = GetIniKeyInt("Short_Test", "Short_PostIdle_H", g_szConfigPath);
        ST.Short.vref_v = GetVerfMapping(GetIniKeyString("Short_Test", "VrefL", g_szConfigPath));
        ST.Short.keyTx_thr = GetIniKeyInt("Short_Test", "KeyTX_Threshold", g_szConfigPath);
        ST.Short.keyRx_thr = GetIniKeyInt("Short_Test", "KeyRX_Threshold", g_szConfigPath);
    }
    printf("Short_Test=%s\n", ST.Short_test ? "True" : "False");
    //[Open_Test]
    tmp = GetIniKeyString("Open_Test", "Enable", g_szConfigPath);
    if(strcmp(tmp, "True") == 0) {
        ST.Open_test = ENABLE_TEST;
        printf("Open_test=%d,tmp:%s\n", ST.Open_test, tmp);
        inFunctions += OPEN_TEST;
        ST.Open_Threshold = GetIniKeyInt("Open_Test", "Min_Threshold", g_szConfigPath);
        ST.Open.key_thr = GetIniKeyInt("Open_Test", "Key_Threshold", g_szConfigPath);
        ST.Open_FrameCount = GetIniKeyInt("Open_Test", "Frame_Count", g_szConfigPath);
        ST.Open_TX_Aver_Diff = GetIniKeyInt("Open_Test", "TX_Average_Diff_Gap", g_szConfigPath);
        tmp = GetIniKeyString("Open_Test", "TX_Average_Diff_Gap_AvoidCorner", g_szConfigPath);
        if(strcmp(tmp, "True") == 0)
            ST.Open_TX_Aver_Diff_Gap_Corner = true;
        PRINTF("TX_Average_Diff_Gap_AvoidCorner=%s\n", ST.Open_TX_Aver_Diff_Gap_Corner ? "True" : "False");
        ST.Open_RX_Delta_Threshold = GetIniKeyInt("Open_Test", "RX_Diff_Gap", g_szConfigPath);
        ST.Open_RX_Continue_Fail_Tolerance = GetIniKeyInt("Open_Test", "RX_Diff_Gap_Tolerance", g_szConfigPath);
        ST.Open.freq = GetIniKeyInt("Open_Test", "Frequency", g_szConfigPath);
        if(ST.Open.freq < 0)
            ST.Open.freq = 100;
        ST.Open.gain = GetIniKeyInt("Open_Test", "Gain", g_szConfigPath);
        if(ST.Open.gain < 0)
            ST.Open.gain = 0;
    }
    printf("Open_Test=%s\n", ST.Open_test ? "True" : "False");
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
        PRINTF("RX_Delta_En=%s,%d\n", ST.MOpen.RxDeltaEn ? "True" : "False" , ST.MOpen.RxDeltaEn);
        tmp = GetIniKeyString("MicroOpen_Test", "TX_Avg_Delta_En", g_szConfigPath);
        if(strcmp(tmp, "True") == 0) {
            ST.MOpen.TxAvgEn = ENABLE_TEST;
            vfReadBenchMarkValue_V6("TX_Avg_Delta", g_szConfigPath, ST.MOpen.tx_avg, 1, ptl.y_ch - 1);
            ST.MOpen.TxAvgThr = GetIniKeyInt("MicroOpen_Test", "TX_Avg_Delta_Threshold", g_szConfigPath);
            tmp = GetIniKeyString("MicroOpen_Test", "TX_Avg_Delta_Threshold_AvoidCorner", g_szConfigPath);
            if(strcmp(tmp, "True") == 0)
                ST.MOpen.TxAvgCorner = ENABLE_TEST;
            PRINTF("TX_Avg_Delta_Threshold_AvoidCorner=%s\n", ST.MOpen.TxAvgCorner ? "True" : "False");
        }
        PRINTF("TX_Avg_Delta_En=%s\n", ST.MOpen.TxAvgEn ? "True" : "False");
        ST.MOpen.RxToles = GetIniKeyInt("MicroOpen_Test", "RX_Delta_Threshold_Tolerance", g_szConfigPath);
    }
    printf("MicroOpen_Test=%s\n", ST.MOpen.En ? "True" : "False");
    //[Uniformity_Test]
    tmp = GetIniKeyString("Uniformity_Test", "Enable", g_szConfigPath);
    if(strcmp(tmp, "True") == 0) {
        ST.Uniformity_test = ENABLE_TEST;
        inFunctions += UNIFORMITY_TEST;
        printf("Uniformity_test=%d,tmp:%s\n", ST.Uniformity_test, tmp);
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
            if(strcmp(tmp, "True") == 0) {
                ST.Uniformity.En_bench = ENABLE_TEST;
                vfReadBenchMarkValue_V6("Uniformity_Benchmark", g_szConfigPath, ST.Uniformity.bench, ptl.x_ch, ptl.y_ch);
            }
            tmp = GetIniKeyString("Uniformity_Test", "Uniformity_RawData_En", g_szConfigPath);
            if(strcmp(tmp, "True") == 0) {
                ST.Uniformity.En_allraw = ENABLE_TEST;
                vfReadBenchMarkValue_V6("Uniformity_RawData", g_szConfigPath, ST.Uniformity.allraw, ptl.x_ch, ptl.y_ch);
            }
            tmp = GetIniKeyString("Uniformity_Test", "Uniformity_Win1_En", g_szConfigPath);
            if(strcmp(tmp, "True") == 0) {
                ST.Uniformity.En_allwin1 = ENABLE_TEST;
                vfReadBenchMarkValue_V6("Uniformity_Win1", g_szConfigPath, ST.Uniformity.allwin1, ptl.x_ch, ptl.y_ch - 1);
            }
            tmp = GetIniKeyString("Uniformity_Test", "Uniformity_Win2_En", g_szConfigPath);
            if(strcmp(tmp, "True") == 0) {
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
            if(strcmp(tmp, "True") == 0) {
                ST.Uniformity.En_bench = ENABLE_TEST;
                vfReadBenchMarkValue_V6("Uniformity_Benchmark", g_szConfigPath, ST.Uniformity.bench, ptl.x_ch, ptl.y_ch);
            }
            tmp = GetIniKeyString("Uniformity_Test", "AllNode_RawData_En", g_szConfigPath);
            if(strcmp(tmp, "True") == 0) {
                ST.Uniformity.En_allraw = ENABLE_TEST;
                vfReadBenchMarkValue_V6("AllNode_RawData", g_szConfigPath, ST.Uniformity.allraw, ptl.x_ch, ptl.y_ch);
            }
            tmp = GetIniKeyString("Uniformity_Test", "AllNode_Win1_En", g_szConfigPath);
            if(strcmp(tmp, "True") == 0) {
                ST.Uniformity.En_allwin1 = ENABLE_TEST;
                vfReadBenchMarkValue_V6("AllNode_Win1", g_szConfigPath, ST.Uniformity.allwin1, ptl.x_ch, ptl.y_ch - 1);
            }
            tmp = GetIniKeyString("Uniformity_Test", "AllNode_Win2_En", g_szConfigPath);
            if(strcmp(tmp, "True") == 0) {
                ST.Uniformity.En_allwin2 = ENABLE_TEST;
                vfReadBenchMarkValue_V6("AllNode_Win2", g_szConfigPath, ST.Uniformity.allwin2, ptl.x_ch - 1, ptl.y_ch - 1);
            }
        }

    }
    //free(tmp);
    return inFunctions;
}
#endif

