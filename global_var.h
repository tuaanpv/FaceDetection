#ifndef __MAIN_H__
#define __MAIN_H__

#include <dlib/gui_widgets.h>
#include <dlib/clustering.h>
#include <dlib/string.h>
#include <dlib/dnn.h>
#include <dlib/image_io.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/opencv.h>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>
#include <dlib/matrix.h>
#include <dlib/sparse_vector.h>
#include <dlib/geometry.h>
#include <dlib/queue.h>

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <ctime>
#include <thread>
#include <mutex>

using namespace dlib;
using namespace std;

// ------------------------------------------

#define ENABLE_DISPLAY TRUE
#define MAX_NUM_CAMERA  5

#define ssleep(x)   usleep(x*1000000)   // delay x seconds
#define msleep(x)   usleep(x*1000)      // delay x miliseconds

// ------------------------------------------

extern image_window win, win_faces;

extern int EnableDisplay;

extern std::vector<matrix<float,0,1>>  guest_faces_descriptors;
extern std::vector<matrix<float,0,1>>  staff_faces_descriptors;
extern std::vector<matrix<float,0,1>>  daily_faces_descriptors;


const string  main_path         = "PT/";
const string  temp_path         = "temp/";
const string  cout_image        = "count_image.pt";
const string  faces_dir         = "faces/";
const string  cout_face         = "count_face.pt";
const string  staff_dir         = "staff/";
const string  staff_face_data   = "staff_face.dat";

const string  staff_name_file   = "staff_name.dat";
const string  guests            = "guests_";
const string  counter           = "count_";
const string  stringStaffArrival= "Staff_Arrival_";
const string  config_file       = "pt.conf";
const string  config			= "config.conf";

extern string daily_guests_counter_file;
extern string old_daily_guests_counter_file;
extern int iday, imonth, iyear, ihour, imin, isec;
extern string sday, smonth, syear, shour, smin, ssec;

extern string staff_data_file;
extern string staff_id_file;
extern string daily_guests_face_data;
extern string old_daily_guests_face_data;

extern string list_file_image;
extern string list_file_face;
extern string list_staff_file;

extern unsigned int old_num_person;
extern unsigned int num_picture;
extern unsigned int num_faces;
extern unsigned int staffCount;
extern std::vector<std::string> staff_id;

extern unsigned int total_guests;
extern unsigned int old_total_guests;

extern std::mutex mutexPT;

extern cv::VideoCapture cap;

extern std::vector<matrix<rgb_pixel>> all_faces;

extern std::mutex  PTMutex;

extern float   face_Threshold;                    
extern int using_opencv_for_detect;

extern int img_downsample_raito;
extern int img_skip_frames;
extern int opencv_gain_faces;

extern std::vector<std::string> Staff_Arrival_Count;
extern std::vector<std::string> Time_Staff_Arrval;

extern int CameraID;
extern int camera_frame_width;
extern int camera_frame_height;

#define num_argc 8
void process_argrument(int& argc, char**& argv);
void LoadConfig();

void cameraInit();

void externalFuntion();

#endif  //  __MAIN_H__
