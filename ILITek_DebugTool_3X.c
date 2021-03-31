
#include "ILITek_CMDDefine.h"
#include "ILITek_Device.h"

unsigned char *temp_ILITEK_PID;
FILE *fp_csvSd;
char raw_log_file_csvSd_tmp[512];
static const char *raw_log_file_csvSd;
static char raw_log_file_csvSd_root[512];
static int file_count = 0;

#define SAVE_DEBUGLOG
//using for i2c adapter check
int active_interface;
char ILITEK_I2C_CONTROLLER[255];
int ILITEK_DEFAULT_I2C_ADDRESS;
int ilitek_fd = 0;
//using for check bl is old
int is_usb_hid_old_bl = 0;

int debug_flag = 0;
int sock_fd = 0;
struct msghdr msg;
int exit_flag = 0;
//#define MAX_PAYLOAD 1024
int MAX_PAYLOAD=64;

int open_device(void)
{
    ilitek_fd = open("/proc/ilitek_ctrl", O_RDWR);
    if(ilitek_fd < 0)
    {
        ilitek_fd = open("/dev/ilitek_ctrl", O_RDWR);
        if(ilitek_fd < 0)
        {
            PRINTF("%s, ilitek controller doesn't exist or no control permission\n", __func__);
            return ilitek_fd;
        }
        else
        {
            PRINTF("%s, device node is /dev/ilitek_ctrl\n", __func__);
            return ilitek_fd;
        }
    }
    else
    {
        PRINTF("%s, device node is /proc/ilitek_ctrl\n", __func__);
        return ilitek_fd;
    }
}
void close_device(void)
{
    if (ilitek_fd > 0)
    {
        close(ilitek_fd);
        ilitek_fd = _FAIL;
    }
}

