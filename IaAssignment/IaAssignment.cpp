#include <iostream>
#include <opencv.hpp>
#include <fstream>

using namespace std;
using namespace cv;

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

int GetNumberOfWhitePixelsInPolygon(const Mat& image, const vector<Point>& polygon)
{
	int result = 0;

	for (int i = 0; i < image.rows; i++)
	for (int j = 0; j < image.cols; j++)
	{
		if (image.at<uchar>(i, j) > 0 && pointPolygonTest(polygon, Point2f(j, i), false) >= 0)
			++result;
	}

	return result;
}

int GetNumberOfWhitePixels(const Mat& image)
{
	int result = 0;

	for (int i = 0; i < image.rows; i++)
	for (int j = 0; j < image.cols; j++)
	{
		if (image.at<uchar>(i, j) > 0)
			++result;
	}

	return result;
}

vector<Mat> backgrounds;

double gaussianFunction(double x, double sigma)
{
	// http://www.cplusplus.com/forum/beginner/62864/#msg340340
	static const double pi = 3.14159265;
	static const double mu = 0.0;
	return exp(-1 * (x - mu) * (x - mu) / (2 * sigma * sigma)) / (sigma * sqrt(2 * pi));
}

bool IsPixelForeground(Mat image, int i, int j)
{
	double backgroundLightness = 0.0;
	for (auto& background : backgrounds)
	{
		backgroundLightness += background.at<Vec3b>(i, j)[1];
	}

	backgroundLightness /= backgrounds.size();

	Vec3b pixel = image.at<Vec3b>(i, j);
	double lightnessRatio = pixel[1] / backgroundLightness;
	if (lightnessRatio >= 0.8 && lightnessRatio <= 1.3)
		return false;

	double chanceOfXIfBackground = 0.0;
	for (auto& background : backgrounds)
	{
		Vec3b backgroundPixel = background.at<Vec3b>(i, j);
		Vec3b diff = pixel - backgroundPixel;
		const double sigma = 4;
		chanceOfXIfBackground += gaussianFunction(diff[0], sigma) * gaussianFunction(diff[2], sigma);
	}

	chanceOfXIfBackground /= backgrounds.size();
	const double chanceOfBackground = 0.7;
	const double chanceOfForeground = 1.0 - chanceOfBackground;
	const double chanceOfXIfForeground = 0.01;
	double chanceOfBackgroundIfX = chanceOfXIfBackground * chanceOfBackground
		/ (chanceOfXIfBackground * chanceOfBackground + chanceOfXIfForeground * chanceOfForeground);

	return chanceOfBackgroundIfX > 0.5;
}

int main(int argc, char *argv[]) try
{
	for (int i = 1; i <= 18; i++)
	{
		ostringstream backgroundPath;
		backgroundPath << "data\\background\\" << i << ".png";
		Mat background = imread(backgroundPath.str(), CV_LOAD_IMAGE_COLOR);

		background = CropCctvBorder(background);
		cvtColor(background, background, COLOR_BGR2XYZ);

		backgrounds.push_back(background);
	}


	const char* sampleFileName;
	const char* emptyRoadFileName;
	if (argc >= 3)
	{
		sampleFileName = argv[1];
		emptyRoadFileName = argv[2];
	}
	else
	{
		sampleFileName = "samples\\all\\lc-00473.png";
		emptyRoadFileName = "data\\empty.png";
	}

	Mat empty = imread(emptyRoadFileName, CV_LOAD_IMAGE_COLOR);
	empty = CropCctvBorder(empty);

	Mat sample = imread(sampleFileName, CV_LOAD_IMAGE_COLOR);
	sample = CropCctvBorder(sample);

	imshow("orig", sample);

	cvtColor(empty, empty, COLOR_BGR2XYZ);
	cvtColor(sample, sample, COLOR_BGR2XYZ);

	Mat diff;
	absdiff(empty, sample, diff);
	
	Mat image(diff.rows, diff.cols, CV_8U);
	for (int i = 0; i < diff.rows; i++)
	for (int j = 0; j < diff.cols; j++)
	{
		Vec3b xyz = diff.at<Vec3b>(i, j);
		image.at<uchar>(i, j) = IsPixelForeground(diff, i, j) ? 255 : 0;
	}

	Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(4, 4));
	dilate(image, image, kernel, Point(-1,-1), 3);
	erode(image, image, kernel, Point(-1, -1), 3);

	erode(image, image, kernel, Point(-1, -1), 3);
	dilate(image, image, kernel, Point(-1, -1), 3);

	vector<vector<Point>> contours;
	findContours(image.clone(), contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

	vector<vector<Point>> polygon;
	polygon.push_back(vector<Point>());
	polygon[0].push_back(Point(0, 0));
	polygon[0].push_back(Point(70, 0));
	polygon[0].push_back(Point(415, 170));
	polygon[0].push_back(Point(190, 285));
	polygon[0].push_back(Point(0, 140));
	drawContours(sample, polygon, 0, Scalar(0, 255, 0), 3);

	Mat imageColour;
	cvtColor(image, imageColour, COLOR_GRAY2BGR);

	for (int i = 0; i < contours.size(); i++)
	{
		auto& contour = contours[i];
		if (IsContourAcceptable(contour))
		{
			drawContours(imageColour, contours, i, Scalar(0, 0, 255), CV_FILLED);

			Mat contourImage = Mat::zeros(image.rows, image.cols, CV_8U);
			drawContours(contourImage, contours, i, Scalar(255), CV_FILLED);
			int number = GetNumberOfWhitePixelsInPolygon(contourImage, polygon[0]);
			double area = GetNumberOfWhitePixels(contourImage);
			double ratio = (double)number / area * 100.0;
			cout << number << " / " << area << " (" << ratio << "%)" << endl;
		}
	}

	imshow("result", imageColour);
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