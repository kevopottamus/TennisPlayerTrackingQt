#include "ParticleFilter.h"

ParticleFilter::ParticleFilter() : m_init(false), m_track(false){
}

void ParticleFilter::scaleBBox(cv::Rect& bbox, float factor){
	int centerX = bbox.x + bbox.width/2;
	int centerY = bbox.y + bbox.height/2;
	bbox.height *= factor;
	bbox.width *= factor;
	bbox.x = centerX - bbox.width/2;
	bbox.y = centerY - bbox.height/2;
}

void ParticleFilter::init(const cv::Mat& image, const cv::Rect& bbox1){
	cv::Rect bbox = bbox1;
	scaleBBox(bbox,0.5);
    int particle_count = 20;
    float weight = 1.f / static_cast<float>(particle_count);
    m_point_vector.clear();
    m_point_vector.reserve(particle_count);
    m_weight_vector.clear();
    m_weight_vector.reserve(particle_count);
    get_color_model(image(bbox), m_color_template);
    m_box_size = bbox.width;
    m_std = static_cast<float>(bbox.width) * 0.6f;
    // initialize particles
    int center_x = bbox.x + bbox.width / 2;
    int center_y = bbox.y + bbox.height / 2;
    for(int i = 0; i < particle_count; ++i){
        m_point_vector.push_back(cv::Point(center_x + m_normal(m_generator) * m_std, center_y + m_normal(m_generator) * m_std));
        m_weight_vector.push_back(weight);
    }
    m_init = true;
}

bool ParticleFilter::update(const cv::Mat& image){
	std::cout << "Count of particle: " << m_point_vector.size() << std::endl;
	m_last_point_vector.assign(m_point_vector.begin(), m_point_vector.end());
    transition(image);
    resample();
	return m_track;
}

void ParticleFilter::get_color_model(const cv::Mat& patch, std::vector<float>& model){
    if(!model.empty())
        model.clear();
    cv::Mat hsv;
    cv::cvtColor(patch, hsv, CV_BGR2HSV);
    std::vector<cv::Mat> hsv_vector;
    cv::split(hsv, hsv_vector);
    cv::Mat s_mask = hsv_vector[1] > 25;
    cv::Mat v_mask = hsv_vector[2] > 50;
    cv::Mat mask = s_mask & v_mask;
    int hbins = 10;
    int sbins = 10;
    int vbins = 10;
    int hs_size[] = {hbins, sbins};
    float hranges[] = {0.f, 180.f};
    float sranges[] = {25.f, 255.f};
    const float* hs_ranges[] = {hranges, sranges};
    int hs_channels[] = {0, 1};
    cv::MatND hs_hist;
    cv::calcHist(&patch, 1, hs_channels, mask, hs_hist, 2, hs_size, hs_ranges);
    int v_size[] = {vbins};
    float vranges[] = {0.f, 255.f};
    const float* v_ranges[] = {vranges};
    int v_channels[] = {2};
    cv::MatND v_hist;
    cv::calcHist(&patch, 1, v_channels, cv::Mat(), v_hist, 1, v_size, v_ranges);
    model.reserve(hbins * sbins + vbins);
    for(int i = 0; i < hbins; ++i){
        for(int j = 0; j < sbins; ++j){
            model.push_back(hs_hist.at<float>(i, j));
        }
    }
    for(int i = 0; i < vbins; ++i){
        model.push_back(v_hist.at<float>(i));
    }
    Utility::l1normalization(model);
}

float ParticleFilter::distance(const std::vector<float>& v1, const std::vector<float>& v2){
    if(v1.empty() || v2.empty() || v1.size() != v2.size()){
        std::cerr << "similarity error: wrong size" << std::endl;
        return 1e6f;
    }
    float sum = 0.f;
    for(size_t i = 0; i < v1.size(); ++i){
        sum += sqrtf(v1[i] * v2[i]);
    }
    return sqrtf(1 - sum);
}

float ParticleFilter::likelihood(const cv::Mat& patch){
    if(patch.empty())
        return 0.f;
    std::vector<float> current_model;
    get_color_model(patch, current_model);
    float dist = distance(current_model, m_color_template);
    return expf(-20.f * dist * dist);
}

void ParticleFilter::transition(const cv::Mat& image){
    if(image.empty())
        return;
    std::vector<cv::Point> point_vector;
    point_vector.reserve(m_point_vector.size());
    m_weight_vector.clear();
    m_weight_vector.reserve(m_point_vector.size());
    for(size_t i = 0; i < m_point_vector.size(); ++i){
        cv::Point point = m_point_vector[i];
        point.x += m_normal(m_generator) * m_std;
        point.y += m_normal(m_generator) * m_std;
        if(point.x >= 0 && point.x < image.cols && point.y >= 0 && point.y < image.rows){
            point_vector.push_back(point);
            cv::Rect rect(point.x - m_box_size / 2, point.y - m_box_size / 2, m_box_size, m_box_size);
            float l = likelihood(image(rect & cv::Rect(0, 0, image.cols, image.rows)));
			m_weight_vector.push_back(l);
        }
    }
	/// m_weight_vector is required to be non-empty
	if(m_weight_vector.empty()){
		m_track = false;
	}else{
	float max = *std::max_element(m_weight_vector.begin(), m_weight_vector.end());
	if(max < 0.01f){
		m_track = false;
	}else{
		m_track = true;
	}
	}
    Utility::l1normalization(m_weight_vector);
    m_point_vector.assign(point_vector.begin(), point_vector.end());
}

void ParticleFilter::resample(){
    int particle_count = static_cast<int>(m_weight_vector.size());
    std::vector<cv::Point> point_vector;
    point_vector.reserve(particle_count);
    float step = 1.f / static_cast<float>(particle_count);
    std::vector<float> cdf(particle_count, 0.f);
    for(int i = 1; i < particle_count; ++i){
        cdf[i] = cdf[i - 1] + m_weight_vector[i - 1];
    }
    std::uniform_real_distribution<float> uniform_dist(1e-6, step);
    float base = uniform_dist(m_generator);
    for(int i = 0; i < particle_count; ++i){
        float u = base + step * i;
        auto low = std::lower_bound(cdf.begin(), cdf.end(), u);
        int pos = --low - cdf.begin();
        point_vector.push_back(m_point_vector[pos]);
    }
    m_point_vector.assign(point_vector.begin(), point_vector.end());
    m_weight_vector.assign(m_point_vector.size(), step);
}

void ParticleFilter::draw(cv::Mat& image){
    for(size_t i = 0; i < m_point_vector.size(); ++i){
        cv::circle(image, m_point_vector[i], 2, cv::Scalar(220, 50, 40), 1, CV_AA);
    }
}

float ParticleFilter::get_offset(){
	cv::Point center = mass_center(m_point_vector);
	cv::Point last_center = mass_center(m_last_point_vector);
	return abs(center.x - last_center.x) + abs(center.y -last_center.y);
}

cv::Point ParticleFilter::mass_center(const std::vector<cv::Point>& point_vector){
	int x = 0;
	int y = 0;
	for(size_t i = 0; i < point_vector.size(); ++i){
		x += point_vector[i].x;
		y += point_vector[i].y;
	}
	return cv::Point(x/point_vector.size(), y/point_vector.size());
}