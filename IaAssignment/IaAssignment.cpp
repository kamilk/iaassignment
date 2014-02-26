#include <iostream>
#include <opencv.hpp>

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

	double perimiter = arcLength(contour, true);
	double compactness = perimiter * perimiter / area;
	if (compactness > 30 * CV_PI)
		return false;

	return true;
}

int main(int argc, char *argv[]) try
{
	Mat empty = imread("data\\empty.png", CV_LOAD_IMAGE_GRAYSCALE);
	empty = CropCctvBorder(empty);

	Mat sample = imread("samples\\all\\lc-00579.png", CV_LOAD_IMAGE_GRAYSCALE);
	sample = CropCctvBorder(sample);
	imshow("orig", sample);

	Mat image;
	absdiff(empty, sample, image);
	
	threshold(image, image, 70, 255, THRESH_BINARY);

	Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(4, 4));
	dilate(image, image, kernel, Point(-1,-1), 3);
	erode(image, image, kernel, Point(-1, -1), 3);

	imshow("inter", image);

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
	waitKey();
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