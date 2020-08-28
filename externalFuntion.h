
#ifndef __EXTERNAL_FUNCTION_H__
#define __EXTERNAL_FUNCTION_H__

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <ctime>
#include <thread>
#include <mutex>
#include <iomanip>
#include <algorithm>
#include <sstream>

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include "colorText.h"

#include "sms.h"
//#include "fileProcess.h"


int iday, imonth, iyear, ihour, imin, isec;
string sday, smonth, syear, shour, smin, ssec;

void init();

time_t get_date_time();

static void read_file(const string& filename, vector<string>& labels, vector<string>& parameter);

#endif  // __EXTERNAL_FUNCTION_H__