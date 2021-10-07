/*
*
* 
* This file contains a demo to render objects that are lit by a precomputed 
* radiance transfer. Before rending these objects, the relevant .prt files
* should be procuded using GeneratePRT and GenerateEM; however, for this
* demo, the needed files are already available.
* 
* Three environment maps are loaded for this demo to allow easy comparision 
* between them. The skyboxes are also loaded. These are provided from 
* polyhaven.com
* 
* This demo responds to the following user inputs:
* 
* click and drag: rotate camera
* mousewheel: zoom in/out
* left and right arrow keys: change the current environment map
* up and down arrow keys: change the exposure
* +/-: change the number of coefficients used in the calculation
* 
* To deomstrate the use of the DxPRT project, this code contains no other
* classes/functions provided in sperate files expect for the DxPRT 
* workspace object, Win32 and DirectX12. The only excption to this is the
* DxPRT_Utility::ThrowIfFailed function, however this functions as expected.
* 
* Due to the above choice, a singificant portion of this code is DirectX 
* functions which can be ignored. To understand the logic behind DxPRT and
* this demo, it is best to read the following explaination, and examine the
* referenced functions in the code:
* 
* Initialization:
*
* The main initialization is performed in the function InitializePRT(). Here,
* multiple thread are started to generate the needed environment maps and meshs,
* using the emThreadFunction() and prtThreadFunction() functions. Once this has been
* compeleted, the command lists are executes, after which a call to 
* Workspace::Initialize() is performed. After this, the relevant parameters are 
* then initialized through functions such Workspace::SetEM().
* 
* After the workspace has been initialized, the contols detailed above can be used
* to alter the parameters stored in the workspace object. This is mostly performed 
* in the WndProc() function and those called from here. 
* 
* Finally, the object is rendered to screen in the Render() function. This follows a
* standard DirectX12 load and present cycle. To load the object, calls to the functions
* Workspace::SetRTVHandle() and Workspace::SetDSVHandle() are used to tell the workspace
* where to send the rendered object. The RTV should be of the format DXGI_R8G8B8A8_UINT.
* After this, Workspace::Render() can be called, this places the draw command onto the
* passed command list, after which it can be executed.
* 
*
* Author: Shaun Bailey
*
* Date created: 04/10/2021
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


#define WINDOWS_LEAN_AND_MEAN
#include <Windows.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "d3dx12.h"
#include <wrl.h>
#include <combaseapi.h>
#include <assert.h>
#include <chrono>
#include <algorithm>
#include <vector>
#include "DxPRT/Workspace.h"
#include "DxPRT/DxPRT_Error.h" // DxPRT_Utility::ThrowIfFailed
#include <thread>


unsigned int g_clientWidth = 500, g_clientHeight = 500;

static const float PI = 3.1415f;

// Global variables

HWND g_hWnd;

// directX 12 global vairables
Microsoft::WRL::ComPtr<ID3D12Device5> g_device;
Microsoft::WRL::ComPtr<ID3D12CommandQueue> g_commandQueue;
Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> g_commandList[4];
Microsoft::WRL::ComPtr<ID3D12CommandAllocator> g_commandAllocator[4];
Microsoft::WRL::ComPtr<ID3D12Resource> depthBuffer;
Microsoft::WRL::ComPtr<IDXGISwapChain4> g_swapChain;
Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> g_rtvHeap, g_dsvHeap;
Microsoft::WRL::ComPtr<ID3D12Resource> g_backBuffers[2], dsvBuffer;
Microsoft::WRL::ComPtr<ID3D12Fence> g_fence;
HANDLE g_fenceEvent;
uint64_t g_fenceValue = 0;


// the DxPRT workspace object to control the loading and initialization
DxPRT::Workspace workspace(3);


// global variables used in the workspace
CD3DX12_RECT rect(CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX));
CD3DX12_VIEWPORT viewport(CD3DX12_VIEWPORT(0.0f, 0.0f,
    static_cast<float>(g_clientWidth), static_cast<float>(g_clientHeight)));
int g_iEM = 0;
float g_exposure = 2.0f;
float g_fov = 70.0f; // field of view
unsigned int g_maxL = 8;


// global variables for fps calculation
static std::chrono::high_resolution_clock chronoClock;
float g_frame = 0.0f, g_time = 0.0f, g_TotalTime;
auto g_currentTime = chronoClock.now();
auto g_StartTime = chronoClock.now();

// global vaiarbles for mouse input
bool mouseMoving = false;
POINT mousePreviousPos;
POINT mouseEndPos;
float g_theta = PI / 2.0f, g_phi = 0.0f; // camera angle


bool g_IsInitialized = false;


static TCHAR szWindowClass[] = _T("DesktopApp");
static TCHAR szTitle[] = _T("DxPRT demo");

// declare functions

void InitializePRT();
void UpdateProjectionMatrix();
void UpdateViewMatrix();
void InitializeDirectX();
void InitializeDevice();
void InitializeCommandQueue();
void InitializeCommandAllocator();
void InitializeCommandList();
void InitializeSwapChain();
void InitializeFence();
void InitializeRTVHeap();
void InitializeDSVHeap();
void UpdateRTV();
void UpdateDSV();
void Signal();
void WaitForFence();
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
bool CreateClientWindow(const HINSTANCE& hInstance, const int& nCmdShow);
void ResizeWindow(unsigned int width, unsigned height);
void Update();


//////////////////////////////////////////////////////////////////////////////////

// the render loop

//////////////////////////////////////////////////////////////////////////////////

void Render() {
    if (g_IsInitialized) {


        // intialize command lists
        auto currentBackBuffer = g_swapChain->GetCurrentBackBufferIndex();
        DxPRT_Utility::ThrowIfFailed(g_commandAllocator[currentBackBuffer]->Reset());
        DxPRT_Utility::ThrowIfFailed(g_commandList[currentBackBuffer]->Reset(g_commandAllocator[currentBackBuffer].Get(), nullptr));

        auto backBuffer = g_backBuffers[currentBackBuffer];

        // set the render target
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            backBuffer.Get(), D3D12_RESOURCE_STATE_PRESENT,
            D3D12_RESOURCE_STATE_RENDER_TARGET);
        g_commandList[currentBackBuffer]->ResourceBarrier(1, &barrier);


        // set handles
        D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = g_dsvHeap->GetCPUDescriptorHandleForHeapStart();

        D3D12_CPU_DESCRIPTOR_HANDLE rtvStart = g_rtvHeap->GetCPUDescriptorHandleForHeapStart();
        int incrementSize = g_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvStart, currentBackBuffer, incrementSize);

        // clear the screen
        float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        g_commandList[currentBackBuffer]->ClearDepthStencilView(dsvHandle,
            D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
        g_commandList[currentBackBuffer]->ClearRenderTargetView(rtvHandle,
            clearColor, 0, nullptr);




        // render the prt object and the skybox
        workspace.SetDSVHandle(dsvHandle);
        workspace.SetRTVHandle(rtvHandle);
        workspace.Render(g_commandList[currentBackBuffer].Get());



        // present
        barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            backBuffer.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_PRESENT);
        g_commandList[currentBackBuffer]->ResourceBarrier(1, &barrier);

        DxPRT_Utility::ThrowIfFailed(g_commandList[currentBackBuffer]->Close());

        ID3D12CommandList* const commandLists[] = {
                g_commandList[currentBackBuffer].Get()
        };

        g_commandQueue->ExecuteCommandLists(1, commandLists);

        WaitForFence(); // wait for previous signal (so the next command list is recording while the other is executing)
        Signal();
        g_swapChain->Present(0, 0);
    }
}

///////////////////////////////////////////////////////////////////////////////

// Initialization

/////////////////////////////////////////////////////////////////////////////

/*
* prtThreadFunction: here we call AddPRT to initalize the relevant resources for the
* mesh. This is done on an independent thread
*/
void prtThreadFunction(DxPRT::Workspace& workspace, std::string fileName,
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
    Microsoft::WRL::ComPtr<ID3D12Device5> device) {
    workspace.AddPRT(device.Get(), commandList.Get(), fileName);
}


