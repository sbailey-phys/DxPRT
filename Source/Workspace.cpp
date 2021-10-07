/*
*
* Implimentation of the Workspace Class (see Workspace.h)
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

#include "DxPRT/Workspace.h"
#include "DxPRT/Skybox.h"



using namespace DxPRT_Utility;

namespace DxPRT {


	Workspace::Workspace(int numEM) : numEM_(numEM) {
		hdrRes_.resize(numEM);
		hdrData_.resize(numEM);
		emRes_.resize(numEM);
		emData_.resize(numEM);
		emMaxL_.resize(numEM);
	}

	void Workspace::AddEM(ID3D12Device* device, ID3D12GraphicsCommandList* commandList,
		const std::string& emFile, const std::string& hdrFile, int iEM) {

		if (isInitialized_) {
			OutputDebugStringA("DxPRT: Cannont add environment map when workspace is initialized!\n");
			return;
		}

		HDRReader hdr(hdrFile); // generate on thread, then copy
		PRTReader em(emFile, true);

		hdrDataMutex_.lock();
		hdrData_[iEM] = hdr; // should use move instead of copy
		hdrDataMutex_.unlock();

		emDataMutex_.lock();
		emData_[iEM] = em; // should use move instead of copy
		emDataMutex_.unlock();

		emResMutex_.lock();
		emRes_[iEM].SetBuffer(emData_[iEM].GetSizeCoefficients(), 4, DXGI_FORMAT_R32_FLOAT);
		emRes_[iEM].SetState(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		emRes_[iEM].InitializeWithData(device, commandList, emData_[iEM].GetCoefficients());
		emResMutex_.unlock();

		hdrResMutex_.lock();
		hdrRes_[iEM].SetTex2D(DXGI_FORMAT_R32G32B32_FLOAT, hdrData_[iEM].GetNPixelsX(),
			hdrData_[iEM].GetNPixelsY(), 12);
		hdrRes_[iEM].SetState(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		hdrRes_[iEM].InitializeWithData(device, commandList, hdrData_[iEM].GetData());
		hdrResMutex_.unlock();

		emMaxLMutex_.lock();
		emMaxL_[iEM] = emData_[iEM].GetNCoefficients();
		emMaxLMutex_.unlock();

	}


	void Workspace::AddPRT(ID3D12Device* device, ID3D12GraphicsCommandList* commandList,
		const std::string& prtFile) {

		if (isInitialized_) {
			OutputDebugStringA("DxPRT: Cannont add mesh when workspace is initialized!\n");
			return;
		}

		prtData_.Load(prtFile);

		// remove any added prt
		prtRes_.Release();
		prtRes_.ReleaseUpload();

		prtRes_.SetBuffer(prtData_.GetSizeCoefficients(), 4, DXGI_FORMAT_R32_FLOAT);
		prtRes_.SetState(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		prtRes_.InitializeWithData(device, commandList, prtData_.GetCoefficients());


		prtModel_.AddModelData(prtData_.GetNumberedVertices(), prtData_.GetSizeNumberedVertices() * 16,
			prtData_.GetIndices(), prtData_.GetSizeIndices(), 16);
		prtModel_.Initialize(device, commandList);

		prtMaxL_ = prtData_.GetMaxL();


		if (!skyboxModelInitalized_) {
			skyboxModelInitalized_ = true;
			skyboxModel_.AddModelData(SKYBOX::skyboxVertices,
				8 * 3 * sizeof(float), SKYBOX::skyboxIndices, 12 * 3, 12);

			skyboxModel_.Initialize(device, commandList);
		}

	}

	void Workspace::SetView(const DirectX::XMMATRIX& view) {
		viewMatrix_ = view;
	}
	void Workspace::SetProjection(const DirectX::XMMATRIX& projection) {
		projectionMatrix_ = projection;
	}


	void Workspace::SetModelMatrix(const float& x, const float& y,
		const float& z, const float& scale) {
		DirectX::XMMATRIX scaleMatrix = DirectX::XMMatrixSet(
			scale, 0.0f, 0.0f, 0.0f,
			0.0f, scale, 0.0f, 0.0f,
			0.0f, 0.0f, scale, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		);
		modelMatrix_ = DirectX::XMMatrixMultiply(DirectX::XMMatrixTranslation(
			x, y, z), scaleMatrix);

	}


	void Workspace::SetRTVHandle(const D3D12_CPU_DESCRIPTOR_HANDLE& rtvHandle) {
		rtvHandle_ = rtvHandle;
	}


	void Workspace::SetDSVHandle(const D3D12_CPU_DESCRIPTOR_HANDLE& dsvHandle) {
		dsvHandle_ = dsvHandle;
	}

	void Workspace::SetRect(const CD3DX12_RECT& rect) {
		rect_ = rect;
	}


	void Workspace::SetViewport(const CD3DX12_VIEWPORT& viewport) {
		viewport_ = viewport;
	}


	void Workspace::SetCurrentEM(const UINT& iEM) {
		iEM_ = iEM % numEM_;
	}

	void Workspace::SetExposure(const float& exposure) {
		exposure_ = exposure;
	}

	void Workspace::SetMaxL(const UINT& maxL) {
		maxL_ = maxL;
	}

	void Workspace::Initalize(ID3D12Device* device, const std::wstring &shaderPath) {

		if (isInitialized_) {
			OutputDebugStringA("DxPRT: Workspace is already initialized!\n");
			return;
		}

		this->InitializeHeaps(device);
		this->InitializePRTPipeline(device, shaderPath);
		this->InitializeSkyboxPipeline(device, shaderPath);
		isInitialized_ = true;
	}

	void Workspace::CleanUpCPU()
	{

		prtRes_.ReleaseUpload();
		for (auto iter = emRes_.begin(); iter != emRes_.end(); ++iter) {
			(*iter).ReleaseUpload();
		}
		for (auto iter = hdrRes_.begin(); iter != hdrRes_.end(); ++iter) {
			(*iter).ReleaseUpload();
		}
		hdrData_.clear();
		emData_.clear();

		prtModel_.ReleaseUpload();
		skyboxModel_.ReleaseUpload();
	}

	void Workspace::Render(ID3D12GraphicsCommandList* commandList) const {


		if (!isInitialized_) {
			OutputDebugStringA("DxPRT: Cannont render until the workspace is initialized!\n");
			return;
		}

		commandList->RSSetViewports(1, &viewport_);
		commandList->RSSetScissorRects(1, &rect_);
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->OMSetRenderTargets(1, &rtvHandle_, FALSE, &dsvHandle_);

		this->RenderPRTObject(commandList);
		this->RenderSkybox(commandList);

	}


	void Workspace::InitializeHeaps(ID3D12Device* device)
	{

		prtHeap_.Initialize(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1 + emRes_.size(), true);
		skyboxHeap_.Initialize(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, hdrRes_.size(), true);


		prtRes_.CreateSRV(device, prtHeap_.GetCPUHandle(0), 4);
		int i = 0;
		auto iterhdr = hdrRes_.cbegin();
		for (auto iter = emRes_.cbegin(); iter != emRes_.cend(); ++iter) {
			(*iter).CreateSRV(device, prtHeap_.GetCPUHandle(1 + i), 4);
			(*iterhdr).CreateSRV(device, skyboxHeap_.GetCPUHandle(i));
			++i;
			++iterhdr;
		}
	}


	void Workspace::InitializePRTPipeline(ID3D12Device* device, const std::wstring &shaderPath) 
	{


		prtRootSig_.AddConstants(20, 0);
		prtRootSig_.AddDesciptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0,
			D3D12_SHADER_VISIBILITY_VERTEX);
		prtRootSig_.AddDesciptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1,
			D3D12_SHADER_VISIBILITY_VERTEX);


		prtRootSig_.Initialize(device);

		prtPipeline_.AddInput("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT);
		prtPipeline_.AddInput("INDEX", 0, DXGI_FORMAT_R32_UINT);

		prtPipeline_.SetRootSignature(prtRootSig_);

		prtPipeline_.SetPixelShader((shaderPath + L"/prtPixelShader.cso").c_str());
		prtPipeline_.SetVertexShader((shaderPath + L"/prtVertexShader.cso").c_str());

		prtPipeline_.Initialize(device);


	}


	void Workspace::InitializeSkyboxPipeline(ID3D12Device* device, const std::wstring& shaderPath)
	{

		skyboxRootSig_.AddConstants(17, 0);
		skyboxRootSig_.AddDesciptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0,
			D3D12_SHADER_VISIBILITY_PIXEL);
		skyboxRootSig_.Initialize(device, true);

		skyboxPipeline_.AddInput("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT);
		skyboxPipeline_.SetPixelShader((shaderPath + L"/skyboxPixelShader.cso").c_str());
		skyboxPipeline_.SetVertexShader((shaderPath + L"/skyboxVertexShader.cso").c_str());
		skyboxPipeline_.SetRootSignature(skyboxRootSig_);
		skyboxPipeline_.Initialize(device);

	}

	void Workspace::RenderPRTObject(ID3D12GraphicsCommandList* commandList) const
	{

		DirectX::XMMATRIX viewProjection = DirectX::XMMatrixMultiply(viewMatrix_,
			projectionMatrix_);


		PRTInData prtInData; // root constants
		prtInData.matrix = DirectX::XMMatrixMultiply(modelMatrix_, viewProjection);
		prtInData.exposure = exposure_;
		prtInData.setMaxL = (std::min)((std::min)(maxL_, prtMaxL_), emMaxL_[iEM_]);
		prtInData.emMaxL = emMaxL_[iEM_];
		prtInData.prtMaxL = prtMaxL_;

		commandList->SetPipelineState(prtPipeline_.GetPipeline().Get());
		commandList->SetGraphicsRootSignature(prtRootSig_.GetRootSignature().Get());

		ID3D12DescriptorHeap* pPrtHeap = prtHeap_.GetHeap();
		commandList->SetDescriptorHeaps(1, &pPrtHeap);
		commandList->SetGraphicsRoot32BitConstants(0, 20, &prtInData, 0);
		commandList->SetGraphicsRootDescriptorTable(1, prtHeap_.GetGPUHandle(0));
		commandList->SetGraphicsRootDescriptorTable(2, prtHeap_.GetGPUHandle(iEM_ + 1));

		D3D12_VERTEX_BUFFER_VIEW vertexDesc = prtModel_.GetVertexView();
		D3D12_INDEX_BUFFER_VIEW indexDesc = prtModel_.GetIndexView();

		commandList->IASetVertexBuffers(0, 1, &vertexDesc);
		commandList->IASetIndexBuffer(&indexDesc);

		commandList->DrawIndexedInstanced(prtModel_.GetNumIndices(), 1,
			0, 0, 0);

	}

	void Workspace::RenderSkybox(ID3D12GraphicsCommandList* commandList) const
	{


		DirectX::XMMATRIX skyboxViewMatrix = DirectX::XMMatrixMultiply(SKYBOX::project3x3, viewMatrix_)
			+ SKYBOX::element4x4; // removes the translation components from the view matrix

		SkyboxInData skyboxInData;

		skyboxInData.matrix = DirectX::XMMatrixMultiply(skyboxViewMatrix, projectionMatrix_);
		skyboxInData.exposure = exposure_; // root constants

		commandList->SetPipelineState(skyboxPipeline_.GetPipeline().Get());
		commandList->SetGraphicsRootSignature(skyboxRootSig_.GetRootSignature().Get());

		ID3D12DescriptorHeap* pSkyboxHeap = skyboxHeap_.GetHeap();
		commandList->SetDescriptorHeaps(1, &pSkyboxHeap);
		commandList->SetGraphicsRoot32BitConstants(0, 17, &skyboxInData, 0);
		commandList->SetGraphicsRootDescriptorTable(1, skyboxHeap_.GetGPUHandle(iEM_));

		D3D12_VERTEX_BUFFER_VIEW vertexDesc = skyboxModel_.GetVertexView();
		D3D12_INDEX_BUFFER_VIEW indexDesc = skyboxModel_.GetIndexView();

		commandList->IASetVertexBuffers(0, 1, &vertexDesc);
		commandList->IASetIndexBuffer(&indexDesc);

		commandList->DrawIndexedInstanced(36, 1, 0, 0, 0);

	}
}

