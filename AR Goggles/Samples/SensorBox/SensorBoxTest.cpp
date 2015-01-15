#include "OVR.h"
#include "Kernel/OVR_String.h"

#include "../CommonSrc/Platform/Platform_Default.h"
#include "../CommonSrc/Render/Render_Device.h"
#include "../CommonSrc/Platform/Gamepad.h"

#include <iostream>
#include <fstream>
#include <opencv\cv.h>
#include <opencv\highgui.h>
#include <time.h>
#include <sstream>
#include <math.h>

using namespace OVR;
using namespace OVR::Platform;
using namespace OVR::Render;
using namespace std;
using namespace cv;

Mat imageCamRight;
Mat imageCamLeft;
Mat superImposeRight;
Mat superImposeLeft;
Mat imageRight;
Mat imageLeft;
Mat imageOut;
Mat backgroundR;
Mat backgroundL;
Mat backgroundTemp;
Mat imageClockL;
Mat imageClockR;
Mat imageClockTemp;
Mat imagePixleBar;
Mat rotatedROIImage;

VideoCapture cap;
VideoCapture cap2;
double alpha = .5;
int width = 640;
int height = 960;
Rect cropBorder(0, 0, 640, 480);
Rect cropText1(0, 5, 640, 480);
Rect cropText2(10, 5, 640, 480);

// image points
Point org(10, 100);
Point orgTextL(84, 480);
Point orgTextR(65, 480);
int rightDis = 19;

Point orgRectL1(40, 400);
Point orgRectL2(440, 600);
Point orgRectR1(40, 400);
Point orgRectR2(440, 600);

// gyro poitns
Point org2(10, 200);
Point org3(150, 100);
Point org4(150, 200);
Point org5(10, 300);
Point org6(200, 300);
Point org7(10, 450);
Point org8(10, 350);
Point org9(200, 350);
Point org10(10, 400);
Point orgRec1(310, 230);
Point orgRec2(330, 250);
Point orgRec3(250, 170);
Point orgRec4(390, 310);

//ROI points
Point topLeft;						//used to box the art
Point bottomRight;

Point orgText(100, 300);
time_t start;
time_t endCam;
time_t stillTime;
float FPS;
char FPSString[10] = "0";

char* superImposeText = "";
const int charLen = 30;
char superImposeTempString[charLen] = "";
char* superImposeTextOut = "";
int superImposeCharLen = 0;
char superText[1];

int j = 0;
double yawData;
double pitchData;
double rollData;
char temp12String[15] = ""; //completetion #
char temp24String[15] = ""; //sitting still value

// for mode switching
char temp19String[25] = "Entering Search Mode";
char temp20String[25] = "Entering Free-Float";
char temp21String[25] = "Search Mode";
char temp22String[25] = "Free-Float Mode";
char temp23String[20] = "Calibration Mode";
char temp25String[25] = "Display Mode";
int mode = 0;
double acclx = 0.0;
double accly = 0.0;
double acclz = 0.0;
int textDisplay = 0;

double temp1 = 0; //yaw temp holder
double temp2 = 0; //pitch temp holder
double temp3 = 0; //roll temp holder
double check1 = 0; ////////////////////////
double check2 = 0; //                    //
double check3 = 0; // Movement Detection //
double check4 = 0; //                    //
double check5 = 0; //                    //
double check6 = 0; ////////////////////////
double sittingStill = 0;
int degree = 0;
int captureFrame = 3000;
int once = 0;
int mode1counter = 0;
// ROI stuff
const int SIZE1 = 20000;
const int boxamount = 7;							//the square of the number changes the amount of boxes the roi will create must be odd to have center tile
int col1[SIZE1];										//Will hold pixel values for each col and compute standard deviation.
int col2[SIZE1];
int col3[SIZE1];
void Mean(Point, int[]);
void standardDev(int[], int[]);
bool compareStat(int[], int[], int[], int[]);
void changePhotoSize(Point[], Point[], int);
void setequal(int[], int[], int[], int[]);
void changestats(Point[], Point[], int[], int[], int[], int[]);
void findblackspace();

// to read stuff txt file from phyton
HANDLE hFile, hFileDone;
FILETIME ftCreate, ftAccess, ftWrite, ftCreateDone, ftAccessDone, ftWriteDone;
SYSTEMTIME stUTC, stLocal, stUTCDone, stLocalDone;
char fileChange1 = '1';
char fileChange2 = '1';
char* fileAddress1 = "C:\\Python27\\Values.txt";
char fileTime[15] = "";
time_t searching;
int waitTime = 50;
int space = 0;
int charSpace = 0;
int tempSpace = 0;
bool returnChar = 0;
int charLeft = 0;
int waiting = 0;
int waitCycle = 0;

void rotate(cv::Mat& src, double angle, cv::Mat& dst)
{
	int len = std::max(src.cols, src.rows);
	cv::Point2f pt(len / 2., len / 2.);
	cv::Mat r = cv::getRotationMatrix2D(pt, angle, 1.0);

	cv::warpAffine(src, dst, r, cv::Size(len, len));
}

void writeFile(char* text)
{
	FILE *f = fopen("C:\\Python27\\Values.txt", "w");
	if (f == NULL)
	{
		printf("Error opening file!\n");
		exit(1);
	}

	/* print some text */
	fprintf(f, "Some text: %s\n", text);
	fclose(f);
}

