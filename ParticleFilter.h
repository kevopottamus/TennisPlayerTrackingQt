#pragma once
#include <random>
#include <vector>
#include <opencv2/opencv.hpp>
#include "Utility.h"
class ParticleFilter
{
public:
    ParticleFilter();
    void init(const cv::Mat& image, const cv::Rect& bbox);
	// if update returns false, tracking is lost.
    bool update(const cv::Mat& image);
    void get_color_model(const cv::Mat& patch, std::vector<float>& model);
    float distance(const std::vector<float>& v1, const std::vector<float>& v2);
    float likelihood(const cv::Mat& patch);
    void transition(const cv::Mat& image); // pass the whole image
    void resample();
    void draw(cv::Mat& image);
    bool is_init(){return m_init;}
    int get_particle_count(){return static_cast<int>(m_point_vector.size());}
	float get_offset();
	cv::Point mass_center(const std::vector<cv::Point>& point_vector);
	const std::vector<cv::Point>& get_point_vector(){return m_point_vector;}
private:
    bool m_init;
    float m_std;
    int m_box_size;
	void scaleBBox(cv::Rect& bbox, float factor = 0.6);
    std::vector<cv::Point> m_point_vector;
	std::vector<cv::Point> m_last_point_vector;
    std::vector<float> m_weight_vector;
    std::vector<float> m_color_template;
    std::mt19937 m_generator;
    std::normal_distribution<float> m_normal;
	bool m_track;
};
