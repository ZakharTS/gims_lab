#include <iostream>
#include <string>
#include <opencv2/opencv.hpp> 
#include <opencv2/imgproc/imgproc.hpp>

cv::Mat filter(cv::Mat image);

cv::Mat bordering(cv::Mat image);

cv::Mat segmentation(cv::Mat image);

cv::Mat addNoise(cv::Mat image, double probability);

std::string readPath();

double readProb();


int main(int argc, char** argv) {
	cv::Mat image = cv::imread(readPath());

	if (image.empty()) return -1;

	//filter(image);

	bordering(image);

	//segmentation(image);
}

cv::Mat filter(cv::Mat image) {
	
	cv::Mat result = addNoise(image, readProb());
	cv::medianBlur(result, result, 3);
	std::cout << "Median filter done." << std::endl;
	cv::imwrite(readPath(), result);
	cv::imshow("Filtered", result);
	cv::waitKey(0);

	return result;
}

cv::Mat bordering(cv::Mat image) {
	cv::Mat result;

	cv::cvtColor(image, result, cv::COLOR_BGR2GRAY);

	cv::Mat diffX;
	cv::Sobel(result, diffX, CV_32F, 1, 0);
	cv::Mat diffY;
	cv::Sobel(result, diffY, CV_32F, 0, 1);

	cv::convertScaleAbs(diffX, diffX);
	cv::convertScaleAbs(diffY, diffY);

	cv::addWeighted(diffX, 0.5, diffY, 0.5, 0, result);

	cv::threshold(result, result, 100, 255, 3);

	std::cout << "Sobel filter done." << std::endl;
	cv::imwrite(readPath(), result);
	cv::imshow("Bordered", result);
	cv::waitKey(0);

	return result;
}

cv::Mat segmentation(cv::Mat image) {
	cv::Mat result;

	cv::pyrMeanShiftFiltering(image, result, 20, 30);

	std::cout << "Meanshift done." << std::endl;
	cv::imwrite(readPath(), result);
	cv::imshow("Segmented", result);
	cv::waitKey(0);
	return result;
}

std::string readPath() {
	std::cout << "Enter path: ";
	std::string str;
	std::cin >> str;
	return str;
}

cv::Mat addNoise(cv::Mat image, double probability) {
	int width = image.cols;
	int height = image.rows;
	int size = width * height;
	int count = (int)(size * probability) / 100;
	int x, y;
	long pos;
	cv::Mat result = image.clone();
	for (int i = 0; i < count; i++)
	{
		x = rand() % width;
		y = rand() % height;
		cv::Vec3b intens;
		intens[0] = rand() % 256;
		intens[1] = rand() % 256;
		intens[2] = rand() % 256;
		result.at<cv::Vec3b>(cv::Point(x, y)) = intens;
	}
	std::cout << "Point was added: " << count << std::endl;
	return result;
}

double readProb() {
	std::cout << "Enter probability: ";
	double value;
	std::cin >> value;
	return value;
}