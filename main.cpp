#include "opencv2/opencv.hpp"
#include <iostream>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <chrono>
#include <sys/time.h>
#include <ctime>
#include <fstream>
int transfer_byte(int from, int to, int is_control, char ch);

typedef struct {char *name; int flag; } speed_spec;

using namespace cv;
using namespace std;


int main(int argc, char *argv[])
{
	
	VideoCapture cap(0);
	
	
	time_t now = time(nullptr);
	int y;
	y=now;
	int t=0;
	int comfd;
	struct termios oldtio, newtio;       //place for old and new port settings for serial port
	struct termios oldkey, newkey;       //place tor old and new port settings for keyboard teletype
	char *devicename = argv[1];
	int need_exit = 0;
	speed_spec speeds[] =
	{
		{"9600", B9600},
	};
	int speed = B9600;

	if(argc < 2) {
		fprintf(stderr, "example: %s /dev/ttyS0 [115200]\n", argv[0]);
		exit(1);
	}

	comfd = open(devicename, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (comfd < 0)
	{
		perror(devicename);
		exit(-1);
	}

	if(argc > 2) {	
		speed_spec *s;
		for(s = speeds; s->name; s++) {
			if(strcmp(s->name, argv[2]) == 0) {
				speed = s->flag;
				fprintf(stderr, "setting speed %s\n", s->name);
				break;
			}
		}
	}

	fprintf(stderr, "Ctrl-Q exit, Ctrl-Y modem lines status\n");

	tcgetattr(STDIN_FILENO,&oldkey);
	newkey.c_cflag = B9600 | CRTSCTS | CS8 | CLOCAL | CREAD;
	newkey.c_iflag = IGNPAR;
	newkey.c_oflag = 0;
	newkey.c_lflag = 0;
	newkey.c_cc[VMIN]=1;
	newkey.c_cc[VTIME]=0;
	tcflush(STDIN_FILENO, TCIFLUSH);
	tcsetattr(STDIN_FILENO,TCSANOW,&newkey);


	tcgetattr(comfd,&oldtio); // save current port settings 
	newtio.c_cflag = speed | CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;
	newtio.c_cc[VMIN]=1;
	newtio.c_cc[VTIME]=0;
	tcflush(comfd, TCIFLUSH);
	tcsetattr(comfd,TCSANOW,&newtio);

	HOGDescriptor hog;
	hog.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector());

	Mat frames, frame;
	while (true) {
		now = time(nullptr);
		std::ofstream ofs("data.txt");
		fd_set fds;
		int ret;

	
		FD_ZERO(&fds);
		FD_SET(STDIN_FILENO, &fds);
		FD_SET(comfd, &fds);
		cap >> frames;
		resize(frames,frame,Size(640,480));
		vector<Rect> detected;
		hog.detectMultiScale(frame, detected);
		ofs << "PersonIn" << std::endl;
		ofs.close();	
		for (Rect r : detected) {
			Scalar c = Scalar(rand() % 256, rand() % 256, rand() % 256);
			rectangle(frame, r, c, 3);
		}
		if(!detected.size() && t==0){
			need_exit = transfer_byte(STDIN_FILENO, comfd, 1, '0');
			ofs << "CarGo" << std::endl;
			ofs.close();	
		}
		else{
			if(t==0){
		 		y = time(nullptr);
			}
			t=1;
		}
		if(now<y+10 && t==1){
			need_exit = transfer_byte(STDIN_FILENO, comfd, 1, '1');
			ofs << "CarWait" << std::endl;
			ofs.close();	
		}else if(now<y+20 && t==1){
			need_exit = transfer_byte(STDIN_FILENO, comfd, 1, '2');
			ofs << "CarStop" << std::endl;
			ofs.close();	
		}else if(now<y+30 && t==1){
			need_exit = transfer_byte(STDIN_FILENO, comfd, 1, '0');
			ofs << "CarGo" << std::endl;
			ofs.close();	
		}else if(now>=y+30 && detected.size()){
			need_exit = transfer_byte(STDIN_FILENO, comfd, 1, '0');
			ofs << "CarGo" << std::endl;
			ofs.close();	
		 	y = time(nullptr);
		 	t=0;
		}
		
		imshow("frame", frame);
		if (waitKey(10) == 27)
			break;
	}
	tcsetattr(comfd,TCSANOW,&oldtio);
	tcsetattr(STDIN_FILENO,TCSANOW,&oldkey);
	close(comfd);

	return 0;
}

int transfer_byte(int from, int to, int is_control, char ch) {
	char c;
	int ret;
	c=ch;
	
	while(write(to, &c, 1) == -1) {
		if(errno!=EAGAIN && errno!=EINTR) { perror("write failed"); break; }
	}

	return 0;
}


