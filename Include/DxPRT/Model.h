/*
*
* Class to contain pointers to the vertex and index data relating to a single model.
* Allows for simple drawing of these objects using the CommandList object (see CommandList.h)
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

#pragma once

#include <wrl.h>
#include <d3d12.h>
#include "d3dx12.h"
#include "DxPRT/Resource.h"


namespace DxPRT_Utility {

	class Model
	{

	public:

		// default initialization
		Model();

		/*
		* AddModelData: stores the relevant index and vertex data 
		* 
		* Must be called before initializing
		* 
		* _IN_ vertices: a pointer to the vertex data
		* _IN_ verticesSizeInBytes: the size, in bytes, of the vertex data
		* _IN_ indices: a pointer to the index data
		* _IN_ indicesNum: the total number of indices in the index data
		* _IN_ strideInBytes: to total amount of data, in bytes, stored for each vertex
		*/

		void AddModelData(void* vertices, const UINT &verticesSizeInBytes,
			UINT32* indices, const UINT &indicesNum, const UINT &strideInBytes);

		/*
		* Initialize: Initializes the main resources and records a command to copy the
		* vertex and index data to the passed command list
		* 
		* Must be called after AddModelData
		* 
		* The commandList used must be executed before any other function is called or
		* this object is used to draw an object
		* 
		* _IN_ device: the currently active device
		* _IN_ commandList: a recordeding command list to record the copy commands
		*/
		void Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);


		// returns the vertex buffer view (must be initialized)
		D3D12_VERTEX_BUFFER_VIEW GetVertexView() const ;
		
		// returns the index buffer view (must be initialized)
		D3D12_INDEX_BUFFER_VIEW GetIndexView() const;

		// return the total number of indices (useful when passing the model to a draw command)
		UINT32 GetNumIndices() const;

		// releases the data held in the vertex and index resources
		void Release();

		// releases the data held in the upload resources used to initalize the vertex and index resources
		void ReleaseUpload();

	private:

		void* vertices_ = nullptr;
		UINT32* indices_ = nullptr;
		UINT verticesSize_ = 0, indicesNum_ = 0, stride_ = 0;
		Resource verticesResource_, indicesResource_;


	};

}