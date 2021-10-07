/*
*
* Implimentation of the CommandList Class (see CommandList.h)
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


#include "DxPRT/CommandList.h"

namespace DxPRT_Utility {

	CommandList::CommandList() {}


	CommandList::CommandList(ID3D12Device* device,
		const D3D12_COMMAND_LIST_TYPE& type) 
	{
		this->Initialize(device, type);
	}

	void CommandList::Initialize(ID3D12Device* device,
		const D3D12_COMMAND_LIST_TYPE& type) 
	{
		ThrowIfFailed(device->CreateCommandAllocator(type, IID_PPV_ARGS(&commandAllocator_)));
		ThrowIfFailed(device->CreateCommandList(0, type, commandAllocator_.Get(),
			nullptr, IID_PPV_ARGS(&commandList_)));
		return;
	}

	void CommandList::Reset() const 
	{
		ThrowIfFailed(commandAllocator_->Reset());
		ThrowIfFailed(commandList_->Reset(commandAllocator_.Get(), nullptr));
		return;
	}
	void CommandList::Close() const 
	{
		ThrowIfFailed(commandList_->Close());
		return;
	}

	void CommandList::Barrier(ID3D12Resource* pResource,
		const D3D12_RESOURCE_STATES& stateBefore,
		const D3D12_RESOURCE_STATES& stateAfter) const 
	{
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			pResource, stateBefore, stateAfter);
		commandList_->ResourceBarrier(1, &barrier);
		return;
	}

	void CommandList::Clear(const CD3DX12_CPU_DESCRIPTOR_HANDLE &rtvHandle,
		const CD3DX12_CPU_DESCRIPTOR_HANDLE& dsvHandle, float clearColour[4]) const 
	{

		commandList_->ClearDepthStencilView(dsvHandle,
			D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
		commandList_->ClearRenderTargetView(rtvHandle,
			clearColour, 0, nullptr);
		return;
	}

	void CommandList::SetPipeline(const Pipeline &pipeline,
		const RootSignature &rootSig) const 
	{

		commandList_->SetPipelineState(pipeline.GetPipeline().Get());
		commandList_->SetGraphicsRootSignature(rootSig.GetRootSignature().Get());
		commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		return;
	}

	void CommandList::SetPipeline(const ComputePipeline &pipeline,
		const RootSignature &rootSig) const 
	{

		commandList_->SetPipelineState(pipeline.GetPipeline().Get());
		commandList_->SetComputeRootSignature(rootSig.GetRootSignature().Get());
		return;
	}

	void CommandList::SetModel(const Model& model) const 
	{
		D3D12_VERTEX_BUFFER_VIEW vertexDesc = model.GetVertexView();
		D3D12_INDEX_BUFFER_VIEW indexDesc = model.GetIndexView();

		commandList_->IASetVertexBuffers(0, 1, &vertexDesc);
		commandList_->IASetIndexBuffer(&indexDesc);
		return;
	}

	void CommandList::SetConstants(const UINT &index, const UINT &size, const void* data) const 
	{
		commandList_->SetGraphicsRoot32BitConstants(index, size,
			data, 0);
		return;
	}


	void CommandList::SetComputeConstants(const UINT &index, const UINT &size, const void* data) const 
	{
		commandList_->SetComputeRoot32BitConstants(index, size,
			data, 0);
		return;
	}
	
	void CommandList::Draw(const Model &model) const 
	{
		size_t size = model.GetNumIndices();
		commandList_->DrawIndexedInstanced(size, 1, 0, 0, 0);
		return;
	}


	void CommandList::SetRasterizer(const CD3DX12_RECT &rect,
		const CD3DX12_VIEWPORT &viewport) const 
	{
		commandList_->RSSetViewports(1, &viewport);
		commandList_->RSSetScissorRects(1, &rect);
	}

	void CommandList::SetOutputMerger(const CD3DX12_CPU_DESCRIPTOR_HANDLE &rtv,
		const CD3DX12_CPU_DESCRIPTOR_HANDLE &dsv) const 
	{
		commandList_->OMSetRenderTargets(1, &rtv, FALSE, &dsv);
	}


	void CommandList::SetComputeDesciptorTable(const UINT &index,
		const D3D12_GPU_DESCRIPTOR_HANDLE &handle) const 
	{
		commandList_->SetComputeRootDescriptorTable(index,
			handle);
	}

	void CommandList::SetDesciptorTable(const UINT &index,
		const D3D12_GPU_DESCRIPTOR_HANDLE &handle) const 
	{
		commandList_->SetGraphicsRootDescriptorTable(index,
			handle);
	}

	void CommandList::SetDescriptorHeap(const UINT &num, ID3D12DescriptorHeap* heap) const 
	{
		commandList_->SetDescriptorHeaps(num, &heap);
	}

	void CommandList::Dispatch(const UINT &numX,const UINT &numY,const UINT& numZ) const 
	{
		commandList_->Dispatch(numX, numY, numZ);
	}


	void CommandList::CopyResource(const Microsoft::WRL::ComPtr<ID3D12Resource> &destRes,
		const Microsoft::WRL::ComPtr<ID3D12Resource> &sourceRes) const 
	{
		commandList_->CopyResource(destRes.Get(), sourceRes.Get());
	}


	ID3D12GraphicsCommandList* CommandList::GetCommandList() const 
	{
		return commandList_.Get();
	}


}