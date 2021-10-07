/*
*
* Implimentation of the Resource Class (see Resource.h)
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

#include "DxPRT/Resource.h"
#include "DxPRT/CommandList.h" // avoids cyclic inclue

namespace DxPRT_Utility {
    
    Resource::Resource() {}

    void Resource::Initialize(ID3D12Device* device) 
    {
        ThrowIfFailed(device->CreateCommittedResource(
            &properties_,
            flags_,
            &desc_,
            state_,
            nullptr,
            IID_PPV_ARGS(&resource_)
        ));
    }


    void Resource::InitializeWithData(ID3D12Device* device, CommandList list,
        const void* data) 
    {
        this->InitializeWithData(device, list.GetCommandList(), data);
    }


    void Resource::InitializeWithData(ID3D12Device* device, ID3D12GraphicsCommandList* list,
        const void* data) 
    {

        // create main resource
        ThrowIfFailed(device->CreateCommittedResource(
            &properties_,
            flags_,
            &desc_,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&resource_)
        ));

        // create upload resource

        D3D12_HEAP_PROPERTIES heap_up =
            CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        D3D12_RESOURCE_FLAGS flagsUp = D3D12_RESOURCE_FLAG_NONE;
        D3D12_RESOURCE_DESC descUp =
            CD3DX12_RESOURCE_DESC::Buffer(width_ * height_, flagsUp);

        ThrowIfFailed(device->CreateCommittedResource(
            &heap_up,
            flags_,
            &descUp,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&uploadRes_)
        ));

        // copy data

        D3D12_SUBRESOURCE_DATA subresourceData = {
            data, width_, width_ * height_
        };

        UpdateSubresources(list, resource_.Get(),
            uploadRes_.Get(), 0, 0, 1, &subresourceData);

        // set up resource in required state

        //list.Barrier(resource_.Get(), D3D12_RESOURCE_STATE_COPY_DEST, state_);

        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            resource_.Get(), D3D12_RESOURCE_STATE_COPY_DEST, state_);
        list->ResourceBarrier(1, &barrier);

    }

    void Resource::SetProperties(const D3D12_HEAP_TYPE& type)
    {
        properties_ = CD3DX12_HEAP_PROPERTIES(type);
    }


    void Resource::SetFlags(const D3D12_HEAP_FLAGS& flag) 
    {
        flags_ = flag;
    }



    void Resource::SetSimpleBuffer(const UINT64& bufferSize, const D3D12_RESOURCE_FLAGS& flags) 
    {

        desc_ = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, flags);
        width_ = bufferSize;
        height_ = 1;
    }



    void Resource::SetBuffer(const UINT64& numElements, const UINT& elementSize, const DXGI_FORMAT& format,
        const D3D12_RESOURCE_FLAGS& flags) 
    {
        format_ = format;
        width_ = numElements * elementSize;
        height_ = 1;
        elementSize_ = elementSize;

        srvDim_ = D3D12_SRV_DIMENSION_BUFFER;
        uavDim_ = D3D12_UAV_DIMENSION_BUFFER;

        desc_ = CD3DX12_RESOURCE_DESC::Buffer(width_, flags);
    }



    void Resource::SetTex2D(const DXGI_FORMAT& format, const UINT64& width, const UINT& height,
        const UINT64& elementSize,
        const UINT16& arraySize, const UINT16& miplevels,
        const UINT& sampleCount, const UINT& sampleQuality,
        const D3D12_RESOURCE_FLAGS& flags) 
    {

        format_ = format;
        width_ = width * elementSize;
        height_ = height * elementSize;
        elementSize_ = elementSize;

        srvDim_ = D3D12_SRV_DIMENSION_TEXTURE2D;
        uavDim_ = D3D12_UAV_DIMENSION_TEXTURE2D;

        desc_ = CD3DX12_RESOURCE_DESC::Tex2D(format, width, height, arraySize, miplevels,
            sampleCount, sampleQuality, flags);
    }


    void Resource::SetState(const D3D12_RESOURCE_STATES& state) 
    {
        state_ = state;
    }



    void Resource::CreateSRV(ID3D12Device* device, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle,
        const UINT& structuredBytes) const 
    {

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        if (structuredBytes == 0) {
            srvDesc.Format = format_;
        }
        else {
            srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        }

        srvDesc.ViewDimension = srvDim_;
        srvDesc.Shader4ComponentMapping =
            D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        if (srvDim_ == D3D12_SRV_DIMENSION_BUFFER) {
            srvDesc.Buffer.NumElements = width_ / elementSize_;
            srvDesc.Buffer.StructureByteStride = structuredBytes;
        }
        else if (srvDim_ == D3D12_SRV_DIMENSION_TEXTURE2D) {
            srvDesc.Texture2D.MipLevels = 1;
        }

        device->CreateShaderResourceView(
            resource_.Get(),
            &srvDesc,
            cpuHandle
        );

    }

    void Resource::CreateCBV(ID3D12Device* device, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle) const 
    {
        
        D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {
            resource_->GetGPUVirtualAddress(), width_
        };

        device->CreateConstantBufferView(
            &desc, cpuHandle
        );
    }



    void Resource::CreateUAV(ID3D12Device* device, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle) const 
    {

        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.Format = format_;
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
        uavDesc.Buffer.NumElements = width_ / elementSize_;


        device->CreateUnorderedAccessView(
            resource_.Get(),
            nullptr,
            &uavDesc,
            cpuHandle
        );

    }


    void Resource::Release()
    {
        resource_.Reset();
    }

    void Resource::ReleaseUpload()
    {
        uploadRes_.Reset();
    }

    ID3D12Resource* Resource::GetResource() const 
    {
        return resource_.Get();
    }

}




