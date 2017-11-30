# Basler 2 color LED tracking

Perform tracking of 2 LEDs (by default one green, one red) with a Basler camera and openCV.
The specific camera model this code was written for is the [acA2040-55uc](https://www.baslerweb.com/en/products/cameras/area-scan-cameras/ace/aca2040-55uc/) (USB3.0).
It was coupled to an [AZURE- 1236ZM](http://www.azurephotonicsus.com/products/azure-1236ZM.html) objective. This enables live tracking of two LEDs at a distance of about 200 cm in a 80x80cm box. The actual tracking runs at 50 fps (exposure time: 10 ms) at 500x500 px in a dark environment (actual image dimensions grabbed from sensor: 1500x1500 px). See end of this readme for link to a short video.
The code is based on an example provided by Basler: "Getting Started with pylon and OpenCV".

The camera is in hardware trigger mode and expects a (regular) TTL input on its opto-coupled input line (e.g. 50 Hz coming from an Arduino). It outputs a TTL while exposing each triggered image via its opto-coupled output line. 

The current version of the tracker can be found under [/Tracker/Current version/](/Tracker/Current version/).

[Example Jupyter notebook](notebooks/Trackings_tests.ipynb) (python 3.6) for analysis of output .csv files. Loads .csv and plots recorded timestamps and shows tracking result.

## Tracker_params.ini
Some tracker settings can be accessed through the [Tracker/Current version/x64/Release/Tracker_params.ini](Tracker_params.ini) in the application base folder. An explanation of all parameters follows:

grabber_timeout=2000 // Time before program aborts when no trigger input is received
ExposureTime=10000 // Exposure time in microseconds
TriggerDelay=0 //
Gain=2
DebouncerTime=100 // line in line debouncing
saveImages=0 // LEAVE OFF (0) FOR NOW
recordVideo=1 // LEAVE OF (0) FOR NOW
acq_frame_height=1500
acq_frame_width=1500
offset_x=0
offset_y=0
scale_factor=3 // make 3x smaller for tracking
red_h_low=116 // HSV threshold values for the two LEDs
red_s_low=233
red_v_low=48
red_h_high=129
red_s_high=255
red_v_high=255
green_h_low=0
green_s_low=94
green_v_low=85
green_h_high=74
green_s_high=255
green_v_high=255
playback_speed_video=50 // adjust to trigger framerate



[![Watch a shaky video on Vimeo](https://i.imgur.com/EmlkPaM.png)](https://vimeo.com/245192287 "Little presentation on vimeo - Click to Watch!")
