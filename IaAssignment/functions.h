#pragma once

#include <opencv.hpp>
#include <vector>

int GetNumberOfWhitePixelsInPolygon(const cv::Mat& image, const std::vector<cv::Point>& polygon);
int GetNumberOfWhitePixels(const cv::Mat& image);
cv::Mat ExpandToOptimalSize(const cv::Mat& I);
cv::Mat ComputeDft(const cv::Mat& image);
cv::Mat ComputeIdft(const cv::Mat& transformImage);
void DisplayDft(const std::string& winname, const cv::Mat& complexI);