/*
* emThreadFunction: here we call AddEM to initalize the relevant resources for the
* environment maps. Each map is loaded on an independent thread
*/
void emThreadFunction(DxPRT::Workspace& workspace, std::string EMfileName,
    std::string HDRfileName,
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
    Microsoft::WRL::ComPtr<ID3D12Device5> device, int iEM) {

    workspace.AddEM(device.Get(), commandList.Get(),
        EMfileName, HDRfileName, iEM);

}

void InitializePRT() {

    // set command lists into a recording state
    for (int i = 0; i < 4; ++i) {
        DxPRT_Utility::ThrowIfFailed(g_commandAllocator[i]->Reset());
        DxPRT_Utility::ThrowIfFailed(g_commandList[i]->Reset(g_commandAllocator[i].Get(), nullptr));
    }

    // load the prt and em files on indepdent threads
    std::thread prtThread(prtThreadFunction, std::ref(workspace), "prtFiles/cleanBunnynew.prt",
        g_commandList[0], g_device);
    std::thread emThread1(emThreadFunction, std::ref(workspace), "prtFiles/cleanFieldEM8.prt", "hdrFiles/lilienstein_2k.hdr",
        g_commandList[1].Get(), g_device, 0);
    std::thread emThread2(emThreadFunction, std::ref(workspace), "prtFiles/cleanSnowEM8.prt", "hdrFiles/snowy_cemetery_2k.hdr",
        g_commandList[2].Get(), g_device, 1);
    std::thread emThread3(emThreadFunction, std::ref(workspace), "prtFiles/cleanStudioEMnew.prt", "hdrFiles/photo_studio_loft_hall_2k.hdr",
        g_commandList[3].Get(), g_device, 2);

    prtThread.join();
    emThread1.join();
    emThread2.join();
    emThread3.join();

    for (int i = 0; i < 4; ++i) {
        DxPRT_Utility::ThrowIfFailed(g_commandList[i]->Close());
    }

    ID3D12CommandList* const commandLists[] = {
            g_commandList[0].Get(),
            g_commandList[1].Get(),
            g_commandList[2].Get(),
            g_commandList[3].Get(),
    };

    // execute the command lists. this loads all of the resources onto the GPU
    g_commandQueue->ExecuteCommandLists(4, commandLists);

    Signal();
    WaitForFence();

    workspace.CleanUpCPU(); // relases any data that is still on the CPU as it is no longer neeses

    workspace.Initalize(g_device.Get(), L"Shaders"); // initialize the workspace object (specifically the heaps and pipelines needed)

    // set the initiaial parameters
    workspace.SetModelMatrix(0.0f, -0.1f, 0.0f, 3.0f);
    workspace.SetRect(rect);
    workspace.SetViewport(viewport);
    workspace.SetCurrentEM(g_iEM);
    workspace.SetExposure(g_exposure);
    workspace.SetMaxL(g_maxL);

    UpdateViewMatrix();
    UpdateProjectionMatrix();


}

