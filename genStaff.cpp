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

#include "genStaff.h"

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

string staff_dir        =   "PT/staff/";
string staff_name_file  =   "PT/staff_name.dat";
string staff_data_file  =   "PT/staff_face.dat";

#define DLIB_JPEG_SUPPORT

// ---------------------------------------------------------------------------------------------------------------
int main(int argc, char** argv) try
{
    cout << "Usage: " << argv[0] << " PT/staff" << endl;
    cout << CGREEN << "PT/staff" << CRESET << "\t\t: dir contain staff image" << endl;
    cout << CGREEN << "image's name is staff's name, ex: PhamTuan.jpg" << CRESET << endl;
    cout << "-----------------------------------------------------------------" << endl;

    // The first thing we are going to do is load all our models.  First, since we need to
    // find faces in the image we will need a face detector:
    frontal_face_detector detector = get_frontal_face_detector();
    // We will also use a face landmarking model to align faces to a standard pose:  (see face_landmark_detection_ex.cpp for an introduction)
    shape_predictor sp;
    deserialize("shape_predictor_68_face_landmarks.dat") >> sp;
    // And finally we load the DNN responsible for face recognition.
    anet_type net;
    deserialize("dlib_face_recognition_resnet_model_v1.dat") >> net;


    std::vector<matrix<float,0,1>> face_descriptors;
    std::vector<matrix<float,0,1>>  staff_faces_descriptors;
    std::vector<std::string> staff_id;

    std::vector<directory> dirs;
    std::vector<file> files;

    // list file in directory
    listInDir(staff_dir, dirs, files);

    for(unsigned long i = 0; i < files.size(); ++i)
    {
        matrix<rgb_pixel> img;
        std::string file_to_open = staff_dir + files[i].name();
        load_image(img, file_to_open);
        //cv::Mat img;
        //img = cv::imread(file_to_open, CV_LOAD_IMAGE_COLOR);   // Read the file

        std::vector<matrix<rgb_pixel>> faces;
        for (auto face : detector(img))
        {
            auto shape = sp(img, face);
            matrix<rgb_pixel> face_chip;
            extract_image_chip(img, get_face_chip_details(shape,150,0.25), face_chip);
            faces.push_back(move(face_chip));
        }

        if (faces.size() == 0)
        {
            cout << "No faces found in image: " << files[i].name() << endl;
        }
        else if(faces.size() == 1)
        {
            face_descriptors = net(faces);
            staff_faces_descriptors.push_back(face_descriptors[0]);

            std::string staff_name = files[i].name();

            for(size_t i = 0; i < staff_name.size(); ++i)
            {
                if(staff_name[i] == '.')
                {
                    staff_name.erase(staff_name.begin() + i, staff_name.end());
                    break;
                }
            }

            cout << "Staff ID: " << staff_id.size() << " - Name: " << staff_name << endl;
            staff_id.push_back(staff_name);
        }
        else
        {
            cout << "Wrong image file: " << files.size() << endl;
        }
    }

    if(staff_id.size() && staff_faces_descriptors.size())
    {
        serialize(staff_name_file) << staff_id;
        cout << "Saved Staff's name: " << staff_name_file << endl;

        serialize(staff_data_file) << staff_faces_descriptors;
        cout << "Saved Staff's data: " << staff_data_file << endl;
    }

    return 1;

}
catch (std::exception& e)
{
    cout << e.what() << endl;
}