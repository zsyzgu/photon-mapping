#include "triangleTree.h"
#include <cstdio>
#include <fstream>
#include <cmath>
#include <thread>

TriangleBox::TriangleBox() {
	minPos = Vector3(1e9, 1e9, 1e9);
	maxPos = Vector3(-1e9, -1e9, -1e9);
}

void TriangleBox::Update(Triangle* tri) {
	for (int coord = 0; coord < 3; coord++) {
		if (tri->GetMinCoord(coord) < minPos.GetCoord(coord)) minPos.GetCoord(coord) = tri->GetMinCoord(coord);
		if (tri->GetMaxCoord(coord) > maxPos.GetCoord(coord)) maxPos.GetCoord(coord) = tri->GetMaxCoord(coord);
	}
}

bool TriangleBox::Cantain(Vector3 O) {
	for (int coord = 0; coord < 3; coord++)
		if (O.GetCoord(coord) <= minPos.GetCoord(coord) - EPS || O.GetCoord(coord) >= maxPos.GetCoord(coord) + EPS) return false;
	return true;
}

double TriangleBox::CalnArea() {
	double a = maxPos.x - minPos.x;
	double b = maxPos.y - minPos.y;
	double c = maxPos.z - minPos.z;
	return 2 * (a * b + b * c + c * a);
}

double TriangleBox::Collide(Vector3 ray_O, Vector3 ray_V) {
	double minDist = -1;
	for (int coord = 0; coord < 3; coord++) {
		double times = -1;
		if (ray_V.GetCoord(coord) >= EPS)
			times = (minPos.GetCoord(coord) - ray_O.GetCoord(coord)) / ray_V.GetCoord(coord);
		if (ray_V.GetCoord(coord) <= -EPS)
			times = (maxPos.GetCoord(coord) - ray_O.GetCoord(coord)) / ray_V.GetCoord(coord);
		if (times >= EPS) {
			Vector3 C = ray_O + ray_V * times;
			if (Cantain(C)) {
				double dist = ray_O.Distance(C);
				if (minDist <= -EPS || dist < minDist)
					minDist = dist;
			}
		}
	}
	return minDist;
}

TriangleNode::TriangleNode() {
	size = 0;
	plane = -1;
	split = 0;
	leftNode = rightNode = NULL;
}

TriangleNode::~TriangleNode() {
	for (int i = 0; i < size; i++)
		delete tris[i];
	delete tris;
	delete leftNode;
	delete rightNode;
}

TriangleTree::TriangleTree() {
	root = new TriangleNode;
}

TriangleTree::~TriangleTree() {
	DeleteTree(root);
}

void TriangleTree::DeleteTree(TriangleNode* node) {
	if (node->leftNode != NULL)
		DeleteTree(node->leftNode);
	if (node->rightNode != NULL)
		DeleteTree(node->rightNode);
	delete node;
}

void TriangleTree::SortTriangle(Triangle** tris, int l, int r, int coord, bool minCoord) {
	double (Triangle::*GetCoord)(int) = minCoord ? &Triangle::GetMinCoord : &Triangle::GetMaxCoord;
	if (l >= r) return;
	int i = l, j = r;
	Triangle* key = tris[(l + r) >> 1];
	while (i <= j) {
		while (j >= l && (key->*GetCoord)(coord) < (tris[j]->*GetCoord)(coord)) j--;
		while (i <= r && (tris[i]->*GetCoord)(coord) < (key->*GetCoord)(coord)) i++;
		if (i <= j) {
			std::swap(tris[i], tris[j]);
			i++;
			j--;
		}
	}
	SortTriangle(tris, i, r, coord, minCoord);
	SortTriangle(tris, l, j, coord, minCoord);
}

