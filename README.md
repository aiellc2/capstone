### Capstone

Earlier work ported from Matlab to C++ for higher performance.

Uses [OpenCV](http://opencv.org) (required to compile)

### Existing functionality

Currently implements the same functionality we had in Matlab, with back/forward gestures. Current keypress code is specific to Mac OS for controlling iBooks, but can be easily changed.

### New functionality

Performance has been improved by using multithreading. One thread captures images from the camera and places them in a queue. Another thread takes these images from the queue and does the processing.

OpenCV documentation suggests using the [HSV colour space] (https://en.wikipedia.org/wiki/HSL_and_HSV) for colour tracking. There are now some on-screen sliders that let you tweak the colour settings to get a good match depending on the condition.

There's another code that tracks two colours, no gestures there yet though.

Some sample OpenCV code:

http://docs.opencv.org/trunk/df/d9d/tutorial_py_colorspaces.html
http://opencv-srf.blogspot.ca/2010/09/object-detection-using-color-seperation.html
