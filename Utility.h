#pragma once
#include <vector>
class Utility
{
public:
	Utility(void);
	~Utility(void);
	static float l1norm(const std::vector<float>& v);
	static void l1normalization(std::vector<float>& v);
};

