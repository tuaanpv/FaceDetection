#ifndef __SMS_H__
#define __SMS_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <string.h>
#include <string>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <ctime>
#include <thread>
#include <mutex>
#include <iomanip>
#include <algorithm>

#include <vector>
#include <map>
#include "colorText.h"


static int open_serial(char *dev);

static int serial_init(char *serial_port);

static int write_serial(char *string);

static int read_serial(char *buffer,int size,int offset);

static int sms_receipt(char *rbuf,int rsize,char *text1,char *text2);

static int write_and_check(char *wbuf, char *rbuf, int rsize, char *expect_answer);

int sms_init(char *sms_port);

int sms_close(void);

int sms_send(std::vector<string>& smsText,char *number);

int sms_read_one(char *rbuf,int size,char *sms_id);

static void sms_trans(struct _sms_buf *sms_buf, char *read_buffer);

int sms_read(struct _sms_buf *sms_buf, int id);

int sms_del(int all_del,char *sms_id);

int sendSMS(char* dev, char* phone_num, std::vector<string>& smsText);

#endif /*  __SMS_H__ */

