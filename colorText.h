#ifndef __COLOR_TEXT__
#define __COLOR_TEXT__

#include <iostream>

using namespace std;

#define CRESET   "\033[0m"
#define CBLACK   "\033[30m"      /* Black */
#define CRED     "\033[31m"      /* Red */
#define CGREEN   "\033[32m"      /* Green */
#define CYELLOW  "\033[33m"      /* Yellow */
#define CBLUE    "\033[34m"      /* Blue */
#define CMAGENTA "\033[35m"      /* Magenta */
#define CCYAN    "\033[36m"      /* Cyan */
#define CWHITE   "\033[37m"      /* White */
#define CBOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define CBOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define CBOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define CBOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define CBOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define CBOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define CBOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define CBOLDWHITE   "\033[1m\033[37m"      /* Bold White */ 
#define CCLEAR "\033[2J"  // clear screen escape code 

#endif  //  __COLOR_TEXT__