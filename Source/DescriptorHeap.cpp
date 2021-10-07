/*
*
* Implimentation of the DescriptorHeap Class (see DescriptorHeap.h)
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

#include "DxPRT/DescriptorHeap.h"


namespace DxPRT_Utility {

    DescriptorHeap::DescriptorHeap() {}


    void DescriptorHeap::Initialize(ID3D12Device* device,
        const D3D12_DESCRIPTOR_HEAP_TYPE &type,
        const UINT &numDescriptors,
        const bool &shaderVisible)
    {
        D3D12_DESCRIPTOR_HEAP_FLAGS flags = (shaderVisible ?
            D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
        D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {
            type, numDescriptors, flags, 0
        };

        ThrowIfFailed(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&descriptorHeap_)));

        CPUStart_ = descriptorHeap_->GetCPUDescriptorHandleForHeapStart();
        GPUStart_ = descriptorHeap_->GetGPUDescriptorHandleForHeapStart();

        incrementSize_ = device->GetDescriptorHandleIncrementSize(type);

        return;
    }

    CD3DX12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetCPUHandle(const int &i) const {
        CD3DX12_CPU_DESCRIPTOR_HANDLE handle(CPUStart_, i, incrementSize_);
        return handle;
    }

    CD3DX12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetGPUHandle(const int &i) const {
        CD3DX12_GPU_DESCRIPTOR_HANDLE handle(GPUStart_, i, incrementSize_);
        return handle;
    }


    ID3D12DescriptorHeap* DescriptorHeap::GetHeap() const {
        return descriptorHeap_.Get();
    }

}