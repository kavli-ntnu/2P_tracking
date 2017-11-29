// GrabNSave.cpp
/*
Add the following include directories for Release and Debug project configurations
(Configuration Properties -> C/C++ -> General -> Additional Include Directories):
"$(PYLON_ROOT)\include";"$(PYLON_GENICAM_ROOT)\library\CPP\include"

Add the following library directories for Release and Debug project configurations 
(Configuration Properties -> Linker -> General -> Additional Library Directories):
"$(PYLON_ROOT)\lib\Win32";"$(PYLON_GENICAM_ROOT)\library\CPP\Lib\Win32_i86"

Enable runtime type info for Release and Debug project configurations 
(Configuration Properties -> C/C++ -> Language -> Enable Runtime Type Info).
yes

Enable C++ exceptions for Release and Debug project configurations 
(Configuration Properties -> C/C++ -> Code Generation -> Enable C++ Exceptions).
yes
*/
#include "stdafx.h"
#include <windows.h>
// Include files to use the PYLON API.
#include <pylon/PylonIncludes.h>
#include <pylon/ImagePersistence.h>
#ifdef PYLON_WIN_BUILD
#include <pylon/PylonGUI.h>
#endif
#include <pylon/gige/BaslerGigEInstantCamera.h>
using namespace Basler_GigECamera;

// Namespace for using pylon objects.
using namespace Pylon;

// Namespace for using cout.
using namespace std;

//Global variables
char ImagePath[250];
int ShowImage;
int SaveImage;

