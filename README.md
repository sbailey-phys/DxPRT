# DxPRT

DxPRT is a precomputed radiance transfer framework implemented in DirectX12 that enables realistic lighting and shadows on static objects from the surrounding environment. For this to be done, the radiance-transfer equation is decomposed into spherical harmonics and the relevant coefficients calculated off-line, enabling rapid evaluation of the resulting lighting when rendering the object. This project contains the code necessary calculate these coefficients for both the transfer functions and the environment map as well as to render the resulting object and lighting. With minimal dependencies and in-built file readers, this project is self-contained, simple to use, and ideal for demonstration purposes.

While this project is fully functioning in its current state, it is still being developed and is thus limited in some areas. In particular the ray-tracing algorithm requires further optimizations, and it is only recommended to process objects with a few thousand vertices. Further, only diffuse objects are supported, with inter-reflected bounces and specular materials to be implemented shortly. It is therefore suggested that this is only used for demonstration purposes, with future updates enabling DxPRT to be used within larger pipelines. See below for the current to-do list.

<img src="https://github.com/sbailey-phys/DxPRT/blob/main/Screenshots/BunnyScreenshot1.png" width=50% height=50%>
## Dependencies
DxPRT has been designed with minimal dependencies on external libraries, only depending on those that are standard with DirextX12 and Win32. In particular, the following libraries need to be linked to the project:
-dxgi.lib
-d3dcompiler.lib
-d3d12.lib
Further to this, the standard helper functions are used, and the following header should be present in the include directory:
-d3dx12.h
Other than these, several Win32 headers are used, although the number of these will reduce in future updates. We do not list these as it is expected that the user will have these standard headers.

To use this project, the DxPRT file should be present in the include directory. All files present in the source files must be present in the current working directory. Finally, the path to the Shaders file (that contains the compiled shaders) needs to be input into various functions during both the generation of coefficients and rendering. All functions, classes and structs defined in this project are under the DxPRT namespace.



## Generation of coefficients
To generate the spherical harmonics coefficients, the header file DxPRT/GeneratePRT.h should be included. With this the functions GeneratePRT and GenerateEM can be called to generate the coefficients for the transfer function and environment map respectively. 
Overloaded functions for both of these enable easy use through reading in a .obj file or a .hdr file to provide the necessary data. For the .hdr file, it must be in the RGBE format, be run-length encoded and have the standard orientation. Further, it is expected that theta varies along the y-direction while phi varies along the x-direction, where (theta, phi) are the standard spherical coordinates. 
The structs PRT_DESC and EM_DESC are also defined in this header and allow the user to define the operation of the integration and the number of coefficients used. For precise definitions of these structs and of the Generate functions, see below.
Please see demos/DxPRTGenerateDemo.cpp for an example of how these functions can be used.
## Rendering the mesh
The rendering of the mesh is handled by the Workspace class, defined in the DxPRT/Workspace.h header. This class loads in the files generated by the GenerateEM and GeneratePRT functions and renders them to the set render target view. Multiple environments maps can be loaded at once to allow for easy comparison, although only one mesh can be loaded at a time. For each environment map, a .hdr file must be provided to allow rendering of the skybox, although this requirement will likely be removed in future iterations. The format of the .hdr file must match the same requirements of the GenerateEM function (see above). The expected use of this class is as follows:
1) construct the class, stating the maximum number of environment maps that will be used
 
 2) add in the relevant environment maps and meshes using the AddEM() and AddPRT() methods. These record copy commands onto a command list and can be done con-currently on multiple threads
 
 3) execute all commands used in the previous step to ensure all resources are on the
     GPU
	
 4) free the CPU memory with the CleanUpCPU() method
 
 5) initialize the object using the Initialize() method
 
 6) set all necessary parameters (this does not need to be done every frame as they are stored in the class).
 
 7) when ready to record the draw command, first call SetRTVHandle() and DSVHandle() to set the relevant non-shader visible descriptor, then call the Render() method

Precise details of each function are given below.



## Details of individual functions and structs

