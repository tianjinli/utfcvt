#include "Converter.h"

#include <io.h>
#include <pplawait.h>
#include <tchar.h>

#include <array>
#include <fstream>
#include <filesystem>
#include <ranges>

#include "StringUtils.h"

static constexpr std::array<uint8_t, 3> kUTF8BOM = {0xEF, 0xBB, 0xBF};
static constexpr std::array<uint8_t, 2> kUTF16LE = {0xFF, 0xFE};
static constexpr std::array<uint8_t, 2> kUTF16BE = {0xFE, 0xFF};

Concurrency::task<CvtResult> Converter::ToUnicode(const std::wstring &file, const CvtConf &conf)
{
	std::filesystem::path path(file);
	OutputDebugString((file + TEXT("\n")).c_str());

	bool is_found = false;
	for (const auto &extension : conf.extensions)
	{
		if (StringUtils::Equals(extension, path.extension().wstring()))
		{
			is_found = true;
			break;
		}
	}

	if (!is_found)
	{ // 跳过非列表内扩展名
		co_return CvtResult::NotFound;
	}

	bool with_bom = false;
	std::vector<uint8_t> data; // 源数据
	OpenEncode encoding = OpenEncode::Ansi;
	{
		std::ifstream ifile(path, std::ios::in | std::ios::binary);
		data.assign((std::istreambuf_iterator<char>(ifile)), std::istreambuf_iterator<char>());
	}

	if (data.empty())
	{ // 跳过内容为空的文件
		co_return CvtResult::Ignore;
	}

	if (std::ranges::equal(std::span(data).subspan(0, kUTF16LE.size()), kUTF16LE))
	{ // 跳过小端 UTF16 编码
		encoding = OpenEncode::Utf16LE;
		data.erase(data.begin(), data.begin() + kUTF16LE.size());
	}
	else if (std::ranges::equal(std::span(data).subspan(0, kUTF16BE.size()), kUTF16BE))
	{ // 跳过大端 UTF16 编码
		encoding = OpenEncode::Utf16BE;
		data.erase(data.begin(), data.begin() + kUTF16BE.size());
	}
	else
	{
		if (std::ranges::equal(std::span(data).subspan(0, kUTF8BOM.size()), kUTF8BOM))
		{ // 删除 UTF8 BOM
			with_bom = true;
			data.erase(data.begin(), data.begin() + kUTF8BOM.size());
		}
		if (StringUtils::IsUtf8(std::string_view(reinterpret_cast<const char *>(data.data()), data.size())))
		{
			encoding = OpenEncode::Utf8;
		}
		else
		{
			encoding = OpenEncode::Ansi;
		}
	}

	if (conf.skip_unicode && encoding != OpenEncode::Ansi)
	{ // 非 ANSI 编码且开启跳过
		co_return CvtResult::Ignore;
	}
	bool skip_convert = false;
	if (encoding == OpenEncode::Utf16LE)
	{
		skip_convert = (conf.save_encode == SaveEncode::Utf16);
	}
	else if (encoding == OpenEncode::Utf8)
	{
		skip_convert = (conf.save_encode == SaveEncode::Utf8 && !with_bom) ||
					   (conf.save_encode == SaveEncode::Utf8Bom && with_bom);
	}
	if (skip_convert)
	{
		co_return CvtResult::Ignore;
	}

	if (conf.save_encode == SaveEncode::Utf16)
	{
		std::wstring utf16;
		switch (encoding)
		{
		case OpenEncode::Utf16LE:
			break; // 如果走这里就有问题了
		case OpenEncode::Utf16BE:
		{
			std::wstring_view utf16_in(reinterpret_cast<const wchar_t *>(data.data()), data.size() / sizeof(wchar_t));
			utf16 = StringUtils::Utf16BeToLe(utf16_in);
		}
		break;
		default:
		{
			std::string_view local_in(reinterpret_cast<const char *>(data.data()), data.size());
			auto local_cp = (encoding == OpenEncode::Ansi) ? conf.code_page : CP_UTF8;
			if (!StringUtils::ToUtf16(local_in, utf16, local_cp))
			{
				co_return CvtResult::Invalid;
			}
		}
		break;
		}
		std::ofstream ofile(path, std::ios::out | std::ios::binary | std::ios::trunc);
		ofile.write(reinterpret_cast<const char *>(kUTF16LE.data()), kUTF16LE.size());
		// 写 UTF‑16LE 宽字符数据
		ofile.write(reinterpret_cast<const char *>(utf16.data()), utf16.size() * sizeof(wchar_t));
	}
	else
	{
		std::string utf8;
		switch (encoding)
		{
		case OpenEncode::Utf16LE:
		{
			std::wstring_view utf16(reinterpret_cast<const wchar_t *>(data.data()), data.size() / sizeof(wchar_t));
			if (!StringUtils::ToUtf8(utf16, utf8))
			{
				co_return CvtResult::Invalid;
			}
		}
		break;
		case OpenEncode::Utf16BE:
		{
			std::wstring_view utf16_be(reinterpret_cast<const wchar_t *>(data.data()), data.size() / sizeof(wchar_t));
			auto utf16 = StringUtils::Utf16BeToLe(utf16_be);
			if (!StringUtils::ToUtf8(utf16, utf8))
			{
				co_return CvtResult::Invalid;
			}
		}
		break;
		case OpenEncode::Utf8:
			utf8.assign(reinterpret_cast<const char *>(data.data()), data.size());
			break;
		default:
		{
			std::string_view local_in(reinterpret_cast<const char *>(data.data()), data.size());
			if (!StringUtils::ToUtf8(local_in, utf8, conf.code_page))
			{
				co_return CvtResult::Invalid;
			}
		}
		break;
		}

		std::ofstream ofile(path, std::ios::out | std::ios::binary | std::ios::trunc);
		if (conf.save_encode == SaveEncode::Utf8Bom)
		{
			ofile.write(reinterpret_cast<const char *>(kUTF8BOM.data()), kUTF8BOM.size());
		}
		ofile.write(reinterpret_cast<const char *>(utf8.data()), utf8.size());
	}
	co_return CvtResult::Success;
}

