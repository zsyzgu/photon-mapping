#ifndef COLOR_H
#define COLOR_H

#include<sstream>

class Color {
public:
	float r, g, b;

	Color(float R = 0, float G = 0, float B = 0) : r(R), g(G), b(B) {}
	~Color() {}

	friend Color operator + (const Color&, const Color&);
	friend Color operator - (const Color&, const Color&);
	friend Color operator * (const Color&, const Color&);
	friend Color operator * (const Color&, const float&);
	friend Color operator / (const Color&, const float&);
	friend Color& operator += (Color&, const Color&);
	friend Color& operator -= (Color&, const Color&);
	friend Color& operator *= (Color&, const Color&);
	friend Color& operator *= (Color&, const float&);
	friend Color& operator /= (Color&, const float&);
	Color Confine(); //luminance must be less than or equal to 1
	Color Exp();
	float Power();
	float RGBMax();
	void Input(std::stringstream&);
	bool IsZeroColor();
};

#endif
