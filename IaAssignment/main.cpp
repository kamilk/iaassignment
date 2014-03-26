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

void ClassifyImage(const char* samplePath, const char* emptyPath, EventLogger& eventLogger);
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
void CheckLines(const Mat& sample, Mat& samplePreview, EventLogger& eventLogger);
bool IsLineBarrier(Line& line);
bool IsLineTrain(Line& line);

int main(int argc, char *argv[]) try
{
	const char* defaultEmptyPath = "data\\empty.png";

#ifdef HANDIN
	for (int i = 1; i < argc; i++)
	{
		EventLogger eventLogger;
		ClassifyImage(argv[i], defaultEmptyPath, eventLogger);
		eventLogger.WriteForMachine(cout, argv[i]);
	}
#else
	const char* sampleFileName;
	const char* emptyRoadFileName;
	if (argc >= 3)
	{
		sampleFileName = argv[1];
		emptyRoadFileName = argv[2];
	}
	else
	{
		sampleFileName = "samples\\all\\lc-00536.png";
		emptyRoadFileName = defaultEmptyPath;
	}

	EventLogger eventLogger;
	ClassifyImage(sampleFileName, emptyRoadFileName, eventLogger);
	eventLogger.WriteForHuman(cout);

	int key = waitKey();
	return key == 'x' ? 1 : 0;
#endif
}
catch (cv::Exception& ex)
{
	std::cerr << ex.code << std::endl;
}
catch (std::exception& ex)
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

void ClassifyImage(const char* samplePath, const char* emptyPath, EventLogger& eventLogger)
{
	// Load the reference image of an empty road
	Mat empty = imread(emptyPath, CV_LOAD_IMAGE_GRAYSCALE);
	if (empty.empty())
	{
		cout << "FAILED TO LOAD THE 'EMPTY' IMAGE!" << endl;
		return;
	}

	empty = CropCctvBorder(empty);

	// Load the image to classify
	Mat sample = imread(samplePath, CV_LOAD_IMAGE_COLOR);
	if (sample.empty())
	{
		cout << "FAILED TO LOAD THE 'SAMPLE' IMAGE!" << endl;
		return;
	}

	sample = CropCctvBorder(sample);

	// Convert the image to greyscale but keep a colourful copy which will be displayed
	// to the user, provided the build is not 'hand-in'.
	Mat samplePreview = sample.clone();
	cvtColor(sample, sample, COLOR_BGR2GRAY);

	CheckCarPresence(sample, samplePreview, empty, eventLogger);
	CheckLines(sample, samplePreview, eventLogger);

	ShowPreviewImage("orig", samplePreview);
}

// Checks if the contour is a significant contour or just some noise.
bool IsContourAcceptable(vector<Point>& contour)
{
	// Reject contours which are too small
	double area = contourArea(contour);
	if (area < 400)
		return false;

	// Reject contours which are not compact enough.
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
	polygon.push_back(Point(200, 404));
	polygon.push_back(Point(90, 305));
	return polygon;
}

// Check events: entering, leaving, ontrack
void CheckCarPresence(const Mat& sample, Mat& samplePreview, const Mat& empty, EventLogger& eventLogger)
{
	// Compute the difference between the classified image and the reference image
	// and convert it to a binary image.
	Mat image;
	absdiff(empty, sample, image);

	threshold(image, image, 70, 255, THRESH_BINARY);

	// Morphology operations - closure and opening
	Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(6, 6));
	dilate(image, image, kernel, Point(-1,-1), 3);
	erode(image, image, kernel, Point(-1, -1), 3);

	erode(image, image, kernel, Point(-1, -1), 3);
	dilate(image, image, kernel, Point(-1, -1), 3);

	// Segment the image into contours so that every 'blob' can be considered separately
	vector<vector<Point>> contours;
	findContours(image.clone(), contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

	// Create a list of polygons which define different areas of the image
	shared_ptr<Polygon> ulPolygon(new Polygon("UL", GetUpperLeftPolygon()));
	shared_ptr<Polygon> urPolygon(new Polygon("UR", GetUpperRightPolygon()));
	shared_ptr<Polygon> blPolygon(new Polygon("BL", GetBottomLeftPolygon()));
	shared_ptr<Polygon> brPolygon(new Polygon("BR", GetBottomRightPolygon()));
	shared_ptr<Polygon> trackPolygon(new Polygon("TRACK", GetTrackPolygon()));

	// All polygons have the default value of the percentage threshold of 30,
	// meaning that a contour will be considered to belong to the polygon if at least
	// 30% of its area is in this conotour (there are also other conditions,
	// see Polygon::IsObjectInIt()).
	// For the track polygon set the threshold to 20.0%.
	trackPolygon->SetPercentageThreshold(20.0);

	vector<shared_ptr<Polygon>> polygons;
	polygons.push_back(ulPolygon);
	polygons.push_back(urPolygon);
	polygons.push_back(blPolygon);
	polygons.push_back(brPolygon);
	polygons.push_back(trackPolygon);

	// Create a colourful image which will be displayed as a preview, unless HANDIN is enabled.
	Mat imageColour;
	cvtColor(image, imageColour, COLOR_GRAY2BGR);

	for (unsigned int i = 0; i < contours.size(); i++)
	{
		auto& contour = contours[i];
		// Reject contours which are e.g. too small - see IsContourAcceptable() for details.
		if (IsContourAcceptable(contour))
		{
			// Draw the contour in red on the preview image
			drawContours(imageColour, contours, i, Scalar(0, 0, 255), CV_FILLED);

			// Create an image with nothing in it except for this contour.
			// Check how many white pixels of this image belong to the polygon.
			Mat contourImage = Mat::zeros(image.rows, image.cols, CV_8U);
			drawContours(contourImage, contours, i, Scalar(255), CV_FILLED);
			for (auto& polygon : polygons)
				polygon->TestContour(contourImage);
		}
	}

#ifndef HANDIN
	// Output some debug information about contours in every polygon.
	for (auto& polygon : polygons)
		polygon->Write(cout);
#endif

	// Draw the polygons on the preview images.
	for (auto& polygon : polygons)
	{
		polygon->Draw(samplePreview, Scalar(0, 255, 0));
		polygon->Draw(imageColour, Scalar(0, 255, 0));
	}

	ShowPreviewImage("result", imageColour);

	// Classify the image according to which polygons contain relevant objects.
	eventLogger.entering = ulPolygon->IsObjectInIt() || brPolygon->IsObjectInIt();
	eventLogger.leaving = blPolygon->IsObjectInIt() || urPolygon->IsObjectInIt();
	eventLogger.ontrack = trackPolygon->IsObjectInIt();
}

