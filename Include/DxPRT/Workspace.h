/*
*
* This file contains the Workspace class, designed to render the objects through
* the use of a precomputed radience transfer.
* 
* The files used during the rendering need to first be generated seperatly 
* (see GeneratePRT.h)
* 
* The expected workflow of this class is as follows:
* 
* 1) constuct the class, stating the maximum number of environment maps that will
*    be used
* 
* 2) add in the relevant enviroment maps and meshs using the AddEM() and AddPRT() methods.
*    This records copy commands onto a command list and can be done con-currently on 
*    multiple threads
* 
* 3) execute all commands used in the previous step to ensure all resources are on the
*     GPU
*	
* 4) free the CPU memory with the CleanUpCPU() method
* 
* 5) initialize the object using the Initialize() method
* 
* 6) set all nessecary parameters (this does not need to be done every frame as they are
*    stored in the class).
* 
* 7) when ready to record the draw command, first call SetRTVHandle() and DSVHandle() to set
*    the relevant non-shader visible desciptor, then call the Render() method
*
*
*
* Forms part of the DxPRT project
*
*
*
* Author: Shaun Bailey
*
* Date created: 05/10/2021
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
#include <string>
#include <d3d12.h>
#include <DirectXMath.h>
#include "d3dx12.h"
#include <vector>
#include <mutex>
#include "DxPRT/Resource.h"
#include "DxPRT/PRTReader.h"
#include "DxPRT/HDRReader.h"
#include "DxPRT/Pipeline.h"
#include "DxPRT/RootSignature.h"
#include "DxPRT/DescriptorHeap.h"
#include "DxPRT/Model.h"
#include "DxPRT/WorkspaceRootParameters.h"

namespace DxPRT {

	class Workspace
	{

	public:

		/*
		* constructor: creates the workspace with the specified number of environment
		* maps. This number should not be exceeded during the lifetime of the object.
		* 
		* _IN_ numEM: maximum number of environment maps
		*/
		Workspace(int numEM = 1);


		/*
		* AddEM: adds an environment map to the workspace. Any number of environment
		* maps can be added in any order. However, the total number of environment
		* maps should not exeed that stated on creation of the workspace. A command
		* list must be passed to record the relevant copy commands and this should be 
		* executed before calling Initialize. This method, as well the AddPRT method
		* can be called upon from multiple threads.
		* 
		* _IN_ device: the currently active device
		* _IN_ commandList: the command list used to record
		* _IN_ emFile: the path to the .prt file containing the environment map coefficients
		* _IN_ hdrFile: the path to the .hdr file containing the image used for the skybox
		* _IN_ iEM: the index used to identify the environment map
		*/
		void AddEM(ID3D12Device* device, ID3D12GraphicsCommandList* commandList,
			const std::string& emFile, const std::string& hdrFile, int iEM);


		/*
		* AddEM: adds a mesh to the workspace. Unlike AddEM, only a single mesh can be 
		* added. This mesh should be from a .prt file, where the coefficients are also
		* listed. A command list must be passed to record the relevant copy commands and
		* this should be executed before calling Initialize. This method, as well the
		* AddEM method can be called upon from multiple threads.
		*
		* _IN_ device: the currently active device
		* _IN_ commandList: the command list used to record
		* _IN_ emFile: the path to the .prt file containing the mesh and coefficients
		*/
		void AddPRT(ID3D12Device* device, ID3D12GraphicsCommandList* commandList,
			const std::string& prtFile);

		/*
		* SetView: sets the view matrix of the object
		* 
		* _IN_ view: the view matirx
		*/
		void SetView(const DirectX::XMMATRIX& view);


		/*
		* SetProjection: sets the projection matrix of the object
		*
		* _IN_ view: the projection matirx
		*/
		void SetProjection(const DirectX::XMMATRIX& projection);

		/*
		* SetModelMatix: uses the position and scale of the object to construct the 
		* model matrix
		* 
		* _IN_ x: the x translation
		* _IN_ y: the y translation
		* _IN_ z: the z translation
		* _IN_ scale: the factor to scale the object (in all 3 directions)
		*/
		void SetModelMatrix(const float& x, const float& y,
			const float& z, const float& scale);

		/*
		* SetRTVHandle: sets the render target view that the GPU will output to.
		* This should have the format DXGI_R8G8B8A8_UINT and should be called just
		* before the render method is called
		* 
		* _IN_ rtvHandle: the CPU handle to the RTV descriptor
		*/
		void SetRTVHandle(const D3D12_CPU_DESCRIPTOR_HANDLE& rtvHandle);


		/*
		* SetDSVHandle: sets the depth stencil view that the GPU will output to.
		* This should be called just before the render method is called
		*
		* _IN_ dsvHandle: the CPU handle to the DSV descriptor
		*/
		void SetDSVHandle(const D3D12_CPU_DESCRIPTOR_HANDLE& dsvHandle);

		/*
		* SetRect: sets the rect for the rasteriser stage
		* 
		* _IN_ rect: the rect to be set
		*/
		void SetRect(const CD3DX12_RECT& rect);

		/*
		* SetViewport: sets the viewport for the rasterizer stage
		* 
		* _IN_ viewport: the viewport to be set
		*/
		void SetViewport(const CD3DX12_VIEWPORT& viewport);

		/*
		* SetCurrentEM: sets the current environment map to be displayed. This should
		* not exceed the number of evironment maps in the constructor, however, if it does
		* then the modulo will be take
		* 
		* _IN_ iEM: the index of the environment map to be displayed
		*/
		void SetCurrentEM(const UINT& iEM);

		/*
		* SetExposure: sets the exposure to be used when converting from a high dynamic 
		* range to a low dynamic range
		* 
		* _IN_ exposure: the exposure to be set
		*/
		void SetExposure(const float& exposure);

		/*
		* SetMaxL: sets the maximum number of coefficients used in the rendering to 
		* (maxL+1)^2. Note that this will not have any effect if this exceeds the number of
		* coefficietns in either the environment map or the mesh
		* 
		* _IN_ maxL: the maximum value of l to be used
		*/
		void SetMaxL(const UINT& maxL);

		/*
		* Initialize: initializes the workspace and ensures that all shader views are created.
		* This should be called after all relevent meshes and environment maps are added and 
		* their command lists have been executed. Further, this should be called before the 
		* render method is used.
		* 
		* _IN_ device: the currently active device
		* _IN_ shaderPath: path to the folder containing the compiled shaders (.cso)
		*/
		void Initalize(ID3D12Device* device, const std::wstring& shaderPath);
		
		/*
		* CleanUpCPU: releases all data held on the CPU. should be called after the command list
		* used to store the copy commands in AddEM and AddPRT has been executed.
		*/
		void CleanUpCPU();


		/*Render: records the commands nessecarry to render the object with the precomputed 
		* radiance transfer used as lighting. All parameters set before this method is called
		* will be used in the render. Initialize needs to be called before this method is used.
		* Further, you should ensure that the correct RTVs and DSVs are set, this is likely to 
		* happen shortly before this method is called.
		* 
		* _IN_ commandList: a command list to record the commands
		*/
		void Render(ID3D12GraphicsCommandList* commandList) const;

	private:

		/*
		* InitializeHeap: initializes the descriptor heaps and the shader resource view for both
		* the PRT and skybox pipelines
		* 
		* _IN_ device: the currently active device
		*/
		void InitializeHeaps(ID3D12Device* device);

		/*
		* InitializePRTPipeline: initializes the pipeline state object and the root signature
		* needed to render the prt object
		* 
		* _IN_ device: the currently active device
		* _IN_ shaderPath: the path to the folder containing the compiled shaders (.cso)
		*/
		void InitializePRTPipeline(ID3D12Device* device, const std::wstring& shaderPath);


		/*
		* InitializeSkyboxPipeline: initializes the pipeline state object and the root signature
		* needed to render the skybox
		*
		* _IN_ device: the currently active device
		* _IN_ shaderPath: the path to the folder containing the compiled shaders (.cso)
		*/
		void InitializeSkyboxPipeline(ID3D12Device* device, const std::wstring& shaderPath);


		/*
		* RenderPRTObject: populates the command list with the commands to render the objec with
		* precomputed randiance transfer used as lighting
		*
		* _IN_ commandList: the command list set to record
		*/
		void RenderPRTObject(ID3D12GraphicsCommandList* commandList) const;


		/*
		* RenderSkybox: populates the command list with the commands to render a skybox
		* 
		* _IN_ commandList: the command list set to record
		*/
		void RenderSkybox(ID3D12GraphicsCommandList* commandList) const;


		DxPRT_Utility::Resource prtRes_;
		std::vector<DxPRT_Utility::Resource> emRes_, hdrRes_;
		DirectX::XMMATRIX viewMatrix_, projectionMatrix_, modelMatrix_;
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle_, dsvHandle_;
		D3D12_RECT rect_;
		CD3DX12_VIEWPORT viewport_;
		UINT iEM_ = 0, maxL_ = 1000, prtMaxL_ = 0, numEM_;
		std::vector<UINT> emMaxL_;
		float exposure_ = 1.0f;

		DxPRT_Utility::DescriptorHeap prtHeap_, skyboxHeap_;
		DxPRT_Utility::Pipeline prtPipeline_, skyboxPipeline_;
		DxPRT_Utility::RootSignature prtRootSig_, skyboxRootSig_;
		DxPRT_Utility::Model skyboxModel_, prtModel_;

		DxPRT_Utility::PRTReader prtData_;
		std::vector<DxPRT_Utility::PRTReader> emData_;
		std::vector<DxPRT_Utility::HDRReader> hdrData_;

		bool skyboxModelInitalized_ = false;
		bool isInitialized_ = false;

		std::mutex hdrResMutex_, emResMutex_, emDataMutex_,
			hdrDataMutex_, emMaxLMutex_; // synchronization for AddEM

	};



}