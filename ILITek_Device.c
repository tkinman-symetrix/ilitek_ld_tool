
#include "ILITek_Protocol.h"
#include "ILITek_CMDDefine.h"
#include "ILITek_Device.h"
#include <stdint.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define CRC_POLY 0x8408      // CRC16-CCITT FCS (X^16+X^12+X^5+1)
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
//I2C-HID
char hidraw_path[64];

#ifdef CONFIG_ILITEK_USE_LIBUSB
ILIUSB_DEVICE iliusb;
#endif

unsigned int hex_2_dec(char *hex, int len)
{
	unsigned int ret = 0, temp = 0;
	int i, shift = (len - 1) * 4;

	for(i = 0; i < len; shift -= 4, i++)
	{
		if((hex[i] >= '0') && (hex[i] <= '9'))
		{
			temp = hex[i] - '0';
		}
		else if((hex[i] >= 'a') && (hex[i] <= 'f'))
		{
			temp = (hex[i] - 'a') + 10;
		}
		else if((hex[i] >= 'A') && (hex[i] <= 'F'))
		{
			temp = (hex[i] - 'A') + 10;
		}
		else
		{
			return _FAIL;
		}
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
	unsigned int i=0;
	// Process each byte in the page into the running CRC
	for(i = startAddr; i < endAddr; i++)
	{
		CRC = UpdateCRC (CRC, input[i]);
	}
	return CRC;
}
//------------------------------------------------------------------
int SetConnectStyle(char *argv[])
{
	int ret = _SUCCESS;
	int hidraw_path_idx = 4;

	if(strcmp(argv[2], "I2C") == 0)
	{
		inConnectStyle=_ConnectStyle_I2C_;
		strcpy(ILITEK_I2C_CONTROLLER,argv[4]);
		ILITEK_DEFAULT_I2C_ADDRESS = hex_2_dec(argv[5], 2);
		if(strcmp(argv[3], "V3") == 0)
		{
			inProtocolStyle = _Protocol_V3_;
		}
		else if(strcmp(argv[3], "V6") == 0)
		{
			inProtocolStyle = _Protocol_V6_;
		}
		else
		{
			if((strcmp(argv[1], "Console") != 0)&&(strcmp(argv[1], "Script") != 0))
			{
				ret = _FAIL;
			}
			else
			{
				inProtocolStyle = _Protocol_V3_;
				strcpy(ILITEK_I2C_CONTROLLER,"/dev/ilitek_ctrl");
				ILITEK_DEFAULT_I2C_ADDRESS = 41;
			}
		}
	}
	else if(strcmp(argv[2], "USB") == 0)
	{
		inConnectStyle = _ConnectStyle_USB_;
		OTHER_VID = hex_2_dec(argv[5], 4);
		if(strcmp(argv[3], "V3") == 0)
		{
			inProtocolStyle = _Protocol_V3_;
		}
		else if(strcmp(argv[3], "V6") == 0)
		{
			inProtocolStyle = _Protocol_V6_;
		}
		else
		{
			if((strcmp(argv[1], "Console") != 0)&&(strcmp(argv[1], "Script") != 0))
			{
				ret = _FAIL;
			}
			else
			{
				inProtocolStyle = _Protocol_V3_;
			}
		}
	}
	else if (!strcmp(argv[2], "I2C-HID")) {
		inConnectStyle = _ConnectStyle_I2CHID_;

		if (strcmp(argv[3], "V3") == 0) {
			inProtocolStyle = _Protocol_V3_;
		} else if (strcmp(argv[3], "V6") == 0) {
			inProtocolStyle = _Protocol_V6_;
		} else if (strcmp(argv[1], "Console") == 0 || strcmp(argv[1], "Script") == 0) {
			hidraw_path_idx = 3;
			inProtocolStyle = _Protocol_V3_;
		} else {
			ret = _FAIL;
		}

		if (strlen(argv[hidraw_path_idx]) >= sizeof(hidraw_path)) {
			PRINTF("wrong args:%s\n", argv[hidraw_path_idx]);
			ret = _FAIL;
		} else {
			strcpy(hidraw_path, argv[hidraw_path_idx]);
		}
	} else {
		ret = _FAIL;
	}

	return ret;
}

int open_hidraw_device()
{
	struct hidraw_devinfo device_info;
	char device_name[256];

	fd = open(hidraw_path, O_RDWR | O_NONBLOCK);
	PRINTF("hidraw_path:%s, fd = %d\n", hidraw_path, fd);

	if (fd > 0) {
		ioctl(fd, HIDIOCGRAWINFO, &device_info);
		if(device_info.vendor == ILITEK_VENDOR_ID) {
			ioctl(fd, HIDIOCGRAWNAME(256), device_name);
			PRINTF("vid = %x, type = %d, name:%s\n", device_info.vendor,
					device_info.bustype, device_name);
			return _SUCCESS;
		}
	}

	// close it if failed.
	close(fd);
	return _FAIL;
}

#ifdef CONFIG_ILITEK_USE_LIBUSB
struct usb_dev_handle *open_usb_hid_device()
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
			if((dev->descriptor.idVendor == ILITEK_VENDOR_ID) || (dev->descriptor.idVendor == OTHER_VENDOR_ID)
					|| (dev->descriptor.idVendor == OTHER_VID))
			{
				PRINTF("%s, ILITEK usb_hid device found, devnum=%u, 0x%04X:0x%04X\n", __func__, dev->devnum, dev->descriptor.idVendor, dev->descriptor.idProduct);
				ILITEK_PID = dev->descriptor.idProduct;
				ILITEK_VID = dev->descriptor.idVendor;
				struct usb_dev_handle *hdev = usb_open(dev);
				iliusb.dev = hdev;
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
						iliusb.ep_in = ENDPOINT_IN;
						iliusb.ep_out = ENDPOINT_OUT;
						if(dev->descriptor.idProduct == ILITEK_BL_PRODUCT_ID)
						{
							is_usb_hid_old_bl = 1;
						}
					}
				}
				return hdev;
			}
		}
	}
	PRINTF("%s, ILITEK usb_hid device not found\n", __func__);
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
				PRINTF("%s, ILITEK usb_hid device found, devnum=%u, 0x%04X:0x%04X\n", __func__, dev->devnum, dev->descriptor.idVendor, dev->descriptor.idProduct);
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
	PRINTF("%s, ILITEK usb_hid device not found\n", __func__);
	return NULL;
}
#endif
int OpenI2CDevice()
{

	fd = open(ILITEK_I2C_CONTROLLER, O_RDWR);

	if(fd<0)
	{
		PRINTF("%s, ilitek controller doesn't exist\n", __func__);
		return _FAIL;
	}

	if(active_interface == ACTIVE_INTERFACE_I2C_ADAPTER)
	{
		PRINTF("%s, device node is i2c adapter\n", __func__);
		if(ioctl(fd, I2C_SLAVE_FORCE, ILITEK_DEFAULT_I2C_ADDRESS) < 0)
		{
			PRINTF("%s, set i2c slave address, failed\n", __func__);
			return _FAIL;
		}
		else
		{
			if(ioctl(fd, I2C_TIMEOUT, ILITEK_DEFAULT_I2C_TIMEOUT) < 0)
			{
				PRINTF("%s, set i2c timeout, failed\n", __func__);
				return _FAIL;
			}
			else
			{
				if(ioctl(fd, I2C_RETRIES, ILITEK_DEFAULT_I2C_RETRY) < 0)
				{
					PRINTF("%s, set i2c retry, failed\n", __func__);
					return _FAIL;
				}
				else
				{
					//PRINTF("%s, set i2c adapter, finish\n", __func__);
				}
			}
		}
	}
	else
	{
		PRINTF("%s, device node is %s\n", __func__, ILITEK_I2C_CONTROLLER);
	}
	return _SUCCESS;
}

