// Tracker Horst

/*
Perform online tracking of images grabbed from Basler camera
See Github Repo for explanations
*/

#include "stdafx.h"
#include <sstream>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <string> 

// Include files to use the PYLON API.
#include <pylon/PylonIncludes.h>
#ifdef PYLON_WIN_BUILD
#    include <pylon/PylonGUI.h>
#endif

// Include files to use OpenCV API.
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/video.hpp>
#include <opencv2/features2d.hpp>
#include "opencv2/core/cvdef.h"
#include "thresh_tracking.h"
#include "init_func.h"

#include <pylon/usb/BaslerUsbInstantCamera.h>
using namespace Pylon;
// Settings for using Basler USB cameras.
typedef Pylon::CBaslerUsbInstantCamera Camera_t;
using namespace Basler_UsbCameraParams;
// Namespace for using OpenCV objects.
using namespace cv;
// Namespace for using cout.
using namespace std;
using namespace GenApi;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int red_h_low;
int red_s_low;
int red_v_low;
int red_h_high;
int red_s_high;
int red_v_high;
int green_h_low;
int green_s_low;
int green_v_low;
int green_h_high;
int green_s_high;
int green_v_high;

void on_red_h_low_trackbar(int position)
{	red_h_low = position; }
void on_red_s_low_trackbar(int position)
{	red_s_low = position; }
void on_red_v_low_trackbar(int position)
{	red_v_low = position; }
void on_red_h_high_trackbar(int position)
{	red_h_high = position; }
void on_red_s_high_trackbar(int position)
{	red_s_high = position; }
void on_red_v_high_trackbar(int position)
{	red_v_high = position; }

