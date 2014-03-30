#include "MotionDetector.h"



MotionDetector::MotionDetector(void)
{
}


MotionDetector::~MotionDetector(void)
{
}

void MotionDetector::compute(const cv::Mat& image){
	Mat gray;
	cvtColor(image, gray, CV_BGR2GRAY);
	if(m_flow_magnitude.empty()){
		m_flow_magnitude = Mat(gray.size(), CV_32FC1, Scalar(0));
	}
	if(!m_prev.empty()){
		cv::calcOpticalFlowFarneback(m_prev, gray, m_flow, 0.5, 3, 15,3, 5, 1.2, 0);
		for(int y = 0; y < m_flow.rows; y++){
			for(int x = 0; x< m_flow.cols; x++){
				const Point2f& fxy = m_flow.at<Point2f>(y, x);
				m_flow_magnitude.at<float>(y,x) = sqrtf(fxy.x * fxy.x + fxy.y * fxy.y);

			}
		}
	}
	m_prev = gray.clone();
}

void MotionDetector::draw_optical_flow(const cv::Mat& flow, cv::Mat& cflow_map, int step){
	for (int y = 0; y < flow.rows; y += step) {
		for (int x = 0; x < flow.cols; x += step) {
		const cv::Point2f& fxy = flow.at<cv::Point2f>(y, x);
		line(cflow_map, cv::Point(x, y),
		cv::Point(cvRound(x + fxy.x), cvRound(y + fxy.y)), cv::Scalar(0, 0, 255));
					cv::circle(cflow_map, cv::Point(x, y), 2, cv::Scalar(100, 220, 100), -1);
	}
}
}