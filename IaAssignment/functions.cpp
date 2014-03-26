#include "functions.h"

using namespace std;
using namespace cv;

int GetNumberOfWhitePixelsInPolygon(const Mat& image, const vector<Point>& polygon)
{
	int result = 0;

	for (int i = 0; i < image.rows; i++)
		for (int j = 0; j < image.cols; j++)
		{
			if (image.at<uchar>(i, j) > 0 && pointPolygonTest(polygon, Point2f(j, i), false) >= 0)
				++result;
		}

		return result;
}

int GetNumberOfWhitePixels(const Mat& image)
{
	int result = 0;

	for (int i = 0; i < image.rows; i++)
		for (int j = 0; j < image.cols; j++)
		{
			if (image.at<uchar>(i, j) > 0)
				++result;
		}

		return result;
}

void DrawLinePolar(cv::Mat& image, float rho, float theta, Scalar colour)
{
	// adapted from http://docs.opencv.org/doc/tutorials/imgproc/imgtrans/hough_lines/hough_lines.html
	Point pt1, pt2;
	double a = cos(theta), b = sin(theta);
	double x0 = a*rho, y0 = b*rho;
	pt1.x = cvRound(x0 + 1000*(-b));
	pt1.y = cvRound(y0 + 1000*(a));
	pt2.x = cvRound(x0 - 1000*(-b));
	pt2.y = cvRound(y0 - 1000*(a));

	line( image, pt1, pt2, colour, 1, CV_AA);
}

// A thin wrapper around cv::imshow(). In HANDIN mode, the function does nothing.
void ShowPreviewImage(const std::string& winname, cv::Mat& image)
{
#ifndef HANDIN
	imshow(winname, image);
#endif
}