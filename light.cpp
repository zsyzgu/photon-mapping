#include"light.h"
#include<sstream>
#include<string>
#include<cmath>
#include<cstdlib>
#include<ctime>
#include<iostream>
#define ran() ( double( rand() % RAND_MAX ) / RAND_MAX )

Light::Light() {
	sample = rand();
	next = NULL;
}

void Light::Input( std::string var , std::stringstream& fin ) {
	if ( var == "color=" ) color.Input( fin );
}

Color Light::CalnIrradiance(Collider* collider, Vector3 V, int* hash) {
	Primitive* pri = collider->GetPrimitive();
	Color ret = color * pri->GetMaterial()->BRDF(V, collider->N, -collider->I);
	ret /= V.Module2(); //rt adapts to ppm

	if (!ret.IsZeroColor() && hash != NULL)
		*hash = (*hash + GetSample()) % HASH_MOD;

	return ret;
}

void PointLight::Input( std::string var , std::stringstream& fin ) {
	if ( var == "O=" ) O.Input( fin );
	Light::Input( var , fin );
}

LightCollider PointLight::Collide(Vector3 ray_O, Vector3 ray_V) {
	LightCollider ret;
	ret.SetLight(this);
	return ret;
}

Photon PointLight::EmitPhoton() {
	Photon ret;
	ret.power = color / color.Power();
	ret.pos = O;
	ret.dir.AssRandomVector();
	return ret;
}

Color PointLight::GetIrradiance(Collider* collider, Primitive* primitive_head, int shade_quality, int* hash) {
	Primitive* pri = collider->GetPrimitive();
	Vector3 V = O - collider->C;
	double dist = V.Module();

	for (Primitive* now = primitive_head; now != NULL; now = now->GetNext()) {
		Collider thisCollider = now->Collide(collider->C, V);
		if (thisCollider.crash && thisCollider.dist < dist) return Color();
	}

	return CalnIrradiance(collider, V, hash);
}

void AreaLight::Input( std::string var , std::stringstream& fin ) {
	if ( var == "O=" ) O.Input( fin );
	if ( var == "Dx=" ) Dx.Input( fin );
	if ( var == "Dy=" ) Dy.Input( fin );
	Light::Input( var , fin );
}

LightCollider AreaLight::Collide(Vector3 ray_O, Vector3 ray_V) {
	LightCollider ret;
	ret.SetLight(this);

	ray_V = ray_V.GetUnitVector();
	Vector3 N = (Dx.Cross(Dy)).GetUnitVector();
	double d = N.Dot(ray_V);
	if (fabs(d) < EPS) return ret;
	double l = (N * O.Dot(N) - ray_O).Dot(N) / d;
	if (l < EPS) return ret;

	Vector3 C = (ray_O + ray_V * l) - O;
	if (fabs(Dx.Dot(C)) > Dx.Dot(Dx)) return ret;
	if (fabs(Dy.Dot(C)) > Dy.Dot(Dy)) return ret;

	ret.crash = true;
	ret.dist = l;
	return ret;
}

Photon AreaLight::EmitPhoton() {
	Photon ret;
	ret.power = color / color.Power();
	ret.pos = O + Dx * ( ran() * 2 - 1 ) + Dy * ( ran() * 2 - 1 );
	ret.dir.AssRandomVector();
	return ret;
}

Color AreaLight::GetIrradiance(Collider* collider, Primitive* primitive_head, int shade_quality, int* hash) {
	Primitive* pri = collider->GetPrimitive();
	Color ret;

	for (int i = -2; i < 2; i++)
		for (int j = -2; j < 2; j++)
			for (int k = 0; k < shade_quality; k++) {
				Vector3 V = (O + Dx * ((ran() + i) / 2) + Dy * ((ran() + j) / 2)) - collider->C;
				double dist = V.Module();

				bool shade = false;
				for (Primitive* now = primitive_head; now != NULL; now = now->GetNext()) {
					Collider thisCollider = now->Collide(collider->C, V);
					if (thisCollider.crash && thisCollider.dist < dist) {
						shade = true;
						break;
					}
				}

				if (shade == false) ret += CalnIrradiance(collider, V, NULL);
			}

	ret /= 16.0 * shade_quality;
	return ret;
}
