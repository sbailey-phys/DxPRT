/*
*
* Class to simplify the use of the ID3D12GraphicsCommandList when generating the
* PRT spheical harmonic coefficients. As not many command lists are used in this
* calculation, each command list has a dedicated command allocator for simplicity. 
*
* This class is part of the implimentation and is not intended for public
* use.
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
#include "DxPRT/RootSignature.h"
#include "DxPRT/Pipeline.h"
#include "DxPRT/ComputePipeline.h"
#include "DxPRT/Model.h"
#include "DxPRT/DxPRT_Error.h"

namespace DxPRT_Utility {

	class CommandList
	{
	public:

		
		//Default initalization
		CommandList();

		
		//Calls CommandList::Initalize upon construction
		CommandList(ID3D12Device* device,
			const D3D12_COMMAND_LIST_TYPE& type);


		/*
		* Initialize: Initalizes both the command list and the command allocator.
		*
		* Must be called before any other function is used.
		* 
		* _IN_ device: the curretly active device
		* _IN_ type: the command list type (direct/compute/copt/bundle) that is desired
		*/
		void Initialize(ID3D12Device* device,
			const D3D12_COMMAND_LIST_TYPE &type);


		// Reset: Resets the command List and the command allocator such that they are ready to record.
		void Reset() const;


		// Close: Cloes the command list such that it can be executed
		void Close() const;


		/*
		* Barrier: Creates a barrier for a resource
		* 
		* _IN_ pResource: poiter to the resource for which the barrier is needed
		* _IN_ stateBefore: the state of the resource before the barrier
		* _IN_ stateAfter: the state of the resource after the barrier
		*/
		void Barrier(ID3D12Resource* pResource,
			const D3D12_RESOURCE_STATES& stateBefore,
			const D3D12_RESOURCE_STATES& stateAfter) const;


		/*
		* Clear: Clears both the render tartget view and the depth stencil view. The depth is
		* cleared to a value of 1.0f
		* 
		* _IN_ rtvHandle: A CPU handle for the render target view
		* _IN_ dsvHandle: A CPU handle for the depth stencil view
		* _IN_ clearColor: An array of four floats containing the color to reset the 
		*                   render target view to.
		*/
		void Clear(const CD3DX12_CPU_DESCRIPTOR_HANDLE &rtvHandle,
			const CD3DX12_CPU_DESCRIPTOR_HANDLE &dsvHandle, float clearColor[4]) const;


		/*
		* SetPipeline: Sets both the pipeline and the root signature. For a graphics pipeline, the 
		* primative topology is set to a triangle list.
		* 
		* _IN_ pipeline: Pipeline ot ComputePipeline object describing the pipeline state to be set
		* _IN_ rootSig: RootSignature object descripting the root signature to be set 
		*/
		void SetPipeline(const Pipeline &pipeline, const RootSignature &rootSig) const;
		void SetPipeline(const ComputePipeline &pipeline, const RootSignature &rootSig) const;


		/*
		* SetModel: set the vertex buffer and index buffer views
		* 
		* _IN_ model: the Model object containing the relevant vertex and index data
		*/
		void SetModel(const Model &model) const;


		/*
		* SetContants: Sets constants to the root signature.
		* 
		* _IN_ index: index in the root signature parameters
		* _IN_ size: the size, in bytes, of the constants set
		* _IN_ data: pointer to the constants that will be set
		*/
		void SetConstants(const UINT &index, const UINT &size, const void* data) const;
		void SetComputeConstants(const UINT &index, const UINT &size, const void* data) const;


		/*
		* Draw: Draws the model set to the command list using the bound pipeline and root signature
		* 
		* _IN_: model: Model object to be drawn. This should be the same as that called in the 
		*              SetModel function.
		*/
		void Draw(const Model &model) const;


		/*
		* SetRasterizer: Sets the relevant objects needed for the rasterizer stage
		* 
		* _IN_ rect: CD3DX12_RECT object to be set
		* _IN_ viewport: CD3DX12_VIERPORT object to be set
		*/
		void SetRasterizer(const CD3DX12_RECT &rect,
			const CD3DX12_VIEWPORT &viewport) const;

		
		/*
		* SetOutputMerger: Sets the relevant objects for the output merger stage
		* 
		* _IN_ rtvHandle: A CPU handle for the render target view
		* _IN_ dsvHandle: A CPU handle for the depth stencil view
		*/
		void SetOutputMerger(const CD3DX12_CPU_DESCRIPTOR_HANDLE &rtv,
			const CD3DX12_CPU_DESCRIPTOR_HANDLE &dsv) const;


		/*
		* SetComputeDesciptorTable/SetDesciptorTable: Sets a desciptor table to 
		* the root signature. The former should be used if the ComputePipeline
		* object is used, the latter used if the Pipeline object is used.
		* 
		* _IN_ index: index in the root signature object
		* _IN_ handle: GPU handle to the descriptor table
		*/
		void SetComputeDesciptorTable(const UINT &index,
			const D3D12_GPU_DESCRIPTOR_HANDLE &handle) const;
		void SetDesciptorTable(const UINT &index,
			const D3D12_GPU_DESCRIPTOR_HANDLE &handle) const;

		/*
		* SetDescriptorHeap: Sets the descriptor heap. Only currently implimented for 
		* a SRV_CBV_UAV heap, with a sampler heap going to be implimented in the future
		* 
		* _IN_ num: not currently implemented, should be set to 1
		* _IN_ heap: pointer to the descriptor heap
		*/
		void SetDescriptorHeap(const UINT &num, ID3D12DescriptorHeap* heap) const;

		/* 
		* Dispatch: Dipatches the currently set compute pipeline.
		* 
		* _IN_ numX: number of thread groups in the x-direction
		* _IN_ numY: number of thread groups in the y-direction
		* _IN_ numZ: number of thread groups in the z-direction
		*/

		void Dispatch(const UINT &numX, const UINT &numY, const UINT &numZ) const;

		/*
		* Copy Resource: copy resource data between two 1-dimension buffers
		* 
		* _IN_ descRes: the destination resource
		* _IN_ sourceRes: the source resource
		*/
		void CopyResource(const Microsoft::WRL::ComPtr<ID3D12Resource> &destRes,
			const Microsoft::WRL::ComPtr<ID3D12Resource> &sourceRes) const;

		/*
		* GetCommandList: Returns the commandList_ object used in the CommandList object
		*/
		ID3D12GraphicsCommandList* GetCommandList() const;

	private:

		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_;

	};
}
