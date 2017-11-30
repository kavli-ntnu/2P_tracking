#include "stdafx.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/video.hpp>
#include <iostream>
using namespace cv;
using namespace std;

cv::Mat GetThresholdedImage(cv::Mat img, uint red_h_low, uint red_s_low, uint red_v_low, uint red_h_high, uint red_s_high, uint red_v_high,
	uint green_h_low, uint green_s_low, uint green_v_low, uint green_h_high, uint green_s_high, uint green_v_high)
{
	// Create an OpenCV image from a grabbed image.
	cv::Mat imgHSV(img.size(), CV_8UC3);
	cv::Mat img_thresh_green(img.size(), CV_8UC3);
	cv::Mat img_thresh_green2(img.size(), CV_8UC3);

	cv::Mat img_thresh_red(img.size(), CV_8UC3);
	cv::Mat img_thresh_red2(img.size(), CV_8UC3);

	// tracking points: 
	Mat tracking_points = Mat(2, 2, CV_64F, double(0));

	cv::cvtColor(img, imgHSV, CV_BGR2HSV); // convert to HSV

	cv::inRange(imgHSV, Scalar(red_h_low, red_s_low, red_v_low), Scalar(red_h_high, red_s_high, red_v_high), img_thresh_red); // threshold!
	cv::inRange(imgHSV, Scalar(green_h_low, green_s_low, green_v_low), Scalar(green_h_high, green_s_high, green_v_high), img_thresh_green); // threshold!

	int morph_elem = 2;
	int morph_size = 3;
	int const max_elem = 2;
	int const max_kernel_size = 21;

	int operation = 3;
	Mat element = getStructuringElement(morph_elem, Size(2 * morph_size + 1, 2 * morph_size + 1), Point(morph_size, morph_size));
	morphologyEx(img_thresh_red, img_thresh_red2, operation, element);
	morphologyEx(img_thresh_green, img_thresh_green2, operation, element);

	// extract connected components and statistics
	cv::Mat labelImage_red(img.size(), CV_8UC3);
	cv::Mat stats_red(img.size(), CV_64F);
	cv::Mat centroids_red(img.size(), CV_64F);
	int nLabels_red = connectedComponentsWithStats(img_thresh_red2, labelImage_red, stats_red, centroids_red, 8, CV_32S);
	std::vector<Vec3b> colors(nLabels_red);
	colors[0] = Vec3b(0, 0, 0); //background
	for (int label = 1; label < nLabels_red; ++label) {
		tracking_points.at<double>(0, 0) = centroids_red.at<double>(label, 0);
		tracking_points.at<double>(0, 1) = centroids_red.at<double>(label, 1);
	}

	cv::Mat labelImage_green(img.size(), CV_8UC3);
	cv::Mat stats_green(img.size(), CV_64F);
	cv::Mat centroids_green(img.size(), CV_64F);
	int nLabels_green = connectedComponentsWithStats(img_thresh_green2, labelImage_green, stats_green, centroids_green, 8, CV_32S);
	colors[0] = Vec3b(0, 0, 0); //background
	for (int label = 1; label < nLabels_green; ++label) {
		tracking_points.at<double>(1, 0) = centroids_green.at<double>(label, 0);
		tracking_points.at<double>(1, 1) = centroids_green.at<double>(label, 1);

	}

	return tracking_points;
}
