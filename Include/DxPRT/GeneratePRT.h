/*
*
* This file contains the functions nessecary to generate the .prt file containing the spherical harmonic functions.
* This is split into two parts: functions that process the environment map, provided in the high dynamic range (hdr),
* and functions to process meshes to process the transfer functions. For the latter, the visibilty is taken into accout
* through the use of an in-built ray tracer, allowing for the generation of shadows.
* 
* Currently, only coefficients for defuse objects can be creates, and multi-bounce rays are not yet supported
* 
* An in built .hdr reader is provided to provide a simple method to process the environment map. This only applys to those in
* the RGBE format and are run-length encoded. If the data is not in this format, then the raw data (in floats) can also
* be read in. In all cases, it is expected that phi varies along the x-direction and theta vaires along the y-direction, 
* where (theta, phi) are the standard spherical cooridinates.
* 
* A in-built .obj reader is also provided to read in meshes, although the raw vertex, index and normal data can be 
* provided if nessecary. It should be noted that the ray tracer may be rather slow for large meshes (>10000 vertices),
* however work is under way to improve this
* 
* To set the settings of the integration performed, EM_DESC and PRT_DESC objects need to be defined. Each member of these
* objects has a default value so it is not nessecary to define each element unless a different value is defined. Please
* see the definitions below for each element. However, note that some values (paricularly the number of Monte Carlo events)
* may be increased such that all threads 
* 
* All functions in this file will generate a PRT file. These can then be read in by the Workspace class (see Workspace.h)
* to render the desired objects lit by the surounding environment map.
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
* Last Updated: 06/10/2021
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
#include <string>
#include <Windows.h>
#include <d3d12.h>
#include "DxPRT/CommandList.h"
#include "DxPRT/CommandQueue.h"
#include "DxPRT/SphericalHarmonics.h"
#include "DxPRT/Resource.h"
#include "DxPRT/DescriptorHeap.h"
#include "DxPRT/PRTWriter.h"
#include "DxPRT/ObjReader.h"
#include "DxPRT/HDRReader.h"
#include "DxPRT/GenerateEM_Utility.h"
#include "DxPRT/GeneratePRT_Utility.h"
#include "DxPRT/GenerateGeneral_Utility.h"

namespace DxPRT {

	// the EM_DESC object used to define the integration over the environment map in GenerateEM
	struct EM_DESC {
		UINT64 MaxL = 3; // maximum l value for the spherical harmonics
		UINT64 NumEvents = 262144; // total number of events used in the Monte Carlo Integration
		UINT64 SHGridNum = 512; // the number of grid points (in both theta and phi) used to store the spherical harmonics
		bool SuppressOutput = false; // if set to true, no text will be output to the console
		std::wstring shaderPath = L""; // path to the folder containing the shader files
	};


	// the EM_DESC object used to define the integration over the transfer funtions in GeneratePRT
	struct PRT_DESC {
		UINT64 MaxL = 3; // maximum l value for the spherical harmonics
		UINT64 NumEvents = 262144; // total number of events used in the Monte Carlo Integration
		UINT64 SHGridNum = 512; // the number of grid points (in both theta and phi) used to store the spherical harmonics
		bool SuppressOutput = false; // if set to true, no text will be output to the console
		std::wstring shaderPath = L""; // path to the folder containing the shader files
	};

	/*
	* GenerateEM: processes and environment map to generate a .prt file containing the 
	* spherical harmonic coefficients. The data file must have phi vary along the 
	* x-direction and theta vary along the y-direction. If the output file cannot be
	* accessed, then this function will fail.
	* 
	* _IN_ device: the currently active device
	* _IN_ numPixelsX: the number of pixels in the x-direction (width)
	* _IN_ numPixelsY: the number of pixels in the y-direction (height)
	* _IN_ outFile: the path to the output file where the coefficients will be stored
	* _IN_ desc: an EM_DESC object containing parameters for the integration
	*/
	void GenerateEM(ID3D12Device* device, void* data,
		const UINT64& numPixelsX, const UINT64& numPixelsY,
		const std::string& outFile, const EM_DESC& desc);

	/*
	* GenerateEM: performs the same operation as the above function but takes in a .hdr
	* file as input. This file must be in the RGBE format and be run-length encoded. If
	* the file cannot be read, then the function will fail and you should instead consider
	* uing the above function instead.
	* 
	* _IN_ device: the currently active device
	* _IN_ hdrFile: the path to the hdr file to be read
	* _IN_ outFile: the path to the output file where the coefficients will be stored
	* _IN_ desc: an EM_DESC object containing parameters for the integration
	*/
	void GenerateEM(ID3D12Device* device, const std::string& hdrFile,
		const std::string& outFile, const EM_DESC& desc);


	/*
	* GeneratePRT: processes a mesh to generate the spherical harmoic coefficients to describe
	* the transfer function. This also includes a ray tracer to take into account the effects
	* of shadows. If the output file cannot be accessed, then this function will fail.
	* 
	* _IN_ device: the currently active device
	* _IN_ vertexData: a pointer to the vertex data, this should contain 3 floats per vertex
	* _IN_ vertexNum: the total number of vertices in the mesh
	* _IN_ indexData: a pointer to the index data, this should contain 3 4-byte unsigned integers per triangle
	* _IN_ triangleNum: the total number of triangles in the mesh
	* _IN_ normalData: a pointer to the normal data, this should contain 3 floats per normal
	* _IN_ outFile: the path to the output file where the coefficients for each vertex will be stored
	* _IN_ desc: an PRT_DESC object containing parameters for the ray tracer and integration
	*/
	void GeneratePRT(ID3D12Device* device, void* vertexData,
		const UINT64& vertexNum, void* indexData, const UINT64& triangleNum,
		void* normalData, const std::string& outFile,
		const PRT_DESC& desc);


	/*
	* GeneratePRT: same functionallity as the above function but takes in a .obj file as input
	* 
	* 
	* _IN_ device: the currently active device
	* _IN_ objFile: the path to the object file to be read
	* _IN_ outFile: the path to the output file where the coefficients for each vertex will be stored
	* _IN_ desc: an PRT_DESC object containing parameters for the ray tracer and integration
	
	*/
	void GeneratePRT(ID3D12Device* device, const std::string& objFile,
		const std::string& outFile, const PRT_DESC& desc);

}

