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

	for (i = 0; i < count; i++)
		outbuf[i] = i;

	for (i = 0; i < count; i++) {
		for (j = 0; j < count -1; j++) {
			if (inbuf[j] > inbuf[j+1]) {
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
	uint16_t *min, *tmp;
	int addr_start = 0;
	int addr_end = 0;

	LD_MSG("[%s] Fre Datas\n", name);
	LD_MSG("[%s] Start:%d End:%d Step=%d, data length:%d\n",
		name, freq.start, freq.end, freq.step, freq.len);

	min = (uint16_t *)calloc(freq.len, sizeof(uint16_t));
	tmp = (uint16_t *)calloc(freq.len, sizeof(uint16_t));
	if (!min || !tmp)
		return;

	for (bit = 0; bit < 8; bit++) {
		if (scan_data & (0x1 << bit)) {
			if (bit == 0) {
				count++;
				LD_MSG("Max Value\n");
			} else if (bit == 1) {
				count++;
				LD_MSG("Average Value\n");
			} else if (bit == 2) {
				count++;
				LD_MSG("Frame Count\n");
			}

			// count -1 beacase index must is 0.
			addr_start = (count - 1) * freq.len + start;
			addr_end = addr_start + freq.len;
			LD_MSG("addr_start:%d, addr_end:%d\n",
				addr_start, addr_end - 1);
			for (i = addr_start, j = 0; i < addr_end; i++, j++) {
				freq.band[j].data = uiTestDatas[i/ptl.x_ch][i%ptl.x_ch];
				freq.band[j].freq = freq.start + (freq.step * j);
				tmp[j] = freq.band[j].data;
				LD_MSG("%d,", freq.band[j].data);
			}
			LD_MSG("\n");

			merge_sort(tmp, min, freq.len);
			LD_MSG("The Min1 Value is: %d, Fre is:%d \n",
				freq.band[min[0]].data, freq.band[min[0]].freq);
			LD_MSG("The Min2 Value is: %d, Fre is:%d \n",
				freq.band[min[1]].data, freq.band[min[1]].freq);
			LD_MSG("The Min3 Value is: %d, Fre is:%d \n",
				freq.band[min[2]].data, freq.band[min[2]].freq);
		}
	}
	LD_MSG("\n");

	free(min);
	free(tmp);
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

	ModeCtrl_V6(ENTER_SUSPEND_MODE, ENABLE_ENGINEER, 100);
	viGetPanelInfor();
	if (ICMode == 0x55) {
		ret = _FAIL;
		goto END;
	}

	mc_sine.start = atoi(argv[6]);
	mc_sine.end = atoi(argv[7]);
	mc_sine.step = atoi(argv[8]);
	mc_swcap.start = atoi(argv[9]);
	mc_swcap.end = atoi(argv[10]);
	mc_swcap.step = atoi(argv[11]);
	sc_swcap.start = atoi(argv[12]);
	sc_swcap.end = atoi(argv[13]);
	sc_swcap.step = atoi(argv[14]);
	if (ptl.ver >= PROTOCOL_V6_0_2) {
		if(argv[15] == NULL) {
			ret = _FAIL;
			LD_ERR("Enter command error\n");
			goto END;
		}

		frame_num = atoi(argv[15]);
		scan_data = atoi(argv[16]);
		count = 0;  //init count
		for(i = 0; i < 8; i++)
			if(scan_data & (0x1 << i))
				count++;
		delay_count = frame_num * 3;
	} else {
		scan_data = 1;
		delay_count = 30;
	}
	ret = ModeCtrl_V6(ENTER_TEST_MODE, DISABLE_ENGINEER, 100);

	mc_sine.len = (abs(mc_sine.end - mc_sine.start) / mc_sine.step) + 1;
	mc_sine.band = (FREQUENCY_BAND_DATA *)calloc(mc_sine.len * count, sizeof(FREQUENCY_BAND_DATA));
	mc_swcap.len = (abs(mc_swcap.end - mc_swcap.start) / mc_swcap.step) + 1;
	mc_swcap.band = (FREQUENCY_BAND_DATA *)calloc(mc_swcap.len * count,
			       sizeof(FREQUENCY_BAND_DATA));
	sc_swcap.len = (abs(sc_swcap.end - sc_swcap.start) / sc_swcap.step) + 1;
	sc_swcap.band = (FREQUENCY_BAND_DATA *)calloc(sc_swcap.len * count,
			       sizeof(FREQUENCY_BAND_DATA));

	if (!mc_sine.band || !mc_swcap.band || !sc_swcap.band) {
		LD_ERR("buffer allocate failed, ret: %d\n", errno);
		ret = _FAIL;
		goto END;
	}

	LD_MSG("MC Sine Start:%d End:%d Step=%d, data length:%d\n",
		mc_sine.start, mc_sine.end, mc_sine.step, mc_sine.len);
	LD_MSG("MC SWCap Start:%d End:%d Step=%d, data length:%d\n",
		mc_swcap.start, mc_swcap.end, mc_swcap.step, mc_swcap.len);
	LD_MSG("SC SWCap Start:%d End:%d Step=%d, data length:%d\n",
		sc_swcap.start, sc_swcap.end, sc_swcap.step, sc_swcap.len);
	LD_MSG("Frame number = %d, Scan data Return = 0x%x\n",
		frame_num, scan_data);

	if (mc_sine.start < 20 || mc_sine.end > 280 ||
	    mc_sine.step < 1 || mc_sine.step > 5) {
		LD_ERR("MC Sine range 20<=>280, Step 1<=>5\n");
		LD_ERR("Enter command error\n");
		ret = _FAIL;
		goto END;
	}
	if (mc_swcap.start < 4 || mc_swcap.end > 300 ||
	    mc_swcap.step < 2 || mc_swcap.step > 10) {
		LD_ERR("MC Sine range 4<=>300, Step 2<=>10\n");
		LD_ERR("Enter command error\n");
		ret = _FAIL;
		goto END;
	}
	if (sc_swcap.start < 4 || sc_swcap.end > 300 ||
	    sc_swcap.step < 2 || sc_swcap.step > 10) {
		LD_ERR("MC Sine range 4<=>300, Step 2<=>10\n");
		LD_ERR("Enter command error\n");
		ret = _FAIL;
		goto END;
	}

	SetFsInfo(mc_sine.start, mc_sine.end, mc_sine.step,
		  mc_swcap.start, mc_swcap.end, mc_swcap.step,
		  sc_swcap.start, sc_swcap.end, sc_swcap.step,
		  frame_num, scan_data);
	d_len = (mc_sine.len + mc_swcap.len + sc_swcap.len) * count;
	LD_MSG("total len:%d, count:%d\n", d_len, count);
	if (viInitRawData_6X(TEST_MODE_V6_SET_FREQ, delay_count) != _FAIL) {
		if (viGetRawData_6X(d_len * 2, NULL) != _FAIL) {
			ret = _SUCCESS;
		} else {
			LD_ERR("Error! Get RawData Failed!\n");
			ret = _FAIL;
		}
	} else {
		LD_ERR("Error! Init RawData Failed!\n");
		ret = _FAIL;
	}
	print_freq_log("MC Sine", mc_sine, scan_data, 0);
	print_freq_log("MC SWCap", mc_swcap, scan_data, mc_sine.len * count);
	print_freq_log("SC SWCap", sc_swcap, scan_data,
		       (mc_sine.len + mc_swcap.len) * count);

END:
	ModeCtrl_V6(ENTER_NORMAL_MODE, DISABLE_ENGINEER, 1000);

	free(mc_sine.band);
	free(mc_swcap.band);
	free(sc_swcap.band);

	return ret;
}


int viRunFre_3X(int inStartValue, int inEndValue, int inStep)
{
	int ret = _SUCCESS;
	int inCounts = 0,backcount = 0;
	int inTotalCounts = 0;
	unsigned char ucMin1 = 0xFF, ucMin2 = 0xFF, ucMin3 = 0xFF;
	int inCurrentFre = inStartValue;
	int inTargeFre1 = 0, inTargeFre2 = 0, inTargeFre3 = 0;

	inTotalCounts = ((((inEndValue - inStartValue) * 10) / inStep) + 1);
	if (inStartValue >= inEndValue)
	{
		LD_ERR("Error! EndValue Must Bigger Than StartValue!\n");
		ret = _FAIL;
	}

	else if (inEndValue > 90 || inStartValue < 30)
	{
		LD_ERR("Error! Support frequency range is 30<=>90\n");
		ret = _FAIL;
	}

	else if (inTotalCounts > _MaxChanelNum_)
	{
		LD_ERR("Error! The number of frequency must be less than 256 \n");
		ret = _FAIL;
	}
	if (ret == _FAIL)
	{
		return ret;
	}
	if (EnterTestMode(100) != _FAIL && viGetPanelInfor() != _FAIL)
	{
		if (ICMode == 0x55) {
			ret = _FAIL;
			goto END;
		}

		if (viInitFre_3X(inStartValue, inEndValue, inStep) != _FAIL)
		{
			if (viGetRawData_3X(0xE4, _FastMode_, inTotalCounts, _DataFormat_8_Bit_, inTotalCounts, NULL) != _FAIL)
			{
				ret = _SUCCESS;
			}
			else
			{
				LD_ERR("Error! Get FreData Failed!\n");
				ret = _FAIL;
			}
		}
		else
		{
			LD_ERR("Error! Init FreData Failed!\n");
			ret = _FAIL;
		}

		if (ret != _FAIL)
		{
			LD_MSG("Fre Datas\n");
			inCurrentFre = inEndValue * 10;
			for (inCounts = 0 , backcount = inTotalCounts - 1; inCounts < inTotalCounts; backcount--, inCounts++)
			{
				LD_MSG("%3d,", uiTestDatas[0][inCounts]);
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
				//LD_MSG("backcount:%d,min1:%d,min2:%d,min3:%d\n" ,backcount, ucMin1, ucMin2, ucMin3);
			}
			LD_MSG("\n\n");

			if (KernelVersion[0] >= 0x10 && KernelVersion[1] == 0x25)
			{
				//iTXPW=2*240000/iFre-1;
				LD_MSG("The Min1 Value is: %d, Fre is:%d (100*Hz), TXPW is:%d\n", ucMin1, inTargeFre1, 2 * 240000 / inTargeFre1 - 1);
				LD_MSG("The Min2 Value is: %d, Fre is:%d (100*Hz), TXPW is:%d\n", ucMin2, inTargeFre2, 2 * 240000 / inTargeFre2 - 1);
				LD_MSG("The Min3 Value is: %d, Fre is:%d (100*Hz), TXPW is:%d\n", ucMin3, inTargeFre3, 2 * 240000 / inTargeFre3 - 1);
			}
			else
			{
				//iTXPW=2*240000/iFre-1;
				LD_MSG("The Min1 Value is: %d, Fre is:%d (100*Hz), TXPW is:%d\n", ucMin1, inTargeFre1, 180000 / inTargeFre1 - 1);
				LD_MSG("The Min2 Value is: %d, Fre is:%d (100*Hz), TXPW is:%d\n", ucMin2, inTargeFre2, 180000 / inTargeFre2 - 1);
				LD_MSG("The Min3 Value is: %d, Fre is:%d (100*Hz), TXPW is:%d\n", ucMin3, inTargeFre3, 180000 / inTargeFre3 - 1);
			}
			LD_MSG("\n\n");
		}
	}
	else
	{
		LD_ERR("Error! Get Base Infor error!\n");
		ret = _FAIL;
	}
END:
	ExitTestMode(100);
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

