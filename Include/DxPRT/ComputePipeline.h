/*
*
* Class to simplify the use of the ID3D12PipelineState when generating the
* PRT spheical harmonic coefficients.
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

#include <wrl.h>
#include <d3d12.h>
#include "d3dx12.h"
#include <d3dcompiler.h>
#include <vector>
#include "DxPRT/RootSignature.h"
#include "DxPRT/DxPRT_Error.h"


namespace DxPRT_Utility {

	class ComputePipeline
	{
	public:

		// default initialization
		ComputePipeline();


		/*
		* Initialize: Initializes the compute pipeline
		* 
		* This must be called before any other function is used
		* 
		* _IN_ device: the currently active device
		*/
		void Initialize(ID3D12Device* device);

		/*
		* SetComputeShader: Reads in the compute shader from a .cso file
		* and sets it on the pipeline
		* 
		* _IN_ shader: a LPCWSTR containing the path to the desired shader
		*/
		void SetComputeShader(const LPCWSTR& shader);

		/*
		* SetRootSignature: Sets the root signature onto the pipeline
		* 
		* _IN_ rootSig: A RootSignature object defining the root signature
		*/
		void SetRootSignature(const RootSignature& rootSig);

		// returns the underlying ID3D12Pipeline object
		Microsoft::WRL::ComPtr<ID3D12PipelineState> GetPipeline() const;


	private:

		/* SetShader: Reads in a generic shader file
		* 
		* _IN_ shader: a LPCWSTR containing the path to the desired shader
		*/
		Microsoft::WRL::ComPtr<ID3DBlob> SetShader(const LPCWSTR &shader) const;


		Microsoft::WRL::ComPtr<ID3D12PipelineState> pipeline_;
		Microsoft::WRL::ComPtr<ID3DBlob> computeShader_;
		ID3D12RootSignature* pRootSig_ = nullptr;

	};

}