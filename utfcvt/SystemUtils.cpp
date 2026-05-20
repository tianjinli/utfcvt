#include "SystemUtils.h"

#include <Windows.h>

std::filesystem::path SystemUtils::GetExecutableFolder()
{
	wchar_t module_path[MAX_PATH] = TEXT("");
	auto name_count = GetModuleFileName(GetModuleHandle(nullptr), module_path, std::size(module_path));
	if (name_count < 0 || name_count >= MAX_PATH)
	{
		return std::filesystem::current_path();
	}
	return std::filesystem::path(module_path).parent_path();
}
