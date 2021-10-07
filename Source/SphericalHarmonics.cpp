/*
*
* Implimentation of the spherical harmonics (see SphericalHarmonics.h)
*
* Forms part of the DxPRT project
*
*
*
* Author: Shaun Bailey
*
* Date created: 04/10/2021
*
* Last Updated: 04/10/2021
*
*
*
* MIT License
*
* Copyright (c) 2021 Shaun Bailey
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
*/

#include "DxPRT/SphericalHarmonics.h"


namespace DxPRT_Utility {

	std::vector<float> CalcSH(const size_t& N, const float& ctheta,
		const float& phi) 
	{

		std::vector<float> SHvec((N + 1ull) * (N + 1ull), 0.0f);

		// calculate Legendre polynomials
		for (long long  l = 0; l <= N; ++l)
		{
			for (long long m = l; m >= 0; --m)
			{
				float res;
				if (l == m) 
				{
					res = CalcLegendre1(m, ctheta);
				}
				else if (l == m + 1) 
				{
					float P1 = SHvec[m * m + 2ull * m];
					res = CalcLegendre2(m, ctheta, P1);
				}
				else 
				{
					float P1 = SHvec[l * l - l +m];
					float P2 = SHvec[l * l - 3ull * l + m + 2ull];
					res = CalcLegendre3(l, m, ctheta, P1, P2);
				}
				SHvec[l * l + l + m] = res;
			}
		}

		// apply coefficient
		for (long long l = 0; l <= N; ++l)
		{
			for (long long m = -(int)l; m <= l; ++m) 
			{
				SHvec[l * l + l + m] = CalcCoefficient(l, m, phi) * SHvec[l * l + l + abs(m)];
			}
		}

		return SHvec;
	}

	// l = m
	float CalcLegendre1(const long long& m, const float& x)
	{
		return pow(-1, m) * DoubleFactorical(2 * m - 1) *
			pow(1 - x * x, double(m) / 2);
	}

	// l = m + 1
	float CalcLegendre2(const long long& m, const float& x,
		const float& P1) 
	{
		return x * (2 * m + 1) * P1;
	}

	// otherwise
	float CalcLegendre3(const long long& l, const long long& m, const float& x,
		const float& P1, const float& P2) 
	{
		return (x * (2 * l - 1) * P1 - (l + m - 1) * P2)
			/ float(l - m);
	}

	float CalcCoefficient(const long long& l, const long long& m,
		const float& phi) 
	{
		static const float PI = 3.1415926f;
		float res = sqrt(float((2 * l + 1)) * Factorical(l - abs(m)) /
			(4.0f * PI * Factorical(l + abs(m))));
		if (m < 0) {
			res *= sqrt(2.0f) * sin(-m * phi);
		}
		else if (m > 0) {
			res *= sqrt(2.0f) * cos(m * phi);
		}
		return res;
	}

	float Factorical(const float& n) 
	{
		if (n > 1.0f) {
			return n * Factorical(n - 1.0f);
		}
		else {
			return 1.0f;
		}
	}

	float DoubleFactorical(const float& n) 
	{
		if (n > 1.0f) {
			return n * DoubleFactorical(n - 2.0f);
		}
		else {
			return 1.0f;
		}
	}



}
