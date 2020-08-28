
#include "faceProcess.h"

// The first thing we are going to do is load all our models.  First, since we need to
// find faces in the image we will need a face detector:
frontal_face_detector detector; // = get_frontal_face_detector();
// We will also use a face landmarking model to align faces to a standard pose:  (see face_landmark_detection_ex.cpp for an introduction)
shape_predictor sp;
//deserialize("shape_predictor_68_face_landmarks.dat") >> sp;
// And finally we load the DNN responsible for face recognition.
anet_type net;
//deserialize("dlib_face_recognition_resnet_model_v1.dat") >> net;


image_window win, win_faces;

int EnableDisplay;
int make_face_database;

std::vector<matrix<float,0,1>>  guest_faces_descriptors;
std::vector<matrix<float,0,1>>  staff_faces_descriptors;
std::vector<matrix<float,0,1>>  daily_faces_descriptors;

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

string daily_guests_counter_file;
string old_daily_guests_counter_file;
int iday, imonth, iyear, ihour, imin, isec;
string sday, smonth, syear, shour, smin, ssec;

string staff_data_file;
string staff_id_file;
string daily_guests_face_data;
string old_daily_guests_face_data;

string list_file_image;
string list_file_face;
string list_staff_file;

unsigned int old_num_person;
unsigned int num_picture;
unsigned int num_faces;
unsigned int staffCount;
std::vector<std::string> staff_id;

unsigned int total_guests;
unsigned int old_total_guests;

std::mutex mutexPT;

cv::VideoCapture cap;

std::vector<matrix<rgb_pixel>> all_faces;

std::mutex  PTMutex;


float   face_Threshold;                    
int using_opencv_for_detect;


// #define FACE_DOWNSAMPLE_RATIO 1
// #define SKIP_FRAMES 2

int img_downsample_raito;
int img_skip_frames;

std::vector<std::string> Staff_Arrival_Count;
std::vector<std::string> Time_Staff_Arrval;

//-------------------------------------------------------------------------------------------------------------------------------------------------

FaceProcess::FaceProcess(void){}

void FaceProcess::Init()
{
    int file_loaded;
    // --------------- Init face detect and recognition ----------------
    detector = get_frontal_face_detector();
    // We will also use a face landmarking model to align faces to a standard pose:  (see face_landmark_detection_ex.cpp for an introduction)
    deserialize("shape_predictor_68_face_landmarks.dat") >> sp;
    // And finally we load the DNN responsible for face recognition.
    deserialize("dlib_face_recognition_resnet_model_v1.dat") >> net;

    //--------------- Init file process --------------

    time_t now = get_date_time();

    cout << "Number of sec since January 1,1970:" << now << endl;
    cout << "Date: " << iday << "/" << imonth << "/"  << iyear << endl;
    cout << "Time: "<< ihour << ":" << imin << ":" << isec << endl;

    //FILE *listImage;
    // system(("mkdir "+main_path).c_str());
    // system(("mkdir "+main_path+temp_path).c_str());
    // system(("mkdir "+main_path+faces_dir).c_str());
    // system(("mkdir "+main_path+staff_dir).c_str());
    mkdir((main_path).c_str(), 0777);
    mkdir((main_path+temp_path).c_str(), 0777);
    mkdir((main_path+faces_dir).c_str(), 0777);
    mkdir((main_path+staff_dir).c_str(), 0777);
    old_num_person = 0;

    // Read temp counter -----------------------------------------------------------------------------------------------
    list_file_image = main_path + temp_path + cout_image;
    ifstream listImage((list_file_image).c_str());
    listImage >> num_picture;
    listImage.close(); cout << "=> Loaded pictures counter: " << list_file_image << endl;
    //-----------------------------------------------------------------------------------------------
    // Read faces couter -----------------------------------------------------------------------------------------------
    list_file_face = main_path + faces_dir + cout_face;
    ifstream listFaces((list_file_face).c_str());
    listFaces >> num_faces;
    listFaces.close();
    cout << "=> Loaded faces counter: " << list_file_face << endl;
    //-----------------------------------------------------------------------------------------------
    // Read guests counter -----------------------------------------------------------------------------------------------
    daily_guests_counter_file = main_path + guests + syear + smonth + sday + ".pt";
    old_daily_guests_counter_file = daily_guests_counter_file;
    ifstream dailyGuest((daily_guests_counter_file).c_str());
    dailyGuest >> total_guests;
    old_total_guests = total_guests;
    dailyGuest.close();
    cout << "=> Loaded guest counter: " << daily_guests_counter_file << endl;
    //-----------------------------------------------------------------------------------------------
    // Read guests faces data -----------------------------------------------------------------------------------------------
    string temp_faces_data_file = guests + syear + smonth + sday + ".dat";
    daily_guests_face_data = main_path + temp_faces_data_file;
    old_daily_guests_face_data = daily_guests_face_data;

    std::vector<file> files;
    std::vector<directory> dirs;
    listInDir(main_path, dirs, files);

    cout << "finding database file: " << temp_faces_data_file << endl;
    file_loaded = 0;
    //cout << "list file in dir: " << endl;
    for(unsigned long i = 0; i < files.size(); ++i)
    {
        if((temp_faces_data_file) == files[i].name())
        {
            deserialize(daily_guests_face_data) >> guest_faces_descriptors;
            cout << "=> Loaded face database: " << files[i].name() << endl;
            file_loaded = 1;
            break;
        }
    }
    if(!file_loaded)
    {
        cout << "can't load database file: " << temp_faces_data_file << endl;
    }
    //-----------------------------------------------------------------------------------------------
    // Generate staff database -----------------------------------------------------------------------------------------------
    cout << CRED << "Generate staff database ..." << CRESET << endl;
    system("./genStaff");

    // Read staff faces data -----------------------------------------------------------------------------------------------
    staff_data_file = main_path + staff_face_data;
    deserialize(staff_data_file) >> staff_faces_descriptors;
    cout << "=> Loaded face database: " << staff_data_file << endl;

    staff_id_file = main_path + staff_name_file;
    deserialize(staff_id_file) >> staff_id;
    cout << "=> Loaded Staff ID file: " << staff_id_file << endl;
    //-----------------------------------------------------------------------------------------------
    cout << "\n-------------------------------------------------------------------------------------------------\r\n" << endl;
}