**Please note: all of the following are under the namespace DxPRT**
```c++
	struct EM_DESC {
		UINT64 MaxL = 3;
		UINT64 NumEvents = 262144;
		UINT64 SHGridNum = 512;
		bool SuppressOutput = false;
		std::wstring shaderPath = L"";
	};
	struct PRT_DESC {
		UINT64 MaxL = 3;
		UINT64 NumEvents = 262144;
		UINT64 SHGridNum = 512;
		bool SuppressOutput = false;
		std::wstring shaderPath = L"";
	};

```
These structs are used to define the integration over the environment map in GenerateEM and the transfer function in GeneratePRT.
-	MaxL: maximum l value for the spherical harmonics
-	NumEvents: total number of events used in the Monte Carlo Integration
-	ShGridNum: the number of grid points (in both theta and phi) used to store the spherical harmonics
-	SuppressOutput: if set to true, no text will be output to the console
-	shaderPath: path to the folder containing the shader files
```c++
	void GenerateEM(ID3D12Device device, void data,
		const UINT64& numPixelsX, const UINT64& numPixelsY,
		const std::string& outFile, const EM_DESC& desc);
```
Processes and environment map to generate a .prt file containing the
 harmonic coefficients. The data file must have phi vary along the x-direction and theta vary along the y-direction. If the output file cannot be accessed, then this function will fail.
-	_IN_ device: the currently active device
-	_IN_ numPixelsX: the number of pixels in the x-direction (width)
-	_IN_ numPixelsY: the number of pixels in the y-direction (height)
-	_IN_ outFile: the path to the output file where the coefficients will be stored
-	_IN_ desc: an EM_DESC object containing parameters for the integration
```c++
void GenerateEM(ID3D12Device device, void data,
		const UINT64& numPixelsX, const UINT64& numPixelsY,
		const std::string& outFile, const EM_DESC& desc);
```
Performs the same operation as the above function but takes in a .hdr as input. This file must be in the RGBE format and be run-length encoded. If the file cannot be read, then the function will fail and you should instead consider using the above function instead. 
-	_IN_ device: the currently active device
-	_IN_ hdrFile: the path to the hdr file to be read
-	_IN_ outFile: the path to the output file where the coefficients will be stored
-	_IN_ desc: an EM_DESC object containing parameters for the integration

```c++
void GeneratePRT(ID3D12Device device, void vertexData,
		const UINT64& vertexNum, void indexData, const UINT64& triangleNum,
		void normalData, const std::string& outFile,
		const PRT_DESC& desc);
```
Processes a mesh to generate the spherical harmonic coefficients to describe the transfer function. This also includes a ray tracer to take into account the effects of shadows. If the output file cannot be accessed, then this function will fail.
	 
-	_IN_ device: the currently active device
-	_IN_ vertexData: a pointer to the vertex data, this should contain 3 floats per vertex
-	_IN_ vertexNum: the total number of vertices in the mesh
-	_IN_ indexData: a pointer to the index data, this should contain 3 4-byte unsigned integers per triangle
-	_IN_ triangleNum: the total number of triangles in the mesh
-	_IN_ normalData: a pointer to the normal data, this should contain 3 floats per normal
-	_IN_ outFile: the path to the output file where the coefficients for each vertex will be stored
-	_IN_ desc: an PRT_DESC object containing parameters for the ray tracer and integration
	/
```c++
void GeneratePRT(ID3D12Device device, const std::string& objFile,
		const std::string& outFile, const PRT_DESC& desc);
```
Same functionality as the above function but takes in a .obj file as input
	 
	 
-	_IN_ device: the currently active device
-	_IN_ objFile: the path to the object file to be read
-	_IN_ outFile: the path to the output file where the coefficients for each vertex will be stored
-	_IN_ desc: an PRT_DESC object containing parameters for the ray tracer and integration
	
```c++
Workspace::Workspace(int numEM = 1);
```
Constructs the workspace with the specified number of environment maps. This number should not be exceeded during the lifetime of the object.
		 
-	_IN_ numEM: maximum number of environment maps

```c++
void Workspace::AddEM(ID3D12Device device, ID3D12GraphicsCommandList commandList,
			const std::string& emFile, const std::string& hdrFile, int iEM);
```
Adds an environment map to the workspace. Any number of environment maps can be added in any order. However, the total number of environment should not exceed that stated on creation of the workspace. A command list must be passed to record the relevant copy commands, and this should be executed before calling Initialize. This method, as well the AddPRT method be called upon from multiple threads.
		 
