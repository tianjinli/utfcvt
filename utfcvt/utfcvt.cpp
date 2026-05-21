#include <Windows.h>
#include <tchar.h>
#include <vector>
#include <sstream>

#include "Config.h"
#include "Language.h"
#include "MainDialog.h"
#include "Resource.h"
#include "StringUtils.h"

// 开启系统样式
#if defined _M_IX86
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

#pragma comment(lib, "Shlwapi.lib")

int APIENTRY _tWinMain(_In_ HINSTANCE instance,
					   _In_opt_ HINSTANCE prev_instance,
					   _In_ LPTSTR cmd_line,
					   _In_ int cmd_show)
{
	UNREFERENCED_PARAMETER(prev_instance);
	UNREFERENCED_PARAMETER(cmd_line);
	UNREFERENCED_PARAMETER(cmd_show);

	CvtConf config;
	std::vector<std::wstring> command_args = StringUtils::Split(cmd_line, TEXT("--")); // 空格分隔
	
	Config::LoadFromIni(config);
	auto& lang = Language::GetInstance();
	lang.LoadFromIni(config.language);

	if (!command_args.empty())
	{
		config.command_line = true;

		if (command_args[0] == TEXT("help"))
		{
			std::wstring helper = lang.GetDynamicStr(TEXT("CmdHelp"), TEXT(R"(--folder=C:\\Users	要转换根文件夹名
--extensions=.h|.c	要转换文件扩展名
--excludes=Public	要排除的文件夹名
--codepage=936	要转换文件源编码
--scansub=true	是否搜索子文件夹
--encoding=0	要转换文件目标编码
--skipping=true	是否跳过UNICODE文件
)"));
			MessageBox(nullptr, helper.c_str(), lang.GetDynamicStr(TEXT("CmdHelpTitle"), TEXT("【命令帮助】")).c_str(), MB_ICONASTERISK);
			return 0;
		}
	}

	for (auto &&command_arg : command_args)
	{
		std::wstring name;
		std::wstring value;
		std::wistringstream split(command_arg);
		std::getline(split, name, TEXT('\x3D'));
		std::getline(split, value, TEXT('\x3D'));
		auto it = std::remove(value.begin(), value.end(), TEXT('\x22'));
		value.erase(it, value.end()); // 删除 " 双引号
		if (name == TEXT("folder"))
		{
			config.root_folder = value;
		}
		else if (name == TEXT("extensions"))
		{
			config.extensions = StringUtils::Split(value, TEXT('\x7C'));
		}
		else if (name == TEXT("excludes"))
		{
			config.excludes = StringUtils::Split(value, TEXT('\x7C'));
		}
		else if (name == TEXT("codepage"))
		{
			config.code_page = std::wcstoul(value.c_str(), nullptr, 10);
		}
		else if (name == TEXT("scansub"))
		{
			StringUtils::ToBool(value, config.scan_subfolders);
		}
		else if (name == TEXT("encoding"))
		{
			config.save_encode = SaveEncode(_wtoi(value.c_str()));
		}
		else if (name == TEXT("skipping"))
		{
			StringUtils::ToBool(value, config.skip_unicode);
		}
		else
		{
			MessageBox(nullptr, command_arg.c_str(), lang.GetDynamicStr(TEXT("CmdInvalidTitle"), TEXT("【无效命令】")).c_str(), MB_ICONHAND);
			return -1;
		}
	}

	LoadLibraryW(L"Msftedit.dll");
	::OleInitialize(nullptr);
	DialogBoxParam(instance, MAKEINTRESOURCE(IDD_TRANSCODER), NULL, MainDialog::WindowProc, (LPARAM)&config);
	::OleUninitialize();
	return 0;
}
