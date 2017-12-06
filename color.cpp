#include"color.h"
#include<sstream>
#include<cmath>
#include <algorithm>

extern const double EPS;

Color operator + (const Color& A, const Color& B) {
	return Color(A.r + B.r, A.g + B.g, A.b + B.b);
}

Color operator - (const Color& A, const Color& B) {
	return Color(A.r - B.r, A.g - B.g, A.b - B.b);
}

Color operator * (const Color& A, const Color& B) {
	return Color(A.r * B.r, A.g * B.g, A.b * B.b);
}

Color operator * (const Color& A, const double& k) {
	return Color(A.r * k, A.g * k, A.b * k);
}

Color operator / (const Color& A, const double& k) {
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

Color& operator *= (Color& A, const double& k) {
	A = A * k;
	return A;
}

Color& operator /= (Color& A, const double& k) {
	A = A / k;
	return A;
}

Color Color::Confine() {
	return Color(std::min(r, 1.0f), std::min(g, 1.0f), std::min(b, 1.0f));
}

Color Color::Exp() {
	return Color(exp(r), exp(g), exp(b));
}

double Color::Power() {
	return (r + g + b) / 3;
}

double Color::RGBMax() {
	if (r > g)
		return (r > b) ? r : b;
	return (g > b) ? g : b;
}

void Color::Input(std::stringstream& fin) {
	fin >> r >> g >> b;
}

bool Color::IsZeroColor() {
	return fabs(r) < EPS && fabs(g) < EPS && fabs(b) < EPS;
}
