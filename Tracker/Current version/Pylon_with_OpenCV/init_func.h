#pragma once
#include "stdafx.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/video.hpp>
using namespace std;


int init(int &ptr_show_trackbars, int &ptr_grabber_timeout, int &ptr_exposure_time, int &ptr_delay_us, int &ptr_gain, int &ptr_debouncer, int &ptr_saveImages, int &ptr_recordVideo,
	int &ptr_acq_frame_height, int &ptr_acq_frame_width, int &ptr_offset_x, int &ptr_offset_y, int &ptr_scale_factor,
	int &ptr_red_h_low, int &ptr_red_s_low, int &ptr_red_v_low,
	int &ptr_red_h_high, int &ptr_red_s_high, int &ptr_red_v_high,
	int &ptr_green_h_low, int &ptr_green_s_low, int &ptr_green_v_low,
	int &ptr_green_h_high, int &ptr_green_s_high, int &ptr_green_v_high, int &ptr_playback_speed_video, int &ptr_alpha, string &base_filename);