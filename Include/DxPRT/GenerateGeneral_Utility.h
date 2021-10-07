/*
*
* General utility functions used by both the GenerateEM and GeneratePRT functions.
* (see GeneratePRT.h and GeneratePRT_Utility.h)
*
*
* This class is part of the implimentation and is not intended for public
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

#include <vector>
#include <d3d12.h>
#include "DxPRT/SphericalHarmonics.h"

namespace DxPRT_Utility {


	/*
	* GenerateSHVector: generates grids of spherical harmonics
	* 
	* _IN_ shGridNum: the number of grid points in both the theta and phi directions
	* _IN_ maxL: the maximum value of l for the generated spherical harmonics
	* _OUT_shVector: a vector containing grids of sphierical hamonics for each l and m value (the grid is
	*                also stored as a vector)
	*/
	void GenerateSHvector(const UINT64& shGridNum, const UINT64& maxL, std::vector<std::vector<float>>& shVector);

	/*
	* GenerateRandomVector: generates a vector of random integers between 128 and 2^32, used for the 
	* generation of random numbers on the GPU
	* 
	* _IN_ numEvents: the number of Monte Carlo Events that will need random numbers
	* _OUT_ randomVector: the vector containing the random numbers
	*/
	void GenerateRandomVector(const UINT64& numEvents, std::vector<UINT32>& randomVector);


	/*
	* RoundInput: increases the value of the numEvents and shGridNum such that they fill out the entire of 
	* each thread group when running the compute shader. In particular, numEvents will be increased such
	* that it can be written in the form (8n)^2 where n is an integer and shGridNum will be increased such
	* it an be written as 8m, where m is an integer
	* 
	* _IN_ numEvents: the input number of Monte Carlo events
	* _IN_ shGridNums: the input number of spherical harmonic grid points along the theta and phi direction 
	* _OUT_ roundedNumEvents: the incresed of Monte Carlo events 
	* _OUT_ roundedNumEventsX: the square root of roundedNumEvents
	* _OUT_ roundedSHGridNum: the increased number of spherical harmonic grid points along the theta and phi direction 
	*/
	void RoundInput(const UINT64& numEvents, const UINT64& shGridNum, UINT64& roundedNumEvents,
		UINT64& roundedNumEventsX, UINT64& roundedSHGridNum);


}





