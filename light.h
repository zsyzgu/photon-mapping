#ifndef LIGHT_H
#define LIGHT_H

#include"vector3.h"
#include"color.h"
#include"primitive.h"
#include"photonmap.h"
#include<sstream>
#include<string>
#include<cmath>

extern const double EPS;
extern const int HASH_MOD;
extern const double SPEC_POWER;

class Light;

class LightCollider {
	Light* light;

public:
	bool crash;
	double dist;

	LightCollider() {
		light = NULL;
		crash = false;
	}
	~LightCollider() {}

	Light* GetLight() { return light; }
	void SetLight(Light* _light) { light = _light; }
};

class Light {
protected:
	int sample;
	Color color;
	Light* next;

	Color CalnIrradiance(Collider* collider, Vector3 V, int* hash);

public:
	double crash_dist;

	Light();
	~Light() {}
	
	int GetSample() { return sample; }
	Color GetColor() { return color; }
	Light* GetNext() { return next; }
	void SetNext( Light* light ) { next = light; }

	virtual void Input( std::string , std::stringstream& );
	virtual Vector3 GetO() = 0;
	virtual LightCollider Collide(Vector3 ray_O, Vector3 ray_V) = 0;
	virtual Color GetIrradiance(Collider* collider, Primitive* primitive_head, int shade_quality, int* hash) = 0;
	virtual Photon EmitPhoton() = 0;
};

class PointLight : public Light {
	Vector3 O;
public:
	PointLight() : Light() {}
	~PointLight() {}
	
	Vector3 GetO() { return O; }
	void Input( std::string , std::stringstream& );
	LightCollider Collide(Vector3 ray_O, Vector3 ray_V);
	Color GetIrradiance(Collider* collider, Primitive* primitive_head, int shade_quality, int* hash);
	Photon EmitPhoton();
};

class AreaLight : public Light {
	Vector3 O , Dx , Dy;
public:
	AreaLight() : Light() {}
	~AreaLight() {}
	
	Vector3 GetO() { return O; }
	void Input( std::string , std::stringstream& );
	LightCollider Collide(Vector3 ray_O, Vector3 ray_V);
	Color GetIrradiance(Collider* collider, Primitive* primitive_head, int shade_quality, int* hash);
	Photon EmitPhoton();
};

#endif
