#ifndef TRIANGLETREE_H
#define TRIANGLETREE_H

#include "triangle.h"
#include <string>

class Triangle;
class Collider;

class TriangleBox {
public:
	Vector3 minPos, maxPos;
	
	TriangleBox();
	~TriangleBox() {}

	void Update(Triangle* tri);
	bool Cantain(Vector3 O);
	double CalnArea();
	double Collide(Vector3 ray_O, Vector3 ray_V);
};

class TriangleNode {
public:
	Triangle** tris;
	int size, plane;
	double split;
	TriangleBox box;
	TriangleNode* leftNode;
	TriangleNode* rightNode;

	TriangleNode();
	~TriangleNode();
};

class TriangleTree {
	TriangleNode* root;

	void DeleteTree(TriangleNode* node);
	void SortTriangle(Triangle** tris, int l, int r, int coord, bool minCoord);
	void DivideNode(TriangleNode* node);
	Collider TravelTree(TriangleNode* node, Vector3 ray_O, Vector3 ray_V);

public:
	TriangleTree();
	~TriangleTree();

	TriangleNode* GetRoot() { return root; }
	void BuildTree();
	Collider Collide(Vector3 ray_O, Vector3 ray_V);
};

#endif
