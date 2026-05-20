#include "Language.h"

#include <Windows.h>

#include <format>

#include "SystemUtils.h"

Language &Language::GetInstance()
{
	static Language instance;
	return instance;
}

Language::Language() : ini_handler_(std::make_unique<CSimpleIni>())
{
	ini_handler_->SetSpaces(false);
}

Language::~Language() = default;

void Language::LoadFromIni(const std::wstring &lang_code)
{
	if (lang_code.empty()) return;

	std::wstring file_name = std::format(TEXT("lang_{}.ini"), lang_code);
	std::filesystem::path langPath = SystemUtils::GetExecutableFolder() / file_name;

	ini_handler_->Reset();
	ini_handler_->LoadFile(langPath.c_str());
}

std::wstring Language::GetStaticStr(const std::wstring &key, const std::wstring &default_val)
{
    return GetString(TEXT("Static"), key, default_val);
}

std::wstring Language::GetDynamicStr(const std::wstring &key, const std::wstring &default_val)
{
    return GetString(TEXT("Dynamic"), key, default_val);
}

std::wstring Language::GetString(const std::wstring &section, const std::wstring &key, const std::wstring &default_val)
{
	return ini_handler_->GetValue(section.c_str(), key.c_str(), default_val.c_str());
}
