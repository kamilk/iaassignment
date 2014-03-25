#include <iostream>
#include <opencv.hpp>
#include <fstream>
#include <memory>
#include "Polygon.h"
#include "functions.h"
#include "EventLogger.h"
#include "Line.h"

using namespace std;
using namespace cv;

Mat CropCctvBorder(Mat& image);
bool IsContourAcceptable(vector<Point>& contour);
vector<Point> GetUpperLeftPolygon();
vector<Point> GetUpperRightPolygon();
vector<Point> GetBottomLeftPolygon();
vector<Point> GetBottomRightPolygon();
vector<Point> GetTrackPolygon();
bool CheckTrain(const Mat& sample, Mat& samplePreview, const Mat& edges);
void CheckCarPresence(const Mat& sample, Mat& samplePreview, const Mat& empty, EventLogger& eventLogger);
bool CheckBarrier(const Mat& sample, Mat& samplePreview, const Mat& edges);

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
		sampleFileName = "samples\\all\\lc-00442.png";
		emptyRoadFileName = "data\\empty.png";
	}

	Mat empty = imread(emptyRoadFileName, CV_LOAD_IMAGE_GRAYSCALE);
	empty = CropCctvBorder(empty);

	Mat sample = imread(sampleFileName, CV_LOAD_IMAGE_COLOR);
	sample = CropCctvBorder(sample);
	Mat samplePreview = sample.clone();
	cvtColor(sample, sample, COLOR_BGR2GRAY);

	Mat edges;
	Canny(sample, edges, 70, 150, 3);
	imshow("canny", edges);

	EventLogger eventLogger;
	CheckCarPresence(sample, samplePreview, empty, eventLogger);
	eventLogger.train = CheckTrain(sample, samplePreview, edges);
	eventLogger.barrier = CheckBarrier(sample, samplePreview, edges);

	imshow("orig", samplePreview);

	eventLogger.Write(cout);

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

vector<Point> GetTrackPolygon()
{
	vector<Point> polygon;
	polygon.push_back(Point(175, 255));
	polygon.push_back(Point(395, 145));
	polygon.push_back(Point(520, 80));
	polygon.push_back(Point(665, 160));
	polygon.push_back(Point(580, 225));
	polygon.push_back(Point(315, 404));
	return polygon;
}

bool CheckTrain(const Mat& sample, Mat& samplePreview, const Mat& edges)
{
	Mat hough = Mat::zeros(sample.size(), CV_8U);

	vector<Vec2f> lines;
	HoughLines(edges, lines, 1, CV_PI/180, 120, 0, 0 );

	bool isTrain = false;
	for( size_t i = 0; i < lines.size(); i++ )
	{
		float rho = lines[i][0], theta = lines[i][1];
		Scalar colour;
		if (rho < 280.0f && theta > 1.15f && theta < 1.23f)
		{
			isTrain = true;
			colour = Scalar(0, 0, 255);
		}
		else
		{
			colour = Scalar(255, 0, 0);
		}

		DrawLinePolar(samplePreview, rho, theta, colour);
	}

	return isTrain;
}

void CheckCarPresence(const Mat& sample, Mat& samplePreview, const Mat& empty, EventLogger& eventLogger)
{
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
	shared_ptr<Polygon> trackPolygon(new Polygon("TRACK", GetTrackPolygon()));

	vector<shared_ptr<Polygon>> polygons;
	polygons.push_back(ulPolygon);
	polygons.push_back(urPolygon);
	polygons.push_back(blPolygon);
	polygons.push_back(brPolygon);
	polygons.push_back(trackPolygon);

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
		polygon->Draw(samplePreview, Scalar(0, 255, 0));
		polygon->Draw(imageColour, Scalar(0, 255, 0));
	}

	imshow("result", imageColour);

	eventLogger.entering = ulPolygon->IsObjectInIt() || brPolygon->IsObjectInIt();
	eventLogger.leaving = blPolygon->IsObjectInIt() || urPolygon->IsObjectInIt();
	eventLogger.ontrack = trackPolygon->IsObjectInIt();
}

bool IsLineBarrier(Line& line)
{
	return line.DistanceFromPoint(Point2d(-13, 223.5)) < 15.0;
}

bool CheckBarrier(const Mat& sample, Mat& samplePreview, const Mat& edges)
{
	vector<Vec4i> lines;
	HoughLinesP(edges, lines, 1, CV_PI/180, 30, 100, 10 );

	bool isBarrier = false;
	for( size_t i = 0; i < lines.size(); i++ )
	{
		Vec4i l = lines[i];
		Point start = Point(l[0], l[1]);
		Point end = Point(l[2], l[3]);

		Line currentLine(start, end);

		Scalar colour;
		if (IsLineBarrier(currentLine))
		{
			colour = Scalar(0, 255, 255);
			isBarrier = true;
		}
		else
		{
			colour = Scalar(255, 0, 255);
		}

		line( samplePreview, start, end, colour, 1, CV_AA);
		rectangle(samplePreview, Point(l[0] - 2, l[1] - 2), Point(l[0] + 2, l[1] + 2), Scalar(0, 0, 255), CV_FILLED);
		rectangle(samplePreview, Point(l[2] - 2, l[3] - 2), Point(l[2] + 2, l[3] + 2), Scalar(0, 0, 255), CV_FILLED);
	}

	return isBarrier;
}