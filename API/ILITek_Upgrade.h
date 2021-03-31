
/******************** (C) COPYRIGHT 2019 ILI Technology Corp. ********************
* File Name :   IliTek_Upgrade.h
* Description   :   Header for IliTek_Upgrade.c file
*
********************************************************************************
*History:
*   Version        Date           Author            Description
*   --------------------------------------------------------------------------
*      1.0       2019/02/15          Randy           Initial version
*******************************************************************************/

#ifndef _ILITEK_UPGRADE_H_
#define _ILITEK_UPGRADE_H_
#include <stdbool.h>
/* Includes of headers ------------------------------------------------------*/
/* Extern define ------------------------------------------------------------*/
#define UPGRAD_FILE_PATH_SIZE               256
#define HEX_FWVERSION_ADDRESS               0x0C
#define HEX_FWVERSION_SIZE                  8
#define HEX_DATA_FLASH_ADDRESS              0x22
#define HEX_DATA_FLASH_SIZE                 3
#define HEX_KERNEL_VERSION_ADDRESS          0x6
#define HEX_KERNEL_VERSION_SIZE             6
#define HEX_MEMONY_MAPPING_VERSION_SIZE     3
#define HEX_MEMONY_MAPPING_VERSION_ADDRESS  0x0
#define HEX_FLASH_BLOCK_NUMMER_ADDRESS      80
#define HEX_FLASH_BLOCK_NUMMER_SIZE         1
#define HEX_FLASH_BLOCK_INFO_ADDRESS        84
#define HEX_FLASH_BLOCK_INFO_SIZE           3
#define HEX_FLASH_BLOCK_END_ADDRESS         123
#define HEX_MEMONY_MAPPING_FLASH_SIZE       128
#define UPGRADE_LENGTH_BLV1_8               2048    //usb 4096 have transfer issue.
#define CRC_CALCULATION_FROM_IC             1
#define CRC_GET_FROM_FLASH                  0
#define NEED_UPGRADE_FW                     1
#define NO_NEED_UPGRADE_FW                  0
#define LEGO_AP_START_ADDRESS				0x3000
/* Extern typedef -----------------------------------------------------------*/
struct IC_DATA {
    unsigned char mode;
    unsigned short crc;
};

struct BLOCK_DATA {
    unsigned int start;
    unsigned int end;
    unsigned short ic_crc;
    unsigned short dae_crc;
    bool chk_crc;               //false: ic and daemon are different.
};

struct UPGRADE_DATA {
    unsigned char *filename;
    unsigned int hexfilesize;
    unsigned int ap_start_addr;
    unsigned int df_start_addr;
    unsigned int exaddr;
    unsigned int ap_end_addr;
    unsigned int df_end_addr;
    unsigned int ap_check;
    unsigned int df_check;
    unsigned int total_check;
    unsigned char hex_fw_ver[HEX_FWVERSION_SIZE];
    unsigned char hex_ic_type[HEX_KERNEL_VERSION_SIZE];
    bool hex_info_flag;
    bool df_tag_exist;
    unsigned int map_ver;
    unsigned int blk_num;
    struct BLOCK_DATA *blk;
    struct IC_DATA *ic;
};
struct UPGRADE_DATA upg;
extern struct UPGRADE_DATA upg;
/* Extern macro -------------------------------------------------------------*/
/* Extern variables ---------------------------------------------------------*/
/* Extern function prototypes -----------------------------------------------*/
/* Extern functions ---------------------------------------------------------*/

int viCheckFWNeedUpgrade(char *cFWVersion);
int viRunFiremwareUpgrade(unsigned char *filename, char *cFWVersion);
int FiremwareUpgrade(unsigned char *filename);
int UpgradeFirmware_Pro1_8(unsigned char *filename);
int UpgradeFirmware_Pro1_7(unsigned char *filename);
int UpgradeFirmware_Pro1_6(unsigned char *filename);
int hex_file_convert(unsigned char *pbuf, unsigned char *buffer, unsigned int hexfilesize);
#endif

