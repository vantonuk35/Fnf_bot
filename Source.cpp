#pragma comment(lib,"opencv_world346.lib")
#pragma optimize("", off)
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <Windows.h>
#include <tchar.h>
#include <chrono>
#include <Atlbase.h>
#include <comdef.h>
#include <memory>
#include <algorithm>
#include <string>
#include <windows.h>
#include <shlobj.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <d3d11.h>
#include <DXGIType.h>
#include <dxgi1_2.h>
#include <processthreadsapi.h>
#include <tlhelp32.h>
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "D3d9.lib")
#pragma comment(lib, "dxgi.lib")
using namespace std;
auto get_time()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()
		);
}
class DXScreenShoter11
{
	HRESULT hr;
	// Supported feature levels.
	IDXGIFactory1* pFactory;
	IDXGIAdapter1* pAdapter;
	IDXGIOutput* pOutput;
	IDXGIOutput1* pOutput1;
	D3D_FEATURE_LEVEL d3dFeatLvl;
	ID3D11Device* pDevice = nullptr;
	ID3D11DeviceContext* pImmediateContext = nullptr;
	ID3D11Texture2D* AcquiredDesktopImage;
	IDXGIResource* DesktopResource = nullptr;
	DXGI_OUTDUPL_FRAME_INFO FrameInfo;
	IDXGIOutputDuplication* pDeskDupl;
	ID3D11DeviceContext* ctx = NULL;
	cv::Mat ExtractBitmap(ID3D11Texture2D* d3dtex, ID3D11Device* pDevice)
	{
		//HRESULT hr;
		//HGDIOBJ hBitmap;
		D3D11_TEXTURE2D_DESC desc;
		ID3D11Texture2D* pNewTexture = NULL;
		D3D11_TEXTURE2D_DESC description;
		d3dtex->GetDesc(&desc);
		d3dtex->GetDesc(&description);
		description.BindFlags = 0;
		description.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		description.Usage = D3D11_USAGE_STAGING;
		description.MiscFlags = 0;
		if (FAILED(pDevice->CreateTexture2D(&description, NULL, &pNewTexture)))
		{
			std::cout << "asfasf";
		}
		ctx->CopyResource(pNewTexture, d3dtex);
		D3D11_MAPPED_SUBRESOURCE resource;
		ctx->Map(pNewTexture, D3D11CalcSubresource(0, 0, 0), D3D11_MAP_READ, 0, &resource);
		// Copy from texture to bitmap buffer.
		cv::Mat Texture = cv::Mat(desc.Height, desc.Width, CV_8UC4);
		for (unsigned int i = 0; i < desc.Height; ++i)
		{
			memcpy(Texture.data + desc.Width * i * 4, (uchar*)resource.pData + desc.Width * i * 4, desc.Width * 4);
		}
		ctx->Unmap(pNewTexture, 0);
		//delete[] sptr;
		pNewTexture->Release();
		return Texture;
	}
public:
	void Init()
	{
		hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)(&pFactory));
		// Get DXGI output.
		hr = pFactory->EnumAdapters1(0, &pAdapter);
		hr = pAdapter->EnumOutputs(0, &pOutput);
		// Query interface for IDXGIOutput1.
		hr = pOutput->QueryInterface(__uuidof(IDXGIOutput1), reinterpret_cast<void**>(&pOutput1));
		hr = D3D11CreateDevice(pAdapter, D3D_DRIVER_TYPE_UNKNOWN,
			0, 0, 0, 0,
			D3D11_SDK_VERSION,
			&pDevice,
			&d3dFeatLvl,
			&pImmediateContext);
		// Create desktop duplication.

		hr = pOutput1->DuplicateOutput(pDevice, &pDeskDupl);
		///////////////////////////////////////////////////////////////////////////
		pDevice->GetImmediateContext(&ctx);
	};
	cv::Mat Take()
	{
		// Get new frame.
		hr = pDeskDupl->AcquireNextFrame(0, &FrameInfo, &DesktopResource);
		if (FAILED(hr))
		{
			return cv::Mat(1080, 1920, CV_8UC4);
		}
		// Query interface for IDXGIResource.
		hr = DesktopResource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&AcquiredDesktopImage));
		DesktopResource->Release();
		cv::Mat hBmp = \
			ExtractBitmap(AcquiredDesktopImage, pDevice);
		hr = pDeskDupl->ReleaseFrame();
		return hBmp;
	};
};
class DetectingArrow
{
	std::array<cv::Scalar, 4> _filters;
public:
	DetectingArrow(std::array<cv::Scalar, 4> Filters)
	{
		_filters = Filters;
	}
	cv::Mat FilterImage(cv::Mat Input)
	{
		cv::Mat Additional, Output;
		cv::inRange(Input, _filters[0], _filters[1], Output);
		cv::inRange(Input, _filters[2], _filters[3], Additional);
		cv::add(Additional, Output, Output);
		return Output;
	}
};
void SendOutput(cv::Mat InputMatrix, int TrashHold, HWND GameWindow, char VK)
{
	SendMessageA(GameWindow, cv::sum(InputMatrix)[0] > TrashHold ? WM_KEYDOWN : WM_KEYUP, VK, 1);
}


