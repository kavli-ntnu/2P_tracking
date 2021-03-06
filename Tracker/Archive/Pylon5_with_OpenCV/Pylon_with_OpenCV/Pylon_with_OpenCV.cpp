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
#include <iomanip>
#include <iostream>
#include <fstream>
// Define if images are to be saved.
// '0'- no; '1'- yes.
#define saveImages 0
// Define if video is to be recorded.
// '0'- no; '1'- yes.
#define recordVideo 1

// Include files to use OpenCV API.
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/video.hpp>

// Include files to use the PYLON API.
#include <pylon/PylonIncludes.h>
#ifdef PYLON_WIN_BUILD
#    include <pylon/PylonGUI.h>
#endif

// Namespace for using pylon objects.
using namespace Pylon;

// Namespace for using OpenCV objects.
using namespace cv;

// Namespace for using cout.
using namespace std;

using namespace GenApi;
// Number of images to be grabbed.
static const uint32_t c_countOfImagesToGrab = 8000;

int main(int argc, char* argv[])
{
	
	// The exit code of the sample application.
    int exitCode = 0;

    // Automagically call PylonInitialize and PylonTerminate to ensure the pylon runtime system
    // is initialized during the lifetime of this object.
    //Pylon::PylonAutoInitTerm autoInitTerm;
	PylonInitialize(); // has a matching "Terminate" statement at the very end.

    try
    {
        // Create an instant camera object with the camera device found first.
        CInstantCamera camera( CTlFactory::GetInstance().CreateFirstDevice());

        // Print the model name of the camera.
		cout << "Using device " << camera.GetDeviceInfo().GetVendorName() << " " << camera.GetDeviceInfo().GetModelName() << endl;

		// Get a camera nodemap in order to access camera parameters.
		INodeMap& nodemap= camera.GetNodeMap();
		// Open the camera before accessing any parameters.
		camera.Open();
		// Create pointers to access the camera Width and Height parameters.
		CIntegerPtr width= nodemap.GetNode("Width");
		CIntegerPtr height= nodemap.GetNode("Height");

        // The parameter MaxNumBuffer can be used to control the count of buffers
        // allocated for grabbing. The default value of this parameter is 10.
        camera.MaxNumBuffer = 5;

		CIntegerPtr(nodemap.GetNode("Width"))->SetValue(1800);
		CIntegerPtr(nodemap.GetNode("Height"))->SetValue(1200);
		CIntegerPtr(nodemap.GetNode("OffsetX"))->SetValue(40);
		CIntegerPtr(nodemap.GetNode("OffsetY"))->SetValue(40);

		CEnumerationPtr gainAuto(nodemap.GetNode("GainAuto"));
		CEnumerationPtr exposureAuto(nodemap.GetNode("ExposureAuto"));
		CEnumerationPtr exposureMode(nodemap.GetNode("ExposureMode"));

		if (IsWritable(gainAuto))
		{

			cout << "Setting auto gain to off" << endl;
			//gainAuto->FromString("Off");
			cout << "Setting auto exposure to off" << endl;
			exposureAuto->FromString("Off");

//			GENAPI_NAMESPACE::NodeList_t *Nodes = new NodeList_t(9);
//			nodemap.GetNodes(*Nodes);
//			for (int i = 0; i < 9; i++)
//			{
//				cout << "Node " << i << " , name " << Nodes->at(i)->GetDisplayName() << endl;
//			}
			
			CFloatPtr gain(nodemap.GetNode("Gain"));
			double newGain = gain->GetMin()+2;
			gain->SetValue(newGain);

			//GENAPI_NAMESPACE::INode* exposerureNode = nodemap.GetNode("ExposureTime");
			double d = CFloatPtr(nodemap.GetNode("ExposureTime"))->GetValue();
			CFloatPtr(nodemap.GetNode("ExposureTime"))->SetValue(15000.);

		}

		// Create a pylon ImageFormatConverter object.
		CImageFormatConverter formatConverter;
		// Specify the output pixel format.
		formatConverter.OutputPixelFormat = PixelType_BGR8packed;
		// Create a PylonImage that will be used to create OpenCV images later.
		CPylonImage pylonImage;
		// Declare an integer variable to count the number of grabbed images
		// and create image file names with ascending number.
		int grabbedImages= 0;

		// Create an OpenCV video creator.
		VideoWriter cvVideoCreator;
		// Create an OpenCV image.
		Mat openCvImage;

		// Define the video file name.
		std::string videoFileName= "openCvVideo.avi";

		// Define the video frame size.
		cv::Size frameSize= Size((int)width->GetValue(), (int)height->GetValue());

		// Set the codec type and the frame rate. You have 3 codec options here.
		// The frame rate should match or be lower than the camera acquisition frame rate.
		//cvVideoCreator.open(videoFileName, CV_FOURCC('D','I','V','X'), 30, frameSize, true);
		//cvVideoCreator.open(videoFileName, CV_FOURCC('M','P','4','2'), 30, frameSize, true); 
		cvVideoCreator.open(videoFileName, CV_FOURCC('M','J','P','G'), 30, frameSize, true);


		// create text file

		ofstream output_timestamps;
		output_timestamps.open("export_timestamps.csv");
		output_timestamps << "frame,timestamp,framerate\n";

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        // Start the grabbing of c_countOfImagesToGrab images.
        // The camera device is parameterized with a default configuration which
        // sets up free-running continuous acquisition.
		camera.StartGrabbing( c_countOfImagesToGrab, GrabStrategy_LatestImageOnly);

        // This smart pointer will receive the grab result data.
        CGrabResultPtr ptrGrabResult;

        // Camera.StopGrabbing() is called automatically by the RetrieveResult() method
        // when c_countOfImagesToGrab images have been retrieved.
		int64_t last_tic;

        while ( camera.IsGrabbing())
        {
			CCommandPtr(nodemap.GetNode("TimestampLatch"))->Execute();
			// Get the timestamp value
			int64_t tic = CIntegerPtr(nodemap.GetNode("TimestampLatchValue"))->GetValue();
			if (grabbedImages == 0)
			{
				int64_t last_tic = tic;
			}
			double framerate_calc = 1000000000. / (tic - last_tic) ;
			cout << "Framerate: " << framerate_calc << " fps" << endl;
			output_timestamps << grabbedImages << "," << tic << "," << framerate_calc << "\n";

     		last_tic = tic;

            // Wait for an image and then retrieve it. A timeout of 5000 ms is used.
            camera.RetrieveResult( 5000, ptrGrabResult, TimeoutHandling_ThrowException);

            // Image grabbed successfully?
            if (ptrGrabResult->GrabSucceeded())
            {
				grabbedImages++;
                // Access the image data.
                //cout << grabbedImages << " | SizeX: " << ptrGrabResult->GetWidth();
                //cout << " SizeY: " << ptrGrabResult->GetHeight() << endl;

				// Convert the grabbed buffer to a pylon image.
				formatConverter.Convert(pylonImage, ptrGrabResult);

				// Create an OpenCV image from a pylon image.
				openCvImage= cv::Mat(ptrGrabResult->GetHeight(), ptrGrabResult->GetWidth(), CV_8UC3, (uint8_t *) pylonImage.GetBuffer());

				// Set saveImages to '1' to save images.
				if (saveImages) {					
					// Create the current image name for saving.
					std::ostringstream s;
					// Create image name files with ascending grabbed image numbers.
					s<< "image_" << grabbedImages << ".jpg";
					std::string imageName(s.str());
					// Save an OpenCV image.

					imwrite(imageName, openCvImage);
				}

				// Set recordVideo to '1' to record AVI video file.
				if (recordVideo)
					cvVideoCreator.write(openCvImage);

				// Create an OpenCV display window.
				namedWindow( "OpenCV Tracker", CV_WINDOW_NORMAL); // other options: CV_AUTOSIZE, CV_FREERATIO, CV_WINDOW_NORMAL
				resizeWindow("OpenCV Tracker", ptrGrabResult->GetWidth()/2, ptrGrabResult->GetHeight()/2);

				// Display the current image in the OpenCV display window.
				imshow( "OpenCV Tracker", openCvImage);
				// Define a timeout for customer's input in ms.
				// '0' means indefinite, i.e. the next image will be displayed after closing the window. 
				// '1' means live stream
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
    //cerr << endl << "Press Enter to exit." << endl;
    //while( cin.get() != '\n');
	// Releases all pylon resources. 
	PylonTerminate();
    return exitCode;
}
