#pragma once

#include <Windows.h>

#include <functional>
#include <set>
#include <ranges>

#include "Config.h"

class MainDialog
{
public:
	static INT_PTR CALLBACK WindowProc(HWND dlg, UINT msg, WPARAM wp, LPARAM lp);

private:
	static void OnInit(HWND dlg, const CvtConf &config);
	static void OnCommand(HWND dlg, WORD id, HWND control);
	static void OnBrowseClicked(HWND dlg);
	static void OnConvertClicked(HWND dlg, CvtConf config);
	static void OnGuessClicked(HWND dlg, std::wstring_view folder);
	static bool OpenFolder(std::wstring_view folder);

	static void AppendText(HWND rich, std::wstring_view text, COLORREF color = RGB(0, 0, 0), bool newline = true, bool scroll = true);

	static constexpr uint32_t kRedColor = RGB(255, 0, 0);
	static constexpr uint32_t kGreenColor = RGB(0, 255, 0);
	static constexpr uint32_t kOrangeColor = RGB(255, 128, 0);
};
