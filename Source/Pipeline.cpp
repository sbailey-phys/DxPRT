/*
*
* Implimentation of the Pipeline Class (see Pipeline.h)
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

#include "DxPRT/Pipeline.h"


namespace DxPRT_Utility {

	Pipeline::Pipeline() 
	{
		RTVFormats_[0] = DXGI_FORMAT_R8G8B8A8_UNORM;// default value
	}

	void Pipeline::Initialize(ID3D12Device* device) 
	{

		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};

		desc.InputLayout = { &inputLayout_[0], (UINT)inputLayout_.size() };

		desc.VS = { static_cast<UINT8*>(vertexShader_->GetBufferPointer()),
		vertexShader_->GetBufferSize() };
		desc.PS = { static_cast<UINT8*>(pixelShader_->GetBufferPointer()),
		pixelShader_->GetBufferSize() };

		desc.pRootSignature = pRootSig_;
		desc.PrimitiveTopologyType = topologyType_;
		desc.DSVFormat = DSVFormat_;
		for (int i = 0; i < 8; ++i) {
			desc.RTVFormats[i] = RTVFormats_[i];
		}
		desc.NumRenderTargets = numRTVs_;

		desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT); // set to default values
		desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		desc.DepthStencilState.DepthEnable = TRUE;
		desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		desc.DepthStencilState.StencilEnable = FALSE;
		desc.SampleMask = UINT_MAX;
		desc.SampleDesc.Count = 1;

		ThrowIfFailed(device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&pipeline_)));

		return;
	}

	void Pipeline::AddInput(const LPCSTR &semanticName, const UINT &semanticIndex,
		const DXGI_FORMAT &format) 
	{
		D3D12_INPUT_ELEMENT_DESC element = {
			semanticName, semanticIndex, format, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		};
		inputLayout_.push_back(element);
		return;
	}

	void Pipeline::SetVertexShader(const LPCWSTR& shader)
	{
		vertexShader_ = this->SetShader(shader);
		return;
	}

	void Pipeline::SetPixelShader(const LPCWSTR& shader)
	{
		pixelShader_ = this->SetShader(shader);
		return;
	}

	void Pipeline::SetRootSignature(const RootSignature& rootSig)
	{
		pRootSig_ = rootSig.GetRootSignature().Get();
		return;
	}

	void Pipeline::SetPrimativeTopologyType(const D3D12_PRIMITIVE_TOPOLOGY_TYPE& type)
	{
		topologyType_ = type;
		return;
	}


	void Pipeline::SetDSVFormat(const DXGI_FORMAT& format)
	{
		DSVFormat_ = format;
		return;
	}
	void Pipeline::SetRTVFormat(const UINT &i, const DXGI_FORMAT& format)
	{
		RTVFormats_[i] = format;
		return;
	}

	void Pipeline::SetNumRTV(const UINT &i)
	{
		numRTVs_ = i;
		return;
	}

	Microsoft::WRL::ComPtr<ID3D12PipelineState> Pipeline::GetPipeline() const
	{
		return pipeline_;
	}


	Microsoft::WRL::ComPtr<ID3DBlob> Pipeline::SetShader(const LPCWSTR &shader) const
	{
		Microsoft::WRL::ComPtr<ID3DBlob> shaderBlob;
		ThrowIfFailed(D3DReadFileToBlob(shader, &shaderBlob));
		return shaderBlob;
	}

}