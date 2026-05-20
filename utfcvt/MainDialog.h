#pragma once

#include <Windows.h>

#include "Config.h"

class MainDialog
{
public:
	static INT_PTR CALLBACK WindowProc(HWND dlg, UINT msg, WPARAM wp, LPARAM lp);

private:
	static void OnInit(HWND dlg, const CvtConf &config);
	static void OnBrowseClicked(HWND dlg);
	static void OnConvertClicked(HWND dlg, CvtConf config);
	static bool OpenFolder(const std::wstring &folder);
};
