
/******************** (C) COPYRIGHT 2019 ILI Technology Corp. ********************
 * File Name :   IliTek_Frequency.h
 * Description   :   Header for IliTek_Frequency.c file
 *
 ********************************************************************************
 *History:
 *   Version        Date           Author            Description
 *   --------------------------------------------------------------------------
 *      1.0       2019/02/15          Randy           Initial version
 *******************************************************************************/

#ifndef _ILITEK_FREQUENCY_H_
#define _ILITEK_FREQUENCY_H_

/* Includes of headers ------------------------------------------------------*/
/* Extern define ------------------------------------------------------------*/
#define FREQ_FORMAT_MAX             1 << 0
#define FREQ_FORMAT_AVERAGE         1 << 1
#define FREQ_FORMAT_FRAME           1 << 2
/* Extern typedef -----------------------------------------------------------*/
typedef struct _FREQUENCY_BAND_DATA_ {
	unsigned short freq;
	unsigned short data;
} FREQUENCY_BAND_DATA;
typedef struct _FREQUENCY_SET_DATA_ {
	unsigned short start;
	unsigned short end;
	unsigned char step;
	int len;
	FREQUENCY_BAND_DATA *band;
} FREQUENCY_SET_DATA;
/* Extern macro -------------------------------------------------------------*/
/* Extern variables ---------------------------------------------------------*/
/* Extern function prototypes -----------------------------------------------*/
/* Extern functions ---------------------------------------------------------*/

int viRunFre(char *argv[]);

#endif

