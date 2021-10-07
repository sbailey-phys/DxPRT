/*
*
* This file contains functions and structs that help in simply the code needed to generate
* the spherical harmonic coefficients for the transfer function. These are all used in the
* GeneratePRT() function (see GeneratePRT.h).
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
* Last Updated: 05/10/2021
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
#include <DirectXMath.h>
#include "DxPRT/CommandList.h"
#include "DxPRT/CommandQueue.h"
#include "DxPRT/SphericalHarmonics.h"
#include "DxPRT/Resource.h"
#include "DxPRT/DescriptorHeap.h"
#include "DxPRT/PRTWriter.h"
#include "DxPRT/ObjReader.h"
#include "DxPRT/HDRReader.h"

namespace DxPRT {
    struct PRT_DESC;
}

namespace DxPRT_Utility {

    // used to hold settings used directly as root constants
    struct RaySettings {
        UINT32 numEventsX;
        UINT32 numPlaneChunks;
        UINT32 numPlanes;
        UINT32 iSH;
    };


    // used to hold information about the direction of the vertex normal and used
    // directly as root constants
    struct RayData {
        DirectX::XMFLOAT4 rayPos;
        DirectX::XMFLOAT4 forward;
        DirectX::XMFLOAT4 xDir;
        RaySettings settings;
    };


    // contains the constants used in the generation of the PRT coefficients
    struct PRTConstantContainer {
        UINT64 numEvents;
        UINT64 numEventsX;
        UINT64 shGridNum;
        UINT64 numThreadGroups;
        UINT64 maxL;
        UINT64 nCoefficients;
        UINT64 triangleNum;
        UINT64 vertexNum;
    };


    // contains the resources used in the generation of the PRT coefficients
    struct PRTResourceContainer {
        Resource randomRes;
        Resource planeRes;
        Resource resultRes;
        Resource readbackRes;
        Resource indexRes;
        Resource vertexRes;
        Resource visibilityRes;
        std::vector<Resource> shRes;
    };

    // constants the pipeline state objects and root signatures used in the generation of the PRT coefficients
    struct PRTPipelineContainer {
        RootSignature rayTracerPrePassRootSig;
        ComputePipeline rayTracerPrePassPipeline;
        RootSignature rayTracerRootSig;
        ComputePipeline rayTracerPipeline;
        RootSignature integrateRootSig;
        ComputePipeline integratePipeline;
    };

    // contins the descriptor heaps used in the generation of the PRT coefficients
    struct PRTHeapContainer {
        DescriptorHeap planeHeap;
        DescriptorHeap rayHeap;
        DescriptorHeap integrateHeap;
    };


    // contains the data and pointers to data used in the generation of the PRT coefficients
    struct PRTDataContainer {
        float* pVertexData;
        UINT32* pIndexData;
        float* pNormalData;
        std::vector<std::vector<float>> shData;
        std::vector<UINT32> randomData;
    };


    /*
    * InitializePRTConstants: initializes the constants used in GeneratePRT. These typically come from
    * from the PRT_DESC object passed to GeneratePRT. Some constants are increased such that they can fit
    * with the implimentation
    *
    * _IN_ desc: the PRT_DESC object passed on initialization
    * _IN_ triangleNum: the number of triangles in the mesh
    * _IN_ vertexNum: number of vertices in the mesh
    */
    PRTConstantContainer InitializePRTConstants(const DxPRT::PRT_DESC& desc,
        const UINT64& triangleNum, const UINT64& vertexNum);

    /*
    * InitializePRTDataContainer: initializes the spherical harmonic grids and random number vector. Also
    * store the pointers to data passed to the GeneratePRT 
    * 
    * _OUT_ dataContainer: container for the data and pointers
    * _IN_ constants: the parameters needed for data generation
    * _IN_ vertexData: pointer to the vertex data
    * _IN_ indexData: pointer to the index data
    * _IN_ normalData: pointer to the normal data
    * _IN_ numVertex: the number of vertices in the mesh
    * _IN_ triangleNum: the number of triangles in the mesh
    */
    void InitalizePRTDataContainer(PRTDataContainer& dataContainer,
        const PRTConstantContainer& constants, float* vertexData,
        UINT32* indexData, float* normalData, const UINT64& numVertex,
        const UINT64& triangleNum);


    /*
    * InitializeEMResource: initializes the resources used in GeneratePRT. A command list and command
    * queue are passed so that the data are also copied onto the GPU in this function.
    *
    * _IN_ device: the currently active device
    * _IN_ commandQueue: the command queue used to copy data
    * _IN_ commandList : the command list used to copy data
    * _OUT_ resources: a container for all the resources to be used
    * _IN_ constants: a container for all the constants used
    * _IN_ dataContainer: pointers to the data to be transfered to the GPU
    */
    void InitializePRTResources(ID3D12Device* device, CommandQueue& commandQueue,
        CommandList& commandList, PRTResourceContainer& resources,
        const PRTConstantContainer& constants, const PRTDataContainer& dataContainer);

    /*
    * CleanUpPRT: releases all possible data from the cpu as these have now been uploaded to the 
    * GPU. Cannot clean up data passed to GeneratePRT, such as the vertex data
    * 
    * _IN/OUT_ data: contains the data to be clear
    * _IN/OUT_ resources: contains the resources, for which their upload resource needs clearing
    * _IN_ constants: contains the nCoefficients constant, needed for the clean up
    */
    void CleanUpPRT(PRTDataContainer& data, PRTResourceContainer& resources,
        const PRTConstantContainer& constants);

    /*
    * InitializePRTHeap: intializes the shader visible desciptor heaps used for the ray tracing and
    * integration
    *
    * _IN_ device: the currently active device
    * _OUT_ heaps: the shader visible heaps
    * _IN_ resources: the resources to be referenced in the heaps
    * _IN_ constants: the constants used to describe the heaps
    */
    void InitializePRTHeaps(ID3D12Device* device, PRTHeapContainer& heaps,
        PRTResourceContainer& resources, const PRTConstantContainer& constants);


    /*
    * InitializePRTPipeline: initializes the pipeline state objects and the root
    * signatures
    *
    * _IN_ device: the currently active device
    * _OUT_ pipelines: contains the pipeline state objects and root signatures
    * _IN_ shaderPath: the path to the folder containing the compiled shaders (.cso)
    */
    void InitializePRTPipelines(ID3D12Device* device, PRTPipelineContainer& pipelines, const std::wstring &shaderPath);

    /*
    * PopulateRayTracer: populates the command list with tasks required by the ray tracer (including the pre-pass)
    * 
    * _IN_ commandList: the command list to record on
    * _IN_ pipelines: the pipeline state objects and root signatures needed for the ray tracer
    * _IN_ heaps: the descriptor heaps needed for the ray tracer
    * _IN_ constants: the parameters needed to describe the ray tracer operations
    * _IN_ resources: the resources used in the ray tracer
    * _IN_ rayData: the information passed to the shader through root constants
    */
    void PopulateRayTracer(CommandList& commandList, const PRTPipelineContainer& pipelines,
        const PRTHeapContainer& heaps, const PRTConstantContainer& constants,
        const PRTResourceContainer& resources, const RayData& rayData);

    /*
    * PopulateIntegrator: populates the command list with tasks required by the integerator
    *
    * _IN_ commandListSH: the command list to record on
    * _IN_ pipelines: the pipeline state objects and root signatures needed for the integrator
    * _IN_ heaps: the descriptor heaps needed for the integrator
    * _IN_ resources: the resources used in the integrator
    * _IN_ constants: the parameters needed to describe the integrator operations
    * _IN_ rayData: the information passed to the shader through root constants
    */
    void PopulateIntegrator(CommandList& commandListSH, const PRTPipelineContainer& pipelines,
        const PRTHeapContainer& heaps, const PRTResourceContainer& resources,
        const PRTConstantContainer& constants, RayData& rayData);

    /*
    * StorePRTResult: Stores the spherical harmonic coefficients for the transfer function
    *
    * _OUT_ coefficients: a vector containing the resulting coefficients
    * _IN_ resources: contains the resources needed to copy the data from the GPU
    * _IN_ constants: contains constants needed to define the copy
    */
    void StorePRTResult(std::vector<float>& coefficients, const PRTResourceContainer& resources,
        const PRTConstantContainer& constants);


}