// Checks if a line segment belongs to the barrier
bool IsLineBarrier(Line& line)
{
	// Reject the line segment if:
	// a) it's too far to the right
	// b) it's rotated so that it points too far down
	// The main condition is whether the line is rotated around a known origin point.
	// If the line (as the whole line, not just the segment) passes close enough to
	// this point, it is considered to be the barrier.
	return line.MaxX() < 345 && line.Theta() < 1.3 && line.DistanceFromPoint(Point2d(-13, 223.5)) < 15.0;
}

// Checks if the line segment belongs to the train
bool IsLineTrain(Line& line)
{
	// At this point we know the line does not belong to the barrier.

	// Check if the line is more less parallel to the tracks.
	if (line.Theta() < 1.15 || line.Theta() > 1.26)
		return false; // wrong angle

	// Check if the line is high enough so it is not a track
	if (line.Rho() > 270.0)
		return false; // too low

	// The shadow of the barrier is known to appear at some height, but only at
	// the left side of the image.
	if (line.MaxX() < 370 && line.Rho() > 241.5 && line.Rho() < 260.0)
		return false; // probably a shadow of the barrier

	// There is a stopping line before the crossing which also often creates a line.
	if (line.MaxX() < 270 && line.Rho() > 172.0 && line.Rho() < 188.0)
		return false; // probably the stopping line

	return true; // passed all the conditions
}

// Check for events: barrier and train
void CheckLines(const Mat& sample, Mat& samplePreview, EventLogger& eventLogger)
{
	// Compute the edge image of the sample image using Canny line detector.
	Mat edges;
	Canny(sample, edges, 70, 150, 3);
	ShowPreviewImage("canny", edges);

	// Find line segments in the image using Probabilistic Hough Transform.
	vector<Vec4i> lines;
	HoughLinesP(edges, lines, 1, CV_PI/180, 30, 100, 10 );

	// Iterate over the found line segments.
	for( size_t i = 0; i < lines.size(); i++ )
	{
		Vec4i l = lines[i];
		Point start = Point(l[0], l[1]);
		Point end = Point(l[2], l[3]);

		// Put the line coordinates into a helper object.
		Line currentLine(start, end);

		// Check if the line is an indication of some event.
		Scalar colour;
		if (IsLineBarrier(currentLine))
		{
			colour = Scalar(0, 255, 255);
			eventLogger.barrier = true;
		}
		else if (IsLineTrain(currentLine))
		{
			colour = Scalar(0, 0, 255);
			eventLogger.train = true;

#ifndef HANDIN
			cout << "rho: " << currentLine.Rho() << " theta: " << currentLine.Theta() << "maxX: " << currentLine.MaxX() << endl;
#endif
		}
		else
		{
			colour = Scalar(255, 0, 255);
		}

		// Draw a line with small rectangles at both ends.
		line( samplePreview, start, end, colour, 1, CV_AA);
		rectangle(samplePreview, Point(l[0] - 2, l[1] - 2), Point(l[0] + 2, l[1] + 2), colour, CV_FILLED);
		rectangle(samplePreview, Point(l[2] - 2, l[3] - 2), Point(l[2] + 2, l[3] + 2), colour, CV_FILLED);
	}
}