all:
	g++ -g -Wall -std=c++11 -O3 -I.. ../dlib/all/source.cpp -pthread -lX11 -o main main.cpp faceProcess.cpp fileProcess.cpp -ldlib `pkg-config --cflags --libs opencv`

cross-arm:
	arm-linux-gnueabihf-g++ -g -std=c++11 -O3 -I.. ../dlib/all/source.cpp -pthread -o main-arm main.cpp faceProcess.cpp fileProcess.cpp `pkg-config --cflags --libs opencv`

video_face_recognition:
	g++ -g -std=c++11 -O3 -I.. ../dlib/all/source.cpp -lpthread -lX11 -o video_face_recognition video_face_recognition.cpp -ldlib `pkg-config --cflags --libs opencv`

test:
	g++ -g -std=c++11 -O3 -I.. ../dlib/all/source.cpp -lpthread -lX11 -o test test.cpp  faceProcess.cpp fileProcess.cpp -ldlib `pkg-config --cflags --libs opencv`

dir_nav_ex:
	g++ -g -std=c++11 -O3 -I.. ../dlib/all/source.cpp -lpthread -lX11 -o dir_nav_ex dir_nav_ex.cpp -ldlib `pkg-config --cflags --libs opencv`

thread:
	g++ -g -std=c++11 -pthread -o thread thread.cpp

webcam_fast:	
	g++ -g -Wall -std=c++11 -O3 -I.. ../dlib/all/source.cpp -pthread -lX11 -o webcam_fast webcam_face_pose_fast.cpp -ldlib `pkg-config --cflags --libs opencv`

genStaff:
	g++ -g -Wall -std=c++11 -DDLIB_PNG_SUPPORT -DDLIB_JPEG_SUPPORT -O3 -I.. ../dlib/all/source.cpp -lpng -ljpeg -pthread -lX11 -o genStaff genStaff.cpp fileProcess.cpp -ldlib `pkg-config --cflags --libs opencv`


externalFuntion:
	g++ -g -Wall -std=c++11 -o externalFuntion externalFuntion.cpp sms.c

run:
	./main 0 disp opencv 0.45 1 1

clean:
	rm -rf video_face_recognition genFaceData test dir_nav_ex thread externalFuntion genStaff webcam_fast cross-arm


#sudo apt-get install libdlib-dev
