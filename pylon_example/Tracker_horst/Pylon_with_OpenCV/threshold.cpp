#include "stdafx.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/video.hpp>
#include <iostream>
using namespace cv;
using namespace std;

cv::Mat GetThresholdedImage(cv::Mat img, uint8_t low_h, uint8_t high_h, uint8_t low_s, uint8_t high_s, uint8_t low_v, uint8_t high_v)
{
	cout << low_h << " | " << high_h << " _ " << low_s << " | " << high_s << " _ " << low_v << " | " << high_v  << endl;

	cv::Mat imgHSV(img.size().height, img.size().width, CV_8UC3);
	cv::cvtColor(img, imgHSV, CV_BGR2HSV); // convert to HSV

	cv::Mat img_thresh(img.size().height, img.size().width, CV_8U);
	//cv::inRange(imgHSV, Scalar(low_h, low_s, low_v), Scalar(high_h, high_s, high_v), img_thresh); // threshold!
	cv::inRange(imgHSV, Scalar(30, 30, 30), Scalar(100, 100, 100), img_thresh); // threshold!

	return img_thresh;

}
