#pragma once
#include <opencv2/opencv.hpp>
class BackgroundRemover
{
public:
	BackgroundRemover(void);
	~BackgroundRemover(void);
	void UpdateBackground(cv::Mat frame){

	}
private:
	cv::Mat frameSum;
	int frameCount;
};

