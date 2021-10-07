/*
*
* This file contains structures used to pass data to the vertex and pixel shaders
* that render the prt object and skybox (see Workspace.cpp)
*
*
* Forms part of the DxPRT project
*
*
*
* Author: Shaun Bailey
*
* Date created: 05/10/2021
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
#include <DirectXMath.h>
#include <d3d12.h>

namespace DxPRT_Utility {
	// structure used to pass data as root constants to the prt shader
	struct PRTInData {
		DirectX::XMMATRIX matrix;
		float exposure;
		UINT32 prtMaxL;
		UINT32 emMaxL;
		UINT32 setMaxL;
	};

	// structure used to pass data as root consaants to the skybox shader
	struct SkyboxInData {
		DirectX::XMMATRIX matrix;
		float exposure;
	};
}