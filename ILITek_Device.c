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

#include "ILITek_Protocol.h"
#include "ILITek_CMDDefine.h"
#include "ILITek_Device.h"
#include <stdint.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define CRC_POLY 0x8408      // CRC16-CCITT FCS (X^16+X^12+X^5+1)

int active_interface = ACTIVE_INTERFACE_ILITEK_CTRL_I2C;
int is_usb_hid_old_bl = 0;

//------------------------------
int inConnectStyle;
int inProtocolStyle;
//USB_HID detail
int ENDPOINT_IN = 0x81;
int ENDPOINT_OUT = 0x02;
int HID_INTERFACE_COUNT = 1;
int ILITEK_PID = 0x0,ILITEK_VID = 0x0, OTHER_VID =0x0;
unsigned char *temp_ILITEK_PID;

struct usb_dev_handle *hdev;

//I2C
char ILITEK_I2C_CONTROLLER[255];
int ILITEK_DEFAULT_I2C_ADDRESS;
int fd;

//Remote
int sockfd;
struct sockaddr_in server_addr;
int inRemote_Flag = 0;
//-----------------------------------
unsigned char buff[TRANSFER_MAX_BUFFER];
unsigned char tempbuff[256];
unsigned char FWVersion[8];
unsigned char CoreVersion[7];
unsigned char ProtocolVersion[4];
unsigned char KernelVersion[6];
unsigned int Resolution_X,Resolution_Y,ChannelNum_X,ChannelNum_Y,ICMode;
unsigned char ucBLAPMode;
short int uiTestDatas[_MaxChanelNum_][_MaxChanelNum_];
short int uiTestDatas_1[_MaxChanelNum_][_MaxChanelNum_];
unsigned char ucSensorTestResult=0;
unsigned char ucSignedDatas=0;
//-----------------------------------

#ifdef CONFIG_ILITEK_USE_LIBUSB
ILIUSB_DEVICE iliusb;
#endif

unsigned int hex_2_dec(char *hex, int len)
{
	unsigned int ret = 0, temp = 0;
	int i, shift = (len - 1) * 4;

	for (i = 0; i < len; shift -= 4, i++) {
		if ((hex[i] >= '0') && (hex[i] <= '9'))
			temp = hex[i] - '0';
		else if((hex[i] >= 'a') && (hex[i] <= 'f'))
			temp = (hex[i] - 'a') + 10;
		else if((hex[i] >= 'A') && (hex[i] <= 'F'))
			temp = (hex[i] - 'A') + 10;
		else
			return _FAIL;
		ret |= (temp << shift);
	}
	return ret;
}


unsigned int UpdateCRC(unsigned int crc,unsigned char newbyte)
{
	unsigned char i;                                  // loop counter

	crc = crc ^ newbyte;

	for (i = 0; i < 8; i++)
	{
		if (crc & 0x01)
		{
			crc = crc >> 1;
			crc ^= CRC_POLY;
		}
		else
		{
			crc = crc >> 1;
		}
	}
	return crc;
}


unsigned int CheckFWCRC(unsigned int startAddr,unsigned int endAddr,unsigned char input[])
{
	unsigned int CRC = 0;
	unsigned int i = 0;

	for (i = startAddr; i < endAddr; i++)
		CRC = UpdateCRC(CRC, input[i]);

	return CRC;
}

int SetConnectStyle(char *argv[])
{
	int ret = _SUCCESS;

	if (!strcmp(argv[2], "I2C")) {
		inConnectStyle=_ConnectStyle_I2C_;
		strcpy(ILITEK_I2C_CONTROLLER, argv[4]);
		ILITEK_DEFAULT_I2C_ADDRESS = hex_2_dec(argv[5], 2);
		if (!strcmp(argv[3], "V3")) {
			inProtocolStyle = _Protocol_V3_;
		} else if (strcmp(argv[3], "V6") == 0) {
			inProtocolStyle = _Protocol_V6_;
		} else {
			inProtocolStyle = _Protocol_V3_;
			strcpy(ILITEK_I2C_CONTROLLER, "/dev/ilitek_ctrl");
			ILITEK_DEFAULT_I2C_ADDRESS = 41;
		}
	} else if (!strcmp(argv[2], "USB")) {
		inConnectStyle = _ConnectStyle_USB_;
		OTHER_VID = hex_2_dec(argv[5], 4);
		if(!strcmp(argv[3], "V3"))
			inProtocolStyle = _Protocol_V3_;
		else if (!strcmp(argv[3], "V6"))
			inProtocolStyle = _Protocol_V6_;
		else
			inProtocolStyle = _Protocol_V3_;

	} else if (!strcmp(argv[2], "I2C-HID")) {
		inConnectStyle = _ConnectStyle_I2CHID_;
		OTHER_VID = hex_2_dec(argv[5], 4);
		if (!strcmp(argv[3], "V3"))
			inProtocolStyle = _Protocol_V3_;
		else if (!strcmp(argv[3], "V6"))
			inProtocolStyle = _Protocol_V6_;
		else
			ret = _FAIL;
	} else {
		ret = _FAIL;
	}

	return ret;
}

