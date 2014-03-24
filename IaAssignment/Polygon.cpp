#include "Polygon.h"
#include "functions.h"

using namespace std;
using namespace cv;

Polygon::Polygon(const std::string& name, std::vector<cv::Point>&& polygon) : _name(name)
{
	_polygon.push_back(polygon);
}


Polygon::~Polygon()
{
}

void Polygon::TestContour(const Mat& contourImage)
{
	ContourPolygonStats stats;
	stats.areaInPolygon = GetNumberOfWhitePixelsInPolygon(contourImage, _polygon[0]);
	stats.totalArea = GetNumberOfWhitePixels(contourImage);
	stats.percentageInPolygon = (double)stats.areaInPolygon / stats.totalArea * 100.0;
	_contourStats.push_back(stats);
}

void Polygon::Write(std::ostream& stream)
{
	stream << "Polygon " << _name << ":" << endl;

	for (auto& stat : _contourStats)
	{
		if (stat.areaInPolygon == 0)
			continue;

		stream << stat.areaInPolygon
			<< " / " << stat.totalArea
			<< " (" << stat.percentageInPolygon << "%)"
			<< endl;
	}

	stream << endl;
}

void Polygon::Draw(cv::Mat& image, cv::Scalar colour)
{
	drawContours(image, _polygon, -1, colour, 2);
}

bool Polygon::IsObjectInIt()
{
	for (auto& stat : _contourStats)
	{
		if (stat.areaInPolygon > 15000 || (stat.areaInPolygon > 2000 && stat.percentageInPolygon > 30.0))
			return true;
	}

	return false;
}