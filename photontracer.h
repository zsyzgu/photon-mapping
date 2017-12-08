#ifndef PHOTONTRACER
#define PHOTONTRACER

#include"scene.h"
#include"photonmap.h"
#include <vector>

extern const int MAX_PHOTONS;
extern const int MAX_PHOTONTRACING_DEP;

class Photontracer {
	Scene* scene;
	Photonmap* photonmap;

	void PhotonTracing(Photon, Photon, Photon, int dep, bool refracted);
	bool PhotonDiffusion(Collider*, Collider*, Collider*, Photon, Photon, Photon, int dep, bool refracted, double* prob);
	bool PhotonReflection(Collider*, Collider*, Collider*, Photon, Photon, Photon, int dep, bool refracted, double* prob);
	bool PhotonRefraction(Collider*, Collider*, Collider*, Photon, Photon, Photon, int dep, bool refracted, double* prob);

public:
	void SetScene( Scene* input ) { scene = input; }
	Photonmap* GetPhotonmap() { return photonmap; }
	void Run();
};

#endif
