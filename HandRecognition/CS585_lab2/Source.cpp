/*	CS585_Lab2.cpp
*	CS585 Image and Video Computing Fall 2014
*	
*	used for:
*	CS440 Lab: Introduction to OpenCV
*	--------------
*	This program introduces the following concepts:
*		a) Reading a stream of images from a webcamera, and displaying the video
*		b) Skin color detection
*		c) Background differencing
*		d) Visualizing motion history
*	--------------
*/

//opencv libraries
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
//C++ standard libraries
#include <iostream>
#include <vector>

using namespace cv;
using namespace std;

//function declarations

/**
Function that returns the maximum of 3 integers
@param a first integer
@param b second integer
@param c third integer
*/
int myMax(int a, int b, int c);

/**
Function that returns the minimum of 3 integers
@param a first integer
@param b second integer
@param c third integer
*/
int myMin(int a, int b, int c);

/**
Function that detects whether a pixel belongs to the skin based on RGB values
@param src The source color image
@param dst The destination grayscale image where skin pixels are colored white and the rest are colored black
*/
void mySkinDetect(Mat& src, Mat& dst);

/**
Function that does frame differencing between the current frame and the previous frame
@param src The current color image
@param prev The previous color image
@param dst The destination grayscale image where pixels are colored white if the corresponding pixel intensities in the current
and previous image are not the same
*/
void myFrameDifferencing(Mat& prev, Mat& curr, Mat& dst);

/**
Function that accumulates the frame differences for a certain number of pairs of frames
@param mh Vector of frame difference images
@param dst The destination grayscale image to store the accumulation of the frame difference images
*/
void myMotionEnergy(vector<Mat> mh, Mat& dst);

void MatchingMethod(Mat& src,Mat& img, Mat&templ,Mat&templ2);

int main()
{

	//----------------
	//a) Reading a stream of images from a webcamera, and displaying the video
	//----------------
	// For more information on reading and writing video: http://docs.opencv.org/modules/highgui/doc/reading_and_writing_images_and_video.html
	// open the video camera no. 0
	VideoCapture cap(0);
	Mat image; Mat image2;
	image = imread("handright.jpg", CV_LOAD_IMAGE_COLOR);
	image2 = imread("quantou.jpg", CV_LOAD_IMAGE_COLOR);
	
	// if not successful, exit program
	if (!cap.isOpened())
	{
		cout << "Cannot open the video cam" << endl;
		return -1;
	}
	



	//create a window called "MyVideoFrame0"
	namedWindow("MyVideo0", WINDOW_AUTOSIZE);
	Mat frame0;

	// read a new frame from video
	bool bSuccess0 = cap.read(frame0);

	//if not successful, break loop
	if (!bSuccess0)
	{
		cout << "Cannot read a frame from video stream" << endl;
	}

	//show the frame in "MyVideo" window
	imshow("MyVideo0", frame0);

	//create a window called "MyVideo"
	namedWindow("MyVideo", WINDOW_AUTOSIZE);
	namedWindow("MyVideoMH", WINDOW_AUTOSIZE);
	namedWindow("Skin", WINDOW_AUTOSIZE); 
	char* image_window = "Source Image";
	namedWindow(image_window, WINDOW_AUTOSIZE);	
	
	vector<Mat> myMotionHistory;
	Mat fMH1, fMH2, fMH3;
	fMH1 = Mat::zeros(frame0.rows, frame0.cols, CV_8UC1);
	fMH2 = fMH1.clone();
	fMH3 = fMH1.clone();
	myMotionHistory.push_back(fMH1);
	myMotionHistory.push_back(fMH2);
	myMotionHistory.push_back(fMH3);

	while (1)
	{
		// read a new frame from video
		Mat frame;
		bool bSuccess = cap.read(frame);

		//if not successful, break loop
		if (!bSuccess)
		{	
			cout << "Cannot read a frame from video stream" << endl;
			break;
		}

		// destination frame
		Mat frameDest; Mat img; Mat templ; Mat col; Mat ski; Mat templ2; Mat frame1;
		frameDest = Mat::zeros(frame.rows, frame.cols, CV_8UC1);//Returns a zero array of same size as src mat, and of type CV_8UC1
		img = Mat::zeros(frame.rows, frame.cols, CV_8UC1);
		cvtColor(frame, img, COLOR_BGR2GRAY);
		templ = Mat::zeros(image.rows, image.cols, CV_8UC1);
		cvtColor(image, templ, COLOR_BGR2GRAY);
		templ2 = Mat::zeros(image2.rows, image2.cols, CV_8UC1);
		cvtColor(image2, templ2, COLOR_BGR2GRAY);
		//----------------
		//	b) Skin color detection
		//----------------
		mySkinDetect(frame, frameDest);
		imshow("Skin", frameDest);
		ski = frameDest.clone();
		frame1 = frame.clone();
	

		//----------------
		//	c) Background differencing
		//----------------

		//call myFrameDifferencing function
		myFrameDifferencing(frame0, frame, frameDest);
		imshow("MyVideo", frameDest);
		myMotionHistory.erase(myMotionHistory.begin());
		myMotionHistory.push_back(frameDest);
		Mat myMH = Mat::zeros(frame0.rows, frame0.cols, CV_8UC1);

		//----------------
		//  d) Visualizing motion history
		//----------------


		myMotionEnergy(myMotionHistory, myMH);
		int movement_threshold = 250;

		
		
		for (int i = 0; i < myMH.rows; i++){
			int count = 0;
			for (int j = 0; j < myMH.cols; j++) {
				if (myMH.at<uchar>(i, j) == 255)
					count += 1;
			}
			if (count > movement_threshold) {
				
				putText(frame1, "Wave horizontally", Point(25, 450), CV_FONT_NORMAL, 2, Scalar(255,255,255));
				break;
			}
		}
		
		
		MatchingMethod(frame1,ski,templ,templ2);
		



		imshow("MyVideoMH", myMH); //show the frame in "MyVideo" window
		frame0 = frame;
		//wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
		if (waitKey(30) == 27)
		{
			cout << "esc key is pressed by user" << endl;
			break;
		}

	}
	cap.release();
	return 0;
}

