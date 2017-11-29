#include "stdafx.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/video.hpp>

using namespace cv;

cv::Mat* GetThresholdedImage(cv::Mat img, uint8_t height, uint8_t width)
{
	cv::Mat imgHSV(height, width, CV_8UC3);

	cv::cvtColor(img, imgHSV, CV_BGR2HSV); // convert to HSV
	cv::Mat img_thresh(height, width, CV_8U);
	//cv::inRange(img, cvScalar(20, 100, 100), cvScalar(30, 255, 255), img_thresh); // threshold!

	//cvReleaseImage(&imgHSV);
	//return imgThreshed;

}