int open_hidraw_device()
{
	struct hidraw_devinfo device_info;
	char device_name[256];
	DIR *dir = NULL;
	struct dirent *ptr;
	char hidraw_path[64];

	dir = opendir("/dev");
	if (!dir) {
		LD_ERR("can't open \"/dev\" directory\n");
		return _FAIL;
	}

	while ((ptr = readdir(dir))) {
		/* filter out non-character device */
		if (ptr->d_type != DT_CHR || strncmp(ptr->d_name, "hidraw", 6))
			continue;

		LD_MSG("Check /dev/%s is ILITEK I2C-HID node ?\n", ptr->d_name);
		snprintf(hidraw_path, sizeof(hidraw_path),
			"/dev/%s", ptr->d_name);

		fd = open(hidraw_path, O_RDWR | O_NONBLOCK);
		if (fd < 0) {
			LD_ERR("can't open %s, fd: %d\n", hidraw_path, fd);
			continue;
		}

		ioctl(fd, HIDIOCGRAWINFO, &device_info);
		if (device_info.bustype != BUS_I2C ||
		    (device_info.vendor != ILITEK_VENDOR_ID &&
		     device_info.vendor != OTHER_VID)) {
			LD_ERR("Invalid vendor id: %x or bustype: %u, VID should be %x or %x\n",
				device_info.vendor, device_info.bustype,
				ILITEK_VENDOR_ID, OTHER_VID);
			close(fd);
			continue;
		}

		ioctl(fd, HIDIOCGRAWNAME(256), device_name);
		LD_MSG("name:%s, bustype: %u, path: %s\n",
			device_name, device_info.bustype, hidraw_path);
		return _SUCCESS;
	}
	LD_ERR("No ilitek hidraw file node found!\n");

	return _FAIL;
}

#ifdef CONFIG_ILITEK_USE_LIBUSB
struct usb_dev_handle *open_usb_hid_device()
{
	struct usb_bus *busses, *bus;
	struct usb_device *dev;
	struct usb_dev_handle *hdev;

	is_usb_hid_old_bl = 0;
	usb_init();
	usb_find_busses();
	usb_find_devices();

	busses = usb_get_busses();
	for (bus = busses; bus; bus = bus->next) {
		for (dev = bus->devices; dev; dev = dev->next) {
			if ((dev->descriptor.idVendor == ILITEK_VENDOR_ID) ||
			    (dev->descriptor.idVendor == OTHER_VENDOR_ID) ||
			    (dev->descriptor.idVendor == OTHER_VID)) {
				ILITEK_PID = dev->descriptor.idProduct;
				ILITEK_VID = dev->descriptor.idVendor;

				hdev = usb_open(dev);
				iliusb.dev = hdev;
				if (!hdev) {
					LD_ERR("open device error\n");
					return NULL;
				}

				//print_device(dev);
				//usb_set_configuration(hdev, dev->config->bConfigurationValue);
				//usb_reset(hdev);
				HID_INTERFACE_COUNT = dev->config->bNumInterfaces;
				usb_detach_kernel_driver_np(hdev, dev->config->interface->altsetting->bInterfaceNumber);
				if (HID_INTERFACE_COUNT == 2) {
					//usb_detach_kernel_driver_np(hdev, 0);
					usb_detach_kernel_driver_np(hdev, 1);
				}

				if (usb_claim_interface(hdev, dev->config->interface->altsetting->bInterfaceNumber) != 0) {
					LD_ERR("claim interface 0 error\n");
					usb_close(hdev);
					return NULL;
				}

				if (HID_INTERFACE_COUNT == 2) {
					if (usb_claim_interface(hdev, 1) != 0) {
						LD_ERR("claim interface 1 error\n");
						usb_close(hdev);
						return NULL;
					}

					ENDPOINT_IN = 0x81;
					ENDPOINT_OUT = 0x82;
				} else {
					ENDPOINT_IN = 0x81;
					ENDPOINT_OUT = 0x02;
				}
				iliusb.ep_in = ENDPOINT_IN;
				iliusb.ep_out = ENDPOINT_OUT;
				if (dev->descriptor.idProduct == ILITEK_BL_PRODUCT_ID)
					is_usb_hid_old_bl = 1;

				LD_MSG("[%s] ILITEK usb_hid[0x%04X:0x%04X] found, devnum: %u, cnt: %d\n",
					__func__, dev->descriptor.idVendor,
					dev->descriptor.idProduct, dev->devnum,
					HID_INTERFACE_COUNT);

				return hdev;
			}
		}
	}
	LD_ERR("[%s] ILITEK usb_hid device not found\n", __func__);
	return NULL;
}

