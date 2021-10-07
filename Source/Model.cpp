/*
*
* Implimentation of the Model Class (see Model.h)
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

#include "DxPRT/Model.h"


namespace DxPRT_Utility {

	Model::Model() {}

	void Model::AddModelData(void* vertices, const UINT &verticesSizeInBytes,
		UINT32* indices, const UINT &indicesNum, const UINT &strideInBytes)
	{
		vertices_ = vertices;
		indices_ = indices;
		verticesSize_ = verticesSizeInBytes;
		indicesNum_ = indicesNum;
		stride_ = strideInBytes;
	}


	void Model::Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
	{
		verticesResource_.SetSimpleBuffer(verticesSize_);
		verticesResource_.SetState(D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

		indicesResource_.SetSimpleBuffer(indicesNum_ * 4u);
		indicesResource_.SetState(D3D12_RESOURCE_STATE_INDEX_BUFFER);

		verticesResource_.InitializeWithData(device, commandList, vertices_);
		indicesResource_.InitializeWithData(device, commandList, indices_);
	}

	D3D12_VERTEX_BUFFER_VIEW Model::GetVertexView() const
	{

		D3D12_VERTEX_BUFFER_VIEW vertexBufferViewDesc = {};
		vertexBufferViewDesc = {
			verticesResource_.GetResource()->GetGPUVirtualAddress(),
			verticesSize_,
			stride_
		};
		return vertexBufferViewDesc;
	}

	D3D12_INDEX_BUFFER_VIEW Model::GetIndexView() const
	{

		D3D12_INDEX_BUFFER_VIEW indexBufferViewDesc = {};
		indexBufferViewDesc = {
			indicesResource_.GetResource()->GetGPUVirtualAddress(),
			indicesNum_ * 4,
			DXGI_FORMAT_R32_UINT
		};
		return indexBufferViewDesc;
	}


	UINT32 Model::GetNumIndices() const
	{
		return indicesNum_;
	}


	void Model::Release()
	{
		verticesResource_.Release();
		indicesResource_.Release();
	}

	void Model::ReleaseUpload()
	{
		verticesResource_.ReleaseUpload();
		indicesResource_.ReleaseUpload();
	}

}