void FaceProcess::frameFaceDetect()
{
    int old_face_size;
    int count = 0;
    double t = (double)cv::getTickCount();
    double fps = 30.0; // Just a place holder. Actual value calculated after 100 frames.

    cv::CascadeClassifier face_cascade;
    cv::CascadeClassifier eyes_cascade;
    cv::CascadeClassifier nose_cascade;
    cv::CascadeClassifier mouth_cascade;
    
    string face_cascade_name = "haarcascades/haarcascade_frontalface_alt.xml";
    string eyes_cascade_name = "haarcascades/haarcascade_eye_tree_eyeglasses.xml";
    string nose_cascade_name = "haarcascades/haarcascade_mcs_nose.xml";
    string mouth_cascade_name = "haarcascades/haarcascade_mcs_mouth.xml";

    face_cascade.load(face_cascade_name);
    eyes_cascade.load(eyes_cascade_name);
    nose_cascade.load(nose_cascade_name);
    mouth_cascade.load(mouth_cascade_name);

    for(;;)
    {
        if ( count == 0 )
        {
            t = cv::getTickCount();
        }

        cv::Mat temp;
        cap >> temp;
        //cv_image<bgr_pixel> img = temp;

        cv::Mat im_small;
        cv::resize(temp, im_small, cv::Size(), 1.0/img_downsample_raito, 1.0/img_downsample_raito);
        cv_image<bgr_pixel> cimg_small(im_small);

        std::vector<matrix<rgb_pixel>> faces;
        std::vector<full_object_detection> shapes;
        //cout << "step 1" << endl;

        std::vector<cv::Rect> dtface;
        std::vector<rectangle> face;

        // Detect faces 
        if(count % img_skip_frames == 0)
        {
            //cout << "count: " << count << " - count/SKIP_FRAMES: "<< count % img_skip_frames << endl;
            if(using_opencv_for_detect)
            {
                std::vector<cv::Rect> eyes;
                std::vector<cv::Rect> nose;
                std::vector<cv::Rect> mouth;

                // cvtColor(im_small, frame_gray, CV_BGR2GRAY);
                // equalizeHist(frame_gray, frame_gray);

                //-- Detect faces
                face_cascade.detectMultiScale( im_small, dtface, 1.1, 2, 0| CV_HAAR_SCALE_IMAGE, cv::Size(100, 100) );

                for( size_t i = 0; i < faces.size(); i++ )
                {
                    int isHuman;

                    eyes_cascade.detectMultiScale( dtface, eyes, 1.1, 2, 0 |CV_HAAR_SCALE_IMAGE, cv::Size(1, 1) );
                    if(eyes.size())
                    {
                        isHuman++;
                    }

                    nose_cascade.detectMultiScale( dtface, nose, 1.1, 2, 0 |CV_HAAR_SCALE_IMAGE, cv::Size(30, 30) );
                    if(nose.size())
                    {
                        isHuman++;
                    }

                    mouth_cascade.detectMultiScale( dtface, mouth, 1.1, 2, 0 |CV_HAAR_SCALE_IMAGE, cv::Size(30, 30) );
                    if(mouth.size())
                    {
                        isHuman++;
                    }

                    if(isHuman < 3)
                    {
                        dtface.erase(dtface.begin() + i);
                    }
                }

                face = VectorRectangleCvToDlib(dtface);
                //---------------------------------------------------------
            }
            else
            {
                // make new thread
                // auto future = std::async(detector, cimg_small);
                // face = future.get();
                
                // or
                face = detector(cimg_small);
            }

            for(size_t i =0; i < face.size(); ++i)
            {
                auto shape = sp(cimg_small, face[i]);
                matrix<rgb_pixel> face_chip;
                chip_details chip = get_face_chip_details(shape,150,0.25);
                extract_image_chip(cimg_small, chip, face_chip);
                faces.push_back(move(face_chip));
                shapes.push_back(move(shape));

            }

            #ifdef  ENABLE_DISPLAY
            if(EnableDisplay)
            {
                win.clear_overlay();
                win.set_image(cimg_small);
                win.add_overlay(render_face_detections(shapes));
                win_faces.set_image(tile_images(faces));
            }
            #endif

            if(faces.size() && (faces.size() != old_face_size))
            {
                // -- Save frame image ----------------------------------------------------------------------------------------------
                for(size_t i = 0; i <faces.size(); ++i)
                {
                    // -- Save a frame --
                    string im_name = main_path + temp_path + to_string(num_picture) + ".jpg";
                    save_jpeg(cimg_small, im_name, 100);
                    num_picture++;
                    if(num_picture > MAX_NUM_FRAME_IMAGAE)
                    {
                        num_picture = 0;
                    }
                    cout << "=> Image saved: " << im_name << endl;
                }
                cout << "Photos count: " << num_picture << endl;
                ofstream listImage((list_file_image).c_str());
                listImage << num_picture << endl;
                listImage.close();;
                // -- End Save frame image ----------------------------------------------------------------------------------------------

                cout << CYELLOW << "Number of people found in the image: " << faces.size() << CRESET << endl;
                
                PTMutex.lock();
                for(size_t i = 0; i < faces.size(); ++i)
                {
                    all_faces.push_back(faces[i]);
                }
                PTMutex.unlock();
            }
                
            old_face_size = faces.size();
        }

        count++;        
        if ( count == 100)
        {
            t = ((double)cv::getTickCount() - t)/cv::getTickFrequency();
            fps = 100.0/t;
            count = 0;
            printf("fps: %.2f \r\n",fps);
        }
    }
}


