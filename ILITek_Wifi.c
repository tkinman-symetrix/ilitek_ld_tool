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

#include "ILITek_Wifi.h"
#include "ILITek_Device.h"
#include "ILITek_Main.h"
#include "ILITek_CMDDefine.h"
#include "ILITek_Protocol.h"
#include "API/ILITek_Upgrade.h"
#include "API/ILITek_RawData.h"

static void Queue_init(struct Queue *q)
{
	pthread_mutex_lock(&q->lock);
	q->curr_size = 0;
	q->max_size = QUEUE_MAX_SIZE;
	q->push_ptr = (uint8_t (*)[QUEUE_ITEM_SIZE])q->buf;
	q->pop_ptr = (uint8_t (*)[QUEUE_ITEM_SIZE])q->buf;
	q->end_ptr = (uint8_t (*)[QUEUE_ITEM_SIZE])&q->buf[QUEUE_BUFF_SIZE -
							   QUEUE_ITEM_SIZE];
	pthread_mutex_unlock(&q->lock);
}

static void Queue_exit(struct Queue *q)
{
	/* Currently, Queue buffer is static
	 * If modified it to dynamic allocated,
	 * Queue_exit() should free buffer here.
	 */
	UNUSED(q);
}

static void Queue_push(struct Queue *q)
{
	pthread_mutex_lock(&q->lock);
	/* Stop push data when queue is full */
	if (q->curr_size >= q->max_size)
		goto release_push_lock;

	q->curr_size++;
	if (q->push_ptr == q->end_ptr)
		q->push_ptr = (uint8_t (*)[64])q->buf;
	else
		q->push_ptr++;

	if (q->push_ptr == q->pop_ptr)
		LD_ERR("[Warn]Queue overload, queue size: %u\n", q->curr_size);

release_push_lock:
	pthread_mutex_unlock(&q->lock);
}

static void Queue_pop(struct Queue *q)
{
	pthread_mutex_lock(&q->lock);
	if (!q->curr_size)
		goto release_pop_lock;

	q->curr_size--;
	if (q->pop_ptr == q->end_ptr)
		q->pop_ptr = (uint8_t (*)[64])q->buf;
	else
		q->pop_ptr++;

release_pop_lock:
	pthread_mutex_unlock(&q->lock);
}

int wifiConnect(int *socket_fd, const char *server_ip, int port)
{
	struct sockaddr_in server_addr;
	int txbuf_size = 0;
	socklen_t len = sizeof(uint8_t);
	int error;

	*socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (*socket_fd < 0)
		return _FAIL;

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(server_ip);
	LD_MSG("Server IP: %s\n", server_ip);

	/* Set Tx Buffer for CDC */
	len = sizeof(int);
	txbuf_size = 262144;
	error = setsockopt(*socket_fd, SOL_SOCKET, SO_SNDBUF,
			   &txbuf_size, len);
	if (error < 0)
		return _FAIL;

	error = getsockopt(*socket_fd, SOL_SOCKET, SO_SNDBUF,
			   &txbuf_size, &len);
	if (error < 0)
		return _FAIL;
	LD_MSG("Send Buffer Size: %u bytes\n", txbuf_size);

	return connect(*socket_fd, (struct sockaddr *)&server_addr,
		       sizeof(server_addr));
}

int sendAck(struct Wifi_Handle *wifi, int flag)
{
	uint8_t txBuff[65];

	txBuff[0] = wifi->tx.txPTL.flag;
	txBuff[1] = 0xAC;
	return send(wifi->socket_fd, txBuff, 65, flag);
}

int recvAck(struct Wifi_Handle *wifi, int flag)
{
	uint8_t rxBuff[65];

	int error = recv(wifi->socket_fd, rxBuff, 65, flag);

	if (error <= 0)
		return _FAIL;
	return (rxBuff[0] == Type_Ack) ? 0 : _FAIL;
}

void *Wifi_do_FWUpgrade(void *data)
{
	struct Wifi_Handle *wifi = *(struct Wifi_Handle **)data;

	switch_irq(0);
	upg.args_fw_ver_check = false;
	wifi->error = viRunFiremwareUpgrade(wifi->fw_filename);
	switch_irq(1);

	/* Notify ITS to stop FW upgrade */
	wifi->tx.txPTL.finish = 0xAD;

	pthread_exit(NULL);
}

