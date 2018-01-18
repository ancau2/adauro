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
#define UnDelayLaNimereala 500000
using namespace std;
using namespace cv;

float TIMProtirepentru1grad=TIMProtirepentru360/360;

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

int sock = 0;

float unghi1_anterior = 0.0;

void on_trackbar(int, void*)
{//This function gets called whenever a
 // trackbar position is changed
}
string intToString(int number) {
	std::stringstream ss;
	ss << number;
	return ss.str();
}
//!!!!!!!!!!!!!!!!!!!DE SCOS
void createTrackbars() {
	//create window for trackbars
	namedWindow("Trackbars", 0);
	//create memory to store trackbar name on window
	char TrackbarName[50];
	sprintf(TrackbarName, "H_MIN");
	sprintf(TrackbarName, "H_MAX");
	sprintf(TrackbarName, "S_MIN");
	sprintf(TrackbarName, "S_MAX");
	sprintf(TrackbarName, "V_MIN");
	sprintf(TrackbarName, "V_MAX");
	//create trackbars and insert them into window
	//3 parameters are: the address of the variable that is changing when the trackbar is moved(eg.H_LOW),
	//the max value the trackbar can move (eg. H_HIGH),
	//and the function that is called whenever the trackbar is moved(eg. on_trackbar)
	//                                  ---->    ---->     ---->
	createTrackbar("H_MIN", "Trackbars", &H_MIN, H_MAX, on_trackbar);
	createTrackbar("H_MAX", "Trackbars", &H_MAX, H_MAX, on_trackbar);
	createTrackbar("S_MIN", "Trackbars", &S_MIN, S_MAX, on_trackbar);
	createTrackbar("S_MAX", "Trackbars", &S_MAX, S_MAX, on_trackbar);
	createTrackbar("V_MIN", "Trackbars", &V_MIN, V_MAX, on_trackbar);
	createTrackbar("V_MAX", "Trackbars", &V_MAX, V_MAX, on_trackbar);
}
void drawObject(int x, int y, Mat &frame) {
	//use some of the openCV drawing functions to draw crosshairs on your tracked image!
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

bool trackFilteredObject(int &x, int &y, Mat threshold, Mat &cameraFeed) {
	//*!!!!!!de scos
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


			}//si asta e de scos
			//let user know you found an object
			if (objectFound == true) {
				putText(cameraFeed, "Tracking Object", Point(0, 50), 2, 1, Scalar(0, 255, 0), 2);
				//draw object location on screen
				//cout << x << "," << y;
				drawObject(x, y, cameraFeed);

			}
			return objectFound;
		}
		//else putText(cameraFeed, "TOO MUCH NOISE! ADJUST FILTER", Point(0, 50), 1, 2, Scalar(0, 0, 255), 2);
	}
	return false;
}

int connect(){
    struct sockaddr_in address;
    int valread;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Socket creation error\n");
        return -1;
    }
  
    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
      
    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "193.226.12.217", &serv_addr.sin_addr)<=0) 
    {
        printf("Invalid address/ Address not supported \n");
        return -1;
    }
  
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("Connection Failed\n");
        return -1;
    }

    return 0;
}

float calculeazaUnghi(float xOld, float yOld, float xNew, float yNew, float x, float y, float eroare){
	// xOld, yOld - coordonatele de la pasul anterior al robotului nostru
	// xNew, yNew - coordonatele de la pasul curent al robotului nostru
	// x, y - coordonatele de la pasul curent al celuilalt robot

	float unghi1, unghi2, unghi;

	if (yOld - eroare <= yNew && yNew <= yOld + eroare && xOld - eroare <= xNew && xNew <= xOld + eroare){
		// robotul nostru nu s-a miscat
		// consider ca unghiul este unghiul de la pasul anterior
		unghi1 = unghi1_anterior;
	} else if (yOld - eroare <= yNew && yNew <= yOld + eroare){
		// miscare pe verticala (traiectoria robotului nostru)
		if (xOld < xNew){
			unghi1 = 270.0;
		}
		else{
			unghi1 = 90.0;
		}
	}
	else if (xOld - eroare <= xNew && xNew <= xOld + eroare){
		// miscare pe orizontala (traiectoria robotului nostru)
		if (yOld < yNew){
			unghi1 = 0.0;
		}
		else{
			unghi1 = 180.0;
		}
	}
	else{
		// miscare oblica (traiectoria robotului nostru)
		unghi1 = atan2(yNew - yOld, xOld - xNew) / PI * 180.0;
	}

	unghi1_anterior = unghi1;

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
	// END DE STERS DUPA TESTARE !!!

	unghi =  unghi2 - unghi1;
	if (unghi < -180.0){
		return unghi + 360.0;
	}
	else if (unghi > 180.0){
		return unghi - 360.0;
	}
	return unghi;
}

