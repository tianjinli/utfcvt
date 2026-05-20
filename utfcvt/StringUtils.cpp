#include "StringUtils.h"

#include <Windows.h>

#include <algorithm>
#include <cctype>
#include <codecvt>
#include <regex>
#include <sstream>

bool StringUtils::IsUtf8(const std::string_view string_in)
{
	int len = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, string_in.data(), static_cast<int>(string_in.size()), nullptr, 0);
	return len > 0 || string_in.empty();
}

bool StringUtils::ToUtf8(const std::string_view local_in, std::string &utf8_out, const uint32_t local_cp)
{
	int u16_size = MultiByteToWideChar(local_cp, MB_ERR_INVALID_CHARS, local_in.data(), int(local_in.size()), nullptr, 0);
	if (u16_size > 0)
	{
		const std::wstring utf16(u16_size, 0);
		u16_size = MultiByteToWideChar(local_cp, MB_ERR_INVALID_CHARS, local_in.data(), int(local_in.size()), const_cast<wchar_t *>(utf16.data()), int(utf16.size()));
		if (u16_size > 0)
		{
			int u8_size = WideCharToMultiByte(CP_UTF8, 0, utf16.data(), int(utf16.size()), nullptr, 0, nullptr, nullptr);
			if (u8_size > 0)
			{
				const std::string utf8(u8_size, 0);
				u8_size = WideCharToMultiByte(CP_UTF8, 0, utf16.data(), int(utf16.size()), const_cast<char *>(utf8.data()), int(utf8.size()), nullptr, nullptr);
				if (local_cp == CP_UTF8)
				{
					return (u8_size == int(local_in.size())) ? (utf8_out = local_in), true : false;
				}
				return (u8_size == int(utf8.size())) ? ((utf8_out = utf8), true) : false;
			}
		}
	}
	return false; // 在这里可以抛异常
}

bool StringUtils::ToUtf8(const std::wstring_view utf16_in, std::string &utf8_out)
{
	int u8_size = WideCharToMultiByte(CP_UTF8, 0, utf16_in.data(), int(utf16_in.size()), nullptr, 0, nullptr, nullptr);
	if (u8_size > 0)
	{
		const std::string utf8(u8_size, 0);
		u8_size = WideCharToMultiByte(CP_UTF8, 0, utf16_in.data(), int(utf16_in.size()), const_cast<char *>(utf8.data()), int(utf8.size()), nullptr, nullptr);

		return (u8_size == int(utf8.size())) ? ((utf8_out = utf8), true) : false;
	}
	return false; // 在这里可以抛异常
}

bool StringUtils::ToUtf16(const std::string_view local_in, std::wstring &utf16_out, uint32_t local_cp)
{
	int u16_size = MultiByteToWideChar(local_cp, MB_ERR_INVALID_CHARS, local_in.data(), int(local_in.size()), nullptr, 0);
	if (u16_size > 0)
	{
		const std::wstring utf16(u16_size, 0);
		u16_size = MultiByteToWideChar(local_cp, MB_ERR_INVALID_CHARS, local_in.data(), int(local_in.size()), const_cast<wchar_t *>(utf16.data()), int(utf16.size()));
		if (u16_size > 0)
		{
			return (u16_size == int(utf16.size())) ? ((utf16_out = utf16), true) : false;
		}
	}
	return false; // 在这里可以抛异常
}

std::wstring StringUtils::Utf16BeToLe(const std::wstring_view utf16_in)
{
	std::wstring utf16_out;
	utf16_out.reserve(utf16_in.size());

	for (char16_t ch : utf16_in)
	{
		char16_t swapped = (ch >> 8) | (ch << 8);
		utf16_out.push_back(swapped);
	}
	return utf16_out;
}

bool StringUtils::ToBool(std::string_view string_in, bool &bool_out)
{
	if (string_in == "true" || string_in == "True" || string_in == "TRUE" || string_in == "yes" || string_in == "Yes" || string_in == "YES")
	{
		bool_out = true;
		return true;
	}
	if (string_in == "false" || string_in == "False" || string_in == "FALSE" || string_in == "no" || string_in == "No" || string_in == "NO")
	{
		bool_out = false;
		return false;
	}
	return false;
}

bool StringUtils::ToBool(std::wstring_view string_in, bool &bool_out)
{
	std::string new_string;
	for (size_t i = 0; i < string_in.length(); i++)
	{
		new_string.push_back(static_cast<const char>(string_in.at(i)));
	}
	return ToBool(new_string, bool_out);
}

std::string &StringUtils::LTrim(std::string &string_in)
{
	string_in.erase(string_in.begin(),
					std::find_if(string_in.begin(), string_in.end(), [](int ch)
								 { return !std::isspace(ch); }));
	return string_in;
}

std::wstring &StringUtils::LTrim(std::wstring &string_in)
{
	string_in.erase(string_in.begin(),
					std::find_if(string_in.begin(), string_in.end(), [](int ch)
								 { return !std::isspace(ch); }));
	return string_in;
}

std::string &StringUtils::RTrim(std::string &string_in)
{
	string_in.erase(std::find_if(string_in.rbegin(), string_in.rend(), [](int ch)
								 { return !std::isspace(ch); })
						.base(),
					string_in.end());
	return string_in;
}

std::wstring &StringUtils::RTrim(std::wstring &string_in)
{
	string_in.erase(std::find_if(string_in.rbegin(), string_in.rend(), [](int ch)
								 { return !std::isspace(ch); })
						.base(),
					string_in.end());
	return string_in;
}

