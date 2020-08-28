#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <time.h>

using namespace std;
using namespace cv;

#define DEBUG_CODE  TRUE

const string  main_path = "PT/";
const string  temp_path = "temp/";
const string  list_im   = "list.pt";

unsigned int old_num_person;
unsigned int num_picture;

/** Function Headers */
void detectAndDisplay( Mat frame );

/** Global variables */
//String face_cascade_name = "haarcascades/haarcascade_frontalface_default.xml";
String face_cascade_name = "haarcascades/haarcascade_frontalface_alt.xml";
String eyes_cascade_name = "haarcascades/haarcascade_eye_tree_eyeglasses.xml";
String nose_cascade_name = "haarcascades/haarcascade_mcs_nose.xml";
String mouth_cascade_name = "haarcascades/haarcascade_mcs_mouth.xml";
// String body_cascade_name = "haarcascades/haarcascade_upperbody.xml";

/* Haarcascades: 
haarcascade_eye_tree_eyeglasses.xml
haarcascade_eye.xml
haarcascade_frontalcatface_extended.xml
haarcascade_frontalcatface.xml
haarcascade_frontalface_alt2.xml
haarcascade_frontalface_alt_tree.xml
haarcascade_frontalface_alt.xml
haarcascade_frontalface_default.xml
haarcascade_fullbody.xml
haarcascade_lefteye_2splits.xml
haarcascade_licence_plate_rus_16stages.xml
haarcascade_lowerbody.xml
haarcascade_mcs_eyepair_big.xml
haarcascade_mcs_eyepair_small.xml
haarcascade_mcs_leftear.xml
haarcascade_mcs_lefteye.xml
haarcascade_mcs_mouth.xml
haarcascade_mcs_nose.xml
haarcascade_mcs_rightear.xml
haarcascade_mcs_righteye.xml
haarcascade_mcs_upperbody.xml
haarcascade_profileface.xml
haarcascade_righteye_2splits.xml
haarcascade_russian_plate_number.xml
haarcascade_smile.xml
haarcascade_upperbody.xml
*/

CascadeClassifier face_cascade;
CascadeClassifier eyes_cascade;
CascadeClassifier nose_cascade;
CascadeClassifier mouth_cascade;
CascadeClassifier body_cascade;

string window_name = "PT - Face detection";
RNG rng(12345);

/** @function main */
int main( int argc, const char** argv )
{
  system(("mkdir "+main_path).c_str());
  system(("mkdir "+main_path+temp_path).c_str());
  old_num_person = 0;

  string list_file_im = main_path + temp_path + list_im;
  //ofstream listImage ((list_file_im).c_str(), std::ios::out | std::ios::trunc);
  ifstream listImage((list_file_im).c_str());
  listImage >> num_picture;

  //listImage.seekg(0, std::ios::end);  // move pointer to end of file 
  //listImage << "\n" << num_picture << ":\t*" << im_name << "#\n" << endl;

  //------------------------------------------------------------------------------------------

  listImage.close();


  CvCapture* capture;
  Mat frame;

  //-- 1. Load the cascades
  if( !face_cascade.load( face_cascade_name ) )
  {
    printf("--(!)Error loading\n"); 
    return -1; 
  }

  if( !eyes_cascade.load( eyes_cascade_name ) )
  { 
    printf("--(!)Error loading\n"); 
    return -1; 
  }

  if( !nose_cascade.load( nose_cascade_name ) )
  { 
    printf("--(!)Error loading\n"); 
    return -1; 
  }

  if( !mouth_cascade.load( mouth_cascade_name ) )
  { 
    printf("--(!)Error loading\n"); 
    return -1; 
  }

  //-- 2. Read the video stream
  capture = cvCaptureFromCAM( -1 );
  if( capture )
  {
    while( true )
    {
      frame = cvQueryFrame( capture );

      //-- 3. Apply the classifier to the frame
      if( !frame.empty() )
      {
        detectAndDisplay( frame ); 
      }
      else
      {
        printf(" --(!) No captured frame -- Break!"); 
        break; 
      }

      int c = waitKey(10);
      if( (char)c == 'c' ) 
      { 
        break; 
      }
    }
  }
  return 0;
}