char* readFile(char* fileDirectory)
{
	char buf[2048];
	char* out;
	size_t nread;
	FILE *f = fopen(fileDirectory, "r");
	int i = 0;

	if (f == NULL)
	{
		printf("Error opening file!\n");
		exit(1);
	}
	while ((nread = fread(buf, 1, sizeof buf, f)) > 0)
	{
		fwrite(buf, 1, nread, stdout);
		printf("\n");
	}
	for (i = 0; i < 1025; i++)
	{
		if (buf[i] == 'Q'&& buf[i+1] =='Z')
		{
			buf[i] = '\0';
			superImposeCharLen = i- 1;
			i = 1025;
		}
	}
	printf("%s %i\n", buf, strlen(buf));
	out = buf;
	printf("%i\n", strlen(out));
	return out;
}

void callibrate(){
	//callabration mode
	for (int i = 0; i < 6; i++)
	{
		line(imageLeft, cvPoint(0, 80 * i + 65), cvPoint(640, 80 * i + 65), Scalar(1, 255, 1), 1);  //green line
		line(imageLeft, cvPoint(80 * i + 80, 0), cvPoint(80 * i + 80, 480), Scalar(255, 1, 1), 1);  //blue line

		line(imageRight, cvPoint(0, 80 * i + 20), cvPoint(640, 80 * i + 20), Scalar(1, 255, 1), 1); //green line
		line(imageRight, cvPoint(80 * i + 80, 0), cvPoint(80 * i + 80, 480), Scalar(255, 1, 1), 1); //blue line
	}
	line(imageLeft, cvPoint(0, 225), cvPoint(640, 225), Scalar(1, 1, 255), 1);
	line(imageRight, cvPoint(0, 260), cvPoint(640, 260), Scalar(1, 1, 255), 1);
	line(imageLeft, cvPoint(320, 0), cvPoint(320, 480), Scalar(1, 1, 255), 1);
	line(imageRight, cvPoint(320, 0), cvPoint(320, 480), Scalar(1, 1, 255), 1);
	//end of callabration mode
}
void killblackspace(Point boxFinal[])
{
	int x = rotatedROIImage.cols - 1;
	int y = 0;
	int check1 = (int)rotatedROIImage.at<cv::Vec3b>(10, 10)[0];
	int check2 = (int)rotatedROIImage.at<cv::Vec3b>(10, 10)[1];
	int check3 = (int)rotatedROIImage.at<cv::Vec3b>(10, 10)[2];
	if (check1 == 0 && check2 == 0 && check3 == 0)  //black box in top corner
	{
		y = 0;
		x = rotatedROIImage.cols / 2;
		check1 = (int)rotatedROIImage.at<cv::Vec3b>(y, x)[0];
		check2 = (int)rotatedROIImage.at<cv::Vec3b>(y, x)[1];
		check3 = (int)rotatedROIImage.at<cv::Vec3b>(y, x)[2];
		while (check1 == 0 && check2 == 0 && check3 == 0 && rotatedROIImage.cols - 1 > y)
		{
			y++;
			check1 = (int)rotatedROIImage.at<cv::Vec3b>(y, x)[0];
			check2 = (int)rotatedROIImage.at<cv::Vec3b>(y, x)[1];
			check3 = (int)rotatedROIImage.at<cv::Vec3b>(y, x)[2];
		}
		boxFinal[0].x = 0;
		boxFinal[1].x = rotatedROIImage.rows;
		boxFinal[2].x = 0;
		boxFinal[3].x = rotatedROIImage.rows;
		boxFinal[0].y = y;
		boxFinal[1].y = y;
		boxFinal[2].y = rotatedROIImage.rows;
		boxFinal[3].y = rotatedROIImage.rows;
	}
	else
	{
		x = rotatedROIImage.cols - 1;
		y = rotatedROIImage.rows / 2;
		check1 = (int)rotatedROIImage.at<cv::Vec3b>(y, x)[0];
		check2 = (int)rotatedROIImage.at<cv::Vec3b>(y, x)[1];
		check3 = (int)rotatedROIImage.at<cv::Vec3b>(y, x)[2];
		while (check1 == 0 && check2 == 0 && check3 == 0 && 1 < x)
		{
			x--;
			check1 = (int)rotatedROIImage.at<cv::Vec3b>(y, x)[0];
			check2 = (int)rotatedROIImage.at<cv::Vec3b>(y, x)[1];
			check3 = (int)rotatedROIImage.at<cv::Vec3b>(y, x)[2];
		}
		boxFinal[0].x = 0;
		boxFinal[1].x = x;
		boxFinal[2].x = 0;
		boxFinal[3].x = x;
		boxFinal[0].y = 0;
		boxFinal[1].y = 0;
		boxFinal[2].y = rotatedROIImage.rows;
		boxFinal[3].y = rotatedROIImage.rows;
	}

}
void ROI()
{
	int meantemp[3];						//contain mean of the three pixel of the square being tested
	int stddevtemp[3];						//contain standard dev of the three pixel values being tested
	int meanfinal[3];						//contain mean of the three pixel of the picture
	int stddevfinal[3];						//contain std of three pixels of the picture
	int cornercount;						//calculates corners of picture 1: topleft, 2: topright, 3: bottomleft, 4: bottom right
	int yuh;								//tells me if this is the first iteration of cornercount;
	cv::Mat original;
	Vec3b intensity;
	Point boxtemp[4];						//topleft>topright>bottomleft>bottomright
	Point boxFinal[4];
	cap.open(0);
	Point center;
	Point center2;
	cornercount = 0;
	meanfinal[0] = 0;
	meanfinal[1] = 0;
	meanfinal[2] = 0;
	stddevfinal[0] = 0;
	stddevfinal[1] = 0;
	stddevfinal[2] = 0;
	yuh = 0;

	/*
	rotate(imageCamLeft, 90, rotatedROIImage);
	Rect rotateRoi(0,0,480,640);
	rotatedROIImage = rotatedROIImage(rotateRoi).clone();
	//rotatedROIImage = imageCamLeft;
	namedWindow("ROI", CV_WINDOW_AUTOSIZE);
	imshow("ROI", rotatedROIImage);
	*/
	rotatedROIImage = imageCamLeft;
		center.x = rotatedROIImage.cols / 2;
		center.y = rotatedROIImage.rows / 2;
		center2.x = rotatedROIImage.cols / 2 + 11;
		center2.y = rotatedROIImage.rows / 2 + 11;
		for (int boxcount = 0; boxcount < (int)pow(boxamount, 2); boxcount++)
		{
			if (boxcount == 0)
			{
				boxFinal[0].x = (boxamount / 2) * (rotatedROIImage.cols / boxamount);					//initialize 4 corners of the box
				boxFinal[0].y = (boxamount / 2) * (rotatedROIImage.rows / boxamount);
				boxFinal[1].x = ((boxamount / 2) * (rotatedROIImage.cols / boxamount)) + (rotatedROIImage.cols / boxamount);
				boxFinal[1].y = (boxamount / 2) * (rotatedROIImage.rows / boxamount);
				boxFinal[2].x = (boxamount / 2) * (rotatedROIImage.cols / boxamount);
				boxFinal[2].y = ((boxamount / 2) * (rotatedROIImage.rows / boxamount)) + (rotatedROIImage.rows / boxamount);
				boxFinal[3].x = ((boxamount / 2) * (rotatedROIImage.cols / boxamount)) + (rotatedROIImage.cols / boxamount);
				boxFinal[3].y = ((boxamount / 2) * (rotatedROIImage.rows / boxamount)) + (rotatedROIImage.rows / boxamount);
				boxtemp[0] = boxFinal[0];
				boxtemp[1] = boxFinal[1];
				boxtemp[2] = boxFinal[2];
				boxtemp[3] = boxFinal[3];
			}

			else if (cornercount == 1)
			{
				if (boxFinal[0].x == 0)														//failsafe checks to see if off the page
				{
					cornercount = 2;
					yuh = 0;
				}
				else
				{
					boxtemp[0].x -= (rotatedROIImage.cols / boxamount);
					boxtemp[1].x -= (rotatedROIImage.cols / boxamount);
					boxtemp[2].x -= (rotatedROIImage.cols / boxamount);
					boxtemp[3].x -= (rotatedROIImage.cols / boxamount);
				}
			}
			if (cornercount == 2)
			{
				if (boxFinal[3].x + 3 == rotatedROIImage.cols)												//failsafe checks to see if off the page
				{
					cornercount = 3;
					yuh = 0;
				}
				else
				{
					if (yuh == 0)
					{
						boxtemp[0].x = boxFinal[1].x;									//plus one to get the the next square over.
						boxtemp[1].x = boxFinal[1].x + (rotatedROIImage.cols / boxamount);
						boxtemp[2].x = boxFinal[1].x;
						boxtemp[3].x = boxFinal[1].x + (rotatedROIImage.cols / boxamount);
						yuh++;
					}
					else
					{
						boxtemp[0].x += (rotatedROIImage.cols / boxamount);
						boxtemp[1].x += (rotatedROIImage.cols / boxamount);
						boxtemp[2].x += (rotatedROIImage.cols / boxamount);
						boxtemp[3].x += (rotatedROIImage.cols / boxamount);
					}
				}

			}
			if (cornercount == 3)
			{
				if (boxFinal[0].y == 0)
				{
					cornercount = 4;
					yuh = 0;
				}
				else
				{
					if (yuh == 0)
					{
						boxtemp[0].x = boxFinal[0].x;
						boxtemp[1].x = boxFinal[1].x;
						boxtemp[2].x = boxFinal[2].x;
						boxtemp[3].x = boxFinal[3].x;
						boxtemp[0].y = boxFinal[0].y - (rotatedROIImage.rows / boxamount);
						boxtemp[1].y = boxFinal[1].y - (rotatedROIImage.rows / boxamount);
						boxtemp[2].y = boxFinal[2].y - (rotatedROIImage.rows / boxamount);
						boxtemp[3].y = boxFinal[3].y - (rotatedROIImage.rows / boxamount);
						yuh++;
					}
					else
					{
						boxtemp[0].y -= (rotatedROIImage.rows / boxamount);
						boxtemp[1].y -= (rotatedROIImage.rows / boxamount);
						boxtemp[2].y -= (rotatedROIImage.rows / boxamount);
						boxtemp[3].y -= (rotatedROIImage.rows / boxamount);
					}
				}
			}
			if (cornercount == 4)
			{
				if (boxFinal[3].y + 4 == rotatedROIImage.rows)
				{
					cornercount = 5;
					yuh = 0;
				}
				else
				{
					if (yuh == 0)
					{
						boxtemp[0].y = boxFinal[0].y + (rotatedROIImage.rows / boxamount);
						boxtemp[1].y = boxFinal[1].y + (rotatedROIImage.rows / boxamount);
						boxtemp[2].y = boxFinal[2].y + (rotatedROIImage.rows / boxamount);
						boxtemp[3].y = boxFinal[3].y + (rotatedROIImage.rows / boxamount);
						yuh++;
					}
					else
					{
						boxtemp[0].y += (rotatedROIImage.rows / boxamount);
						boxtemp[1].y += (rotatedROIImage.rows / boxamount);
						boxtemp[2].y += (rotatedROIImage.rows / boxamount);
						boxtemp[3].y += (rotatedROIImage.rows / boxamount);
					}
				}
			}
			if (cornercount == 5)
			{
				//boxcount = (int)pow(boxamount, 2);
			}
			Mean(boxtemp[0], meantemp);
			standardDev(meantemp, stddevtemp);


			if (boxcount == 0)
			{
				setequal(meantemp, meanfinal, stddevtemp, stddevfinal);
			}
			if (compareStat(meantemp, meanfinal, stddevtemp, stddevfinal))
			{
				changePhotoSize(boxtemp, boxFinal, cornercount);
				changestats(boxtemp, boxFinal, meantemp, meanfinal, stddevtemp, stddevfinal);
				setequal(meantemp, meanfinal, stddevtemp, stddevfinal);
				//		cout << "dev good " << stddevtemp[0] + stddevtemp[1] + stddevtemp[2] << " " << endl;
				//			cout << "cornercount " << cornercount << " boxcount " <<boxcount << endl;
				if (boxcount == 0)
				{
					yuh++;
					cornercount = 1;
				}

			}
			else
			{
				cornercount++;
				yuh = 0;
				//	cout << "dev bad " << stddevtemp[0] + stddevtemp[1] + stddevtemp[2] << " " << endl;
				//	cout << "cornercount " << cornercount << " boxcount " << boxcount << endl;
			}
			//cout << "mean " <<  (meantemp[0] + meantemp[1] + meantemp[2]) - (meanfinal[0] - meanfinal[1] - meanfinal[2]) << endl << endl;
		}//end of: for (int boxcount = 0; boxcount < (int)pow(boxamount, 2); boxcount++)
		
		//Real-Time RoI box
		//rectangle(rotatedROIImage, boxFinal[0], boxFinal[3], Scalar(255, 255, 0), 5);
		Rect cropRoi(boxFinal[0].x, boxFinal[0].y, boxFinal[3].x - boxFinal[0].x, boxFinal[3].y - boxFinal[0].y);
		rotatedROIImage = rotatedROIImage(cropRoi).clone();
		rotate(rotatedROIImage, 90, rotatedROIImage);

		//rotate(rotatedROIImage, 90, rotatedROIImage);
		//Rect rotateROI()
		//rotatedROIImage = rotatedROIImage(rotatedROI).clone();

		//namedWindow("ROI", CV_WINDOW_AUTOSIZE);
		//imshow("ROI", rotatedROIImage);
		killblackspace(boxFinal);
		Rect cropRoi1(boxFinal[0].x, boxFinal[0].y, boxFinal[1].x - boxFinal[0].x, boxFinal[2].y - boxFinal[0].y);
		rotatedROIImage = rotatedROIImage(cropRoi1).clone();
		imwrite("C:\\Python27\\pic.jpg", rotatedROIImage);

		//cout << "wedone" << endl << endl << endl;
		return;
}//end of main