void FaceProcess::faceRecognition()
{ 
    int old_face_size;

    for(;;)
    {
        PTMutex.lock();
        int temp_total_guests = total_guests;        
        std::vector<matrix<rgb_pixel>> faces = all_faces;
        all_faces.clear();        
        std::vector<matrix<float,0,1>> temp_guest_faces_descriptors = guest_faces_descriptors;
        PTMutex.unlock();

        std::vector<unsigned long> delete_face;

        if(faces.size() && (old_face_size != faces.size()))
        {
            std::vector<matrix<float,0,1>> faces_descriptors = net(faces);

            for(size_t i = 0; i < faces_descriptors.size(); ++i)
            {
                unsigned long staff_id_index = faceCompare(faces_descriptors[i], staff_faces_descriptors);

                if(staff_id_index)
                {
                    staff_id_index -= 1;
                    string temp_StaffName = staff_id[staff_id_index];

                    cout << CBOLDCYAN << "=> Staff ID: " << staff_id_index << " - Name: " << temp_StaffName
                         << CRESET << endl;

                    int Staff_appeared = 0;
                    for(size_t k = 0; k < Staff_Arrival_Count.size(); ++k)
                    {
                        if(Staff_Arrival_Count[k] == temp_StaffName)
                        {
                            Staff_appeared = 1;
                        }
                    }

                    if(!Staff_appeared)
                    {
                        get_date_time();
                        Staff_Arrival_Count.push_back(temp_StaffName);
                        string temp_StaffDateArrival = syear + smonth + sday;
                        string temp_StaffTimeArrival = sday + "/" + smonth + "/" + syear + "-" + shour + ":" + smin + ":" + ssec;
                        Time_Staff_Arrval.push_back(temp_StaffTimeArrival);

                        // -- Save Staff and time arrival to file ----------------
                        staffCount++;
                        string temp_staff_arival = to_string(staffCount) + ". " + temp_StaffName + " (Time arrival: " 
                                                    + temp_StaffTimeArrival + ")\n";
                        list_staff_file = main_path + stringStaffArrival + temp_StaffDateArrival + ".txt";
                        FILE * listStaff;
                        listStaff = fopen((list_staff_file).c_str(), "a");
                        fputs(temp_staff_arival.c_str(), listStaff);
                        fclose(listStaff);
                        // -- End Save Staff and time arrival to file ----------------

                        // -- Save crop faces -----
                        string im_name = main_path + faces_dir + temp_StaffName + " " + to_string(num_faces) + ".jpg";
                        save_jpeg(faces[i], im_name, 100);
                        num_faces++;
                        if(num_faces > MAX_NUM_FACE_IMAGAE)
                        {
                            num_faces = 0;
                        }
                        cout << "Staff faces saved" << endl;
                        ofstream listFaces((list_file_face).c_str());
                        listFaces << num_faces << endl;
                        listFaces.close();
                        // -- End crop faces --------
                    }

                    // faces_descriptors.erase(faces_descriptors.begin() + j);
                    // faces.erase(faces.begin() + j);
                    delete_face.push_back(i);
                }
            }

        //     // cout << "faces_descriptors.size(): " << faces_descriptors.size() << endl;
        //     // cout << "delete_face.size(): " << delete_face.size() << endl;
        //     // Delete staff faces
            DeleteFacesData(faces, faces_descriptors, delete_face);
        //     // End delete staff faces            
        //     // cout << "faces_descriptors.size(): " << faces_descriptors.size() << endl;
        //     // cout << "delete_face.size(): " << delete_face.size() << endl;

            if(temp_guest_faces_descriptors.size())
            {
                for(size_t i = 0; i < faces_descriptors.size(); ++i)
                {
                    unsigned long guest_faces_index = faceCompare(faces_descriptors[i], temp_guest_faces_descriptors);

                    if(guest_faces_index)
                    {
                        guest_faces_index -= 1;

                        cout << CBOLDGREEN << "Old Person" << CRESET << endl;

                        delete_face.push_back(guest_faces_index);
                    }
                }

                // Delete staff faces                
                DeleteFacesData(faces, faces_descriptors, delete_face);
                // End delete staff faces

                if(faces.size())
                {
                    cout << CBOLDRED << "New Guest: " << faces.size() << CRESET << endl;
                    temp_total_guests += faces.size();

                    // -- Save crop faces -----
                    for (size_t j = 0; j < faces.size(); ++j)
                    {
                        string im_name = main_path + faces_dir + to_string(num_faces) + ".jpg";
                        save_jpeg(faces[j], im_name, 100);
                        num_faces++;
                        if(num_faces > MAX_NUM_FACE_IMAGAE)
                        {
                            num_faces = 0;
                        }
                        cout << "=> Guest faces saved: " << im_name << endl;
                    }
                    cout << "Faces image count: " << num_faces << endl;
                    ofstream listFaces((list_file_face).c_str());
                    listFaces << num_faces << endl;
                    listFaces.close();
                    // -- End crop faces --------
                }

                // -- Save face descriptors to guest --
                for (size_t j = 0; j < faces_descriptors.size(); ++j)
                {
                    temp_guest_faces_descriptors.push_back(faces_descriptors[j]);
                }
                // -- End Save face descriptors to guest --
            }            
            else
            {
                if(faces.size())
                {
                    // -- Save face descriptors to guest --
                    temp_guest_faces_descriptors = faces_descriptors;
                    cout << CRED << "New person record to new database: " << faces_descriptors.size() << CRESET << endl;
                    // -- Save face descriptors to guest --
                    temp_total_guests += faces_descriptors.size();

                    // -- Save crop faces -----
                    for (size_t j = 0; j < faces.size(); ++j)
                    {
                        string im_name = main_path + faces_dir + to_string(num_faces) + ".jpg";
                        save_jpeg(faces[j], im_name, 100);
                        num_faces++;
                        if(num_faces > MAX_NUM_FACE_IMAGAE)
                        {
                            num_faces = 0;
                        }
                        cout << "=> Guest faces saved: " << im_name << endl;
                    }
                    cout << "Faces image count: " << num_faces << endl;
                    ofstream listFaces((list_file_face).c_str());
                    listFaces << num_faces << endl;
                    listFaces.close();
                    // -- End crop faces --------
                }
            }

            PTMutex.lock();
            total_guests = temp_total_guests;            
            guest_faces_descriptors = temp_guest_faces_descriptors;
            PTMutex.unlock();
        }

        old_face_size = faces.size();
    }
}