std::generator<std::wstring> Converter::GetFiles(const CvtConf &config)
{
	std::vector<std::wstring> folders{config.root_folder};
	for (size_t i = 0; i < folders.size(); i++)
	{
		auto root = folders[i]; // 这里只能用新的拷贝
		for (const auto &file : EnumFiles(root))
		{
			co_yield (file);
		}
		if (config.scan_subfolders)
		{
			for (const auto &folder : EnumFolders(root))
			{
				bool add_folder = true;
				for (auto &exclude : config.excludes)
				{
					if (StringUtils::Equals(folder, exclude))
					{ // 只需过滤父文件夹
						add_folder = false;
						break;
					}
				}
				if (add_folder)
				{
					folders.emplace_back(folder);
				}
			}
		}
	}
}

std::generator<std::wstring> Converter::EnumFolders(const std::wstring &folder)
{
	_tfinddata64_t find_data;
	const std::wstring search_pattern = folder + TEXT("\\*");
	const intptr_t find_handle = _tfindfirst64(search_pattern.c_str(), &find_data);

	if (find_handle != -1)
	{
		do
		{
			if (find_data.attrib & _A_SUBDIR)
			{
				if (find_data.name[0] != TEXT('.'))
				{ // . 与 .. 首字符均为 .
					co_yield std::move(folder + TEXT("\\") + find_data.name);
				}
			}
		} while (_tfindnext64(find_handle, &find_data) == 0);
		_findclose(find_handle);
	}
}

std::generator<std::wstring> Converter::EnumFiles(const std::wstring &folder)
{
	_tfinddata64_t find_data;
	const std::wstring search_pattern = folder + TEXT("\\*");
	const intptr_t find_handle = _tfindfirst64(search_pattern.c_str(), &find_data);

	if (find_handle != -1)
	{
		do
		{
			if (!(find_data.attrib & _A_SUBDIR))
			{
				co_yield std::move(folder + TEXT("\\") + find_data.name);
			}
		} while (_tfindnext64(find_handle, &find_data) == 0);
		_findclose(find_handle);
	}
}