int main()
{
	HWND game_window = FindWindowA(NULL, "Friday Night Funkin'");
	DetectingArrow LeftArrow({ cv::Scalar(147,147,194),cv::Scalar(165,161,206),
		cv::Scalar(150,85,165),cv::Scalar(155,95,175) });
	DetectingArrow RightArrow({ cv::Scalar(172,187,243),cv::Scalar(187,205,253),
		cv::Scalar(170,125,200),cv::Scalar(180,135,210) });
	DetectingArrow DownArrow({ cv::Scalar(85,245,255),cv::Scalar(95,255,255)
		,cv::Scalar(87,185,215),cv::Scalar(108,195,225) });
	DetectingArrow UpArrow({ cv::Scalar(56,236,246),cv::Scalar(59,255,253),
	cv::Scalar(60,175,210),cv::Scalar(65,185,220) });
	DXScreenShoter11 screen_shot_manager;
	screen_shot_manager.Init();
	int key;
	cv::namedWindow("output_target");
	int down_scale = 3;
	int pixel_reaction_sum = 25000 / down_scale / down_scale;
	while ((game_window = FindWindowA(NULL, "Friday Night Funkin'")))
	{
		static auto last_time = get_time();
		std::cout << (get_time() - last_time).count() << "\t milliseconds " << '\n';
		last_time = get_time();
		cv::Mat src = screen_shot_manager.Take();
		RECT gameRect;
		GetWindowRect(game_window, &gameRect);
		src = src({ gameRect.left,gameRect.top,gameRect.right - gameRect.left,gameRect.bottom - gameRect.top });
		cv::resize(src, src, { 1920 / down_scale,1080 / down_scale });
		cv::Mat hsv_src;  cv::cvtColor(src, hsv_src, cv::COLOR_BGR2HSV);
		hsv_src = hsv_src({ 1920 / down_scale / 2, 0, 1920 / down_scale / 2, 1080 / down_scale / 4 - 20 });
		cv::Mat PixelsLeft, PixelsRight, PixelsDown, PixelsUp;
		PixelsLeft = LeftArrow.FilterImage(hsv_src);
		PixelsRight = RightArrow.FilterImage(hsv_src);
		PixelsUp = UpArrow.FilterImage(hsv_src);
		PixelsDown = DownArrow.FilterImage(hsv_src);
		cv::Mat AllRes;
		cv::add(PixelsDown, PixelsUp, AllRes);
		cv::add(AllRes, PixelsRight, AllRes);
		cv::add(AllRes, PixelsLeft, AllRes);
		cv::cvtColor(AllRes, AllRes, cv::COLOR_GRAY2BGR);
		cv::add(AllRes, hsv_src, hsv_src);
		std::cout << '\n';
		SendOutput(PixelsLeft, pixel_reaction_sum, game_window, VK_LEFT);
		SendOutput(PixelsDown, pixel_reaction_sum, game_window, VK_DOWN);
		SendOutput(PixelsUp, pixel_reaction_sum, game_window, VK_UP);
		SendOutput(PixelsRight, pixel_reaction_sum, game_window, VK_RIGHT);
		//cv::imshow("output_game", src);
		cv::imshow("output_target", hsv_src);
		key = cv::waitKey(1);
	}
	return 0;
}
