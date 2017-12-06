#include"primitive.h"
#include<sstream>
#include<cstdio>
#include<string>
#include<cmath>
#include<iostream>
#include<cstdlib>
#include<algorithm>

Material::Material() {
	color = absor = Color();
	refl = refr = diff = spec = 0;
	rindex = 0;
	drefl = 0;
	texture = NULL;
}

void Material::Input( std::string var , std::stringstream& fin ) {
	if ( var == "color=" ) color.Input( fin );
	if ( var == "absor=" ) absor.Input( fin );
	if ( var == "refl=" ) fin >> refl;
	if ( var == "refr=" ) fin >> refr;
	if ( var == "diff=" ) fin >> diff;
	if ( var == "spec=" ) fin >> spec;
	if ( var == "drefl=" ) fin >> drefl;
	if ( var == "rindex=" ) fin >> rindex;
	if ( var == "texture=" ) {
		std::string file; fin >> file;
		texture = new Bmp;
		texture->Input( file );
	}
}

double Material::BRDF(Vector3 ray_R, Vector3 N, Vector3 ray_I) {
	double ret = 0;
	ray_R = ray_R.GetUnitVector();
	ray_I = ray_I.GetUnitVector();

	if (diff > EPS && ray_R.Dot(N) > EPS)
		ret += diff * ray_R.Dot(N);
	if (spec > EPS && ray_R.Dot(-ray_I.Reflect(N)) > EPS)
		ret += spec * pow(ray_R.Dot(-ray_I.Reflect(N)), 50);

	return ret;
}

Primitive::Primitive() {
	sample = rand();
	material = new Material;
	next = NULL;
}

Primitive::Primitive( const Primitive& primitive ) {
	*this = primitive;
	material = new Material;
	*material = *primitive.material;
}

Primitive::~Primitive() {
	delete material;
}

void Primitive::Input( std::string var , std::stringstream& fin ) {
	material->Input( var , fin );
}

Sphere::Sphere() : Primitive() {
	De = Vector3( 0 , 0 , 1 );
	Dc = Vector3( 0 , 1 , 0 );
}

void Sphere::Input( std::string var , std::stringstream& fin ) {
	if ( var == "O=" ) O.Input( fin );
	if ( var == "R=" ) fin >> R;
	if ( var == "De=" ) De.Input( fin );
	if ( var == "Dc=" ) Dc.Input( fin );
	Primitive::Input( var , fin );
}

Collider Sphere::Collide( Vector3 ray_O , Vector3 ray_V ) {
	Collider collider;
	ray_V = ray_V.GetUnitVector();
	Vector3 P = ray_O - O;
	double b = -P.Dot(ray_V);
	double det = b * b - P.Module2() + R * R;

	if (det > EPS) {
		det = sqrt(det);
		double x1 = b - det, x2 = b + det;

		if (x2 < EPS) return collider;
		if (x1 > EPS) {
			collider.dist = x1;
			collider.front = true;
		}
		else {
			collider.dist = x2;
			collider.front = false;
		}
	}
	else
		return collider;

	collider.crash = true;
	collider.I = ray_V;
	collider.SetPrimitive(this);
	collider.C = ray_O + ray_V * collider.dist;
	collider.N = (collider.C - O).GetUnitVector();
	if (!collider.front) collider.N = -collider.N;
	return collider;
}

Color Sphere::GetTexture(Vector3 C) {
	Vector3 I = (C - O).GetUnitVector();
	double a = acos(-I.Dot(De));
	double b = acos(std::min(std::max(I.Dot(Dc) / sin(a), -1.0), 1.0));
	double u = a / PI, v = b / 2 / PI;
	if (I.Dot(Dc * De) < 0) v = 1 - v;
	return material->texture->GetSmoothColor(u, v);
}

void Plane::Input( std::string var , std::stringstream& fin ) {
	if ( var == "N=" ) N.Input( fin );
	if ( var == "R=" ) fin >> R;
	if ( var == "Dx=" ) Dx.Input( fin );
	if ( var == "Dy=" ) Dy.Input( fin );
	Primitive::Input( var , fin );
}

Collider Plane::Collide(Vector3 ray_O, Vector3 ray_V) {
	Collider collider;
	ray_V = ray_V.GetUnitVector();
	N = N.GetUnitVector();
	double d = N.Dot(ray_V);
	if (fabs(d) < EPS) return collider;
	double l = (N * R - ray_O).Dot(N) / d;
	if (l < EPS) return collider;

	collider.crash = true;
	collider.I = ray_V;
	collider.SetPrimitive(this);
	collider.dist = l;
	collider.front = (d < 0);
	collider.C = ray_O + ray_V * collider.dist;
	collider.N = (collider.front) ? N : -N;
	return collider;
}

Color Plane::GetTexture(Vector3 C) {
	double u = C.Dot(Dx) / Dx.Module2() + 0.5;
	double v = C.Dot(Dy) / Dy.Module2() + 0.5;
	return material->texture->GetSmoothColor(u, v);
}

void Rectangle::Input( std::string var , std::stringstream& fin ) {
	if ( var == "N=" ) N.Input( fin );
	if ( var == "R=" ) fin >> R;
	if ( var == "Dx=" ) Dx.Input( fin );
	if ( var == "Dy=" ) Dy.Input( fin );
	Primitive::Input( var , fin );
}

Collider Rectangle::Collide(Vector3 ray_O, Vector3 ray_V) {
	Collider collider;
	ray_V = ray_V.GetUnitVector();
	N = N.GetUnitVector();
	double d = N.Dot(ray_V);
	if (fabs(d) < EPS) return collider;
	double l = (N * R - ray_O).Dot(N) / d;
	if (l < EPS) return collider;

	collider.dist = l;
	collider.front = (d < 0);
	collider.C = ray_O + ray_V * collider.dist;
	collider.N = (collider.front) ? N : -N;
	double u = collider.C.Dot(Dx) / Dx.Module2();
	double v = collider.C.Dot(Dy) / Dy.Module2();
	collider.crash = (0 <= u && u <= 1 && 0 <= v && v <= 1);
	collider.I = ray_V;
	collider.SetPrimitive(this);
	return collider;
}

Color Rectangle::GetTexture(Vector3 C) {
	double u = C.Dot(Dx) / Dx.Module2();
	double v = C.Dot(Dy) / Dy.Module2();
	return material->texture->GetSmoothColor(u, v);
}
