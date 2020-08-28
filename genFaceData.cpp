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

#include "main.h"

// ---------------------------------------------------------------------------------------------------------------
int main(int argc, char** argv) try
{
    cout << "Usage: " << argv[0] << " cam 0" << endl;
    cout << "or   : " << argv[0] << " pic faces/" << endl;
    cout << CGREEN << "cam" << CRESET << "\t: using Camera" << endl;
    cout << CGREEN << "0..." << MAX_NUM_CAMERA << CRESET << "\t\t: id of camera" << endl;
    cout << CGREEN << "pic" << CRESET << "\t: using Picture folder" << endl;
    cout << CGREEN << "faces/" << CRESET << "\t: folder contain pictures" << endl;
    cout << "-----------------------------------------------------------------" << endl;
    process_argrument(argc, argv);

    if(using_Camera)
    {
        cv::VideoCapture cap(CameraID);
        while(!cap.isOpened())
        {
            cout << "Unable to connect to camera " <<  CameraID << endl;
            cap = CameraID;
            CameraID++;
            cout << "Retry another camera: " <<  CameraID << endl;

            if(CameraID > MAX_NUM_CAMERA)
            {
                cout << "Wait 5 seconds and try again" << endl;
                CameraID = 0;
                ssleep(5);
            }
        }
        cout << "Camera " << CameraID << " OK! \r\n" << endl;
        cout << "Camera fps   : " << cap.get(CV_CAP_PROP_FPS) << endl;
        cout << "Camera width : " << cap.get(CV_CAP_PROP_FRAME_WIDTH) << endl;
        cout << "Camera height: " << cap.get(CV_CAP_PROP_FRAME_HEIGHT) << endl;
        cout << "-----------------------------------------------------------------" << endl;
    }
    else
    {

    }

    FaceProcess faceDetect;

    while(1)
    {
        if(using_Camera)
        {
            cv::Mat temp;
            cap >> temp;
            cv_image<bgr_pixel> img(temp);
        }
        else
        {
            matrix<rgb_pixel> img;
            load_image(img, picture_folder);
        }

        std::vector<matrix<rgb_pixel>> faces;
        int num_people_in_frame = faceDetect.frameFaceDetect(img, faces);

        // -- Save frame image ----------------------------------------------------------------------------------------------
        if(faces.size())
        {
            cout << CYELLOW << "Number of people found in the image: " << num_people_in_frame << CRESET << endl;

            std::vector<matrix<float,0,1>> face_descriptors;
            faceDetect.faceRecognition(faces, face_descriptors, guest_faces_descriptors);
            // -- Save a frame --
            string im_name = main_path + temp_path + to_string(num_picture) + ".jpg";
            cout << "Image saved: " << im_name << endl;
            save_jpeg(img, im_name, 100);
            cout << "Photos count: " << num_picture << endl;
            num_picture++;
            ofstream listImage((list_file_im).c_str());
            listImage << num_picture << endl;
            listImage.close();;
        }
        // -- End Save frame image ----------------------------------------------------------------------------------------------

        if(total_guests != guest_faces_descriptors.size())
        {
            serialize(main_path + temp_path + guest_face_data) << guest_faces_descriptors;
            cout << "Saved guest face database" << endl;
        }

        old_num_person = faces.size();
        total_guests = guest_faces_descriptors.size();

        cout << CRED << "### Total of people appeared in this Camera: " << total_guests << " ###" << CRESET << endl;
        cout << "-------------------------------------------------------------------------------------------------\r\n" << endl;
    }
}
catch (std::exception& e)
{
    cout << e.what() << endl;
}


//------------------------------------------------------------------------------------------------------------------------
void read_csv(const std::string& filename, std::vector<cv::Mat>& images, std::vector<int>& labels, char separator = ';') 
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


void process_argrument(int& argc, char**& argv)
{
    if(argc == 3)
    {
        if(!strcmp(argv[2],"cam"))
        {
            using_Camera = 1;
            CameraID = stoi(argv[3]);
        }
        else if(!strcmp(argv[2],"pic"))
        {
            using_Camera = 0;
            picture_folder = argv[3];
        }
        else
        {            
            cout << "Wrong argruments" << endl;
            return 1;
        }

        EnableDisplay = 1;
        realtime_generation_face_descriptors=1;
    }
    else
    {
        cout << "Wrong argruments" << endl;
        return 1;
    }
}