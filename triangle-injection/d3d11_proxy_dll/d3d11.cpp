#pragma once
#include <Windows.h>
#include "debug.h"
#include <stdint.h>
#include <d3dcompiler.h>
#include <d3d11.h>
#include <d3d11_4.h>
#include <shlwapi.h>
#include "hooking.h"
#include <DirectXMath.h>
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <chrono>
#include <string> 

#pragma comment (lib, "Shlwapi.lib") //for PathRemoveFileSpecA
#pragma comment(lib, "d3dcompiler.lib")


typedef HRESULT(__stdcall* fn_D3D11CreateDeviceAndSwapChain)(
	IDXGIAdapter*,
	D3D_DRIVER_TYPE,
	HMODULE,
	UINT,
	const D3D_FEATURE_LEVEL*,
	UINT,
	UINT,
	const DXGI_SWAP_CHAIN_DESC*,
	IDXGISwapChain**,
	ID3D11Device**,
	D3D_FEATURE_LEVEL*,
	ID3D11DeviceContext**);

typedef HRESULT(__stdcall* fn_D3D11CreateDevice)(
	IDXGIAdapter*,
	D3D_DRIVER_TYPE,
	HMODULE,
	UINT,
	const D3D_FEATURE_LEVEL*,
	UINT,
	UINT,
	ID3D11Device**,
	D3D_FEATURE_LEVEL*,
	ID3D11DeviceContext**);


typedef HRESULT(__stdcall* fn_DXGISwapChain_Present)(IDXGISwapChain*, UINT, UINT);

IDXGISwapChain* swapChain = nullptr;
ID3D11Device5* device = nullptr;
ID3D11DeviceContext4* devCon = nullptr;
ID3D10Blob* vs_blob = nullptr;
ID3D11VertexShader* vs = nullptr;
ID3D10Blob* ps_blob = nullptr;
ID3D11PixelShader* ps = nullptr;
ID3D10Blob* bilinearInterpolation_ps_blob = nullptr;
ID3D10Blob* topLeftQuadrant_ps_blob = nullptr;
ID3D11PixelShader* billinearInterpolation_ps = nullptr;
ID3D11PixelShader* topLeftQuadrant_ps = nullptr;
ID3D11Buffer* vertex_buffer = nullptr;
ID3D11Buffer* index_buffer = nullptr;
ID3D11InputLayout* vertLayout = nullptr;
ID3D11RasterizerState* SolidRasterState = nullptr;
ID3D11DepthStencilState* SolidDepthStencilState = nullptr;
ID3D11ShaderResourceView* g_bbSrv = nullptr;	// backbuffer's SRV.
ID3D11Texture2D* g_tempTexture = nullptr;
ID3D11RenderTargetView* g_tempTextureRtv = nullptr;
ID3D11ShaderResourceView* g_tempTextureSrv = nullptr;
ID3D11SamplerState* g_samplerState = nullptr;
DirectX::SpriteBatch* g_spriteBatch = nullptr;
DirectX::SpriteFont* g_spriteFont = nullptr;

auto lastTime = std::chrono::high_resolution_clock::now();
int lastFps;
double fpsTimePassed = 0.0;
double fpsTimePassMax = 1.0;	// пропускать одну секунду перед обновлением счётчика кадров.


struct VertexData
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT2 Tex;
};

int GetFrameTime()
{
	auto currentTime = std::chrono::high_resolution_clock::now();
	auto elapsed = std::chrono::duration<double, std::milli>(currentTime - lastTime);

	double dt = elapsed.count() * 0.001f;

	lastTime = currentTime;
	fpsTimePassed += dt;

	if (fpsTimePassed > fpsTimePassMax)
	{
		fpsTimePassed = 0.0;
		lastFps = 1 / dt;
	}

	return lastFps;
}

