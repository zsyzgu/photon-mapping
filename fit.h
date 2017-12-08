#ifndef FIT  
#define FIT  
#include <vector>  

class Fit {
private:
	float a, b, c, r2;
public:
	bool linearFit(std::vector<float> xs, std::vector<float> ys, std::vector<float>zs);
	float getValue(float x, float y);
	float getR2();
};

#endif