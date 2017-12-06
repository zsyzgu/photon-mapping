#ifndef PRIMITIVE_H
#define PRIMITIVE_H

#include"color.h"
#include"vector3.h"
#include"bmp.h"
#include<iostream>
#include<sstream>
#include<string>

extern const double EPS;
extern const double PI;

class Material {
public:
	Color color , absor;
	double refl , refr;
	double diff , spec;
	double rindex;
	double drefl;
	Bmp* texture;

	Material();
	~Material() {}

	void Input( std::string , std::stringstream& );
	double BRDF(Vector3 ray_R, Vector3 N, Vector3 ray_I);
};

class Primitive;

class Collider {
	Primitive* pri;

public:
	double dist;
	bool crash, front;
	Vector3 N, C, I;

	Collider() {
		pri = NULL;
		crash = false;
	}
	~Collider() {}

	Primitive* GetPrimitive() { return pri; }
	void SetPrimitive(Primitive* _pri) { pri = _pri; }
};

class Primitive {
protected:
	int sample;
	Material* material;
	Primitive* next;

public:
	Primitive();
	Primitive( const Primitive& );
	~Primitive();
	
	int GetSample() { return sample; }
	Material* GetMaterial() { return material; }
	Primitive* GetNext() { return next; }
	void SetNext( Primitive* primitive ) { next = primitive; }

	virtual void Input( std::string , std::stringstream& );
	virtual Collider Collide( Vector3 ray_O , Vector3 ray_V ) = 0;
	virtual Color GetTexture(Vector3 C) = 0;
};

class Sphere : public Primitive {
	Vector3 O , De , Dc;
	double R;

public:
	Sphere();
	~Sphere() {}

	void Input( std::string , std::stringstream& );
	Collider Collide( Vector3 ray_O , Vector3 ray_V );
	Color GetTexture(Vector3 C);
};

class Plane : public Primitive {
	Vector3 N , Dx , Dy;
	double R;

public:
	Plane() : Primitive() {}
	~Plane() {}

	void Input( std::string , std::stringstream& );
	Collider Collide( Vector3 ray_O , Vector3 ray_V );
	Color GetTexture(Vector3 C);
};

class Rectangle : public Primitive {
	Vector3 N , Dx , Dy;
	double R;

public:
	Rectangle() : Primitive() {}
	~Rectangle() {}

	void Input( std::string , std::stringstream& );
	Collider Collide( Vector3 ray_O , Vector3 ray_V );
	Color GetTexture(Vector3 C);
};

#endif
