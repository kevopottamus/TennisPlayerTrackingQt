TennisPlayerTrackingQt
======================
Uses OpenCV C++ library to track and report positions and paths of tennis players from video.
A particle filter is used to compare color histograms with initial user input to follow players through frames.
When tracking is lost, reinitialize using a haarcascade (haarpedestrians.xml)
