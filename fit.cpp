#include "fit.h"
#include <iostream>

//按第一行展开计算|A|
float getA(float arcs[3][3], int n)
{
	if (n == 1)
	{
		return arcs[0][0];
	}
	float ans = 0;
	float temp[3][3] = { 0.0 };
	int i, j, k;
	for (i = 0; i<n; i++)
	{
		for (j = 0; j<n - 1; j++)
		{
			for (k = 0; k<n - 1; k++)
			{
				temp[j][k] = arcs[j + 1][(k >= i) ? k + 1 : k];

			}
		}
		float t = getA(temp, n - 1);
		if (i % 2 == 0)
		{
			ans += arcs[0][i] * t;
		}
		else
		{
			ans -= arcs[0][i] * t;
		}
	}
	return ans;
}

//计算每一行每一列的每个元素所对应的余子式，组成A*
void  getAStart(float arcs[3][3], int n, float ans[3][3])
{
	if (n == 1)
	{
		ans[0][0] = 1;
		return;
	}
	int i, j, k, t;
	float temp[3][3];
	for (i = 0; i<n; i++)
	{
		for (j = 0; j<n; j++)
		{
			for (k = 0; k<n - 1; k++)
			{
				for (t = 0; t<n - 1; t++)
				{
					temp[k][t] = arcs[k >= i ? k + 1 : k][t >= j ? t + 1 : t];
				}
			}


			ans[j][i] = getA(temp, n - 1);  //此处顺便进行了转置
			if ((i + j) % 2 == 1)
			{
				ans[j][i] = -ans[j][i];
			}
		}
	}
}

//得到给定矩阵src的逆矩阵保存到des中。
bool GetMatrixInverse(float src[3][3], int n, float des[3][3])
{
	float flag = getA(src, n);
	float t[3][3];
	if (0 == flag)
	{
		return false;//如果算出矩阵的行列式为0，则不往下进行
	}
	else
	{
		getAStart(src, n, t);
		for (int i = 0; i<n; i++)
		{
			for (int j = 0; j<n; j++)
			{
				des[i][j] = t[i][j] / flag;
			}

		}
	}

	return true;
}

bool Fit::linearFit(std::vector<float> xs, std::vector<float> ys, std::vector<float> zs)
{
	float x = 0, y = 0, z = 0, xx = 0, xy = 0, xz = 0, yy = 0, yz = 0;
	int n = xs.size();
	for (int i = 0; i < n; i++) {
		x += xs[i];
		y += ys[i];
		z += zs[i];
		xx += xs[i] * xs[i];
		xy += xs[i] * ys[i];
		xz += xs[i] * zs[i];
		yy += ys[i] * ys[i];
		yz += ys[i] * zs[i];
	}
	float A[3][3] = {
		{xx, xy, x},
		{xy, yy, y},
		{x, y, n}
	};
	float B[3] = { xz, yz, z };
	float invA[3][3];
	bool flag = GetMatrixInverse(A, 3, invA);
	if (flag) {
		a = invA[0][0] * B[0] + invA[0][1] * B[1] + invA[0][2] * B[2];
		b = invA[1][0] * B[0] + invA[1][1] * B[1] + invA[1][2] * B[2];
		c = invA[2][0] * B[0] + invA[2][1] * B[1] + invA[2][2] * B[2];
		float ssr = 0;
		float sse = 0;
		float meanZ = 0;
		for (int i = 0; i < n; i++) {
			meanZ += zs[i];
		}
		meanZ /= n;
		for (int i = 0; i < n; i++) {
			float zPredict = getValue(xs[i], ys[i]);
			ssr += (zPredict - meanZ) * (zPredict - meanZ);
			sse += (zPredict - zs[i]) * (zPredict - zs[i]);
		}
		r2 = 1 - (sse / (ssr + sse));
	}
	return flag;
}

float Fit::getValue(float x, float y)
{
	return a * x + b * y + c;
}

float Fit::getR2()
{
	return r2;
}
