#include"photontracer.h"
#include"scene.h"
#include<cstdlib>
#include<iostream>
#define ran() (double(rand() % RAND_MAX) / RAND_MAX)

const int MAX_PHOTONS = 10000000;
const int MAX_PHOTONTRACING_DEP = 10;
const float PERTURBATION = 0.001f;

bool Photontracer::PhotonDiffusion(Collider* collider, Photon photon, int dep, bool refracted, double* prob) {
	Primitive* pri = collider->GetPrimitive();
	Material* material = pri->GetMaterial();
	Color color = material->color;
	if (material->texture != NULL) color = color * pri->GetTexture(collider->C);
	double eta = material->diff * color.Power();
	if (eta <= ran() * (*prob)) {
		*prob -= eta;
		return false;
	}

	photon.dir = collider->N.Diffuse();
	photon.power = photon.power * color / color.Power();
	PhotonTracing(photon, dep + 1, refracted);
	return true;
}

bool Photontracer::PhotonReflection(Collider* collider, Photon photon, int dep, bool refracted, double* prob) {
	Primitive* pri = collider->GetPrimitive();
	Material* material = pri->GetMaterial();
	Color color = material->color;
	if (material->texture != NULL) color = color * pri->GetTexture(collider->C);
	double eta = material->refl * color.Power();

	if (eta <= ran() * (*prob)) {
		*prob -= material->refl;
		return false;
	}

	photon.dir = photon.dir.Reflect(collider->N);
	/*if (material->drefl > EPS) {
		Vector3 Dx = photon.dir.GetAnVerticalVector();
		Vector3 Dy = photon.dir * Dx;
		Dx = Dx.GetUnitVector() * material->drefl;
		Dy = Dy.GetUnitVector() * material->drefl;
		double x, y;
		do {
			x = ran() * 2 - 1;
			y = ran() * 2 - 1;
		} while (x * x + y * y > 1);
		x *= material->drefl;
		y *= material->drefl;
		photon.dir += Dx * x + Dy * y;
	}*/
	photon.power = photon.power * color / color.Power();
	PhotonTracing(photon, dep + 1, refracted);
	return true;
}

bool Photontracer::PhotonRefraction(Collider* collider, Photon photon, int dep, bool refracted, double* prob) {
	Primitive* pri = collider->GetPrimitive();
	Material* material = pri->GetMaterial();
	double eta = material->refr;
	if (refracted) {
		Color trans = (material->absor * -collider->dist).Exp();
		eta *= trans.Power();
		photon.power = photon.power * trans / trans.Power();
	}

	if (eta <= ran() * (*prob)) {
		*prob -= material->refr;
		return false;
	}

	double n = material->rindex;
	if (!refracted) n = 1 / n;
	bool nextRefracted = refracted;
	photon.dir = photon.dir.Refract(collider->N, n, &nextRefracted);
	PhotonTracing(photon, dep + 1, nextRefracted);
	return true;
}

void Photontracer::PhotonTracing(Photon photon, int dep, bool refracted) {
	if (dep > MAX_PHOTONTRACING_DEP) return;
	Collider* collider = scene->FindNearestCollide(photon.pos, photon.dir);

	if (collider != NULL) {
		Primitive* nearest_primitive = collider->GetPrimitive();

		photon.pos = collider->C;
		if (nearest_primitive->GetMaterial()->diff > EPS) {
			if (photonmap != NULL)
				photonmap->Store(photon);
		}

		double prob = 1;
		if (PhotonDiffusion(collider, photon, dep, refracted, &prob) == false)
			if (PhotonReflection(collider, photon, dep, refracted, &prob) == false)
				if (PhotonRefraction(collider, photon, dep, refracted, &prob) == false);
		delete collider;
	}
}

void Photontracer::Run() {
	int n = scene->GetCamera()->GetEmitPhotons();
	photonmap = new Photonmap( scene->GetCamera()->GetMaxPhotons() );
	photonmap->SetEmitPhotons( n );

	double total_power = 0;
	for ( Light* light = scene->GetLightHead() ; light != NULL ; light = light->GetNext() )
		total_power += light->GetColor().Power();
	double photon_power = total_power / n;
	
	int emited_photons = 0;
	for ( Light* light = scene->GetLightHead() ; light != NULL ; light = light->GetNext() ) {
		double light_power = light->GetColor().Power();
		while ( light_power >= photon_power ) {
			if ( ( ++emited_photons & 1048575 ) == 0 ) std::cout << "Emited photons: " << emited_photons << std::endl;
			Photon photon = light->EmitPhoton();

			/*Photon subP1 = photon;
			Photon subP2 = photon;
			Vector3 Dx = photon.dir.GetAnVerticalVector();
			Vector3 Dy = photon.dir * Dx;
			Dx = Dx.GetUnitVector() * PERTURBATION;
			Dy = Dy.GetUnitVector() * PERTURBATION;
			subP1.dir += Dx;
			subP2.dir += Dy;*/

			photon.power *= total_power;
			PhotonTracing( photon, 1, false );
			light_power -= photon_power;
		}
		std::cout << "Stored photons: " << photonmap->GetStoredPhotons() << std::endl;
	}
	

	std::cout << "Tree balancing..." << std::endl;
	photonmap->Balance();
}