void Mean(Point topleft, int mean[])
{
	int colorcount = 0;
	mean[0] = 0;						//reset variables
	mean[1] = 0;
	mean[2] = 0;
	for (int i = topleft.x; i < (rotatedROIImage.cols / boxamount) + topleft.x; i += 1)	//ROIbegining
	{
		int debug;
		debug = 0;
		for (int j = topleft.y; j < (rotatedROIImage.rows / boxamount) + topleft.y; j += 1)
		{
			col1[colorcount] = (int)rotatedROIImage.at<cv::Vec3b>(j, i)[0];						//add pixel values
			col2[colorcount] = (int)rotatedROIImage.at<cv::Vec3b>(j, i)[1];
			col3[colorcount] = (int)rotatedROIImage.at<cv::Vec3b>(j, i)[2];
			colorcount++;
		}//end of: for (int j = jcount; j < (rotatedROIImage.cols / boxamount) + jcount; j += 1)
	}//end of: for (int i = icount; i <= (rotatedROIImage.rows / boxamount) + icount - 1; i += 1)
	for (int k = 0; k < (rotatedROIImage.cols / boxamount) * (rotatedROIImage.rows / boxamount); k++)
	{
		mean[0] += col1[k];
		mean[1] += col2[k];
		mean[2] += col3[k];
	}
	mean[0] /= ((rotatedROIImage.cols / boxamount) * (rotatedROIImage.rows / boxamount));
	mean[1] /= ((rotatedROIImage.cols / boxamount) * (rotatedROIImage.rows / boxamount));
	mean[2] /= ((rotatedROIImage.cols / boxamount) * (rotatedROIImage.rows / boxamount));
}//end of Mean()

