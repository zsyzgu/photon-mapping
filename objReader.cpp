#include "objReader.h"
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <algorithm>

ObjReader::ObjReader() {
	polyhedron = NULL;
	vSize = 0;
	vtSize = 0;
	vnSize = 0;
	fSize = 0;
	matSize = 0;
}

void ObjReader::ReadMtlSize(std::string file) {
	std::ifstream fin(file.c_str());
	std::string order;

	while (getline(fin, order, '\n')) {
		std::stringstream fin2(order);
		std::string var;
		if (!(fin2 >> var)) continue;
		if (var == "newmtl")
			matSize++;
	}
	fin.close();

	mat = new Material*[matSize + 1];
}

void ObjReader::ReadObjSize(std::string file) {
	std::ifstream fin(file.c_str());
	std::string order;
	
	while (getline(fin, order, '\n')) {
		std::stringstream fin2(order);
		std::string var;
		if (!(fin2 >> var)) continue;
		if (var == "mtllib") {
			std::string mtlFile;
			fin2 >> mtlFile;
			ReadMtlSize(mtlFile);
		}
		if (var == "v")
			vSize++;
		if (var == "vt")
			vtSize++;
		if (var == "vn")
			vnSize++;
		if (var == "f") {
			int vertexCnt = 0;
			std::string var;
			while (fin2 >> var)
				vertexCnt++;
			fSize += std::max(0, vertexCnt - 2);
		}
	}
	fin.close();

	v = new Vector3[vSize + 1];
	vt = new std::pair<double, double>[vtSize + 1];
	if (vnSize == 0)
		vn = new Vector3[vSize + 1];
	else
		vn = new Vector3[vnSize + 1];
	tris = new Triangle*[fSize];
}

void ObjReader::ReadMtl(std::string file) {
	std::ifstream fin(file.c_str());
	std::string order;
	
	int matCnt = 0;
	while (getline(fin, order, '\n')) {
		std::stringstream fin2(order);
		std::string var;
		if (!(fin2 >> var)) continue;

		if (var == "newmtl") {
			std::string matName;
			fin2 >> matName;
			matMap[matName] = ++matCnt;
			mat[matCnt] = new Material;
		}
		if (var == "Ka") {

		}
		if (var == "Kd") {
			mat[matCnt]->color.Input(fin2);
			mat[matCnt]->diff = mat[matCnt]->color.RGBMax();
			mat[matCnt]->color /= mat[matCnt]->diff;
		}
		if (var == "Ks") {
			fin2 >> mat[matCnt]->refl;
		}
		if (var == "Tf") {
			mat[matCnt]->absor.Input(fin2);
			if (mat[matCnt]->absor.Power() < 1 - EPS) {
				mat[matCnt]->refr = 1;
				//mat[matCnt]->diff = mat[matCnt]->spec = mat[matCnt]->refl = 0;
			}
		}
		if (var == "Ni") {
			fin2 >> mat[matCnt]->rindex;
		}
		if (var == "map_Kd") {
			std::string bmpFile;
			fin2 >> bmpFile;
			mat[matCnt]->texture = new Bmp;
			mat[matCnt]->texture->Input(bmpFile);
		}
		if (var == "map_bump") {
			std::string bmpFile;
			fin2 >> bmpFile;
			mat[matCnt]->bump = new Bmp;
			mat[matCnt]->bump->Input(bmpFile);
		}
	}
	fin.close();
}

void ObjReader::CalnVn() {
	if (vnSize > 0) {
		for (int i = 1; i <= vnSize; i++) {
			vn[i] = vn[i].Rotate(Vector3(1, 0, 0), polyhedron->GetAngles().GetCoord(0));
			vn[i] = vn[i].Rotate(Vector3(0, 1, 0), polyhedron->GetAngles().GetCoord(1));
			vn[i] = vn[i].Rotate(Vector3(0, 0, 1), polyhedron->GetAngles().GetCoord(2));
		}
	} else {
		//Assess vn
		/*vnSize = vSize;
		for (int i = 0; i < fSize; i++)
			for (int j = 0; j < 3; j++) {
				int id = tris[i]->GetNormalVectorID(j) = tris[i]->GetVertex(j);
				if (vn[id].Dot(tris[i]->GetN()) < -EPS)
					vn[id] -= tris[i]->GetN();
				else
					vn[id] += tris[i]->GetN();
			}
		for (int i = 1; i <= vnSize; i++)
			vn[i] = vn[i].GetUnitVector();*/
	}
}

