#ifndef __FACE_PROCESS__
#define __FACE_PROCESS__


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
//#include "utils.h"

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
#include "fileProcess.h"
#include "leds.h"

using namespace dlib;
using namespace std;

// ----------------------------------------------------------------------------------------

// The next bit of code defines a ResNet network.  It's basically copied
// and pasted from the dnn_imagenet_ex.cpp example, except we replaced the loss
// layer with loss_metric and made the network somewhat smaller.  Go read the introductory
// dlib DNN examples to learn what all this stuff means.
//
// Also, the dnn_metric_learning_on_images_ex.cpp example shows how to train this network.
// The dlib_face_recognition_resnet_model_v1 model used by this example was trained using
// essentially the code shown in dnn_metric_learning_on_images_ex.cpp except the
// mini-batches were made larger (35x15 instead of 5x5), the iterations without progress
// was set to 10000, the jittering you can see below in jitter_image() was used during
// training, and the training dataset consisted of about 3 million images instead of 55.
template <template <int,template<typename>class,int,typename> class block, int N, template<typename>class BN, typename SUBNET>
using residual = add_prev1<block<N,BN,1,tag1<SUBNET>>>;

template <template <int,template<typename>class,int,typename> class block, int N, template<typename>class BN, typename SUBNET>
using residual_down = add_prev2<avg_pool<2,2,2,2,skip1<tag2<block<N,BN,2,tag1<SUBNET>>>>>>;

template <int N, template <typename> class BN, int stride, typename SUBNET> 
using block  = BN<con<N,3,3,1,1,relu<BN<con<N,3,3,stride,stride,SUBNET>>>>>;

template <int N, typename SUBNET> using ares      = relu<residual<block,N,affine,SUBNET>>;
template <int N, typename SUBNET> using ares_down = relu<residual_down<block,N,affine,SUBNET>>;

template <typename SUBNET> using alevel0 = ares_down<256,SUBNET>;
template <typename SUBNET> using alevel1 = ares<256,ares<256,ares_down<256,SUBNET>>>;
template <typename SUBNET> using alevel2 = ares<128,ares<128,ares_down<128,SUBNET>>>;
template <typename SUBNET> using alevel3 = ares<64,ares<64,ares<64,ares_down<64,SUBNET>>>>;
template <typename SUBNET> using alevel4 = ares<32,ares<32,ares<32,SUBNET>>>;

using anet_type = loss_metric<fc_no_bias<128,avg_pool_everything<
                            alevel0<
                            alevel1<
                            alevel2<
                            alevel3<
                            alevel4<
                            max_pool<3,3,2,2,relu<affine<con<32,7,7,2,2,
                            input_rgb_image_sized<150>
                            >>>>>>>>>>>>;


#define FACE_THRESHOLD   0.7 //default 0.6        

#define ENABLE_DISPLAY TRUE // display video and faces crop
#define MAX_NUM_CAMERA  5   // max cameras can connect to computer

#define ssleep(x)   usleep(x*1000000)   // delay x seconds
#define msleep(x)   usleep(x*1000)      // delay x miliseconds

//  Max number of picture can save, if over, it's overwrite back to first picture
#define MAX_NUM_FACE_IMAGAE 7777    // Max image of face to save
#define MAX_NUM_FRAME_IMAGAE 7777   // Max image of frame to save

//------------------------------------------------------------------------------------------------------------------------------------

class FaceProcess
{
    public:
        FaceProcess();
        void Init();
        void frameFaceDetect();
        void faceRecognition();
        void peopleProcess();

    private:
        unsigned long faceCompare(matrix<float,0,1>& faces_video_descriptors, 
                        std::vector<matrix<float,0,1>>& faces_data_descriptors);
        void DeleteFacesData(std::vector<matrix<rgb_pixel>>& faces, std::vector<matrix<float,0,1>>& faces_descriptors, 
                            std::vector<unsigned long>& delete_face);
        std::vector<matrix<rgb_pixel>> jitter_image(const matrix<rgb_pixel>& img);
};


time_t get_date_time();

static cv::Rect RectangleDlibToCv(dlib::rectangle r)
{
    return cv::Rect(cv::Point2i(r.left(), r.top()), cv::Point2i(r.right() + 1, r.bottom() + 1));
}

static dlib::rectangle RectangleCvToDlib(cv::Rect r)
{
    return dlib::rectangle((long)r.tl().x, (long)r.tl().y, (long)r.br().x - 1, (long)r.br().y - 1);
}


static std::vector<rectangle> VectorRectangleCvToDlib(std::vector<cv::Rect> r)
{
    std::vector<rectangle> result;

    for(size_t i = 0; i < r.size(); ++i)
    {
        result.push_back(dlib::rectangle((long)r[i].tl().x, (long)r[i].tl().y, (long)r[i].br().x - 1, (long)r[i].br().y - 1));
    }

    return result;
}

static std::vector<cv::Rect> VectorRectangleDlibToCv(std::vector<rectangle> r)
{
    std::vector<cv::Rect> result;

    for(size_t i = 0; i < r.size(); ++i)
    {
        result.push_back(cv::Rect(cv::Point2i(r[i].left(), r[i].top()), cv::Point2i(r[i].right() + 1, r[i].bottom() + 1)));
    }

    return result;
}


// #define USING_OPENCV_DETECT


// // -- for slowdown fps
// #define FACE_DOWNSAMPLE_RATIO 1
// #define SKIP_FRAMES 2

#endif  //  __FACE_PROCESS__