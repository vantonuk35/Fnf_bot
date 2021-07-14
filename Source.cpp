#pragma comment(lib,"opencv_world453.lib")
#define USE_UNIFIED_MEM
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include "G:/open222/opencv/build/include/opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <Windows.h>

#include <chrono>
#include <unordered_map>
#include <Atlbase.h>
#include <comdef.h>
#include <memory>
#include <algorithm>
#include <string>
#include <windows.h>
#include <shlobj.h>
#include <future>
#include "ScreenShoter.h"
#include "G:/open222/opencv/build/include/opencv2/core/cuda.hpp"
#include <opencv2/cudawarping.hpp>
#include <vector>
#include <cuda_runtime_api.h>
#ifndef CUDART_VERSION
#error CUDART_VERSION Undefined!
#elif (CUDART_VERSION < 10000)
printf("Oh no, make you you have cuda 10.0 or more !!");
#error Unknown CUDART_VERSION!
#endif
#include <cuda.h>
#include <DXGI.h>  
using namespace std;
using namespace cv;
using namespace dnn;
using namespace cuda;



auto get_time()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()
		);
}



class DetectingArrow
{
	std::vector<std::pair<cv::Scalar, cv::Scalar>> _filters;//First - low_pass ; Second - High_pass
	int _arrow_num;
public:
	DetectingArrow(std::vector<std::pair<cv::Scalar, cv::Scalar>> Filters, int arrow_num)
	{
		_filters = Filters;
		_arrow_num = arrow_num;
	}
	cv::Mat FilterImage(cv::Mat Input, int downscale_factor)
	{
		cv::Rect new_size = { _arrow_num * 160 / downscale_factor,0,160 / downscale_factor ,Input.rows };
		Input = Input(new_size);
		cv::Mat Additional, Output;
		Output = cv::Mat::zeros(Input.rows, Input.cols, CV_8UC1);

		for (auto& Filter : _filters)
		{
			cv::inRange(Input, Filter.first, Filter.second, Additional);
			cv::add(Additional, Output, Output);
		}
		return Output;
	}
};

class ArrowHandler
{
	const enum Arrow_IDs
	{
		ARR_LEFT = VK_LEFT,
		ARR_RIGHT = VK_RIGHT,
		ARR_DOWN = VK_DOWN,
		ARR_UP = VK_UP
	};

	std::unordered_map<int, DetectingArrow> Arrows = {
		{
			ARR_LEFT,
			DetectingArrow({
				{cv::Scalar(147, 147, 194), cv::Scalar(165, 161, 206)},
				{cv::Scalar(150, 85, 165), cv::Scalar(155, 95, 175)}
			},0)
		},
		{
			ARR_RIGHT,
			DetectingArrow({
				{cv::Scalar(172, 187, 243), cv::Scalar(187, 205, 253)},
				{cv::Scalar(170, 125, 200), cv::Scalar(180, 135, 210)}
			},3)
		},
		{
			ARR_UP,
			DetectingArrow({
				{cv::Scalar(56, 236, 246), cv::Scalar(59, 255, 253)},
				{cv::Scalar(60, 175, 210), cv::Scalar(65, 185, 220)}
			},2)
		},
		{
			ARR_DOWN,
			DetectingArrow({
				{cv::Scalar(85, 245, 255), cv::Scalar(95, 255, 255)}, {cv::Scalar(87, 185, 215), cv::Scalar(108, 195, 225)}
			},1)
		}
	};
	int TrashHold;

	void SendOutput(cv::Mat InputMatrix, HWND GameWindow, unsigned char VK)
	{
		SendMessageA(GameWindow, cv::sum(InputMatrix)[0] > TrashHold ? WM_KEYDOWN : WM_KEYUP, VK, 1);
	}

public:
	ArrowHandler(int new_trashhold) : TrashHold(new_trashhold) {};

	void ProcessImage(cv::Mat& Input, HWND gameWindow, int downscale_factor)
	{
		for (auto& [arrow_id, arrow_instance] : Arrows)
		{
			SendOutput(arrow_instance.FilterImage(Input, downscale_factor), gameWindow, arrow_id);
		}
	}
};

void Cut_screenshot_to_arrow_zone(cv::Mat& Input, int downscale_factor, HWND game_window)
{
	cuda::GpuMat; RECT gameRect;
	cuda::GpuMat; GetWindowRect(game_window, &gameRect);
	Input = Input({ gameRect.left, gameRect.top, gameRect.right - gameRect.left, gameRect.bottom - gameRect.top });
    cv::resize(Input, Input,
		{ 1920 / downscale_factor, 1080 / downscale_factor });
	Input = Input({ 1920 / downscale_factor / 2 + int(100 / downscale_factor), int(120 / downscale_factor),
		1920 / downscale_factor / 2 - int(310 / downscale_factor), 1080 / downscale_factor / 4 - int(160 / downscale_factor) });
}
std::vector <IDXGIAdapter*> vAdapters;
std::string WStringToString(const std::wstring& wstr)
{
	std::string str(wstr.length(), ' ');
	std::copy(wstr.begin(), wstr.end(), str.begin());
	return str;
}

