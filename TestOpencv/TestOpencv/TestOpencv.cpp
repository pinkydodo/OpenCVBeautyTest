// TestOpencv.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include <opencv2/opencv.hpp>
using namespace cv;
#include "Whiten.h"
#include <opencv2/core/ocl.hpp>
using namespace std;

void showImg(std::string file)
{
	Mat img = imread(file);

	namedWindow("picture", 1);

	imshow("picture", img);
	waitKey(0);
}

Mat beautify(Mat img, float beautyLevel )
{
	if (beautyLevel < 0)
	{
		beautyLevel = 0;
	}
	else if (beautyLevel > 1)
	{
		beautyLevel = 1;
	}

	int value1 = 3;
	int value2 = 1;

	int dx = value1 * 5;
	double fc = value1 * 12.5;

	double p = 0.1;

	//获取完全美颜的图片
	cv::Mat dst;
	cv::Mat temp1, temp2, temp3, temp4;
	cv::bilateralFilter(img, temp1, dx, fc, fc);
	cv::subtract(temp1, img, temp2);
	cv::add(temp2, cv::Scalar(10, 10, 10, 128), temp2);
	cv::GaussianBlur(temp2, temp3, cv::Size(2 * value2 - 1, 2 * value2 - 1), 0);
	cv::add(img, temp3, temp4);

	cv::addWeighted(img, p, temp4, 1 - p, 0, dst);
	cv::add(dst, cv::Scalar(10, 10, 10, 255), dst);

	//通过融合原始图像和美颜图像，来处理beautyLevel
	cv::addWeighted(img, (1 - beautyLevel), dst, beautyLevel, 0, dst);

	return dst;
}

void testCV(std::string path)
{
	Mat img = imread(path);

	if (img.empty())
	{
		return;
	}

	Mat gray, bilater, canny;

	cvtColor(img, gray, COLOR_BGR2GRAY);
	//GaussianBlur( gray ,  gray ,  Size(5,5 ), 3, 3 );
	bilateralFilter(gray, bilater, 15, 36, 36);
	Canny(bilater, canny, 10, 100, 3, true);

}

void testCL(std::string path)
{
	UMat img;
	imread(path).copyTo(img);

	if (img.empty())
	{
		return;
	}

	UMat gray, bilater, canny;

	cvtColor(img, gray, COLOR_BGR2GRAY);
	//    GaussianBlur( gray ,  gray ,  Size(5,5 ), 3, 3 );
	bilateralFilter(gray, bilater, 15, 36, 36);
	Canny(bilater, canny, 10, 100, 3, true);
}

Mat testCVBeautify(Mat img)
{
	int value1 = 3;
	int value2 = 1;

	int dx = 15;
	double fc = value1 * 12.5;

	static Mat dst;
	static Mat temp1, temp2;
	bilateralFilter(img, temp1, dx, fc, fc);

	subtract(temp1, img, temp2);
	cv::add(temp2, Scalar(10, 10, 10, 128), temp2);
	GaussianBlur(temp2, temp1, Size(2 * value2 - 1, 2 * value2 - 1), 0);
	cv::add(img, temp1, dst);

	return dst;
}


