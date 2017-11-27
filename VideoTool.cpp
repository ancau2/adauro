#include <sstream>
#include <string>
#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/opencv.hpp"
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <math.h>
#define PI 3.1415
#define PORT 20232
#define TIMProtirepentru360 2.25
using namespace std;
using namespace cv;
//initial min and max HSV filter values.
//these will be changed using trackbars
int H_MIN = 0;
int H_MAX = 256;
int S_MIN = 0;
int S_MAX = 256;
int V_MIN = 0;
int V_MAX = 256;
//default capture width and height
const int FRAME_WIDTH = 640;
const int FRAME_HEIGHT = 480;
//max number of objects to be detected in frame
//minimum and maximum object area
const int MIN_OBJECT_AREA = 20 * 20;
const int MAX_OBJECT_AREA = FRAME_HEIGHT*FRAME_WIDTH / 1.5;
//names that will appear at the top of each window
const std::string windowName = "Original Image";
const std::string windowName2 = "Thresholded Image";
const std::string trackbarWindowName = "Trackbars";





void on_trackbar(int, void*)
{//This function gets called whenever a
 // trackbar position is changed
}

string intToString(int number) {


	std::stringstream ss;
	ss << number;
	return ss.str();
}
//DE SCOS
void createTrackbars() {
	//create window for trackbars


	namedWindow(trackbarWindowName, 0);
	//create memory to store trackbar name on window
	char TrackbarName[50];
	sprintf(TrackbarName, "H_MIN", H_MIN);
	sprintf(TrackbarName, "H_MAX", H_MAX);
	sprintf(TrackbarName, "S_MIN", S_MIN);
	sprintf(TrackbarName, "S_MAX", S_MAX);
	sprintf(TrackbarName, "V_MIN", V_MIN);
	sprintf(TrackbarName, "V_MAX", V_MAX);
	//create trackbars and insert them into window
	//3 parameters are: the address of the variable that is changing when the trackbar is moved(eg.H_LOW),
	//the max value the trackbar can move (eg. H_HIGH),
	//and the function that is called whenever the trackbar is moved(eg. on_trackbar)
	//                                  ---->    ---->     ---->
	createTrackbar("H_MIN", trackbarWindowName, &H_MIN, H_MAX, on_trackbar);
	createTrackbar("H_MAX", trackbarWindowName, &H_MAX, H_MAX, on_trackbar);
	createTrackbar("S_MIN", trackbarWindowName, &S_MIN, S_MAX, on_trackbar);
	createTrackbar("S_MAX", trackbarWindowName, &S_MAX, S_MAX, on_trackbar);
	createTrackbar("V_MIN", trackbarWindowName, &V_MIN, V_MAX, on_trackbar);
	createTrackbar("V_MAX", trackbarWindowName, &V_MAX, V_MAX, on_trackbar);


}

