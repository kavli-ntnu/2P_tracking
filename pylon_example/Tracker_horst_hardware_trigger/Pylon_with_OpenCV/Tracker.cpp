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


// Include files to use OpenCV API.
#include <opencv2/core/core.hpp>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/video.hpp>
#include <opencv2/features2d.hpp>
#include "opencv2/core/cvdef.h"

// Include files to use the PYLON API.
#include <pylon/PylonIncludes.h>
#ifdef PYLON_WIN_BUILD
#    include <pylon/PylonGUI.h>
#endif

#include "thresh_tracking.h"
#include "init_func.h"

//Global variables
char ImagePath[250];
// Number of images to be grabbed.
static const uint32_t c_countOfImagesToGrab = 2000;

//void on_low_h_thresh_trackbar(int, void *);
//void on_high_h_thresh_trackbar(int, void *);
//void on_low_s_thresh_trackbar(int, void *);
//void on_high_s_thresh_trackbar(int, void *);
//void on_low_v_thresh_trackbar(int, void *);
//void on_high_v_thresh_trackbar(int, void *);
// if trackbar:
int low_h = 116, low_s = 250, low_v = 2;
int high_h = 129, high_s = 255, high_v = 189;


// Namespace for using pylon objects.
using namespace Pylon;
// Namespace for using OpenCV objects.
using namespace cv;
// Namespace for using cout.
using namespace std;
using namespace GenApi;



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

	cout << "Reading ini-file..." << endl;
	retval = init(exposure_time, delay_us, gain_increase, debouncer_us, saveImages, recordVideo,
		acq_frame_height, acq_frame_width, scale_factor,
		red_h_low,red_s_low, red_v_low, red_h_high, red_s_high, red_v_high,
		green_h_low, green_s_low, green_v_low, green_h_high, green_s_high, green_v_high);

    // Automagically call PylonInitialize and PylonTerminate to ensure the pylon runtime system
    // is initialized during the lifetime of this object.
    Pylon::PylonAutoInitTerm autoInitTerm;
	//PylonInitialize(); // has a matching "Terminate" statement at the very end.
	SYSTEMTIME st;
	GetLocalTime(&st);	//GetSystemTime(&st);
	cout << "Started: " << st.wYear << "-" <<  st.wMonth << "-" << st.wDay << " " << st.wHour  << ":" <<  st.wMinute << ":" << st.wSecond << endl;

    try
    {
        // Create an instant camera object with the camera device found first.
        CInstantCamera camera( CTlFactory::GetInstance().CreateFirstDevice());

        // Print the model name of the camera.
		cout << "Using device " << camera.GetDeviceInfo().GetVendorName() << " " << camera.GetDeviceInfo().GetModelName() << endl;

		// Get a camera nodemap in order to access camera parameters.
		INodeMap& nodemap = camera.GetNodeMap();
		// Open the camera before accessing any parameters.
		camera.Open();

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

		// Declare an integer variable to count the number of grabbed images
		// and create image file names with ascending number.
		int grabbedImages= 0;

		// Create an OpenCV video creator.
		VideoWriter cvVideoCreator;
		// Define the video file name.
		std::string videoFileName= "openCvVideo.avi";
		// Define the video frame size.
		cv::Size frameSize= Size(acq_frame_width, acq_frame_height);

		// Set the codec type and the frame rate. You have 3 codec options here.
		// The frame rate should match or be lower than the camera acquisition frame rate.
		cvVideoCreator.open(videoFileName, CV_FOURCC('M','J','P','G'), 30, frameSize, true); // 'M','P','4','2' , 'D','I','V','X'

		// create text file
		ofstream output_timestamps;
		output_timestamps.open("export_timestamps.csv");
		output_timestamps << "frame,timestamp,green_x,green_y,red_x,red_y\n";

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		// Start the grabbing of c_countOfImagesToGrab images.
        // The camera device is parameterized with a default configuration which
        // sets up free-running continuous acquisition.
		camera.StartGrabbing( c_countOfImagesToGrab, GrabStrategy_LatestImageOnly); // GrabStrategy_LatestImageOnly GrabStrategy_OneByOne

        // This smart pointer will receive the grab result data.
        CGrabResultPtr ptrGrabResult;

        // Camera.StopGrabbing() is called automatically by the RetrieveResult() method
        // when c_countOfImagesToGrab images have been retrieved.
		
		cv::Mat mat8_uc3_c(acq_frame_height, acq_frame_width, CV_8UC3);

		// start tracking code here.

		int64_t last_tic;

		namedWindow("OpenCV Tracker", CV_WINDOW_NORMAL); // other options: CV_AUTOSIZE, CV_FREERATIO, CV_WINDOW_NORMAL
		resizeWindow("OpenCV Tracker", 500, 500);
		// Create another OpenCV display window.
		namedWindow("OpenCV Tracker Thresh", CV_WINDOW_NORMAL); // other options: CV_AUTOSIZE, CV_FREERATIO, CV_WINDOW_NORMAL
		//resizeWindow("OpenCV Tracker Thresh", 500,885);
		resizeWindow("OpenCV Tracker Thresh", 500, 500);

		cv::Mat tracking_result; // what comes out of the actual tracking function 
		vector<double> g_x;
		vector<double> r_x;
		vector<double> g_y;
		vector<double> r_y;

        while ( camera.IsGrabbing())
        {
			CCommandPtr(nodemap.GetNode("TimestampLatch"))->Execute();
			// Get the timestamp value
			int64_t tic = CIntegerPtr(nodemap.GetNode("TimestampLatchValue"))->GetValue();
			if (grabbedImages == 0)
			{
				int64_t last_tic = tic;
			}
			double framerate_calc = 1/((tic - last_tic) / 1000000000.); // GHz
			//cout << grabbedImages << "| Framerate: " << framerate_calc << " fps" << endl;

     		last_tic = tic;

            // Wait for an image and then retrieve it. A timeout of 5000 ms is used.
            camera.RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);

            // Image grabbed successfully?
            if (ptrGrabResult->GrabSucceeded())
            {
				grabbedImages++;
				// Create an OpenCV image from a grabbed image.
				cv::Mat mat8_uc3(acq_frame_height, acq_frame_width, CV_8UC3, (uintmax_t *)ptrGrabResult->GetBuffer());
				Size size(acq_frame_height/scale_factor, acq_frame_width / scale_factor); // resize
				cv::Mat mat8_uc3_small(acq_frame_height, acq_frame_width, CV_8UC3);
				resize(mat8_uc3, mat8_uc3_small, size);//resize image
				mat8_uc3_small = mat8_uc3_small / 20; // scale brigthness so it's black

				// Start tracking here 
				tracking_result = GetThresholdedImage(mat8_uc3, scale_factor, red_h_low, red_s_low, red_v_low, 
					red_h_high, red_s_high, red_v_high, green_h_low, green_s_low, green_v_low, green_h_high, green_s_high, green_v_high);

				// draw tracking
				Point pt_green = Point(tracking_result.at<double>(0, 0), tracking_result.at<double>(0, 1));
				circle(mat8_uc3_small, pt_green, 1, cvScalar(0, 0, 255), 1.5);
				Point pt_red = Point(tracking_result.at<double>(1, 0), tracking_result.at<double>(1, 1));
				circle(mat8_uc3_small, pt_red, 1, cvScalar(0, 255, 0), 1.5);

				// save:
				output_timestamps << grabbedImages << "," << tic << "," << tracking_result.at<double>(0, 0) << "," <<
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
				cv::cvtColor(mat8_uc3, mat8_uc3_c, cv::COLOR_BGR2RGB);
				cv::Mat scaled_ = mat8_uc3_c * 2; // only for viewing purposes (make it all a bit brighter)

				// Set saveImages to '1' to save images.
				if (saveImages) {					
					// Create the current image name for saving.
					std::ostringstream s;
					// Create image name files with ascending grabbed image numbers.
					s << "export/image_" << grabbedImages << ".jpg";
					std::string imageName(s.str());
					// Save an OpenCV image.
					imwrite(imageName, mat8_uc3_c);
				}

				// Set recordVideo to '1' to record AVI video file.
				if (recordVideo)
					cvVideoCreator.write(scaled_);

				// Burn framerate into image
				std::ostringstream strs;
				strs << framerate_calc;
				std::string str = strs.str();
				putText(mat8_uc3_small, str, cvPoint(3, 15),
					FONT_HERSHEY_SIMPLEX, 0.3, cvScalar(255, 255, 255), 0, CV_AA);
				// Create an OpenCV display window.
				imshow( "OpenCV Tracker", scaled_);
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
    while( cin.get() != '\n');
	// Releases all pylon resources. 
	//PylonTerminate();
    return exitCode;
}
