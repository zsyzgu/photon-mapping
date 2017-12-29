#include"photonmap.h"
#include<iostream>
#include<cmath>
#include<algorithm>
#include <map>
#include <vector>
#include "fit.h"

const double INF = 1e8;

Nearestphotons::Nearestphotons() {
	max_photons = found = 0;
	got_heap = false;
	dist2 = NULL;
	photons = NULL;
}

Nearestphotons::~Nearestphotons() {
	delete[] dist2;
	delete[] photons;
}

Photonmap::Photonmap( int size ) {
	max_photons = size;
	stored_photons = 0;
	photons = new Photon[size + 1];
	box_min = Vector3( INF , INF , INF );
	box_max = Vector3( -INF , -INF , -INF );
}

Photonmap::~Photonmap() {
	delete[] photons;
}

void Photonmap::Store( Photon photon ) {
	if ( stored_photons >= max_photons ) return;
	photons[++stored_photons] = photon;
	box_min = Vector3( std::min( box_min.x , photon.pos.x ) , std::min( box_min.y , photon.pos.y ) , std::min( box_min.z , photon.pos.z ) );
	box_max = Vector3( std::max( box_max.x , photon.pos.x ) , std::max( box_max.y , photon.pos.y ) , std::max( box_max.z , photon.pos.z ) );
}

void Photonmap::Balance() {
	Photon** pbal = new Photon*[stored_photons + 1];
	Photon** porg = new Photon*[stored_photons + 1];

	for ( int i = 0 ; i <= stored_photons ; i++ )
		porg[i] = &photons[i];
	
	BalanceSegment( pbal, porg , 1 , 1 , stored_photons );
	delete[] porg;

	int j = 1, foo = 1;
	Photon foo_photon = photons[j];
	for (int i = 1; i <= stored_photons; i++) {
		int d = pbal[j] - photons;
		pbal[j] = NULL;
		if (d != foo) {
			photons[j] = photons[d];
		}
		else {
			photons[j] = foo_photon;

			if (i < stored_photons) {
				for (; foo <= stored_photons; foo++) {
					if (pbal[foo] != NULL) {
						break;
					}
				}
				foo_photon = photons[foo];
				j = foo;
			}
			continue;
		}
		j = d;
	}
	delete[] pbal;
}

void Photonmap::BalanceSegment( Photon** pbal, Photon** porg , int index , int st , int en ) {
	int med = 1;
	while ( 4 * med <= en - st + 1 ) med <<= 1;

	if ( 3 * med <= en - st + 1 ) med = med * 2 + st - 1;
		else med = en + 1 - med;
	
	int axis = 2;
	if ( box_max.x - box_min.x > box_max.y - box_min.y && box_max.x - box_min.x > box_max.z - box_min.z ) axis = 0; else
	if ( box_max.y - box_min.y > box_max.z - box_min.z ) axis = 1;

	MedianSplit( porg , st , en , med , axis );
	pbal[index] = porg[med];
	pbal[index]->plane = axis;

	if ( st < med ) {
		if (st < med - 1) {
			double tmp = box_max.GetCoord(axis);
			box_max.GetCoord(axis) = pbal[index]->pos.GetCoord(axis);
			BalanceSegment(pbal, porg, index * 2, st, med - 1);
			box_max.GetCoord(axis) = tmp;
		}
		else {
			pbal[index * 2] = porg[st];
		}
	}

	if ( med < en ) {
		if (med + 1 < en) {
			double tmp = box_min.GetCoord(axis);
			box_min.GetCoord(axis) = pbal[index]->pos.GetCoord(axis);
			BalanceSegment(pbal, porg, index * 2 + 1, med + 1, en);
			box_min.GetCoord(axis) = tmp;
		}
		else {
			pbal[index * 2 + 1] = porg[en];
		}
	}
}

void Photonmap::MedianSplit( Photon** porg , int st , int en , int med , int axis ) {
	int l = st , r = en;

	while ( l < r ) {
		double key = porg[r]->pos.GetCoord( axis );
		int i = l - 1 , j = r;
		for ( ; ; ) {
			while ( porg[++i]->pos.GetCoord( axis ) < key );
			while ( porg[--j]->pos.GetCoord( axis ) > key && j > l );
			if ( i >= j ) break;
			std::swap( porg[i] , porg[j] );
		}

		std::swap( porg[i] , porg[r] );
		if ( i >= med ) r = i - 1;
		if ( i <= med ) l = i + 1;
	}
}