void FaceProcess::peopleProcess()
{    
    int old_num_guest_face;

    for(;;)
    {
        PTMutex.lock();
        std::vector<matrix<float,0,1>> temp_guest_faces_descriptors = guest_faces_descriptors;
        int temp_total_guests = total_guests;
        int temp_old_total_guests = old_total_guests;
        PTMutex.unlock();

        if(old_num_guest_face != temp_guest_faces_descriptors.size())
        {
            get_date_time();
            daily_guests_face_data = main_path + guests + syear + smonth + sday + ".dat";

            if(old_daily_guests_face_data != daily_guests_face_data)
            {
                temp_guest_faces_descriptors.clear();
                old_daily_guests_face_data = daily_guests_face_data;
            }

            serialize(daily_guests_face_data) << temp_guest_faces_descriptors;
            cout << "Saved guest face database: " << daily_guests_face_data << endl;

            old_num_guest_face = temp_guest_faces_descriptors.size();

            PTMutex.lock();
            guest_faces_descriptors = temp_guest_faces_descriptors;
            PTMutex.unlock();
        }

        //cout << "temp1 temp_total_guests: " << temp_total_guests << endl;
        if(temp_old_total_guests != temp_total_guests)
        {
            //cout << "temp2 temp_total_guests: " << temp_total_guests << endl;
            // Save guests counter -----------------------------------------------------------------------------------------------
            get_date_time();
            daily_guests_counter_file = main_path + guests + syear + smonth + sday + ".pt";
            //cout << "file compare: " << daily_guests_counter_file << " | old: " << old_daily_guests_counter_file << endl;

            if(old_daily_guests_counter_file != daily_guests_counter_file)
            {
                temp_total_guests = 0;
                old_daily_guests_counter_file = daily_guests_counter_file;
            }

            ofstream dailyGuest((daily_guests_counter_file).c_str());
            dailyGuest << temp_total_guests;
            dailyGuest.close();
            cout << "Saved file of daily face counter: " << CYELLOW << daily_guests_counter_file << CRESET << endl;

            temp_old_total_guests = temp_total_guests;
            PTMutex.lock();
            //guest_faces_descriptors = temp_guest_faces_descriptors;
            old_total_guests = temp_old_total_guests;
            total_guests = temp_total_guests;
            PTMutex.unlock();
            
            cout << CRED << "### Total Guests today (" << sday << "/" << smonth << "/" << syear << "/" << "): ";
            cout << temp_total_guests << " ###" << CRESET << endl;
            cout << "-------------------------------------------------------------------------------------------------\r\n" << endl;
        }
    }
}