HRESULT DXGISwapChain_Present_Hook(IDXGISwapChain* thisPtr, UINT SyncInterval, UINT Flags)
{
	ID3D11RenderTargetView* rtvs_Orig[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = { nullptr };
	ID3D11DepthStencilView* depthStencilView_Orig = nullptr;
	devCon->OMGetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, rtvs_Orig, &depthStencilView_Orig);
	devCon->OMSetRenderTargets(1, &g_tempTextureRtv, NULL);
	devCon->VSSetShader(vs, 0, 0);
	devCon->PSSetShader(billinearInterpolation_ps, 0, 0);
	devCon->PSSetShaderResources(0, 1, &g_bbSrv);
	devCon->PSSetSamplers(0, 1, &g_samplerState);
	devCon->IASetInputLayout(vertLayout);
	devCon->RSSetState(SolidRasterState);
	devCon->OMSetDepthStencilState(SolidDepthStencilState, 0);

	UINT stride = sizeof(VertexData);
	UINT offset = 0;
	devCon->IASetVertexBuffers(0, 1, &vertex_buffer, &stride, &offset);
	devCon->IASetIndexBuffer(index_buffer, DXGI_FORMAT_R32_UINT, 0);
	devCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	devCon->DrawIndexed(6, 0, 0);


	devCon->OMSetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, rtvs_Orig, depthStencilView_Orig);
	devCon->VSSetShader(vs, 0, 0);
	devCon->PSSetShader(topLeftQuadrant_ps, 0, 0);
	devCon->PSSetShaderResources(0, 1, &g_tempTextureSrv);
	devCon->PSSetSamplers(0, 1, &g_samplerState);
	devCon->IASetInputLayout(vertLayout);
	devCon->RSSetState(SolidRasterState);
	devCon->OMSetDepthStencilState(SolidDepthStencilState, 0);

	devCon->IASetVertexBuffers(0, 1, &vertex_buffer, &stride, &offset);
	devCon->IASetIndexBuffer(index_buffer, DXGI_FORMAT_R32_UINT, 0);
	devCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	devCon->DrawIndexed(6, 0, 0);


	int fps = GetFrameTime();
	ID3D11BlendState* blendState_Orig;
	float blendFactor_Orig[4];
	UINT sampleMask_Orig;
	devCon->OMGetBlendState(&blendState_Orig, blendFactor_Orig, &sampleMask_Orig);	// We need to store blending, because DrawString overrides it.
	g_spriteBatch->Begin();
	g_spriteFont->DrawString(g_spriteBatch, std::to_string(fps).c_str(), DirectX::XMFLOAT2(1, 1), DirectX::Colors::Red, 0.0f);
	g_spriteBatch->End();
	devCon->OMSetBlendState(blendState_Orig, blendFactor_Orig, sampleMask_Orig);


	fn_DXGISwapChain_Present DXGISwapChain_Present_Orig;
	PopAddress(uint64_t(&DXGISwapChain_Present_Orig));

	HRESULT r = DXGISwapChain_Present_Orig(thisPtr, SyncInterval, Flags);
	return r;
}