void drawObject(int x, int y, Mat &frame) {

	//use some of the openCV drawing functions to draw crosshairs
	//on your tracked image!

	//UPDATE:JUNE 18TH, 2013
	//added 'if' and 'else' statements to prevent
	//memory errors from writing off the screen (ie. (-25,-25) is not within the window!)

	circle(frame, Point(x, y), 20, Scalar(0, 255, 0), 2);
	if (y - 25 > 0)
		line(frame, Point(x, y), Point(x, y - 25), Scalar(0, 255, 0), 2);
	else line(frame, Point(x, y), Point(x, 0), Scalar(0, 255, 0), 2);
	if (y + 25 < FRAME_HEIGHT)
		line(frame, Point(x, y), Point(x, y + 25), Scalar(0, 255, 0), 2);
	else line(frame, Point(x, y), Point(x, FRAME_HEIGHT), Scalar(0, 255, 0), 2);
	if (x - 25 > 0)
		line(frame, Point(x, y), Point(x - 25, y), Scalar(0, 255, 0), 2);
	else line(frame, Point(x, y), Point(0, y), Scalar(0, 255, 0), 2);
	if (x + 25 < FRAME_WIDTH)
		line(frame, Point(x, y), Point(x + 25, y), Scalar(0, 255, 0), 2);
	else line(frame, Point(x, y), Point(FRAME_WIDTH, y), Scalar(0, 255, 0), 2);

	putText(frame, intToString(x) + "," + intToString(y), Point(x, y + 30), 1, 1, Scalar(0, 255, 0), 2);
	//cout << "x,y: " << x << ", " << y;

}
void morphOps(Mat &thresh) {

	//create structuring element that will be used to "dilate" and "erode" image.
	//the element chosen here is a 3px by 3px rectangle

	Mat erodeElement = getStructuringElement(MORPH_RECT, Size(3, 3));
	//dilate with larger element so make sure object is nicely visible
	Mat dilateElement = getStructuringElement(MORPH_RECT, Size(8, 8));

	erode(thresh, thresh, erodeElement);
	erode(thresh, thresh, erodeElement);


	dilate(thresh, thresh, dilateElement);
	dilate(thresh, thresh, dilateElement);



}
void trackFilteredObject(int &x, int &y, Mat threshold, Mat &cameraFeed) {
	//*de scos
	Mat temp;
	threshold.copyTo(temp);
	//*//

	//these two vectors needed for output of findContours
	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;
	//find contours of filtered image using openCV findContours function
	findContours(temp, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
	//use moments method to find our filtered object
	double refArea = 0;
	bool objectFound = false;
	
	if (hierarchy.size() > 0) {
		int numObjects = hierarchy.size();
		//if number of objects greater than MAX_NUM_OBJECTS we have a noisy filter
		if (numObjects < 10) {
			for (int index = 0; index >= 0; index = hierarchy[index][0]) {

				Moments moment = moments((cv::Mat)contours[index]);
				double area = moment.m00;

				//if the area is less than 20 px by 20px then it is probably just noise
				//if the area is the same as the 3/2 of the image size, probably just a bad filter
				//we only want the object with the largest area so we safe a reference area each
				//iteration and compare it to the area in the next iteration.
				if (area > MIN_OBJECT_AREA && area<MAX_OBJECT_AREA && area>refArea) {
					x = moment.m10 / area;
					y = moment.m01 / area;
					objectFound = true;
					refArea = area;
				}
				else objectFound = false;


			}
			//let user know you found an object
			if (objectFound == true) {
				putText(cameraFeed, "Tracking Object", Point(0, 50), 2, 1, Scalar(0, 255, 0), 2);
				//draw object location on screen
				//cout << x << "," << y;
				drawObject(x, y, cameraFeed);

			}


		}
		else putText(cameraFeed, "TOO MUCH NOISE! ADJUST FILTER", Point(0, 50), 1, 2, Scalar(0, 0, 255), 2);
	}
}


int move(char string[],float delay)
{
    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    //char hello[256] =argv[1];
    //char buffer[1024] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }
  
    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
      
    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "193.226.12.217", &serv_addr.sin_addr)<=0) 
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
  
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }
	char cmd[2];
	for(int i = 0; i < strlen(string); ++i)
			if(string[i] != 'f' && string[i] != 'b' && string[i] != 'r' && string[i] != 'l' && string[i] != 's')
				continue;
			else{
				sprintf(cmd, "%c", string[i]);
				send(sock , cmd, strlen(cmd), 0 );
				sleep(delay);
			}
    return 0;
}