void standardDev(int mean[], int stddev[])
{
	stddev[0] = 0;
	stddev[1] = 0;
	stddev[2] = 0;
	for (int k = 0; k < ((rotatedROIImage.cols / boxamount) * (rotatedROIImage.rows / boxamount)); k++)
	{
		stddev[0] += pow((col1[k] - mean[0]), 2);
		stddev[1] += pow((col2[k] - mean[0]), 2);
		stddev[2] += pow((col3[k] - mean[0]), 2);
	}
	stddev[0] = sqrt(stddev[0] / ((rotatedROIImage.cols / boxamount) * (rotatedROIImage.rows / boxamount)));
	stddev[1] = sqrt(stddev[1] / ((rotatedROIImage.cols / boxamount) * (rotatedROIImage.rows / boxamount)));
	stddev[2] = sqrt(stddev[2] / ((rotatedROIImage.cols / boxamount) * (rotatedROIImage.rows / boxamount)));
}//end of StandardDev

bool compareStat(int meantemp[], int meanfinal[], int stddevtemp[], int stddevfinal[])
{
	if (abs((meantemp[0] + meantemp[1] + meantemp[2]) - (meanfinal[0] - meanfinal[1] - meanfinal[2])) >= 30)
	{
		if (stddevtemp[0] + stddevtemp[1] + stddevtemp[2] > 65)
		{
			return true;
		}
	}
	return false;
}//end of compareStat()

