#include"raytracer.h"
#include<cstdlib>
#include<iostream>
#define ran() ( double( rand() % RAND_MAX ) / RAND_MAX )

const double SPEC_POWER = 20;
const int MAX_DREFL_DEP = 2;
const int MAX_RAYTRACING_DEP = 10;
const int HASH_FAC = 7;
const int HASH_MOD = 10000007;

Raytracer::Raytracer() {
	scene = new Scene;
	photonmap = NULL;
}

Raytracer::~Raytracer() {
	if ( scene != NULL ) delete scene;
	if ( photonmap != NULL ) delete photonmap;
}

Color Raytracer::CalnDiffusion(Collider* collider, int* hash ) {
	Primitive* pri = collider->GetPrimitive();
	Color color = pri->GetMaterial()->color;

	if ( pri->GetMaterial()->texture != NULL ) color = color * pri->GetTexture(collider->C);
	
	Color ret = color * scene->GetBackgroundColor() * pri->GetMaterial()->diff;

	Camera* camera = scene->GetCamera();

	ret += color * photonmap->GetIrradiance( collider , camera->GetSampleDist() , camera->GetSamplePhotons() ) * pri->GetMaterial()->diff;

	return ret;
}

Color Raytracer::CalnReflection(Collider* collider, Vector3 ray_V , int dep, bool refracted, int* hash ) {
	Primitive* pri = collider->GetPrimitive();
	ray_V = ray_V.Reflect(collider->N);

	Color alpha = pri->GetMaterial()->color * pri->GetMaterial()->refl;
	return RayTracing(collider->C, ray_V, dep + 1, refracted, hash) * alpha;
}

Color Raytracer::CalnRefraction(Collider* collider, Vector3 ray_V , int dep, bool refracted, int* hash ) {
	Primitive* pri = collider->GetPrimitive();
	double n = pri->GetMaterial()->rindex;
	if (!refracted) n = 1 / n;

	bool nextRefracted = refracted;
	ray_V = ray_V.Refract(collider->N, n, &nextRefracted);

	Color alpha = Color(1, 1, 1) * pri->GetMaterial()->refr;
	if (refracted)
		alpha = alpha * (pri->GetMaterial()->absor * -collider->dist).Exp();
	Color rcol = RayTracing(collider->C, ray_V, dep + 1, nextRefracted, hash);
	return rcol * alpha;
}

Color Raytracer::RayTracing( Vector3 ray_O , Vector3 ray_V , int dep, bool refracted, int* hash ) {
	if (dep > MAX_RAYTRACING_DEP) return Color();
	if (hash != NULL) *hash = (*hash * HASH_FAC) % HASH_MOD;

	Color ret;
	Collider* collider = scene->FindNearestCollide(ray_O, ray_V);
	LightCollider* lightCollider = scene->FindNearestLight(ray_O, ray_V);

	if (lightCollider != NULL) {
		Light* nearest_light = lightCollider->GetLight();
		if (collider == NULL || lightCollider->dist < collider->dist) {
			if (hash != NULL) *hash = (*hash + nearest_light->GetSample()) % HASH_MOD;
			ret += nearest_light->GetColor() / nearest_light->GetColor().RGBMax();
		}
		delete lightCollider;
	}

	if (collider != NULL) {
		Primitive* nearest_primitive = collider->GetPrimitive();
		if (hash != NULL) *hash = (*hash + nearest_primitive->GetSample()) % HASH_MOD;
		if (nearest_primitive->GetMaterial()->diff > EPS) ret += CalnDiffusion(collider, hash);
		if (nearest_primitive->GetMaterial()->refl > EPS) ret += CalnReflection(collider, ray_V, dep, refracted, hash);
		if (nearest_primitive->GetMaterial()->refr > EPS) ret += CalnRefraction(collider, ray_V, dep, refracted, hash);
		delete collider;
	}

	if (dep == 1) ret = ret.Confine();
	return ret;
}

void Raytracer::Run() {
	Camera* camera = scene->GetCamera();
	scene->CreateScene( input );

	Photontracer* photontracer = new Photontracer;
	photontracer->SetScene( scene );
	photontracer->Run();
	photonmap = photontracer->GetPhotonmap();
	delete photontracer;

	Vector3 ray_O = camera->GetO();
	int H = camera->GetH() , W = camera->GetW();
	int** sample = new int*[H];
	for ( int i = 0 ; i < H ; i++ ) {
		sample[i] = new int[W];
		for ( int j = 0 ; j < W ; j++ )
			sample[i][j] = 0;
	}

#pragma omp parallel for
	for ( int i = 0 ; i < H ; i++ ) {
		std::cout << "Sampling:   " << i << "/" << H << std::endl;
		for ( int j = 0 ; j < W ; j++ ) {
			Vector3 ray_V = camera->Emit( i , j );
			Color color = RayTracing( ray_O , ray_V , 1 , false, &sample[i][j] );
			camera->SetColor( i , j , color );
		}
	}

#pragma omp parallel for
	for ( int i = 0 ; i < H ; i++ ) {
		std::cout << "Resampling: " << i << "/" << H << std::endl;
		for ( int j = 0 ; j < W ; j++ ) {
			if ( ( i == 0 || sample[i][j] == sample[i - 1][j] ) && ( i == H - 1 || sample[i][j] == sample[i + 1][j] ) &&
			     ( j == 0 || sample[i][j] == sample[i][j - 1] ) && ( j == W - 1 || sample[i][j] == sample[i][j + 1] ) ) continue;

			Color color;
			for ( int r = -1 ; r <= 1 ; r++ )
				for ( int c = -1 ; c <= 1 ; c++ ) {
					Vector3 ray_V = camera->Emit( i + ( double ) r / 3 , j + ( double ) c / 3 );
					color += RayTracing( ray_O , ray_V , 1 , false, NULL ) / 9;
				}
			camera->SetColor( i , j , color );
		}
	}
	
	for ( int i = 0 ; i < H ; i++ )
		delete[] sample[i];
	delete[] sample;

	Bmp* bmp = new Bmp( H , W );
	camera->Output( bmp );
	bmp->Output( output );
	delete bmp;

	system("Pause");
}