unsigned long FaceProcess::faceCompare(matrix<float,0,1>& faces_video_descriptors, 
                std::vector<matrix<float,0,1>>& faces_data_descriptors)
{
    std::vector<matrix<float,0,1>> add_faces_descriptors;
    add_faces_descriptors.push_back(faces_video_descriptors);
    
    std::vector<unsigned long> faces_data_index;
    faces_data_index.push_back(0);

    for (size_t i = 0; i < faces_data_descriptors.size(); ++i)
    {
        // Faces are connected in the graph if they are close enough.  Here we check if
        // the distance between two face descriptors is less than 0.6, which is the
        // decision threshold the network was trained to use.  Although you can
        // certainly use any other threshold you find useful.
        if (length(faces_data_descriptors[i] - faces_video_descriptors) < face_Threshold)
        {
            add_faces_descriptors.push_back(faces_data_descriptors[i]);
            faces_data_index.push_back(i); //  index of faces_data_descriptors
        }
    }

    std::vector<sample_pair> edges;
    for (size_t i = 1; i < add_faces_descriptors.size(); ++i)
    {
        edges.push_back(sample_pair(0,i));
    }

    std::vector<unsigned long> labels;
    const auto num_clusters = chinese_whispers(edges, labels);

    unsigned long faces_id = 0;
    for (size_t j = 1; j < labels.size(); ++j)
    {
        if (labels[0] == labels[j])
        {
            // cout << CBOLDGREEN << "=> Cluster ID: " << cluster_id << " - labels " << j << ": " << labels[j]
            //      << CRESET << endl;
            faces_id = faces_data_index[j] + 1;
        }
    }

    return faces_id;
}