float calculeazaUnghi(float xOld, float yOld, float xNew, float yNew, float x, float y, float eroare){
	// xOld, yOld - coordonatele de la pasul anterior al robotului nostru
	// xNew, yNew - coordonatele de la pasul curent al robotului nostru
	// x, y - coordonatele de la pasul curent al celuilalt robot

	float unghi1, unghi2, unghi;

	if (yOld - eroare <= yNew && yNew <= yOld + eroare){
		// miscare pe verticala
		if (xOld < xNew){
			unghi1 = 270.0;
		}
		else{
			unghi1 = 90.0;
		}
	}
	else if (xOld - eroare <= xNew && xNew <= xOld + eroare){
		// miscare pe orizontala
		if (yOld < yNew){
			unghi1 = 0.0;
		}
		else{
			unghi1 = 180.0;
		}
	}
	else{
		// miscare oblica
		unghi1 = atan2(yNew - yOld, xOld - xNew) / PI * 180.0;
	}

	if (yNew - eroare <= y && y <= yNew + eroare){
		// miscare pe verticala
		if (xNew < x){
			unghi2 = 270.0;
		}
		else{
			unghi2 = 90.0;
		}
	}
	else if (xNew - eroare <= x && x <= xNew + eroare){
		// miscare pe orizontala
		if (yNew < y){
			unghi2 = 0.0;
		}
		else{
			unghi2 = 180.0;
		}
	}
	else{
		// miscare oblica
		unghi2 = atan2(y - yNew, xNew - x) / PI * 180.0;
	}


	// DE STERS DUPA TESTARE !!!
	printf("unghi_1 = %f ; unghi_2 = %f\n", unghi1, unghi2);

	unghi =  unghi2 - unghi1;

	if (unghi < -180.0){
		return unghi + 360.0;
	}
	else if (unghi > 180.0){
		return unghi - 360.0;
	}
	return unghi;
}

void move(char dir, float unghi){
	if (dir == 'l'){
		printf("Se roteste la stanga cu %f grade timp de %f\n", unghi,(unghi*TIMProtirepentru360)/360);
	}
	else if (dir == 'r'){
		printf("Se roteste la dreapta cu %f grade timpe de %f\n", unghi,(unghi*TIMProtirepentru360)/360);
	}

	// apoi se misca in fata x ms
	printf("Se deplaseaza in fata\n\n");
}

int main(int argc, char* argv[])
{
	//some boolean variables for different functionality within this
	//program
	bool trackObjects = true;
	bool useMorphOps = true;
	//Point p;
	//Matrix to store each frame of the webcam feed
	Mat cameraFeed;
	//matrix storage for HSV image
	Mat HSV;
	//matrix storage for binary threshold image
	Mat threshold;
	//x and y values for the location of the object
	int x = 0, y = 0;
	int xeu=0,yeu=0;
	//create slider bars for HSV filtering
	createTrackbars();
	//video capture object to acquire webcam feed
	VideoCapture capture;

	//open capture object at location zero (default location for webcam)
//!!!!//capture.open("rtmp://172.16.254.99/live/nimic");
	// open the default camera, use something different from 0 otherwise;
    // Check VideoCapture documentation.
    if(!capture.open(0))
        return 0;
	//set height and width of capture frame
	//capture.set(CV_CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
	//capture.set(CV_CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);
	//start an infinite loop where webcam feed is copied to cameraFeed matrix
	//all of our operations will be performed within this loop
	//directie();
	while (1) {
		//store image to matrix
		capture.read(cameraFeed);
		if(!cameraFeed.empty()){
			//convert frame from BGR to HSV colorspace
			cvtColor(cameraFeed, HSV, COLOR_BGR2HSV);
			//filter HSV image between values and store filtered image to
			//threshold matrix
			inRange(HSV, Scalar(H_MIN, S_MIN, V_MIN), Scalar(H_MAX, S_MAX, V_MAX), threshold);
			//perform morphological operations on thresholded image to eliminate noise
			//and emphasize the filtered object(s)
			if (useMorphOps)
				morphOps(threshold);
			//pass in thresholded frame to our object tracking function
			//this function will return the x and y coordinates of the
			//filtered object
			if (trackObjects){
				trackFilteredObject(x, y, threshold, cameraFeed);
			}
			//x si y coordonate  le  gfusyd
			xeu=x;
			yeu=y;
			/*inRange(HSV, Scalar(0, 79, 223), Scalar(91, S_MAX, V_MAX), threshold);
			if (useMorphOps)
				morphOps(threshold);
			if (trackObjects){
				trackFilteredObject(x, y, threshold, cameraFeed);
			}*/
			
			//strategie(x,y,xeu,yeu);
			//show frames
			imshow(windowName2, threshold);
			imshow(windowName, cameraFeed);
			//imshow(windowName1, HSV);
			//delay 30ms so that screen can refresh.
			//image will not appear without this waitKey() command
			waitKey(30);
		}
	}
//168 256 60 256 70 256 ROZ
//0 91 79 256 223 256 GALBEN

	//move(argv[1]);
	return 0;
}