int Wifi_FWUpgrade(struct Wifi_Handle *wifi)
{
	int ret = 0;
	pthread_t fw_update_t;

	upg.progress = 0;
	upg.force_update = wifi->rx.rxPTL.force_update;
	strcpy(wifi->fw_filename, wifi->rx.rxPTL.fw_filename);

	wifi->tx.txPTL.flag = Type_Success;
	ret = sendAck(wifi, 0);
	if (ret < 0)
		return ret;

	wifi->error = 0;
	wifi->tx.txPTL.finish = 0;
	pthread_create(&fw_update_t, NULL, Wifi_do_FWUpgrade, &wifi);

	while (1) {
		ret = recv(wifi->socket_fd, wifi->rx.rxbuf, 65, 0);
		if (ret <= 0) {
			wifi->tx.txPTL.flag = (ret < 0) ? ret : Type_Fail;
			goto err_wait_thread;
		}

		switch (wifi->rx.rxPTL.packetType) {
		case Type_FWUpgrade_QueryProgress:
			wifi->tx.txPTL.flag = (wifi->error < 0) ?
				wifi->error : Type_Success;
			wifi->tx.txPTL.progress = upg.progress;
			ret = send(wifi->socket_fd, wifi->tx.txbuf, 65, 0);
			if (ret < 0)
				return ret;
			break;
		case Type_FWUpgrade_Stop:
			wifi->tx.txPTL.flag = Type_Success;
			goto err_wait_thread;
		default:
			wifi->tx.txPTL.flag = -EINVAL;
			goto err_wait_thread;
		};
	}

err_wait_thread:
	pthread_join(fw_update_t, NULL);
	return sendAck(wifi, 0);
}

int Wifi_MakeFwFile(struct Wifi_Handle *wifi)
{
	FILE *fw_file;
	uint8_t *fw_data;
	uint32_t offset = 0;
	int ret = 0;

	fw_file = fopen(wifi->rx.rxPTL.fw_filename, "wb");
	if (!fw_file)
		ret = -errno;

	fw_data = (uint8_t *)malloc(wifi->rx.rxPTL.fw_size);
	if (!fw_data)
		ret = -ENOMEM;

	wifi->tx.txPTL.flag = (ret < 0) ? ret : Type_Success;
	ret = sendAck(wifi, 0);
	if (ret < 0)
		return ret;

	if (wifi->tx.txPTL.flag < 0)
		goto err_free_fw_data;

	while (offset < wifi->rx.rxPTL.fw_size) {
		ret = recv(wifi->socket_fd, fw_data + offset,
			   get_min(wifi->rx.rxPTL.fw_size - offset, 4096),
			   MSG_WAITALL);
		if (ret <= 0)
			goto err_free_fw_data;

		offset += ret;

		wifi->tx.txPTL.flag = Type_Success;
		ret = sendAck(wifi, 0);
		if (ret < 0)
			goto err_free_fw_data;
	}

	fwrite(fw_data, sizeof(uint8_t), wifi->rx.rxPTL.fw_size, fw_file);
	fclose(fw_file);

err_free_fw_data:
	free(fw_data);

	return (ret < 0) ? ret : 0;
}

int Wifi_CDC_3X(struct Wifi_Handle *wifi)
{
	int nodeLen = wifi->rx.rxPTL.totalLen;
	int data_format;

	if (wifi->rx.rxPTL.u16) {
		data_format = _DataFormat_16_Bit_;
		nodeLen /= 2;
	} else {
		data_format = _DataFormat_8_Bit_;
	}

	/*
	 * cmd[0]: CDC return type
	 * cmd[1]: CDC control/drive
	 */
	if (viInitRawData_3X(wifi->rx.rxPTL.cmd_cdc[0],
			     wifi->rx.rxPTL.cmd_cdc[1]) < 0)
		return _FAIL;

	if (viGetRawData_3X(wifi->rx.rxPTL.cmd_cdc[1], _FastMode_, nodeLen,
			    data_format, ptl.x_ch, wifi->CDC_buf + 1) < 0)
		return _FAIL;

	return 0;
}