void FaceProcess::DeleteFacesData(std::vector<matrix<rgb_pixel>>& faces, std::vector<matrix<float,0,1>>& faces_descriptors, 
                                    std::vector<unsigned long>& delete_face)
{
    // Delete faces
    for(size_t j = 0; j < delete_face.size(); ++j)
    {
        faces_descriptors.erase(faces_descriptors.begin() + delete_face[j]);
        faces.erase(faces.begin() + delete_face[j]);
    }
    delete_face.clear();
    // End delete faces
}


// ----------------------------------------------------------------------------------------
// matrix<float,0,1> face_descriptor = mean(mat(net(jitter_image(faces[0]))));
// cout << "jittered face descriptor for one face: " << trans(face_descriptor) << endl;

std::vector<matrix<rgb_pixel>> FaceProcess::jitter_image(const matrix<rgb_pixel>& img)
{
    // All this function does is make 100 copies of img, all slightly jittered by being
    // zoomed, rotated, and translated a little bit differently.
    thread_local random_cropper cropper;
    cropper.set_chip_dims(150,150);
    cropper.set_randomly_flip(true);
    cropper.set_max_object_height(0.99999);
    cropper.set_background_crops_fraction(0);
    cropper.set_min_object_height(0.97);
    cropper.set_translate_amount(0.02);
    cropper.set_max_rotation_degrees(3);

    std::vector<mmod_rect> raw_boxes(1), ignored_crop_boxes;
    raw_boxes[0] = shrink_rect(get_rect(img),3);
    std::vector<matrix<rgb_pixel>> crops;

    matrix<rgb_pixel> temp;
    for (int i = 0; i < 100; ++i)
    {
        cropper(img, raw_boxes, temp, ignored_crop_boxes);
        crops.push_back(move(temp));
    }
    return crops;
}
// ----------------------------------------------------------------------------------------

time_t get_date_time()
{
    time_t now = time(0);

    //cout << "Number of sec since January 1,1970:" << now << endl;
    tm *ltm = localtime(&now);
    //cout << "Date: " << 1 + ltm->tm_mday << "/" << 1 + ltm->tm_mon << "/"  << 1900 + ltm->tm_year << endl;
    //cout << "Time: "<< 1 + ltm->tm_hour << ":" << 1 + ltm->tm_min << ":" << 1 + ltm->tm_sec << endl;
    iday    = ltm->tm_mday;
    sday    = to_string(iday);
    imonth  = 1 + ltm->tm_mon;
    smonth  = to_string(imonth);
    iyear   = 1900 + ltm->tm_year;
    syear   = to_string(iyear);
    ihour   = ltm->tm_hour;
    shour   = to_string(ihour);
    imin    = ltm->tm_min;
    smin    = to_string(imin);
    isec    = ltm->tm_sec;
    ssec    = to_string(isec);

    return now;
}