struct usb_dev_handle *open_usb_hid_device_with_pid()
{
	struct usb_bus *busses, *bus;
	is_usb_hid_old_bl = 0;

	usb_init();
	usb_find_busses();
	usb_find_devices();

	busses = usb_get_busses();
	for(bus = busses; bus; bus = bus->next)
	{
		struct usb_device *dev;
		for(dev = bus->devices; dev; dev = dev->next)
		{
			if(((dev->descriptor.idVendor == ILITEK_VENDOR_ID) || (dev->descriptor.idVendor == OTHER_VENDOR_ID) || (dev->descriptor.idVendor ==OTHER_VID))
					&& ((dev->descriptor.idProduct == hex_2_dec((char *)temp_ILITEK_PID, 4)) || dev->descriptor.idProduct == 0x0006))
			{
				LD_MSG("%s, ILITEK usb_hid device found, devnum=%u, 0x%04X:0x%04X\n", __func__, dev->devnum, dev->descriptor.idVendor, dev->descriptor.idProduct);
				ILITEK_PID = dev->descriptor.idProduct;
				struct usb_dev_handle *hdev = usb_open(dev);
				if(!hdev)
				{
					perror("open device error\n");
				}
				else
				{
					//print_device(dev);
					//usb_set_configuration(hdev, dev->config->bConfigurationValue);
					//usb_reset(hdev);
					HID_INTERFACE_COUNT = dev->config->bNumInterfaces;
					usb_detach_kernel_driver_np(hdev, dev->config->interface->altsetting->bInterfaceNumber);
					if(HID_INTERFACE_COUNT == 2)
					{
						//usb_detach_kernel_driver_np(hdev, 0);
						usb_detach_kernel_driver_np(hdev, 1);
					}

					if(usb_claim_interface(hdev, dev->config->interface->altsetting->bInterfaceNumber) != 0)
					{
						perror("claim interface 0 error\n");
						usb_close(hdev);
						hdev = NULL;
					}
					else
					{
						if(HID_INTERFACE_COUNT == 2)
						{
							if(usb_claim_interface(hdev, 1) != 0)
							{
								perror("claim interface 1 error\n");
								usb_close(hdev);
								hdev = NULL;
							}
							else
							{
								ENDPOINT_IN = 0x81;
								ENDPOINT_OUT = 0x82;
							}
						}
						else
						{
							ENDPOINT_IN = 0x81;
							ENDPOINT_OUT = 0x02;
						}

						if(dev->descriptor.idProduct == ILITEK_BL_PRODUCT_ID)
						{
							is_usb_hid_old_bl = 1;
						}
					}
					usb_clear_halt(hdev, ENDPOINT_IN);
					usb_clear_halt(hdev, ENDPOINT_OUT);
				}
				return hdev;
			}
		}
	}
	LD_ERR("%s, ILITEK usb_hid device not found\n", __func__);
	return NULL;
}
#endif

void i2c_read_data_enable(bool enable)
{
	if (enable)
		ioctl(fd, ILITEK_IOCTL_START_READ_DATA, 1);
	else
		ioctl(fd, ILITEK_IOCTL_STOP_READ_DATA, 0);
}

