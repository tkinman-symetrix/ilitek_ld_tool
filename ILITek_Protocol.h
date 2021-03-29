
#ifndef INC_ILITEK_PROTOCOL_3X_H_
#define INC_ILITEK_PROTOCOL_3X_H_

#define PROTOCOL_V3_4_0         0x30400
#define PROTOCOL_V6_0_0         0x60000
#define PROTOCOL_V6_0_2         0x60002
//------------------------------------------------
#include <stdint.h>
typedef struct _PROTOCOL_DATA_ {
    unsigned char ic_num;
    unsigned int x_max;
    unsigned int y_max;
    unsigned int ic;
    unsigned int ver;
    unsigned int x_ch;
    unsigned int y_ch;
    unsigned char mode_num;
    unsigned char mode[3];
    unsigned char block_num;
    unsigned int key_num;
    unsigned char key_mode;
} PROTOCOL_DATA;
PROTOCOL_DATA ptl;
extern PROTOCOL_DATA ptl;
//-------------------------------------------------
extern int GetFWVersion();
extern int GetCoreVersion();
extern int GetProtocol();
extern int monitor_extend();
extern int monitor_Copy();
extern int check_status();
extern int switch_testmode(unsigned char *para_m, unsigned char *para_f);
extern int GetICMode();
extern int ExitTestMode();
extern int EnterTestMode();
extern int SetProgramKey();
extern int ChangeTOBL();
extern int ChangeTOAP();
extern int GetKernelVer();
extern int WriteDataFlashKey(unsigned int df_end_addr,unsigned int df_check);
extern int WriteAPCodeKey(unsigned int ap_end_addr,unsigned int ap_check);
extern int EraseDataFlash();
extern int CheckBusy(int count, int delay, int type);
extern unsigned int GetCodeCheckSum(unsigned char ucType);
extern uint32_t SaveFlashFile(uint8_t *buff, uint32_t start, uint32_t len, char *path);
extern int GetFWMode();
extern int software_reset();

//-------------------------------AP V3---------------------------------
extern int PanelInfor_V3();
extern int viSwitchMode_V3(int mode);
int SetProgramKey_V3();
extern int CheckBusy_3X(int count, int delay);
extern int GetKeyInfor_V3(int key_num); 
extern int32_t GetFlashData_V3(uint32_t start, uint32_t len, char *path);
//-------------------------------AP V6---------------------------------
extern int PanelInfor_V6();
extern int viSwitchMode_V6(int mode);
extern unsigned int GetICBlockCrcAddr(unsigned int start, unsigned int end, unsigned int type);
int SetProgramKey_V6();
extern int CheckBusy_6X(int count, int delay, int type);
extern int SetFsInfo(uint16_t mc_sine_start, uint16_t mc_sine_end, uint8_t mc_sine_step,
              uint16_t mc_swcap_start, uint16_t mc_swcap_end, uint8_t mc_swcap_step,
              uint16_t sc_swcap_start, uint16_t sc_swcap_end, uint8_t sc_swcap_step,
              uint16_t frame_num, uint8_t scan_data);
extern uint32_t SetShortInfo(uint8_t dump1,uint8_t dump2, uint8_t verf, uint8_t posidleL, uint8_t posidleH);
extern int SetDataLength_V6(uint32_t data_len);
extern int GetSlaveICMode_V6(int number);
int GetKeyInfor_V6(int key_num);
extern int SetAccessSlave(int number, uint8_t type);
extern uint32_t GetAPCRC(int number);
extern int Program_Slave_Pro1_8(uint8_t *buffer, int block, uint32_t len);
extern int ModeCtrl_V6(uint8_t mode, uint8_t engineer);
//-------------------------------BL V1.8----------------------------------
extern int WriteFlashEnable_BL1_8(unsigned int start,unsigned int end);
extern int WriteSlaveFlashEnable_BL1_8(uint32_t start,uint32_t end);
#endif
