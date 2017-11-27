#include "stdafx.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/video.hpp>
#include <iostream>
using namespace cv;
using namespace std;

cv::Mat GetThresholdedImage(cv::Mat img, uint low_h, uint high_h, uint low_s, uint high_s, uint low_v, uint high_v)
{
	// Create an OpenCV image from a grabbed image.
	cv::Mat mat8_uc3_2(img.size() / 2, CV_8UC3);
	cv::Mat imgHSV(img.size() / 2, CV_8UC3);
	cv::Mat img_thresh_green(img.size() /2, CV_8UC3);
	cv::Mat img_thresh_green2(img.size() / 2, CV_8UC3);

	cv::Mat img_thresh_red(img.size() / 2, CV_8UC3);
	cv::Mat img_thresh_red2(img.size() / 2, CV_8UC3);

	//cv::Mat img_scribble(img.size() / 4, CV_8UC3);

	// tracking points: 
	Mat tracking_points = Mat(2, 2, CV_64F, double(0));

	Size size(500, 500);// resize
	resize(img, mat8_uc3_2, size);//resize image

	cv::cvtColor(mat8_uc3_2, imgHSV, CV_BGR2HSV); // convert to HSV

	cv::inRange(imgHSV, Scalar(116, 233, 48), Scalar(129, 255, 255), img_thresh_red); // threshold!
	cv::inRange(imgHSV, Scalar(0, 94, 85), Scalar(74, 255, 255), img_thresh_green); // threshold!
	//cv::inRange(imgHSV, Scalar(low_h, low_s, low_v), Scalar(high_h, high_s, high_v), img_thresh_red); // threshold!
	//cout << low_h << " | " << high_h << " _ " << low_s << " | " << high_s << " _ " << low_v << " | " << high_v << " |   " << endl;

	int morph_elem = 2;
	int morph_size = 3;
	int const max_elem = 2;
	int const max_kernel_size = 21;

	int operation = 3;
	Mat element = getStructuringElement(morph_elem, Size(2 * morph_size + 1, 2 * morph_size + 1), Point(morph_size, morph_size));
	morphologyEx(img_thresh_red, img_thresh_red2, operation, element);
	morphologyEx(img_thresh_green, img_thresh_green2, operation, element);

	// extract connected components and statistics
	cv::Mat labelImage_red(img.size() / 4, CV_8UC3);
	cv::Mat stats_red(img.size() / 4, CV_32S);
	cv::Mat centroids_red(img.size() / 4, CV_32S);
	int nLabels_red = connectedComponentsWithStats(img_thresh_red2, labelImage_red, stats_red, centroids_red, 8, CV_32S);
	std::vector<Vec3b> colors(nLabels_red);
	colors[0] = Vec3b(0, 0, 0); //background
	for (int label = 1; label < nLabels_red; ++label) {
		Point pt_red = Point(centroids_red.at<double>(label, 0), centroids_red.at<double>(label, 1));
		//cout << "Label " << label << "   " << (cv::Point)(centroids.at<double>(label, 0), centroids.at<double>(label, 1)) << endl;
		//circle(img_scribble, pt_red, 1, cvScalar(255, 0, 0), 1);

		// Just write to output matrix for now: 
		tracking_points.at<double>(0, 0) = centroids_red.at<double>(label, 0);
		tracking_points.at<double>(0, 1) = centroids_red.at<double>(label, 1);
	}

	cv::Mat labelImage_green(img.size() / 4, CV_8UC3);
	cv::Mat stats_green(img.size() / 4, CV_32S);
	cv::Mat centroids_green(img.size() / 4, CV_32S);
	int nLabels_green = connectedComponentsWithStats(img_thresh_green2, labelImage_green, stats_green, centroids_green, 8, CV_32S);
	colors[0] = Vec3b(0, 0, 0); //background
	for (int label = 1; label < nLabels_green; ++label) {
		Point pt_green = Point(centroids_green.at<double>(label, 0), centroids_green.at<double>(label, 1));
		//cout << "Label " << label << "   " << (cv::Point)(centroids.at<double>(label, 0), centroids.at<double>(label, 1)) << endl;
		//circle(img_scribble, pt_green, 1, cvScalar(0, 255, 0), 1);
		
		// Just write to output matrix for now: 
		tracking_points.at<double>(1, 0) = centroids_green.at<double>(label, 0);
		tracking_points.at<double>(1, 1) = centroids_green.at<double>(label, 1);

	}

	return tracking_points; // img_thresh2  img_scribble
}
