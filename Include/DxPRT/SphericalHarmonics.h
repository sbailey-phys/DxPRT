/*
*
* Calculates a vector of spherical harmonics evaluate at a particular 
* value of theta and phi. This is used to generate grids of spherical
* harmoincs that is used in the integration
*
*
* This is part of the implimentation and is not intended for public
* use.
*
*
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

#pragma once

#include<vector>
#include <iostream>


namespace DxPRT_Utility {


	/*
	* CalcSH: calculates a vector of spherical harmonics evaluated at a 
	* particular theta and phi
	* 
	* _IN_ N: the maximum value of l to be calculated
	* _IN_ ctheta: the value of cos(theta) where the SHs shall be evaluated
	* _IN_ phi: the value of phi where the SHs will be evaluated
	*/
	std::vector<float> CalcSH(const size_t& N, const float& ctheta
		, const float& phi);


	/*
	* CalcLegendre1: calculates the Legendre function when m = l
	* 
	* _IN_ m
	* _IN_ x: (or cos(theta))
	*/
	float CalcLegendre1(const long long& m, const float& x);


	/*
	* CalcLegendre2: calculates the Legendre function when l = m + 1
	*
	* _IN_ m
	* _IN_ x: (or cos(theta))
	* _IN_ P1: the Legendre function with m_P1 = m, l_P1 = m
	*/
	float CalcLegendre2(const long long& m, const float& x,
		const float& P1);


	/*
	* CalcLegendre3: calculates the Legendre function when the other two functions
	* do not apply
	*
	* _IN_ l
	* _IN_ m
	* _IN_ x: (or cos(theta))
	* _IN_ P1: the Legendre function with m_P1 = m, l_P1 = l-1
	* _IN_ P2: the Legendre function with m_P2 = m, l_P2 = l-2
	*/
	float CalcLegendre3(const long long& l, const long long& m, const float& x,
		const float& P1, const float& P2);

	/*
	* CalcCoefficient: Calculates the normalization of the spherical harmonic
	* 
	* _IN_ l
	* _IN_ m
	* _IN_ phi
	*/
	float CalcCoefficient(const long long& l, const long long& m,
		const float& phi);

	/*
	* Factorial: calculates n!. This is done using floats to avoid overload
	* 
	* _IN_ n
	*/
	float Factorical(const float& n);


	/*
	* DoubleFactorial: calculates n!!. This is done using floats to avoid overload
	* 
	* _IN_ n
	*/
	float DoubleFactorical(const float& n);

}