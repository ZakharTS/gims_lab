#include <iostream>
#include <string>
#include <opencv2/opencv.hpp> 
#include <opencv2/imgproc/imgproc.hpp>

std::string readPath(std::string type);


int main(int argc, char** argv) {
	cv::VideoCapture inputVideo(
		readPath("Input video")
		//"D:\\9mm.mp4"
	);
	if (!inputVideo.isOpened()) {
		return -1;
	}

	cv::Size frameSize = cv::Size((int)inputViddeo.get(cv::CAP_PROP_FRAME_WIDTH), (int)inputVideo.get(cv::CAP_PROP_FRAME_HEIGHT));
	int codec = (int)inputVideo.get(cv::CAP_PROP_FOURCC);
	cv::VideoWriter outputVideo;
	outputVideo.open(
		readPath("Output video")
		//"D:\\9mm_output.mp4"
		, codec, inputVideo.get(cv::CAP_PROP_FPS), frameSize, true);
	if (!outputVideo.isOpened()) {
		return -1;
	}


	cv::Mat image = cv::imread(
		readPath("Background image")
		//"D:\\Astronaut.bmp"
	);
	if (image.empty()) {
		return -1;
	}

	resize(image, image, frameSize);
	//cv::cvtColor(image, image, cv::COLOR_RGB2HSV);
	int red = 0, green = 255, blue = 0;
	int tolerance = 50;


	std::cout << "RGB color: ";
	std::cin >> red >> green >> blue;
	std::cout << "Tolerance: ";
	std::cin >> tolerance;


	cv::Vec3b color(blue, green, red);
	cv::Scalar lower(
		std::max(0, color[0] - tolerance),
		std::max(0, color[1] - tolerance),
		std::max(0, color[2] - tolerance)
	);

	cv::Scalar upper(
		std::min(255, color[0] + tolerance),
		std::min(255, color[1] + tolerance),
		std::min(255, color[2] + tolerance)
	);


	for (;;) {
		cv::Mat frame;
		inputVideo >> frame;
		if (frame.empty()) break;
		cv::Mat mask;
		cv::inRange(frame, lower, upper, mask);
		//cv::bitwise_not(mask, mask);
		//cv::imshow("mask", mask);

		//cv::waitKey(0);

		image.copyTo(frame, mask);

		outputVideo << frame;
		//cv::imshow("video", frame);
	}
}

std::string readPath(std::string type = "default") {
	std::cout << "[" << type << "] " << "Enter path: ";
	std::string str;
	std::cin >> str;
	return str;
}