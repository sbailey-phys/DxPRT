/*
*
* This file contains functions and structs that help in simply the code needed to generate
* the spherical harmonic coefficients for an environment map. These are all used in the 
* GenerateEM() function (see GeneratePRT.h).
*
*
* This file is part of the implimentation and is not intended for public
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

#include <string>
#include <Windows.h>
#include <d3d12.h>
#include "DxPRT/GenerateGeneral_Utility.h"
#include "DxPRT/CommandList.h"
#include "DxPRT/CommandQueue.h"
#include "DxPRT/SphericalHarmonics.h"
#include "DxPRT/Resource.h"
#include "DxPRT/DescriptorHeap.h"
#include "DxPRT/PRTWriter.h"
#include "DxPRT/HDRReader.h"

namespace DxPRT {
    struct EM_DESC;
}

namespace DxPRT_Utility {

    // contains the data that is sent to the EMIntegrate shader
    struct EMSETTINGS 
    {
        UINT32 numEventsX;
        UINT32 iCoefficient;
        UINT32 pad1;
        UINT32 pad2;
    };


    // contains the resources used in the generation of the EM coefficients
    struct EMResourceContainer 
    {
        Resource randomRes;
        Resource resultRes;
        Resource readbackRes;
        Resource hdrRes;
        std::vector<Resource> shRes;
    };


    // contains the constants used in the generation of the EM coefficients
    struct EMConstantContainer 
    {
        UINT64 numPixelsX;
        UINT64 numPixelsY;
        UINT64 numEvents;
        UINT64 numEventsX; // = sqrt(numEvents)
        UINT64 shGridNum;
        UINT64 numThreadGroups;
        UINT64 maxL;
        UINT64 nCoefficients;
    };

    /*
    * InitializeEMConstants: initializes the constants used in GenerateEM. These typically come from
    * from the EM_DESC object passed to GenerateEM. Some constants are increased such that they can fit
    * with the implimentation
    * 
    * _IN_ desc: the EM_DESC object passed on initialization
    * _IN_ numPixelsX: width of the hdr image
    * _IN_ numPixelsY: height of the hdr image
    */
    EMConstantContainer InitializeEMConstants(const DxPRT::EM_DESC& desc, const UINT64& numPixelsX,
        const UINT64& numPixelsY);


    /*
    * InitializeEMResource: initializes the resources used in GenerateEM. A command list and command
    * queue are passed so that the data are also copied onto the GPU in this function.
    * 
    * _IN_ device: the currently active device
    * _IN_ commandQueue: the command queue used to copy data
    * _IN_ commandList : the command list used to copy data
    * _OUT_ resources: a container for all the resources to be used
    * _IN_ constants: a container for all the constants used
    * _IN_ data: a poiner to the hdr data
    * _IN_ shData: grids containing the spherical harmonics
    * _IN_ randomVector: a vector containing random integers
    */
    void InitializeEMResources(ID3D12Device* device, CommandQueue& commandQueue, CommandList& commandList,
        EMResourceContainer& resources, const EMConstantContainer& constants,
        const void* data, const std::vector<std::vector<float>>& shData,
        const std::vector<UINT32>& randomVector);

    /*
    * InitializeEMHeap: intializes the shader visible desciptor heap used for the integration
    * 
    * _IN_ device: the currently active device
    * _OUT_ heap: the shader visible heap
    * _IN_ resources: the resources to be referenced in the heap
    * _IN_ constants: the constants used to describe the heap
    */
    void InitializeEMHeap(ID3D12Device* device, DescriptorHeap& heap,
        EMResourceContainer& resources, const EMConstantContainer& constants);


    /*
    * InitializeEMPipeline: initializes the pipeline state object and the root 
    * signature
    * 
    * _IN_ device: the currently active device
    * _OUT_ rootSig: the root signature used for the integration
    * _OUT_ pipeline: the pipeline state object used for the integration
    * _IN_ shaderPath: the path to the folder containing the compiled shaders (.cso)
    */
    void InitalizeEMPipeline(ID3D12Device* device, RootSignature& rootSig,
        ComputePipeline& pipeline, const std::wstring &shaderPath);


    /*
    * ExecuteEMPipeline: builds and executes the command line nessecary to perform the
    * integration on the GPU
    * 
    * _IN_ commandQueue: the queue upon which the command list will be executed
    * _IN_ commandList: the command list to record the commands
    * _IN_ integratePipeline: the pipeline state object containing the required settings
    * _IN_ integrateRootSig: the root signature containing the nessecary parameters
    * _IN_ heap: the descriptor heap referencing the required resources
    * _IN_ constants: the constants needed for the integration
    * _IN_ resources: the resources needed for the integration
    */
    void ExecuteEMPipeline(CommandQueue& commandQueue, CommandList& commandList,
        const ComputePipeline& integratePipeline, const RootSignature& integrateRootSig,
        const DescriptorHeap& heap, const EMConstantContainer& constants,
        const EMResourceContainer& resources);

    /*
    * StoreEMResult: Stores the spherical harmonic coefficients for the environment map
    * 
    * _OUT_ coefficients: a vector containing the resulting coefficients
    * _IN_ resources: contains the resources needed to copy the data from the GPU
    * _IN_ constants: contains constants needed to define the copy
    */
    void StoreEMResult(std::vector<float>& coefficients, const EMResourceContainer& resources,
        const EMConstantContainer& constants);

}