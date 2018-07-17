#include "stdafx.h"
#include <sstream>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <vector>
#include <Windows.h>


// Include files to use the PYLON API.
#include <pylon/PylonIncludes.h>
#ifdef PYLON_WIN_BUILD
#    include <pylon/PylonGUI.h>
#endif

// Namespace for using pylon objects.
using namespace Pylon;
// Namespace for using cout.
using namespace std;

#ifndef UNICODE  
typedef std::string String;
#else
typedef std::wstring String;
#endif

// INIT FUNCTION:
// Grab parameters from file in script folder
int init(int &ptr_show_trackbars, int &ptr_grabber_timeout, int &ptr_exposure_time, int &ptr_delay_us, int &ptr_gain, int &ptr_debouncer, int &ptr_saveImages, int &ptr_recordVideo,
	int &ptr_acq_frame_height, int &ptr_acq_frame_width, int &ptr_offset_x, int &ptr_offset_y, int &ptr_scale_factor,
	int &ptr_red_h_low, int &ptr_red_s_low, int &ptr_red_v_low, 
	int &ptr_red_h_high, int &ptr_red_s_high, int &ptr_red_v_high,
	int &ptr_green_h_low, int &ptr_green_s_low, int &ptr_green_v_low,
	int &ptr_green_h_high, int &ptr_green_s_high, int &ptr_green_v_high, int &ptr_playback_speed_video, int &ptr_alpha, string &base_filename)
{
	int retval = 0;
	int bufsize = 250;
	TCHAR CurrentDirectory[250];
	TCHAR strAppName[30] = L"Tracker_params";
	TCHAR iniFilePath[250]; 
	String iniFilePath_;
	size_t nbrOfConvertedChars = 0;

	SYSTEMTIME st; // get timestamp to eventually put this as export folder id
	GetLocalTime(&st);	//GetSystemTime(&st);

	try
	{
		GetCurrentDirectory(bufsize, CurrentDirectory);
		swprintf(iniFilePath, 250, L"%s\\%s.ini", CurrentDirectory, strAppName);
		iniFilePath_ = iniFilePath; // Convert TCHAR to string

		ptr_show_trackbars = GetPrivateProfileInt(TEXT("SETTINGS"), TEXT("show_trackbars"), 1, iniFilePath);
		cout << "Show trackbars (0=False):" << ptr_show_trackbars << endl;

		ptr_grabber_timeout = GetPrivateProfileInt(TEXT("SETTINGS"), TEXT("grabber_timeout "), 3000, iniFilePath);
		cout << "Grabber timeout set to: " << ptr_grabber_timeout << endl;

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

		ptr_offset_x = GetPrivateProfileInt(TEXT("SETTINGS"), TEXT("offset_x"), 0, iniFilePath);
		cout << "Offset x: " << ptr_offset_x << endl;

		ptr_offset_y = GetPrivateProfileInt(TEXT("SETTINGS"), TEXT("offset_y"), 0, iniFilePath);
		cout << "Offset y: " << ptr_offset_y << endl;
		
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

		ptr_playback_speed_video = GetPrivateProfileInt(TEXT("SETTINGS"), TEXT("playback_speed_video"), 0, iniFilePath);
		cout << "Playback speed video: " << ptr_playback_speed_video << endl;

		// Base 10 alpha value for drawing dots
		ptr_alpha = GetPrivateProfileInt(TEXT("SETTINGS"), TEXT("alpha"), 0, iniFilePath);
		cout << "Alpha of dots in tracking (/10): " << ptr_alpha << endl;

		// Get base filename path
		// https://stackoverflow.com/questions/11876290/c-fastest-way-to-read-only-last-line-of-text-file

		ifstream fin;
		string lastLine;
		string base_filename_ = "base_filename"; // what to search for 

		fin.open(iniFilePath_);
		if (fin.is_open()) {
			fin.seekg(-1, ios_base::end);
			bool keepLooping = true;
			while (keepLooping) {
				char ch;
				fin.get(ch);
				if ((int)fin.tellg() <= 1) {
					fin.seekg(0);
					getline(fin, lastLine);
					if (lastLine.find(base_filename_) != std::string::npos) {
						keepLooping = false;
					}
				}
				else if (ch == '\n') {
					getline(fin, lastLine);
					if (lastLine.find(base_filename_) != std::string::npos) {
						keepLooping = false;
					}
				}
				else {
					fin.seekg(-2, ios_base::cur);
				}
			}

			string delimiter = "=";
			base_filename = lastLine.substr(lastLine.find(delimiter) + 1, 500); // read up to 500 characters here
			cout << "Base filename: " << base_filename << '\n';
			fin.close();
		}
	}
	catch (...)
	{
		cout << "Error reading ini-file! Using default settings." << endl;
		retval = 1;
	}
	return retval;
}