int OpenI2CDevice()
{

	fd = open(ILITEK_I2C_CONTROLLER, O_RDWR);

	if (fd < 0) {
		LD_ERR("%s, ilitek controller doesn't exist\n", __func__);
		return _FAIL;
	}

	if (active_interface == ACTIVE_INTERFACE_I2C_ADAPTER) {
		LD_MSG("%s, device node is i2c adapter\n", __func__);
		if (ioctl(fd, I2C_SLAVE_FORCE, ILITEK_DEFAULT_I2C_ADDRESS) < 0) {
			LD_ERR("%s, set i2c slave address, failed\n", __func__);
			return _FAIL;
		} else {
			if (ioctl(fd, I2C_TIMEOUT, ILITEK_DEFAULT_I2C_TIMEOUT) < 0) {
				LD_ERR("%s, set i2c timeout, failed\n", __func__);
				return _FAIL;
			} else if (ioctl(fd, I2C_RETRIES, ILITEK_DEFAULT_I2C_RETRY) < 0) {
				LD_ERR("%s, set i2c retry, failed\n", __func__);
				return _FAIL;
			}
		}
	} else {
		LD_MSG("%s, device node is %s\n", __func__, ILITEK_I2C_CONTROLLER);
	}

	i2c_read_data_enable(false);

	LD_MSG("[%s] driver_ver: %x\n", __func__, get_driver_ver());

	return _SUCCESS;
}

int InitDevice()
{
	int ret = _SUCCESS;

	if (inConnectStyle == _ConnectStyle_I2C_) {
		ret = OpenI2CDevice();
	} else if (inConnectStyle == _ConnectStyle_I2CHID_) {
		ret = open_hidraw_device();
	}

#ifdef CONFIG_ILITEK_USE_LIBUSB
	else if (inConnectStyle == _ConnectStyle_USB_) {
		hdev = open_usb_hid_device();
		if (!hdev)
			ret = _FAIL;
	} else if (inConnectStyle == _ConnectStyle_USBPID_) {
		hdev = open_usb_hid_device_with_pid();
		if (!hdev)
			ret = _FAIL;
	}
#endif

	return ret;
}

int write_data(int fd, unsigned char *buf, int len)
{
	int ret = _SUCCESS;

	switch (active_interface) {
	case ACTIVE_INTERFACE_I2C_ADAPTER:
		ret = write(fd, buf, len);
		break;
	case ACTIVE_INTERFACE_ILITEK_CTRL_I2C:
		ret = ioctl(fd, ILITEK_IOCTL_I2C_WRITE_LENGTH, len);
		ret = ioctl(fd, ILITEK_IOCTL_I2C_WRITE_DATA, buf);
		break;
	default:
		LD_ERR("%s, invalid active interface\n", __func__);
		break;
	}
	return ret;
}

int read_data(int fd, unsigned char *buf, int len)
{
	int ret = _SUCCESS;
	switch(active_interface)
	{
		case ACTIVE_INTERFACE_I2C_ADAPTER:
			ret = read(fd, buf, len);
			break;
		case ACTIVE_INTERFACE_ILITEK_CTRL_I2C:
			ret = ioctl(fd, ILITEK_IOCTL_I2C_READ_LENGTH, len);
			ret = ioctl(fd, ILITEK_IOCTL_I2C_READ_DATA, buf);
			break;
		default:
			LD_ERR("%s, invalid active interface\n", __func__);
			break;

	}
	return ret;
}

int hidraw_read(int fd, uint8_t *buf, int len, int timeout_ms,
		uint8_t cmd, bool check_validity, bool check_ack)
{
	int ret = 0, t_ms = 0;

	if (!buf)
		return _FAIL;

	do {
		ret = read(fd, buf, len);

		if (!check_validity && ret > 0)
			return ret;

		if (ret == len && buf[0] == 0x03 && buf[1] == 0xA3 &&
		    buf[2] == cmd) {
			if ((check_ack && buf[4] == 0xAC) || !check_ack)
				return ret;
		}

		debugBuffPrintf("[hidraw_read]:", buf, len);
		LD_DBG("[%s] failed, read again, ret: %d\n", __func__, ret);

		usleep(1000);
		t_ms += 1;
	} while (t_ms < timeout_ms);

	return _FAIL;
}

int usb_read(uint8_t *buf, int len, int __attribute__((unused)) timeout_ms,
	     uint8_t cmd, bool check_validity, bool check_ack)
{
	int error = _FAIL, retry = 5;

	if (!buf)
		return _FAIL;

	do {
#ifdef CONFIG_ILITEK_USE_LIBUSB
		error = usb_interrupt_read(hdev, ENDPOINT_IN, (char *)buf,
					   len, timeout_ms);
#endif

		if (!check_validity)
			return error;

		if (buf[0] == 0x03 && buf[1] == 0xA3) {
			if (!check_ack || (check_ack && buf[4] == 0xAC))
				return error;
		}

		debugBuffPrintf("[usb_read]:", buf, len);
		LD_ERR("cmd: %x, err: %d, errno: %d\n", cmd, error, errno);

		usleep(1000);
	} while (--retry > 0);

	return _FAIL;
}

