// Pylon_with_OpenCV.cpp

/*
    Note: Before getting started, Basler recommends reading the Programmer's Guide topic
    in the pylon C++ API documentation that gets installed with pylon.
    If you are upgrading to a higher major version of pylon, Basler also
    strongly recommends reading the Migration topic in the pylon C++ API documentation.

    This sample illustrates how to grab and process images using the CInstantCamera class and OpenCV.
    The images are grabbed and processed asynchronously, i.e.,
    while the application is processing a buffer, the acquisition of the next buffer is done
    in parallel.
	
	OpenCV is used to demonstrate an image display, an image saving and a video recording.

    The CInstantCamera class uses a pool of buffers to retrieve image data
    from the camera device. Once a buffer is filled and ready,
    the buffer can be retrieved from the camera object for processing. The buffer
    and additional image data are collected in a grab result. The grab result is
    held by a smart pointer after retrieval. The buffer is automatically reused
    when explicitly released or when the smart pointer object is destroyed.
*/

#include "stdafx.h"
#include <sstream>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <vector>

// Include files to use the PYLON API.
#include <pylon/PylonIncludes.h>
#ifdef PYLON_WIN_BUILD
#    include <pylon/PylonGUI.h>
#endif

// Include files used by samples.
#include "ConfigurationEventPrinter.h"
#include "CameraEventPrinter.h"

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
#include <pylon/usb/BaslerUsbInstantCamera.h>
typedef Pylon::CBaslerUsbInstantCamera Camera_t;
typedef CBaslerUsbCameraEventHandler CameraEventHandler_t; // Or use Camera_t::CameraEventHandler_t
using namespace Basler_UsbCameraParams;


// Namespace for using OpenCV objects.
using namespace cv;
// Namespace for using cout.
using namespace std;
using namespace GenApi;

// Number of images to be grabbed.
static const uint32_t c_countOfImagesToGrab = 100;
vector<double> framerate_vec;
vector<double> frame_id_vec;

//Enumeration used for distinguishing different events.
enum MyEvents
{
	ExposureEndEvent = 100
	// More events can be added here.
};

