#include "Utility.h"


Utility::Utility(void)
{
}


Utility::~Utility(void)
{
}

float Utility::l1norm(const std::vector<float>& v){
	float sum = 0.f;
	for(size_t i = 0; i < v.size(); ++i){
		sum += abs(v[i]);
	}
	return sum;
}

void Utility::l1normalization(std::vector<float>& v){
	float norm = l1norm(v);
	for(size_t i = 0; i < v.size(); ++i){
		v[i] /= norm;
	}
}