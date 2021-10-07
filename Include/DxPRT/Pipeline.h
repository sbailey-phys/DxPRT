/*
*
* Class to simplify the use of the ID3D12PipelineState when rendering the
* PRT objects
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
#include <d3dcompiler.h>
#include "d3dx12.h"
#include <vector>
#include "DxPRT/RootSignature.h"
#include "DxPRT/DxPRT_Error.h"


namespace DxPRT_Utility {

	class Pipeline
	{
	public:

		// default initialization
		Pipeline();

		/*
		* Initialize: Initializes the pipeline
		*
		* This must be called before any other function is used
		*
		* _IN_ device: the currently active device
		*/
		void Initialize(ID3D12Device* device);

		/*
		* AddInput: adds in an element for the input assempler.
		* Currently no support for instancing
		* 
		* _IN_ semanticName: the name used in the shaders
		* _IN_ semanticIndex: the index used for this variable in the shader
		*                     (used if two contain the same name, for example
		*                      in an array)
		* _IN_ format: the DXGI_FORMAT describing the input data
		*/
		void AddInput(const LPCSTR &semanticName, const UINT &semanticIndex,
			const DXGI_FORMAT &format);

		/*
		* SetVertexShader/SetPixelShader: Reads in the vertex/pixel shader from 
		* a .cso file and sets it on the pipeline
		*
		* _IN_ shader: a LPCWSTR containing the path to the desired shader
		*/
		void SetVertexShader(const LPCWSTR& shader);
		void SetPixelShader(const LPCWSTR& shader);


		/*
		* SetRootSignature: Sets the root signature onto the pipeline
		*
		* _IN_ rootSig: A RootSignature object defining the root signature
		*/
		void SetRootSignature(const RootSignature& rootSig);


		/* 
		* SetPrimativeTopologyType: 
		* 
		* default = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE
		* 
		* _IN_ type: the desired D3D12_PRIMITIVE_TOPOLOGY_TYPE
		*/
		void SetPrimativeTopologyType(const D3D12_PRIMITIVE_TOPOLOGY_TYPE& type);
		

		/*
		* SetDSVFormat: Sets the format for the type used by the depth stencil view
		*
		* default: DXGI_FORMAT_D32_FLOAT
		* 
		* _IN_ format: the desired DXGI_FORMAT
		*/
		void SetDSVFormat(const DXGI_FORMAT& format);


		/*
		* SetRTVFormat: Sets the format for the type used by the render target view.
		*
		* default: DXGI_FORMAT_R8G8B8A8_UNORM
		*
		* _IN_ i: the index for the render target view (max 8)
		* _IN_ format: the desired DXGI_FORMAT
		*/
		void SetRTVFormat(const UINT& i, const DXGI_FORMAT& format);
		
		/*
		* SetNumRTV: set the number of render target views
		* 
		* default: 1
		* 
		* _IN_ i: the number of views to set
		*/
		void SetNumRTV(const UINT& i);


		// returns the underlying ID3D12PipelineState
		Microsoft::WRL::ComPtr<ID3D12PipelineState> GetPipeline() const; 


	private:


		/* SetShader: Reads in a generic shader file
		*
		* _IN_ shader: a LPCWSTR containing the path to the desired shader
		*/
		Microsoft::WRL::ComPtr<ID3DBlob> SetShader(const LPCWSTR &shader) const;
		
		Microsoft::WRL::ComPtr<ID3D12PipelineState> pipeline_;

		Microsoft::WRL::ComPtr<ID3DBlob> vertexShader_;
		Microsoft::WRL::ComPtr<ID3DBlob> pixelShader_;
		ID3D12RootSignature* pRootSig_ = nullptr;
		D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType_ =
			D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		DXGI_FORMAT DSVFormat_ = DXGI_FORMAT_D32_FLOAT;
		DXGI_FORMAT RTVFormats_[8];
		UINT numRTVs_ = 1;
		std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout_;

	};


}