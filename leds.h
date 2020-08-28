#ifndef __LEDS_H__
#define __LEDS_H__

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

#define ON 1
#define OFF 0

#define LED_GREEN   0
#define LED_RED     1

static int write_led(int led, int data);

#endif /*  __LEDS_H__ */

