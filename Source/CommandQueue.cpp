/*
*
* Implimentation of the CommandQueue Class (see CommandQueue.h)
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

#include "DxPRT/CommandQueue.h"

namespace DxPRT_Utility {

	CommandQueue::CommandQueue() {}


	CommandQueue::CommandQueue(ID3D12Device* device,
		const D3D12_COMMAND_LIST_TYPE& type) 
	{
		this->Initialize(device, type);
	}

	void CommandQueue::Initialize(ID3D12Device* device,
		const D3D12_COMMAND_LIST_TYPE &type) 
	{

		D3D12_COMMAND_QUEUE_DESC desc = {};
		desc.Type = type;
		desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		desc.NodeMask = 0;

		ThrowIfFailed(device->CreateCommandQueue(&desc,
			IID_PPV_ARGS(&commandQueue_)));

		ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE,
			IID_PPV_ARGS(&fence_)));

		fenceEvent_ = CreateEvent(NULL, FALSE, FALSE, NULL);

		return;
	}

	void CommandQueue::Signal() 
	{
		++fenceValue_;

		ThrowIfFailed(commandQueue_->Signal(fence_.Get(), fenceValue_));
	}

	void CommandQueue::WaitForFence() const 
	{
		if (fence_->GetCompletedValue() < fenceValue_) {
			ThrowIfFailed(fence_->SetEventOnCompletion(fenceValue_, fenceEvent_));
			ThrowIfFailed(WaitForSingleObject(fenceEvent_, INFINITE));
		}
	}

	void CommandQueue::Flush() 
	{
		this->Signal();
		this->WaitForFence();
	}

	void CommandQueue::Execute(const CommandList &list) const 
	{
		ID3D12CommandList* const commandLists[] = {
				list.GetCommandList()
		};
		commandQueue_->ExecuteCommandLists(1, commandLists);
	}


	ID3D12CommandQueue* CommandQueue::GetCommandQueue() const 
	{
		return commandQueue_.Get();
	}


	void CommandQueue::CloseFence() 
	{
		CloseHandle(fenceEvent_);
	}
}