std::string &StringUtils::Trim(std::string &string_in)
{
	return LTrim(string_in), RTrim(string_in);
}

std::wstring &StringUtils::Trim(std::wstring &string_in)
{
	return LTrim(string_in), RTrim(string_in);
}

bool StringUtils::Equals(std::string_view l_string, std::string_view r_string, bool ignore_case)
{
	if (ignore_case)
	{
		return std::equal(l_string.begin(), l_string.end(), r_string.begin(), r_string.end(),
						  [](const char l, const char r)
						  { return tolower(l) == tolower(r); });
	}
	return l_string == r_string;
}

bool StringUtils::Equals(std::wstring_view l_string, std::wstring_view r_string, bool ignore_case)
{
	if (ignore_case)
	{
		return std::equal(l_string.begin(), l_string.end(), r_string.begin(), r_string.end(),
						  [](const wchar_t l, const wchar_t r)
						  { return tolower(l) == tolower(r); });
	}
	return l_string == r_string;
}

std::vector<std::string> StringUtils::Split(const std::string &input, const std::string &patten, bool skip_empty)
{
	// passing -1 as the submatch index parameter performs splitting
	std::regex regex(patten);
	std::sregex_token_iterator
		first{input.begin(), input.end(), regex, -1},
		last;

	std::vector<std::string> tokens;
	if (skip_empty)
	{
		for (auto it = first; it != last; ++it)
		{
			std::string each = *it;
			if (skip_empty)
			{
				auto &trim = Trim(each);
				if (trim.empty())
				{
					continue;
				}
			}
			tokens.push_back(each);
		}
	}
	return tokens;
}

std::vector<std::wstring> StringUtils::Split(const std::wstring &input, const std::wstring &patten, bool skip_empty)
{
	// passing -1 as the submatch index parameter performs splitting
	std::wregex regex(patten);
	std::wsregex_token_iterator
		first{input.begin(), input.end(), regex, -1},
		last;

	// std::vector<std::wstring> tokens = { first, last };
	// if (skip_empty)
	//{
	//	for (auto it = tokens.begin(); it != tokens.end(); )
	//	{
	//		std::wstring each = *it;
	//		auto& trim = Trim(each);
	//		if (trim.empty())
	//		{
	//			it = tokens.erase(it);
	//			continue;
	//		}
	//		++it;
	//	}
	// }

	// 运行效率为注释代码的两倍
	std::vector<std::wstring> tokens;
	if (skip_empty)
	{
		for (auto it = first; it != last; ++it)
		{
			std::wstring each = *it;
			if (skip_empty)
			{
				auto &trim = Trim(each);
				if (trim.empty())
				{
					continue;
				}
			}
			tokens.push_back(each);
		}
	}
	return tokens;
}

std::vector<std::string> StringUtils::Split(const std::string &input, const char delim, bool skip_empty)
{
	std::istringstream split(input);
	std::vector<std::string> tokens;
	if (skip_empty)
	{
		for (std::string each; std::getline(split, each, delim);)
		{
			auto &trim = Trim(each);
			if (trim.empty())
			{
				continue;
			}
			tokens.push_back(each);
		}
	}
	else
	{
		for (std::string each; std::getline(split, each, delim); tokens.push_back(each))
			;
	}
	return tokens;
}

std::vector<std::wstring> StringUtils::Split(const std::wstring &input, const wchar_t delim, bool skip_empty)
{
	std::wistringstream split(input);
	std::vector<std::wstring> tokens;
	if (skip_empty)
	{
		for (std::wstring each; std::getline(split, each, delim);)
		{
			auto &trim = Trim(each);
			if (trim.empty())
			{
				continue;
			}
			tokens.push_back(each);
		}
	}
	else
	{
		for (std::wstring each; std::getline(split, each, delim); tokens.push_back(each))
			;
	}
	return tokens;
}

std::string StringUtils::Join(const std::vector<std::string> &strings, std::string_view delim)
{
	std::string result;
	if (strings.empty())
	{
		return result;
	}

	// {
	// 	size_t count = 0;
	// 	for (const auto &str : strings)
	// 	{
	// 		count += str.size();
	// 	}
	// 	result.reserve(count + strings.size() * delim.size());
	// }

	result += strings.front();
	for (size_t i = 1; i < strings.size(); ++i)
	{
		result += delim;
		result += strings[i];
	}
	return result;
}

std::wstring StringUtils::Join(const std::vector<std::wstring> &strings, std::wstring_view delim)
{
	std::wstring result;
	if (strings.empty())
	{
		return result;
	}

	result += strings.front();
	for (size_t i = 1; i < strings.size(); ++i)
	{
		result += delim;
		result += strings[i];
	}
	return result;
}

std::string StringUtils::Join(const std::vector<std::string> &strings, const char delim)
{
	std::string result;
	if (strings.empty())
	{
		return result;
	}
	
	result += strings.front();
	for (size_t i = 1; i < strings.size(); ++i)
	{
		result += delim;
		result += strings[i];
	}
	return result;
}

std::wstring StringUtils::Join(const std::vector<std::wstring> &strings, const wchar_t delim)
{
	std::wstring result;
	if (strings.empty())
	{
		return result;
	}
	
	result += strings.front();
	for (size_t i = 1; i < strings.size(); ++i)
	{
		result += delim;
		result += strings[i];
	}
	return result;
}