void changePhotoSize(Point boxTemp[], Point boxFinal[], int cornercount)
{
	if (cornercount == 0)
	{
		boxFinal[0] = boxTemp[0];
		boxFinal[1] = boxTemp[1];
		boxFinal[2] = boxTemp[2];
		boxFinal[3] = boxTemp[3];
		cornercount++;
	}
	else if (cornercount == 1)
	{
		boxFinal[0].x = boxTemp[0].x;
		boxFinal[2].x = boxTemp[2].x;
	}
	else if (cornercount == 2)
	{
		boxFinal[1].x = boxTemp[1].x;
		boxFinal[3].x = boxTemp[3].x;
	}
	else if (cornercount == 3)
	{
		boxFinal[0].y = boxTemp[0].y;
		boxFinal[1].y = boxTemp[1].y;
	}
	else if (cornercount == 4)
	{
		boxFinal[2].y = boxTemp[2].y;
		boxFinal[3].y = boxTemp[3].y;
	}
}//end of changePhotoSize

void setequal(int meantemp[], int meanfinal[], int stddevtemp[], int stddevfinal[])
{
	meanfinal[0] = meantemp[0];
	meanfinal[1] = meantemp[1];
	meanfinal[2] = meantemp[2];
	stddevfinal[0] = stddevtemp[0];
	stddevfinal[1] = stddevtemp[1];
	stddevfinal[2] = stddevtemp[2];
}

void changestats(Point temp[], Point Final[], int meantemp[], int meanfinal[], int stdtemp[], int stdfinal[])
{
	int areaoftemp = (temp[3].x - temp[0].x) * (temp[4].y - temp[0].y);				//compute area to fin ratios
	int areaoffinal = (Final[3].x - Final[0].x) * (Final[4].y - Final[0].y);

	meanfinal[0] = (areaoftemp / areaoffinal) * meantemp[0] + (1 - (areaoftemp / areaoffinal)) * meanfinal[0];
	meanfinal[1] = (areaoftemp / areaoffinal) * meantemp[1] + (1 - (areaoftemp / areaoffinal)) * meanfinal[1];
	meanfinal[2] = (areaoftemp / areaoffinal) * meantemp[2] + (1 - (areaoftemp / areaoffinal)) * meanfinal[2];
	stdfinal[0] = (areaoftemp / areaoffinal) * stdtemp[0] + (1 - (areaoftemp / areaoffinal)) * stdfinal[0];
	stdfinal[1] = (areaoftemp / areaoffinal) * stdtemp[1] + (1 - (areaoftemp / areaoffinal)) * stdfinal[1];
	stdfinal[2] = (areaoftemp / areaoffinal) * stdtemp[2] + (1 - (areaoftemp / areaoffinal)) * stdfinal[2];
}
void findblackspace()
{
}

