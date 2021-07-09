//#pragma comment(lib,"opencv_world452.lib")
#pragma comment(lib,"opencv_world452d.lib")
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "C:/Users/edoua/source/repos/Source/ScreenShoter.h"
class DetectingArrow
{
	std::vector<std::pair<cv::Scalar, cv::Scalar>> _filters;//First - low_pass ; Second - High_pass
public:
	DetectingArrow(std::vector<std::pair<cv::Scalar, cv::Scalar>> Filters)
	{
		_filters = Filters;
	}
	cv::Mat FilterImage(cv::Mat Input)
	{
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
				{cv::Scalar(147, 147, 194), cv::Scalar(165, 161, 206)}, {cv::Scalar(150, 85, 165), cv::Scalar(155, 95, 175)}
			})
		},
		{
			ARR_RIGHT,
			DetectingArrow({
				{cv::Scalar(172, 187, 243), cv::Scalar(187, 205, 253)}, {cv::Scalar(170, 125, 200), cv::Scalar(180, 135, 210)}
			})
		},
		{
			ARR_UP,
			DetectingArrow({
				{cv::Scalar(56, 236, 246), cv::Scalar(59, 255, 253)}, {cv::Scalar(60, 175, 210), cv::Scalar(65, 185, 220)}
			})
		},
		{
			ARR_DOWN,
			DetectingArrow({
				{cv::Scalar(85, 245, 255), cv::Scalar(95, 255, 255)}, {cv::Scalar(87, 185, 215), cv::Scalar(108, 195, 225)}
			})
		}
	};
	int TrashHold;
	void SendOutput(cv::Mat InputMatrix, HWND GameWindow, unsigned char VK)
	{
		SendMessageA(GameWindow, cv::sum(InputMatrix)[0] > TrashHold ? WM_KEYDOWN : WM_KEYUP, VK, 1);
	}
public:
	ArrowHandler(int new_trashhold) : TrashHold(new_trashhold)
	{
	};
	void ProcessImage(cv::Mat Input, HWND gameWindow)
	{
		for (auto& [arrow_id, arrow_instance] : Arrows)
		{
			SendOutput(arrow_instance.FilterImage(Input), gameWindow, arrow_id);
			
		}
	}
};
cv::Mat Cut_screenshot_to_arrow_zone(cv::Mat Input, int downscale_factor, HWND game_window)
{
	RECT gameRect;
	GetWindowRect(game_window, &gameRect);
	Input = Input({ gameRect.left, gameRect.top, gameRect.right - gameRect.left, gameRect.bottom - gameRect.top });
	cv::resize(Input, Input,
		{ 1920 / downscale_factor, 1080 / downscale_factor });
	Input = Input({ 1920 / downscale_factor / 2, 30,
		1920 / downscale_factor / 2, 1080 / downscale_factor / 4 - 50});
	return Input;
}
int main()
{
	HWND game_window = FindWindowA(NULL, "Friday Night Funkin'");
	DXScreenShoter11 screen_shot_manager;
	screen_shot_manager.Init();
	int down_scale = 2;
	int pixel_reaction_sum = 37000;
	ArrowHandler arrow_handler(pixel_reaction_sum);
	while (game_window = FindWindowA(NULL, "Friday Night Funkin'"))
	{
		cv::Mat src = screen_shot_manager.Take();
		cv::Mat hsv_src;
		cv::cvtColor(Cut_screenshot_to_arrow_zone(src, down_scale, game_window), hsv_src, cv::COLOR_BGR2HSV);
		arrow_handler.ProcessImage(hsv_src, game_window);
	}
	return 0;
}
