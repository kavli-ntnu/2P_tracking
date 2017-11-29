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

// Namespace for using pylon objects.
using namespace Pylon;
// Namespace for using cout.
using namespace std;

// INIT FUNCTION:
// Grab parameters from file in script folder
int init(int &ptr_exposure_time, int &ptr_delay_us, int &ptr_gain, int &ptr_debouncer, int &ptr_saveImages, int &ptr_recordVideo,
	int &ptr_acq_frame_height, int &ptr_acq_frame_width, int &ptr_scale_factor,
	int &ptr_red_h_low, int &ptr_red_s_low, int &ptr_red_v_low, 
	int &ptr_red_h_high, int &ptr_red_s_high, int &ptr_red_v_high,
	int &ptr_green_h_low, int &ptr_green_s_low, int &ptr_green_v_low,
	int &ptr_green_h_high, int &ptr_green_s_high, int &ptr_green_v_high)
{
	int retval = 0;
	int bufsize = 250;
	TCHAR CurrentDirectory[250];
	TCHAR strAppName[30] = L"Tracker_params";
	TCHAR iniFilePath[250];

	size_t nbrOfConvertedChars = 0;

	SYSTEMTIME st; // get timestamp to eventually put this as export folder id
	GetLocalTime(&st);	//GetSystemTime(&st);

	try
	{
		GetCurrentDirectory(bufsize, CurrentDirectory);
		swprintf(iniFilePath, 250, L"%s\\%s.ini", CurrentDirectory, strAppName);

		ptr_exposure_time = GetPrivateProfileInt(TEXT("SETTINGS"), TEXT("ExposureTime"), 3000, iniFilePath);
		cout << "Exposure time set to: " << ptr_exposure_time << endl;

		ptr_delay_us = GetPrivateProfileInt(TEXT("SETTINGS"), TEXT("TriggerDelay"), 0, iniFilePath);
		cout << "Trigger delay set to (us): " << ptr_delay_us << endl;

		ptr_gain = GetPrivateProfileInt(TEXT("SETTINGS"), TEXT("Gain"), 0, iniFilePath);
		cout << "Gain set to: " << ptr_gain << endl;

		ptr_debouncer = GetPrivateProfileInt(TEXT("SETTINGS"), TEXT("DebouncerTime"), 0, iniFilePath);
		cout << "Line1 Debouncer Time set to (us): " << ptr_debouncer << endl;

		ptr_saveImages = GetPrivateProfileInt(TEXT("SETTINGS"), TEXT("saveImages"), 1, iniFilePath);
		cout << "SaveImages set to (0=inactive): " << ptr_saveImages << endl;

		ptr_recordVideo = GetPrivateProfileInt(TEXT("SETTINGS"), TEXT("recordVideo"), 1, iniFilePath);
		cout << "RecordVideo set to (0=inactive): " << ptr_recordVideo << endl;

		ptr_acq_frame_height = GetPrivateProfileInt(TEXT("SETTINGS"), TEXT("acq_frame_height"), 0, iniFilePath);
		cout << "Acquisition frame height: " << ptr_acq_frame_height << endl;

		ptr_acq_frame_width = GetPrivateProfileInt(TEXT("SETTINGS"), TEXT("acq_frame_width"), 0, iniFilePath);
		cout << "Acquisition frame width: " << ptr_acq_frame_width << endl;

		ptr_scale_factor = GetPrivateProfileInt(TEXT("SETTINGS"), TEXT("scale_factor"), 0, iniFilePath);
		cout << "Scale factor: " << ptr_scale_factor << endl;

		ptr_red_h_low = GetPrivateProfileInt(TEXT("SETTINGS"), TEXT("red_h_low"), 0, iniFilePath);
		cout << "Red H low: " << ptr_red_h_low << endl;

		ptr_red_s_low = GetPrivateProfileInt(TEXT("SETTINGS"), TEXT("red_s_low"), 0, iniFilePath);
		cout << "Red S low: " << ptr_red_s_low << endl;

		ptr_red_v_low = GetPrivateProfileInt(TEXT("SETTINGS"), TEXT("red_v_low"), 0, iniFilePath);
		cout << "Red V low: " << ptr_red_v_low << endl;

		ptr_red_h_high = GetPrivateProfileInt(TEXT("SETTINGS"), TEXT("red_h_high"), 0, iniFilePath);
		cout << "Red H high: " << ptr_red_h_high << endl;

		ptr_red_s_high = GetPrivateProfileInt(TEXT("SETTINGS"), TEXT("red_s_high"), 0, iniFilePath);
		cout << "Red S high: " << ptr_red_s_high << endl;

		ptr_red_v_high = GetPrivateProfileInt(TEXT("SETTINGS"), TEXT("red_v_high"), 0, iniFilePath);
		cout << "Red V high: " << ptr_red_v_high << endl;

		ptr_green_h_low = GetPrivateProfileInt(TEXT("SETTINGS"), TEXT("green_h_low"), 0, iniFilePath);
		cout << "Green H low: " << ptr_green_h_low << endl;

		ptr_green_s_low = GetPrivateProfileInt(TEXT("SETTINGS"), TEXT("green_s_low"), 0, iniFilePath);
		cout << "Green S low: " << ptr_green_s_low << endl;

		ptr_green_v_low = GetPrivateProfileInt(TEXT("SETTINGS"), TEXT("green_v_low"), 0, iniFilePath);
		cout << "Green V low: " << ptr_green_v_low << endl;

		ptr_green_h_high = GetPrivateProfileInt(TEXT("SETTINGS"), TEXT("green_h_high"), 0, iniFilePath);
		cout << "Green H high: " << ptr_green_h_high << endl;

		ptr_green_s_high = GetPrivateProfileInt(TEXT("SETTINGS"), TEXT("green_s_high"), 0, iniFilePath);
		cout << "Green S high: " << ptr_green_s_high << endl;

		ptr_green_v_high = GetPrivateProfileInt(TEXT("SETTINGS"), TEXT("green_v_high"), 0, iniFilePath);
		cout << "Green V high: " << ptr_green_v_high << endl;

	}
	catch (...)
	{
		cout << "Error reading ini-file! Using default settings." << endl;
		retval = 1;
	}
	return retval;
}
