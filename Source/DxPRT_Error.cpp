/*
*
* Implimentation of the ThrowifFailed function (see DxPRT_Error.h)
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

#include "DxPRT/DxPRT_Error.h"

namespace DxPRT_Utility {

	void ThrowIfFailed(const HRESULT& res) {

		switch (res) {
		case S_OK:
			break;
		case D3D12_ERROR_ADAPTER_NOT_FOUND:
			OutputDebugStringA("DxPRT: DirextX Error Code: D3D12_ERROR_ADAPTER_NOT_FOUND\n");
			throw std::exception();
			break;
		case D3D12_ERROR_DRIVER_VERSION_MISMATCH:
			OutputDebugStringA("DxPRT: DirextX Error Code: D3D12_ERROR_DRVIER_VERSION_MISMATCH\n");
			throw std::exception();
			break;
		case DXGI_ERROR_INVALID_CALL:
			OutputDebugStringA("DxPRT: DirextX Error Code: DXGI_ERROR_INVALID_CALL\n");
			throw std::exception();
			break;
		case DXGI_ERROR_WAS_STILL_DRAWING:
			OutputDebugStringA("DxPRT: DirextX Error Code: DXGI_ERROR_WAS_STILL_DRAWING\n");
			throw std::exception();
			break;
		case E_FAIL:
			OutputDebugStringA("DxPRT: DirextX Error Code: E_FAIL\n");
			throw std::exception();
			break;
		case E_INVALIDARG:
			OutputDebugStringA("DxPRT: DirextX Error Code: E_INVALIDARG\n");
			throw std::exception();
			break;
		case E_OUTOFMEMORY:
			OutputDebugStringA("DxPRT: DirextX Error Code: E_OUTOFMEMORY\n");
			throw std::exception();
			break;
		case E_NOTIMPL:
			OutputDebugStringA("DxPRT: DirextX Error Code: E_NOTIMPL\n");
			throw std::exception();
			break;
		case S_FALSE:
			OutputDebugStringA("DxPRT: DirextX Error Code: S_FALSE\n");
			throw std::exception();
			break;
		default:
			OutputDebugStringA("DxPRT: DirextX Error Code: UNKNOWN\n");
			throw std::exception();
			break;
		}
	}

}