//functia care comanda deplasare pt un anumit timp
void move(char string[], float unghi){
	char cmd[2];
	for(int i = 0; i < strlen(string); ++i){
		if(string[i] != 'f' && string[i] != 'b' && string[i] != 'r' && string[i] != 'l' && string[i] != 's')
			continue;
		else{
			sprintf(cmd, "%c", string[i]);
			printf("%s\n",cmd);
			send(sock , cmd, strlen(cmd), 0 );
			if(string[i]=='l'||string[i]=='r'){
				usleep(UnDelayLaNimereala*TIMProtirepentru1grad*unghi);
			}
			if(string[i]=='s'){
				continue;
			}
			else{
				usleep(UnDelayLaNimereala);
			}
		}
	}
}

/* void move(char dir, float unghi){
// 	if (dir == 'l'){
// 		printf("Se roteste la stanga cu %f grade timp de %f\n", unghi, unghi * TIMProtirepentru1grad);
// 	}
// 	else if (dir == 'r'){
// 		printf("Se roteste la dreapta cu %f grade timpe de %f\n", unghi, (unghi * TIMProtirepentru360) / 360);
// 	}

// 	// apoi se misca in fata x ms
// 	printf("Se deplaseaza in fata\n\n");
//  }
*/
int main(int argc, char* argv[]){

	// robotul nostru - albastru
	int H_MIN_2 = 1;
	int H_MAX_2 = 256;
	int S_MIN_2 = 235;
	int S_MAX_2 = 256;
	int V_MIN_2 = 33;
	int V_MAX_2 = 256;
	
	// robotul adversar - rosu
	int H_MIN_1 = 136;
	int H_MAX_1 = 256;
	int S_MIN_1 = 198;
	int S_MAX_1 = 256;
	int V_MIN_1 = 1;
	int V_MAX_1 = 256;
	bool trackObjects = true;
	bool useMorphOps = true;

	int eroare = 0.0;
	float deviere = 10.0;

	float unghi;

	Mat cameraFeed;
	Mat HSV;
	Mat threshold;

	//x and y values for the location of the object
	int x = 0, y = 0;

	int xNew, yNew; // ultimele coordonate ale robotului nostru
	int xOld, yOld; // penultimele coordonate ale robotului nostru
	int xAdv, yAdv; // ultimele coordonate ale adversarului

	// coordonate temporare
	int tmp_xOld, tmp_yOld, tmp_xNew, tmp_yNew;

	//create slider bars for HSV filtering
	//createTrackbars();
	//video capture object to acquire webcam feed
	VideoCapture capture;

	//open capture object at location zero (default location for webcam)
	//capture.open("rtmp://172.16.254.99/live/nimic");
	// open the default camera, use something different from 0 otherwise;
    if(!capture.open(0))
        return 0;

	//set height and width of capture frame
	//capture.set(CV_CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
	//capture.set(CV_CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);

    // conectarea la socket
    if(connect()!=0){printf("nu se poate connecta la socket\n");
		return 0;
	}

	printf("Robotul este pregatit pentru competitie. Apasa orice tasta pentru a incepe\n");
	getchar();
	createTrackbars();
	// Determin coordonatele initiale ale robotului nostru
	
	while(true){
		capture.read(cameraFeed);
		if(!cameraFeed.empty()){
			//convert frame from BGR to HSV colorspace
			cvtColor(cameraFeed, HSV, COLOR_BGR2HSV);
			printf("[1]");
			/********************** ROBOTUL NOSTRU **************************/
			inRange(HSV, Scalar(H_MIN_1, S_MIN_1, V_MIN_1), Scalar(H_MAX_1, S_MAX_1, V_MAX_1), threshold);

			if (useMorphOps){
				morphOps(threshold);
				//printf("[1] %d %d\n", x, y);
			}

			if (trackObjects){
				if(trackFilteredObject(x, y, threshold, cameraFeed)){
					imshow("FEED", cameraFeed);
					waitKey(30);
					//waitKey(30);
					xOld = x;
					yOld = y;
					break;
				}
			}
			
		}
	}

	// il mut putin in fata pentru a afla directia
	//printf("forward, wait(?), stop\n");
	move("fx",1);

	// detectez noua pozitie a robotului, dupa care detectez directia lui (unghi1_anterior)
	while(true){
		capture.read(cameraFeed);
		if(!cameraFeed.empty()){
			printf("[2]");
			//convert frame from BGR to HSV colorspace
			cvtColor(cameraFeed, HSV, COLOR_BGR2HSV);
			/* ********************* ROBOTUL NOSTRU ************************* */
			inRange(HSV, Scalar(H_MIN_1, S_MIN_1, V_MIN_1), Scalar(H_MAX_1, S_MAX_1, V_MAX_1), threshold);

			if (useMorphOps){
				morphOps(threshold);
				//printf("[1] %d %d\n", x, y);
			}

			if (trackObjects){
				if(trackFilteredObject(x, y, threshold, cameraFeed)){
					xNew = x;
					yNew = y;
					imshow("FEED", cameraFeed);
					waitKey(30);
					//imshow("FEED", cameraFeed);
					//waitKey(30);

					// determin unghiul cu care s-a miscat robotul

					if (yOld - eroare <= yNew && yNew <= yOld + eroare && xOld - eroare <= xNew && xNew <= xOld + eroare){
						// robotul nostru nu s-a miscat - consider ca a avut suficienta baterie ca sa se miste, dar camera inca nu a detectat miscarea
						continue;
					} else if (yOld - eroare <= yNew && yNew <= yOld + eroare){
						// miscare pe verticala (traiectoria robotului nostru)
						if (xOld < xNew){
							unghi1_anterior = 270.0;
						}
						else{
							unghi1_anterior = 90.0;
						}
					}
					else if (xOld - eroare <= xNew && xNew <= xOld + eroare){
						// miscare pe orizontala (traiectoria robotului nostru)
						if (yOld < yNew){
							unghi1_anterior = 0.0;
						}
						else{
							unghi1_anterior = 180.0;
						}
					}
					else{
						// miscare oblica (traiectoria robotului nostru)
						unghi1_anterior = atan2(yNew - yOld, xOld - xNew) / PI * 180.0;
					} // OBSERVSATIE! E imposibil ca robotul sa stea pe loc, pentru ca ii dau un 'forward'

					break;
				}
			}

			//imshow("FEED", cameraFeed);
			//waitKey(30);
		}
	}

	printf("Directie detectata\n");
	printf("Unghi = %f\n", unghi1_anterior);
	while(cameraFeed.empty());
	inRange(HSV, Scalar(H_MIN_1, S_MIN_1, V_MIN_1), Scalar(H_MAX_1, S_MAX_1, V_MAX_1), threshold);

	while (1) {
		//store image to matrix
		capture.read(cameraFeed);

		if(!cameraFeed.empty()){
			
			//convert frame from BGR to HSV colorspace
			cvtColor(cameraFeed, HSV, COLOR_BGR2HSV);
			
			/* ********************* ROBOTUL NOSTRU ************************* */
			inRange(HSV, Scalar(H_MIN_1, S_MIN_1, V_MIN_1), Scalar(H_MAX_1, S_MAX_1, V_MAX_1), threshold);

			if (useMorphOps){
				morphOps(threshold);
				//printf("[1] %d %d\n", x, y);
			}

			if (trackObjects){
				if(!trackFilteredObject(x, y, threshold, cameraFeed)){
					continue;
				}

				// s-a detectat culoarea, deci actualizez coordonatele
				// actualizez doar temporar coordonatele, pentru ca acestea trebuie actualizate doar atunci cand si adversarul a fost detectat
				tmp_xOld = xNew;
				tmp_yOld = yNew;
				tmp_xNew = x;
				tmp_yNew = y;
			}
					
			/* ********************* ROBOTUL ADVERSAR ************************ */
			inRange(HSV, Scalar(H_MIN_2, S_MIN_2, V_MIN_2), Scalar(H_MAX_2, S_MAX_2, V_MAX_2), threshold);

			if (useMorphOps){
				morphOps(threshold);
			}
	
			if (trackObjects){
				if(!trackFilteredObject(x, y, threshold, cameraFeed)){
					imshow("FEED", cameraFeed);
					waitKey(30);
					continue;
				}

				// s-a detectat culoarea, deci actualizez coordonatele
				xAdv = x;
				yAdv = y;

				xOld = tmp_xOld;
				yOld = tmp_yOld;
				xNew = tmp_xNew;
				yNew = tmp_yNew;
			}
			imshow("FEED", cameraFeed);
			waitKey(30);
			
			//show frames
			//imshow("threshold", threshold);
			//imshow("FEED", cameraFeed);
			//imshow(windowName1, HSV);

			// ca sa nu facem atat de multe miscari de rotatie (pentru ca nu putem obtine miscari
			// fine de rotatie) putem sa verificam daca eroarea traiectoriei noastre  este mai 
			// mica decat o EROARE MAXIMA (variabila deviere:float)
	
			unghi = calculeazaUnghi(xOld, yOld, xNew, yNew, xAdv, yAdv, eroare);

			if (unghi < -deviere){
				move("lf", -unghi); // left
			}
			else if (unghi > deviere){
				move("rf", unghi); // right
			}
			else{
				move("fx", unghi);
			}

			//delay 30ms so that screen can refresh.
			//image will not appear without this waitKey() command
			 // de ales intarzierea potrivita pentru a detecta corect pozitia!
		
		}
	}
	return -1;
}