void ObjReader::ReadObj(std::string file) {
	ReadObjSize(file);
	std::ifstream fin(file.c_str());
	std::string order;

	int matID = -1;
	int vCnt = 0, vtCnt = 0, vnCnt = 0, fCnt = 0;
	while (getline(fin, order, '\n')) {
		std::stringstream fin2(order);
		std::string var;
		if (!(fin2 >> var)) continue;

		if (var == "mtllib") {
			std::string mtlFile;
			fin2 >> mtlFile;
			ReadMtl(mtlFile);
		}
		if (var == "usemtl") {
			std::string matName;
			fin2 >> matName;
			matID = matMap[matName];
		}
		if (var == "v") {
			vCnt++;
			v[vCnt].Input(fin2);
		}
		if (var == "vt") {
			vtCnt++;
			fin2 >> vt[vtCnt].second >> vt[vtCnt].first;
		}
		if (var == "vn") {
			vnCnt++;
			vn[vnCnt].Input(fin2);
		}
		if (var == "f") {
			Triangle* tri = tris[fCnt] = new Triangle;
			tri->SetParent(polyhedron);
			//
			tri->SetSample(polyhedron->GetSample() + matID);
			if (matID != -1)
				tri->SetMaterial(mat[matID]);
			else
				tri->SetMaterial(polyhedron->GetMaterial());
			std::string str;
			for (int i = 0; fin2 >> str; i++) {
				int bufferLen = 0, buffer[3];
				buffer[0] = buffer[1] = buffer[2] = -1;
				for (int s = 0, t = 0; t < (int)str.length(); t++)
					if (t + 1 >= (int)str.length() || str[t + 1] == '/') {
						buffer[bufferLen++] = atoi(str.substr(s, t - s + 1).c_str());
						s = t + 2;
					}
				int vertexID = i;
				if (i >= 3) {
					vertexID = 2;
					tri = tris[fCnt] = new Triangle;
					*tri = *tris[fCnt - 1];
					tri->GetVertex(1) = tri->GetVertex(2);
					tri->GetPos(1) = tri->GetPos(2);
					tri->GetTextureVertex(1) = tri->GetTextureVertex(2);
					tri->GetNormalVectorID(1) = tri->GetNormalVectorID(2);
				}
				if (buffer[0] > 0) {
					tri->GetVertex(vertexID) = buffer[0];
					Vector3 vertexPos = v[buffer[0]];
					vertexPos = vertexPos.Rotate(Vector3(1, 0, 0), polyhedron->GetAngles().GetCoord(0));
					vertexPos = vertexPos.Rotate(Vector3(0, 1, 0), polyhedron->GetAngles().GetCoord(1));
					vertexPos = vertexPos.Rotate(Vector3(0, 0, 1), polyhedron->GetAngles().GetCoord(2));
					vertexPos = polyhedron->GetO() + vertexPos * polyhedron->GetSize();
					tri->GetPos(vertexID) = vertexPos;
				}
				if (buffer[1] > 0) {
					tri->GetTextureVertex(vertexID) = buffer[1];
				}
				if (buffer[2] > 0) {
					tri->GetNormalVectorID(vertexID) = buffer[2];
				}
				if (i >= 2) {
					tri->PreTreatment();
					fCnt++;
				}
			}
		}
	}
	fin.close();
	
	CalnVn();

	TriangleNode* root = polyhedron->GetTree()->GetRoot();
	root->size = fCnt;
	root->tris = new Triangle*[root->size];
	for (int i = 0; i < root->size; i++) {
		root->tris[i] = tris[i];
		root->box.Update(tris[i]);
	}
	polyhedron->GetTree()->BuildTree();
	
	polyhedron->SetVertexN(vn);
	polyhedron->SetPixel(vt);
	delete[] v;
	delete[] tris;
	delete[] mat;
}