void LoadShaders()
{
#if defined(_DEBUG)
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif
	{
		char filepath[512];
		HMODULE hModule = GetModuleHandle(NULL);
		GetModuleFileNameA(hModule, filepath, 512);
		PathRemoveFileSpecA(filepath);

		strcat_s(filepath, 512, "\\hook_content\\passthrough_vs.shader");

		wchar_t wPath[513];
		size_t outSize;

		mbstowcs_s(&outSize, &wPath[0], strlen(filepath) + 1, filepath, strlen(filepath));
		ID3D10Blob* compileErrors = nullptr;

		HRESULT err = D3DCompileFromFile(wPath, 0, 0, "main", "vs_5_0", compileFlags, 0, &vs_blob, &compileErrors);
		if (compileErrors != nullptr && compileErrors)
		{
			ID3D10Blob* outErrorsDeref = compileErrors;
			OutputDebugStringA((char*)compileErrors->GetBufferPointer());
		}

		err = device->CreateVertexShader(vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), NULL, &vs);
		check(err == S_OK);
	}
	{
		char filepath[512];
		HMODULE hModule = GetModuleHandle(NULL);
		GetModuleFileNameA(hModule, filepath, 512);
		PathRemoveFileSpecA(filepath);

		strcat_s(filepath, 512, "\\hook_content\\bilinearInterpolation_ps.shader");

		wchar_t wPath[513];
		size_t outSize;

		mbstowcs_s(&outSize, &wPath[0], strlen(filepath) + 1, filepath, strlen(filepath));
		ID3D10Blob* compileErrors;

		HRESULT err = D3DCompileFromFile(wPath, 0, 0, "main", "ps_5_0", compileFlags, 0, &bilinearInterpolation_ps_blob, &compileErrors);
		if (compileErrors != nullptr && compileErrors)
		{
			ID3D10Blob* outErrorsDeref = compileErrors;
			OutputDebugStringA((char*)compileErrors->GetBufferPointer());
		}

		err = device->CreatePixelShader(bilinearInterpolation_ps_blob->GetBufferPointer(), bilinearInterpolation_ps_blob->GetBufferSize(), NULL, &billinearInterpolation_ps);
		check(err == S_OK);
	}
	{
		char filepath[512];
		HMODULE hModule = GetModuleHandle(NULL);
		GetModuleFileNameA(hModule, filepath, 512);
		PathRemoveFileSpecA(filepath);

		strcat_s(filepath, 512, "\\hook_content\\topLeftQuadrant_ps.shader");

		wchar_t wPath[513];
		size_t outSize;

		mbstowcs_s(&outSize, &wPath[0], strlen(filepath) + 1, filepath, strlen(filepath));
		ID3D10Blob* compileErrors;

		HRESULT err = D3DCompileFromFile(wPath, 0, 0, "main", "ps_5_0", compileFlags, 0, &topLeftQuadrant_ps_blob, &compileErrors);
		if (compileErrors != nullptr && compileErrors)
		{
			ID3D10Blob* outErrorsDeref = compileErrors;
			OutputDebugStringA((char*)compileErrors->GetBufferPointer());
		}

		err = device->CreatePixelShader(topLeftQuadrant_ps_blob->GetBufferPointer(), topLeftQuadrant_ps_blob->GetBufferSize(), NULL, &topLeftQuadrant_ps);
		check(err == S_OK);
	}
	{
		char filepath[512];
		HMODULE hModule = GetModuleHandle(NULL);
		GetModuleFileNameA(hModule, filepath, 512);
		PathRemoveFileSpecA(filepath);

		strcat_s(filepath, 512, "\\hook_content\\vertex_color_ps.shader");

		wchar_t wPath[513];
		size_t outSize;

		mbstowcs_s(&outSize, &wPath[0], strlen(filepath) + 1, filepath, strlen(filepath));
		ID3D10Blob* compileErrors;

		HRESULT err = D3DCompileFromFile(wPath, 0, 0, "main", "ps_5_0", compileFlags, 0, &ps_blob, &compileErrors);
		if (compileErrors != nullptr && compileErrors)
		{
			ID3D10Blob* outErrorsDeref = compileErrors;
			OutputDebugStringA((char*)compileErrors->GetBufferPointer());
		}

		err = device->CreatePixelShader(ps_blob->GetBufferPointer(), ps_blob->GetBufferSize(), NULL, &ps);
		check(err == S_OK);
	}
}


