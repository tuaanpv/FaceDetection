#ifndef __FILE_PROCESS__
#define __FILE_PROCESS__

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
#include <dlib/misc_api.h>
#include <iomanip>
#include <dlib/dir_nav.h>
#include <algorithm>

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include "colorText.h"

using namespace dlib;
using namespace std;
extern string list_file_staff;

// class FileProcess
// {
//     public:
//         FileProcess();
//         ~FileProcess();
//         //std::vector<std::vector<string>> listDir (const string& dir);
//         void listDir(string& loc, std::vector<directory>& dirs, std::vector<file>& files);
// };

int listInDir(const string& loc, std::vector<directory>& dirs, std::vector<file>& files);

static void read_file(const std::string& filename, std::vector<string>& labels, std::vector<string>& parameter);

#endif  //  __FILE_PROCESS__