void inita(){
	superImposeRight = imread("..\\source\\super image test.png");
	superImposeLeft = imread("..\\source\\super image test.png");
	backgroundTemp = imread("..\\source\\background.png");
	backgroundR = imread("..\\source\\background3.png");
	backgroundL = imread("..\\source\\background3.png");
	imageClockL = imread("..\\source\\background.png");
	imageClockR = imread("..\\source\\background.png");
	imageClockTemp = imread("..\\source\\background2.png");
	imagePixleBar = imread("..\\source\\pixle bar.png");


	/*
	//initilize file to get current time of file
	//create file
	hFile = CreateFile(L"C:\\Python27\\Values.txt", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	// get attributes of file
	GetFileTime(hFile, &ftCreate, &ftAccess, &ftWrite);
	//convert time to local time
	FileTimeToSystemTime(&ftWrite, &stUTC);
	SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

	//initilize file to get present time
	//create file
	hFileDone = CreateFile(L"C:\\Python27\\Values.txt", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	// get attributes of file
	GetFileTime(hFileDone, &ftCreateDone, &ftAccessDone, &ftWriteDone);
	//convert time to local time
	FileTimeToSystemTime(&ftWriteDone, &stUTCDone);
	SystemTimeToTzSpecificLocalTime(NULL, &stUTCDone, &stLocalDone);
	*/

	cap.set(CV_CAP_PROP_FRAME_WIDTH, width);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, height);
	cap.open(0);
	cap2.set(CV_CAP_PROP_FRAME_WIDTH, width);
	cap2.set(CV_CAP_PROP_FRAME_HEIGHT, height);
	cap2.open(1);
}
void cam(double yaw, double pitch, double roll, double accx, double accy, double accz){
	superImposeText = "a test";

	namedWindow("original", 1);

	int i = 0;
	int fileRead = 0;

	while (i < 10)
	{
		i++;
		try
		{
			start = clock();

			//angle checks for movement
			check1 = temp1 + 2;
			check2 = temp1 - 2;
			check3 = temp2 + 2;
			check4 = temp2 - 2;
			check5 = temp3 + 2;
			check6 = temp3 - 2;

			cap >> imageCamRight;
			cap2 >> imageCamLeft;
			//imageCamLeft = imageCamRight;
			if (sittingStill > 4000){
				sittingStill = 0;
				stillTime = clock();
			}
			if (yaw > check1 || yaw < check2 || pitch > check3 || pitch < check4 || roll > check5 || roll < check6){ //checks for moving
				if (yaw < 40){
					sittingStill = 0;
					stillTime = clock();
				}
				else{
					sittingStill = (int)(clock() - stillTime);
				}
			}
			else
			{
				sittingStill = (int)(clock()-stillTime);
			}
			sprintf(temp24String, "%2.2f", sittingStill); //sitting still text being updated
			// to display sitting still timer
			//putText(imageClockR, temp24String, cvPoint(100,300), CV_FONT_HERSHEY_COMPLEX, 1, Scalar(1, 1, 225), 1, 8); //sitting still value
			if (mode == 1){
				if (sittingStill >= captureFrame && !(yaw > 40 && acclz > 1) && mode1counter == 0){
					//imwrite("..\\source\\imagecap.png", imageCamLeft);
					//
					writeFile("this is a test");
					ROI();
					//system("cd C:\\Python27\\ & python Artist.py");
					//system("cd C:\\Python27\\");
					//ShellExecute(NULL, L"open", L"cmd.exe", L"cd C:\\Python27\\ & python Artist.py", NULL, SW_SHOW);
					mode = 3;
					mode1counter = 1;
					searching = clock();
				}
			}

			imageRight = imageCamRight;
			imageLeft = imageCamLeft;

			//to display fps
			//putText(imageCamLeft, FPSString, org, CV_FONT_HERSHEY_COMPLEX, 1, Scalar(1, 1, 225), 1, 8);
			
			//for mode switching
			if (mode == 0){
				putText(imageLeft, temp23String, cvPoint(10, 450), CV_FONT_HERSHEY_COMPLEX, 1, Scalar(1, 1, 225), 1, 8); //Calibration mode
				callibrate();
			}
			if (yaw > 40 && acclz > 1){
				if (mode != 1){
					putText(imageLeft, temp19String, org10, CV_FONT_HERSHEY_COMPLEX, 1, Scalar(1, 1, 225), 1, 8); //Entering Search Mode
				}
				if (sittingStill > 2000){
					sittingStill = 0;
					mode = 1;
					mode1counter = 0;
				}
			}
			if (mode == 1){

				//crosshair
				line(imageLeft, cvPoint(320, 225), cvPoint(320, 255), Scalar(0, 255, 0), 3);
				line(imageLeft, cvPoint(305, 240), cvPoint(335, 240), Scalar(0, 255, 0), 3);

				if (sittingStill > 1 && sittingStill < captureFrame && yaw < 40){
					degree = (sittingStill / captureFrame) * 360;
					sprintf(temp12String, "%2.0f", sittingStill / captureFrame * 100);
					ellipse(imageClockL, cvPoint(320, 230), cvSize(60, 60), 0, 0, degree, CV_RGB(1, 1, 225), 3, 8);
					ellipse(imageClockL, cvPoint(320, 230), cvSize(50, 50), 0, 0, 360, CV_RGB(1, 1, 225), 1, 8);
					ellipse(imageClockL, cvPoint(320, 230), cvSize(70, 70), 0, 0, 360, CV_RGB(1, 1, 225), 1, 8);
					putText(imageClockL, temp12String, cvPoint(300, 150), CV_FONT_HERSHEY_COMPLEX, 1, Scalar(1, 1, 225), 1, 8); //% of completion

					ellipse(imageClockR, cvPoint(320, 240 + rightDis), cvSize(60, 60), 0, 0, degree, CV_RGB(1, 1, 225), 3, 8);
					ellipse(imageClockR, cvPoint(320, 240 + rightDis), cvSize(50, 50), 0, 0, 360, CV_RGB(1, 1, 225), 1, 8);
					ellipse(imageClockR, cvPoint(320, 240 + rightDis), cvSize(70, 70), 0, 0, 360, CV_RGB(1, 1, 225), 1, 8);
					putText(imageClockR, temp12String, cvPoint(300, 150 + rightDis), CV_FONT_HERSHEY_COMPLEX, 1, Scalar(1, 1, 225), 1, 8); //% of completion
				}
			}
			if (mode == 1){
				putText(imageLeft, temp21String, cvPoint(10, 450), CV_FONT_HERSHEY_COMPLEX, 1, Scalar(1, 1, 225), 1, 8); //Search Mode
			}
			if (yaw > 40 && acclz < -1){
				if (mode != 2){
					putText(imageLeft, temp20String, org10, CV_FONT_HERSHEY_COMPLEX, 1, Scalar(1, 1, 225), 1, 8); //Entering Free Float Mode
				}
				if (sittingStill > 2000){
					sittingStill = 0;
					mode = 2;
				}
			}
			if (mode == 2){
				putText(imageLeft, temp22String, cvPoint(10, 450), CV_FONT_HERSHEY_COMPLEX, 1, Scalar(1, 1, 225), 1, 8); //Free Float Mode
			}
			if (mode == 3){
				//wait until return from web
				putText(imageLeft, temp25String, cvPoint(10, 450), CV_FONT_HERSHEY_COMPLEX, 1, Scalar(1, 1, 225), 1, 8); //Display mode
				
				if ((clock() - searching) > waitTime || fileRead == 1){
					waitTime = 300;
					searching = clock();
					if (waitCycle % 10 == 0){
						if (textDisplay == 0){
							superImposeText = readFile(fileAddress1);
							fileChange1 = superImposeText[0];
							fileChange2 = superImposeText[1];
						}
					}
					waitCycle++;

					if (fileChange1 != 'Z' && fileChange2 != 'Q' && textDisplay == 0){
						if (fileRead == 0){
							fileRead = 1;
							if (waiting % 4 == 0){
								sprintf(fileTime, "waiting");
							}
							else if (waiting % 4 == 1){
								sprintf(fileTime, "waiting.");
							}
							else if (waiting % 4 == 2){
								sprintf(fileTime, "waiting..");
							}
							else{
								sprintf(fileTime, "waiting...");
							}
							
							waiting++;

							backgroundR = imread("..\\source\\background3.png");
							backgroundL = imread("..\\source\\background3.png");
							putText(backgroundL, fileTime, cvPoint(164, 400), CV_FONT_HERSHEY_COMPLEX, .75, Scalar(255, 255, 255), 1, 4);
							putText(backgroundR, fileTime, cvPoint(184, 400), CV_FONT_HERSHEY_COMPLEX, .75, Scalar(255, 255, 255), 1, 4);

							rotate(backgroundL, 270, backgroundL);
							rotate(backgroundR, 270, backgroundR);
							backgroundL = backgroundL(cropBorder).clone();
							backgroundR = backgroundR(cropBorder).clone();
						}

					}
					else if (textDisplay == 0){
						waitCycle = 0;
						waiting = 0;
						textDisplay = 1;
						// for superimpose text
						backgroundR = imread("..\\source\\background3.png");
						backgroundL = imread("..\\source\\background3.png");
						superImposeText = readFile(fileAddress1);
						space = 0;

						for (int i = 0; i < (int)(superImposeCharLen / charLen) + 1; i++){
							returnChar = 0;
							for (int j = 0; j < charLen; j++){
								if (!(superImposeText[space] == 'Q' && superImposeText[space + 1] == 'Q')){
									space++;
								}
								if (superImposeText[space] == ' ' && !(superImposeText[space] == 'Q' && superImposeText[space + 1] == 'Q')){
									charSpace = space;
								}
								if(i == 0 && j<1){
									superImposeTempString[j] = ' ';
								}
								else if (superImposeText[space] == 'Q' && superImposeText[space + 1] == 'Q'){
									charSpace = space + 2;
									superImposeTempString[j] = ' ';
								}
								else{
									//superImposeTempString[j] = superImposeText[(i)*charLen + j];
									superImposeTempString[j] = superImposeText[space];
								}
								//if (j == charLen - 1 && (superImposeText[space] != ' ' || superImposeText[space + 1] != ' ')){
								if ((j == charLen - 1 && returnChar == 0) && !(superImposeText[space] == 'Q' && superImposeText[space + 1] == 'Q')){
									tempSpace = charSpace;
									for (charSpace; charSpace <= space; charSpace++){
										superImposeTempString[charLen - (space - charSpace)] = ' ';
									}
									space = tempSpace - 1;
								}
								else if ((j == charLen - 1 && returnChar == 0) && (superImposeText[space] == 'Q' && superImposeText[space + 1] == 'Q')){
									space += 1;
								}
							}
							//superImposeTextOut = superImposeTempString;
							putText(backgroundL, superImposeTempString, cvPoint(84, 250 + 25 * i), CV_FONT_HERSHEY_COMPLEX, .65, Scalar(255, 255, 255), 1, 4);
							putText(backgroundR, superImposeTempString, cvPoint(64, 250 + 25 * i), CV_FONT_HERSHEY_COMPLEX, .65, Scalar(255, 255, 255), 1, 4);
							superImposeText = readFile(fileAddress1);
						}

						//putText(backgroundL, superText, orgTextL, CV_FONT_HERSHEY_COMPLEX, 1, Scalar(255, 255, 255), 1, 8);
						//putText(backgroundR, superText, orgTextR, CV_FONT_HERSHEY_COMPLEX, 1, Scalar(255, 255, 255), 1, 8);

						rotate(backgroundL, 270, backgroundL);
						rotate(backgroundR, 270, backgroundR);
						backgroundL = backgroundL(cropBorder).clone();
						backgroundR = backgroundR(cropBorder).clone();
						//end of superimpose text

					}
				}
				else if (yaw > 20 && yaw < 35 && acclz > 1){ //scroll down 
					copyMakeBorder(backgroundR, backgroundR, 5, 5, 5, 5, BORDER_CONSTANT, Scalar(0, 0, 0));
					copyMakeBorder(backgroundL, backgroundL, 5, 5, 5, 5, BORDER_CONSTANT, Scalar(0, 0, 0));
					backgroundL = backgroundL(cropText2).clone();
					backgroundR = backgroundR(cropText2).clone();
				}
				else if (yaw > 20 && yaw < 35 && acclz < -1){ //scroll up
					copyMakeBorder(backgroundR, backgroundR, 5, 5, 5, 5, BORDER_CONSTANT, Scalar(0, 0, 0));
					copyMakeBorder(backgroundL, backgroundL, 5, 5, 5, 5, BORDER_CONSTANT, Scalar(0, 0, 0));
					backgroundL = backgroundL(cropText1).clone();
					backgroundR = backgroundR(cropText1).clone();
				}
				add(imageRight, backgroundR, imageRight);
				add(imageLeft, backgroundL, imageLeft);
			}
			else if (textDisplay != 0){
				textDisplay = 0;
				backgroundR = backgroundTemp;
				backgroundL = backgroundTemp;
			}
			//end of mode switching code

			add(imageLeft, imageClockR, imageLeft);
			add(imageRight, imageClockL, imageRight);


			//Real-time RoI
			//ROI();

			imageLeft.push_back(imageRight);


			imshow("original", imageLeft);

			//reset timer picture
			add(imageClockR, imageClockTemp, imageClockR);
			bitwise_not(imageClockR, imageClockR);
			add(imageClockL, imageClockTemp, imageClockL);
			bitwise_not(imageClockL, imageClockL);
		}
		catch (Exception& e)
		{
			const char* err_msg = e.what();
			std::cout << "exception caught: imshow:\n" << err_msg << std::endl;
		}
		waitKey(1); //orig = 33
		endCam = clock();
		//for text
		FPS = CLOCKS_PER_SEC / ((float)(endCam - start));

		sprintf(FPSString, "FPS = %f",FPS);
	}
	temp1 = yaw;
	temp2 = pitch;
	temp3 = roll;
}
// Camera view types.