int Wifi_CDC_6X(struct Wifi_Handle *wifi)
{
	int nodeLen = wifi->rx.rxPTL.totalLen;

	if (wifi->rx.rxPTL.u16)
		nodeLen /= 2;

	if (viInitRawData_6X(wifi->rx.rxPTL.cmd_cdc[0], 10) < 0)
		return _FAIL;

	if (viGetRawData_6X(nodeLen, wifi->CDC_buf + 1) < 0)
		return _FAIL;

	return 0;
}

int Wifi_CDC(struct Wifi_Handle *wifi)
{
	int error = _FAIL;

	ptl.x_ch = wifi->rx.rxPTL.x_ch;
	ptl.y_ch = wifi->rx.rxPTL.y_ch;

	if (inProtocolStyle == _Protocol_V3_)
		error = Wifi_CDC_3X(wifi);
	else if (inProtocolStyle == _Protocol_V6_)
		error = Wifi_CDC_6X(wifi);

	wifi->CDC_buf[0] = (error < 0) ? error : Type_Success;

	return send(wifi->socket_fd, wifi->CDC_buf,
		    wifi->rx.rxPTL.totalLen + 1, 0);
}

int Wifi_Normal(struct Wifi_Handle *wifi)
{
	int error;

	if (wifi->rx.rxPTL.usbCMD == 0xA3) {
		if (wifi->rx.rxPTL.reportID == 0x03)
			error = TransferData_HID(&wifi->rx.rxPTL.reportID,
					 	 wifi->rx.rxPTL.wlen,
					 	 wifi->tx.txPTL.buf,
					 	 wifi->rx.rxPTL.rlen, 1000);
		else
			error = TransferData_HID(&wifi->rx.rxPTL.reportID,
					 	 wifi->rx.rxPTL.wlen_u16,
						 wifi->tx.txPTL.buf,
						 wifi->rx.rxPTL.rlen_u16, 1000);

		wifi->tx.txPTL.flag = (error < 0) ? error : Type_Success;

		if (wifi->rx.rxPTL.reportID == 0x03 && wifi->rx.rxPTL.rlen)
			return send(wifi->socket_fd, wifi->tx.txbuf, 65, 0);
		else if (wifi->rx.rxPTL.reportID != 0x03 &&
			   wifi->rx.rxPTL.rlen_u16)
			return wifi_send(wifi, wifi->tx.txbuf,
					 wifi->rx.rxPTL.rlen_u16 + 1);
		else
			return sendAck(wifi, 0);
	} else if (wifi->rx.rxPTL.usbCMD == 0xA4) {
		/* Used for V6 I2C Only */
		error = TransferData_HID(&wifi->rx.rxPTL.reportID,
					 wifi->rx.rxPTL.wlen_u16,
					 wifi->tx.txPTL.buf + 1,
					 wifi->rx.rxPTL.rlen_u16, 1000);
		wifi->tx.txPTL.flag = (error < 0) ? error : Type_Success;

		wifi->tx.txPTL.buf[0] = 0x05;	/* Sentinel */
		return wifi_send(wifi, wifi->tx.txbuf,
				 wifi->rx.rxPTL.rlen_u16 + 1);
	}

	LD_ERR("Wifi_Normal enter invalid status, please confirm !\n");

	return _FAIL;
}

int Wifi_WaitAck(struct Wifi_Handle *wifi, int timeout_ms)
{
	int error = 0;
	uint8_t cmd;
	uint8_t ack = 0xAC;

	cmd = (wifi->rx.rxPTL.reportID == 0x03) ?
		wifi->rx.rxPTL.cmd[0] : wifi->rx.rxPTL.cmd_u16[0];

	switch (inConnectStyle) {
	case _ConnectStyle_I2C_:
		if (!wait_INT(timeout_ms))
			ack = 0xFF;

		wifi->tx.txPTL.buf[0] = 0x03;
		wifi->tx.txPTL.buf[1] = 0xA3;
		wifi->tx.txPTL.buf[2] = cmd;
		wifi->tx.txPTL.buf[3] = 0x01;
		wifi->tx.txPTL.buf[4] = ack;
		break;
	case _ConnectStyle_USB_:
	case _ConnectStyle_I2CHID_:
		error = viWaitAck(cmd, wifi->tx.txPTL.buf, timeout_ms);
		ack = wifi->tx.txPTL.buf[4];
		break;
	};

	LD_MSG("[%s] cmd: %x, timeout: %d ms, ack: %x, err: %d\n",
		__func__, cmd, timeout_ms, ack, error);

	return error;
}

