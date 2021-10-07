/*
*
* Class to simplify the use of the ID3D12CommandQueue when generating the
* PRT spheical harmonic coefficients. This object also controls the 
* implemtation of the fence and the relevant synchronisation
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
#include "DxPRT/CommandList.h"
#include "DxPRT/DxPRT_Error.h"


namespace DxPRT_Utility {

	class CommandQueue
	{
	public:
		// default initialization
		CommandQueue();


		// calls CommandQueue::Initialize upon initialization 
		CommandQueue(ID3D12Device* device,
			const D3D12_COMMAND_LIST_TYPE& type);


		/*
		* Initialize: Initailizes the command queue and the fence
		* 
		* Must be called befure any other function is used
		* 
		* _IN_ device: the currently active device
		* _IN_ type: the command list type desired for this queue
		*/
		void Initialize(ID3D12Device* device,
			const D3D12_COMMAND_LIST_TYPE &type);

		
		//Signal: Update the fence value on the GPU side
		void Signal();

		
		// WaitForFence: wait for the GPU to finish computing
		void WaitForFence() const;


		// Flush: Ensure that all GPU computations have finished
		void Flush();

		
		/*
		* Execute: execute a singal command list on the command queue
		* 
		* _IN_ list: the command list to be executed. This must be closed first
		*/
		void Execute(const CommandList &list) const;


		// Returns the underlying commandqueue object
		ID3D12CommandQueue* GetCommandQueue() const;


		// CloseFence: closes the fence handle
		void CloseFence();

	private:

		Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_;
		Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
		HANDLE fenceEvent_ = INVALID_HANDLE_VALUE;
		uint64_t fenceValue_ = 0;
	};

}