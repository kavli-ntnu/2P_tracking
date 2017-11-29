// Tracker Horst

/*
Perform online tracking of images grabbed from Basler camera



*/

#include "stdafx.h"
#include <sstream>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>

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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{

	// The exit code of the sample application.
	int exitCode = 0;
	int retval = 0;
	int exposure_time = 3000;
	int delay_us = 0;
	int debouncer_us = 0;
	int gain_increase = 0;
	int saveImages = 0;
	int recordVideo = 0;
	int acq_frame_height;
	int acq_frame_width;
	int scale_factor;
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
	int playback_speed_video;

	cout << "Reading ini-file..." << endl;
	retval = init(exposure_time, delay_us, gain_increase, debouncer_us, saveImages, recordVideo,
		acq_frame_height, acq_frame_width, scale_factor,
		red_h_low, red_s_low, red_v_low, red_h_high, red_s_high, red_v_high,
		green_h_low, green_s_low, green_v_low, green_h_high, green_s_high, green_v_high, playback_speed_video);

	// Automagically call PylonInitialize and PylonTerminate to ensure the pylon runtime system
	// is initialized during the lifetime of this object.
	Pylon::PylonAutoInitTerm autoInitTerm;
	SYSTEMTIME st;
	GetLocalTime(&st);	//GetSystemTime(&st);
	cout << endl << endl << "Started: " << st.wYear << "-" << st.wMonth << "-" << st.wDay << " " << st.wHour << ":" << st.wMinute << ":" << st.wSecond << endl;

	try
	{
		CDeviceInfo info;
		info.SetDeviceClass(Camera_t::DeviceClass());
		// Create an instant camera object with the camera device found first.
		Camera_t camera(CTlFactory::GetInstance().CreateFirstDevice());
		// Print the model name of the camera.
		cout << "Using device " << camera.GetDeviceInfo().GetVendorName() << " " << camera.GetDeviceInfo().GetModelName() << endl;

		// Get a camera nodemap in order to access camera parameters.
		INodeMap& nodemap = camera.GetNodeMap();
		// Open the camera before accessing any parameters.
		camera.Open();

		camera.MaxNumBuffer = 20; // Make sure buffer does not overfloat if framerate is too high! 
		CIntegerPtr(nodemap.GetNode("Width"))->SetValue(acq_frame_width);
		CIntegerPtr(nodemap.GetNode("Height"))->SetValue(acq_frame_height);
		CIntegerPtr(nodemap.GetNode("OffsetX"))->SetValue(0);
		CIntegerPtr(nodemap.GetNode("OffsetY"))->SetValue(0);
		CEnumerationPtr gainAuto(nodemap.GetNode("GainAuto"));
		CEnumerationPtr exposureAuto(nodemap.GetNode("ExposureAuto"));
		CEnumerationPtr exposureMode(nodemap.GetNode("ExposureMode"));

		// set gain and exposure time
		cout << "Setting auto gain to off" << endl;
		//gainAuto->FromString("Off");
		cout << "Setting auto exposure to off" << endl;
		exposureAuto->FromString("Off");
		CFloatPtr gain(nodemap.GetNode("Gain"));
		double newGain = gain->GetMin() + gain_increase;
		cout << "Setting gain to " << newGain << endl;
		gain->SetValue(newGain);
		// set auto wyite balance to off
		cout << "Setting auto whitebalance to off " << endl;
		camera.AutoFunctionROISelector.SetValue(AutoFunctionROISelector_ROI1);
		camera.AutoFunctionROIUseWhiteBalance.SetValue(false);

		// exposure time
		CFloatPtr(nodemap.GetNode("ExposureTime"))->SetValue(exposure_time);
		// Set the pixel format to RGB8 8bit
		CEnumerationPtr(nodemap.GetNode("PixelFormat"))->FromString("RGB8");

		// Set to hardware trigger mode: 
		// Select the frame start trigger
		camera.TriggerSelector.SetValue(TriggerSelector_FrameStart);
		camera.TriggerMode.SetValue(TriggerMode_On);
		camera.TriggerSource.SetValue(TriggerSource_Line1);
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
			std::string videoFileName = "openCvVideo.avi";
			// Define the video frame size.
			cv::Size frameSize = Size(acq_frame_width / scale_factor, acq_frame_height / scale_factor);
			cvVideoCreator.open(videoFileName, CV_FOURCC('M', 'J', 'P', 'G'), playback_speed_video, frameSize, true);
		}
																												  
	    // Get a fresh system timestamp: 
		SYSTEMTIME st;
		GetLocalTime(&st);
		cout << endl << "Started: " << st.wYear << "-" << st.wMonth << "-" << st.wDay << " " << st.wHour << ":" << st.wMinute << ":" << st.wSecond << endl;

		// create text file
		ofstream output_timestamps;
		output_timestamps.open("export_timestamps.csv");
		output_timestamps << camera.GetDeviceInfo().GetVendorName() << "_" << camera.GetDeviceInfo().GetModelName() << "\n";
		output_timestamps << "Tracking height (px): " << acq_frame_height / scale_factor << " | Tracking width (px): " << acq_frame_width / scale_factor << "\n";
		output_timestamps << "File creation timestamp:  " << st.wYear << "-" << st.wMonth << "-" << st.wDay << " " << st.wHour << ":" << st.wMinute << ":" << st.wSecond << "\n";
		// Header written, now actual columns:
		output_timestamps << "frame,cam_timestamp,sys_timestamp, green_x,green_y,red_x,red_y\n";

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		// Create display windows
		namedWindow("Live Image", CV_WINDOW_NORMAL); // other options: CV_AUTOSIZE, CV_FREERATIO, CV_WINDOW_NORMAL
		resizeWindow("Live Image", 500, 500);
		namedWindow("Tracking", CV_WINDOW_NORMAL);
		resizeWindow("Tracking", 500, 500);

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

		// Clock
		using Clock = std::chrono::high_resolution_clock;

		// Enable Time chunk retrieval 
		camera.ChunkModeActive.SetValue(true);
		camera.ChunkSelector.SetValue(ChunkSelector_Timestamp);
		camera.ChunkEnable.SetValue(true);

		camera.AcquisitionStart.Execute();
		camera.StartGrabbing(GrabStrategy_OneByOne, GrabLoop_ProvidedByUser); // Grab all images that are triggered. 

		cout << "Starting grabbing..." << endl;
		CGrabResultPtr ptrGrabResult;

		while (camera.IsGrabbing()) 
		{
			// Wait for an image and then retrieve it. A timeout of 3000 ms is used.
			camera.RetrieveResult(3000, ptrGrabResult, TimeoutHandling_ThrowException); // wait for 3 seconds then stop
																																								
			if (ptrGrabResult->GrabSucceeded())
			{
				// get timestamp chunk: 
				CIntegerPtr chunkTimestamp(ptrGrabResult->GetChunkDataNodeMap().GetNode("ChunkTimestamp"));

				if (grabbedImages == 0)
				{
					first_tic = chunkTimestamp->GetValue();
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

				// draw tracking
				Point pt_green = Point(tracking_result.at<double>(0, 0), tracking_result.at<double>(0, 1));
				circle(mat8_uc3_small_track, pt_green, 1, cvScalar(0, 0, 255), 1.5);
				Point pt_red = Point(tracking_result.at<double>(1, 0), tracking_result.at<double>(1, 1));
				circle(mat8_uc3_small_track, pt_red, 1, cvScalar(0, 255, 0), 1.5);

				// save:
				output_timestamps << grabbedImages << "," << chunkTimestamp->GetValue() << "," << tic_system  << "," << tracking_result.at<double>(0, 0) << "," <<
					tracking_result.at<double>(0, 1) << "," << tracking_result.at<double>(1, 0) << ","
					<< tracking_result.at<double>(1, 1) << "\n";
	
				// invert channel order
				cv::cvtColor(mat8_uc3_small_video, mat8_uc3_small_video, cv::COLOR_BGR2RGB);
				mat8_uc3_small_video = mat8_uc3_small_video * 4; // only for viewing purposes (make it all a bit brighter)

			    // Set saveImages to '1' to save images.
				if (saveImages) {
					// Create the current image name for saving.
					std::ostringstream s;
					// Create image name files with ascending grabbed image numbers.
					s << "export/image_" << grabbedImages << ".jpg";
					std::string imageName(s.str());
					// Save an OpenCV image.
					imwrite(imageName, mat8_uc3_small_video);
				}

				// Set recordVideo to '1' to record AVI video file.
				if (recordVideo)
					cvVideoCreator.write(mat8_uc3_small_video);

				// Burn framerate into image
				std::ostringstream strs;
				strs << fixed << setprecision(1) << framerate_calc << " fps | " << grabbedImages;
				std::string str = strs.str();
				putText(mat8_uc3_small_video, str, cvPoint(3, 15),
					FONT_HERSHEY_SIMPLEX, 0.35, cvScalar(255, 255, 255), 0, CV_AA);
				// Create an OpenCV display window.
				imshow("Live Image", mat8_uc3_small_video);
				imshow("Tracking", mat8_uc3_small_track);

				mat8_uc3_small_video.release();
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
	cerr << endl << "Press Enter to exit." << endl;
	while (cin.get() != '\n');
	// Releases all pylon resources. 
	//PylonTerminate();
	return exitCode;
}