int InitDevice()
{
	int8_t ret = _SUCCESS;
	if(inConnectStyle==_ConnectStyle_I2C_)
	{
		ret = OpenI2CDevice();
	}
#ifdef CONFIG_ILITEK_USE_LIBUSB
	else if(inConnectStyle==_ConnectStyle_USB_) {
		hdev=open_usb_hid_device();
		if(!hdev)
			ret = _FAIL;
	}
	else if(inConnectStyle==_ConnectStyle_USBPID_) {
		hdev=open_usb_hid_device_with_pid();
		if(!hdev)
			ret = _FAIL;
	}
#endif
	else if(inConnectStyle==_ConnectStyle_I2CHID_) {
		ret = open_hidraw_device();
	}

	return (int)ret;
}

int write_data(int fd, unsigned char *buf, int len)
{
	int ret = _SUCCESS;
	switch(active_interface)
	{
		case ACTIVE_INTERFACE_I2C_ADAPTER:
			ret = write(fd, buf, len);
			break;
		case ACTIVE_INTERFACE_ILITEK_CTRL_I2C:
			ret = ioctl(fd, ILITEK_IOCTL_I2C_WRITE_LENGTH, len);
			ret = ioctl(fd, ILITEK_IOCTL_I2C_WRITE_DATA, buf);
			break;
		default:
			PRINTF("%s, invalid active interface\n", __func__);
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
			PRINTF("%s, invalid active interface\n", __func__);
			break;

	}
	return ret;
}

