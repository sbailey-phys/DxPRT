/*
*
* Class to simplify the use of the ID3D12Resource when generating the
* PRT spheical harmonic coefficients and rendering. This class is also 
* able to create CBVs, SRVs and UAV, using the information input when
* the resource was created. The current implimentation uses committed 
* resources, with little regard to memory management on the GPU.
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
#include "DxPRT/DxPRT_Error.h"

namespace DxPRT_Utility {

	class CommandList; // forward declaration, avoids cyclic includes

	class Resource
	{

	public:

		// default initialization
		Resource();

		/*
		* Initialize: initializes the resource using the properties set beforehand.
		* 
		* The properties can be set using functions such as SetBuffer before the initialization,
		* these cannot be changed once the resource has been initialized
		* 
		* _IN_ device: the currently active device
		*/
		void Initialize(ID3D12Device* device);

		/*
		* InitializeWithData: Initializes the resources and records a command to copy
		* the data. The commandlist used must be in a recording state and should be executed
		* before the resource is used.
		* 
		* The properties of the resource should be set up before the running this function
		* (see Initialize)
		* 
		* _IN_ device: the currently active device
		* _IN_ list: a CommandList or ID3D12GraphicsCommandList* object for the copy command to
		*            be recorded on
		* _IN_ data: a pointer to the data used to initialize the buffer
		*/
		void InitializeWithData(ID3D12Device* device, CommandList list,
			const void* data);
		void InitializeWithData(ID3D12Device* device, ID3D12GraphicsCommandList* list,
			const void* data);

		/*
		* SetProperties: sets the properties of the buffer
		* 
		* _IN_ type: the type of heap that the resource will be stored on
		*/
		void SetProperties(const D3D12_HEAP_TYPE& type);

		/*
		* SetFlags:
		* 
		* _IN_ flag: the heap flag to be set
		*/
		void SetFlags(const D3D12_HEAP_FLAGS& flag);

		/*
		* SetSimpleBuffer: Sets the resource to be 1-dimensional and sets its size.
		* This requires fewer input parameters and is intended for use on non-shader
		* visible heaps.
		* 
		* _IN_ bufferSize: the size, in bytes, of the buffer
		* _IN_ flags: the resource flags required
		*/
		void SetSimpleBuffer(const UINT64& bufferSize,
			const D3D12_RESOURCE_FLAGS& flags = D3D12_RESOURCE_FLAG_NONE);


		/*
		* SetBuffer: Sets the resource to be 1-dimensional and sets its size.
		* This is intended for use on resources that will be placed in shader visible
		* heaps with the relevant SRV/CBV/UBV created later. It thus requires more
		* parameters than nessecary to simply the later stages
		*
		* _IN_ numElements: the total number of elements to be set
		* _IN_ elementSize: the size of each element in bytes
		* _IN_ flags: the resource flags required
		*/
		void SetBuffer(const UINT64& numElements, const UINT& elementSize, const DXGI_FORMAT& format,
			const D3D12_RESOURCE_FLAGS& flags = D3D12_RESOURCE_FLAG_NONE);
		


		/*
		* SetTex2D: Sets the resource to be 2-dimensional and sets its size.
		* This is intended for use on resources that will be placed in shader visible
		* heaps with the relevant SRV/CBV/UBV created later. It thus requires more
		* parameters than nessecary to simply the later stages
		*
		* _IN_ format: the DXGI_FORMAT of the data to be stored in the resource
		* _IN_ width: the width of the texture
		* _IN_ height: the height of the texture
		* _IN_ elementSize, the size of each element in bytes (should be consistent with format)
		* all other inputs should have obvious meaning
		*/
		void SetTex2D(const DXGI_FORMAT& format, const UINT64& width, const UINT& height,
			const UINT64& elementSize,
			const UINT16& arraySize = 1, const UINT16& miplevels = 0,
			const UINT& sampleCount = 1, const UINT& sampleQuality = 0,
			const D3D12_RESOURCE_FLAGS& flags = D3D12_RESOURCE_FLAG_NONE);

		/*
		* SetState: sets the current state of the resource. If initialized with the
		* InitializeWithData function, then this is the state the resource will be
		* left in after the data has been copied onto the GPU
		* 
		* _IN_ state: the desired resource state
		*/
		void SetState(const D3D12_RESOURCE_STATES& state);


		/*
		* CreateSRV: creates a shader resource view for the resource
		* 
		* To use this, the resource should have been created with SetBuffer
		* 
		* _IN_ device: the currently active device
		* _IN_ cpuHandle: a CPU Handle to the relevant descriptor
		* _IN_ structuredBytes: the number of bytes per data if the resource is to be 
		*                       used as a structured buffer
		*/
		void CreateSRV(ID3D12Device* device, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle,
			const UINT& structuredBytes = 0) const;

		/*
		* CreateUAV: creates an unordered access view for the resource
		*
		* To use this, the resource should have been created with SetBuffer
		*
		* _IN_ device: the currently active device
		* _IN_ cpuHandle: a CPU Handle to the relevant descriptor
		*/
		void CreateUAV(ID3D12Device* device, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle) const;


		/*
		* CreateUAV: creates a constant buffer view for the resource
		*
		* To use this, the resource should have been created with SetBuffer
		*
		* _IN_ device: the currently active device
		* _IN_ cpuHandle: a CPU Handle to the relevant descriptor
		*/
		void CreateCBV(ID3D12Device* device, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle) const;


		/*
		* Release: releases the data held on the resource, note that this does not clear the 
		* resource used for upload
		*/
		void Release();

		/*
		* releases the data on the resource used to upload data to the GPU. Should be called after
		* the command list used with InitializeWithData has executed
		*/
		void ReleaseUpload();

		// returns the underlying ID3D12Resource
		ID3D12Resource* GetResource() const;

	private:

		Microsoft::WRL::ComPtr<ID3D12Resource> resource_;
		Microsoft::WRL::ComPtr<ID3D12Resource> uploadRes_;

		// default choices
		D3D12_HEAP_PROPERTIES properties_ =
			CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		D3D12_HEAP_FLAGS flags_ = D3D12_HEAP_FLAG_NONE;
		D3D12_RESOURCE_DESC desc_ =
			CD3DX12_RESOURCE_DESC::Buffer(0); // zero size buffer
		D3D12_RESOURCE_STATES state_ = D3D12_RESOURCE_STATE_GENERIC_READ;
		D3D12_CLEAR_VALUE* clearValue_ = nullptr;
		LONG_PTR width_ = 0, height_ = 0;
		UINT elementSize_ = 0;


		DXGI_FORMAT format_ = DXGI_FORMAT_UNKNOWN;
		D3D12_SRV_DIMENSION srvDim_ = D3D12_SRV_DIMENSION_UNKNOWN;
		D3D12_UAV_DIMENSION uavDim_ = D3D12_UAV_DIMENSION_UNKNOWN;



	};

}