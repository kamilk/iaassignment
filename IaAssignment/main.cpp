#include <iostream>
#include <opencv.hpp>
#include <fstream>
#include <memory>
#include "Polygon.h"

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

vector<Point> GetUpperLeftPolygon()
{
	vector<Point> polygon;
	polygon.push_back(Point(0, 0));
	polygon.push_back(Point(70, 0));
	polygon.push_back(Point(395, 145));
	polygon.push_back(Point(175, 255));
	polygon.push_back(Point(0, 140));
	return polygon;
}

vector<Point> GetUpperRightPolygon()
{
	vector<Point> polygon;
	polygon.push_back(Point(70, 0));
	polygon.push_back(Point(395, 145));
	polygon.push_back(Point(520, 80));
	polygon.push_back(Point(285, 0));
	return polygon;
}

vector<Point> GetBottomLeftPolygon()
{
	vector<Point> polygon;
	polygon.push_back(Point(580, 225));
	polygon.push_back(Point(665, 275));
	polygon.push_back(Point(665, 404));
	polygon.push_back(Point(315, 404));
	return polygon;
}

vector<Point> GetBottomRightPolygon()
{
	vector<Point> polygon;
	polygon.push_back(Point(580, 225));
	polygon.push_back(Point(665, 275));
	polygon.push_back(Point(665, 160));
	return polygon;
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
		sampleFileName = "samples\\all\\lc-00241.png";
		emptyRoadFileName = "data\\empty.png";
	}

	Mat empty = imread(emptyRoadFileName, CV_LOAD_IMAGE_GRAYSCALE);
	empty = CropCctvBorder(empty);

	Mat sample = imread(sampleFileName, CV_LOAD_IMAGE_GRAYSCALE);
	sample = CropCctvBorder(sample);

	Mat image;
	absdiff(empty, sample, image);
	
	threshold(image, image, 70, 255, THRESH_BINARY);

	Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(6, 6));
	dilate(image, image, kernel, Point(-1,-1), 3);
	erode(image, image, kernel, Point(-1, -1), 3);

	erode(image, image, kernel, Point(-1, -1), 3);
	dilate(image, image, kernel, Point(-1, -1), 3);

	vector<vector<Point>> contours;
	findContours(image.clone(), contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

	shared_ptr<Polygon> ulPolygon(new Polygon("UL", GetUpperLeftPolygon()));
	shared_ptr<Polygon> urPolygon(new Polygon("UR", GetUpperRightPolygon()));
	shared_ptr<Polygon> blPolygon(new Polygon("BL", GetBottomLeftPolygon()));
	shared_ptr<Polygon> brPolygon(new Polygon("BR", GetBottomRightPolygon()));

	vector<shared_ptr<Polygon>> polygons;
	polygons.push_back(ulPolygon);
	polygons.push_back(urPolygon);
	polygons.push_back(blPolygon);
	polygons.push_back(brPolygon);

	Mat imageColour;
	cvtColor(image, imageColour, COLOR_GRAY2BGR);

	for (unsigned int i = 0; i < contours.size(); i++)
	{
		auto& contour = contours[i];
		if (IsContourAcceptable(contour))
		{
			drawContours(imageColour, contours, i, Scalar(0, 0, 255), CV_FILLED);

			Mat contourImage = Mat::zeros(image.rows, image.cols, CV_8U);
			drawContours(contourImage, contours, i, Scalar(255), CV_FILLED);
			for (auto& polygon : polygons)
				polygon->TestContour(contourImage);
		}
	}

	for (auto& polygon : polygons)
		polygon->Write(cout);

	for (auto& polygon : polygons)
	{
		polygon->Draw(sample, Scalar(0));
		polygon->Draw(imageColour, Scalar(0, 255, 0));
	}

	if (ulPolygon->IsObjectInIt() || brPolygon->IsObjectInIt())
		cout << "ENTERING!!!" << endl;
	if (blPolygon->IsObjectInIt() || urPolygon->IsObjectInIt())
		cout << "LEAVING!!!" << endl;

	cout << endl;

	imshow("orig", sample);
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