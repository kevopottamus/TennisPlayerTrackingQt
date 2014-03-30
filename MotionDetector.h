#pragma once

#include <opencv2/opencv.hpp>
using namespace cv;
class MotionDetector
{
public:
	MotionDetector(void);
	~MotionDetector(void);
	void compute(const cv::Mat& image);
	void draw_optical_flow(const cv::Mat& flow, cv::Mat& cflow_map, int step);
	cv::Mat get_flow(){return m_flow;}
	cv::Mat get_flow_magnitude(){return m_flow_magnitude;}
private:
	Mat m_prev;
	Mat m_flow;
	Mat m_flow_magnitude;
};

