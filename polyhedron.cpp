#include "polyhedron.h"
#include "objReader.h"
#include <cmath>

Polyhedron::Polyhedron() : Primitive() {
	size = Vector3(1, 1, 1);
	angles = Vector3(0, 0, 0);
	tree = new TriangleTree;
}

Polyhedron::~Polyhedron() {
	delete tree;
	if (vertexN != NULL)
		delete[] vertexN;
	if (pixel != NULL)
		delete[] pixel;
}

void Polyhedron::Input(std::string var, std::stringstream& fin) {
	if (var == "mesh=") fin >> meshFile;
	if (var == "O=") O.Input(fin);
	if (var == "size=") size.Input(fin);
	if (var == "angles=") {
		angles.Input(fin);
		angles *= PI / 180;
	}
	Primitive::Input(var, fin);
}

void Polyhedron::PreTreatment() {
	ObjReader* objReader = new ObjReader();
	objReader->SetPolyhedron(this);
	objReader->ReadObj(meshFile);
	delete objReader;
}

Collider Polyhedron::Collide(Vector3 ray_O, Vector3 ray_V) {
	return tree->Collide(ray_O, ray_V);
}
