#include"photontracer.h"
#include"scene.h"
#include<cstdlib>
#include<iostream>
#define ran() (double(rand() % RAND_MAX) / RAND_MAX)

const int MAX_PHOTONS = 10000000;
const int MAX_PHOTONTRACING_DEP = 10;
const float PERTURBATION = 1e-5;

bool Photontracer::PhotonDiffusion(Collider* collider, Collider* collider1, Collider* collider2, Photon photon, Photon subPhoton1, Photon subPhoton2, int dep, bool refracted, double* prob) {
	Primitive* pri = collider->GetPrimitive();
	Material* material = pri->GetMaterial();
	Color color = material->color;
	if (material->texture != NULL) color = color * pri->GetTexture(collider->C);
	double eta = material->diff * color.Power();
	if (eta <= ran() * (*prob)) {
		*prob -= eta;
		return false;
	}

	photon.power = photon.power * color / color.Power();
	photon.dir = collider->N.Diffuse();
	subPhoton1.dir = photon.dir;
	subPhoton2.dir = photon.dir;
	photon.irr = 0;
	PhotonTracing(photon, subPhoton1, subPhoton2, dep + 1, refracted);
	return true;
}

bool Photontracer::PhotonReflection(Collider* collider, Collider* collider1, Collider* collider2, Photon photon, Photon subPhoton1, Photon subPhoton2, int dep, bool refracted, double* prob) {
	Primitive* pri = collider->GetPrimitive();
	Material* material = pri->GetMaterial();
	Color color = material->color;
	if (material->texture != NULL) color = color * pri->GetTexture(collider->C);
	double eta = material->refl * color.Power();

	if (eta <= ran() * (*prob)) {
		*prob -= material->refl;
		return false;
	}

	photon.power = photon.power * color / color.Power();

	photon.dir = photon.dir.Reflect(collider->N);
	subPhoton1.dir = subPhoton1.dir.Reflect(collider1->N);
	subPhoton2.dir = subPhoton2.dir.Reflect(collider2->N);
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
	photon.irr *= eta;
	PhotonTracing(photon, subPhoton1, subPhoton2, dep + 1, refracted);
	return true;
}

bool Photontracer::PhotonRefraction(Collider* collider, Collider* collider1, Collider* collider2, Photon photon, Photon subPhoton1, Photon subPhoton2, int dep, bool refracted, double* prob) {
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
	nextRefracted = refracted;
	subPhoton1.dir = subPhoton1.dir.Refract(collider1->N, n, &nextRefracted);
	nextRefracted = refracted;
	subPhoton2.dir = subPhoton2.dir.Refract(collider2->N, n, &nextRefracted);
	photon.irr *= eta;
	PhotonTracing(photon, subPhoton1, subPhoton2, dep + 1, nextRefracted);
	return true;
}

void Photontracer::PhotonTracing(Photon photon, Photon subPhoton1, Photon subPhoton2, int dep, bool refracted) {
	if (dep > MAX_PHOTONTRACING_DEP) return;
	Collider* collider = scene->FindNearestCollide(photon.pos, photon.dir);
	Collider* collider1 = scene->FindNearestCollide(subPhoton1.pos, subPhoton1.dir);
	Collider* collider2 = scene->FindNearestCollide(subPhoton2.pos, subPhoton2.dir);

	if (collider != NULL) {
		if (collider1 == NULL || collider2 == NULL) {
			std::cout << "Photon Tracing: accuracy error [1]" << std::endl;
			return;
		}

		Primitive* nearest_primitive = collider->GetPrimitive();
		Primitive* np1 = collider1->GetPrimitive();
		Primitive* np2 = collider2->GetPrimitive();
		if (nearest_primitive != np1 || nearest_primitive != np2) {
			if (collider1 == NULL || collider2 == NULL) {
				std::cout << "Photon Tracing: accuracy error [2]" << std::endl;
				return;
			}
		}

		photon.pos = collider->C;
		subPhoton1.pos = collider1->C;
		subPhoton2.pos = collider2->C;
		photon.hash = (photon.hash * 7 + nearest_primitive->GetSample()) % 10000007;
		if (nearest_primitive->GetMaterial()->diff > EPS) {
			if (photonmap != NULL) {
				Photon storePhoton = photon;
				float a = photon.pos.Distance(subPhoton1.pos);
				float b = photon.pos.Distance(subPhoton2.pos);
				float c = subPhoton1.pos.Distance(subPhoton2.pos);
				float p = (a + b + c) / 2;
				float area = sqrt(p * (p - a) * (p - b) * (p - c));
				storePhoton.irr *= (PERTURBATION * PERTURBATION / 2) / area;
				photonmap->Store(storePhoton);
			}
		}

		double prob = 1;
		if (PhotonDiffusion(collider, collider1, collider2, photon, subPhoton1, subPhoton2, dep, refracted, &prob) == false)
			if (PhotonReflection(collider, collider1, collider2, photon, subPhoton1, subPhoton2, dep, refracted, &prob) == false)
				if (PhotonRefraction(collider, collider1, collider2, photon, subPhoton1, subPhoton2, dep, refracted, &prob) == false);
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

			Photon subPhoton1 = photon;
			Photon subPhoton2 = photon;
			Vector3 Dx = photon.dir.GetAnVerticalVector();
			Vector3 Dy = photon.dir * Dx;
			Dx = Dx.GetUnitVector() * PERTURBATION;
			Dy = Dy.GetUnitVector() * PERTURBATION;
			subPhoton1.dir += Dx;
			subPhoton2.dir += Dy;

			photon.power *= total_power;
			PhotonTracing( photon, subPhoton1, subPhoton2, 1, false );
			light_power -= photon_power;
		}
		std::cout << "Stored photons: " << photonmap->GetStoredPhotons() << std::endl;
	}
	

	std::cout << "Tree balancing..." << std::endl;
	photonmap->Balance();
}
