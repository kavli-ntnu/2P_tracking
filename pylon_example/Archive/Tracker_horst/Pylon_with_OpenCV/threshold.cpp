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
	cv::Mat mat8_uc3_2(img.size()/2, CV_8UC3);
	cv::Mat imgHSV(img.size() / 2, CV_8UC3);
	cv::Mat img_thresh(img.size() / 2, CV_8UC3);
	cv::Mat img_thresh2(img.size() / 2, CV_8UC3);
	cv::Mat labelImage(img.size() / 2, CV_8UC3);
	cv::Mat img_scribble(img.size() / 2, CV_8UC3);

	cv::Mat stats(img.size(), CV_32S);
	cv::Mat centroids(img.size(), CV_32S);

	Size size(500, 500);// resize
	resize(img, mat8_uc3_2, size);//resize image

	cv::cvtColor(mat8_uc3_2, imgHSV, CV_BGR2HSV); // convert to HSV
	cv::inRange(imgHSV, Scalar(low_h, low_s, low_v), Scalar(high_h, high_s, high_v), img_thresh); // threshold!
	//cout << low_h << " | " << high_h << " _ " << low_s << " | " << high_s << " _ " << low_v << " | " << high_v << " |   " << endl;

	int morph_elem = 2;
	int morph_size = 10;
	int const max_operator = 4;
	int const max_elem = 2;
	int const max_kernel_size = 21;

	int operation = 3;
	Mat element = getStructuringElement(morph_elem, Size(2 * morph_size + 1, 2 * morph_size + 1), Point(morph_size, morph_size));
	morphologyEx(img_thresh, img_thresh2, operation, element);

	// extract connected components and statistics
	int nLabels = connectedComponentsWithStats(img_thresh2, labelImage, stats, centroids, 8, CV_32S);

	std::vector<Vec3b> colors(nLabels);
	colors[0] = Vec3b(0, 0, 0); //background
	for (int label = 1; label < nLabels; ++label) {
		colors[label] = Vec3b((255), (255), (255));
		Point pt = Point(centroids.at<double>(label, 0), centroids.at<double>(label, 1));
		//cout << "Label " << label << "   " << (cv::Point)(centroids.at<double>(label, 0), centroids.at<double>(label, 1)) << endl;
		circle(img_scribble, pt, 2, cvScalar(0, 255, 150), 2);
	}

	return img_thresh2; // img_thresh2  img_scribble
}
