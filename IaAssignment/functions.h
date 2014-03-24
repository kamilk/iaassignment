#pragma once

#include <opencv.hpp>
#include <vector>

int GetNumberOfWhitePixelsInPolygon(const cv::Mat& image, const std::vector<cv::Point>& polygon);
int GetNumberOfWhitePixels(const cv::Mat& image);
void DrawLinePolar(cv::Mat& image, float rho, float theta, cv::Scalar colour, int thickness = 1);