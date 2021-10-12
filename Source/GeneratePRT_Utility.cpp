/*
*
* Implimentation of the GeneratePRT_Utility.h
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

#include "DxPRT/GeneratePRT_Utility.h"
#include "DxPRT/GeneratePRT.h"

using namespace DxPRT;

namespace DxPRT_Utility {
    

    PRTConstantContainer InitializePRTConstants(const DxPRT::PRT_DESC& desc,
        const UINT64& triangleNum, const UINT64& vertexNum) {
        PRTConstantContainer constants = {};
        constants.maxL = desc.MaxL;
        constants.nCoefficients = (desc.MaxL + 1) * (desc.MaxL + 1);
        constants.triangleNum = triangleNum;
        constants.vertexNum = vertexNum;


        RoundInput(desc.NumEvents, desc.SHGridNum, constants.numEvents,
            constants.numEventsX, constants.shGridNum); // round some input constants

        constants.numThreadGroups = constants.numEvents / (8 * 8);

        return constants;

    }


    void InitalizePRTDataContainer(PRTDataContainer& dataContainer,
        const PRTConstantContainer& constants, float* vertexData,
        UINT32* indexData, float* normalData, const UINT64& numVertex,
        const UINT64& triangleNum) {

        dataContainer.shData.resize(constants.nCoefficients);
        GenerateSHvector(constants.shGridNum, constants.maxL, dataContainer.shData);

        GenerateRandomVector(constants.numEvents, dataContainer.randomData);

        dataContainer.pIndexData = indexData;
        dataContainer.pVertexData = vertexData;

        dataContainer.pNormalData = normalData;

    }


    void InitializePRTResources(ID3D12Device* device, CommandQueue& commandQueue,
        CommandList& commandList, PRTResourceContainer& resources,
        const PRTConstantContainer& constants, const PRTDataContainer& dataContainer) {

        commandList.Reset();

        // contains the random numbers needed for Monte Carlo integration
        resources.randomRes.SetBuffer(constants.numEvents * 8, 4, DXGI_FORMAT_R32_UINT,
            D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
        resources.randomRes.SetState(D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        resources.randomRes.InitializeWithData(device, commandList,
            &dataContainer.randomData[0]);

        // buffer containing the index data
        resources.indexRes.SetBuffer(constants.triangleNum * 3, 4, DXGI_FORMAT_R32_UINT);
        resources.indexRes.SetState(D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
        resources.indexRes.InitializeWithData(device, commandList, dataContainer.pIndexData);

        // buffer contating the vertex data
        resources.vertexRes.SetBuffer(constants.vertexNum, 12, DXGI_FORMAT_R32G32B32_FLOAT);
        resources.vertexRes.SetState(D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
        resources.vertexRes.InitializeWithData(device, commandList, dataContainer.pVertexData);


        /* buffer containg indexed planes that pass the pre-pass stage.
        *  to enable calculation in a compute shader, the buffer is split
        *  into chunks of 512 indicies, with the first n indicies in each
        *  chunk containing the planes that pass the pre-pass. The number
        *  n for each chunk is then stored in the first triangleNum/512 + 1
        *  indices of the buffer
        */
        resources.planeRes.SetBuffer(constants.triangleNum / 512 + 1 + (constants.triangleNum / 512 + 1) * 512, 4, DXGI_FORMAT_R32_UINT,
            D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
        resources.planeRes.SetState(D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        resources.planeRes.Initialize(device);
        
        // buffer containing result of ray trasing algorithm
        resources.visibilityRes.SetBuffer(constants.numEvents, 4, DXGI_FORMAT_R32_UINT,
            D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
        resources.visibilityRes.SetState(D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        resources.visibilityRes.Initialize(device);

        // buffer containing the result of the integration for each group thread.
        // contains all SH components to allow for a long command list
        resources.resultRes.SetBuffer(constants.nCoefficients * constants.numThreadGroups, 4, DXGI_FORMAT_R32_FLOAT,
            D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
        resources.resultRes.SetState(D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        resources.resultRes.Initialize(device);

        // readback buffer to read data on the CPU
        resources.readbackRes.SetBuffer(constants.nCoefficients * constants.numThreadGroups, 4, DXGI_FORMAT_R32_FLOAT);
        resources.readbackRes.SetProperties(D3D12_HEAP_TYPE_READBACK);
        resources.readbackRes.SetState(D3D12_RESOURCE_STATE_COPY_DEST);
        resources.readbackRes.Initialize(device);

        // set up SH texture resources
        resources.shRes.resize(constants.nCoefficients);
        for (int i = 0; i < constants.nCoefficients; ++i) {
            resources.shRes[i].SetTex2D(DXGI_FORMAT_R32_FLOAT, (UINT) constants.shGridNum, (UINT) constants.shGridNum, 4);
            resources.shRes[i].SetState(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            resources.shRes[i].InitializeWithData(device, commandList, &dataContainer.shData[i][0]);
        }

        commandList.Close();
        commandQueue.Execute(commandList);
        commandQueue.Signal();
        commandQueue.WaitForFence();
    }


    void CleanUpPRT(PRTDataContainer& data, PRTResourceContainer& resources,
        const PRTConstantContainer& constants) {
        data.randomData.clear();
        data.shData.clear();

        resources.indexRes.ReleaseUpload();
        resources.planeRes.ReleaseUpload();
        resources.randomRes.ReleaseUpload();
        resources.vertexRes.ReleaseUpload();
        
        for (int i = 0; i < constants.nCoefficients; ++i) {
            resources.shRes[i].ReleaseUpload();
        }

    }


    void InitializePRTHeaps(ID3D12Device* device, PRTHeapContainer& heaps,
        PRTResourceContainer& resources, const PRTConstantContainer& constants) {


        heaps.planeHeap.Initialize(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 3, true);

        resources.vertexRes.CreateSRV(device, heaps.planeHeap.GetCPUHandle(0), 12);
        resources.indexRes.CreateSRV(device, heaps.planeHeap.GetCPUHandle(1), 4);
        resources.planeRes.CreateUAV(device, heaps.planeHeap.GetCPUHandle(2));


        heaps.rayHeap.Initialize(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 5, true);

        resources.vertexRes.CreateSRV(device, heaps.rayHeap.GetCPUHandle(0), 12);
        resources.indexRes.CreateSRV(device, heaps.rayHeap.GetCPUHandle(1), 4);
        resources.planeRes.CreateUAV(device, heaps.rayHeap.GetCPUHandle(2));
        resources.randomRes.CreateUAV(device, heaps.rayHeap.GetCPUHandle(3));
        resources.visibilityRes.CreateUAV(device, heaps.rayHeap.GetCPUHandle(4));


        heaps.integrateHeap.Initialize(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
            3 + (UINT) constants.nCoefficients, true);

        resources.visibilityRes.CreateSRV(device, heaps.integrateHeap.GetCPUHandle(0), 4);
        resources.randomRes.CreateSRV(device, heaps.integrateHeap.GetCPUHandle(1), 4);
        resources.resultRes.CreateUAV(device, heaps.integrateHeap.GetCPUHandle(2));
        for (int i = 0; i < constants.nCoefficients; ++i) {
            resources.shRes[i].CreateSRV(device, heaps.integrateHeap.GetCPUHandle(3 + i));
        }


    }


    void InitializePRTPipelines(ID3D12Device* device, PRTPipelineContainer& pipelines, const std::wstring& shaderPath) {


        pipelines.rayTracerPrePassRootSig.AddConstants(sizeof(RayData) / 4, 0, D3D12_SHADER_VISIBILITY_ALL);
        pipelines.rayTracerPrePassRootSig.AddDesciptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
            2, 0);
        pipelines.rayTracerPrePassRootSig.AddDesciptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
            1, 0);

        pipelines.rayTracerPrePassRootSig.Initialize(device);

        pipelines.rayTracerPrePassPipeline.SetRootSignature(pipelines.rayTracerPrePassRootSig);
        pipelines.rayTracerPrePassPipeline.SetComputeShader((shaderPath + L"/RayTracerPrePassShader.cso").c_str());
        pipelines.rayTracerPrePassPipeline.Initialize(device);


        pipelines.rayTracerRootSig.AddConstants(sizeof(RayData) / 4, 0, D3D12_SHADER_VISIBILITY_ALL);
        pipelines.rayTracerRootSig.AddDesciptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
            2, 0);
        pipelines.rayTracerRootSig.AddDesciptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
            3, 0);
        pipelines.rayTracerRootSig.Initialize(device);

        pipelines.rayTracerPipeline.SetRootSignature(pipelines.rayTracerRootSig);
        pipelines.rayTracerPipeline.SetComputeShader((shaderPath + L"/RayTracerShader.cso").c_str());
        pipelines.rayTracerPipeline.Initialize(device);


        pipelines.integrateRootSig.AddConstants(sizeof(RayData) / 4, 0, D3D12_SHADER_VISIBILITY_ALL);
        pipelines.integrateRootSig.AddDesciptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
            1, 0);
        pipelines.integrateRootSig.AddDesciptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
            2, 1);
        pipelines.integrateRootSig.AddDesciptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
            1, 0);
        pipelines.integrateRootSig.Initialize(device, true);

        pipelines.integratePipeline.SetComputeShader((shaderPath + L"/PRTIntegrateShader.cso").c_str());
        pipelines.integratePipeline.SetRootSignature(pipelines.integrateRootSig);
        pipelines.integratePipeline.Initialize(device);
    }



    void PopulateRayTracer(CommandList& commandList, const PRTPipelineContainer& pipelines,
        const PRTHeapContainer& heaps, const PRTConstantContainer& constants,
        const PRTResourceContainer& resources, const RayData& rayData) {

        commandList.Reset();

        commandList.SetPipeline(pipelines.rayTracerPrePassPipeline, pipelines.rayTracerPrePassRootSig);
        commandList.SetDescriptorHeap(1, heaps.planeHeap.GetHeap());
        commandList.SetComputeConstants(0, sizeof(RayData) / 4, &rayData);
        commandList.SetComputeDesciptorTable(1, heaps.planeHeap.GetGPUHandle(0));
        commandList.SetComputeDesciptorTable(2, heaps.planeHeap.GetGPUHandle(2));

        commandList.Dispatch(rayData.settings.numPlaneChunks, 1, 1);

        commandList.Barrier(resources.planeRes.GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);


        commandList.SetPipeline(pipelines.rayTracerPipeline, pipelines.rayTracerRootSig);
        commandList.SetDescriptorHeap(1, heaps.rayHeap.GetHeap());
        commandList.SetComputeConstants(0, sizeof(RayData) / 4, &rayData);
        commandList.SetComputeDesciptorTable(1, heaps.rayHeap.GetGPUHandle(0));
        commandList.SetComputeDesciptorTable(2, heaps.rayHeap.GetGPUHandle(2));

        commandList.Dispatch((UINT) constants.numEventsX / 8, (UINT) constants.numEventsX / 8, 1);

        commandList.Barrier(resources.planeRes.GetResource(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

        commandList.Close();

    }


    void PopulateIntegrator(CommandList& commandListSH, const PRTPipelineContainer& pipelines,
        const PRTHeapContainer& heaps, const PRTResourceContainer& resources,
        const PRTConstantContainer& constants, RayData& rayData) {

        commandListSH.Reset();

        commandListSH.Barrier(resources.randomRes.GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        commandListSH.Barrier(resources.visibilityRes.GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);


        for (int j = 0; j < constants.nCoefficients; ++j) {

            commandListSH.SetPipeline(pipelines.integratePipeline, pipelines.integrateRootSig);

            commandListSH.SetDescriptorHeap(1, heaps.integrateHeap.GetHeap());

            rayData.settings.iSH = j;

            commandListSH.SetComputeConstants(0, sizeof(RayData) / 4, &rayData);

            commandListSH.SetComputeDesciptorTable(1, heaps.integrateHeap.GetGPUHandle(3 + j));
            commandListSH.SetComputeDesciptorTable(2, heaps.integrateHeap.GetGPUHandle(0));
            commandListSH.SetComputeDesciptorTable(3, heaps.integrateHeap.GetGPUHandle(2));

            commandListSH.Dispatch((UINT) constants.numEventsX / 8, (UINT) constants.numEventsX / 8, 1);

        }


        commandListSH.Barrier(resources.randomRes.GetResource(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        commandListSH.Barrier(resources.visibilityRes.GetResource(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS);


        commandListSH.Barrier(resources.resultRes.GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
            D3D12_RESOURCE_STATE_COPY_SOURCE);

        commandListSH.CopyResource(resources.readbackRes.GetResource(), resources.resultRes.GetResource());

        commandListSH.Barrier(resources.resultRes.GetResource(), D3D12_RESOURCE_STATE_COPY_SOURCE,
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

        commandListSH.Close();
    }


    void StorePRTResult(std::vector<float>& coefficients, const PRTResourceContainer& resources,
        const PRTConstantContainer& constants) {


        D3D12_RANGE readbackBufferRange{ 0, constants.nCoefficients * constants.numThreadGroups * sizeof(float) };
        float* pReadbackBufferData{};
        resources.readbackRes.GetResource()->Map(0, &readbackBufferRange,
            reinterpret_cast<void**>(&pReadbackBufferData));

        for (int j = 0; j < constants.nCoefficients; ++j) {
            float total = 0;
            for (int k = 0; k < constants.numThreadGroups; ++k) {
                total += *(pReadbackBufferData + j * constants.numThreadGroups + k);
            }

            coefficients.push_back(total / float(constants.numEvents) * 4.0f);
        }
        D3D12_RANGE emptyRange{ 0, 0 };
        resources.readbackRes.GetResource()->Unmap(0, &emptyRange);
    }

}