unsigned int getLength(unsigned int len, unsigned int *reportID)
{
	if (len <= BYTE_64) {
		*reportID = REPORT_ID_64_BYTE;
		return BYTE_64;
	} else if (len <= BYTE_256) {
		*reportID = REPORT_ID_256_BYTE;
		return BYTE_256 + 1 + 6;
	} else if (len <= BYTE_1K + 1) {
		*reportID = REPORT_ID_1024_BYTE;
		return BYTE_1K + 1 + 6;
	} else if (len <= BYTE_2K + 1) {
		*reportID = REPORT_ID_2048_BYTE;
		return BYTE_2K + 1 + 6;
	}

	*reportID = REPORT_ID_4096_BYTE;

	return BYTE_4K + 1 + 6;
}

void debugBuffPrintf(const char *str, uint8_t *buf, int len)
{
	int i = 0;

	if (dbg_level < LOG_LEVEL_DBG)
		return;

	if (!buf || !len)
		return;

	LD_DBG("%s", str);
	for (i = 0; i < len; i++)
		LD_DBG("%02X,", buf[i]);
	LD_DBG(", len: %d\n", len);
}

uint16_t get_le16(const uint8_t *p)
{
	return p[0] | p[1] << 8;
}

/*
 * TransferData_HID will return HID format packet,
 * no matter which interface is selected.
 *
 * Write/Read length will be modified to aligned power of 2.
 * will return write/read length on success.
 */
int TransferData_HID(uint8_t *OutBuff, int writelen,
		     uint8_t *InBuff, int readlen, int timeout_ms)
{
	int ret = 0;
	unsigned int wlen, rlen;
	unsigned int w_report, r_report;
	uint8_t cmd;
	int __attribute__((unused)) retry = 50;

	wlen = getLength(writelen, &w_report);
	rlen = getLength(readlen, &r_report);

	if (!OutBuff)
		cmd = 0;
	else if (OutBuff[0] == 0x03)
		cmd = OutBuff[4];
	else
		cmd = OutBuff[6];

	if (writelen > 0)
		debugBuffPrintf("[OutBuff]:", OutBuff, wlen);

	switch (inConnectStyle) {
	case _ConnectStyle_I2C_:
		if (writelen > 0) {
			if (w_report == REPORT_ID_64_BYTE && OutBuff[1] == 0xA3)
				ret = write_data(fd, OutBuff + 4, writelen);
			else
				ret = write_data(fd, OutBuff + 6, writelen);
			if (ret < 0) {
				LD_ERR("[%s] I2C write fail, cmd: 0x%x, wlen: %d, ret: %d\n",
					__func__, cmd, writelen, ret);
				return ret;
			}
			usleep(1000);
		}
		if (readlen > 0) {
			if (r_report == REPORT_ID_64_BYTE &&
			    OutBuff[1] == 0xA3) {
				ret = read_data(fd, InBuff + 4, readlen);
				InBuff[0] = 0x03;
				InBuff[1] = 0xA3;
				InBuff[2] = cmd;
				InBuff[3] = readlen;
			} else {
				ret = read_data(fd, InBuff, readlen);
			}

			if (ret < 0)
				return ret;
		}
		break;

#ifdef CONFIG_ILITEK_USE_LIBUSB
	case _ConnectStyle_USB_:
WRITE_AGAIN:
		if (writelen > 0) {
			ret = usb_control_msg(hdev, 0x21, 0x09, w_report, 0, (char *)OutBuff, wlen, 10000);
			if (ret < 0) {
				if (cmd != ILITEK_TP_CMD_SOFTWARE_RESET) {
					LD_ERR("[%s] USB write fail, cmd: 0x%x, ret:%d, wlen:%d\n",
						__func__, cmd, ret, wlen);
				}
				return ret;
			}
		}

		if (readlen > 0) {
			if (r_report == REPORT_ID_64_BYTE) {
				ret = usb_read(InBuff, rlen, timeout_ms,
					       cmd, true, false);
				if (ret < 0) {
					LD_ERR("USB read failed write again, cmd: %x\n",
						cmd);
					if (--retry > 0)
						goto WRITE_AGAIN;
				}
			} else {
				ret = usb_control_msg(hdev, 0xA1, 0x01, r_report, 0, (char *)InBuff, rlen, 10000);
			}

			if (ret < 0)
				return ret;
		}
		break;
#endif

	case _ConnectStyle_I2CHID_:
		if (writelen > 0) {
			/* only V3 251x HID-I2C need write syscall */
			if (inProtocolStyle == _Protocol_V3_ &&
			    w_report == REPORT_ID_64_BYTE)
				ret = write(fd, OutBuff, wlen);
			else
				ret = ioctl(fd, HIDIOCSFEATURE(wlen), OutBuff);
			if (ret < 0) {
				LD_ERR("[%s] I2C-HID write fail, cmd: 0x%x, ret:%d, wlen:%d\n",
					__func__, cmd, ret, wlen);
				return ret;
			}
		}

		if (readlen > 0) {
			if (r_report == REPORT_ID_64_BYTE) {
				ret = hidraw_read(fd, InBuff, rlen,
						  timeout_ms + 100,
						  cmd, true, false);
			} else {
				/* Must set report id before IOCTL */
				InBuff[0] = r_report & 0xFF;
				ret = ioctl(fd, HIDIOCGFEATURE(rlen), InBuff);
			}

			if (ret < 0) {
				LD_ERR("[%s] I2C-HID Read fail, cmd: 0x%x, ret:%d\n",
					__func__, cmd, ret);
				return ret;
			}
		}

		break;
	default:
		LD_ERR("unexpected inConnectStyle: %d\n", inConnectStyle);
		return _FAIL;
	};

	if (readlen > 0)
		debugBuffPrintf("[InBuff]:", InBuff, rlen);

	return 0;
}