void TriangleTree::DivideNode(TriangleNode* node) {
	//iff area0 * size0 + area1 * size1 + totalArea <= totalArea * totalSize then divide
	Triangle** minNode = new Triangle*[node->size];
	Triangle** maxNode = new Triangle*[node->size];
	for (int i = 0; i < node->size; i++) {
		minNode[i] = node->tris[i];
		maxNode[i] = node->tris[i];
	}
	
	double thisCost = node->box.CalnArea() * (node->size - 1);
	double minCost = thisCost;
	int bestCoord = -1, leftSize = 0, rightSize = 0;
	double bestSplit = 0;
	for (int coord = 0; coord < 3; coord++) {
		SortTriangle(minNode, 0, node->size - 1, coord, true);
		SortTriangle(maxNode, 0, node->size - 1, coord, false);
		TriangleBox leftBox = node->box;
		TriangleBox rightBox = node->box;

		int j = 0;
		for (int i = 0; i < node->size; i++) {
			double split = minNode[i]->GetMinCoord(coord);
			leftBox.maxPos.GetCoord(coord) = split;
			rightBox.minPos.GetCoord(coord) = split;
			for ( ; j < node->size && maxNode[j]->GetMaxCoord(coord) <= split + EPS; j++);
			double cost = leftBox.CalnArea() * i + rightBox.CalnArea() * (node->size - j);
			if (cost < minCost) {
				minCost = cost;
				bestCoord = coord;
				bestSplit = split;
				leftSize = i;
				rightSize = node->size - j;
			}
		}

		j = 0;
		for (int i = 0; i < node->size; i++) {
			double split = maxNode[i]->GetMaxCoord(coord);
			leftBox.maxPos.GetCoord(coord) = split;
			rightBox.minPos.GetCoord(coord) = split;
			for ( ; j < node->size && minNode[j]->GetMinCoord(coord) <= split - EPS; j++);
			double cost = leftBox.CalnArea() * j + rightBox.CalnArea() * (node->size - i);
			if (cost < minCost) {
				minCost = cost;
				bestCoord = coord;
				bestSplit = split;
				leftSize = j;
				rightSize = node->size - i;
			}
		}
	}

	delete minNode;
	delete maxNode;

	if (bestCoord != -1) {
		leftSize = rightSize = 0;
		for (int i = 0; i < node->size; i++) {
			if (node->tris[i]->GetMinCoord(bestCoord) <= bestSplit - EPS || node->tris[i]->GetMaxCoord(bestCoord) <= bestSplit + EPS)
				leftSize++;
			if (node->tris[i]->GetMaxCoord(bestCoord) >= bestSplit + EPS || node->tris[i]->GetMinCoord(bestCoord) >= bestSplit - EPS)
				rightSize++;
		}
		TriangleBox leftBox = node->box;
		TriangleBox rightBox = node->box;
		leftBox.maxPos.GetCoord(bestCoord) = bestSplit;
		rightBox.minPos.GetCoord(bestCoord) = bestSplit;
		double cost = leftBox.CalnArea() * leftSize + rightBox.CalnArea() * rightSize;

		if (cost < thisCost) {
			node->plane = bestCoord;
			node->split = bestSplit;

			node->leftNode = new TriangleNode;
			node->leftNode->box = node->box;
			node->leftNode->box.maxPos.GetCoord(node->plane) = node->split;
			
			node->rightNode = new TriangleNode;
			node->rightNode->box = node->box;
			node->rightNode->box.minPos.GetCoord(node->plane) = node->split;
			
			node->leftNode->tris = new Triangle*[leftSize];
			node->rightNode->tris = new Triangle*[rightSize];
			int leftCnt = 0, rightCnt = 0;
			for (int i = 0; i < node->size; i++) {
				if (node->tris[i]->GetMinCoord(node->plane) <= node->split - EPS || node->tris[i]->GetMaxCoord(node->plane) <= node->split + EPS)
					node->leftNode->tris[leftCnt++] = node->tris[i];
				if (node->tris[i]->GetMaxCoord(node->plane) >= node->split + EPS || node->tris[i]->GetMinCoord(node->plane) >= node->split - EPS)
					node->rightNode->tris[rightCnt++] = node->tris[i];
			}
			node->leftNode->size = leftSize;
			node->rightNode->size = rightSize;

			DivideNode(node->leftNode);
			DivideNode(node->rightNode);
		}
	}
}

Collider TriangleTree::TravelTree(TriangleNode* node, Vector3 ray_O, Vector3 ray_V) {
	if (!node->box.Cantain(ray_O) && node->box.Collide(ray_O, ray_V) <= -EPS)
		return Collider();

	if (node->leftNode == NULL && node->rightNode == NULL) {
		Collider ret;
		for (int i = 0; i < node->size; i++) {
			Collider collider = node->tris[i]->Collide(ray_O, ray_V);
			if (collider.crash && node->box.Cantain(collider.C) && (!ret.crash || collider.dist < ret.dist))
				ret = collider;
		}
		return ret;
	}
	
	if (node->leftNode->box.Cantain(ray_O)) {
		Collider collider = TravelTree(node->leftNode, ray_O, ray_V);
		if (collider.crash) return collider;
		return TravelTree(node->rightNode, ray_O, ray_V);
	}
	if (node->rightNode->box.Cantain(ray_O)) {
		Collider collider = TravelTree(node->rightNode, ray_O, ray_V);
		if (collider.crash) return collider;
		return TravelTree(node->leftNode, ray_O, ray_V);
	}

	double leftDist = node->leftNode->box.Collide(ray_O, ray_V);
	double rightDist = node->rightNode->box.Collide(ray_O, ray_V);
	if (rightDist <= -EPS)
		return TravelTree(node->leftNode, ray_O, ray_V);
	if (leftDist <= -EPS)
		return TravelTree(node->rightNode, ray_O, ray_V);
	
	if (leftDist < rightDist) {
		Collider collider = TravelTree(node->leftNode, ray_O, ray_V);
		if (collider.crash) return collider;
		return TravelTree(node->rightNode, ray_O, ray_V);
	}
	Collider collider = TravelTree(node->rightNode, ray_O, ray_V);
	if (collider.crash) return collider;
	return TravelTree(node->leftNode, ray_O, ray_V);
}

void TriangleTree::BuildTree() {
	DivideNode(root);
}

Collider TriangleTree::Collide(Vector3 ray_O, Vector3 ray_V) {
	return TravelTree(root, ray_O, ray_V);
}
