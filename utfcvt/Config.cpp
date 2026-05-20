#include "Config.h"

#include "SimpleIni.h"
#include "StringUtils.h"
#include "SystemUtils.h"

int Config::CodePageToIndex(uint32_t code_page)
{
	switch (code_page)
	{
	case 936:
		return 1;
	case 950:
		return 2;
	default:
		return 0;
	}
}

int Config::IndexToCodePage(int index)
{
	switch (index)
	{
	case 1:
		return 936;
	case 2:
		return 950;
	default:
		return CP_ACP;
	}
}

bool Config::LoadFromIni(CvtConf &config)
{
	CSimpleIni ini_handler;
	auto config_path = SystemUtils::GetExecutableFolder() / kFileName;
	if (ini_handler.LoadFile(config_path.c_str()) != SI_OK)
	{
		config.extensions = {TEXT(".txt"), TEXT(".doc")};
		config.language = TEXT("zh-CN"); // 默认语言
		return false;
	}
	ini_handler.SetSpaces(false);
	config.command_line = false;
	std::wstring extensions = ini_handler.GetValue(kRootSection, TEXT("extensions"), TEXT(""));
	std::wstring excludes = ini_handler.GetValue(kRootSection, TEXT("excludes"), TEXT(""));
	config.root_folder = ini_handler.GetValue(kRootSection, TEXT("root_folder"), TEXT(""));
	config.extensions = StringUtils::Split(extensions, TEXT('|'));
	config.excludes = StringUtils::Split(excludes, TEXT('|'));
	config.code_page = ini_handler.GetLongValue(kRootSection, TEXT("code_page"), CP_ACP);
	config.scan_subfolders = ini_handler.GetBoolValue(kRootSection, TEXT("scan_subfolders"), true);
	config.save_encode = SaveEncode(ini_handler.GetLongValue(kRootSection, TEXT("save_encode"), static_cast<int>(SaveEncode::Utf8)));
	config.skip_unicode = ini_handler.GetBoolValue(kRootSection, TEXT("skip_unicode"), false);
	config.language = ini_handler.GetValue(kRootSection, TEXT("language"), TEXT("zh-CN"));
	return true;
}

bool Config::SaveToIni(const CvtConf &config)
{
	if (config.command_line)
	{
		return true;
	}
	CSimpleIni ini_handler;
	ini_handler.SetSpaces(false);
	ini_handler.SetValue(kRootSection, TEXT("root_folder"), config.root_folder.c_str());
	ini_handler.SetValue(kRootSection, TEXT("extensions"), StringUtils::Join(config.extensions, TEXT('|')).c_str());
	ini_handler.SetValue(kRootSection, TEXT("excludes"), StringUtils::Join(config.excludes, TEXT('|')).c_str());
	ini_handler.SetLongValue(kRootSection, TEXT("code_page"), config.code_page);
	ini_handler.SetBoolValue(kRootSection, TEXT("scan_subfolders"), config.scan_subfolders);
	ini_handler.SetLongValue(kRootSection, TEXT("save_encode"), static_cast<int>(config.save_encode));
	ini_handler.SetBoolValue(kRootSection, TEXT("skip_unicode"), config.skip_unicode);
	ini_handler.SetValue(kRootSection, TEXT("language"), config.language.c_str());
	auto config_path = SystemUtils::GetExecutableFolder() / kFileName;
	return ini_handler.SaveFile(config_path.c_str()) != SI_OK;
}