/*
 * TransferData will make sure write buffer is HID format
 * before enter _TransferData.
 */
int TransferData(uint8_t *OutBuff, int writelen, uint8_t *InBuff,
		 int readlen, int timeout_ms)
{
	int error = _SUCCESS;
	uint8_t WriteBuff[8192], ReadBuff[8192];
	uint32_t w_report = 0, wlen = 0;
	uint32_t r_report = 0, rlen = 0;

	wlen = getLength(writelen, &w_report);
	rlen = getLength(readlen, &r_report);

	if (writelen > 0 && w_report == REPORT_ID_64_BYTE &&
	    readlen > 0 && r_report != REPORT_ID_64_BYTE) {
		error = TransferData(OutBuff, writelen, NULL, 0, timeout_ms);
		if (error < 0)
			return error;
		return TransferData(NULL, 0, InBuff, readlen, timeout_ms);
	}

	if (writelen > 0)
		debugBuffPrintf("[Write]:", OutBuff, writelen);

	memset(WriteBuff, 0, wlen);
	memset(ReadBuff, 0, rlen);

	if (w_report == REPORT_ID_64_BYTE) {
		WriteBuff[0] = w_report & 0xFF;
		WriteBuff[1] = 0xA3;
		WriteBuff[2] = writelen;
		WriteBuff[3] = readlen;
		memcpy(WriteBuff + 4, OutBuff, writelen);
	} else {
		WriteBuff[0] = w_report & 0xFF;
		WriteBuff[1] = 0xA3;
		WriteBuff[2] = writelen & 0xFF;
		WriteBuff[3] = (writelen >> 8) & 0xFF;
		WriteBuff[4] = readlen & 0xFF;
		WriteBuff[5] = (readlen >> 8) & 0xFF;
		memcpy(WriteBuff + 6, OutBuff, writelen);
	}

	error = TransferData_HID(WriteBuff, writelen,
				 ReadBuff, readlen, timeout_ms);

	switch (inConnectStyle) {
	case _ConnectStyle_I2C_:
		if (r_report == REPORT_ID_64_BYTE)
			memcpy(InBuff, ReadBuff + 4, readlen);
		else
			memcpy(InBuff, ReadBuff, readlen);
		break;

	case _ConnectStyle_USB_:
	case _ConnectStyle_I2CHID_:
		if (r_report == REPORT_ID_64_BYTE)
			memcpy(InBuff, ReadBuff + 4, readlen);
		else
			memcpy(InBuff, ReadBuff, rlen);
		break;
	default:
		LD_ERR("unexpected inConnectStyle: %d\n", inConnectStyle);
	};

	if (error < 0) {
		if ((OutBuff && OutBuff[0] != ILITEK_TP_CMD_SOFTWARE_RESET) ||
		    !OutBuff)
			return error;
	}

	if (readlen > 0)
		debugBuffPrintf("[Read]:", InBuff, readlen);

	return 0;
}

