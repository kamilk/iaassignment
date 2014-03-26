#pragma once

// Contains statistics of a portion of a contour which belongs to a polygon
struct ContourPolygonStats
{
	int areaInPolygon;
	int totalArea;
	double percentageInPolygon;
};