/* SPDX-License-Identifier: GPL-2.0 */
/*
 * ILITEK Linux Daemon Tool
 *
 * Copyright (c) 2021 Luca Hsu <luca_hsu@ilitek.com>
 * Copyright (c) 2021 Joe Hung <joe_hung@ilitek.com>
 *
 * The code could be used by anyone for any purpose,
 * and could perform firmware update for ILITEK's touch IC.
 */

#ifndef INC_ILITEK_DEVICE_H_
#define INC_ILITEK_DEVICE_H_
#include <stdint.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <linux/netlink.h>

//---------------------------------------------------------------
#define TOOL_VERSION "ILITEK LINUX DAEMON V3.0.7.4"
#define REPORT_VERSION  "0.0.1.2"

#ifdef CONFIG_ILITEK_USE_LIBUSB
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

#define UNUSED(x) (void)(x)

extern int inConnectStyle;
extern int inProtocolStyle;
extern int is_usb_hid_old_bl;
extern int ILITEK_PID, ILITEK_VID, OTHER_VID;

//---------------------------------------------------------------
#define get_min(a, b)	(((a) > (b)) ? (b) : (a))
#define get_max(a, b)	(((a) < (b)) ? (b) : (a))

struct Netlink_Handle {
	int fd;
	int data_size;
	struct sockaddr_nl src_addr;
	struct sockaddr_nl dest_addr;
	struct nlmsghdr *nlh;
	struct msghdr msg;
	struct iovec iov;
};

extern uint16_t get_le16(const uint8_t *p);
extern int netlink_connect(struct Netlink_Handle *nl, const char *str,
			   uint32_t size, uint32_t timeout_ms);
extern void netlink_disconnect(struct Netlink_Handle *nl, const char *str);
extern int netlink_recv(struct Netlink_Handle *nl, char *buf);

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
unsigned int UpdateCRC(unsigned int crc,unsigned char newbyte);
extern unsigned int CheckFWCRC(unsigned int startAddr,unsigned int endAddr,unsigned char input[]);
extern int TransferData_HID(uint8_t *OutBuff, int writelen, uint8_t *InBuff, int readlen, int timeout_ms);
extern int TransferData(uint8_t *OutBuff, int writelen, uint8_t *InBuff, int readlen, int timeout_ms);
//----------------------------------------------------------------
extern int SetConnectStyle(char *argv[]);
extern int InitDevice();
extern void CloseDevice();
extern int write_data(int fd, unsigned char *buf, int len);
extern int read_data(int fd, unsigned char *buf, int len);
extern int switch_irq(int flag);
extern void viDriverCtrlReset();
extern int viWaitAck(uint8_t cmd, uint8_t *buf, int timeout_ms);
extern void i2c_read_data_enable(bool enable);
extern int hidraw_read(int fd, uint8_t *buf, int len, int timeout_ms,
		       uint8_t cmd, bool check_validity, bool check_ack);
extern void debugBuffPrintf(const char *str, uint8_t *buf, int len);
extern int read_report(char *buf, int len, int t_ms);

extern void init_INT();
extern bool wait_INT(int timeout_ms);
extern uint32_t get_driver_ver();

extern int write_and_wait_ack(uint8_t *Wbuff, int wlen, int timeout_ms, int cnt, int delay_ms, int type);

extern int set_engineer(bool enable);
#endif
