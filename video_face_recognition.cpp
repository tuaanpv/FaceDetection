// The contents of this file are in the public domain. See LICENSE_FOR_EXAMPLE_PROGRAMS.txt
/*
    This is an example illustrating the use of the deep learning tools from the dlib C++
    Library.  In it, we will show how to do face recognition.  This example uses the
    pretrained dlib_face_recognition_resnet_model_v1 model which is freely available from
    the dlib web site.  This model has a 99.38% accuracy on the standard LFW face
    recognition benchmark, which is comparable to other state-of-the-art methods for face
    recognition as of February 2017. 
    
    In this example, we will use dlib to do face clustering.  Included in the examples
    folder is an image, bald_guys.jpg, which contains a bunch of photos of action movie
    stars Vin Diesel, The Rock, Jason Statham, and Bruce Willis.   We will use dlib to
    automatically find their faces in the image and then to automatically determine how
    many people there are (4 in this case) as well as which faces belong to each person.
    
    Finally, this example uses a network with the loss_metric loss.  Therefore, if you want
    to learn how to train your own models, or to get a general introduction to this loss
    layer, you should read the dnn_metric_learning_ex.cpp and
    dnn_metric_learning_on_images_ex.cpp examples.
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
#include <unistd.h>
#include <ctime>

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

#define FACE_THRESHOLD   0.6    //default 0.6

// // // ----------------------------------------------------------------------------------------

// std::vector<matrix<rgb_pixel>> jitter_image(
//     const matrix<rgb_pixel>& img
// );

// // ----------------------------------------------------------------------------------------

std::vector<matrix<float,0,1>>  guest_faces_descriptors;
std::vector<matrix<float,0,1>>  staff_faces_descriptors;
std::vector<int>                faces_appear_frequently_counter;


const string  main_path     = "PT/";
const string  temp_path     = "temp/";
const string  list_im       = "list_im.pt";
const string  faces_dir     = "faces/";
const string  list_fa       = "list_fa.pt";
const string  staff_dir     = "staff/";
const string  list_st       = "list_st.pt";

unsigned int old_num_person;
unsigned int num_picture;      
unsigned int num_faces;     
unsigned int num_staff;

#define ENABLE_DISPLAY TRUE
#define MAX_NUM_CAMERA  5

#define ssleep(x)   usleep(x*1000000)   // delay x seconds
#define msleep(x)   usleep(x*1000)      // delay x miliseconds


// ---------------------------------------------------------------------------------------------------------------
int main(int argc, char** argv) try
{
    int CameraID, EnableDisplay;

    cout << "Usage: " << argv[0] << " 0 disp" << endl;
    cout << "0: id of camera" << endl;
    cout << "disp/nodisp: enable/disable display video \r\n" << endl;

    if((argc < 2) || (argc > 3))
    {
        CameraID = 0;
        EnableDisplay = 1;
    }
    else
    {
        CameraID = stoi(argv[1]);

        if(!strcmp(argv[2],"disp"))
        {
            EnableDisplay = 1;
        }
        else if(!strcmp(argv[2],"nodisp"))
        {
            EnableDisplay = 0;
        }
        else
        {
            EnableDisplay = 1;
        }
    }
    //------------------------------------------------------------

    time_t now = time(0);


    cout << "Number of sec since January 1,1970:" << now << endl;
    tm *ltm = localtime(&now);
    cout << "Date: " << 1 + ltm->tm_mday << "/" << 1 + ltm->tm_mon << "/"  << 1900 + ltm->tm_year << endl;
    cout << "Time: "<< 1 + ltm->tm_hour << ":" << 1 + ltm->tm_min << ":" << 1 + ltm->tm_sec << endl;
    
    //FILE *listImage;
    system(("mkdir "+main_path).c_str());
    system(("mkdir "+main_path+temp_path).c_str());
    system(("mkdir "+main_path+faces_dir).c_str());
    system(("mkdir "+main_path+staff_dir).c_str());
    old_num_person = 0;

    //-----------------------------------------------------------------------------------------------
    string list_file_im = main_path + temp_path + list_im;
    ifstream listImage((list_file_im).c_str());
    listImage >> num_picture;
    listImage.close();
    //-----------------------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------------------
    string list_file_fa = main_path + faces_dir + list_fa;
    ifstream listFaces((list_file_fa).c_str());
    listFaces >> num_faces;
    listFaces.close();
    //-----------------------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------------------
    string list_file_st = main_path + staff_dir + list_st;
    ifstream listStaff((list_file_st).c_str());
    listStaff >> num_staff;
    listStaff.close();
    //-----------------------------------------------------------------------------------------------

    cv::VideoCapture cap(CameraID);
    if (!cap.isOpened())
    {
        cout << "Unable to connect to camera " <<  CameraID << endl;
        int i = 0;
        do
        {
            cout << "Retry another camera " <<  i << endl;
            cap = i;
            i++;
            if(i > MAX_NUM_CAMERA)
            {
                cerr << "Unable to connect to camera " << endl;
                //return 1;
                cout << "Wait 5 seconds and try again" << endl;
                i = 0;
                ssleep(5);
            }
        }while(!cap.isOpened());
    }

    cout << "Camera OK!" << endl;
    cout << "-------------------------------------------------------------------------------------------------\r\n" << endl;

    // The first thing we are going to do is load all our models.  First, since we need to
    // find faces in the image we will need a face detector:
    frontal_face_detector detector = get_frontal_face_detector();
    // We will also use a face landmarking model to align faces to a standard pose:  (see face_landmark_detection_ex.cpp for an introduction)
    shape_predictor sp;
    deserialize("shape_predictor_68_face_landmarks.dat") >> sp;
    // And finally we load the DNN responsible for face recognition.
    anet_type net;
    deserialize("dlib_face_recognition_resnet_model_v1.dat") >> net;

    image_window win, win_faces; 

    while(!win.is_closed())
    {
        cv::Mat temp;
        cap >> temp;
        cv_image<bgr_pixel> img(temp);

        #ifdef  ENABLE_DISPLAY
        if(EnableDisplay)
        {
            win.clear_overlay();
            win.set_image(img);
        }
        #endif

        // Run the face detector on the image of our action heroes, and for each face extract a
        // copy that has been normalized to 150x150 pixels in size and appropriately rotated
        // and centered.
        std::vector<matrix<rgb_pixel>> faces;
        for (auto face : detector(img))
        {
            auto shape = sp(img, face);
            matrix<rgb_pixel> face_chip;
            extract_image_chip(img, get_face_chip_details(shape,150,0.25), face_chip);
            faces.push_back(move(face_chip));

            #ifdef  ENABLE_DISPLAY
            // Also put some boxes on the faces so we can see that the detector is finding
            // them.
            if(EnableDisplay)
            {
                win_faces.set_image(tile_images(faces));
                
                win.add_overlay(face);
                win.add_overlay(render_face_detections(shape));
            }
            #endif
        }

        // This call asks the DNN to convert each face image in faces into a 128D vector.
        // In this 128D vector space, images from the same person will be close to each other
        // but vectors from different people will be far apart.  So we can use these vectors to
        // identify if a pair of images are from the same person or from different people. 

        if(faces.size() != old_num_person)
        { 
            std::vector<matrix<float,0,1>> face_descriptors = net(faces);
            cout << "Number of people found in the image: " << faces.size() << endl;

            // -- Face Recogition ----------------------------------------------------------------------------------------------
            if(guest_faces_descriptors.size())
            {
                std::vector<matrix<rgb_pixel>> temp_faces = faces;
                std::vector<matrix<float,0,1>> temp_staff;
                std::vector<matrix<rgb_pixel>> staff_faces;
                for (size_t i = 0; i < guest_faces_descriptors.size(); ++i)
                {
                    for (size_t j = 0; j < face_descriptors.size(); ++j)
                    {
                        // Faces are connected in the graph if they are close enough.  Here we check if
                        // the distance between two face descriptors is less than 0.6, which is the
                        // decision threshold the network was trained to use.  Although you can
                        // certainly use any other threshold you find useful.
                        if (length(face_descriptors[j] - guest_faces_descriptors[i]) < FACE_THRESHOLD)
                        {
                            cout << "Old person" << endl;

                            temp_staff.push_back(face_descriptors[j]);
                            staff_faces.push_back(temp_faces[j]);

                            face_descriptors.erase(face_descriptors.begin() + j);
                            temp_faces.erase(temp_faces.begin() + j);
                        }
                    }
                }

                for (size_t j = 0; j < face_descriptors.size(); ++j)
                {
                    cout << "New person" << endl;
                    guest_faces_descriptors.push_back(face_descriptors[j]);

                    // -- Save crop face --
                    string im_name = main_path + faces_dir + to_string(num_faces) + ".jpg";
                    cout << "Face saved: " << im_name << endl;
                    save_jpeg(temp_faces[j], im_name, 100);
                    cout << "Faces count: " << num_faces << endl;
                    num_faces++;
                    ofstream listFaces((list_file_fa).c_str());
                    listFaces << num_faces << endl;
                    listFaces.close();
                }

                if(staff_faces_descriptors.size())
                {
                    for(size_t i = 0; i < staff_faces_descriptors.size(); ++i)
                    {
                        for(size_t j = 0; j < temp_staff.size(); ++j)
                        {
                            if(length(temp_staff[j] - staff_faces_descriptors[i]) < FACE_THRESHOLD)
                            {
                                cout << "Person appear > 3 times" << endl;
                                faces_appear_frequently_counter[i] += 1;
                                temp_staff.erase(temp_staff.begin() + j);    


                                // -- Save crop face to staff/ if appear > 3 times --
                                string im_name = main_path + staff_dir + to_string(num_staff) + ".jpg";
                                cout << "Staff face saved: " << im_name << endl;
                                save_jpeg(staff_faces[j], im_name, 100);
                                cout << "Staff faces count: " << num_staff << endl;
                                num_staff++;
                                ofstream listStaff((list_file_st).c_str());
                                listStaff << num_staff << endl;
                                listStaff.close();                    
                            }
                        }
                    }

                    for(size_t i = 0; i < temp_staff.size(); ++i)
                    {
                        staff_faces_descriptors.push_back(temp_staff[i]);
                        faces_appear_frequently_counter.push_back(2);
                    }
                }
                else
                {
                    staff_faces_descriptors = temp_staff;

                    for(size_t i = 0; i < staff_faces_descriptors.size(); ++i)
                    {
                        faces_appear_frequently_counter.push_back(2);
                    }
                }
            }
            else
            {
                guest_faces_descriptors = face_descriptors;
                cout << "New person record database" << endl;

                // -- Save crop face --
                for(int i = 0; i < faces.size(); i++)
                {
                    string im_name = main_path + faces_dir + to_string(num_faces) + ".jpg";
                    cout << "Face saved: " << im_name << endl;
                    save_jpeg(faces[i], im_name, 100);
                    cout << "Faces count: " << num_faces << endl;
                    num_faces++;
                }
                ofstream listFaces((list_file_fa).c_str());
                listFaces << num_faces << endl;
                listFaces.close();
            }
            // -- End Face Recogition ----------------------------------------------------------------------------------------------

            // -- Save image ----------------------------------------------------------------------------------------------
            if(faces.size())
            {
                // -- Save a frame --
                string im_name = main_path + temp_path + to_string(num_picture) + ".jpg";
                cout << "Image saved: " << im_name << endl;
                save_jpeg(img, im_name, 100);
                cout << "Photos count: " << num_picture << endl;
                num_picture++;
                ofstream listImage((list_file_im).c_str());
                listImage << num_picture << endl;
                listImage.close();

                // // -- Save crop face --
                // for(int i = 0; i < faces.size(); i++)
                // {
                //     string im_name = main_path + faces_dir + to_string(num_faces) + ".jpg";
                //     cout << "Face saved: " << im_name << endl;
                //     save_jpeg(faces[i], im_name, 100);
                //     cout << "Faces count: " << num_faces << endl;
                //     num_faces++;
                // }
                // ofstream listFaces((list_file_fa).c_str());
                // listFaces << num_faces << endl;
                // listFaces.close();
            }
            // -- End Save image ----------------------------------------------------------------------------------------------

            old_num_person = faces.size();

            cout << "### Total of people appeared in this Camera: " << guest_faces_descriptors.size() << " ###" << endl;
            cout << "-------------------------------------------------------------------------------------------------\r\n" << endl;
        }


        // //matrix<float,0,1> face_descriptor = mean(mat(net(jitter_image(faces[0]))));
        // //cout << "jittered face descriptor for one face: " << trans(face_descriptor) << endl;
    }
}
catch (std::exception& e)
{
    cout << e.what() << endl;
}

// // ----------------------------------------------------------------------------------------

// std::vector<matrix<rgb_pixel>> jitter_image(
//     const matrix<rgb_pixel>& img
// )
// {
//     // All this function does is make 100 copies of img, all slightly jittered by being
//     // zoomed, rotated, and translated a little bit differently.
//     thread_local random_cropper cropper;
//     cropper.set_chip_dims(150,150);
//     cropper.set_randomly_flip(true);
//     cropper.set_max_object_height(0.99999);
//     cropper.set_background_crops_fraction(0);
//     cropper.set_min_object_height(0.97);
//     cropper.set_translate_amount(0.02);
//     cropper.set_max_rotation_degrees(3);

//     std::vector<mmod_rect> raw_boxes(1), ignored_crop_boxes;
//     raw_boxes[0] = shrink_rect(get_rect(img),3);
//     std::vector<matrix<rgb_pixel>> crops; 

//     matrix<rgb_pixel> temp; 
//     for (int i = 0; i < 100; ++i)
//     {
//         cropper(img, raw_boxes, temp, ignored_crop_boxes);
//         crops.push_back(move(temp));
//     }
//     return crops;
// }

// // ----------------------------------------------------------------------------------------



static void read_csv(const std::string& filename, std::vector<cv::Mat>& images, std::vector<int>& labels, char separator = ';') 
{
    std::ifstream file(filename.c_str(), std::ifstream::in);
    if (!file) 
    {
        std::string error_message = "No valid input file was given, please check the given filename.";
        CV_Error(CV_StsBadArg, error_message);
    }
    std::string line, path, classlabel;
    while (getline(file, line))
    {
        std::stringstream liness(line);
        getline(liness, path, separator);
        getline(liness, classlabel);
        if(!path.empty() && !classlabel.empty())
        {
            images.push_back(cv::imread(path, 0));
            labels.push_back(atoi(classlabel.c_str()));
        }
    }
}