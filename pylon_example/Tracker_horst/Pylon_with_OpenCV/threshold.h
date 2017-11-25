#include "stdafx.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/video.hpp>


cv::Mat GetThresholdedImage(cv::Mat img,  uint8_t low_h, uint8_t high_h, uint8_t low_s, uint8_t high_s, uint8_t low_v, uint8_t high_v);