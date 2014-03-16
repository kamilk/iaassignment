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