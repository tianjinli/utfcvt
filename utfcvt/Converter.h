#pragma once

#include <ppltasks.h>

#include <filesystem>
#include <generator>
#include <string>

#include "Config.h"

class Converter
{
public:
	/// <summary>多字节转UTF8</summary>
	/// <param name="file">文件绝对路径</param>
	/// <param name="config">当前转换配置</param>
	static Concurrency::task<CvtResult> ToUnicode(const std::wstring &file, const CvtConf &config);

	/// <summary>获取所有欲转换文件</summary>
	/// <param name="config">当前转换配置</param>
	static std::generator<std::wstring> GetFiles(const CvtConf &config);

	/// <summary>获取所有欲转换文件</summary>
	/// <param name="root">根目录</param>
	/// <param name="recursive">是否递归</param>
	/// <returns>所有欲转换文件</returns>
	static std::generator<std::filesystem::path> GetFiles(const std::filesystem::path& root, bool recursive);

	/// <summary>枚举欲转换目录</summary>
	/// <param name="folder">当前欲转换目录</param>
	/// <remarks>co_yield 不支持递归调用</remarks>
	static std::generator<std::wstring> EnumFolders(const std::wstring &folder);

	/// <summary>枚举欲转换文件</summary>
	/// <param name="folder">当前欲转换目录</param>
	static std::generator<std::wstring> EnumFiles(const std::wstring &folder);
};