int hidraw_read(int fd, uint8_t *buf, int len, int timeout_ms)
{
	int ret = 0, t_ms = 0;

	if (!buf)
		return _FAIL;

	do {
		ret = read(fd, buf, len);
		usleep(1000);
		t_ms += 1000;
	} while (ret != len && t_ms < timeout_ms);
	return (ret == len) ? _SUCCESS : _FAIL;
}

int TransferData(uint8_t *OutBuff, int writelen, uint8_t *InBuff, int readlen, int inTimeOut)
{
	int ret = _SUCCESS;

#ifdef DEBUG_TRANSFER_DATA
	if(writelen > 0) {
		printf("Len %d,W:", writelen);
		for(i = 0; i < writelen; i++)
			printf("0x%x,", OutBuff[i]);
		printf("\n");
	}
#endif
	if(inConnectStyle==_ConnectStyle_I2C_)
	{
		if(writelen != 0)
		{
			ret = write_data(fd, OutBuff, writelen);
			//printf("CMD:0x%x len:%d\n", OutBuff[0], writelen);
			usleep(inTimeOut);
		}
		if(ret >= 0 && readlen != 0)
		{
			ret = read_data(fd, InBuff, readlen);
			//printf("CMD:0x%x len:%d read:%d\n", OutBuff[0], writelen, readlen);
			if(ret < 0)
			{
				PRINTF("%s, read command fail, ret=%d\n", __func__, ret);
				return _FAIL;
			}
		}
		else if (ret < 0)
		{
			PRINTF("%s, write 0x%x command fail, ret=%d\n", __func__, OutBuff[0],ret);
			return _FAIL;
		}
	} else {
		uint8_t *WriteBuff = NULL, *ReadBuff = NULL;
		//bool INTInFlag = true;
		uint32_t w_report = 0, wlen = 0;
		uint32_t r_report = 0, rlen = 0;

		if(writelen > BYTE_2K + 1)
			writelen = BYTE_2K + 1;
		if(readlen > BYTE_2K)
			readlen = BYTE_2K;

		if(writelen + 4 <= BYTE_64) {
			w_report = REPORT_ID_64_BYTE;
			wlen = BYTE_64;
		}
		else if(writelen > BYTE_64 && writelen <= BYTE_256){
			w_report = REPORT_ID_256_BYTE;
			wlen = BYTE_256 + 1 + 6;
		}
		else if(writelen > BYTE_256 && writelen <= BYTE_1K + 1){
			w_report = REPORT_ID_1024_BYTE;
			wlen = BYTE_1K + 1 + 6;
		}
		else if(writelen > BYTE_1K+1 && writelen <= BYTE_2K + 1){
			w_report = REPORT_ID_2048_BYTE;
			wlen = BYTE_2K + 1 + 6;
		}
		else if(writelen > BYTE_2K + 1 && writelen <= BYTE_4K + 1){
			w_report = REPORT_ID_4096_BYTE;
			wlen = BYTE_4K + 1 + 6;
		}
		WriteBuff = (uint8_t *)calloc(wlen, sizeof(uint8_t));

		if(WriteBuff == NULL) {
			PRINTF("WriteBuff: unable to allocate required memory\n");
			return _FAIL;
		}
		if(readlen + 4 <= BYTE_64) {
			r_report = REPORT_ID_64_BYTE;
			rlen = BYTE_64;
		}
		else if(readlen > BYTE_64 && readlen <= BYTE_256){
			r_report = REPORT_ID_256_BYTE;
			rlen = BYTE_256+1 + 6;
		}
		else if(readlen > BYTE_256 && readlen <= BYTE_1K){
			r_report = REPORT_ID_1024_BYTE;
			rlen = BYTE_1K + 1 + 6;
		}
		else if(readlen > BYTE_1K && readlen <= BYTE_2K){
			r_report = REPORT_ID_2048_BYTE;
			rlen = BYTE_2K + 1 + 6;
		}
		else if(readlen > BYTE_2K && readlen <= BYTE_4K){
			r_report = REPORT_ID_4096_BYTE;
			rlen = BYTE_4K + 1 + 6;
		}
		ReadBuff = (uint8_t *)calloc(rlen, sizeof(uint8_t));
		if(ReadBuff == NULL) {
			free(WriteBuff);
			PRINTF("WriteBuff: unable to allocate required memory\n");
			return _FAIL;
		}
		if(w_report == REPORT_ID_64_BYTE) {
			WriteBuff[0] = w_report & 0xFF;
			WriteBuff[1] = 0xA3;
			WriteBuff[2] = writelen;
			WriteBuff[3] = readlen;
			memcpy(WriteBuff + 4, OutBuff, writelen);
		}
		else{
			WriteBuff[0] = w_report & 0xFF;
			WriteBuff[1] = 0xA3;
			WriteBuff[2] = writelen & 0xFF;
			WriteBuff[3] = (writelen >> 8) & 0xFF;
			WriteBuff[4] = readlen & 0xFF;
			WriteBuff[5] = (readlen >> 8) & 0xFF;
			memcpy(WriteBuff + 6, OutBuff, writelen);
		}
#ifdef CONFIG_ILITEK_USE_LIBUSB
		if (inConnectStyle==_ConnectStyle_USB_) {
			int retry_count = 0;
WRITE_AGAIN:
			if (writelen == 0 || usb_control_msg(hdev, 0x21, 0x09, w_report, 0, (char *)WriteBuff, wlen, 10000) > 0) //bl nk ap ok
			{
				if (readlen > 0) {
					usleep(1000);
					if(r_report == REPORT_ID_64_BYTE)
					{
AGAIN:
						ret = usb_interrupt_read(hdev, ENDPOINT_IN, (char *)ReadBuff, 64, (inTimeOut+3000));
						if(ret < 0)
						{
#ifdef DEBUG_TRANSFER_DATA
							PRINTF("%s, CMD:0x%x read command fail\n", __func__, OutBuff[0]);
#endif
							//return ret;
						}
						if(ReadBuff[0] != (REPORT_ID_64_BYTE & 0xFF) || ReadBuff[1] != (uint8_t)0xA3) {
#ifdef DEBUG_TRANSFER_DATA
							PRINTF("IN data Report ID:0x%x,0x%x, must again read\n", ReadBuff[0], ReadBuff[0]);
#endif
							retry_count++;
							if(retry_count >= 3)
								goto WRITE_AGAIN;
							goto AGAIN;
						}
						memcpy(InBuff, ReadBuff + 4, readlen);
					}
					else
					{
						ret = usb_control_msg(hdev, 0xA1, 0x01, r_report, 0, (char *)ReadBuff, rlen, 10000);
						if(ret < 0)
						{
							PRINTF("%s, read command fail\n", __func__);
							return _FAIL;
						}
						if(r_report == REPORT_ID_64_BYTE) {
							memcpy(InBuff, ReadBuff + 4, readlen);
						}
						else {
							memcpy(InBuff, ReadBuff, rlen);
						}
					}
				}
			} else {
				if(OutBuff[0] != ILITEK_TP_CMD_SOFTWARE_RESET) {
					PRINTF("%s, Send command fail W:0x%x ,WL:%d\n", __func__, OutBuff[0], writelen);
					return _FAIL;
				}
			}
		}
#endif
		if (inConnectStyle==_ConnectStyle_I2CHID_) {
			if (writelen == 0 || ioctl(fd, HIDIOCSFEATURE(wlen), WriteBuff) > 0) {
				if (readlen > 0) {
					usleep(1000);

					if (r_report == REPORT_ID_64_BYTE) {
						ret = hidraw_read(fd, ReadBuff, 64, inTimeOut + 100000);
					} else {
						ReadBuff[0] = r_report & 0xFF; // Must set report ID
						ret = ioctl(fd, HIDIOCGFEATURE(rlen), ReadBuff);
					}

					if (ret < 0) {
						PRINTF("%s, I2C-HID Read fail, ret:%d\n", __func__, ret);
						return _FAIL;
					}

					if (r_report == REPORT_ID_64_BYTE)
						memcpy(InBuff, ReadBuff + 4, readlen);
					else
						memcpy(InBuff, ReadBuff, readlen);
				}
				/* Cmd needs to wait Ack */
				else if (OutBuff[0] == ILITEK_TP_CMD_GET_BLOCK_CRC_FOR_ADDR ||
					 OutBuff[0] == ILITEK_TP_CMD_GET_BLOCK_CRC_FOR_NUM ||
					 OutBuff[0] == ILITEK_TP_CMD_ACCESS_SLAVE ||
					 OutBuff[0] == ILITEK_TP_CMD_SET_CDC_INITOAL_V6 ||
					 OutBuff[0] == ILITEK_TP_CMD_WRITE_DATA ||
					 OutBuff[0] == ILITEK_TP_CMD_GET_CDC_DATA_V6) {
					int i = 0, retryCnt = 5;

					for (i = 0; i < retryCnt; i++) {
						ret = hidraw_read(fd, ReadBuff, 64, inTimeOut + 1500000);
						if (ReadBuff[2] == OutBuff[0] &&
								ReadBuff[4] == 0xAC)
							break;
						PRINTF("[%s] Wait Ack failed(%d/%d) for %#x, ret = %d\n", __func__, i, retryCnt, OutBuff[0], ret);
						usleep(10000);
					}
					if (i == retryCnt) {
						PRINTF("[%s] Wait Ack failed(%d/%d) for %#x\n", __func__, i, retryCnt, OutBuff[0]);
						return _FAIL;
					}
				}
			} else {
				if (OutBuff[0] != ILITEK_TP_CMD_SOFTWARE_RESET) {
					PRINTF("%s, Send command fail W:0x%x ,WL:%d\n", __func__, OutBuff[0], writelen);
					return _FAIL;
				}
			}
		}
		usleep(3000);
		free(WriteBuff);
		free(ReadBuff);
	}
#ifdef DEBUG_TRANSFER_DATA
	if(readlen > 0) {
		printf("Len %d,R:", readlen);
		for(i = 0; i < readlen; i++)
			printf("0x%x,", InBuff[i]);
		printf("\n");
	}
#endif
	return _SUCCESS;
}

void CloseDevice()
{
	if(inConnectStyle==_ConnectStyle_I2C_ ||
			inConnectStyle==_ConnectStyle_I2CHID_)
	{
		close(fd);
	}
#ifdef CONFIG_ILITEK_USE_LIBUSB
	else
	{
		usb_release_interface(hdev, 0);
		if(HID_INTERFACE_COUNT == 2)
		{
			usb_release_interface(hdev, 1);
		}
		usb_close(hdev);
	}
#endif
}

int switch_irq(int flag)
{
	int buf[16];
	int ret = 0;

	if (inConnectStyle == _ConnectStyle_I2C_) {
		buf[0] = flag;
		usleep(10000);
		ret = ioctl(fd, ILITEK_IOCTL_I2C_SWITCH_IRQ, buf);
	}
	return ret;
}

void viDriverCtrlReset()
{
	PRINTF("Set Driver ioctl reset TP\n");
	ioctl(fd, ILITEK_IOCTL_SET_RESET, 0);
}
