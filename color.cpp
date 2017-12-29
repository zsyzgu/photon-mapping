#include"color.h"
#include<sstream>
#include<cmath>
#include <algorithm>

extern const float EPS;

Color operator + (const Color& A, const Color& B) {
	return Color(A.r + B.r, A.g + B.g, A.b + B.b);
}

Color operator - (const Color& A, const Color& B) {
	return Color(A.r - B.r, A.g - B.g, A.b - B.b);
}

Color operator * (const Color& A, const Color& B) {
	return Color(A.r * B.r, A.g * B.g, A.b * B.b);
}

Color operator * (const Color& A, const float& k) {
	return Color(A.r * k, A.g * k, A.b * k);
}

Color operator / (const Color& A, const float& k) {
	return Color(A.r / k, A.g / k, A.b / k);
}

Color& operator += (Color& A, const Color& B) {
	A = A + B;
	return A;
}

Color& operator -= (Color& A, const Color& B) {
	A = A - B;
	return A;
}

Color& operator *= (Color& A, const Color& B) {
	A = A * B;
	return A;
}

Color& operator *= (Color& A, const float& k) {
	A = A * k;
	return A;
}

Color& operator /= (Color& A, const float& k) {
	A = A / k;
	return A;
}

Color Color::Confine() {
	return Color(std::min(r, 1.0f), std::min(g, 1.0f), std::min(b, 1.0f));
}

Color Color::Exp() {
	return Color(exp(r), exp(g), exp(b));
}

float Color::Power() {
	return (r + g + b) / 3;
}

float Color::RGBMax() {
	if (r > g)
		return (r > b) ? r : b;
	return (g > b) ? g : b;
}

void Color::Input(std::stringstream& fin) {
	fin >> r >> g >> b;
}

bool Color::IsZeroColor() {
	return fabs(r) < 1e-6 && fabs(g) < 1e-6 && fabs(b) < 1e-6;
}
