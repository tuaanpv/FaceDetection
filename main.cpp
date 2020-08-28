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

//#define USE_MULTI_THREAD    true

// ---------------------------------------------------------------------------------------------------------------
int main(int argc, char** argv) try
{
    process_argrument(argc, argv);

    cameraInit();

    FaceProcess faceProcess;

    faceProcess.Init();

    std::thread Thread1(&FaceProcess::frameFaceDetect, FaceProcess());
    std::thread Thread2(&FaceProcess::faceRecognition, FaceProcess());
    std::thread Thread3(&FaceProcess::peopleProcess, FaceProcess());
    //std::thread Thread4(&externalFuntion);

    Thread1.join();
    Thread2.join();
    Thread3.join();
    //Thread4.join();
}
catch (std::exception& e)
{
    cout << e.what() << endl;
}


void process_argrument(int& argc, char**& argv)
{
    cout << "Usage: " << argv[0] << " 0 disp opencv 0.6 1 2" << endl;
    cout << CGREEN << "0..." << MAX_NUM_CAMERA << CRESET << "\t\t: id of camera" << endl;
    cout << CGREEN << "disp/nodisp" << CRESET << "\t: enable/disable display video" << endl;
    cout << CGREEN << "opencv/dlib" << CRESET << "\t: opencv/dlib for face detector" << endl;
    cout << CGREEN << "0.6" << CRESET << "\t\t: FACE_THRESHOLD" << endl;
    cout << CGREEN << "1" << CRESET << "\t\t: Down sample raito" << endl;
    cout << CGREEN << "2" << CRESET << "\t\t: Skip Frames" << endl;
    cout << "-----------------------------------------------------------------" << endl;

    if((argc < 2) || (argc > 7))
    {
        CameraID = 0;
        EnableDisplay = 1;
        using_opencv_for_detect = 1;
        face_Threshold = 0.6;
        img_downsample_raito = 1;
        img_skip_frames = 2;
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

        if(!strcmp(argv[3],"opencv"))
        {
            using_opencv_for_detect = 1;
        }
        else if(!strcmp(argv[3],"dlib"))
        {
            using_opencv_for_detect = 0;
        }
        else
        {
            using_opencv_for_detect = 1;
        }

        face_Threshold = stof(argv[4]);

        img_downsample_raito = stoi(argv[5]);
        img_skip_frames = stoi(argv[6]);

    }
}

void cameraInit()
{
    cap = CameraID;
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
    //cout << "Camera fps   : " << cap.get(CV_CAP_PROP_FPS) << endl;
    cout << "Camera width : " << cap.get(CV_CAP_PROP_FRAME_WIDTH) << endl;
    cout << "Camera height: " << cap.get(CV_CAP_PROP_FRAME_HEIGHT) << endl;
    cout << "-----------------------------------------------------------------" << endl;
}


void externalFuntion()
{
    system("./externalFuntion &");
}