int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR     lpCmdLine,
    _In_ int       nCmdShow
)
{

    if (!CreateClientWindow(hInstance, nCmdShow)) return 0;

    InitializeDirectX();

    InitializePRT();

    Signal(); // Render loop expects a signal

    g_IsInitialized = true;

    // Main message loop:
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    Signal();// flush
    WaitForFence();
    CloseHandle(g_fenceEvent);

    return (int)msg.wParam;
}


///////////////////////////////////////////////////////////////////////////

// User controls

////////////////////////////////////////////////////////////////////////////


void ResizeWindow(unsigned int width, unsigned height) {

    if ((g_clientWidth != width || g_clientHeight != height) && g_IsInitialized) {

        g_clientWidth = (width < 1u) ? 1u : width;
        g_clientHeight = (height < 1u) ? 1u : height;

        // ensure the resources are not in use

        Signal();
        WaitForFence();

        // reset buffers
        depthBuffer.Reset();
        for (int i = 0; i < 2; ++i) {
            g_backBuffers[i].Reset();
        }

        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        g_swapChain->GetDesc1(&swapChainDesc);
        g_swapChain->ResizeBuffers(2, g_clientWidth,
            g_clientHeight, swapChainDesc.Format,
            swapChainDesc.Flags);

        UpdateRTV();
        UpdateDSV();

        viewport = CD3DX12_VIEWPORT(0.0f, 0.0f,
            static_cast<float>(g_clientWidth),
            static_cast<float>(g_clientHeight));

        workspace.SetViewport(viewport); // set the new viewport with the window dimensions
        UpdateProjectionMatrix();


    }
}

// updates the projection matirx when the camera is zoomed or the window size changes
void UpdateProjectionMatrix() {

    float aspectRatio = (float)(g_clientWidth) / (float)(g_clientHeight);
    DirectX::XMMATRIX projectionMatrix = DirectX::
        XMMatrixPerspectiveFovLH(
            DirectX::XMConvertToRadians(g_fov), aspectRatio, 0.1f, 100.0f);
    workspace.SetProjection(projectionMatrix);
}

