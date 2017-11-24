#include "stdafx.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/video.hpp>

using namespace cv;

cv::Mat GetThresholdedImage(cv::Mat img, uint8_t height, uint8_t width, uint8_t low_h, uint8_t high_h, uint8_t low_s, uint8_t high_s, uint8_t low_v, uint8_t high_v)
{
	cv::Mat imgHSV(height, width, CV_8UC3);
	cv::cvtColor(img, imgHSV, CV_BGR2HSV); // convert to HSV

	cv::Mat img_thresh(height, width, CV_8U);

	cv::inRange(img, Scalar(low_h, low_s, low_v), Scalar(high_h, high_s, high_v), img_thresh); // threshold!

	return img_thresh;

}