int main()
{
	HWND game_window = FindWindowA(NULL, "Friday Night Funkin'");
	DXScreenShoter11 screen_shot_manager;
	screen_shot_manager.Init();

	int down_scale = 8;
	int pixel_reaction_sum = 40000 / down_scale / down_scale;
	ArrowHandler arrow_handler(pixel_reaction_sum);
	system("mode 650");
	SYSTEM_INFO siSysInfo;

	// Copy the hardware information to the SYSTEM_INFO structure. 

	GetSystemInfo(&siSysInfo);

	// Display the contents of the SYSTEM_INFO structure. 

	printf("Hardware information: \n");
	printf("  OEM ID: %u\n", siSysInfo.dwOemId);
	printf("  Number of processors: %u\n",
		siSysInfo.dwNumberOfProcessors);
	printf("  Page size: %u\n", siSysInfo.dwPageSize);
	printf("  Processor type: %u\n", siSysInfo.dwProcessorType);
	printf("  Minimum application address: %lx\n",
		siSysInfo.lpMinimumApplicationAddress);
	printf("  Maximum application address: %lx\n",
		siSysInfo.lpMaximumApplicationAddress);
	printf("  Active processor mask: %u\n",
		siSysInfo.dwActiveProcessorMask);
	// parameter definition  
	IDXGIFactory* pFactory;
	IDXGIAdapter* pAdapter;
	std::vector <IDXGIAdapter*> vAdapters; // Graphics card  


	// Number of graphics cards  
	int iAdapterNum = 0;


	// Create a DXGI factory  
	HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)(&pFactory));

	if (FAILED(hr))
		return -1;

	// enumerate adapter  
	while (pFactory->EnumAdapters(iAdapterNum, &pAdapter) != DXGI_ERROR_NOT_FOUND)
	{
		vAdapters.push_back(pAdapter);
		++iAdapterNum;
	}

	// Information output   
	cout << "================ Get to " << iAdapterNum << "Block graphics card ===============" << endl;
	for (size_t i = 0; i < vAdapters.size(); i++)
	{
		// getting information     
		DXGI_ADAPTER_DESC adapterDesc;
		vAdapters[i]->GetDesc(&adapterDesc);
		wstring aa(adapterDesc.Description);
		std::string bb = WStringToString(aa);
		// Output graphics card information  
		cout << "System video memory:" << adapterDesc.DedicatedSystemMemory / 1024 / 1024 << "M" << endl;
		cout << "Dedicated video memory:" << adapterDesc.DedicatedVideoMemory / 1024 / 1024 << "M" << endl;
		cout << "Shared system memory:" << adapterDesc.SharedSystemMemory / 1024 / 1024 << "M" << endl;
		cout << "Device description:" << bb.c_str() << endl;
		cout << "Device ID:" << adapterDesc.DeviceId << endl;
		cout << "PCI ID revision version:" << adapterDesc.Revision << endl;
		cout << "Subsystem PIC ID:" << adapterDesc.SubSysId << endl;
		cout << "Vendor ID:" << adapterDesc.VendorId << endl;

		// output device  
		IDXGIOutput* pOutput;
		std::vector<IDXGIOutput*> vOutputs;
		// Number of output devices  
		int iOutputNum = 0;
		while (vAdapters[i]->EnumOutputs(iOutputNum, &pOutput) != DXGI_ERROR_NOT_FOUND)
		{
			vOutputs.push_back(pOutput);
			iOutputNum++;
		}

		cout << "-----------------------------------------" << endl;
		cout << "Get to" << iOutputNum << "Display devices:" << endl;
		cout << endl;

		for (size_t n = 0; n < vOutputs.size(); n++)
		{
			// Get display device information  
			DXGI_OUTPUT_DESC outputDesc;
			vOutputs[n]->GetDesc(&outputDesc);

			// Get device support  
			UINT uModeNum = 0;
			DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
			UINT flags = DXGI_ENUM_MODES_INTERLACED;

			vOutputs[n]->GetDisplayModeList(format, flags, &uModeNum, 0);
			DXGI_MODE_DESC* pModeDescs = new DXGI_MODE_DESC[uModeNum];
			vOutputs[n]->GetDisplayModeList(format, flags, &uModeNum, pModeDescs);

			cout << "Display device name:" << outputDesc.DeviceName << endl;
			cout << endl;
		}
		vOutputs.clear();

	}
	vAdapters.clear();
	system("nvcc --version");
	while ((game_window = FindWindowA(NULL, "Friday Night Funkin'")))
	{
		
		cv::Mat src = screen_shot_manager.Take();
		cv::Mat hsv_src;
		Cut_screenshot_to_arrow_zone(src, down_scale, game_window);
		cv::cvtColor(src, hsv_src, cv::COLOR_BGR2HSV);
		arrow_handler.ProcessImage(hsv_src, game_window, down_scale);

	}
	return 0;
}