void CloseDevice()
{
	switch (inConnectStyle) {
	case _ConnectStyle_I2C_:
		i2c_read_data_enable(true);
		close(fd);
		break;
	case _ConnectStyle_I2CHID_:
		close(fd);
		break;
#ifdef CONFIG_ILITEK_USE_LIBUSB
	case _ConnectStyle_USB_:
		usb_release_interface(hdev, 0);
		if (HID_INTERFACE_COUNT == 2)
			usb_release_interface(hdev, 1);

		usb_close(hdev);
		break;
#endif
	}
}

int switch_irq(int flag)
{
	int buf[16];
	int ret = 0;

	if (inConnectStyle == _ConnectStyle_I2C_) {
		buf[0] = flag;
		ret = ioctl(fd, ILITEK_IOCTL_I2C_SWITCH_IRQ, buf);
	}
	return ret;
}

void viDriverCtrlReset()
{
	if (inConnectStyle == _ConnectStyle_I2C_) {
		LD_MSG("Set Driver ioctl reset TP\n");
		ioctl(fd, ILITEK_IOCTL_SET_RESET, 0);
	}
}

/* Default wait ack timeout should be 1500000 us */
int viWaitAck(uint8_t cmd, uint8_t *buf, int timeout_ms)
{
	int error;

	switch (inConnectStyle) {
	case _ConnectStyle_USB_:
		error = usb_read(buf, 64, timeout_ms, cmd, true, true);
		break;
	case _ConnectStyle_I2CHID_:
		error = hidraw_read(fd, buf, 64, timeout_ms, cmd, true, true);
		break;
	default:
		error = _FAIL;
		break;
	}

	if (error < 0) {
		LD_ERR("timeout_ms: %d, cmd: %x, err: %d\n",
			timeout_ms, cmd, error);
		return _FAIL;
	}

	return _SUCCESS;
}

int read_report(char *buf, int len, int t_ms)
{
	int error;

	switch (inConnectStyle) {

#ifdef CONFIG_ILITEK_USE_LIBUSB
	case _ConnectStyle_USB_:
		error = usb_read((uint8_t *)buf, len, t_ms, 0, false, false);
		break;
#endif

	case _ConnectStyle_I2CHID_:
		error = hidraw_read(fd, (uint8_t *)buf, len,
				    t_ms, 0, false, false);
		break;
	default:
		error = _FAIL;
	}

	return error;
}

/*
 * Create netlink socket and Initialization msg buffer
 * Notify user thread's pid to kernel driver
 * Please make sure kernel driver is ready and supports netlink flow
 */
int netlink_connect(struct Netlink_Handle *nl, const char *str,
		    uint32_t size, uint32_t tout_ms)
{
	int error;
	struct timeval tv;

	nl->data_size = size;

	/* Create netlink socket and bind with thread's pid */
	nl->fd = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_USERSOCK);
	if (nl->fd < 0) {
		LD_ERR("netlink socket create failed, err: %d\n", nl->fd);
		return _FAIL;
	}

	tv.tv_sec = tout_ms / 1000;
	tv.tv_usec = 1000 * (tout_ms % 1000);
	setsockopt(nl->fd, SOL_SOCKET, SO_RCVTIMEO,
		   (const char*)&tv, sizeof(tv));

	memset(&nl->src_addr, 0, sizeof(nl->src_addr));
	nl->src_addr.nl_family = AF_NETLINK;
	nl->src_addr.nl_pid = getpid();	/* bind sockt with specific pid */
	nl->src_addr.nl_groups = 0;

	if (bind(nl->fd, (struct sockaddr *)&nl->src_addr,
	    sizeof(nl->src_addr)) < 0)
		goto err_close_socket;

	/* msg struct initialization */
	nl->nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(size));
	if (!nl->nlh)
		goto err_close_socket;
	memset(nl->nlh, 0, NLMSG_SPACE(size));
	memset(&nl->dest_addr, 0, sizeof(nl->dest_addr));
	memset(&nl->msg, 0, sizeof(nl->msg));
	nl->dest_addr.nl_family = AF_NETLINK;
	nl->iov.iov_base = (void *)nl->nlh;
	nl->iov.iov_len = NLMSG_SPACE(size);
	nl->msg.msg_name = (void *)&nl->dest_addr;
	nl->msg.msg_namelen = sizeof(nl->dest_addr);
	nl->msg.msg_iov = &nl->iov;
	nl->msg.msg_iovlen = 1;

	/* Notify user thread pid to kernel */
	nl->nlh->nlmsg_pid = getpid();	/* kernel unicast to specific pid */
	nl->nlh->nlmsg_len = NLMSG_SPACE(size);
	strcpy((char *)NLMSG_DATA(nl->nlh), str);

	error = sendmsg(nl->fd, &nl->msg, 0);
	if (error < 0) {
		LD_ERR("netlink sendmsg failed, err: %d\n", error);
		goto err_free_nlh;
	}

	LD_MSG("netlink socket create success, nl_socket: %d\n", nl->fd);

	return 0;

