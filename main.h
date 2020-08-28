#ifndef __MAIN_H__
#define __MAIN_H__


#include <dlib/gui_widgets.h>
#include <dlib/clustering.h>
#include <dlib/string.h>
#include <dlib/dnn.h>
#include <dlib/image_io.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/opencv.h>
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
#include "colorText.h"
#include "faceProcess.h"
#include "fileProcess.h"

using namespace dlib;
using namespace std;



//#define FACE_THRESHOLD   0.6    //default 0.6

#define ENABLE_DISPLAY TRUE
#define MAX_NUM_CAMERA  5

#define ssleep(x)   usleep(x*1000000)   // delay x seconds
#define msleep(x)   usleep(x*1000)      // delay x miliseconds

extern std::vector<matrix<float,0,1>>  guest_faces_descriptors;
extern std::vector<matrix<float,0,1>>  staff_faces_descriptors;
extern std::vector<matrix<float,0,1>>  daily_faces_descriptors;

extern const string  main_path         = "PT/";
extern const string  temp_path         = "temp/";
extern const string  cout_image        = "count_image.pt";
extern const string  faces_dir         = "faces/";
extern const string  cout_face         = "count_face.pt";
extern const string  staff_dir         = "staff/";
extern const string  staff_face_data   = "staff_face.dat";

extern const string  staff_file        = "staff";
extern const string  guests            = "guests_";
extern const string  counter           = "count_";
extern const string  config_file       = "pt.conf";

extern string daily_guests_counter_file;
extern int iday, imonth, iyear, ihour, imin, isec;
extern string sday, smonth, syear, shour, smin, ssec;

extern string daily_guests_face_data;
extern string old_daily_guests_face_data;

extern string list_file_image;
extern string list_file_face;
extern string list_file_staff;

extern unsigned int old_num_person;
extern unsigned int num_picture;
extern unsigned int num_faces;
extern unsigned int num_staff;

extern unsigned int total_guests;
extern unsigned int old_total_guests;

extern int EnableDisplay;
extern int make_face_database;

int using_Camera;
string picture_folder;
int CameraID;

extern std::mutex mutexPT;

extern cv::VideoCapture cap;

void process_argrument(int& argc, char**& argv);

void cameraInit();

void externalFuntion();

extern float face_Threshold; 
extern int using_opencv_for_detect;

extern int img_downsample_raito;
extern int img_skip_frames;


#endif  //  __MAIN_H__