int Wifi_WriteAndWaitAck(struct Wifi_Handle *wifi)
{
	int error;

	if (inConnectStyle == _ConnectStyle_I2C_)
		init_INT();

	if (wifi->rx.rxPTL.reportID == 0x03)
		error = TransferData_HID(&wifi->rx.rxPTL.reportID,
					 wifi->rx.rxPTL.wlen,
					 wifi->tx.txPTL.buf,
					 wifi->rx.rxPTL.rlen, 1000);
	else
		error = TransferData_HID(&wifi->rx.rxPTL.reportID,
					 wifi->rx.rxPTL.wlen_u16,
					 wifi->tx.txPTL.buf,
					 wifi->rx.rxPTL.rlen_u16, 1000);
	if (error < 0)
		goto err_notify;

	error = Wifi_WaitAck(wifi, wifi->t_ms);

err_notify:
	wifi->tx.txPTL.flag = (error < 0) ? error : Type_Success;

	return send(wifi->socket_fd, wifi->tx.txbuf, 65, 0);
}

int Wifi_GetFeatureData(struct Wifi_Handle *wifi)
{
	int error = _FAIL;

	switch (inConnectStyle) {
	/* GetFeature should not used for I2C interface */
	case _ConnectStyle_I2C_:
		error = _FAIL;
		break;
	case _ConnectStyle_USB_:
	case _ConnectStyle_I2CHID_:
		error = TransferData_HID(&wifi->rx.rxPTL.reportID,
					 0, wifi->tx.txPTL.buf,
					 wifi->rx.rxPTL.rlen_u16 - 7,
					 1000);
		break;
	};

	wifi->tx.txPTL.flag = (error < 0) ? error : Type_Success;

	return send(wifi->socket_fd, wifi->tx.txbuf, wifi->rx.rxPTL.rlen_u16 + 1, 0);
}

int Wifi_Info(struct Wifi_Handle *wifi)
{
	uint32_t ver;
	int idx = 0;
	int daemon_ver[4];

	switch (inConnectStyle) {
	case _ConnectStyle_I2C_:
		wifi->tx.txPTL.buf[idx++] = wifi_I2C;
		break;
	case _ConnectStyle_USB_:
		wifi->tx.txPTL.buf[idx++] = wifi_USB;
		break;
	case _ConnectStyle_I2CHID_:
		wifi->tx.txPTL.buf[idx++] = wifi_I2C_HID;
		break;
	default:
		wifi->tx.txPTL.buf[idx++] = 0xFF;
	}

	switch (inProtocolStyle) {
	case _Protocol_V3_:
		wifi->tx.txPTL.buf[idx++] = 3;
		break;
	case _Protocol_V6_:
		wifi->tx.txPTL.buf[idx++] = 6;
		break;
	default:
		wifi->tx.txPTL.buf[idx++] = 0xFF;
	}

	sscanf(TOOL_VERSION, "ILITEK LINUX DAEMON V%d.%d.%d.%d",
		daemon_ver, daemon_ver + 1, daemon_ver + 2, daemon_ver + 3);

	wifi->tx.txPTL.buf[idx++] = daemon_ver[3];
	wifi->tx.txPTL.buf[idx++] = daemon_ver[2];
	wifi->tx.txPTL.buf[idx++] = daemon_ver[1];
	wifi->tx.txPTL.buf[idx++] = daemon_ver[0];

	if (inConnectStyle == _ConnectStyle_I2C_) {
		ver = get_driver_ver();

		wifi->tx.txPTL.buf[idx++] = ver & 0xFF;
		wifi->tx.txPTL.buf[idx++] = (ver >> 8) & 0xFF;
		wifi->tx.txPTL.buf[idx++] = (ver >> 16) & 0xFF;
		wifi->tx.txPTL.buf[idx++] = (ver >> 24) & 0xFF;
	}

	wifi->tx.txPTL.flag = Type_Success;
	return send(wifi->socket_fd, wifi->tx.txbuf, 65, 0);
}

