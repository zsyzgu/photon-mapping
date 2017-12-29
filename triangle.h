#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "polyhedron.h"

class Polyhedron;
class Collider;

class Triangle : public Primitive {
	Polyhedron* parent;
	Vector3 N, pos[3];
	int vertex[3], textureVertex[3], normalVectorID[3];
	int mainCoord;
	double nu, nv, nd, bnu, bnv, cnu, cnv;

	Vector3 CalnBump(Vector3 N, Vector3 C);

public:
	Triangle();
	~Triangle() {}

	void Input(std::string, std::stringstream&);
	void PreTreatment();
	Collider Collide(Vector3 ray_O, Vector3 ray_V);
	Color GetTexture(Vector3 C);
	void SetParent(Polyhedron* _parent) { parent = _parent; }
	Vector3& GetPos(int i) { return pos[i]; }
	int& GetVertex(int i) { return vertex[i]; }
	int& GetTextureVertex(int i) { return textureVertex[i]; }
	int& GetNormalVectorID(int i) { return normalVectorID[i]; }
	Vector3& GetN() { return N; }
	double GetMinCoord(int coord);
	double GetMaxCoord(int coord);
};

#endif
