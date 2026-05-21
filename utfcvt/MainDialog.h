#pragma once

#include <Windows.h>

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
	static bool OpenFolder(const std::wstring &folder);

	static void AppendText(HWND rich, const std::wstring& text, COLORREF color = RGB(0, 0, 0), bool newline = true, bool scroll = true);

	static constexpr uint32_t kRedColor = RGB(255, 0, 0);
	static constexpr uint32_t kGreenColor = RGB(0, 255, 0);
	static constexpr uint32_t kOrangeColor = RGB(255, 128, 0);
};
