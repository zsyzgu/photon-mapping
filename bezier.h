#ifndef BEZIER_H
#define BEZIER_H

#include "primitive.h"

class Bezier : public Primitive {
	int n, m;
	int cnt;
	Vector3** points;

	float** calnB(int n, float t);
	void freeB(int n, float** B);
	Vector3 calnP(float u, float v);
	Vector3 calnPu(float u, float v);
	Vector3 calnPv(float u, float v);
	Collider accurateCollide(Vector3 ray_O, Vector3 ray_V, float u, float v, float t);

public:
	Bezier();
	~Bezier();

	void PreTreatment();
	void Input(std::string, std::stringstream&);
	Collider Collide(Vector3 ray_O, Vector3 ray_V);
};

#endif