//Function that returns the maximum of 3 integers
int myMax(int a, int b, int c) {
	int m = a;
	(void)((m < b) && (m = b));
	(void)((m < c) && (m = c));
	return m;
}

//Function that returns the minimum of 3 integers
int myMin(int a, int b, int c) {
	int m = a;
	(void)((m > b) && (m = b));
	(void)((m > c) && (m = c));
	return m;
}

//Function that detects whether a pixel belongs to the skin based on RGB values
void mySkinDetect(Mat& src, Mat& dst) {
	//Surveys of skin color modeling and detection techniques:
	//Vezhnevets, Vladimir, Vassili Sazonov, and Alla Andreeva. "A survey on pixel-based skin color detection techniques." Proc. Graphicon. Vol. 3. 2003.
	//Kakumanu, Praveen, Sokratis Makrogiannis, and Nikolaos Bourbakis. "A survey of skin-color modeling and detection methods." Pattern recognition 40.3 (2007): 1106-1122.
	for (int i = 0; i < src.rows; i++){
		for (int j = 0; j < src.cols; j++){
			//For each pixel, compute the average intensity of the 3 color channels
			Vec3b intensity = src.at<Vec3b>(i, j); //Vec3b is a vector of 3 uchar (unsigned character)
			int B = intensity[0]; int G = intensity[1]; int R = intensity[2];
			if ((R > 95 && G > 40 && B > 20) && (myMax(R, G, B) - myMin(R, G, B) > 15) && (abs(R - G) > 15) && (R > G) && (R > B)){
				dst.at<uchar>(i, j) = 255;
			}
		}
	}
}


void myFrameDifferencing(Mat& prev, Mat& curr, Mat& dst) {
	//For more information on operation with arrays: http://docs.opencv.org/modules/core/doc/operations_on_arrays.html
	//For more information on how to use background subtraction methods: http://docs.opencv.org/trunk/doc/tutorials/video/background_subtraction/background_subtraction.html
	absdiff(prev, curr, dst);
	Mat gs = dst.clone();
	cvtColor(dst, gs, CV_BGR2GRAY);
	dst = gs > 50;
	Vec3b intensity = dst.at<Vec3b>(100, 100);
}


void myMotionEnergy(vector<Mat> mh, Mat& dst) {
	Mat mh0 = mh[0];
	Mat mh1 = mh[1];
	Mat mh2 = mh[2];

	for (int i = 0; i < dst.rows; i++){
		for (int j = 0; j < dst.cols; j++){
			if (mh0.at<uchar>(i, j) == 255 || mh1.at<uchar>(i, j) == 255 || mh2.at<uchar>(i, j) == 255){
				dst.at<uchar>(i, j) = 255;
			}
		}
	}
	
}

//
void MatchingMethod(Mat& src, Mat& img, Mat& templ,Mat& templ2)
{
	Mat result; Mat result2;
	char* image_window = "Source Image";
	char* result_window = "Result window";


	Mat img_display;
	img_display = img.clone();

	
	int result_cols = img.cols - templ.cols + 1;
	int result_rows = img.rows - templ.rows + 1;
	int result_cols2 = img.cols - templ2.cols + 1;
	int result_rows2 = img.rows - templ2.rows + 1;


	result.create(result_rows, result_cols, CV_32FC1);
	result.create(result_rows2, result_cols2, CV_32FC1);


	matchTemplate(img, templ, result, CV_TM_CCORR_NORMED);
	matchTemplate(img_display, templ2, result2, CV_TM_CCORR_NORMED);
	


	double minVal; double maxVal; Point minLoc; Point maxLoc;
	double minVal2; double maxVal2; Point minLoc2; Point maxLoc2;
	string text = "It's a High Five!"; string text2 = "It's a FIST!";
	int fontFace = FONT_HERSHEY_SIMPLEX;
	double fontScale = 1;
	int thickness = 2; int baseline = 0;
	Size textSize = getTextSize(text, fontFace,fontScale, thickness, &baseline);
	Point textOrg(10,25);

	Point matchLoc;

	minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());
	minMaxLoc(result2, &minVal2, &maxVal2, &minLoc2, &maxLoc2, Mat());

	
	
	
	if (maxVal >0.715&&maxVal>maxVal2){
		matchLoc = maxLoc;
		rectangle(src, matchLoc, Point(matchLoc.x + templ.cols, matchLoc.y + templ.rows), Scalar::all(0), 2, 8, 0);
		rectangle(result, matchLoc, Point(matchLoc.x + templ.cols, matchLoc.y + templ.rows), Scalar::all(0), 2, 8, 0);
		putText(src, text, textOrg, fontFace, fontScale,Scalar::all(0), thickness, 8);
	}
	else if (maxVal2 > 0.715&&maxVal < maxVal2){
		matchLoc = maxLoc2;
		rectangle(src, matchLoc, Point(matchLoc.x + templ.cols, matchLoc.y + templ.rows), Scalar::all(0), 2, 8, 0);
		rectangle(result, matchLoc, Point(matchLoc.x + templ.cols, matchLoc.y + templ.rows), Scalar::all(0), 2, 8, 0);
		putText(src, text2, textOrg, fontFace, fontScale, Scalar::all(0), thickness, 8);
	}
	imshow(image_window, src);
	
	


	return;
}


