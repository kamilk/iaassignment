#include <iostream>
#include <opencv.hpp>

using namespace std;
using namespace cv;

Mat CropCctvBorder(Mat& image)
{
	return image(Range(56, 460), Range(25, 690));
}

int main(int argc, char *argv[]) try
{
	Mat empty = imread("data\\empty.png", CV_LOAD_IMAGE_GRAYSCALE);
	empty = CropCctvBorder(empty);

	Mat sample = imread("samples\\all\\lc-00564.png", CV_LOAD_IMAGE_GRAYSCALE);
	sample = CropCctvBorder(sample);
	imshow("orig", sample);

	Mat result;
	absdiff(empty, sample, result);

	threshold(result, result, 70, 255, THRESH_BINARY);
	imshow("inter", result);

	Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(4, 4));
	dilate(result, result, kernel, Point(-1,-1), 3);
	erode(result, result, kernel, Point(-1, -1), 3);

	imshow("result", result);
	waitKey();
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