int wifi_read_report(struct Wifi_Handle *wifi, char *buf)
{
	int error = _FAIL;

	switch (inConnectStyle) {
	case _ConnectStyle_USB_:
	case _ConnectStyle_I2CHID_:
		error = read_report(buf, QUEUE_ITEM_SIZE, 1000);
		break;
	case _ConnectStyle_I2C_:
		error = netlink_recv(&wifi->nl, buf);
		break;
	}

	if (error < 0)
		LD_DBG("wifi_read_report failed, err: %d\n", error);
	else
		debugBuffPrintf("[Report]:", (uint8_t *)buf, 64);

	return error;
}

void *Wifi_do_Paint(void *data)
{
	struct Wifi_Handle *wifi = *(struct Wifi_Handle **)data;
	char *buf_ptr;
	int error;

	if (inConnectStyle == _ConnectStyle_I2C_) {
		/* should notify main thread netlink is not ready */
		wifi->error = netlink_connect(&wifi->nl, "Wifi_Paint_Start",
					      QUEUE_ITEM_SIZE, 1000);
		if (wifi->error < 0)
			goto exit_pthread;
		i2c_read_data_enable(true);
	}

	while (!wifi->paint_stop) {
		buf_ptr = (char *)wifi->paint_q.push_ptr;

		if (!wifi->use_queue) {
			if (wifi->paint_q.curr_size)
				Queue_init(&wifi->paint_q);
			continue;
		}

		error = wifi_read_report(wifi, buf_ptr);
		if (error < 0)
			continue;

		Queue_push(&wifi->paint_q);
	}

exit_pthread:
	if (inConnectStyle == _ConnectStyle_I2C_) {
		i2c_read_data_enable(false);
		netlink_disconnect(&wifi->nl, "Wifi_Paint_End");
	}
	pthread_exit(NULL);
}

int Wifi_Paint(struct Wifi_Handle *wifi)
{
	int error;
	uint32_t tx_size;
	uint32_t q_size, idx;

	wifi->use_queue = wifi->rx.rxPTL.use_queue;
	wifi->error = 0;
	pthread_mutex_init(&wifi->paint_q.lock, NULL);
	memset(wifi->Paint_buf, 0, sizeof(wifi->Paint_buf));
	Queue_init(&wifi->paint_q);
	wifi->paint_stop = false;
	pthread_create(&wifi->paint_t, NULL, Wifi_do_Paint, &wifi);
	wifi->tx.txPTL.flag = Type_Success;
	error = sendAck(wifi, 0);
	if (error < 0)
		return error;

	while (1) {
		error = recv(wifi->socket_fd, wifi->rx.rxbuf, 65, 0);
		if (error <= 0) {
			wifi->tx.txPTL.flag = (error < 0) ? error : Type_Fail;
			goto err_exit_thread;
		}
		wifi->use_queue = wifi->rx.rxPTL.use_queue;

		switch (wifi->rx.rxPTL.packetType) {
		case Type_Paint_Query:
			/* Make ITS knows/check no report */
			wifi->Paint_buf[1] = 0xFF;
			tx_size = QUEUE_ITEM_SIZE + 1;

			if (wifi->use_queue) {
				for (q_size = wifi->paint_q.curr_size, idx = 1;
				     q_size > 0; idx += QUEUE_ITEM_SIZE,
				     q_size--) {
					memcpy(wifi->Paint_buf + idx,
					       wifi->paint_q.pop_ptr,
					       QUEUE_ITEM_SIZE);
					Queue_pop(&wifi->paint_q);
				}
				tx_size = (idx == 1) ? tx_size : idx;
			} else {
				wifi_read_report(wifi,
						 (char *)(wifi->Paint_buf + 1));
			}

			wifi->Paint_buf[0] = Type_Success;
			error = wifi_send(wifi, wifi->Paint_buf, tx_size);
			if (error < 0)
				return error;
			break;
		case Type_Paint_Stop:
			wifi->tx.txPTL.flag = Type_Success;
			goto err_exit_thread;
		default:
			wifi->tx.txPTL.flag = -EINVAL;
			goto err_exit_thread;
		}
	}

err_exit_thread:
	wifi->paint_stop = true;
	pthread_join(wifi->paint_t, NULL);
	Queue_exit(&wifi->paint_q);
	pthread_mutex_destroy(&wifi->paint_q.lock);
	return sendAck(wifi, 0);
}

