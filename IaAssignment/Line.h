#pragma once

#include <opencv.hpp>

class Line
{
public:
	Line(cv::Point start, cv::Point end) : start(start), end(end) {}

	double Width() { return end.x - start.x; }
	double Height() { return end.y - start.y; }
	double Slope() { return Height() / Width(); }
	double Displacement() { return start.y - Slope() * start.x; }
	double DistanceFromPoint(cv::Point2d point)
	{
		return abs(Slope() * point.x - point.y + Displacement()) / sqrt(Slope()*Slope() + 1);
	}

	cv::Point start;
	cv::Point end;
};