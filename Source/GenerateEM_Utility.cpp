/*
*
* Implimentation of the GenerateEM_Utility.h
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

#include "DxPRT/GenerateEM_Utility.h"
#include "DxPRT/GeneratePRT.h"

using namespace DxPRT;

namespace DxPRT_Utility {

    EMConstantContainer InitializeEMConstants(const DxPRT::EM_DESC& desc, const UINT64& numPixelsX,
        const UINT64& numPixelsY) {
        EMConstantContainer constants = {};
        constants.maxL = desc.MaxL;
        constants.nCoefficients = (desc.MaxL + 1) * (desc.MaxL + 1);
        constants.numPixelsX = numPixelsX;
        constants.numPixelsY = numPixelsY;

        RoundInput(desc.NumEvents, desc.SHGridNum, constants.numEvents,
            constants.numEventsX, constants.shGridNum);
        constants.numThreadGroups = constants.numEvents / (8 * 8);

        return constants;
    }


    void InitializeEMResources(ID3D12Device* device, CommandQueue& commandQueue, CommandList& commandList,
        EMResourceContainer& resources, const EMConstantContainer& constants,
        const void* data, const std::vector<std::vector<float>>& shData,
        const std::vector<UINT32>& randomVector) {

        commandList.Reset();

        resources.randomRes.SetBuffer(constants.numEvents * 8, sizeof(UINT32), DXGI_FORMAT_R32_UINT,
            D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
        resources.randomRes.SetState(D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        resources.randomRes.InitializeWithData(device, commandList.GetCommandList(), &randomVector[0]);

        resources.resultRes.SetBuffer(constants.nCoefficients * constants.numThreadGroups * 3,
            sizeof(float), DXGI_FORMAT_R32_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
        resources.resultRes.SetState(D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        resources.resultRes.Initialize(device);

        resources.readbackRes.SetProperties(D3D12_HEAP_TYPE_READBACK);
        resources.readbackRes.SetBuffer(constants.nCoefficients * constants.numThreadGroups * 3,
            sizeof(float), DXGI_FORMAT_R32_FLOAT);
        resources.readbackRes.SetState(D3D12_RESOURCE_STATE_COPY_DEST);
        resources.readbackRes.Initialize(device);

        resources.hdrRes.SetTex2D(DXGI_FORMAT_R32G32B32_FLOAT, constants.numPixelsX, constants.numPixelsY, 12);
        resources.hdrRes.SetState(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        resources.hdrRes.InitializeWithData(device, commandList.GetCommandList(), data);

        resources.shRes.resize(constants.nCoefficients);
        for (int i = 0; i < constants.nCoefficients; ++i) {
            resources.shRes[i].SetTex2D(DXGI_FORMAT_R32_FLOAT, constants.shGridNum, constants.shGridNum, 4);
            resources.shRes[i].SetState(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            resources.shRes[i].InitializeWithData(device, commandList.GetCommandList(), &shData[i][0]);
        }

        commandList.Close();
        commandQueue.Execute(commandList);
        commandQueue.Signal();
        commandQueue.WaitForFence();

    }

    void InitializeEMHeap(ID3D12Device* device, DescriptorHeap& heap,
        EMResourceContainer& resources, const EMConstantContainer& constants) {

        heap.Initialize(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
            3 + constants.nCoefficients, true);

        resources.randomRes.CreateUAV(device, heap.GetCPUHandle(0));
        resources.hdrRes.CreateSRV(device, heap.GetCPUHandle(1));
        resources.resultRes.CreateUAV(device, heap.GetCPUHandle(2));

        for (int i = 0; i < constants.nCoefficients; ++i) {
            resources.shRes[i].CreateSRV(device, heap.GetCPUHandle(3 + i));
        }
    }

    void InitalizeEMPipeline(ID3D12Device* device, RootSignature& rootSig,
        ComputePipeline& pipeline, const std::wstring& shaderPath) {
        rootSig.AddDesciptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
            1, 0);
        rootSig.AddDesciptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
            1, 0);
        rootSig.AddDesciptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
            1, 1);
        rootSig.AddDesciptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
            1, 1);

        rootSig.AddConstants(4, 0, D3D12_SHADER_VISIBILITY_ALL);
        rootSig.Initialize(device, true);

        std::wstring integrateShaderPath = (shaderPath + L"/EMIntegrateShader.cso");

        pipeline.SetComputeShader(integrateShaderPath.c_str());
        pipeline.SetRootSignature(rootSig);
        pipeline.Initialize(device);
    }


    void ExecuteEMPipeline(CommandQueue& commandQueue, CommandList& commandList,
        const ComputePipeline& integratePipeline, const RootSignature& integrateRootSig,
        const DescriptorHeap& heap, const EMConstantContainer& constants,
        const EMResourceContainer& resources) {

        EMSETTINGS emSettings;
        emSettings.numEventsX = constants.numEventsX;

        commandList.Reset();

        commandList.SetPipeline(integratePipeline, integrateRootSig);

        commandList.SetDescriptorHeap(1, heap.GetHeap());

        commandList.SetComputeDesciptorTable(0, heap.GetGPUHandle(0));
        commandList.SetComputeDesciptorTable(1, heap.GetGPUHandle(1));
        commandList.SetComputeDesciptorTable(2, heap.GetGPUHandle(2));

        for (int i = 0; i < constants.nCoefficients; ++i) {

            emSettings.iCoefficient = i;

            commandList.SetComputeDesciptorTable(3, heap.GetGPUHandle(3 + i));

            commandList.SetComputeConstants(4, 4, &emSettings);

            commandList.Dispatch(constants.numEventsX / 8, constants.numEventsX / 8, 1);

        }

        commandList.Barrier(resources.resultRes.GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
            D3D12_RESOURCE_STATE_COPY_SOURCE);

        commandList.CopyResource(resources.readbackRes.GetResource(),
            resources.resultRes.GetResource());

        commandList.Barrier(resources.resultRes.GetResource(), D3D12_RESOURCE_STATE_COPY_SOURCE,
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

        commandList.Close();

        commandQueue.Execute(commandList);
        commandQueue.Signal();
        commandQueue.WaitForFence();
    }


    void StoreEMResult(std::vector<float>& coefficients, const EMResourceContainer& resources,
        const EMConstantContainer& constants) {

        D3D12_RANGE readbackBufferRange{ 0, sizeof(float) * 3 * constants.numThreadGroups * constants.nCoefficients };
        float* pReadbackBufferData{};
        resources.readbackRes.GetResource()->Map(
            0, &readbackBufferRange,
            reinterpret_cast<void**>(&pReadbackBufferData));


        for (int j = 0; j < constants.nCoefficients; ++j) {
            float total[] = { 0.0f, 0.0f, 0.0f };
            for (int k = 0; k < constants.numThreadGroups; ++k) {
                total[0] += *(pReadbackBufferData + (j * constants.numThreadGroups + k) * 3);
                total[1] += *(pReadbackBufferData + (j * constants.numThreadGroups + k) * 3 + 1);
                total[2] += *(pReadbackBufferData + (j * constants.numThreadGroups + k) * 3 + 2);
            }

            for (int k = 0; k < 3; ++k) {
                coefficients.push_back(total[k] / float(constants.numEvents) * 4.0f * 3.14159f);
            }
        }

        D3D12_RANGE emptyRange{ 0, 0 };
        resources.readbackRes.GetResource()->Unmap(0, &emptyRange);

    }

}