# Basler 2 color LED tracking

Performs tracking of 2 LEDs (by default one green, one red) with a Basler camera and openCV.

## Requirements
- OpenCV 3.3.1
- Pylon 5
- Visual Studio 2017
(Tested under Win10)

The specific camera model this code was written for is the [acA2040-55uc](https://www.baslerweb.com/en/products/cameras/area-scan-cameras/ace/aca2040-55uc/) (USB3.0).
It was coupled to an [AZURE- 1236ZM](http://www.azurephotonicsus.com/products/azure-1236ZM.html) objective. This enables live tracking of two LEDs at a distance of about 200 cm in a 80x80cm box. The actual tracking runs at 50 fps (exposure time: 10 ms) at 500x500 px in a dark environment (actual image dimensions grabbed from sensor: 1500x1500 px). See end of this README for link to a short [demonstration video](#video-time).
The code is based on an example provided by Basler: "Getting Started with pylon and OpenCV".
By default (see parameters below), online tracking is enabled and each grabbed frame is written to a video container, which gets saved at the end together with a tracking results .csv that contains timestamps for every analyzed frame and tracked coordinates of both LEDs.

## Timing considerations
At startup the camera is put into hardware trigger mode and expects a (regular) TTL trigger input on its opto-coupled input line (e.g. 50 Hz or lower coming from an Arduino). It also outputs a TTL via its opto-coupled output line while it is exposing each triggered image. Accurate timestamps are captured for every image as part of a "data chunk" (see Basler definitions) attached to every grabbed frame (internal clock frequency: 1 GHz).
The actual tracking algorithm is "as easy as possible": Conversion to HSV -> thresholding for two LEDs -> morphological operation -> connected component extraction -> centroids. This is performed on a scaled version of the grabbed image (1500 x 1500px for me, but adjustable).
At 500x500px, 50 fps, the computational demands are quite high and it might be a good idea to start with trigger frequencies < 50 Hz. The grab strategy is set to *"OnebyOne"* meaning that no frames are lost if the acquisition PC can take it and the default frame buffer (25 images) is not overrun. This works fine on my computer, but the jitter induced by processing every frame can be substantial. See the example Jupyter notebook with analysis output below. It is important to note though that *the actual acquisition is not affected* (recorded frame times remain accurate).
Writing images out for every grabbed frame (*saveImages* option, see below) is not advisable: I didn't manage to speed that process up and everything slows down considerably. For quick test images and to adjust threshold parameters for tracking, this is however a quick and dirty way to get an image out.
If things are too slow / frames are lost:
- Trigger < 50 Hz
- Kill the video output
- lower the resolution of tracking. This is currently done through a "scale_factor" (see *Tracker_params.ini* below, e.g. 1500/scale_factor=500)

## Installation
The current version of the tracker can be found [here](Tracker/current%20version).
It is written in C++, requires a correctly installed *opencv* and *pylon* environment. Check the [PDF](Tracker/AW00136801000_Getting_Started_with_pylon5_and_OpenCV.PDF) from Basler for a getting started guide and setup your environment variables accordingly. It explains the basics of how to use the Pylon SDK to access and manipulate camera parameters.
Check that your camera works and parameters can be accessed by starting the Pylon viewer software.
You have to set up the parameters for color thresholding according to the LEDs you use. All thresholds are exposed in the **Tracker_params.ini** (see below).

## Tracker_params.ini
Some tracker settings can be accessed through the [Tracker_params.ini](Tracker/Current%20version/x64/Release/Tracker_params.ini) in the application base folder. An explanation of all parameters follows:

- **grabber_timeout=2000** // Time before program aborts when no trigger input is received
- **ExposureTime=10000** // Exposure time in microseconds
- **TriggerDelay=0** // Delay in microseconds between the receipt of a hardware trigger signal and the moment the trigger becomes active
- **Gain=2** // Digital gain - helped the tracking to set it to 2, shouldn't be much higher
- **DebouncerTime=100** // Debouncing of the input line TTL
- **saveImages=0** // **LEAVE at 0!** Slows down tracking considerably! Outputs ".jpg" images under /export
- **recordVideo=1** // Outputs ".avi" file under /export, which has the same size as the tracked (not grabbed) images
- **acq_frame_height=1500** // actual height of image (ROI) grabbed from sensor - adjust to your tracked area
- **acq_frame_width=1500** // actual width of image (ROI) grabbed from sensor
- **offset_x=0** // ROI offset in x
- **offset_y=0** // ROI offset in y
- **scale_factor=3** // Resize image (divide dimensions by scale_factor) to speed up tracking
- **red_h_low=116** // HSV threshold values for the two LEDs - adjust to the color of your LEDs
- **red_s_low=233**
- **red_v_low=48**
- **red_h_high=129**
- **red_s_high=255**
- **red_v_high=255**
- **green_h_low=0**
- **green_s_low=94**
- **green_v_low=85**
- **green_h_high=74**
- **green_s_high=255**
- **green_v_high=255**
- **playback_speed_video=50** // Adjust to trigger frequency to playback video in realtime

## Output
By default (*saveImages* and *recordVideo* both 0) a .csv file with timestamps and tracking results of the acquisition is saved under /export.
Have a look at the [example Jupyter notebook](notebooks/Trackings_tests.ipynb) (python 3.6) for analysis of output .csv files. It shows how to work with the .csv and plots recorded timestamps / shows tracking results.

Example header of a .csv:

Columns of the .csv:
- **frame** - frame number from 1 to total number of registered frames
- **cam_timestamp** - internal timestamp (in ticks, nanoseconds) of the moment when image acquisition was triggered (transferred as "data chunk")
- **sys_timestamp** - system timestamp (nanoseconds) for every processing loop. This will jitter a lot depending on the speed of your processing  
- **green_x** - x coordinate of green LED
- **green_y** - y coordinate of green LED
- **red_x** - x coordinate of red LED
- **red_y** - y coordinate of red LED

If videos are exported, they are saved under /export as MJPG compressed .avi.
If images are exported, they are saved under /export as .jpg.
Both videos and images have the dimensions of scaled (in my case 500x500 px) frames.

### Video time
2 LEDs on a string, tracked 500x500 px, 50fps.
[![Watch a shaky video on Vimeo](https://i.imgur.com/EmlkPaM.png)](https://vimeo.com/245192287 "Little presentation on vimeo - Click to Watch!")
