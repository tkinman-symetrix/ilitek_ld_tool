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

#ifndef INC_ILITEK_WIFI_H_
#define INC_ILITEK_WIFI_H_
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "ILITek_Device.h"

#define QUEUE_ITEM_SIZE			64
#define QUEUE_MAX_SIZE			2048
#define QUEUE_BUFF_SIZE			((QUEUE_MAX_SIZE) * (QUEUE_ITEM_SIZE))

enum Wifi_PacketRxType {
	Type_Normal = 0,
	Type_WriteAndWaitAck,
	Type_GetFeatureData,
	Type_CDC,
	Type_SendFwFile,
	Type_FWUpgrade,
	Type_FWUpgrade_QueryProgress,
	Type_FWUpgrade_Stop,
	Type_BigDataRx,
	Type_Info,
	Type_Paint,
	Type_Paint_Query,
	Type_Paint_Stop,
	Type_SetTimeout,

	Type_Ack = 0xAC,
};

enum Wifi_Interface {
	wifi_I2C = 0,
	wifi_USB,
	wifi_I2C_HID,
};

enum Wifi_PacketTxType {
    Type_Fail = -1,
    Type_Success = 0,
    Type_BigDataTx,
};

/* Total size should be 4096 */
struct __attribute__((__packed__)) Wifi_RxPTL {
	uint8_t packetType;
	union __attribute__((__packed__)) {
		/* BigData */
		uint16_t big_data_rxlen;

		/* SetTimeout */
		uint32_t timeout_ms;

		/* Normal */
		struct __attribute__((__packed__)) {
			uint8_t reportID;
			uint8_t usbCMD;
			union {
				struct __attribute__((__packed__)) {
					uint8_t wlen;
					uint8_t rlen;
					uint8_t cmd[4091];
				};

				struct __attribute__((__packed__)) {
					uint16_t wlen_u16;
					uint16_t rlen_u16;
					uint8_t cmd_u16[4089];
				};
			};
		};

		/* CDC */
		struct __attribute__((__packed__)) {
			uint32_t totalLen;
			uint16_t packetLen;
			uint16_t x_ch;
			uint16_t y_ch;
			uint8_t u16;
			uint8_t cmd_cdc[4084];
		};

		/* SendFWfile and FWUpgrade */
		struct __attribute__((__packed__)) {
			union __attribute__((__packed__)) {
				uint32_t fw_size;

				/* FWUpgrade */
				struct __attribute__((__packed__)) {
					bool force_update;
				};
			};
			char fw_filename[4091];
		};

		/* Painting */
		uint8_t use_queue;
	};
};

struct __attribute__((__packed__)) Wifi_TxPTL {
	int8_t flag;
	union __attribute__((__packed__)) {
		/* Normal */
		uint8_t buf[4095];

		/* FWUpgrade */
		struct __attribute__((__packed__)) {
			uint8_t finish;
			uint8_t progress;
		};
	};
};

struct Wifi_RxBuff {
	union {
		uint8_t rxbuf[4096];
		struct Wifi_RxPTL rxPTL;
	};
};

struct Wifi_TxBuff {
	union {
		uint8_t txbuf[4096];
		struct Wifi_TxPTL txPTL;
	};
};

struct Queue {
	uint32_t curr_size;
	uint32_t max_size;
	uint8_t buf[QUEUE_BUFF_SIZE];
	uint8_t (*push_ptr)[QUEUE_ITEM_SIZE];
	uint8_t (*pop_ptr)[QUEUE_ITEM_SIZE];
	uint8_t (*end_ptr)[QUEUE_ITEM_SIZE];
	pthread_mutex_t lock;
};

struct Wifi_Handle {
	int socket_fd;
	struct Wifi_RxBuff rx;
	struct Wifi_TxBuff tx;
	int error;

	uint32_t t_ms;

	/* Paint */
	bool paint_stop;
	pthread_t paint_t;
	bool use_queue;
	uint8_t Paint_buf[QUEUE_BUFF_SIZE + 1];
	struct Queue paint_q;
	struct Netlink_Handle nl;

	/* CDC */
	uint8_t CDC_buf[300 * 300 * 2 + 1];

	/* FWUpgrade */
	char fw_filename[128];
};

int wifi_recv(struct Wifi_Handle *wifi, uint8_t * buf, uint32_t rx_size);
int wifi_send(struct Wifi_Handle *wifi, uint8_t *buf, uint32_t tx_size);

int sendAck(struct Wifi_Handle *wifi, int flag);
int recvAck(struct Wifi_Handle *wifi, int flag);
int wifiConnect(int *socket_fd, const char *server_ip, int port);
int Wifi_Normal(struct Wifi_Handle *wifi);
int Wifi_WriteAndWaitAck(struct Wifi_Handle *wifi);
int Wifi_GetFeatureData(struct Wifi_Handle *wifi);
int Wifi_CDC(struct Wifi_Handle *wifi);
int Wifi_MakeFwFile(struct Wifi_Handle *wifi);
int Wifi_FWUpgrade(struct Wifi_Handle *wifi);
int Wifi_Info(struct Wifi_Handle *wifi);
int Wifi_Main(const char *server_ip);
#endif
