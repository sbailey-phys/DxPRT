/*
*
* Implimentation of GeneratePRT.h
*
* Many of the functions and structures used here are defined in:
*   GeneratePRT_Utility.h
*   GenerateEM_Utility.h
*   GenerateGeneral_Utility.h
* 
* Forms part of the DxPRT project
*
*
*
* Author: Shaun Bailey
*
* Date created: 05/10/2021
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

#include "DxPRT/GeneratePRT.h"

using namespace DxPRT_Utility;

namespace DxPRT {


    void GenerateEM(ID3D12Device* device, void* data,
        const UINT64& numPixelsX, const UINT64& numPixelsY,
        const std::string& outFile, const EM_DESC& desc) {

        if (!desc.SuppressOutput) std::cout << "Initializing" << std::endl;

        // set up command queue
        CommandQueue commandQueue(device, D3D12_COMMAND_LIST_TYPE_COMPUTE);
        CommandList commandList(device, D3D12_COMMAND_LIST_TYPE_COMPUTE);
        commandList.Close();

        EMConstantContainer constants = InitializeEMConstants(desc, numPixelsX,
            numPixelsY);

        std::vector<std::vector<float>> shData(constants.nCoefficients);
        GenerateSHvector(constants.shGridNum, constants.maxL, shData);

        std::vector<UINT32> randomVector;
        GenerateRandomVector(constants.numEvents, randomVector);

        EMResourceContainer resources;
        InitializeEMResources(device, commandQueue, commandList, resources,
            constants, data, shData, randomVector);

        shData.clear(); // clear up CPU
        randomVector.clear();
        resources.hdrRes.ReleaseUpload();
        resources.randomRes.ReleaseUpload();
        for (int i = 0; i < constants.nCoefficients; ++i) {
            resources.shRes[i].ReleaseUpload();
        }
        
        DescriptorHeap integrateHeap;
        InitializeEMHeap(device, integrateHeap, resources, constants);

        RootSignature integrateRootSig;
        ComputePipeline integratePipeline;
        InitalizeEMPipeline(device, integrateRootSig, integratePipeline, desc.shaderPath);

        if (!desc.SuppressOutput) std::cout << "Calculating coefficients" << std::endl;

        std::vector<float> coefficients;

        ExecuteEMPipeline(commandQueue, commandList, integratePipeline,
            integrateRootSig, integrateHeap, constants, resources);

        StoreEMResult(coefficients, resources, constants);

        if (!desc.SuppressOutput) std::cout << "Writing file" << std::endl;

        PRTWriter outPRTFile;

        outPRTFile.AddCoefficients(desc.MaxL, &coefficients[0], coefficients.size());

        if (!outPRTFile.Write(outFile, true)) {
            std::string warningMessage = "DxPRT: Unable to write to file " + outFile + ". Please provide a location" +
                " that can be accessed\n.";
            OutputDebugStringA(warningMessage.c_str());
        }

        commandQueue.Flush();
        commandQueue.CloseFence();

        if (!desc.SuppressOutput) std::cout << "Finished writing to file: " << outFile << std::endl;

    }

    void GenerateEM(ID3D12Device* device, const std::string& hdrFile,
        const std::string& outFile, const EM_DESC& desc) {

        if (!desc.SuppressOutput) std::cout << "Reading file: " << hdrFile << std::endl;

        HDRReader hdr;
        if (!hdr.Load(hdrFile)) {
            std::string warningMessage = "DxPRT: Unable to read hdr file: " + outFile + ". Please use a valid file" +
                " and check the README document to ensure that it is supported.\n";
            OutputDebugStringA(warningMessage.c_str());
            return;
        }

        GenerateEM(device, hdr.GetData(), hdr.GetNPixelsX(), hdr.GetNPixelsY(),
            outFile, desc);

    }

    void GeneratePRT(ID3D12Device* device, void* vertexData,
        const UINT64& vertexNum, void* indexData, const UINT64& triangleNum,
        void* normalData, const std::string& outFile,
        const PRT_DESC& desc) {

        if (!desc.SuppressOutput) std::cout << "Initializing" << std::endl;

        CommandQueue commandQueue(device, D3D12_COMMAND_LIST_TYPE_COMPUTE);
        CommandList commandList(device, D3D12_COMMAND_LIST_TYPE_COMPUTE),
            commandListSH(device, D3D12_COMMAND_LIST_TYPE_COMPUTE);
        commandList.Close();
        commandListSH.Close();

        PRTDataContainer dataContainer; //passed by reference
        PRTHeapContainer heaps;
        PRTPipelineContainer pipelines;
        PRTResourceContainer resources;

        PRTConstantContainer constants = InitializePRTConstants(desc, triangleNum, vertexNum);
        InitalizePRTDataContainer(dataContainer, constants, (float*)vertexData, (UINT32*)indexData,
            (float*)normalData, vertexNum, triangleNum);
        InitializePRTResources(device, commandQueue, commandList, resources,
            constants, dataContainer);
        CleanUpPRT(dataContainer, resources, constants);
        InitializePRTHeaps(device, heaps, resources, constants);
        InitializePRTPipelines(device, pipelines, desc.shaderPath);

        RayData rayData; // root constants
        rayData.settings.numEventsX = constants.numEventsX;
        rayData.settings.numPlaneChunks = constants.triangleNum / 512 + 1;
        rayData.settings.numPlanes = constants.triangleNum;

        float* pVertex = (float*)vertexData;
        float* pNormal = (float*)normalData;

        std::vector<float> coefficients; // final result

        if (!desc.SuppressOutput) std::cout << "Calculating coefficients" << std::endl;

        for (int i = 0; i < vertexNum; ++i) {

            if (i % 100 == 0 && !desc.SuppressOutput) {
                std::cout << i << " out of " << vertexNum << " vertices processed"
                    << std::endl;
            }

            rayData.rayPos = DirectX::XMFLOAT4(*pVertex, *(pVertex + 1), *(pVertex + 2), 0.0f); //root constants
            rayData.forward = DirectX::XMFLOAT4(*pNormal, *(pNormal + 1), *(pNormal + 2), 0.0f);
            rayData.xDir = DirectX::XMFLOAT4(-(*(pNormal + 1)), *pNormal, 0.0f, 0.0f);

            PopulateRayTracer(commandList, pipelines, heaps, constants, resources, rayData);

            commandQueue.Execute(commandList);
            commandQueue.Signal();

            PopulateIntegrator(commandListSH, pipelines, heaps, resources,
                constants, rayData);

            commandQueue.WaitForFence();

            commandQueue.Execute(commandListSH);
            commandQueue.Signal();
            commandQueue.WaitForFence();

            StorePRTResult(coefficients, resources, constants);

            pVertex += 3;
            pNormal += 3;


        }

        if (!desc.SuppressOutput) std::cout << "Writing to file: " << outFile << std::endl;

        PRTWriter outPRTFile;

        outPRTFile.AddVertices((float*)vertexData, vertexNum * 3);
        outPRTFile.AddCoefficients(desc.MaxL, &coefficients[0], coefficients.size());
        outPRTFile.AddIndices((UINT32*)indexData, triangleNum * 3);

        if (!outPRTFile.Write(outFile)) {
            std::string warningMessage = "DxPRT: Unable to write to file " + outFile + ". Please provide a location" +
                " that can be accessed\n.";
            OutputDebugStringA(warningMessage.c_str());
        }

        commandQueue.Flush();
        commandQueue.CloseFence();

    }

    void GeneratePRT(ID3D12Device* device, const std::string& objFile,
        const std::string& outFile, const PRT_DESC& desc) {

        if (!desc.SuppressOutput) std::cout << "Reading file: " << objFile << std::endl;

        ObjReader obj;
        if (!obj.Load(objFile)) {
            std::string warningMessage = "DxPRT: Unable to read obj file: " + outFile + ". Please use a valid file" +
                " and check the README document to ensure that it is supported\n.";
            OutputDebugStringA(warningMessage.c_str());
            return;
        }

        GeneratePRT(device, obj.GetVertices(), obj.GetSizeVertices() / 3,
            obj.GetIndices(), obj.GetSizeIndices() / 3, obj.GetNormals(),
            outFile, desc);

    }

}
