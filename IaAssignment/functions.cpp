#include "functions.h"

using namespace std;
using namespace cv;

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

// adapted from http://docs.opencv.org/doc/tutorials/core/discrete_fourier_transform/discrete_fourier_transform.html
Mat ExpandToOptimalSize(const Mat& I)
{
	Mat padded;                            //expand input image to optimal size
	int m = getOptimalDFTSize(I.rows);
	int n = getOptimalDFTSize(I.cols); // on the border add zero values
	copyMakeBorder(I, padded, 0, m - I.rows, 0, n - I.cols, BORDER_CONSTANT, Scalar::all(0));
	return padded;
}

Mat ComputeDft(const Mat& image)
{
	Mat padded = ExpandToOptimalSize(image);
	cv::Mat imageFloat;
	padded.convertTo(imageFloat, CV_64F);

	cv::Mat transform;
	cv::dft(imageFloat, transform, DFT_COMPLEX_OUTPUT);
	return transform;
}

Mat ComputeIdft(const Mat& transformImage)
{
	Mat invertedImage;
	dft(transformImage, invertedImage, DFT_SCALE | DFT_INVERSE | DFT_REAL_OUTPUT);

	Mat finalImage;
	invertedImage.convertTo(finalImage, CV_8UC1);
	return finalImage;
}

// adapted from http://docs.opencv.org/doc/tutorials/core/discrete_fourier_transform/discrete_fourier_transform.html
void DisplayDft(const string& winname, const Mat& complexI)
{
	// compute the magnitude and switch to logarithmic scale
	// => log(1 + sqrt(Re(DFT(I))^2 + Im(DFT(I))^2))
	Mat planes[2];
	split(complexI, planes);                   // planes[0] = Re(DFT(I), planes[1] = Im(DFT(I))
	magnitude(planes[0], planes[1], planes[0]);// planes[0] = magnitude
	Mat magI = planes[0];

	magI += Scalar::all(1);                    // switch to logarithmic scale
	log(magI, magI);

	// crop the spectrum, if it has an odd number of rows or columns
	magI = magI(Rect(0, 0, magI.cols & -2, magI.rows & -2));

	// rearrange the quadrants of Fourier image  so that the origin is at the image center
	int cx = magI.cols / 2;
	int cy = magI.rows / 2;

	Mat q0(magI, Rect(0, 0, cx, cy));   // Top-Left - Create a ROI per quadrant
	Mat q1(magI, Rect(cx, 0, cx, cy));  // Top-Right
	Mat q2(magI, Rect(0, cy, cx, cy));  // Bottom-Left
	Mat q3(magI, Rect(cx, cy, cx, cy)); // Bottom-Right

	Mat tmp;                           // swap quadrants (Top-Left with Bottom-Right)
	q0.copyTo(tmp);
	q3.copyTo(q0);
	tmp.copyTo(q3);

	q1.copyTo(tmp);                    // swap quadrant (Top-Right with Bottom-Left)
	q2.copyTo(q1);
	tmp.copyTo(q2);

	normalize(magI, magI, 0, 1, CV_MINMAX); // Transform the matrix with float values into a
	// viewable image form (float between values 0 and 1).

	imshow(winname, magI);
}