void CreateMesh()
{
	using namespace DirectX;

	const VertexData vertData[] =
	{
		VertexData { XMFLOAT3(-1, -1, 0.1), XMFLOAT2(0, 1) },
		VertexData { XMFLOAT3(1,  1, 0.1), XMFLOAT2(1, 0) },
		VertexData { XMFLOAT3(-1,  1, 0.1), XMFLOAT2(0, 0) },
		VertexData { XMFLOAT3(1, -1, 0.1), XMFLOAT2(1, 1) }
	};


	D3D11_BUFFER_DESC vertBufferDesc;
	ZeroMemory(&vertBufferDesc, sizeof(vertBufferDesc));
	vertBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertBufferDesc.ByteWidth = sizeof(VertexData) * 4;
	vertBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertBufferDesc.CPUAccessFlags = 0;
	vertBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vertBufferData;
	ZeroMemory(&vertBufferData, sizeof(vertBufferData));
	vertBufferData.pSysMem = vertData;

	HRESULT res = device->CreateBuffer(&vertBufferDesc, &vertBufferData, &vertex_buffer);
	check(res == S_OK);

	// Indices:
	const uint32_t indices[] =
	{
		0, 1, 2,
		0, 1, 3
	};

	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(uint32_t) * 6;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA indexBufferData;
	ZeroMemory(&indexBufferData, sizeof(indexBufferData));
	indexBufferData.pSysMem = indices;

	res = device->CreateBuffer(&indexBufferDesc, &indexBufferData, &index_buffer);
	check(res == S_OK);
}


void CreateInputLayout()
{
	D3D11_INPUT_ELEMENT_DESC vertElements[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	HRESULT err = device->CreateInputLayout(vertElements, _countof(vertElements), vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), &vertLayout);
	check(err == S_OK);
}

