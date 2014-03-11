#include <iostream>
#include <opencv.hpp>
#include <fstream>

using namespace std;
using namespace cv;

Mat CropCctvBorder(Mat& image)
{
	return image(Range(56, 460), Range(25, 690));
}

bool IsContourAcceptable(vector<Point>& contour)
{
	double area = contourArea(contour);
	if (area < 400)
		return false;

	double perimeter = arcLength(contour, true);
	double compactness = perimeter * perimeter / area;
	if (compactness > 30 * CV_PI)
		return false;

	return true;
}

int main(int argc, char *argv[]) try
{
	const char* sampleFileName;
	const char* emptyRoadFileName;
	if (argc >= 3)
	{
		sampleFileName = argv[1];
		emptyRoadFileName = argv[2];
	}
	else
	{
		sampleFileName = "samples\\all\\lc-00579.png";
		emptyRoadFileName = "data\\empty.png";
	}

	Mat empty = imread(emptyRoadFileName, CV_LOAD_IMAGE_GRAYSCALE);
	empty = CropCctvBorder(empty);

	Mat sample = imread(sampleFileName, CV_LOAD_IMAGE_GRAYSCALE);
	sample = CropCctvBorder(sample);
	imshow("orig", sample);

	Mat image;
	absdiff(empty, sample, image);
	
	threshold(image, image, 70, 255, THRESH_BINARY);

	Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(4, 4));
	dilate(image, image, kernel, Point(-1,-1), 3);
	erode(image, image, kernel, Point(-1, -1), 3);

	erode(image, image, kernel, Point(-1, -1), 3);
	dilate(image, image, kernel, Point(-1, -1), 3);

	vector<vector<Point>> contours;
	findContours(image.clone(), contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

	Mat imageColour;
	cvtColor(image, imageColour, COLOR_GRAY2BGR);

	for (int i = 0; i < contours.size(); i++)
	{
		auto& contour = contours[i];
		if (IsContourAcceptable(contour))
			drawContours(imageColour, contours, i, Scalar(0, 0, 255), CV_FILLED);
	}

	imshow("result", imageColour);
	int key = waitKey();
	return key == 'x' ? 1 : 0;
}
catch (cv::Exception &ex)
{
	std::cerr << ex.code << std::endl;
}
catch (std::exception &ex)
{
	std::cerr << "Exception of type " << typeid(ex).name() << " with message " << ex.what() << std::endl;
}
catch (...)
{
	std::cerr << "Unexpected error!" << std::endl;
}