/** @function detectAndDisplay */
void detectAndDisplay( Mat frame )
{
  std::vector<Rect> faces;
  Mat frame_gray;

  cvtColor( frame, frame_gray, CV_BGR2GRAY );
  equalizeHist( frame_gray, frame_gray );


  unsigned int isHuman = 0;


  //-- Detect faces
  face_cascade.detectMultiScale( frame_gray, faces, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE, Size(100, 100) );

  for( size_t i = 0; i < faces.size(); i++ )
  {
    Point center( faces[i].x + faces[i].width*0.5, faces[i].y + faces[i].height*0.5 );

#ifdef DEBUG_CODE
    //ellipse( frame, center, Size( faces[i].width*0.5, faces[i].height*0.5), 0, 0, 360, Scalar( 255, 0, 255 ), 4, 8, 0 );
    ellipse( frame, center, Size( faces[i].width*0.7, faces[i].height*0.7), 0, 0, 360, Scalar( 255, 0, 255 ), 1, 1, 0 );
    string imgText=string("Face Detected");
    putText(frame,imgText,center, 1, 1.2f,Scalar(200,0,0),2);
#endif

    Mat faceROI = frame_gray( faces[i] );
    std::vector<Rect> eyes;
    std::vector<Rect> nose;
    std::vector<Rect> mouth;

    //-- In each face, detect eyes
    eyes_cascade.detectMultiScale( faceROI, eyes, 1.1, 2, 0 |CV_HAAR_SCALE_IMAGE, Size(1, 1) );

    for( size_t j = 0; j < eyes.size(); j++ )
    {
      Point center( faces[i].x + eyes[j].x + eyes[j].width*0.5, faces[i].y + eyes[j].y + eyes[j].height*0.5 );
      int radius = cvRound( (eyes[j].width + eyes[j].height)*0.25 );

#ifdef DEBUG_CODE
      if(j < 2)
      {
        circle( frame, center, radius, Scalar( 255, 0, 0 ), 4, 8, 0 );
        string imgText=string("Eye");
        putText(frame,imgText,center, 1, 1.2f,Scalar(200,0,0),2);
      }
#endif      

      isHuman++;
    }

    //-- In each face, detect nose
    nose_cascade.detectMultiScale( faceROI, nose, 1.1, 2, 0 |CV_HAAR_SCALE_IMAGE, Size(30, 30) );

    for( size_t j = 0; j < nose.size(); j++ )
    {
      Point center( faces[i].x + nose[j].x + nose[j].width*0.5, faces[i].y + nose[j].y + nose[j].height*0.5 );
      int radius = cvRound( (nose[j].width + nose[j].height)*0.25 );

#ifdef DEBUG_CODE
      if(j < 1)
      {
        circle( frame, center, radius, Scalar( 10, 10, 200 ), 4, 8, 0 );
        string imgText=string("Nose");
        putText(frame,imgText,center, 1, 1.2f,Scalar(10,10,200),2);
      }
#endif      

      isHuman++;
    }

    //-- In each face, detect mouth
    mouth_cascade.detectMultiScale( faceROI, mouth, 1.1, 2, 0 |CV_HAAR_SCALE_IMAGE, Size(30, 30) );

    for( size_t j = 0; j < mouth.size(); j++ )
    {
      Point center( faces[i].x + mouth[j].x + mouth[j].width*0.5, faces[i].y + mouth[j].y + mouth[j].height*0.5 );
      int radius = cvRound( (mouth[j].width + mouth[j].height)*0.25 );

#ifdef DEBUG_CODE
      if(j < 1)
      {
        circle( frame, center, radius, Scalar( 100, 100, 10 ), 4, 8, 0 );
        string imgText=string("Mouth");
        putText(frame,imgText,center, 1, 1.2f,Scalar(100,100,10),2);
      }
#endif      

      isHuman++;
    }

  }

  time_t curtime;
  time(&curtime);
  //printf("Current time = %s", ctime(&curtime));

  string timeDisp = ctime(&curtime);
  string countText = "PT - "+ timeDisp;
  putText(frame,countText,Point(10,470), 1, 1.2f,Scalar(100,10,200),2);

  countText = "Face counting: " + to_string(faces.size());
  putText(frame,countText,Point(400,470), 1, 1.2f,Scalar(100,10,200),2);

  // cout << "### Human Detected" << endl;
  // cout << "### old_num_person: " << old_num_person << " ~ faces.size(): " << faces.size() << endl;
  if(old_num_person != faces.size())
  {
    if(faces.size() && isHuman)
    {
      string im_name = main_path + temp_path + to_string(num_picture) + ".jpg";
      //im_name << num_picture;
      cout << "\r\nImage saved: " << im_name << endl;
      imwrite(im_name, frame);
      num_picture++;
      cout << "Photos count: " << num_picture;


      //string list_file_im = main_path + temp_path + list_im;
      //ofstream listImage ((list_file_im).c_str(), std::ios::out | std::ios::trunc);
      string list_file_im = main_path + temp_path + list_im;
      ofstream listImage((list_file_im).c_str());
      listImage << num_picture << endl;

      //listImage.seekg(0, std::ios::end);  // move pointer to end of file 
      //listImage << "\n" << num_picture << ":\t*" << im_name << "#\n" << endl;

      listImage.close();
    }
    cout << "\r\nPerson increase or decrease" << endl;
  }
  old_num_person = faces.size();
  // countText = "Total Persons: " + to_string(humanCounter);
  // putText(frame,countText,Point(10,470), 1, 1.2f,Scalar(100,10,200),2);

  //-- Show what you got
  imshow( window_name, frame );
}