void CreateRasterizerAndDepthStates()
{
	D3D11_RASTERIZER_DESC soliddesc;
	ZeroMemory(&soliddesc, sizeof(D3D11_RASTERIZER_DESC));
	soliddesc.FillMode = D3D11_FILL_SOLID;
	soliddesc.CullMode = D3D11_CULL_NONE;
	HRESULT err = device->CreateRasterizerState(&soliddesc, &SolidRasterState);
	check(err == S_OK);

	D3D11_DEPTH_STENCIL_DESC depthDesc;
	ZeroMemory(&depthDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	depthDesc.DepthEnable = true;
	depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
	err = device->CreateDepthStencilState(&depthDesc, &SolidDepthStencilState);
	check(err == S_OK);
}

void CreateSRVFromBackBuffer()
{
	ID3D11Texture2D* backBuffer;
	HRESULT hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
	hr = device->CreateShaderResourceView(backBuffer, NULL, &g_bbSrv);

	D3D11_TEXTURE2D_DESC tempTextureDesc;
	backBuffer->GetDesc(&tempTextureDesc);
	tempTextureDesc.Width *= 2;
	tempTextureDesc.Height *= 2;
	hr = device->CreateTexture2D(&tempTextureDesc, NULL, &g_tempTexture);
	check(hr == S_OK);
	hr = device->CreateRenderTargetView(g_tempTexture, NULL, &g_tempTextureRtv);
	check(hr == S_OK);
	hr = device->CreateShaderResourceView(g_tempTexture, NULL, &g_tempTextureSrv);
	check(hr == S_OK);


	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
	// TODO: change?
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 1;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	hr = device->CreateSamplerState(&samplerDesc, &g_samplerState);
}

void InitFonts()
{
	g_spriteBatch = new DirectX::SpriteBatch(devCon);
	g_spriteFont = new DirectX::SpriteFont(device, L"C:\\myfont.font");
}

fn_D3D11CreateDeviceAndSwapChain LoadD3D11AndGetOriginalFuncPointer()
{
	char path[MAX_PATH];
	if (!GetSystemDirectoryA(path, MAX_PATH)) return nullptr;

	strcat_s(path, MAX_PATH * sizeof(char), "\\d3d11.dll");
	HMODULE d3d_dll = LoadLibraryA(path);

	if (!d3d_dll)
	{
		MessageBox(NULL, TEXT("Could Not Locate Original D3D11 DLL"), TEXT("Darn"), 0);
		return nullptr;
	}

	return (fn_D3D11CreateDeviceAndSwapChain)GetProcAddress(d3d_dll, TEXT("D3D11CreateDeviceAndSwapChain"));
}

fn_D3D11CreateDevice LoadD3D11AndGetOriginalFuncPointer2()
{
	char path[MAX_PATH];
	if (!GetSystemDirectoryA(path, MAX_PATH)) return nullptr;

	strcat_s(path, MAX_PATH * sizeof(char), "\\d3d11.dll");
	HMODULE d3d_dll = LoadLibraryA(path);

	if (!d3d_dll)
	{
		MessageBox(NULL, TEXT("Could Not Locate Original D3D11 DLL"), TEXT("Darn"), 0);
		return nullptr;
	}

	return (fn_D3D11CreateDevice)GetProcAddress(d3d_dll, TEXT("D3D11CreateDevice"));
}

inline void** get_vtable_ptr(void* obj)
{
	return *reinterpret_cast<void***>(obj);
}

extern "C" HRESULT __stdcall D3D11CreateDeviceAndSwapChain(
	IDXGIAdapter * pAdapter,
	D3D_DRIVER_TYPE            DriverType,
	HMODULE                    Software,
	UINT                       Flags,
	const D3D_FEATURE_LEVEL * pFeatureLevels,
	UINT                       FeatureLevels,
	UINT                       SDKVersion,
	const DXGI_SWAP_CHAIN_DESC * pSwapChainDesc,
	IDXGISwapChain * *ppSwapChain,
	ID3D11Device * *ppDevice,
	D3D_FEATURE_LEVEL * pFeatureLevel,
	ID3D11DeviceContext * *ppImmediateContext
)
{
	//uncomment if you need to debug an issue in a project you aren't launching from VS
	//this gives you an easy way to make sure you can attach a debugger at the right time
	//MessageBox(NULL, TEXT("Calling D3D11CreateDeviceAndSwapChain"), TEXT("Ok"), 0);

	fn_D3D11CreateDeviceAndSwapChain D3D11CreateDeviceAndSwapChain_Orig = LoadD3D11AndGetOriginalFuncPointer();

	DXGI_SWAP_CHAIN_DESC newSwapChainDesc = *pSwapChainDesc;
	newSwapChainDesc.BufferUsage |= DXGI_USAGE_SHADER_INPUT;
	HRESULT res = D3D11CreateDeviceAndSwapChain_Orig(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, &newSwapChainDesc, ppSwapChain, ppDevice, pFeatureLevel, ppImmediateContext);

	HRESULT hr = (*ppDevice)->QueryInterface(__uuidof(ID3D11Device5), (void**)&device);
	hr = (*ppImmediateContext)->QueryInterface(__uuidof(ID3D11DeviceContext), (void**)&devCon);

	LoadShaders();
	CreateMesh();
	CreateInputLayout();
	CreateRasterizerAndDepthStates();

	swapChain = *ppSwapChain;
	void** swapChainVTable = get_vtable_ptr(swapChain);

	InstallHook(swapChainVTable[8], DXGISwapChain_Present_Hook);
	//present is [8];

	CreateSRVFromBackBuffer();
	InitFonts();

	return res;
}

extern "C" HRESULT __stdcall D3D11CreateDevice(
	IDXGIAdapter * pAdapter,
	D3D_DRIVER_TYPE            DriverType,
	HMODULE                    Software,
	UINT                       Flags,
	const D3D_FEATURE_LEVEL * pFeatureLevels,
	UINT                       FeatureLevels,
	UINT                       SDKVersion,
	ID3D11Device * *ppDevice,
	D3D_FEATURE_LEVEL * pFeatureLevel,
	ID3D11DeviceContext * *ppImmediateContext
)
{
	fn_D3D11CreateDevice D3D11CreateDevice_Orig = LoadD3D11AndGetOriginalFuncPointer2();

	HRESULT res = D3D11CreateDevice_Orig(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, ppDevice, pFeatureLevel, ppImmediateContext);
	return res;
}
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD ul_reason_for_call, LPVOID lpvReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		MessageBox(NULL, TEXT("Target app has loaded your proxy d3d11.dll and called DllMain. If you're launching Skyrim via steam, you need to dismiss this popup quickly, otherwise you get a load error"), TEXT("Success"), 0);
	}

	return true;
}
