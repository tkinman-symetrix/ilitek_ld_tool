/******************** (C) COPYRIGHT 2019 ILI Technology Corp. ********************
* File Name :   IliTek_Frequency.c
* Description   :   Frequency function
*
********************************************************************************
*History:
*   Version        Date           Author            Description
*   --------------------------------------------------------------------------
*      1.0       2019/02/15          Randy           Initial version
*******************************************************************************/

#ifndef _ILITEK_FREQUENCY_C_
#define _ILITEK_FREQUENCY_C_

/* Includes of headers ------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include "../ILITek_CMDDefine.h"
#include "../ILITek_Device.h"
#include "ILITek_RawData.h"
#include "ILITek_Frequency.h"
#include "../ILITek_Protocol.h"
#include "../ILITek_Main.h"
/* Private define ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

int viInitFre_3X(unsigned char inStartValue, unsigned char inEndValue, unsigned char inStep)
{
    int ret = _SUCCESS;
    uint8_t Wbuff[64] = {0};

    Wbuff[0] = 0xF3;
    Wbuff[1] = 0x0F;
    Wbuff[2] = inStartValue;
    Wbuff[3] = inEndValue;
    Wbuff[4] = inStep;
    ret = TransferData(Wbuff, 5, NULL, 0, 1000);
    usleep(200000);
    if (ret != _FAIL)
    {
        ret = CheckBusy(600, 10, NO_NEED);
    }
    return ret;
}

void merge_sort(unsigned short *inbuf, unsigned short *outbuf, int count) {
    unsigned short tmp = 0, tmp1 = 0;
    int i, j;

    for(i = 0; i < count; i++) {
        outbuf[i] = i;
    }
    for(i = 0; i < count; i++) {
        for(j = 0; j < count -1; j++) {
            if(inbuf[j] > inbuf[j+1]) {
                tmp = inbuf[j];
                tmp1 = outbuf[j];
                inbuf[j] = inbuf[j+1];
                outbuf[j] = outbuf[j+1];
                inbuf[j+1] = tmp;
                outbuf[j+1] = tmp1;
            }
        }
    }
}

void print_freq_log(const char *name, FREQUENCY_SET_DATA freq, int scan_data, int start) {
    int i = 0, j = 0, count = 0, bit = 0;
    uint16_t min[256] = {0}, tmp[256] = {0};
    int addr_start = 0;
    int addr_end = 0;

    PRINTF("%s Fre Datas\n", name);
    printf("%s Start:%d End:%d Step=%d, data length:%d\n", name, freq.start, freq.end, freq.step, freq.len);
    for(bit = 0; bit < 8; bit++) {
        if(scan_data & (0x1 << count)) {
            if(bit == 0) {
                count++;
                printf("Max Value\n");
            }
            else if (bit == 1) {
                count++;
                printf("Average Value\n");
            }
            else if (bit == 2) {
                count++;
                printf("Frame Count\n");
            }
            // count -1 beacase index must is 0.
            addr_start = (count - 1) * freq.len + start;
            addr_end = addr_start + freq.len;
            printf("addr_start:%d addr_end:%d\n", addr_start, addr_end - 1);
            for(i = addr_start, j = 0; i < addr_end; i++, j++) {
                freq.band[j].data = uiTestDatas[i/ptl.x_ch][i%ptl.x_ch];
                freq.band[j].freq = freq.start + (freq.step * j);
                tmp[j] = freq.band[j].data;
                PRINTF("%d,", freq.band[j].data);
            }
            PRINTF("\n");
            merge_sort(tmp, min, freq.len);
            PRINTF("The Min1 Value is: %d, Fre is:%d \n", freq.band[min[0]].data, freq.band[min[0]].freq);
            PRINTF("The Min2 Value is: %d, Fre is:%d \n", freq.band[min[1]].data, freq.band[min[1]].freq);
            PRINTF("The Min3 Value is: %d, Fre is:%d \n", freq.band[min[2]].data, freq.band[min[2]].freq);
        }
    }
    PRINTF("\n");
}

int viRunFre_6X(char *argv[])
{
    int ret = _SUCCESS, i = 0;
    int d_len = 0, delay_count = 10;
    FREQUENCY_SET_DATA mc_sine;
    FREQUENCY_SET_DATA mc_swcap;
    FREQUENCY_SET_DATA sc_swcap;
    uint16_t frame_num = 0;
    uint8_t scan_data = 0;
    int count = 1;

    ModeCtrl_V6(ENTER_SUSPEND_MODE, ENABLE_ENGINEER);
    viGetPanelInfor();
    mc_sine.start = atoi(argv[6]);
    mc_sine.end = atoi(argv[7]);
    mc_sine.step = atoi(argv[8]);
    mc_swcap.start = atoi(argv[9]);
    mc_swcap.end = atoi(argv[10]);
    mc_swcap.step = atoi(argv[11]);
    sc_swcap.start = atoi(argv[12]);
    sc_swcap.end = atoi(argv[13]);
    sc_swcap.step = atoi(argv[14]);
    if(ptl.ver >= PROTOCOL_V6_0_2) {
        if(argv[15] == NULL) {
            ret = _FAIL;
            PRINTF("Enter command error\n");
            goto END;
        }

        frame_num = atoi(argv[15]);
        scan_data = atoi(argv[16]);
        count = 0;  //init count
        for(i = 0; i < 8; i++)
            if(scan_data & (0x1 << i))
                count++;
        delay_count = frame_num * 3;
    }
    else {
        scan_data = 1;
        delay_count = 30;
    }
    ret = ModeCtrl_V6(ENTER_TEST_MODE, DISABLE_ENGINEER);

    mc_sine.len = (abs(mc_sine.end - mc_sine.start) / mc_sine.step) + 1;
    mc_sine.band = (FREQUENCY_BAND_DATA *)calloc(mc_sine.len * count, sizeof(FREQUENCY_BAND_DATA));
    mc_swcap.len = (abs(mc_swcap.end - mc_swcap.start) / mc_swcap.step) + 1;
    mc_swcap.band = (FREQUENCY_BAND_DATA *)calloc(mc_swcap.len * count, sizeof(FREQUENCY_BAND_DATA));
    sc_swcap.len = (abs(sc_swcap.end - sc_swcap.start) / sc_swcap.step) + 1;
    sc_swcap.band = (FREQUENCY_BAND_DATA *)calloc(sc_swcap.len * count, sizeof(FREQUENCY_BAND_DATA));
    printf("MC Sine Start:%d End:%d Step=%d, data length:%d\n", mc_sine.start, mc_sine.end, mc_sine.step, mc_sine.len);
    printf("MC SWCap Start:%d End:%d Step=%d, data length:%d\n", mc_swcap.start, mc_swcap.end, mc_swcap.step, mc_swcap.len);
    printf("SC SWCap Start:%d End:%d Step=%d, data length:%d\n", sc_swcap.start, sc_swcap.end, sc_swcap.step, sc_swcap.len);
    printf("Frame number = %d, Scan data Return = 0x%x\n", frame_num, scan_data);
    if(mc_sine.start < 20 || mc_sine.end > 280 || mc_sine.step < 1 || mc_sine.step > 5) {
        PRINTF("MC Sine range 20<=>280, Step 1<=>5\n");
        ret = _FAIL;
        PRINTF("Enter command error\n");
        goto END;
    }
    if(mc_swcap.start < 4 || mc_swcap.end > 300 || mc_swcap.step < 2 || mc_swcap.step > 10) {
        PRINTF("MC Sine range 4<=>300, Step 2<=>10\n");
        ret = _FAIL;
        PRINTF("Enter command error\n");
        goto END;
    }
    if(sc_swcap.start < 4 || sc_swcap.end > 300 || sc_swcap.step < 2 || sc_swcap.step > 10) {
        PRINTF("MC Sine range 4<=>300, Step 2<=>10\n");
        ret = _FAIL;
        PRINTF("Enter command error\n");
        goto END;
    }
    SetFsInfo(mc_sine.start, mc_sine.end, mc_sine.step, mc_swcap.start, mc_swcap.end,
        mc_swcap.step, sc_swcap.start, sc_swcap.end, sc_swcap.step, frame_num, scan_data);
    d_len = (mc_sine.len + mc_swcap.len + sc_swcap.len) * count;
    printf("total len:%d, count:%d\n", d_len, count);
    if (viInitRawData_6X(TEST_MODE_V6_SET_FREQ, delay_count) != _FAIL)
    {
        if (viGetRawData_6X(d_len * 2) != _FAIL)
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
    print_freq_log("MC Sine", mc_sine, scan_data, 0);

    print_freq_log("MC SWCap", mc_swcap, scan_data, mc_sine.len * count);

    print_freq_log("SC SWCap", sc_swcap, scan_data, (mc_sine.len + mc_swcap.len) * count);
END:
    ModeCtrl_V6(ENTER_NORMAL_MODE, DISABLE_ENGINEER);
    return ret;
}


int viRunFre_3X(int inStartValue, int inEndValue, int inStep)
{
    int ret = _SUCCESS;
    int inCounts = 0,backcount = 0;
    int inTotalCounts = 0;
    unsigned char ucMin1 = 0xFF, ucMin2 = 0xFF, ucMin3 = 0xFF;
    int inCurrentFre = inStartValue;
    int inTargeFre1, inTargeFre2, inTargeFre3;

    inTotalCounts = ((((inEndValue - inStartValue) * 10) / inStep) + 1);
    if (inStartValue >= inEndValue)
    {
        PRINTF("Error! EndValue Must Bigger Than StartValue!\n");
        ret = _FAIL;
    }

    else if (inEndValue > 90 || inStartValue < 30)
    {
        PRINTF("Error! Support frequency range is 30<=>90\n");
        ret = _FAIL;
    }

    else if (inTotalCounts > _MaxChanelNum_)
    {
        PRINTF("Error! The number of frequency must be less than 256 \n");
        ret = _FAIL;
    }   
    if (ret == _FAIL)
    {
        return ret;
    }
    if (EnterTestMode() != _FAIL && viGetPanelInfor() != _FAIL)
    {
        if (viInitFre_3X(inStartValue, inEndValue, inStep) != _FAIL)
        {
            if (viGetRawData_3X(0xE4, _FastMode_, inTotalCounts, _DataFormat_8_Bit_, inTotalCounts) != _FAIL)
            {
                ret = _SUCCESS;
            }
            else
            {
                PRINTF("Error! Get FreData Failed!\n");
                ret = _FAIL;
            }
        }
        else
        {
            PRINTF("Error! Init FreData Failed!\n");
            ret = _FAIL;
        }

        if (ret != _FAIL)
        {
            PRINTF("Fre Datas\n");
            inCurrentFre = inEndValue * 10;
            for (inCounts = 0 , backcount = inTotalCounts - 1; inCounts < inTotalCounts; backcount--, inCounts++)
            {
                PRINTF("%3d,", uiTestDatas[0][inCounts]);
                if (uiTestDatas[0][backcount] < ucMin1)
                {
                    ucMin1 = uiTestDatas[0][backcount];
                    inTargeFre1 = inCurrentFre;
                }
                else if (uiTestDatas[0][backcount] < ucMin2)
                {
                    ucMin2 = uiTestDatas[0][backcount];
                    inTargeFre2 = inCurrentFre;
                }
                else if (uiTestDatas[0][backcount] < ucMin3)
                {
                    ucMin3 = uiTestDatas[0][backcount];
                    inTargeFre3 = inCurrentFre;
                }
                inCurrentFre -= inStep;
               //PRINTF("backcount:%d,min1:%d,min2:%d,min3:%d\n" ,backcount, ucMin1, ucMin2, ucMin3);
            }
            PRINTF("\n\n");

            if (KernelVersion[0] >= 0x10 && KernelVersion[1] == 0x25)
            {
                //iTXPW=2*240000/iFre-1;
                PRINTF("The Min1 Value is: %d, Fre is:%d (100*Hz), TXPW is:%d\n", ucMin1, inTargeFre1, 2 * 240000 / inTargeFre1 - 1);
                PRINTF("The Min2 Value is: %d, Fre is:%d (100*Hz), TXPW is:%d\n", ucMin2, inTargeFre2, 2 * 240000 / inTargeFre2 - 1);
                PRINTF("The Min3 Value is: %d, Fre is:%d (100*Hz), TXPW is:%d\n", ucMin3, inTargeFre3, 2 * 240000 / inTargeFre3 - 1);
            }
            else
            {
                //iTXPW=2*240000/iFre-1;
                PRINTF("The Min1 Value is: %d, Fre is:%d (100*Hz), TXPW is:%d\n", ucMin1, inTargeFre1, 180000 / inTargeFre1 - 1);
                PRINTF("The Min2 Value is: %d, Fre is:%d (100*Hz), TXPW is:%d\n", ucMin2, inTargeFre2, 180000 / inTargeFre2 - 1);
                PRINTF("The Min3 Value is: %d, Fre is:%d (100*Hz), TXPW is:%d\n", ucMin3, inTargeFre3, 180000 / inTargeFre3 - 1);
            }
            PRINTF("\n\n");
        }
    }
    else
    {
        PRINTF("Error! Get Base Infor error!\n");
        ret = _FAIL;
    }
    ExitTestMode();
    return ret;
}

int viRunFre(char *argv[])
{
    int ret = 1;
    if (inProtocolStyle == _Protocol_V3_)
    {
        ret = viRunFre_3X(atoi(argv[6]), atoi(argv[7]), atoi(argv[8]));
    }
    else if (inProtocolStyle == _Protocol_V6_)
    {
        ret = viRunFre_6X(argv);
    }
    return ret;
}

#endif

