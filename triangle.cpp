#include "triangle.h"
#include <cmath>

Triangle::Triangle() : Primitive() {
	parent = NULL;
	vertex[0] = vertex[1] = vertex[2] = 0;
	textureVertex[0] = textureVertex[1] = textureVertex[2] = 0;
	normalVectorID[0] = normalVectorID[1] = normalVectorID[2] = 0;
}

void Triangle::Input(std::string var, std::stringstream& fin) {
	if (var == "P0=") pos[0].Input(fin);
	if (var == "P1=") pos[1].Input(fin);
	if (var == "P2=") pos[2].Input(fin);
	Primitive::Input(var, fin);
}

void Triangle::PreTreatment() {
	Vector3 B = pos[2] - pos[0], C = pos[1] - pos[0];
	N = C.Cross(B);
	if (N.IsZeroVector()) {
		N = Vector3(0, 0, 1);
		return;
	}
	if (fabs(N.x) > fabs(N.y))
		mainCoord = (fabs(N.x) > fabs(N.z)) ? 0 : 2;
	else
		mainCoord = (fabs(N.y) > fabs(N.z)) ? 1 : 2;
	int u = (mainCoord + 1) % 3, v = (mainCoord + 2) % 3;
	double krec = 1.0 / N.GetCoord(mainCoord);
	nu = N.GetCoord(u) * krec;
	nv = N.GetCoord(v) * krec;
	nd = N.Dot(pos[0]) * krec;
	double reci = 1.0 / (B.GetCoord(u) * C.GetCoord(v) - B.GetCoord(v) * C.GetCoord(u));
	bnu = B.GetCoord(u) * reci;
	bnv = -B.GetCoord(v) * reci;
	cnu = C.GetCoord(v) * reci;
	cnv = -C.GetCoord(u) * reci;
	N = N.GetUnitVector();
}

Collider Triangle::Collide(Vector3 ray_O, Vector3 ray_V) {
	Collider collider;
	if ((pos[0] - pos[1]).IsZeroVector() || (pos[0] - pos[2]).IsZeroVector() || (pos[1] - pos[2]).IsZeroVector()) {
		return collider;
	}
	ray_V = ray_V.GetUnitVector();
	int u = (mainCoord + 1) % 3, v = (mainCoord + 2) % 3;
	double lnd = 1.0 / (ray_V.GetCoord(mainCoord) + nu * ray_V.GetCoord(u) + nv * ray_V.GetCoord(v));
	if (lnd > 1e9) return collider;
	double l = (nd - ray_O.GetCoord(mainCoord) - nu * ray_O.GetCoord(u) - nv * ray_O.GetCoord(v)) * lnd;
	if (l < EPS) return collider;
	double hu = ray_O.GetCoord(u) + l * ray_V.GetCoord(u) - pos[0].GetCoord(u);
	double hv = ray_O.GetCoord(v) + l * ray_V.GetCoord(v) - pos[0].GetCoord(v);
	double x = hv * bnu + hu * bnv;
	if (x < 0) return collider;
	double y = hv * cnv + hu * cnu;
	if (y < 0) return collider;
	if (x + y > 1) return collider;
	if (parent != NULL && !parent->GetVertexN(normalVectorID[0]).IsZeroVector())
		collider.N = parent->GetVertexN(normalVectorID[0]) * (1 - x - y) + parent->GetVertexN(normalVectorID[1]) * x + parent->GetVertexN(normalVectorID[2]) * y;
	else {
		collider.N = N;
	}
	
	double d = collider.N.Dot(ray_V);
	collider.crash = true;
	collider.I = ray_V;
	collider.SetPrimitive(this);
	collider.dist = l;
	collider.front = (d < 0);
	collider.C = ray_O + ray_V * collider.dist;
	
	if (material->bump != NULL)
		collider.N = CalnBump(collider.N, collider.C);
	
	if (!collider.front)
		collider.N = -collider.N;
	return collider;
}

Vector3 Triangle::CalnBump(Vector3 N, Vector3 C) {
	double totalArea = (pos[1] - pos[0]).Cross(pos[2] - pos[0]).Module();
	double area1 = (C - pos[0]).Cross(pos[2] - pos[0]).Module();
	double area2 = (C - pos[0]).Cross(pos[1] - pos[0]).Module();
	double x = 0, y = 0;
	if (totalArea != 0) {
		x = area1 / totalArea;
		y = area2 / totalArea;
	}
	std::pair<double, double> p0 = parent->GetPixel(textureVertex[0]);
	std::pair<double, double> p1 = parent->GetPixel(textureVertex[1]);
	std::pair<double, double> p2 = parent->GetPixel(textureVertex[2]);
	double u1 = p1.first - p0.first;
	double u2 = p2.first - p0.first;
	double v1 = p1.second - p0.second;
	double v2 = p2.second - p0.second;
	if (u1 * v2 - u2 * v1 == 0) return N;
	double u = p0.first + u1 * x + u2 * y;
	double v = p0.second + v1 * x + v2 * y;
	
	int H = material->bump->GetH();
	int W = material->bump->GetW();
	double deltaX = material->bump->GetSmoothColor(u - 0.5 / H, v).Power() - material->bump->GetSmoothColor(u + 0.5 / H, v).Power();
	double deltaY = material->bump->GetSmoothColor(u, v - 0.5 / W).Power() - material->bump->GetSmoothColor(u, v + 0.5 / W).Power();
	Vector3 tX = ((pos[1] - pos[0]) * v2 - (pos[2] - pos[0]) * v1) / (H * (u1 * v2 - u2 * v1));
	Vector3 tY = ((pos[1] - pos[0]) * u2 - (pos[2] - pos[0]) * u1) / (W * (v1 * u2 - v2 * u1));
	N = (N + tX.GetUnitVector() * deltaX * 10 + tY.GetUnitVector() * deltaY * 10).GetUnitVector();
	
	return N;
}

Color Triangle::GetTexture(Vector3 C) {
	double totalArea = (pos[1] - pos[0]).Cross(pos[2] - pos[0]).Module();
	double area1 = (C - pos[0]).Cross(pos[2] - pos[0]).Module();
	double area2 = (C - pos[0]).Cross(pos[1] - pos[0]).Module();
	double x = 0, y = 0;
	if (totalArea != 0) {
		x = area1 / totalArea;
		y = area2 / totalArea;
	}
	std::pair<double, double> p0 = parent->GetPixel(textureVertex[0]);
	std::pair<double, double> p1 = parent->GetPixel(textureVertex[1]);
	std::pair<double, double> p2 = parent->GetPixel(textureVertex[2]);
	p0.first = p0.first + (p1.first - p0.first) * x + (p2.first - p0.first) * y;
	p0.second = p0.second + (p1.second - p0.second) * x + (p2.second - p0.second) * y;
	return material->texture->GetSmoothColor(p0.first, p0.second);
}

double Triangle::GetMinCoord(int coord) {
	double x0 = pos[0].GetCoord(coord);
	double x1 = pos[1].GetCoord(coord);
	double x2 = pos[2].GetCoord(coord);
	if (x0 < x1)
		return (x0 < x2) ? x0 : x2;
	return (x1 < x2) ? x1 : x2;
}

double Triangle::GetMaxCoord(int coord) {
	double x0 = pos[0].GetCoord(coord);
	double x1 = pos[1].GetCoord(coord);
	double x2 = pos[2].GetCoord(coord);
	if (x0 > x1)
		return (x0 > x2) ? x0 : x2;
	return (x1 > x2) ? x1 : x2;
}