// Example handler for camera events.
class CSampleCameraEventHandler : public CameraEventHandler_t
{
public:
	virtual void OnCameraEvent(Camera_t& camera, intptr_t userProvidedId, GenApi::INode* /* pNode */)
	{
		switch (userProvidedId)
		{
		case ExposureEndEvent: // Exposure End event
			int64_t tic = camera.EventExposureEndTimestamp.GetValue();
			cout << "Exposure End event. FrameID: " << camera.EventExposureEndFrameID.GetValue() << " Timestamp: " << tic << endl;
			frame_id_vec.push_back(camera.EventExposureEndFrameID.GetValue());
			framerate_vec.push_back(tic);
			break;
			// More events can be added here.
		}
	}
};

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
		red_h_low,red_s_low, red_v_low, red_h_high, red_s_high, red_v_high,
		green_h_low, green_s_low, green_v_low, green_h_high, green_s_high, green_v_high, playback_speed_video);

    // Automagically call PylonInitialize and PylonTerminate to ensure the pylon runtime system
    // is initialized during the lifetime of this object.
    Pylon::PylonAutoInitTerm autoInitTerm;
	SYSTEMTIME st;
	GetLocalTime(&st);	//GetSystemTime(&st);
	cout << endl << endl << "Started: " << st.wYear << "-" <<  st.wMonth << "-" << st.wDay << " " << st.wHour  << ":" <<  st.wMinute << ":" << st.wSecond << endl;

	// Create an example event handler. In the present case, we use one single camera handler for handling multiple camera events.
	// The handler prints a message for each received event.
	CSampleCameraEventHandler* Handler_exposure_ev = new CSampleCameraEventHandler;

    try
    {
		CDeviceInfo info;
		info.SetDeviceClass(Camera_t::DeviceClass());
        // Create an instant camera object with the camera device found first.
		Camera_t camera( CTlFactory::GetInstance().CreateFirstDevice());

		// ENABLE HARDWARE TRIGGERING HERE

		camera.GrabCameraEvents = true;
		camera.RegisterCameraEventHandler(Handler_exposure_ev, "EventExposureEndData", ExposureEndEvent, RegistrationMode_ReplaceAll, Cleanup_None);

        // Print the model name of the camera.
		cout << "Using device " << camera.GetDeviceInfo().GetVendorName() << " " << camera.GetDeviceInfo().GetModelName() << endl;

		// Get a camera nodemap in order to access camera parameters.
		INodeMap& nodemap = camera.GetNodeMap();
		// Open the camera before accessing any parameters.
		camera.Open();
		if (!GenApi::IsAvailable(camera.EventSelector))
		{
			throw RUNTIME_EXCEPTION("The device doesn't support events.");
		}

		// Enable sending of Exposure End events.
		camera.EventSelector.SetValue(EventSelector_ExposureEnd);
		camera.EventNotification.SetValue(EventNotification_On);

		// The parameter MaxNumBuffer can be used to control the count of buffers
        // allocated for grabbing. The default value of this parameter is 10.
        camera.MaxNumBuffer = 10;
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
		cout << "Setting gain to " << newGain <<  endl;
		gain->SetValue(newGain);
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

		// Output signal on Line2 when exposing image:
		camera.LineSelector.SetValue(LineSelector_Line2);
		camera.LineSource.SetValue(LineSource_ExposureActive);


		// Create an OpenCV video creator.
		VideoWriter cvVideoCreator;
		// Define the video file name.
		std::string videoFileName= "openCvVideo.avi";
		// Define the video frame size.
		cv::Size frameSize= Size(acq_frame_width/scale_factor, acq_frame_height/scale_factor);
		cvVideoCreator.open(videoFileName, CV_FOURCC('M', 'J', 'P', 'G'), playback_speed_video, frameSize, true); // 'M','J','P','G'

		// Get a fresh timestamp: 
		SYSTEMTIME st;
		GetLocalTime(&st);	//GetSystemTime(&st);
		cout << "Started: " << st.wYear << "-" << st.wMonth << "-" << st.wDay << " " << st.wHour << ":" << st.wMinute << ":" << st.wSecond << endl; 

		// create text file
		ofstream output_timestamps;
		output_timestamps.open("export_timestamps.csv");
		output_timestamps << camera.GetDeviceInfo().GetVendorName() << "_" << camera.GetDeviceInfo().GetModelName() <<  "\n";
		output_timestamps << "Tracking height (px): " << acq_frame_height/scale_factor << " | Tracking width (px): " << acq_frame_width / scale_factor << "\n";
		output_timestamps << "File creation timestamp:  " << st.wYear << "-" << st.wMonth << "-" << st.wDay << " " << st.wHour << ":" << st.wMinute << ":" << st.wSecond  << "\n";
		// Header written, now actual columns:
		output_timestamps << "frame,cam_timestamp,green_x,green_y,red_x,red_y\n";

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		
		// Create display windows
		namedWindow("OpenCV Tracker", CV_WINDOW_NORMAL); // other options: CV_AUTOSIZE, CV_FREERATIO, CV_WINDOW_NORMAL
		resizeWindow("OpenCV Tracker", 500, 500);
		namedWindow("OpenCV Tracker Thresh", CV_WINDOW_NORMAL); 
		resizeWindow("OpenCV Tracker Thresh", 500, 500);

		cv::Mat tracking_result; // what comes out of the actual tracking function 
		vector<double> g_x;
		vector<double> r_x;
		vector<double> g_y;
		vector<double> r_y;
		int64_t tic;
		double framerate_calc = 0.;

		// Initialize grabbed matrix:
		cv::Mat mat8_uc3_c(acq_frame_height, acq_frame_width, CV_8UC3);
		int64_t grabbedImages = 0;

		// initialize matrices:
		cv::Mat mat8_uc3(acq_frame_height, acq_frame_width, CV_8UC3);

		Size size_small(acq_frame_height / scale_factor, acq_frame_width / scale_factor); // resize
		cv::Mat mat8_uc3_small(acq_frame_height / scale_factor, acq_frame_width / scale_factor, CV_8UC3); // black image- stays black
		cv::Mat mat8_uc3_small_video(acq_frame_height / scale_factor, acq_frame_width / scale_factor, CV_8UC3); // for showing in video
											  
		camera.AcquisitionStart.Execute();
		camera.StartGrabbing(GrabStrategy_LatestImageOnly, GrabLoop_ProvidedByUser);
		cout << "Starting grabbing..." << endl;
		CGrabResultPtr ptrGrabResult;

		while (1) // camera.IsGrabbing()
        {
			// calculate running framerate:
			if (grabbedImages > 0) framerate_calc = 1000000000./((framerate_vec.back() - framerate_vec.at(0)) / framerate_vec.size());

            // Wait for an image and then retrieve it. A timeout of 5000 ms is used.
			camera.WaitForFrameTriggerReady(INFINITE, TimeoutHandling_Return);			
			camera.RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException); // wait for 5 seconds then stop
			CCommandPtr(nodemap.GetNode("TimestampLatch"))->Execute();
			// Get the timestamp value
			int64_t tic = CIntegerPtr(nodemap.GetNode("TimestampLatchValue"))->GetValue();
			cout << tic << endl;
            // Image grabbed successfully?
            if (ptrGrabResult->GrabSucceeded())
            {
				grabbedImages++;
				// Create an OpenCV image from a grabbed image.
				cv::Mat mat8_uc3(acq_frame_height, acq_frame_width, CV_8UC3, (uintmax_t *)ptrGrabResult->GetBuffer());
				resize(mat8_uc3, mat8_uc3_small, size_small); //resize images
				mat8_uc3_small.copyTo(mat8_uc3_small_video); // create a copy

				// Start tracking here 
				tracking_result = GetThresholdedImage(mat8_uc3_small, red_h_low, red_s_low, red_v_low,
					red_h_high, red_s_high, red_v_high, green_h_low, green_s_low, green_v_low, green_h_high, green_s_high, green_v_high);

				// draw tracking
				Point pt_green = Point(tracking_result.at<double>(0, 0), tracking_result.at<double>(0, 1));
				circle(mat8_uc3_small, pt_green, 1, cvScalar(0, 0, 255), 1.5);
				Point pt_red = Point(tracking_result.at<double>(1, 0), tracking_result.at<double>(1, 1));
				circle(mat8_uc3_small, pt_red, 1, cvScalar(0, 255, 0), 1.5);

				// save:
				output_timestamps << frame_id_vec.back() << "," << framerate_vec.back() << "," << tracking_result.at<double>(0, 0) << "," <<
					tracking_result.at<double>(0, 1) << "," << tracking_result.at<double>(1, 0) << "," 
					 << tracking_result.at<double>(1, 1) << "\n";

				g_x.push_back(tracking_result.at<double>(0, 0)); 
				r_x.push_back(tracking_result.at<double>(1, 0));
				g_y.push_back(tracking_result.at<double>(0, 1));
				r_y.push_back(tracking_result.at<double>(1, 1));

				for (int i = 0; i < g_x.size(); i++)
				{
					pt_green = Point(g_x.at(i), g_y.at(i));
					circle(mat8_uc3_small, pt_green, 1, cvScalar(0, 0, 250), 1);
					pt_red = Point(r_x.at(i), r_y.at(i));
					circle(mat8_uc3_small, pt_red, 1, cvScalar(0, 250, 0), 1);
				}

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
				strs << fixed << setprecision(1) << framerate_calc << " fps";
				std::string str = strs.str();
				putText(mat8_uc3_small, str, cvPoint(3, 15),
					FONT_HERSHEY_SIMPLEX, 0.35, cvScalar(255, 255, 255), 0, CV_AA);
				// Create an OpenCV display window.
				imshow("OpenCV Tracker", mat8_uc3_small_video);
				imshow("OpenCV Tracker Thresh", mat8_uc3_small);

				tracking_result.release();
				waitKey(1);

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

		camera.EventSelector.SetValue(EventSelector_ExposureEnd);
		camera.EventNotification.SetValue(EventNotification_Off);

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

	// Delete the event handlers.
	delete Handler_exposure_ev;

    // Comment the following two lines to disable waiting on exit.
    cerr << endl << "Press Enter to exit." << endl;
    while( cin.get() != '\n');
	// Releases all pylon resources. 
	//PylonTerminate();
    return exitCode;
}