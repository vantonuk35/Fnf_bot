#pragma once
#include <opencv2/opencv.hpp>
#include <d3d9.h>
#include <d3dx9.h>
#include <d3d11.h>
#include <DXGIType.h>
#include <dxgi1_2.h>
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "D3d9.lib")
#pragma comment(lib, "dxgi.lib")
class DXScreenShoter11
{
	HRESULT hr;
	ID3D11Device* pDevice = nullptr;
	ID3D11DeviceContext* pImmediateContext = nullptr;
	ID3D11Texture2D* AcquiredDesktopImage;
	IDXGIResource* DesktopResource = nullptr;
	IDXGIOutputDuplication* pDeskDupl = nullptr;
	ID3D11DeviceContext* ctx = nullptr;

	cv::Mat ExtractBitmap(ID3D11Texture2D* d3dtex, ID3D11Device* pDevice);

public:
	/*Factory->Adapter->Output->SubOutput
		Get New device
		link subOutput & device to DeskDupl*/
	void Init();
	cv::Mat Take();
};