enum ViewType
{
	View_Perspective,
	View_XZ_UpY,
	View_XY_DownZ,
	View_Count
};


class InputTestApp : public Application
{
	RenderDevice*      pRender;

	Ptr<DeviceManager> pManager;
	Ptr<HMDDevice>     pHMD;
	Ptr<SensorDevice>  pSensor;
	Ptr<SensorDevice>  pSensor2;

	SensorFusion       SFusion;
	SensorFusion       SFusion2;

	double          LastUpdate;
	ViewType        CurrentView;

	double          LastTitleUpdate;

public:

	InputTestApp();
	~InputTestApp();

	virtual int  OnStartup(int argc, const char** argv);
	virtual void OnIdle();

};

InputTestApp::InputTestApp()
: pRender(0), CurrentView(View_Perspective),
LastUpdate(0), LastTitleUpdate(0)
{

}


InputTestApp::~InputTestApp()
{
	pSensor.Clear();
	pManager.Clear();

}


int InputTestApp::OnStartup(int argc, const char** argv)
{
	pManager = *DeviceManager::Create();

	// This initialization logic supports running two sensors at the same time.

	DeviceEnumerator<SensorDevice> isensor = pManager->EnumerateDevices<SensorDevice>();
	DeviceEnumerator<SensorDevice> oculusSensor;
	DeviceEnumerator<SensorDevice> oculusSensor2;

	while (isensor)
	{
		DeviceInfo di;
		if (isensor.GetDeviceInfo(&di))
		{
			if (strstr(di.ProductName, "Tracker"))
			{
				if (!oculusSensor)
					oculusSensor = isensor;
				else if (!oculusSensor2)
					oculusSensor2 = isensor;
			}
		}

		isensor.Next();
	}

	if (oculusSensor)
	{
		pSensor = *oculusSensor.CreateDevice();

		if (pSensor)
			pSensor->SetRange(SensorRange(4 * 9.81f, 8 * Math<float>::Pi, 1.0f), true);

		if (oculusSensor2)
		{
			// Second Oculus sensor, useful for comparing firmware behavior & settings.
			pSensor2 = *oculusSensor2.CreateDevice();

			if (pSensor2)
				pSensor2->SetRange(SensorRange(4 * 9.81f, 8 * Math<float>::Pi, 1.0f), true);
		}
	}

	oculusSensor.Clear();
	oculusSensor2.Clear();

	if (pSensor)
		SFusion.AttachToSensor(pSensor);
	if (pSensor2)
		SFusion2.AttachToSensor(pSensor2);

	LastUpdate = pPlatform->GetAppTime();

	return 0;
}

