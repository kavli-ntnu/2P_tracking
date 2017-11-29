#include "stdafx.h"

#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>
#include <stdlib.h>


// Include files to use the PYLON API.
#include <pylon/PylonIncludes.h>
#ifdef PYLON_WIN_BUILD
#    include <pylon/PylonGUI.h>
#endif

using namespace std;
using namespace cv;
using namespace Pylon;
using namespace GenApi;


void on_low_r_thresh_trackbar(int, void *);
void on_high_r_thresh_trackbar(int, void *);
void on_low_g_thresh_trackbar(int, void *);
void on_high_g_thresh_trackbar(int, void *);
void on_low_b_thresh_trackbar(int, void *);
void on_high_b_thresh_trackbar(int, void *);
int low_r = 30, low_g = 30, low_b = 30;
int high_r = 100, high_g = 100, high_b = 100;
int main()
{

	Pylon::PylonAutoInitTerm autoInitTerm;
	CInstantCamera camera(CTlFactory::GetInstance().CreateFirstDevice());
	cout << "Using device " << camera.GetDeviceInfo().GetVendorName() << " " << camera.GetDeviceInfo().GetModelName() << endl;
	INodeMap& nodemap = camera.GetNodeMap();
	camera.Open();
	CEnumerationPtr gainAuto(nodemap.GetNode("GainAuto"));
	CEnumerationPtr exposureAuto(nodemap.GetNode("ExposureAuto"));
	CEnumerationPtr exposureMode(nodemap.GetNode("ExposureMode"));


	// set gain and exposure time

	cout << "Setting auto gain to off" << endl;
	//gainAuto->FromString("Off");
	cout << "Setting auto exposure to off" << endl;
	exposureAuto->FromString("Off");
	CFloatPtr gain(nodemap.GetNode("Gain"));
	double newGain = gain->GetMin() + 2;
	gain->SetValue(newGain);
	// exposure time
	double d = CFloatPtr(nodemap.GetNode("ExposureTime"))->GetValue();
	CFloatPtr(nodemap.GetNode("ExposureTime"))->SetValue(15000.);

	// Set the pixel format to _BayerRG12Packed
	CEnumerationPtr(nodemap.GetNode("PixelFormat"))->FromString("RGB8");
	//CEnumerationPtr(nodemap.GetNode("PixelFormat"))->FromString("BayerRG12");

	// Declare an integer variable to count the number of grabbed images
	// and create image file names with ascending number.
	int grabbedImages = 0;
	camera.StartGrabbing(1000, GrabStrategy_LatestImageOnly);


	
	namedWindow("Video Capture", WINDOW_NORMAL);
	namedWindow("Object Detection", WINDOW_NORMAL);
	//-- Trackbars to set thresholds for RGB values
	createTrackbar("Low R", "Object Detection", &low_r, 255, on_low_r_thresh_trackbar);
	createTrackbar("High R", "Object Detection", &high_r, 255, on_high_r_thresh_trackbar);
	createTrackbar("Low G", "Object Detection", &low_g, 255, on_low_g_thresh_trackbar);
	createTrackbar("High G", "Object Detection", &high_g, 255, on_high_g_thresh_trackbar);
	createTrackbar("Low B", "Object Detection", &low_b, 255, on_low_b_thresh_trackbar);
	createTrackbar("High B", "Object Detection", &high_b, 255, on_high_b_thresh_trackbar);

	CGrabResultPtr ptrGrabResult;


	while (camera.IsGrabbing()){
		camera.RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);

		cv::Mat frame(ptrGrabResult->GetHeight(), ptrGrabResult->GetWidth(), CV_8UC3, (uintmax_t *)ptrGrabResult->GetBuffer());
		cv::Mat frame_threshold(ptrGrabResult->GetHeight(), ptrGrabResult->GetWidth(), CV_8UC3, (uintmax_t *)ptrGrabResult->GetBuffer());

		if (frame.empty())
			break;
		//-- Detect the object based on RGB Range Values
		inRange(frame, Scalar(low_b, low_g, low_r), Scalar(high_b, high_g, high_r), frame_threshold);
		//-- Show the frames
		imshow("Video Capture", frame);
		imshow("Object Detection", frame_threshold);
	}
	return 0;
}
void on_low_r_thresh_trackbar(int, void *)
{
	low_r = min(high_r - 1, low_r);
	setTrackbarPos("Low R", "Object Detection", low_r);
}
void on_high_r_thresh_trackbar(int, void *)
{
	high_r = max(high_r, low_r + 1);
	setTrackbarPos("High R", "Object Detection", high_r);
}
void on_low_g_thresh_trackbar(int, void *)
{
	low_g = min(high_g - 1, low_g);
	setTrackbarPos("Low G", "Object Detection", low_g);
}
void on_high_g_thresh_trackbar(int, void *)
{
	high_g = max(high_g, low_g + 1);
	setTrackbarPos("High G", "Object Detection", high_g);
}
void on_low_b_thresh_trackbar(int, void *)
{
	low_b = min(high_b - 1, low_b);
	setTrackbarPos("Low B", "Object Detection", low_b);
}
void on_high_b_thresh_trackbar(int, void *)
{
	high_b = max(high_b, low_b + 1);
	setTrackbarPos("High B", "Object Detection", high_b);
}