int debug_tool_get_data_form_kernel(void)
{
    PRINTF("debug_tool_get_data_form_kernel\n");
    struct sockaddr_nl src_addr, dest_addr;
    struct nlmsghdr *nlh = NULL;
    struct iovec iov;
    int ret;
    unsigned char buf[32] = {0};
    unsigned char sendbuf[64] = {0};
    int line_num = 0;
    unsigned char filename_end[16] = {0};
    unsigned char filename_start[128] = {0};
    time_t rawtime;
    struct tm *timeinfo;

    //MAX_PAYLOAD=sizeof(file_fw_data);
    //MAX_PAYLOAD=169;
    memset(&msg,0,sizeof(msg));
    sock_fd = socket(PF_NETLINK, SOCK_RAW,21);
    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = 100;//getpid();  /* self pid */
    src_addr.nl_groups = 0;  /* not in mcast groups */
    bind(sock_fd, (struct sockaddr *)&src_addr,sizeof(src_addr));

    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0;   /* For Linux Kernel */
    dest_addr.nl_groups = 0; /* unicast */

    nlh=(struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    /* Fill the netlink message header */
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nlh->nlmsg_pid = 100;//getpid();  /* self pid */
    nlh->nlmsg_flags = 0;
    /* Fill in the netlink message payload */
    strcpy(NLMSG_DATA(nlh), "Hello World!");
    //NLMSG_DATA(nlh)=file_fw_data;
    // memcpy(NLMSG_DATA(nlh),file_fw_data,MAX_PAYLOAD);
    ret = mkdir("/sdcard/ILITEK/", 0777);
    if (ret)
    {
        PRINTF("Unable to create directory\n");
    }
#ifdef SAVE_DEBUGLOG
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    PRINTF ( "\007The current date/time is: %s", asctime (timeinfo) );
    strcat(filename_start, "/sdcard/ILITEK/");
#if 0
    sprintf(buf, "%d_", (int)(timeinfo->tm_year));
    strcat(filename_start, buf);
    memset(buf,0,32);
    sprintf(buf, "%d_", (int)(timeinfo->tm_mon));
    strcat(filename_start, buf);
#endif
    memset(buf,0,32);
    sprintf(buf, "%d_", (int)(timeinfo->tm_mday));
    strcat(filename_start, buf);
    memset(buf,0,32);
    sprintf(buf, "%d_", (int)(timeinfo->tm_hour));
    strcat(filename_start, buf);
    memset(buf,0,32);
    sprintf(buf, "%d_", (int)(timeinfo->tm_min));
    strcat(filename_start, buf);
    memset(buf,0,32);
    sprintf(buf, "%d", (int)(timeinfo->tm_sec));
    strcat(filename_start, buf);
    PRINTF("filename_start = %s\n", filename_start);
    raw_log_file_csvSd = filename_start;//asctime (timeinfo);
    memset(raw_log_file_csvSd_root,0,512);
    memcpy(raw_log_file_csvSd_root,raw_log_file_csvSd,strlen(raw_log_file_csvSd));
    memset(raw_log_file_csvSd_tmp, 0, 512);
    memcpy(raw_log_file_csvSd_tmp, raw_log_file_csvSd_root,strlen(raw_log_file_csvSd_root));
#endif
    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    //jbyteArray jbyte;
    exit_flag = 0;
#if 0
    sendmsg(sock_fd, &msg, 0);
    PRINTF("Waiting for message from kernel 11111\n");
    ret=recvmsg(sock_fd, &msg, 0);
    memcpy(buf,NLMSG_DATA(msg.msg_iov->iov_base),MAX_PAYLOAD);
    if(ret>0)
    {
        PRINTF("Received message payload: %s\n",(char *)NLMSG_DATA(msg.msg_iov->iov_base));
        //break;
    }
    else
    {
        PRINTF("check error \n");
        //return -1;
    }
#endif
    /* Read message from kernel */
    if (debug_flag == 1)
    {
        PRINTF("ILITEK_IOCTL_DEBUG_SWITCH on  \n");
        buf[0] = 1;
        if(ioctl(ilitek_fd, ILITEK_IOCTL_DEBUG_SWITCH, buf) < 0)
        {
            PRINTF("%s, ILITEK_ERR_IOCTL_ERROR\n", __func__);
            //return ILITEK_ERR_IOCTL_ERROR;
        }
    }
#if 1
    while(1)
    {
        if (debug_flag == 0)
        {
            PRINTF("%s, ILITEK_IOCTL_DEBUG_SWITCH\n", __func__);
            buf[0] = 0;
            if(ioctl(ilitek_fd, ILITEK_IOCTL_DEBUG_SWITCH, buf) < 0)
            {
                PRINTF("%s, ILITEK_ERR_IOCTL_ERROR\n", __func__);
                //return ILITEK_ERR_IOCTL_ERROR;
            }
            break;
        }
        ret=recvmsg(sock_fd, &msg, 0);
        if(ret > 0)
        {
            memset(sendbuf,0,sizeof(sendbuf));
            memcpy(sendbuf,NLMSG_DATA(msg.msg_iov->iov_base),MAX_PAYLOAD);

            //PRINTF("\n\n0x%X\n\n", sendbuf[0]);
            if (sendbuf[0] == 0xDB && sendbuf[1] >= 2)
            {
                sendbuf[sendbuf[1]] = '\0';
                PRINTF("%s", &sendbuf[2]);
                fflush(NULL);
            }

            //i++;
#ifdef SAVE_DEBUGLOG
            if(line_num == 0)
            {
                sprintf(filename_end, "_%03d.csv", (int)(file_count + 1));
                strcat(raw_log_file_csvSd_tmp, filename_end);
                raw_log_file_csvSd = raw_log_file_csvSd_tmp;
                fp_csvSd = fopen(raw_log_file_csvSd, "w");
                if (fp_csvSd == NULL)
                {
                    PRINTF("Raw Data File csv Sd can't open !!!!\n");
                }
                else
                {
                    //PRINTF("Raw Data File csv Sd open successfully !!!!\n");
                }
            }
            if (line_num == (300000 -1))
            {
                line_num = -1;
#if 0
                sprintf(filename_end, "%3d", (int)(file_count + 1));
                strcat(raw_log_file_csvSd_tmp, filename_end);
                raw_log_file_csvSd = raw_log_file_csvSd_tmp;
                strcat(raw_log_file_csvApk_tmp, filename_end);
                raw_log_file_csvApk = raw_log_file_csvApk_tmp;
#endif
                memset(raw_log_file_csvSd_tmp, 0, 512);
                memcpy(raw_log_file_csvSd_tmp, raw_log_file_csvSd_root,strlen(raw_log_file_csvSd_root));
                //raw_log_file_csvSd_tmp = raw_log_file_csvSd_root;//strchr(raw_log_file_csvSd_root , '.');
                //raw_log_file_csvApk_tmp = raw_log_file_csvApk_root;//strchr(raw_log_file_csvApk_root , '.');
                //PRINTF("raw_log_file_csvApk_tmp = %s raw_log_file_csvSd_tmp = %s \n", raw_log_file_csvApk_tmp, raw_log_file_csvSd_tmp);
                //PRINTF("raw_log_file_csvSd_root = %s	raw_log_file_csvApk_root = %sn", raw_log_file_csvSd_root, raw_log_file_csvApk_root);
                file_count++;
            }

            if (fp_csvSd != NULL)
            {
                if (sendbuf[0] == 0xDB && sendbuf[1] >= 2)
                {
                    sendbuf[sendbuf[1]] = '\0';
                    fprintf(fp_csvSd, "%s", &sendbuf[2]);
                }
            }

            if (fp_csvSd != NULL)
            {
                fflush(fp_csvSd);
            }
#endif
            line_num++;
            /* Fill the netlink message header */
            nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
            nlh->nlmsg_pid = 100;//getpid();  /* self pid */
            nlh->nlmsg_flags = 0;
            /* Fill in the netlink message payload */
            //strcpy(NLMSG_DATA(nlh), "Hello World!");
            //NLMSG_DATA(nlh)=file_fw_data;
            if (debug_flag == 0)
            {
                PRINTF("%s, ILITEK_IOCTL_DEBUG_SWITCH\n", __func__);
                buf[0] = 0;
                if(ioctl(ilitek_fd, ILITEK_IOCTL_DEBUG_SWITCH, buf) < 0)
                {
                    PRINTF("%s, ILITEK_ERR_IOCTL_ERROR\n", __func__);
                    //return ILITEK_ERR_IOCTL_ERROR;
                }
                break;
            }
            //memcpy(NLMSG_DATA(nlh),sendbuf,MAX_PAYLOAD);
            //sendmsg(sock_fd, &msg, 0);
        }
        //PRINTF("[ILITEK] end of recvmsg \n");
    }
    PRINTF("[ILITEK] end of while \n");
#endif
    //memcpy(NLMSG_DATA(nlh),sendbuf,MAX_PAYLOAD);
    //sendmsg(sock_fd, &msg, 0);
    close(sock_fd);
    exit_flag = 1;
    return _SUCCESS;
}

void vfStartI2CDebug_3X()
{
    int ret;
    unsigned char buf[32] = {0};

    active_interface = ACTIVE_INTERFACE_ILITEK_CTRL_I2C;
    debug_flag = 1;
    ret = open_device();
    if (ret < 0)
    {
        PRINTF("open devices error check the driver is right or the Permission\n");
    }

    buf[0] = 1;
    if(ioctl(ilitek_fd, ILITEK_IOCTL_DEBUG_SWITCH, buf) < 0)
    {
        PRINTF("%s, ILITEK_IOCTL_DEBUG_SWITCH on ERROR\n", __func__);
        //return -1;
    }
    debug_tool_get_data_form_kernel();
    close_device();
}

void vfStopI2CDebug_3X()
{
    int ret;
    unsigned char buf[32] = {0};

    active_interface = ACTIVE_INTERFACE_ILITEK_CTRL_I2C;
    debug_flag = 0;
    ret = open_device();
    if (ret < 0)
    {
        PRINTF("open devices error check the driver is right or the Permission\n");
    }

    buf[0] = 0;
    if(ioctl(ilitek_fd, ILITEK_IOCTL_DEBUG_SWITCH, buf) < 0)
    {
        PRINTF("%s, ILITEK_IOCTL_DEBUG_SWITCH on\n", __func__);
        //	return -1;
    }
    close_device();
}

void vfRunUSBDebug_3X()
{
    int ret;
    unsigned char buf[32] = {0};
    int isStop = 0;
    int index_hidraw;
    char raw_devicenode[64];
    int fd;
    int size = 0;
    int line_num = 0;
    unsigned char filename_end[16] = {0};
    unsigned char filename_start[128] = {0};
    time_t rawtime;
    struct tm *timeinfo;
    struct hidraw_devinfo device_info;
    char device_name[256];
    int bufferlen = 63;
    int validlen;

    active_interface = ACTIVE_INTERFACE_ILITEK_CTRL_I2C;
    for(index_hidraw = 0; index_hidraw < 10; index_hidraw++)
    {
        sprintf(raw_devicenode, "/dev/hidraw%d", index_hidraw);
        PRINTF("raw_devicenode = %s\n", raw_devicenode);

        fd = open(raw_devicenode, O_RDWR);
        PRINTF("fd = %d\n", fd);
        if(fd > 0)
        {
            //check vid is 222a
            ioctl(fd, HIDIOCGRAWINFO, &device_info);
            if(device_info.vendor == 8746)
            {
                PRINTF("vid = %x\n", device_info.vendor);
                PRINTF("type = %d\n", device_info.bustype);
                ioctl(fd, HIDIOCGRAWNAME(256), device_name);
                PRINTF("name = %s\n", device_name);

                //get size check is mouse or touch or debug
                size = 0;
                ioctl(fd, HIDIOCGRDESCSIZE, &size);
                PRINTF("size = %d\n", size);

                //using debug endpoint
                if(size < 150)
                {
#ifdef SAVE_DEBUGLOG
                    time ( &rawtime );
                    timeinfo = localtime ( &rawtime );
                    PRINTF ( "\007The current date/time is: %s", asctime (timeinfo) );
                    strcat(filename_start, "/sdcard/ILITEK/");
#if 0
                    sprintf(buf, "%d_", (int)(timeinfo->tm_year));
                    strcat(filename_start, buf);
                    memset(buf,0,32);
                    sprintf(buf, "%d_", (int)(timeinfo->tm_mon));
                    strcat(filename_start, buf);
#endif
                    memset(buf,0,32);
                    sprintf(buf, "%d_", (int)(timeinfo->tm_mday));
                    strcat(filename_start, buf);
                    memset(buf,0,32);
                    sprintf(buf, "%d_", (int)(timeinfo->tm_hour));
                    strcat(filename_start, buf);
                    memset(buf,0,32);
                    sprintf(buf, "%d_", (int)(timeinfo->tm_min));
                    strcat(filename_start, buf);
                    memset(buf,0,32);
                    sprintf(buf, "%d", (int)(timeinfo->tm_sec));
                    strcat(filename_start, buf);
                    PRINTF("filename_start = %s\n", filename_start);
                    raw_log_file_csvSd = filename_start;//asctime (timeinfo);
                    memset(raw_log_file_csvSd_root,0,512);
                    memcpy(raw_log_file_csvSd_root,raw_log_file_csvSd,strlen(raw_log_file_csvSd));
                    memset(raw_log_file_csvSd_tmp, 0, 512);
                    memcpy(raw_log_file_csvSd_tmp, raw_log_file_csvSd_root,strlen(raw_log_file_csvSd_root));
#endif
                    while(isStop != 1)
                    {
                        ret = read(fd, buf, bufferlen);

                        //for(bufferindex = 0; bufferindex < 63; bufferindex++)
                        //{
                        //	PRINTF("buf[%d]=%d\n", bufferindex, buf[bufferindex]);
                        //}

                        //for 32 bit system
                        if(buf[0] == 0)
                        {
                            validlen = buf[2];
                            //PRINTF("validlen=%d\n", validlen);
                            //for(bufferindex = 3; bufferindex <= validlen; bufferindex++)
                            //{
                            //	PRINTF("%c", buf[bufferindex]);
                            //}
                            buf[buf[2]] = '\0';
                            PRINTF("%s", &buf[3]);
                            fflush(NULL);

#ifdef SAVE_DEBUGLOG
                            if(line_num == 0)
                            {
                                sprintf(filename_end, "_%03d.csv", (int)(file_count + 1));
                                strcat(raw_log_file_csvSd_tmp, filename_end);
                                raw_log_file_csvSd = raw_log_file_csvSd_tmp;
                                fp_csvSd = fopen(raw_log_file_csvSd, "w");
                                if (fp_csvSd == NULL)
                                {
                                    PRINTF("Raw Data File csv Sd can't open !!!!\n");
                                }
                                else
                                {
                                    //PRINTF("Raw Data File csv Sd open successfully !!!!\n");
                                }
                            }
                            if (line_num == (300000 -1))
                            {
                                line_num = -1;
#if 0
                                sprintf(filename_end, "%3d", (int)(file_count + 1));
                                strcat(raw_log_file_csvSd_tmp, filename_end);
                                raw_log_file_csvSd = raw_log_file_csvSd_tmp;
                                strcat(raw_log_file_csvApk_tmp, filename_end);
                                raw_log_file_csvApk = raw_log_file_csvApk_tmp;
#endif
                                memset(raw_log_file_csvSd_tmp, 0, 512);
                                memcpy(raw_log_file_csvSd_tmp, raw_log_file_csvSd_root,strlen(raw_log_file_csvSd_root));
                                //raw_log_file_csvSd_tmp = raw_log_file_csvSd_root;//strchr(raw_log_file_csvSd_root , '.');
                                //raw_log_file_csvApk_tmp = raw_log_file_csvApk_root;//strchr(raw_log_file_csvApk_root , '.');
                                //PRINTF("raw_log_file_csvApk_tmp = %s raw_log_file_csvSd_tmp = %s \n", raw_log_file_csvApk_tmp, raw_log_file_csvSd_tmp);
                                //PRINTF("raw_log_file_csvSd_root = %s	raw_log_file_csvApk_root = %sn", raw_log_file_csvSd_root, raw_log_file_csvApk_root);
                                file_count++;
                            }
#if 1
                            if (fp_csvSd != NULL)
                            {
                                //if (buf[1] == 0xDB && buf[2] >= 2) {
                                //buf[buf[2]] = '\0';
                                fprintf(fp_csvSd, "%s", &buf[3]);
                                //}
                            }
#endif
                            if (fp_csvSd != NULL)
                            {
                                fflush(fp_csvSd);
                            }
#endif

                            line_num++;
                        }
                        //for 64 bit system
                        else if(buf[0] == 2)
                        {
                            validlen = buf[1];
                            //PRINTF("validlen=%d\n", validlen);
                            //for(bufferindex = 2; bufferindex < validlen; bufferindex++)
                            //{
                            //	PRINTF("%c", buf[bufferindex]);
                            //}
                            buf[buf[1]] = '\0';
                            PRINTF("%s", &buf[2]);
                            fflush(NULL);

#ifdef SAVE_DEBUGLOG
                            if(line_num == 0)
                            {
                                sprintf(filename_end, "_%03d.csv", (int)(file_count + 1));
                                strcat(raw_log_file_csvSd_tmp, filename_end);
                                raw_log_file_csvSd = raw_log_file_csvSd_tmp;
                                fp_csvSd = fopen(raw_log_file_csvSd, "w");
                                if (fp_csvSd == NULL)
                                {
                                    PRINTF("Raw Data File csv Sd can't open !!!!\n");
                                }
                                else
                                {
                                    //PRINTF("Raw Data File csv Sd open successfully !!!!\n");
                                }
                            }
                            if (line_num == (300000 -1))
                            {
                                line_num = -1;
                                memset(raw_log_file_csvSd_tmp, 0, 512);
                                memcpy(raw_log_file_csvSd_tmp, raw_log_file_csvSd_root,strlen(raw_log_file_csvSd_root));
                                //raw_log_file_csvSd_tmp = raw_log_file_csvSd_root;//strchr(raw_log_file_csvSd_root , '.');
                                //raw_log_file_csvApk_tmp = raw_log_file_csvApk_root;//strchr(raw_log_file_csvApk_root , '.');
                                //PRINTF("raw_log_file_csvApk_tmp = %s raw_log_file_csvSd_tmp = %s \n", raw_log_file_csvApk_tmp, raw_log_file_csvSd_tmp);
                                //PRINTF("raw_log_file_csvSd_root = %s	raw_log_file_csvApk_root = %sn", raw_log_file_csvSd_root, raw_log_file_csvApk_root);
                                file_count++;
                            }
#if 1
                            if (fp_csvSd != NULL)
                            {
                                //if (buf[0] == 0xDB && buf[1] >= 2) {
                                //buf[buf[1]] = '\0';
                                fprintf(fp_csvSd, "%s", &buf[2]);
                                //}
                            }
#endif
                            if (fp_csvSd != NULL)
                            {
                                fflush(fp_csvSd);
                            }
#endif
                            line_num++;
                        }
                    }
                }
            }
        }
        close(fd);
    }
}

void vfRunDebug_3X(char *cStyle,int iRun)
{
    if(strcmp(cStyle, "I2C") == 0)
    {
        if(iRun == 0)
        {
            vfStopI2CDebug_3X();
        }
        else
        {
            vfStartI2CDebug_3X();
        }
    }
    else if(strcmp(cStyle, "USB") == 0)
    {
        vfRunUSBDebug_3X();
    }
    else
    {
        PRINTF("Error! Input Wrong Para!");
    }
}