Color Photonmap::GetIrradiance(Collider* collider, double max_dist, int n) {
	Color ret;

	Nearestphotons np;
	np.pos = collider->C;
	np.max_photons = n;
	np.dist2 = new double[n + 1];
	np.photons = new Photon*[n + 1];
	np.dist2[0] = max_dist * max_dist;

	LocatePhotons(&np, 1);

	std::map<int, std::vector<Photon> > hitPhotons;
	for (int i = 1; i <= np.found; i++) {
		Vector3 dir = np.photons[i]->dir;
		if (collider->N.Dot(dir) < 0) {
			int hash = np.photons[i]->hash;
			if (np.photons[i]->irr < EPS) {
				hash = -1;
			}
			hitPhotons[hash].push_back(*np.photons[i]);
		}
	}

	std::map<int, std::vector<Photon> >::iterator it = hitPhotons.begin();
	for (; it != hitPhotons.end(); it++) {
		int n = it->second.size();

		if (it->second[0].irr < EPS || n < 4) {
			Color color;
			for (int i = 0; i < n; i++) {
				color += it->second[i].power * collider->GetPrimitive()->GetMaterial()->BRDF(-it->second[i].dir, collider->N, -collider->I);
			}
			color = color * (4 / (emit_photons * np.dist2[0]));
			ret += color;
		} else {
			Color color;

			float sigma2 = np.dist2[0] / 9;
			float denom = 1.0 / n; //should be 0 accurately, add a black point at (0,0) for soft shadow
			for (int i = 0; i < n; i++) {
				float dist2 = collider->C.Distance2(it->second[i].pos);
				float k = exp(-dist2 / sigma2);
				denom += k;
				float BRDF = collider->GetPrimitive()->GetMaterial()->BRDF(-it->second[i].dir, collider->N, -collider->I);
				color += it->second[i].power * it->second[i].irr * BRDF * k;
			}
			color = color / denom;
			ret += color;

			/*float aveIrr = 0;
			for (int i = 0; i < n; i++) {
				color += it->second[i].power * collider->GetPrimitive()->GetMaterial()->BRDF(-it->second[i].dir, collider->N, -collider->I);
				aveIrr += it->second[i].irr;
			}
			color = color / n;
			aveIrr = aveIrr / n;

			ret += color * aveIrr;*/

			/*Fit fit;
			Vector3 Dx = collider->N.GetAnVerticalVector();
			Vector3 Dy = (collider->N * Dx).GetUnitVector();
			std::vector<float> xs, ys, zs;
			for (int i = 0; i < n; i++) {
				xs.push_back((it->second[i].pos - collider->C).Dot(Dx));
				ys.push_back((it->second[i].pos - collider->C).Dot(Dy));
				zs.push_back(it->second[i].irr);
			}
			std::vector<float> atans;
			for (int i = 0; i < n; i++) {
				atans.push_back(atan2(xs[i], ys[i]));
			}
			sort(atans.begin(), atans.end());
			atans.push_back(atans[0] + 2 * 3.1415927);
			bool flag = false;
			for (int i = 0; i < n; i++) {
				if (atans[i + 1] - atans[i] > 3.1415927) {
					flag = true;
				}
			}
			if (flag) {
				color = color * n * (4 / (emit_photons * np.dist2[0]));
				ret += color;
			} else

			if (fit.linearFit(xs, ys, zs)) {
				if (abs(fit.getValue(0, 0) - aveIrr) > 0.05) {
					std::cout << " =========== " << n << " " << fit.getValue(0, 0) << " " << aveIrr << " " << fit.getR2() << std::endl;
					for (int i = 0; i < n; i++) {
						std::cout << xs[i] << ", " << ys[i] << ", " << zs[i] << std::endl;
					}
				}

				ret += fit.getValue(0, 0);
			}*/
		}
	}

	/*for (int i = 1; i <= np.found; i++) {
		Vector3 dir = np.photons[i]->dir;
		if (collider->N.Dot(dir) < 0) {
			ret += np.photons[i]->power * collider->GetPrimitive()->GetMaterial()->BRDF(-dir, collider->N, -collider->I);
		}
	}

	ret = ret * (4 / (emit_photons * np.dist2[0]));*/
	return ret;
}

void Photonmap::LocatePhotons( Nearestphotons* np , int index ) {
	if ( index > stored_photons ) return;
	Photon *photon = &photons[index];

	if ( index * 2 <= stored_photons ) {
		double dist = np->pos.GetCoord( photon->plane ) - photon->pos.GetCoord( photon->plane );
		if ( dist < 0 ) {
			LocatePhotons( np , index * 2 );
			if ( dist * dist < np->dist2[0] ) LocatePhotons( np , index * 2 + 1 );
		} else {
			LocatePhotons( np , index * 2 + 1 );
			if ( dist * dist < np->dist2[0] ) LocatePhotons( np , index * 2 );
		}
	}

	double dist2 = photon->pos.Distance2( np->pos );
	if ( dist2 > np->dist2[0] ) return;

	if ( np->found < np->max_photons ) {
		np->found++;
		np->dist2[np->found] = dist2;
		np->photons[np->found] = photon;
	} else {
		if ( np->got_heap == false ) {
			for ( int i = np->found >> 1 ; i >= 1 ; i-- ) {
				int par = i;
				Photon* tmp_photon = np->photons[i];
				double tmp_dist2 = np->dist2[i];
				while ( ( par << 1 ) <= np->found ) {
					int j = par << 1;
					if ( j + 1 <= np->found && np->dist2[j] < np->dist2[j + 1] ) j++;
					if ( tmp_dist2 >= np->dist2[j] ) break;
					
					np->photons[par] = np->photons[j];
					np->dist2[par] = np->dist2[j];
					par = j;
				}
				np->photons[par] = tmp_photon;
				np->dist2[par] = tmp_dist2;
			}
			np->got_heap = true;
		}

		int par = 1;
		while ( ( par << 1 ) <= np->found ) {
			int j = par << 1;
			if ( j + 1 <= np->found && np->dist2[j] < np->dist2[j + 1] ) j++;
			if ( dist2 > np->dist2[j] ) break;

			np->photons[par] = np->photons[j];
			np->dist2[par] = np->dist2[j];
			par = j;
		}
		np->photons[par] = photon;
		np->dist2[par] = dist2;

		np->dist2[0] = np->dist2[1];
	}
}
