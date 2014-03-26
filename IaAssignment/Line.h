#pragma once

#include <opencv.hpp>

// Stores information about a line segment and computes some of its mathematical properties.
class Line
{
public:
	Line(cv::Point start, cv::Point end) : start(start), end(end) {}

	// The difference in x coordianates (can be negative)
	double Width() { return end.x - start.x; }
	// The difference in y coordianates (can be negative)
	double Height() { return end.y - start.y; }
	
	// If the line is expressed as y=mx+b, Slope() returns the m.
	double Slope() { return Height() / Width(); }
	// If the line is expressed as y=mx+b, Slope() returns the b.
	double Displacement() { return start.y - Slope() * start.x; }
	
	// Computes the shortest distance between a point and the line
	// (the whole straight line, not just the segment)
	double DistanceFromPoint(cv::Point2d point)
	{
		return abs(Slope() * point.x - point.y + Displacement()) / sqrt(Slope()*Slope() + 1);
	}
	
	// Computes the length of the segment.
	double Length()
	{
		double w = Width();
		double h = Height();
		return sqrt(w*w + h*h);
	}
	
	// Computes the rotation of the line: 0 - vertical, pi/2 - horizontal
	// (like the rho returned by cv::HoughLines().
	double Rho() { return Displacement() * Width() / Length(); }
	// Computes the distance of the line from the origin point (0,0) (top-left)
	// (like the theta returned by cv::HoughLines().
	double Theta() { return -atan(Width() / Height()); }

	// Returns the maximum x coordinate of the segment.
	int MaxX() { return std::max(end.x, start.x); }

	cv::Point start;
	cv::Point end;
};