-	_IN_ device: the currently active device
-	_IN_ commandList: the command list used to record
-	_IN_ emFile: the path to the .prt file containing the environment map coefficients
-	_IN_ hdrFile: the path to the .hdr file containing the image used for the skybox
-	_IN_ iEM: the index used to identify the environment map
```c++
void Workspace::AddPRT(ID3D12Device device, ID3D12GraphicsCommandList commandList,
			const std::string& prtFile);
```
Adds a mesh to the workspace. Unlike AddEM, only a single mesh can be added. This mesh should be from a .prt file, where the coefficients are also listed. A command list must be passed to record the relevant copy commands and this should be executed before calling Initialize. This method, as well the AddEM method can be called upon from multiple threads.
		
-	_IN_ device: the currently active device
-	_IN_ commandList: the command list used to record
-	_IN_ emFile: the path to the .prt file containing the mesh and coefficients
```c++
void Workspace::SetView(const DirectX::XMMATRIX& view);
```
Sets the view matrix of the object
		 
-	_IN_ view: the view matirx

```c++
void Workspace::SetProjection(const DirectX::XMMATRIX& projection);
```
Sets the projection matrix of the object
		
-	_IN_ view: the projection matirx
```c++
void Workspace::SetModelMatrix(const float& x, const float& y,
			const float& z, const float& scale);
```
Uses the position and scale of the object to construct the model matrix
		 
-	_IN_ x: the x translation
-	_IN_ y: the y translation
-	_IN_ z: the z translation
-	_IN_ scale: the factor to scale the object (in all 3 directions)
```c++
void Workspace::SetRTVHandle(const D3D12_CPU_DESCRIPTOR_HANDLE& rtvHandle);
```
Sets the render target view that the GPU will output to. This should have the format DXGI_R8G8B8A8_UINT and should be called just before the render method is called
		 
-	_IN_ rtvHandle: the CPU handle to the RTV descriptor
```c++
void Workspace::SetDSVHandle(const D3D12_CPU_DESCRIPTOR_HANDLE& dsvHandle);
```
Sets the depth stencil view that the GPU will output to. This should be called just before the render method is called
		
-	_IN_ dsvHandle: the CPU handle to the DSV descriptor
```c++
void Workspace::SetRect(const CD3DX12_RECT& rect);
```
Sets the rect for the rasteriser stage
		 
-	_IN_ rect: the rect to be set
```c++
void Workspace::SetViewport(const CD3DX12_VIEWPORT& viewport);
```
Sets the viewport for the rasterizer stage
		 
-	_IN_ viewport: the viewport to be set
```c++
void Workspace::SetCurrentEM(const UINT& iEM);
```
Sets the current environment map to be displayed. This should exceed the number of environment maps in the constructor, however, if it does then the modulo will be taken.
		 
-	_IN_ iEM: the index of the environment map to be displayed
```c++
void Workspace::SetExposure(const float& exposure);
```
Sets the exposure to be used when converting from a high dynamic range to a low dynamic range
		 
-	_IN_ exposure: the exposure to be set
```c++
void Workspace::Initalize(ID3D12Device device, const std::wstring& shaderPath);
```
Initializes the workspace and ensures that all shader views are created. This should be called after all relevant meshes and environment maps are added and their command lists have been executed. Further, this should be called before the render method is used.
		 
-	_IN_ device: the currently active device
-	_IN_ shaderPath: path to the folder containing the compiled shaders (.cso)

```c++
void Workspace::CleanUpCPU();
```
Releases all data held on the CPU. should be called after the command list used to store the copy commands in AddEM and AddPRT has been executed.
```c++
void Workspace::Render(ID3D12GraphicsCommandList commandList) const;
```
Records the commands necessary to render the object with the precomputed radiance transfer used as lighting. All parameters set before this method is called will be used in the render. Initialize needs to be called before this method is used.  Further, you should ensure that the correct RTVs and DSVs are set, this is likely to happen shortly before this method is called.
		 
-	_IN_ commandList: a command list to record the commands

## To do list
### Main features
-	[ ] Optimise the performance of the ray tracer
-	[ ] Enable the use of inter-reflected bounces
-	[ ] Specular materials
-	[ ] Analytical area lights
-	[ ] un-shadowed rendering
### Utility
-	[ ] Allow for the skybox to be disabled
-	[ ] Allow for the shader to output to a high dynamic range
-	[ ] Allow for more configurability when rendering, in particular colour




