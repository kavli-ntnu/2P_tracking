#include "stdafx.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/video.hpp>


cv::Mat GetThresholdedImage(cv::Mat img, uint low_h, uint high_h, uint low_s, uint high_s, uint low_v, uint high_v);