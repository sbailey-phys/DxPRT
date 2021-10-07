/*
*
* Implimentation of the RootSignature Class (see RootSignature.h)
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

#include "DxPRT/RootSignature.h"


namespace DxPRT_Utility {

	RootSignature::RootSignature() {}

	void RootSignature::Initialize(ID3D12Device* device, const bool &addSampler) 
	{

		int iRange = 0; // store parameters
		for (auto iter = descTables_.cbegin(); iter != descTables_.cend();
			++iter) {
			parameters_[*iter].InitAsDescriptorTable(1, &descRanges_[iRange]);
			++iRange;
		}

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC description;

		if (!addSampler) // no linear sampler
		{
			description = { (UINT)parameters_.size(),
				&parameters_[0], 0, nullptr, flags_ };
		}
		else  // use linear sampler
		{
			description = { (UINT)parameters_.size(),
				&parameters_[0], 1, &linearClampSampler_, flags_ };
		}

		// check feature level
		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
		if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
		{
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}

		// serialize
		Microsoft::WRL::ComPtr<ID3DBlob> rootSignatureBlob;
		Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
		ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&description,
			featureData.HighestVersion, &rootSignatureBlob, &errorBlob));

		ThrowIfFailed(device->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(),
			rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature_)));

		return;
	}

	void RootSignature::SetFlags(const D3D12_ROOT_SIGNATURE_FLAGS& flags) 
	{
		flags_ = flags;
		return;
	}
	void RootSignature::AddConstants(const UINT& size, const UINT& shaderRegister,
		const D3D12_SHADER_VISIBILITY& visibility) 
	{

		CD3DX12_ROOT_PARAMETER1 parameter;
		parameter.InitAsConstants(size, shaderRegister, 0, visibility);
		parameters_.push_back(parameter);
		return;

	}

	void RootSignature::AddDesciptorTable(const D3D12_DESCRIPTOR_RANGE_TYPE &type,
		const UINT &num, const UINT &shaderRegister,
		const D3D12_SHADER_VISIBILITY& shaderVisibility)
	{
		CD3DX12_DESCRIPTOR_RANGE1 descRange;
		descRange.Init(type, num, shaderRegister);
		descRanges_.push_back(descRange);

		CD3DX12_ROOT_PARAMETER1 parameter;
		// pointer to vector can change as it grows, reset pointers once vector is finished
		parameter.InitAsDescriptorTable(1, &descRanges_[descRanges_.size()], shaderVisibility);
		parameters_.push_back(parameter);

		descTables_.push_back(parameters_.size() - 1);

		addSampler_ = true;

	}

	Microsoft::WRL::ComPtr<ID3D12RootSignature> RootSignature::GetRootSignature() const {
		return rootSignature_;
	}

}