#pragma once

#include <Windows.h>

#include <string>
#include <vector>
#include <filesystem>

enum class OpenEncode
{
	Ansi = 0,
	Utf16LE = 1,
	Utf16BE = 2,
	Utf8 = 3,
};

enum class SaveEncode
{
	Utf8 = 0,
	Utf8Bom = 1,
	Utf16 = 2, // 默认 UTF16-LE
};

enum class CvtResult
{
	NotFound = -1, // 不在列表
	Success = 0,   // 转换成功
	Ignore = 1,	   // 忽略转换
	Invalid = 2,   // 无效编码
};

/// <summary>转换参数</summary>
struct CvtConf
{
	std::wstring root_folder;				  // 要转换的根目录
	std::vector<std::wstring> extensions;	  // 要转换的扩展名
	std::vector<std::wstring> excludes;		  // 要排除的目录名
	uint32_t code_page{CP_ACP};				  // 当前字符串编码
	bool scan_subfolders{true};				  // 是否扫描子目录
	SaveEncode save_encode{SaveEncode::Utf8}; // 保存编码
	bool skip_unicode{true};				  // 是否跳过UNICODE(即遇到UNICODE不转换)
	bool command_line{false};				  // 是否来自命令行
	std::wstring language;					  // 界面语言
};

class Config
{
	/**
	 * \brief 配置文件名
	 */
	static constexpr auto kFileName = TEXT("utfcvt.ini");
	static constexpr auto kRootSection = TEXT("");

public:
	static bool LoadFromIni(CvtConf &config);
	static bool SaveToIni(const CvtConf &config);
	static int CodePageToIndex(uint32_t code_page);
	static int IndexToCodePage(int index);
};
