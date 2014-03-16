#pragma once

#include <opencv.hpp>
#include <vector>

int GetNumberOfWhitePixelsInPolygon(const cv::Mat& image, const std::vector<cv::Point>& polygon);
int GetNumberOfWhitePixels(const cv::Mat& image);