#include "stdafx.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/video.hpp>


cv::Mat GetThresholdedImage(cv::Mat img, uint red_h_low, uint red_s_low, uint red_v_low, uint red_h_high, uint red_s_high, uint red_v_high,
	uint green_h_low, uint green_s_low, uint green_v_low, uint green_h_high, uint green_s_high, uint green_v_high);