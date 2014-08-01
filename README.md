OpenCV_ImageBackgroundRemoval
=============================

Using openCV to remove image background and shadows.


The algorithm used in this repository are largely adapted from this article:
    http://developers.lyst.com/data/images/2014/02/13/background-removal/
    

For this implementation, the steps used in shadow and background removal are:

1. Remove shadows by converting to HSV and setting V to a fixed value
2. Convert to grayscale and normalize
3. Apply gaussian blur and Canny edge detector
4. Dilate to close gaps
5. Flood fill the image from borders
6. Erode to account for previous dilation
7. Find largest contour
8. Mask original image