static float CalcDownAngleDegrees(Quatf q)
{
	Vector3f downVector(0.0f, -1.0f, 0.0f);
	Vector3f val = q.Rotate(downVector);
	return RadToDegree(downVector.Angle(val));
}

static float CalcDownAngleDegrees2(Quatf q)
{
	Vector3f downVector(-1.0f, 0.0f, 0.0f);
	Vector3f val = q.Rotate(downVector);
	return RadToDegree(downVector.Angle(val));
}
static float CalcDownAngleDegrees3(Quatf q)
{
	Vector3f downVector(0.0f, 0.0f, -1.0f);
	Vector3f val = q.Rotate(downVector);
	return RadToDegree(downVector.Angle(val));
}

void InputTestApp::OnIdle()
{
	if (j == 0){
		inita();
		j= 1;
	}
	double curtime = pPlatform->GetAppTime();
	//   float  dt      = float(LastUpdate - curtime);
	LastUpdate = curtime;

	OVR::Vector3f acceldata = SFusion.GetAcceleration();

	//ofstream outfile;
	//outfile.open("outfile.txt");

	// Quatf q = SFusion.GetOrientation();

	// Update titlebar every 20th of a second.

	//outfile << acceldata.x << "," << acceldata.y << "," << acceldata.z << "\n";
	//outfile.close();

	yawData = CalcDownAngleDegrees(SFusion.GetOrientation());
	pitchData = CalcDownAngleDegrees2(SFusion.GetOrientation());
	rollData = CalcDownAngleDegrees3(SFusion.GetOrientation());
	acclx = acceldata.x;
	accly = acceldata.y;
	acclz = acceldata.z;
	cam(yawData, pitchData, rollData,acclx,accly,acclz);
	//test();
}

OVR_PLATFORM_APP(InputTestApp);