err_free_nlh:
	free(nl->nlh);
err_close_socket:
	close(nl->fd);

	/* notify following flow netlink is not connected */
	nl->fd = -1;
	nl->nlh = NULL;

	LD_ERR("%s failed\n", __func__);

	return _FAIL;
}

void netlink_disconnect(struct Netlink_Handle *nl, const char *str)
{
	strcpy((char *)NLMSG_DATA(nl->nlh), str);
	sendmsg(nl->fd, &nl->msg, 0);

	if (nl->nlh)
		free(nl->nlh);
	if (nl->fd >= 0)
		close(nl->fd);
}

int netlink_recv(struct Netlink_Handle *nl, char *buf)
{
	int error;

	if (!nl->nlh || nl->fd < 0)
		return _FAIL;

	error = recvmsg(nl->fd, &nl->msg, 0);
	if (error < 0)
		return _FAIL;

	memcpy(buf, NLMSG_DATA(nl->nlh), nl->data_size);

	return 0;
}

void init_INT()
{
	if (!support_INT_ack || inProtocolStyle != _Protocol_V6_ ||
	    inConnectStyle != _ConnectStyle_I2C_)
		return;

	ioctl(fd, ILITEK_IOCTL_I2C_INT_CLR, 0);
	switch_irq(1);
}

bool wait_INT(int timeout_ms)
{
	uint8_t get_INT[64];
	bool ret = false;
	int t_ms = timeout_ms;

	if (!support_INT_ack || inProtocolStyle != _Protocol_V6_ ||
	    inConnectStyle != _ConnectStyle_I2C_) {
		LD_MSG("[%s] not support or disabled\n", __func__);
		return false;
	}

	while (t_ms > 0) {
		ioctl(fd, ILITEK_IOCTL_I2C_INT_POLL, &get_INT);

		if (get_INT[0]) {
			ret = true;
			break;
		}
		t_ms--;
		usleep(1000);
	}

	switch_irq(0);
	if (!ret)
		LD_ERR("%d ms timeout failed\n", timeout_ms);

	return ret;
}

uint32_t get_driver_ver()
{
	int error;
	uint32_t driver_ver;
	uint8_t buf[8];

	if (inConnectStyle != _ConnectStyle_I2C_)
		return 0;

	error = ioctl(fd, ILITEK_IOCTL_DRIVER_INFORMATION, &buf);
	if (error < 0)
		return 0;

	driver_ver = (buf[0] << 24) + (buf[1] << 16) + (buf[2] << 8) + buf[3];

	return driver_ver;
}

int write_and_wait_ack(uint8_t *Wbuff, int wlen, int timeout_ms,
	int cnt, int delay_ms, int type)
{
	int error;
	uint8_t Rbuff[64];

	switch (inConnectStyle) {
	case _ConnectStyle_USB_:
	case _ConnectStyle_I2CHID_:
		error = TransferData(Wbuff, wlen, NULL, 0, timeout_ms);
		if (viWaitAck(Wbuff[0], Rbuff, timeout_ms) < 0)
			return _FAIL;
		break;
	case _ConnectStyle_I2C_:
		init_INT();

		error = TransferData(Wbuff, wlen, NULL, 0, 0);

		if (!wait_INT(timeout_ms)) {
			if (CheckBusy(cnt, delay_ms, type) < 0)
				return _FAIL;
		}
		break;
	default:
		LD_ERR("unexpected interface: %d\n", inConnectStyle);
		return _FAIL;
	}

	return error;
}

int set_engineer(bool enable)
{
	uint8_t buf[64];

	buf[0] = 0x03;
	buf[1] = 0xF1;

	if (enable)
		buf[2] = 0x01;
	else
		buf[2] = 0x00;

	if (inConnectStyle == _ConnectStyle_I2CHID_)
		return write(fd, buf, 64);

	return 0;
}
