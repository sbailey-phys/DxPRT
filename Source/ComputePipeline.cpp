/*
*
* Implimentation of the ComputePipeline Class (see ComputePipeline.h)
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

#include "DxPRT/ComputePipeline.h"


namespace DxPRT_Utility {

	ComputePipeline::ComputePipeline() {
	}


	void ComputePipeline::Initialize(ID3D12Device* device) {

		D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};

		desc.CS = { static_cast<UINT8*>(computeShader_->GetBufferPointer()),
		computeShader_->GetBufferSize() }; // compute shader

		desc.pRootSignature = pRootSig_;

		desc.NodeMask = 0; // default values, no special value is needed for this implimentation
		desc.CachedPSO = { nullptr, 0 };
		desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

		ThrowIfFailed(device->CreateComputePipelineState(&desc, IID_PPV_ARGS(&pipeline_)));

		return;
	}


	void ComputePipeline::SetComputeShader(const LPCWSTR& shader) {
		computeShader_ = this->SetShader(shader);
		return;
	}


	void ComputePipeline::SetRootSignature(const RootSignature& rootSig) {
		pRootSig_ = rootSig.GetRootSignature().Get();
		return;
	}

	Microsoft::WRL::ComPtr<ID3D12PipelineState> ComputePipeline::GetPipeline() const {
		return pipeline_;
	}


	Microsoft::WRL::ComPtr<ID3DBlob> ComputePipeline::SetShader(const LPCWSTR &shader) const
	{
		Microsoft::WRL::ComPtr<ID3DBlob> shaderBlob;
		HRESULT result = D3DReadFileToBlob(shader, &shaderBlob);
		if (result != S_OK) {
			OutputDebugStringA("Unable to open shader file. Ensure that shaderPath is set correctly!\n");
			ThrowIfFailed(result);
		}
		return shaderBlob;
	}

}