//Image event handler. When an image is grabbed, save the image to the program folder.
class CMyImageEventHandler : public CBaslerGigEImageEventHandler//CImageEventHandler
{
public:
	virtual void OnImageGrabbed(CBaslerGigEInstantCamera& camera, const CBaslerGigEGrabResultPtr& ptrGrabResult) //const CGrabResultPtr& ptrGrabResult // CInstantCamera
	{
		try
		{
			if (ptrGrabResult->GrabSucceeded())
			{
				if (SaveImage != 0)
				{
				const uint8_t *pImageBuffer = (uint8_t *)ptrGrabResult->GetBuffer();
				char image_file_name[255];
				SYSTEMTIME st;
				int camera = ptrGrabResult->GetCameraContext();
				GetLocalTime(&st);	//GetSystemTime(&st);
				sprintf_s(image_file_name, "%sCamera[%ld] %04d%02d%02d %02d%02d%02d.bmp", ImagePath, camera, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
				CImagePersistence::Save(
					ImageFileFormat_Bmp,
					image_file_name,
					pImageBuffer,
					ptrGrabResult->GetImageSize(),
					ptrGrabResult->GetPixelType(),
					ptrGrabResult->GetWidth(),
					ptrGrabResult->GetHeight(),
					ptrGrabResult->GetPaddingX(),
					ImageOrientation_TopDown);
				cout << "Image " << image_file_name << " saved" << endl;
				cout << endl;
				}
#ifdef PYLON_WIN_BUILD
				if (ShowImage != 0)
				{
					// Display the image
					Pylon::DisplayImage(ptrGrabResult->GetCameraContext(), ptrGrabResult);
				}
#endif
			}
			else
			{
				SYSTEMTIME st;
				int camera = ptrGrabResult->GetCameraContext();
				GetLocalTime(&st);	//GetSystemTime(&st);
				cout << "Grab error! Camera " << camera << "Timestamp: " << st.wYear << st.wMonth << st.wDay << " " << st.wHour << st.wMinute << st.wSecond << endl;
			}
		}
		catch (GenICam::GenericException &e)
		{
			// Error handling.
			cerr << "An exception occurred in OnImageGrabbed function" << endl
				<< e.GetDescription() << endl;
		}
	}
};
int init(int &ptr_exposure_time, int &ptr_delay_us, int &ptr_gain, int &ptr_debouncer, int &ptr_frame_rate)
{
	int retval = 0;
	int bufsize = 250;
	TCHAR CurrentDirectory[250];
	TCHAR StrImageDirectory[250];
	TCHAR strAppName[30] = L"GrabNSave";
	TCHAR iniFilePath[250];
	size_t nbrOfConvertedChars = 0;
	try
	{
		GetCurrentDirectory(bufsize, CurrentDirectory);
		swprintf(iniFilePath, 250, L"%s\\%s.ini", CurrentDirectory, strAppName);
		ptr_exposure_time = GetPrivateProfileInt(TEXT("SETTINGS"), TEXT("ExposureTime"), 3000, iniFilePath);
		cout << "Exposure time set to: " << ptr_exposure_time << endl;
		ptr_frame_rate = GetPrivateProfileInt(TEXT("SETTINGS"), TEXT("FrameRate"), 0, iniFilePath);
		cout << "Frame rate set to (0=HardwareTrigger): " << ptr_frame_rate << endl;
		ptr_delay_us = GetPrivateProfileInt(TEXT("SETTINGS"), TEXT("TriggerDelay"), 0, iniFilePath);
		cout << "Trigger delay set to (us): " << ptr_delay_us << endl;
		ptr_gain = GetPrivateProfileInt(TEXT("SETTINGS"), TEXT("Gain"), 0, iniFilePath);
		cout << "Gain set to: " << ptr_gain << endl;
		ptr_debouncer = GetPrivateProfileInt(TEXT("SETTINGS"), TEXT("DebouncerTime"), 0, iniFilePath);
		cout << "Line1 Debouncer Time set to (us): " << ptr_debouncer << endl;
		ShowImage = GetPrivateProfileInt(TEXT("SETTINGS"), TEXT("ShowImage"), 1, iniFilePath);
		cout << "ShowImage set to (0=inactive): " << ShowImage << endl;
		SaveImage = GetPrivateProfileInt(TEXT("SETTINGS"), TEXT("SaveImage"), 1, iniFilePath);
		cout << "SaveImage set to (0=inactive): " << SaveImage << endl;
		GetPrivateProfileString(TEXT("SETTINGS"), TEXT("ImageDirectory"), TEXT(""), StrImageDirectory, 100, iniFilePath);
		wcstombs_s(&nbrOfConvertedChars, ImagePath, StrImageDirectory, wcslen(StrImageDirectory) + 1);
		cout << "Image path: " << ImagePath << endl << endl;
	}
	catch (...)
	{
		cout << "Error reading ini-file! Using default settings." << endl;
		retval = 1;
	}
	return retval;
}
int main(int argc, char* argv[])
{
	int exitCode = 0;
	int retval = 0;
	int exposure_time = 3000;
	int delay_us = 0;
	int debouncer_us = 0;
	int frame_rate = 0;
	int gain = 0;
	int number_of_cameras = 1;
	size_t nbrOfConvertedChars = 0;
	char feature[250]; //String for error messages
	ShowImage = 1;
	SaveImage = 1;

	cout << "Reading ini-file..." << endl;
	retval = init(exposure_time, delay_us, gain, debouncer_us, frame_rate);

	// Automagically call PylonInitialize and PylonTerminate to ensure the pylon runtime system
	// is initialized during the lifetime of this object.
	Pylon::PylonAutoInitTerm autoInitTerm;

	try
	{
		cout << "Finding camera. Please wait... " << endl;

		// Only look for cameras supported by Camera_t
		//CDeviceInfo info;
		//info.SetDeviceClass(CBaslerUsbInstantCamera::DeviceClass());
		// Get the transport layer factory.
		CTlFactory& tlFactory = CTlFactory::GetInstance();
		// Get all attached devices and exit application if no device is found.
		DeviceInfoList_t devices;
		if (tlFactory.EnumerateDevices(devices) == 0)
		{
			throw RUNTIME_EXCEPTION("No camera present.");
		}
		number_of_cameras = devices.size();
		// Create instant camera objects // NUMBER OF CAMERAS MUST BE A COMSTANT // DEFAULT 16
		CBaslerGigEInstantCamera camera[16];//(CTlFactory::GetInstance().CreateFirstDevice(info));

		for (int i = 0; i < number_of_cameras; i++)
		{
			camera[i].Attach(tlFactory.CreateDevice(devices[i]));
			camera[i].SetCameraContext(i);
			// Print the model name of the camera.
			cout << "Using device " << camera[i].GetDeviceInfo().GetModelName() << endl;
			// Register an image event handler.
			camera[i].RegisterImageEventHandler(new CMyImageEventHandler, RegistrationMode_Append, Cleanup_Delete);
			// Camera event processing must be activated first, the default is off.
			camera[i].GrabCameraEvents = true;
			// Open the camera.
			camera[i].Open();
			try // Standardfunktioner
			{
				sprintf_s(feature, "AcquisitionMode");
				// Set the acquisition mode to continuous
				camera[i].AcquisitionMode.SetValue(AcquisitionMode_Continuous);
				sprintf_s(feature, "ExposureMode");
				// Set for the timed exposure mode
				camera[i].ExposureMode.SetValue(ExposureMode_Timed);
				sprintf_s(feature, "ExposureTime");
				// Set the exposure time
				camera[i].ExposureTimeRaw.SetValue(exposure_time);
				// Set the gain
				camera[i].GainRaw.SetValue(gain);
			}
			catch (GenICam::GenericException &e)
			{
				cerr << "An exception occurred when setting camera feature [" << feature << "]!" << endl << e.GetDescription() << endl;
			}
			if (frame_rate > 0) // Självsvängande
			{
				try // FrameRate enable
				{
					sprintf_s(feature, "AcquisitionFrameRateEnable");
					camera[i].AcquisitionFrameRateEnable.SetValue(true);
				}
				catch (GenICam::GenericException &e)
				{
					cerr << "An exception occurred when setting camera feature [" << feature << "]!" << endl << e.GetDescription() << endl;
				}
				try // FrameRate
				{
					sprintf_s(feature, "AcquisitionFrameRate");
					camera[i].AcquisitionFrameRateAbs.SetValue(frame_rate);
				}
				catch (GenICam::GenericException &e)
				{
					cerr << "An exception occurred when setting camera feature [" << feature << "]!" << endl << e.GetDescription() << endl;
				}
			}
			else // Hårdvarutrigg
			{
				try 
				{
					sprintf_s(feature, "TriggerSelector");
					/*// Set trigger mode off for frame burst
					camera[i].TriggerSelector.SetValue(TriggerSelector_FrameBurstStart);
					sprintf_s(feature, "TriggerMode");
					camera[i].TriggerMode.SetValue(TriggerMode_Off);*/
					sprintf_s(feature, "TriggerSelector");
					// Select the frame start trigger
					camera[i].TriggerSelector.SetValue(TriggerSelector_FrameStart);
					sprintf_s(feature, "TriggerMode");
					// Set the mode for the selected trigger
					camera[i].TriggerMode.SetValue(TriggerMode_On);
					sprintf_s(feature, "TriggerSource");
					// Set the source for the selected trigger
					camera[i].TriggerSource.SetValue(TriggerSource_Line1);
					sprintf_s(feature, "TriggerActivation");
					// Set the trigger activation mode to rising edge
					camera[i].TriggerActivation.SetValue(TriggerActivation_RisingEdge);
					sprintf_s(feature, "TriggerDelay");
					// Set the trigger delay for one millisecond (1000us == 1ms == 0.001s)
					camera[i].TriggerDelayAbs.SetValue(delay_us);
					sprintf_s(feature, "LineSelector");
					// Set the debouncer time
					camera[i].LineSelector.SetValue(LineSelector_Line1);
					sprintf_s(feature, "LineDebouncerTime");
					camera[i].LineDebouncerTimeAbs.SetValue(debouncer_us);
				}
				catch (GenICam::GenericException &e)
				{
					// Error handling.
					cerr << "An exception occurred when setting camera trigger feature [" << feature << "]!" << endl << e.GetDescription() << endl;
				}
			}
		}
		for (int i = 0; i < number_of_cameras; i++)
		{
			// Execute an acquisition start command to prepare for frame acquisition
			camera[i].AcquisitionStart.Execute();
			// Frame acquisition will start when the externally generated
			// frame start trigger signal (ExFSTrig signal)goes high
			// Start the grabbing

			// Start the grabbing using the grab loop thread, by setting the grabLoopType parameter
			// to GrabLoop_ProvidedByInstantCamera. The grab results are delivered to the image event handlers.
			// The GrabStrategy_OneByOne default grab strategy is used.
			camera[i].StartGrabbing(GrabStrategy_LatestImageOnly, GrabLoop_ProvidedByInstantCamera);
		}
			cout << "Starting grabbing..." << endl;

		while(1)//(camera.IsGrabbing()) //while (1) //Sätt in vilkor här för att avsluta programmet...
		{
			//camera.WaitForFrameTriggerReady(INFINITE, TimeoutHandling_Return);
			//if (camera.RetrieveResult(INFINITE, ptrGrabResult, TimeoutHandling_Return))
				//Sleep(10);

			Sleep(1000);
		}
		//camera.StopGrabbing();
	}
	catch (GenICam::GenericException &e)
	{
		// Error handling.
		cerr << "An exception occurred." << endl
			<< e.GetDescription() << endl;
		exitCode = 1;
		cerr << endl << "Press Enter to exit." << endl;
		while (cin.get() != '\n');
	}
	return exitCode;
}
