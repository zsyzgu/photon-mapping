#include "Bezier.h"
#include "triangle.h"

Bezier::Bezier()
{
	n = m = 0;
	cnt = 0;
	points = NULL;
}

Bezier::~Bezier()
{
	if (points != NULL) {
		for (int i = 0; i <= n; i++) {
			delete[] points[i];
		}
		delete[] points;
	}
}

void Bezier::PreTreatment()
{

}

void Bezier::Input(std::string var, std::stringstream & fin)
{
	if (var == "N=") fin >> n;
	if (var == "M=") fin >> m;
	if (var == "newPoint=") {
		if (points == NULL) {
			points = new Vector3*[n + 1];
			for (int i = 0; i <= n; i++) {
				points[i] = new Vector3[m + 1];
			}
		}
		int i = cnt / (m + 1);
		int j = cnt % (m + 1);
		Vector3 pt;
		fin >> pt.x >> pt.y >> pt.z;
		points[i][j] = pt;
		cnt++;
	}
	Primitive::Input(var, fin);
}

float** Bezier::calnB(int n, float t) {
	float** B = new float*[n + 1];
	for (int i = 0; i <= n; i++) {
		B[i] = new float[n + 1];
	}

	B[0][0] = 1;
	for (int j = 1; j <= n; j++) {
		B[0][j] = (1 - t) * B[0][j - 1];
		for (int i = 1; i < j; i++) {
			B[i][j] = (1 - t) * B[i][j - 1] + t * B[i - 1][j - 1];
		}
		B[j][j] = t * B[j - 1][j - 1];
	}

	return B;
}

void Bezier::freeB(int n, float ** B)
{
	for (int i = 0; i <= n; i++) {
		delete[] B[i];
	}
	delete B;
}

Vector3 Bezier::calnP(float u, float v)
{
	Vector3 P;
	float** Bu = calnB(n, u);
	float** Bv = calnB(m, v);
	for (int i = 0; i <= n; i++) {
		for (int j = 0; j <= m; j++) {
			P += points[i][j] * Bu[i][n] * Bv[j][m];
		}
	}
	freeB(n, Bu);
	freeB(m, Bv);
	return P;
}

Vector3 Bezier::calnPu(float u, float v)
{
	Vector3 Pu;
	float** Bu = calnB(n, u);
	float** Bv = calnB(m, v);
	for (int i = 0; i <= n; i++) {
		for (int j = 0; j <= m; j++) {
			float dB = 0;
			if (i - 1 >= 0) {
				dB += n * Bu[i - 1][n - 1];
			}
			if (i <= n - 1) {
				dB -= n * Bu[i][n - 1];
			}
			Pu += points[i][j] * dB * Bv[j][m];
		}
	}
	freeB(n, Bu);
	freeB(m, Bv);
	return Pu;
}

Vector3 Bezier::calnPv(float u, float v)
{
	Vector3 Pv;
	float** Bu = calnB(n, u);
	float** Bv = calnB(m, v);
	for (int i = 0; i <= n; i++) {
		for (int j = 0; j <= m; j++) {
			float dB = 0;
			if (j - 1 >= 0) {
				dB += m * Bv[j - 1][m - 1];
			}
			if (j <= m - 1) {
				dB -= m * Bv[j][m - 1];
			}
			Pv += points[i][j] * Bu[i][n] * dB;
		}
	}
	freeB(n, Bu);
	freeB(m, Bv);
	return Pv;
}

Collider Bezier::accurateCollide(Vector3 ray_O, Vector3 ray_V, float u, float v, float t)
{
	Collider collider;
	for (int k = 0; k < 20; k++) {
		Vector3 P = calnP(u, v);
		Vector3 Pu = calnPu(u, v);
		Vector3 Pv = calnPv(u, v);
		Vector3 df = P - (ray_O + ray_V * t);
		if (df.Module() < EPS) {
			break;
		}
		float D = ray_V.Dot(Pu.Cross(Pv));
		t = t + Pu.Dot(Pv.Cross(df)) / D;
		u = u + ray_V.Dot(Pv.Cross(df)) / D;
		v = v - ray_V.Dot(Pu.Cross(df)) / D;
	}

	Vector3 P = calnP(u, v);
	Vector3 Pu = calnPu(u, v);
	Vector3 Pv = calnPv(u, v);
	Vector3 df = P - (ray_O + ray_V * t);
	if (df.Module() >= EPS || t < EPS) {
		collider.crash = false;
		return collider;
	}
	collider.crash = true;
	collider.C = P;
	collider.dist = t;
	collider.I = ray_V;
	collider.N = Pv.Cross(Pu).GetUnitVector();
	collider.front = ray_V.Dot(collider.N) < 0;
	collider.SetPrimitive(this);
	return collider;
}

Collider Bezier::Collide(Vector3 ray_O, Vector3 ray_V)
{
	const int N = 10;
	Vector3 points[N + 1][N + 1];

	for (int i = 0; i <= N; i++) {
		for (int j = 0; j <= N; j++) {
			float u = 1.0 / N * i;
			float v = 1.0 / N * j;
			points[i][j] = calnP(u, v);
		}
	}

	Collider collider;

	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			Triangle tri;
			tri.SetPos(0, points[i][j]);
			tri.SetPos(1, points[i + 1][j]);
			tri.SetPos(2, points[i][j + 1]);
			tri.PreTreatment();
			Collider col = tri.Collide(ray_O, ray_V);
			if (col.crash) {
				float u = 1.0 / N * (i + 0.5); 
				float v = 1.0 / N * (j + 0.5);
				float t = col.dist;
				col = accurateCollide(ray_O, ray_V, u, v, t);
				if (col.crash) {
					if (collider.crash == false || collider.dist > col.dist) {
						collider = col;
					}
				}
			}
			Triangle tri2;
			tri2.SetPos(0, points[i + 1][j + 1]);
			tri2.SetPos(1, points[i][j + 1]);
			tri2.SetPos(2, points[i + 1][j]);
			tri2.PreTreatment();
			col = tri2.Collide(ray_O, ray_V);
			if (col.crash) {
				float u = 1.0 / N * (i + 0.5);
				float v = 1.0 / N * (j + 0.5);
				float t = col.dist;
				col = accurateCollide(ray_O, ray_V, u, v, t);
				if (col.crash) {
					if (collider.crash == false || collider.dist > col.dist) {
						collider = col;
					}
				}
			}
		}
	}

	collider.SetPrimitive(this);
	return collider;
}