// updates the view matrix when the camera angle is changed
void UpdateViewMatrix() {

    const DirectX::XMVECTOR eyePosition = DirectX::XMVectorSet(sin(g_theta) * cos(g_phi),
        cos(g_theta), sin(g_theta) * sin(g_phi), 1);
    const DirectX::XMVECTOR focusPoint = DirectX::XMVectorSet(0, 0, 0, 1);
    const DirectX::XMVECTOR upDirection = DirectX::XMVectorSet(-cos(g_theta) * cos(g_phi),
        sin(g_theta), -cos(g_theta) * sin(g_phi), 0);
    DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(eyePosition, focusPoint, upDirection);
    workspace.SetView(viewMatrix);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_PAINT:  // the render loop
        Update();
        Render();
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_KEYDOWN:
        switch (wParam)
        {
        case VK_LEFT: // change the current environment map
            --g_iEM;
            (g_iEM < 0) ? g_iEM = 2 : g_iEM = g_iEM % 3;
            workspace.SetCurrentEM(g_iEM);
            break;
        case VK_RIGHT:
            ++g_iEM;
            g_iEM = g_iEM % 3;
            workspace.SetCurrentEM(g_iEM);
            break;
        case VK_UP: // change the current exposure
            g_exposure += 0.1;
            workspace.SetExposure(g_exposure);
            break;
        case VK_DOWN:
            if (g_exposure > 0.2) g_exposure -= 0.1;
            workspace.SetExposure(g_exposure);
            break;
        case VK_OEM_PLUS: // change the current number of coefficients used in the caluclaion
            if (g_maxL < 8) ++g_maxL;
            workspace.SetMaxL(g_maxL);
            break;
        case VK_OEM_MINUS:
            if (g_maxL > 0) --g_maxL;
            workspace.SetMaxL(g_maxL);
        default:
            break;
        }
        break;
    case WM_LBUTTONDOWN:
        SetCapture(hWnd);
        GetCursorPos(&mousePreviousPos);
        mouseMoving = true;
        break;
    case WM_LBUTTONUP:
        ReleaseCapture();
        mouseMoving = false;
        break;
    case WM_MOUSEMOVE: // moving the camera
        if (mouseMoving) {
            POINT currentPos;
            GetCursorPos(&currentPos);
            g_theta -= (float)(currentPos.y - mousePreviousPos.y) / 250;
            g_phi -= (float)(currentPos.x - mousePreviousPos.x) / 250;
            mousePreviousPos = currentPos;
            if (g_theta < 0.01) g_theta = 0.01;
            if (g_theta > PI - 0.01) g_theta = PI - 0.01;
            UpdateViewMatrix();
        }
        break;
    case WM_MOUSEWHEEL: // zoom in and out
        g_fov -= (float)GET_WHEEL_DELTA_WPARAM(wParam) / 120.0f;
        if (g_fov < 30) g_fov = 30;
        if (g_fov > 80) g_fov = 80;
        UpdateProjectionMatrix();
        break;
    case WM_SIZE:
        ResizeWindow(LOWORD(lParam), HIWORD(lParam));
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
        break;
    }

    return 0;
}

//////////////////////////////////////////////////////////////////////////

// other functions, no use of DxPRT

//////////////////////////////////////////////////////////////////////////

void Update() {

    g_time += (chronoClock.now() - g_currentTime).count() / pow(10, 9);
    g_currentTime = chronoClock.now();
    g_frame += 1.0f;

    g_TotalTime = (chronoClock.now() - g_StartTime).count() / pow(10, 9);

    char buffer[500];

    if (g_time > 1) {
        sprintf_s(buffer, "fps: %f\n", g_frame / g_time);
        OutputDebugStringA(buffer);
        g_time = 0;
        g_frame = 0;
    }
}

void UpdateDSV() {

    D3D12_CLEAR_VALUE optimizedClearValue = {};
    optimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
    optimizedClearValue.DepthStencil = { 1.0f, 0 };

    D3D12_HEAP_PROPERTIES heap_def =
        CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    D3D12_RESOURCE_DESC heap_desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, g_clientWidth, g_clientHeight,
        1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);


    g_device->CreateCommittedResource(
        &heap_def,
        D3D12_HEAP_FLAG_NONE,
        &heap_desc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &optimizedClearValue,
        IID_PPV_ARGS(&depthBuffer)
    );

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = g_dsvHeap->GetCPUDescriptorHandleForHeapStart();

    g_device->CreateDepthStencilView(depthBuffer.Get(), &dsvDesc,
        cpuHandle);
}

