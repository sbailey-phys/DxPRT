/*
*
* Class to simplify the use of the ID3D12DescriptorHeap when generating and rendering the
* PRT spheical harmonic coefficients.
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
#include <vector>
#include "DxPRT/DxPRT_Error.h"


namespace DxPRT_Utility {

    class DescriptorHeap
    {
    public:

        //default initialization
        DescriptorHeap();

        /*
        * Initialize: Initializes the descriptor heap
        * 
        * Must be called before using any other function
        * 
        * _IN_ device: the currently active device
        * _IN_ type: the descriptor heap type desired
        * _IN_ numDescriptors: the total number of descriptors that will be used
        * _IN_ shaderVisible: if the desciptor heap should be visible to the shaders
        */
        void Initialize(ID3D12Device* device,
            const D3D12_DESCRIPTOR_HEAP_TYPE &type,
            const UINT &numDescriptors,
            const bool &shaderVisible = false);

        /*
        * GetCPUHandle/GetGPUHandle: returns the cpu/gpu to the descriptor indexed
        * by i
        * 
        * _IN_ i: descriptor index
        */
        CD3DX12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(const int &i) const;
        CD3DX12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(const int &i) const;

        //returns the underlying ID3D12DescriptorHeap object
        ID3D12DescriptorHeap* GetHeap() const;

    private:

        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap_;

        UINT incrementSize_;
        D3D12_CPU_DESCRIPTOR_HANDLE CPUStart_;
        D3D12_GPU_DESCRIPTOR_HANDLE GPUStart_;
    };

}