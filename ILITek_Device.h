#ifndef INC_ILITEK_DEVICE_H_
#define INC_ILITEK_DEVICE_H_
#include <stdint.h>

//---------------------------------------------------------------
#define TOOL_VERSION "ILITEK LINUX DAEMON V3.0.5.Test1\n"
#define REPORT_VERSION  "Report_Format_version  ,0.0.1.0\n"
#define USE_LIBUSB
#ifdef USE_LIBUSB
#include <usb.h>
typedef struct _ILIUSB_DEVICE_
{
	struct usb_dev_handle *dev;
	uint32_t ep_in;
	uint32_t ep_out;

} ILIUSB_DEVICE;
#endif
#define _APMode_  0x01
#define _BLMode_  0x02
#define _MaxChanelNum_          300
#define _UpdatePageDelayTime_   10000
#define _UpdateDelayTime_       5000

#define TRANSFER_MAX_BUFFER     5000

#define REPORT_ID_64_BYTE        0x0203
#define REPORT_ID_256_BYTE       0x0307
#define REPORT_ID_1024_BYTE      0x0308
#define REPORT_ID_2048_BYTE      0x0309
#define REPORT_ID_4096_BYTE      0x030A
#define TRANS_USB_256_BYTE       256+1

//#define DEBUG_TRANSFER_DATA
typedef struct _SLAVE_INFO_
{
	int	CRC_Code_1;
	int CRC_Code_2;
	int FW_Ver_1;
	int	FW_Ver_2;
} SLAVE_INFO;

extern int inConnectStyle;
extern int inProtocolStyle;
extern int is_usb_hid_old_bl;
extern int ILITEK_PID, ILITEK_VID, OTHER_VID;

extern int active_interface;

//---------------------------------------------------------------
extern int sockfd;
extern struct sockaddr_in server_addr;
extern int inRemote_Flag;
//---------------------------------------------------------------
extern unsigned char buff[TRANSFER_MAX_BUFFER];
extern unsigned char tempbuff[256];
extern unsigned char FWVersion[8];
extern unsigned char CoreVersion[7];
extern unsigned char ProtocolVersion[4];
extern unsigned char KernelVersion[6];
extern unsigned int Resolution_X,Resolution_Y,ChannelNum_X,ChannelNum_Y,ICMode;
extern unsigned char ucBLAPMode;
extern short int uiTestDatas[_MaxChanelNum_][_MaxChanelNum_];
extern short int uiTestDatas_1[_MaxChanelNum_][_MaxChanelNum_];
extern unsigned char ucSensorTestResult;
extern unsigned char ucSignedDatas;
//----------------------------------------------------------------
extern unsigned int hex_2_dec(char *hex, int len);
extern unsigned int CheckFWCRC(unsigned int startAddr,unsigned int endAddr,unsigned char input[]);
extern int TransferData(uint8_t *OutBuff, int writelen, uint8_t *InBuff, int readlen, int inTimeOut);
//----------------------------------------------------------------
extern int SetConnectStyle(char *argv[]);
extern int InitDevice();
//extern int ReadData(int readlen,int inTimeOut);
extern void CloseDevice();
extern int write_data(int fd, unsigned char *buf, int len);
extern int read_data(int fd, unsigned char *buf, int len);
extern int switch_irq(int flag);
extern void viDriverCtrlReset();
#endif
