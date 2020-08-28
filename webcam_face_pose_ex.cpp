// The contents of this file are in the public domain. See LICENSE_FOR_EXAMPLE_PROGRAMS.txt
/*

    This example program shows how to find frontal human faces in an image and
    estimate their pose.  The pose takes the form of 68 landmarks.  These are
    points on the face such as the corners of the mouth, along the eyebrows, on
    the eyes, and so forth.  
    

    This example is essentially just a version of the face_landmark_detection_ex.cpp
    example modified to use OpenCV's VideoCapture object to read from a camera instead 
    of files.


    Finally, note that the face detector is fastest when compiled with at least
    SSE2 instructions enabled.  So if you are using a PC with an Intel or AMD
    chip then you should enable at least SSE2 instructions.  If you are using
    cmake to compile this program you can enable them by using one of the
    following commands when you create the build project:
        cmake path_to_dlib_root/examples -DUSE_SSE2_INSTRUCTIONS=ON
        cmake path_to_dlib_root/examples -DUSE_SSE4_INSTRUCTIONS=ON
        cmake path_to_dlib_root/examples -DUSE_AVX_INSTRUCTIONS=ON
    This will set the appropriate compiler options for GCC, clang, Visual
    Studio, or the Intel compiler.  If you are using another compiler then you
    need to consult your compiler's manual to determine how to enable these
    instructions.  Note that AVX is the fastest but requires a CPU from at least
    2011.  SSE4 is the next fastest and is supported by most current machines.  
*/

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

// ----------------------------------------------------------------------------------------

const string  main_path = "PT/";
const string  temp_path = "temp/";
const string  list_im   = "list.pt";

unsigned int old_num_person;
unsigned int num_picture;

int main()
{
    //FILE *listImage;

    system(("mkdir "+main_path).c_str());
    system(("mkdir "+main_path+temp_path).c_str());
    old_num_person = 0;

    string list_file_im = main_path + temp_path + list_im;
    //ofstream listImage ((list_file_im).c_str(), std::ios::out | std::ios::trunc);
    ifstream listImage((list_file_im).c_str());
    listImage >> num_picture;

    //listImage.seekg(0, std::ios::end);  // move pointer to end of file 
    //listImage << "\n" << num_picture << ":\t*" << im_name << "#\n" << endl;

    listImage.close();

    try
    {
        cv::VideoCapture cap(0);
        if (!cap.isOpened())
        {
            cerr << "Unable to connect to camera" << endl;
            return 1;
        }

        cout << "OK" << endl;

        image_window win, win_faces;

        // Load face detection and pose estimation models.
        frontal_face_detector detector = get_frontal_face_detector();
        shape_predictor pose_model;
        deserialize("shape_predictor_68_face_landmarks.dat") >> pose_model;
        // // And finally we load the DNN responsible for face recognition.
        anet_type net;
        deserialize("dlib_face_recognition_resnet_model_v1.dat") >> net;


        // Grab and process frames until the main window is closed by the user.
        while(!win.is_closed())
        {
            // Grab a frame
            cv::Mat temp;
            cap >> temp;
            // Turn OpenCV's Mat into something dlib can deal with.  Note that this just
            // wraps the Mat object, it doesn't copy anything.  So cimg is only valid as
            // long as temp is valid.  Also don't do anything to temp that would cause it
            // to reallocate the memory which stores the image as that will make cimg
            // contain dangling pointers.  This basically means you shouldn't modify temp
            // while using cimg.
            cv_image<bgr_pixel> cimg(temp);

            // Detect faces 
            std::vector<rectangle> faces = detector(cimg);
            // Find the pose of each face.
            std::vector<full_object_detection> shapes;
            for (unsigned long i = 0; i < faces.size(); ++i)
                shapes.push_back(pose_model(cimg, faces[i]));

            // Display it all on the screen
            win.clear_overlay();
            win.set_image(cimg);
            win.add_overlay(render_face_detections(shapes));

            dlib::array<array2d<rgb_pixel> > face_chips;
            extract_image_chips(cimg, get_face_chip_details(shapes), face_chips);
            win_faces.set_image(tile_images(face_chips));

            
            if(old_num_person != faces.size())
            {
                if(faces.size())
                {
                    string im_name = main_path + temp_path + to_string(num_picture) + ".jpg";
                    //im_name << num_picture;
                    cout << "\r\nImage saved: " << im_name << endl;
                    save_jpeg(cimg, im_name, 100);
                    num_picture++;
                    cout << "Photos count: " << num_picture;

                    for(int count_face_im = 0; count_face_im < faces.size(); count_face_im++)
                    {
                        im_name = main_path + temp_path + to_string(num_picture) + "_" + to_string(count_face_im)+ ".jpg";
                        save_jpeg(face_chips[count_face_im], im_name, 100);
                    }


                    //string list_file_im = main_path + temp_path + list_im;
                    //ofstream listImage ((list_file_im).c_str(), std::ios::out | std::ios::trunc);
                    ofstream listImage((list_file_im).c_str());
                    listImage << num_picture << endl;

                    //listImage.seekg(0, std::ios::end);  // move pointer to end of file 
                    //listImage << "\n" << num_picture << ":\t*" << im_name << "#\n" << endl;

                    listImage.close();
                }
                cout << "\r\nPerson increase or decrease" << endl;
                old_num_person = faces.size();
            }

            cout << "\rPersons in frame: " << faces.size();
            fflush(stdout);
        }        
    }
    catch(serialization_error& e)
    {
        cout << "You need dlib's default face landmarking model file to run this example." << endl;
        cout << "You can get it from the following URL: " << endl;
        cout << "   http://dlib.net/files/shape_predictor_68_face_landmarks.dat.bz2" << endl;
        cout << endl << e.what() << endl;
    }
    catch(exception& e)
    {
        cout << e.what() << endl;
    }
}