int wifi_send(struct Wifi_Handle *wifi, uint8_t *buf, uint32_t tx_size)
{
	int error;
	uint8_t txBuff[65];

	if (tx_size <= 65)
		return send(wifi->socket_fd, buf, 65, 0);

	txBuff[0] = Type_BigDataTx;
	txBuff[1] = tx_size & 0xFF;
	txBuff[2] = (tx_size >> 8) & 0xFF;
	error = send(wifi->socket_fd, txBuff, 65, 0);
	if (error < 0)
		return _FAIL;

	error = recvAck(wifi, 0);
	if (error < 0)
		return _FAIL;

	return send(wifi->socket_fd, buf, tx_size, 0);
}

int wifi_recv(struct Wifi_Handle *wifi, uint8_t *buf, uint32_t rx_size)
{
	int error;
	unsigned int big_rx_size = 0;
	struct Wifi_RxPTL *rxptl;

	//wifi->rx.rxbuf
	error = recv(wifi->socket_fd, buf, rx_size, 0);
	if (error <= 0)
		return error;

	rxptl = (struct Wifi_RxPTL *)buf;
	if (rxptl->packetType == Type_BigDataRx) {
		big_rx_size = rxptl->big_data_rxlen;
		wifi->tx.txPTL.flag = Type_Success;
		error = sendAck(wifi, 0);
		if (error < 0)
			return error;
		error = recv(wifi->socket_fd, buf, big_rx_size, MSG_WAITALL);
	}

	return error;
}

int Wifi_Main(const char *server_ip)
{
	int error;
	struct Wifi_Handle wifi;

	error = wifiConnect(&wifi.socket_fd, server_ip, 8080);
	if (error < 0) {
		LD_ERR("connect failed, err:%d\n", errno);
		return error;
	}

	LD_MSG("connect success!, start communication\n");
	/* Start communication... */
	while (true) {
		memset(wifi.rx.rxbuf, 0, 65);
		memset(wifi.tx.txbuf, 0, 65);

		error = wifi_recv(&wifi, wifi.rx.rxbuf, 65);
		if (error <= 0)
			break;

		debugBuffPrintf("[Recv]:", wifi.rx.rxbuf, error);

		switch (wifi.rx.rxPTL.packetType) {
		case Type_Normal:
			error = Wifi_Normal(&wifi);
			break;
		case Type_WriteAndWaitAck:
			error = Wifi_WriteAndWaitAck(&wifi);
			break;
		case Type_GetFeatureData:
			error = Wifi_GetFeatureData(&wifi);
			break;
		case Type_CDC:
			error = Wifi_CDC(&wifi);
			break;
		case Type_SendFwFile:
			error = Wifi_MakeFwFile(&wifi);
			break;
		case Type_FWUpgrade:
			error = Wifi_FWUpgrade(&wifi);
			break;
		case Type_Info:
			error = Wifi_Info(&wifi);
			break;
		case Type_Paint:
			error = Wifi_Paint(&wifi);
			break;
		case Type_SetTimeout:
			wifi.t_ms = wifi.rx.rxPTL.timeout_ms;
			error = sendAck(&wifi, 0);
			break;
		default:
			LD_ERR("Unrecognized type: %#x\n", wifi.rx.rxPTL.packetType);
			goto err_close_socket;
		};

		debugBuffPrintf("[Send]:", wifi.tx.txbuf, error);

		if (error < 0)
			break;
	}

err_close_socket:
	close(wifi.socket_fd);

	return (error < 0) ? _FAIL : _SUCCESS;
}
