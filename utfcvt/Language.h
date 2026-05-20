#pragma once

#include <filesystem>
#include <memory>
#include <string>

#include "SimpleIni.h"

class Language
{
public:
	static Language &GetInstance();

	void LoadFromIni(const std::wstring &lang_code);

	std::wstring GetStaticStr(const std::wstring &key, const std::wstring &default_val);

	std::wstring GetDynamicStr(const std::wstring &key, const std::wstring &default_val);

private:
	Language();
	~Language();

	/**
	 * @brief 获取语言字符串
	 * 
	 * @param section 配置节
	 * @param key 配置键
	 * @param default_val 默认值
	 * @return std::wstring 语言字符串
	 */
	std::wstring GetString(const std::wstring &section, const std::wstring &key, const std::wstring &default_val);

	std::unique_ptr<CSimpleIni> ini_handler_;
};
