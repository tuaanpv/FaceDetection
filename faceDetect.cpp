
#include "faceDetect.h"
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

int frameFaceDetect(cv_image<bgr_pixel>& img, std::vector<matrix<rgb_pixel>>& faces, std::vector<matrix<float,0,1>>& face_descriptors)
{
    //cv_image<bgr_pixel> img(frame);

    for (auto face : detector(img))
    {
        auto shape = sp(img, face);
        matrix<rgb_pixel> face_chip;
        extract_image_chip(img, get_face_chip_details(shape,150,0.25), face_chip);
        faces.push_back(move(face_chip));


        #ifdef  ENABLE_DISPLAY
        if(EnableDisplay)
        {
            win.clear_overlay();
            win.set_image(img);

            win_faces.set_image(tile_images(faces));
            
            win.add_overlay(face);
            win.add_overlay(render_face_detections(shape));
        }
        #endif
    }

    face_descriptors = net(faces);

    return 0;
}


int initFrameFaceDetect()
{
	detector = get_frontal_face_detector();
    // We will also use a face landmarking model to align faces to a standard pose:  (see face_landmark_detection_ex.cpp for an introduction)
    //shape_predictor sp;
    deserialize("shape_predictor_68_face_landmarks.dat") >> sp;
    // And finally we load the DNN responsible for face recognition.
    //anet_type net;
    deserialize("dlib_face_recognition_resnet_model_v1.dat") >> net;

    return 0;
}

int saveImageFrame(cv_image<bgr_pixel>& img, string list_file_im, string im_name)
{
	// int num_picture;
	
 //    ifstream listImage((list_file_im).c_str());
 //    listImage >> num_picture;
 //    listImage.close();
 //    num_picture++;

 //    cout << "Image saved: " << im_name << endl;
 //    save_jpeg(img, im_name, 100);
 //    cout << "Photos count: " << num_picture << endl;
 //    ofstream listImage((list_file_im).c_str());
 //    listImage << num_picture << endl;
 //    listImage.close();	
}