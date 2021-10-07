/*
*
* Class to simplify the use of the ID3D12RootSignature when generating the
* PRT spheical harmonic coefficients and rendering the final objects.
* 
* Please note:
* This class also handles the root parameters which can be added using functions
* like "AddConstants". The parameter index is determined by order in which they are
* added
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
#include "DxPRT/DescriptorHeap.h"
#include <vector>
#include "DxPRT/DxPRT_Error.h"


namespace DxPRT_Utility {

	class RootSignature
	{
	public:

		// default initialization
		RootSignature();

		/* Initialize: initializes the root signature. 
		* 
		* Should be called after all parameters have been set.
		* 
		* _IN_ device: the currently active device
		* _IN_ addSampler: if set to true a static linear sampler will
		*                  be added to the root signature
		*/
		void Initialize(ID3D12Device* device, const bool &addSampler = false);


		/*
		* SetFlags:
		* 
		* default: D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		*	D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		*	D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		*	D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
		* 
		* _IN_ flags: the root signature flags to be set
		*/
		void SetFlags(const D3D12_ROOT_SIGNATURE_FLAGS& flags);
		

		/* 
		* AddConstants: adds root constants to the root signature. 
		* 
		* _IN_ size: the size, in 4 byte chunks, of the constants to be added
		* _IN_ shaderRegister: the shader register where the constants will be added
		* _IN_ visibility: the visibility of the constants to the various shader stages
		*/
		void AddConstants(const UINT& size, const UINT& shaderRegister,
			const D3D12_SHADER_VISIBILITY& visibility = D3D12_SHADER_VISIBILITY_ALL);

		/*
		* AddConstants: adds a descriptor table to the root signature.
		*
		* _IN_ type: type of descriptor to be added
		* _IN_ num: the number of descriptors in the table
		* _IN_ shaderRegister: the shader register where the constants will be added
		* _IN_ visibility: the visibility of the constants to the various shader stages
		*/
		void AddDesciptorTable(const D3D12_DESCRIPTOR_RANGE_TYPE &type,
			const UINT &num, const UINT &shaderRegister,
			const D3D12_SHADER_VISIBILITY& shaderVisibility = D3D12_SHADER_VISIBILITY_ALL);

		// returns the underlying ID3D12RootSignature object
		Microsoft::WRL::ComPtr<ID3D12RootSignature> GetRootSignature() const;


	private:

		std::vector<CD3DX12_ROOT_PARAMETER1> parameters_;
		std::vector<CD3DX12_DESCRIPTOR_RANGE1> descRanges_;

		// this is the default used by this demo, other values
		// could also be chosen
		D3D12_ROOT_SIGNATURE_FLAGS flags_ =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

		Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;

		std::vector<int> descTables_;

		D3D12_STATIC_SAMPLER_DESC linearClampSampler_ =
			CD3DX12_STATIC_SAMPLER_DESC(
				0,
				D3D12_FILTER_ANISOTROPIC,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP
			); // the default sampler used by this demo

		bool addSampler_ = false;

	};

}