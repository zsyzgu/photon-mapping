#ifndef POLYHEDRON_H
#define POLYHEDRON_H

#include "primitive.h"
#include "triangleTree.h"

class TriangleTree;

class Polyhedron : public Primitive {
	Vector3 O, size, angles;
	Vector3* vertexN;
	std::pair<double, double>* pixel;
	std::string meshFile;
	TriangleTree* tree;

public:
	Polyhedron();
	~Polyhedron();
	
	Vector3 GetO() { return O; }
	Vector3 GetSize() { return size; }
	Vector3 GetAngles() { return angles; }
	void SetVertexN(Vector3* _vertexN) { vertexN = _vertexN; }
	Vector3& GetVertexN(int i) { return vertexN[i]; }
	TriangleTree* GetTree() { return tree; }
	void SetPixel(std::pair<double, double>* _pixel) { pixel = _pixel; }
	std::pair<double, double>& GetPixel(int i) { return pixel[i]; }
	void Input(std::string, std::stringstream&);
	void PreTreatment();
	Collider Collide(Vector3 ray_O, Vector3 ray_V);
};

#endif