#pragma once

#include <ios>
#include <vector>
#include <opencv.hpp>
#include <string>
#include "ContourPolygonStats.h"

class Polygon
{
public:
	Polygon(const std::string& name, std::vector<cv::Point>&& polygon);
	~Polygon();
	void TestContour(const cv::Mat& contourImage);
	void Write(std::ostream& stream);
	void Draw(cv::Mat& image, cv::Scalar colour);
	bool IsObjectInIt();
	void SetPercentageThreshold(double threshold) { _percentageThreshold = threshold; }

private:
	std::string _name;
	std::vector<std::vector<cv::Point>> _polygon;
	std::vector<ContourPolygonStats> _contourStats;
	double _percentageThreshold;
};

