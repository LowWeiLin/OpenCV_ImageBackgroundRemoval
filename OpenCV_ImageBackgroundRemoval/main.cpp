#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include "opencv\cv.h"
 
using namespace cv;
using namespace std;


Mat sobel(Mat gray);
Mat canny(Mat src);


int main()
{
	//Load source image	
	//Mat src = imread("human-t-pose.jpg");
	//Mat src = imread("person1.jpg");
	//Mat src = imread("person2.jpg");
	Mat src = imread("shoe.jpg");

	//windows
	/*
	namedWindow("src", WINDOW_NORMAL);
	namedWindow("1. \"Remove Shadows\"", WINDOW_NORMAL);
	namedWindow("2. Grayscale", WINDOW_NORMAL);
	namedWindow("3. Edge Detector", WINDOW_NORMAL);
	namedWindow("4. Dilate", WINDOW_NORMAL);
	namedWindow("5. Floodfill", WINDOW_NORMAL);
	namedWindow("6. Erode", WINDOW_NORMAL);
	namedWindow("7. Largest Contour", WINDOW_NORMAL);
	namedWindow("8. Masked Source", WINDOW_NORMAL);
	namedWindow("src boxed", WINDOW_NORMAL);
	*/

	//0. Source Image
	imshow("src", src);

	//1. Remove Shadows
	//Convert to HSV
	Mat hsvImg;
	cvtColor(src, hsvImg, CV_BGR2HSV);
	Mat channel[3];
	split(hsvImg, channel);
	channel[2] = Mat(hsvImg.rows, hsvImg.cols, CV_8UC1, 200);//Set V
	//Merge channels
	merge(channel, 3, hsvImg);
	Mat rgbImg;
	cvtColor(hsvImg, rgbImg, CV_HSV2BGR);
	imshow("1. \"Remove Shadows\"", rgbImg);

	//2. Convert to gray and normalize
	Mat gray(rgbImg.rows, src.cols, CV_8UC1);
	cvtColor(rgbImg, gray, CV_BGR2GRAY);
	normalize(gray, gray, 0, 255, NORM_MINMAX, CV_8UC1);
	imshow("2. Grayscale", gray);

	//3. Edge detector
	GaussianBlur(gray, gray, Size(3,3), 0, 0, BORDER_DEFAULT);
	Mat edges;
	bool useCanny = false;
	if(useCanny){
		edges = canny(gray);
	} else {
		//Use Sobel filter and thresholding.
		edges = sobel(gray);
		//Automatic thresholding
		//threshold(edges, edges, 0, 255, cv::THRESH_OTSU);
		//Manual thresholding
		threshold(edges, edges, 25, 255, cv::THRESH_BINARY);
	}

	imshow("3. Edge Detector", edges);

	//4. Dilate
	Mat dilateGrad = edges;
	int dilateType = MORPH_ELLIPSE;
	int dilateSize = 3;
	Mat elementDilate = getStructuringElement(dilateType,
		Size(2*dilateSize + 1, 2*dilateSize+1),
		Point(dilateSize, dilateSize));
	dilate(edges, dilateGrad, elementDilate);
	imshow("4. Dilate", dilateGrad);

	//5. Floodfill
	Mat floodFilled = cv::Mat::zeros(dilateGrad.rows+2, dilateGrad.cols+2, CV_8U);
	floodFill(dilateGrad, floodFilled, cv::Point(0, 0), 0, 0, cv::Scalar(), cv::Scalar(), 4 + (255 << 8) + cv::FLOODFILL_MASK_ONLY);
	floodFilled = cv::Scalar::all(255) - floodFilled;
	Mat temp;
	floodFilled(Rect(1, 1, dilateGrad.cols-2, dilateGrad.rows-2)).copyTo(temp);
	floodFilled = temp;
	imshow("5. Floodfill", floodFilled);

	//6. Erode
	int erosionType = MORPH_ELLIPSE;
	int erosionSize = 4;
	Mat erosionElement = getStructuringElement(erosionType,
		Size(2*erosionSize+1, 2*erosionSize+1),
		Point(erosionSize, erosionSize));
	erode(floodFilled, floodFilled, erosionElement);
	imshow("6. Erode", floodFilled);

	//7. Find largest contour
	int largestArea = 0;
	int largestContourIndex = 0;
	Rect boundingRectangle;
	Mat largestContour(src.rows, src.cols, CV_8UC1, Scalar::all(0));
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
 
	findContours(floodFilled, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
   
	for(int i=0; i<contours.size(); i++)
	{
		double a = contourArea(contours[i], false);
		if(a > largestArea)
		{
			largestArea = a;
			largestContourIndex = i;
			boundingRectangle = boundingRect(contours[i]);
		}
	}
 
	Scalar color(255, 255, 255);
	drawContours(largestContour, contours, largestContourIndex, color, CV_FILLED, 8, hierarchy); //Draw the largest contour using previously stored index.
	rectangle(src, boundingRectangle, Scalar(0, 255, 0), 1, 8, 0);
	imshow("7. Largest Contour", largestContour);

	//8. Mask original image
	Mat maskedSrc;
	src.copyTo(maskedSrc, largestContour);
	imshow("8. Masked Source", maskedSrc);

	//Source with largest contour boxed
	imshow("src boxed", src);


	waitKey(0);
}

Mat sobel(Mat gray){
	Mat edges;

	int scale = 1;
	int delta = 0;
	int ddepth = CV_16S;
	Mat edges_x, edges_y;
	Mat abs_edges_x, abs_edges_y;
	Sobel(gray, edges_x, ddepth, 1, 0, 3, scale, delta, BORDER_DEFAULT);
	convertScaleAbs( edges_x, abs_edges_x );
	Sobel(gray, edges_y, ddepth, 0, 1, 3, scale, delta, BORDER_DEFAULT);
	convertScaleAbs(edges_y, abs_edges_y);
	addWeighted(abs_edges_x, 0.5, abs_edges_y, 0.5, 0, edges);

	return edges;
}

Mat canny(Mat src)
{
	Mat detected_edges;

	int edgeThresh = 1;
	int lowThreshold = 250;
	int highThreshold = 750;
	int kernel_size = 5;
	Canny(src, detected_edges, lowThreshold, highThreshold, kernel_size);

	return detected_edges;
 }