void on_green_h_low_trackbar(int position)
{	green_h_low = position; }
void on_green_s_low_trackbar(int position)
{	green_s_low = position; }
void on_green_v_low_trackbar(int position)
{	green_v_low = position; }
void on_green_h_high_trackbar(int position)
{	green_h_high = position; }
void on_green_s_high_trackbar(int position)
{	green_s_high = position; }
void on_green_v_high_trackbar(int position)
{	green_v_high = position; }
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{

	// The exit code of the sample application.
	int exitCode = 0;
	int show_trackbars = 1;
	int grabber_timeout = 3000;
	int retval = 0;
	int exposure_time = 3000;
	int delay_us = 0;
	int debouncer_us = 0;
	int gain_increase = 0;
	int saveImages = 0;
	int recordVideo = 0;
	int acq_frame_height;
	int acq_frame_width;
	int offset_x;
	int offset_y;
	int scale_factor;
	int playback_speed_video;
	int alpha = 2;
	double alpha_ = .2;
	string base_filename;

	cout << "Reading ini-file..." << endl;
	retval = init(show_trackbars, grabber_timeout, exposure_time, delay_us, gain_increase, debouncer_us, saveImages, recordVideo,
		acq_frame_height, acq_frame_width, offset_x, offset_y, scale_factor,
		red_h_low, red_s_low, red_v_low, red_h_high, red_s_high, red_v_high,
		green_h_low, green_s_low, green_v_low, green_h_high, green_s_high, green_v_high, playback_speed_video, alpha, base_filename);

	// Automagically call PylonInitialize and PylonTerminate to ensure the pylon runtime system
	// is initialized during the lifetime of this object.
	Pylon::PylonAutoInitTerm autoInitTerm;
	SYSTEMTIME st;
	GetLocalTime(&st);	
	cout << endl << endl << "Started: " << st.wYear << "-" << st.wMonth << "-" << st.wDay << " " << st.wHour << ":" << st.wMinute << ":" << st.wSecond << endl << endl;

	try
	{
		CDeviceInfo info;
		info.SetDeviceClass(Camera_t::DeviceClass());
		Camera_t camera(CTlFactory::GetInstance().CreateFirstDevice());
		// Print the model name of the camera
		cout << "Using device " << camera.GetDeviceInfo().GetVendorName() << " " << camera.GetDeviceInfo().GetModelName() << endl;

		// Get a camera nodemap in order to access camera parameters
		INodeMap& nodemap = camera.GetNodeMap();
		// Open the camera before accessing any parameters
		camera.Open();

		// Set basic camera settings
		camera.MaxNumBuffer = 25; // Make sure buffer does not overfloat if framerate is too high! 
		CIntegerPtr(nodemap.GetNode("Width"))->SetValue(acq_frame_width);
		CIntegerPtr(nodemap.GetNode("Height"))->SetValue(acq_frame_height);
		CIntegerPtr(nodemap.GetNode("OffsetX"))->SetValue(offset_x);
		CIntegerPtr(nodemap.GetNode("OffsetY"))->SetValue(offset_y);
		CEnumerationPtr gainAuto(nodemap.GetNode("GainAuto"));
		CEnumerationPtr exposureAuto(nodemap.GetNode("ExposureAuto"));
		CEnumerationPtr exposureMode(nodemap.GetNode("ExposureMode"));

		cout << "Setting auto gain to off" << endl;
		gainAuto->FromString("Off");
		cout << "Setting auto exposure to off" << endl;
		exposureAuto->FromString("Off");
		cout << "Setting auto whitebalance to off " << endl;
		camera.AutoFunctionROISelector.SetValue(AutoFunctionROISelector_ROI1);
		camera.AutoFunctionROIUseWhiteBalance.SetValue(false);
		cout << "Setting Exposure mode to Timed" << endl;
		exposureMode->FromString("Timed");
		CFloatPtr gain(nodemap.GetNode("Gain"));
		double newGain = gain->GetMin() + gain_increase;
		cout << "Setting gain to " << newGain << endl;
		gain->SetValue(newGain);

		CFloatPtr(nodemap.GetNode("ExposureTime"))->SetValue(exposure_time);
		CEnumerationPtr(nodemap.GetNode("PixelFormat"))->FromString("RGB8");

		// Set to hardware trigger mode: 
		// Select the frame start trigger
		camera.TriggerSelector.SetValue(TriggerSelector_FrameStart);
		camera.TriggerMode.SetValue(TriggerMode_On);
		camera.TriggerSource.SetValue(TriggerSource_Line1); // That corresponds to optocoupled line 1 
		camera.TriggerActivation.SetValue(TriggerActivation_RisingEdge);
		camera.TriggerDelay.SetValue(delay_us);
		camera.LineSelector.SetValue(LineSelector_Line1);
		camera.LineDebouncerTime.SetValue(debouncer_us);

		// Output signal on Line2 (opto coupled output) when exposing image:
		camera.LineSelector.SetValue(LineSelector_Line2);
		camera.LineSource.SetValue(LineSource_ExposureActive);

		// Create an OpenCV video creator.
		VideoWriter cvVideoCreator;
		if (recordVideo)
		{
			// Define the video file name.
			std::stringstream videoFileName_;
			videoFileName_ << base_filename << ".avi";
			std::string videoFileName = videoFileName_.str();
			// Define the video frame size.
			cv::Size frameSize = Size(acq_frame_width / scale_factor, acq_frame_height / scale_factor);
			cvVideoCreator.open(videoFileName, CV_FOURCC('M', 'J', 'P', 'G'), playback_speed_video, frameSize, true);
		}
																												  
		// create output tracking .csv file
		std::stringstream outputfilename_;
		outputfilename_ << base_filename << ".csv";
		std::string outputfilename = outputfilename_.str();
		ofstream output_timestamps;
		output_timestamps.open(outputfilename);
		output_timestamps << camera.GetDeviceInfo().GetVendorName() << "_" << camera.GetDeviceInfo().GetModelName() << "\n";
		output_timestamps << "Tracking height (px): " << acq_frame_height / scale_factor << " | Tracking width (px): " << acq_frame_width / scale_factor << "\n";

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		// Create display windows
		namedWindow("Live Image", CV_WINDOW_NORMAL); // other options: CV_AUTOSIZE, CV_FREERATIO, CV_WINDOW_NORMAL
		resizeWindow("Live Image", 500, 500);
		namedWindow("Tracking", CV_WINDOW_NORMAL);
		resizeWindow("Tracking", 500, 500);

		// Create bars
		if (show_trackbars == 1)  {
			namedWindow("Thresholds", CV_WINDOW_NORMAL);
			resizeWindow("Thresholds", 500, 800);

			cvCreateTrackbar("R H LOW", "Thresholds",  &red_h_low,  255, on_red_h_low_trackbar);
			cvCreateTrackbar("R H HIGH", "Thresholds", &red_h_high, 255, on_red_h_high_trackbar);
			cvCreateTrackbar("R S LOW", "Thresholds",  &red_s_low,  255, on_red_s_low_trackbar);
			cvCreateTrackbar("R S HIGH", "Thresholds", &red_s_high, 255, on_red_s_high_trackbar);
			cvCreateTrackbar("R V LOW", "Thresholds",  &red_h_low,  255, on_red_v_low_trackbar);
			cvCreateTrackbar("R V HIGH", "Thresholds", &red_h_high, 255, on_red_v_high_trackbar);

			cvCreateTrackbar("G H LOW",  "Thresholds", &green_h_low,  255, on_green_h_low_trackbar);
			cvCreateTrackbar("G H HIGH", "Thresholds", &green_h_high, 255, on_green_h_high_trackbar);
			cvCreateTrackbar("G S LOW",  "Thresholds", &green_s_low,  255, on_green_s_low_trackbar);
			cvCreateTrackbar("G S HIGH", "Thresholds", &green_s_high, 255, on_green_s_high_trackbar);
			cvCreateTrackbar("G V LOW",  "Thresholds", &green_h_low,  255, on_green_v_low_trackbar);
			cvCreateTrackbar("G V HIGH", "Thresholds", &green_h_high, 255, on_green_v_high_trackbar);
		} 

		Mat tracking_result = Mat(2, 2, CV_64F, double(0)); // what comes out of the actual tracking function 
		double framerate_calc = 0.;

		// Initialize grabbed matrix:
		cv::Mat mat8_uc3_c(acq_frame_height, acq_frame_width, CV_8UC3);
		int64_t grabbedImages = 0;
		int64_t first_tic;

		// initialize matrices:
		cv::Mat mat8_uc3(acq_frame_height, acq_frame_width, CV_8UC3);
		Size size_small(acq_frame_height / scale_factor, acq_frame_width / scale_factor); // resize
		cv::Mat mat8_uc3_small_track(acq_frame_height / scale_factor, acq_frame_width / scale_factor, CV_8UC3); // black image - stays black
		cv::Mat mat8_uc3_small_video(acq_frame_height / scale_factor, acq_frame_width / scale_factor, CV_8UC3); // for showing in video

		// Clock for system time timestamps
		using Clock = std::chrono::high_resolution_clock;

		// Enable Time chunk retrieval 
		camera.ChunkModeActive.SetValue(true);
		camera.ChunkSelector.SetValue(ChunkSelector_Timestamp);
		camera.ChunkEnable.SetValue(true);

		camera.AcquisitionStart.Execute();
		camera.StartGrabbing(GrabStrategy_OneByOne, GrabLoop_ProvidedByUser); // Grab all images that are triggered - careful with buffer overflow! 

		cout << "Starting grabbing..." << endl;
		CGrabResultPtr ptrGrabResult;

		while (camera.IsGrabbing()) 
		{
			// Wait for an image and then retrieve it. A timeout of XX ms is used.
			camera.RetrieveResult(grabber_timeout, ptrGrabResult, TimeoutHandling_ThrowException); 
			
			if (GetAsyncKeyState(VK_RETURN))
			{
				cout << "Pressed RETURN, aborting...";
				cin.ignore();
				break;
			}

			if (ptrGrabResult->GrabSucceeded())
			{
				// get timestamp chunk: 
				CIntegerPtr chunkTimestamp(ptrGrabResult->GetChunkDataNodeMap().GetNode("ChunkTimestamp"));

				if (grabbedImages == 0)
				{
					first_tic = chunkTimestamp->GetValue();
					// Get a fresh system timestamp: 
					SYSTEMTIME st;
					GetLocalTime(&st);
					output_timestamps << "First frame system timestamp:  " << st.wYear << "-" << st.wMonth << "-" << st.wDay << "_" << st.wHour << "-" << st.wMinute << "-" << st.wSecond << "\n";
					// Header written, now set up actual columns:
					output_timestamps << "frame,cam_timestamp,sys_timestamp,green_x,green_y,red_x,red_y\n";
				}
				// get high res system time stamp
				auto tic_system = Clock::now().time_since_epoch().count();
				
				grabbedImages++;
				// calculate running framerate:
				if (grabbedImages > 1) framerate_calc = 1000000000. / ((chunkTimestamp->GetValue() - first_tic) / grabbedImages);

				// Create an OpenCV image from a grabbed image.
				cv::Mat mat8_uc3(acq_frame_height, acq_frame_width, CV_8UC3, (uintmax_t *)ptrGrabResult->GetBuffer());
				resize(mat8_uc3, mat8_uc3_small_video, size_small); //resize images
				mat8_uc3.release();

				// Start tracking here 
				tracking_result = GetThresholdedImage(mat8_uc3_small_video, red_h_low, red_s_low, red_v_low,
					red_h_high, red_s_high, red_v_high, green_h_low, green_s_low, green_v_low, green_h_high, green_s_high, green_v_high);

				// add overlay
				cv::Mat mat8_uc3_small_track_overlay;
				mat8_uc3_small_track.copyTo(mat8_uc3_small_track_overlay);

				// draw tracking
				Point pt_green = Point(tracking_result.at<double>(0, 0), tracking_result.at<double>(0, 1));
				circle(mat8_uc3_small_track_overlay, pt_green, 1.5, cvScalar(0, 0, 255), CV_FILLED, 1.5);

				Point pt_red = Point(tracking_result.at<double>(1, 0), tracking_result.at<double>(1, 1));
				circle(mat8_uc3_small_track_overlay, pt_red, 1.5, cvScalar(0, 255, 0), CV_FILLED, 1.5);

				// add tracking to overlay and combine with source matrix
				alpha_ = (double)alpha/10;
				cv::addWeighted(mat8_uc3_small_track_overlay, alpha_, mat8_uc3_small_track, 1 - alpha_, 0, mat8_uc3_small_track);

				// save
				output_timestamps << grabbedImages << "," << chunkTimestamp->GetValue() << "," << tic_system  << "," << tracking_result.at<double>(0, 0) << "," <<
					tracking_result.at<double>(0, 1) << "," << tracking_result.at<double>(1, 0) << ","
					<< tracking_result.at<double>(1, 1) << "\n";
	
				// invert channel order
				cv::cvtColor(mat8_uc3_small_video, mat8_uc3_small_video, cv::COLOR_BGR2RGB);

				// Set saveImages to '1' to save images. Careful, this is extremly slow.
				if (saveImages) {
					std::stringstream imagefilename_;
					imagefilename_ << "export/" << st.wYear << "-" << st.wMonth << "-" << st.wDay << "_" << st.wHour << "-" << st.wMinute << "-" << st.wSecond << "__" << grabbedImages << ".jpg";
					std::string imagefilename = imagefilename_.str();
					imwrite(imagefilename, mat8_uc3_small_video);
				}

				// Set recordVideo to '1' to record AVI video file.
				if (recordVideo)
					cvVideoCreator.write(mat8_uc3_small_video);

				mat8_uc3_small_video = mat8_uc3_small_video * 4; // only for viewing purposes (make it all a bit brighter)

				// Burn framerate into image
				std::ostringstream strs;
				strs << fixed << setprecision(2) << framerate_calc << " fps | " << grabbedImages;
				std::string str = strs.str();
				putText(mat8_uc3_small_video, str, cvPoint(3, 15),
					FONT_HERSHEY_SIMPLEX, 0.35, cvScalar(255, 255, 255), 0, CV_AA);
				// Create an OpenCV display window.
				imshow("Live Image", mat8_uc3_small_video);
				imshow("Tracking", mat8_uc3_small_track);

				mat8_uc3_small_video.release(); // release opencv matrices. Don't know if this is strictly necessary.
				tracking_result.release();
				waitKey(1);
				// 
#ifdef PYLON_WIN_BUILD
				// Display the grabbed image in pylon.
				//Pylon::DisplayImage(1, ptrGrabResult);
#endif
			}
			else
			{
				cout << "Error: " << ptrGrabResult->GetErrorCode() << " " << ptrGrabResult->GetErrorDescription() << endl;
			}
		}

		// close timestamp file 
		output_timestamps.close();

		// Release the video file on leaving.
		if (recordVideo)
			cvVideoCreator.release();
	}
	catch (GenICam::GenericException &e)
	{
		// Error handling.
		cout << "An exception occurred." << endl
			<< e.GetDescription() << endl;
		exitCode = 1;
	}

	// Comment the following two lines to disable waiting on exit.
	//cerr << endl << "Press ENTER to exit." << endl;
	//while (cin.get() != '\n');
	return exitCode;
}