void UpdateRTV() {

    UINT incrementSize = g_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    D3D12_CPU_DESCRIPTOR_HANDLE cpuStart = g_rtvHeap->GetCPUDescriptorHandleForHeapStart();

    for (int i = 0; i < 2; ++i) {

        g_swapChain->GetBuffer(i, IID_PPV_ARGS(&g_backBuffers[i]));

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(cpuStart, i, incrementSize);

        g_device->CreateRenderTargetView(g_backBuffers[i].Get(), nullptr, rtvHandle);

    }

}

void InitializeDirectX() {
    InitializeDevice();
    InitializeCommandQueue();
    InitializeCommandAllocator();
    InitializeCommandList();
    InitializeSwapChain();
    InitializeFence();
    InitializeRTVHeap();
    InitializeDSVHeap();

    UpdateRTV();
    UpdateDSV();
}

void InitializeDevice() {
    Microsoft::WRL::ComPtr<IDXGIFactory5> dxgiFactory5;
    CreateDXGIFactory2(0, IID_PPV_ARGS(&dxgiFactory5));
    D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&g_device));
}

void InitializeCommandQueue() {
    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.NodeMask = 0;

    DxPRT_Utility::ThrowIfFailed(g_device->CreateCommandQueue(&desc,
        IID_PPV_ARGS(&g_commandQueue)));

}

void InitializeCommandAllocator() {
    for (int i = 0; i < 4; ++i) {
        DxPRT_Utility::ThrowIfFailed(g_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(&g_commandAllocator[i])));
    }
}

void InitializeCommandList() {
    for (int i = 0; i < 4; ++i) {
        DxPRT_Utility::ThrowIfFailed(g_device->CreateCommandList(0,
            D3D12_COMMAND_LIST_TYPE_DIRECT, g_commandAllocator[i].Get(),
            nullptr, IID_PPV_ARGS(&g_commandList[i])));
        DxPRT_Utility::ThrowIfFailed(g_commandList[i]->Close());
    }
}

void InitializeSwapChain() {

    Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory4;
    CreateDXGIFactory2(0, IID_PPV_ARGS(&dxgiFactory4));

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = g_clientWidth;
    swapChainDesc.Height = g_clientHeight;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.SampleDesc = { 1, 0 };
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 2;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapChainDesc.Flags = 0;

    Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain1;
    dxgiFactory4->CreateSwapChainForHwnd(
        g_commandQueue.Get(),
        g_hWnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain1);

    swapChain1.As(&g_swapChain);

}

void InitializeRTVHeap() {

    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {
        D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
        2,// 2 buffers used in this demo
        D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
        0
    };

    DxPRT_Utility::ThrowIfFailed(g_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&g_rtvHeap)));

}

void InitializeDSVHeap() {

    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {
        D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
        2, // 2 buffer are used in this demo
        D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
        0
    };

    DxPRT_Utility::ThrowIfFailed(g_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&g_dsvHeap)));

}

void InitializeFence() {

    DxPRT_Utility::ThrowIfFailed(g_device->CreateFence(0, D3D12_FENCE_FLAG_NONE,
        IID_PPV_ARGS(&g_fence)));

    g_fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

}


void Signal() {

    ++g_fenceValue;
    DxPRT_Utility::ThrowIfFailed(g_commandQueue->Signal(g_fence.Get(), g_fenceValue));

}

void WaitForFence() {
    if (g_fence->GetCompletedValue() < g_fenceValue) {
        DxPRT_Utility::ThrowIfFailed(g_fence->SetEventOnCompletion(g_fenceValue, g_fenceEvent));
        DxPRT_Utility::ThrowIfFailed(WaitForSingleObject(g_fenceEvent, INFINITE));
    }

}

bool CreateClientWindow(const HINSTANCE& hInstance, const int& nCmdShow) {

    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(wcex.hInstance, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

    if (!RegisterClassEx(&wcex))
    {
        MessageBox(NULL,
            _T("Call to RegisterClassEx failed!"),
            _T("Windows Desktop Guided Tour"),
            NULL);

        return false;
    }

    g_hWnd = CreateWindowEx(
        WS_EX_OVERLAPPEDWINDOW,
        szWindowClass,
        szTitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        g_clientWidth, g_clientHeight,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (!g_hWnd)
    {
        MessageBox(NULL,
            _T("Call to CreateWindow failed!"),
            _T("Windows Desktop Guided Tour"),
            NULL);

        return false;
    }

    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);

    return true;
}