Mat testCLBeautify(Mat img)
{
	static UMat src;
	img.copyTo(src);
	int value1 = 3;
	int value2 = 1;

	printf("after copy\n");
	int dx = 15;
	double fc = value1 * 12.5;

	static Mat dst;
	static UMat temp1, temp2;
	static Mat temp3, temp4 ;
	bilateralFilter(src, temp1, dx, fc, fc);

	printf("after bilateralFilter\n");

	subtract(temp1, src, temp2);
	cv::add(temp2, Scalar(10, 10, 10, 128), temp2);

	printf("after add\n");
	temp2.copyTo(temp3);
	printf("after copyTo back\n");
	//GaussianBlur(temp3, temp4, Size(2 * value2 - 1, 2 * value2 - 1), 0);

	printf("after GaussianBlur\n");
	cv::add(img, temp4, dst);

	//temp2.copyTo(dst);

	printf("after copy back\n");

	return dst;
}
void checkOpt()
{
	//    bool has = cv::ocl::haveOpenCL();
	//    bool use = cv::ocl::useOpenCL();

	//cv::setUseOptimized(true);
	bool opt_status = cv::useOptimized();
	cout << "当前的指令集优化状态：" << opt_status << endl;

	bool simd = checkHardwareSupport(CV_CPU_SSE);
	cout << "当前的指令集优化状态：" << simd << endl;
	bool simd1 = checkHardwareSupport(CV_CPU_SSE2);
	cout << "当前的指令集优化状态：" << simd1 << endl;
	bool simd2 = checkHardwareSupport(CV_CPU_SSE3);
	cout << "当前的指令集优化状态：" << simd2 << endl;
	bool simd3 = checkHardwareSupport(CV_CPU_SSSE3);
	cout << "当前的指令集优化状态：" << simd3 << endl;
	bool simd4 = checkHardwareSupport(CV_CPU_MMX);
	cout << "当前的指令集优化状态：" << simd4 << endl;
	bool simd5 = checkHardwareSupport(CV_CPU_SSE4_1);
	cout << "当前的指令集优化状态：" << simd5 << endl;
	bool simd6 = checkHardwareSupport(CV_CPU_SSE4_2);
	cout << "当前的指令集优化状态：" << simd6 << endl;
	bool simd7 = checkHardwareSupport(CV_CPU_AVX);
	cout << "当前的指令集优化状态：" << simd7 << endl;
	bool simd8 = checkHardwareSupport(CV_CPU_POPCNT);
	cout << "当前的指令集优化状态：" << simd8 << endl;

	//    int num_devices = cv::cuda::getCudaEnabledDeviceCount();
	auto device = cv::ocl::Device::getDefault();
	bool isAmd = device.isAMD();
	bool isIntel = device.isIntel();
	bool isNavid = device.isNVidia();
	cout << "device:" << endl;
	cout << " isAmd:" << isAmd << endl;
	cout << " isIntel:" << isIntel << endl;
	cout << " isNavide:" << isNavid << endl;
	cout << "	available:" << device.available() << endl;

}

void testGPU()
{
	std::string imgPath = "E:\\Project\\TestOpencv\\res\\tu.jpg";

	double tick = getTickCount();
	Mat img = imread(imgPath);
	for (int i = 0; i < 500; i++)
	{
		testCV(imgPath);
	}


	double tick2 = getTickCount();

	printf("begin cl test ");

	for (int i = 0; i < 100; i++)
	{
		//printf("cl test:%d ", i );
		//testCL(imgPath);
	}

	double tick3 = getTickCount();

	float gap1 = (tick2 - tick) / getTickFrequency();

	float gap2 = (tick3 - tick2) / getTickFrequency();

	printf("time1:%f  vs time2 :%f\n", gap1, gap2);
}

int _tmain(int argc, _TCHAR* argv[])
{
	checkOpt();
	//cv::setUseOptimized(false);
	//testGPU();
	//cv::setUseOptimized(true);
	//testGPU();

	//cv::ocl::setUseOpenCL(true);
	//std::cout << cv::ocl::haveOpenCL() << std::endl;
	//if (!cv::ocl::haveOpenCL())
	//{
	//	std::cout << "OpenCL IS not avaiable ..." << std::endl;
	//	return 0;
	//}
	//else {
	//	std::cout << "OpenCL IS valid ..." << std::endl;
	//}

	//std::cout << "Hello, World!\n";
	std::string imgPath = "E:\\Project\\TestOpencv\\res\\tu.jpg";
	//showImg( imgPath );
	
	Mat img = imread(imgPath);
	imshow("ori", img);
	Mat imgOut = beautify(img, 1.0);
	imwrite("E:\\Project\\TestOpencv\\res\\whiten.jpg", imgOut);

	imshow("test", imgOut);

	waitKey(0);
	//std::cout << "Hello, World!\n";
	return 0;
}

