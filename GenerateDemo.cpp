/*
*
* This file contains a demo to show the use of the GeneratePRT and GenerateEM functions in the
* DxPRT project.
*
* Note that this demo generates 100 coefficients for the environment map and each vertex of the
* mesh. More Monte Carlo events are used for the environment map as this is a more complicated 
* funtion and only one integration is required.
* 
* To use this demo, please ensure that the files are correctly linked, in paritular ensure that the
* .obj and .hdr files are in the current directory, the header folder DxPRT is in the include directory
* and the source code is in the current project. Also ensure that all the relevant DirectX libraries are
* loaded, in paricular "d3dx12.h" is required.
*
*
* Author: Shaun Bailey
*
* Date created: 06/10/2021
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



#define WINDOWS_LEAN_AND_MEAN
#include <d3d12.h>
#include <dxgi1_6.h>
#include "DxPRT/GeneratePRT.h"



int main() 
{

	// create device on default addaptor
	Microsoft::WRL::ComPtr<ID3D12Device5> device;
	Microsoft::WRL::ComPtr<IDXGIFactory5> dxgiFactory5;
	CreateDXGIFactory2(0, IID_PPV_ARGS(&dxgiFactory5));
	D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&device));


	DxPRT::PRT_DESC descPRT; // define the integration for GeneratePRT
	descPRT.NumEvents = 50000;
	descPRT.MaxL = 9;
	descPRT.shaderPath = L"Shaders";

	// generate PRT file
	DxPRT::GeneratePRT(device.Get(), "Bunny.obj", "newBunny9_2.prt", descPRT);

	DxPRT::EM_DESC descEM; // define the integration for GenerateEM
	descEM.MaxL = 9;
	descEM.NumEvents = 5000000;
	descEM.shaderPath = L"Shaders";

	// generate EM file
	DxPRT::GenerateEM(device.Get(), "lilienstein_2k.hdr",
		"newFieldEM9_2.prt", descEM);

	return 1;
}


