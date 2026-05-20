#pragma once

#include <string>
#include <string_view>
#include <vector>

/**
 * \brief 字符串实用类
 * \author sf
 * \date 2019年6月18日
 */
class StringUtils final
{
public:
	/**
	 * \brief 判断字符串是否为UTF8编码
	 * \param string_in 字符串
	 * \return 是否为UTF8编码
	 */
	static bool IsUtf8(const std::string_view string_in);

	/**
	 * \brief 本地字符串转UTF8字符串
	 * \param local_in 本地字符串
	 * \param utf8_out UTF8字符串(成功则修改)
	 * \param local_cp 本地字符串编码
	 * \return 是否转换成功
	 */
	static bool ToUtf8(const std::string_view local_in, std::string &utf8_out, uint32_t local_cp = 0);
	/**
	 * \brief UTF16字符串转UTF8字符串
	 * \param utf16_in UTF16字符串
	 * \param utf8_out UTF8字符串(成功则修改)
	 * \return 是否转换成功
	 */
	static bool ToUtf8(const std::wstring_view utf16_in, std::string &utf8_out);

	/**
	 * \brief 本地字符串转UTF16字符串
	 * \param local_in 本地字符串
	 * \param utf16_out UTF16字符串(成功则修改)
	 * \param local_cp 本地字符串编码
	 * \return 是否转换成功
	 */
	static bool ToUtf16(const std::string_view local_in, std::wstring &utf16_out, uint32_t local_cp = 0);

	/**
	 * \brief UTF16BE字符串转UTF16LE字符串
	 * \param utf16_in UTF16BE字符串
	 * \param utf16_out UTF16LE字符串(成功则修改)
	 * \return 是否转换成功
	 */
	static std::wstring Utf16BeToLe(const std::wstring_view utf16_in);

	/**
	 * \brief 字符串转Bool类型
	 * \param string_in 字符串
	 * \param bool_out Bool类型(成功则修改)
	 * \return 是否转换成功
	 */
	static bool ToBool(std::string_view string_in, bool &bool_out);
	static bool ToBool(std::wstring_view string_in, bool &bool_out);

	/**
	 * \brief 删除左边空白字符
	 * \param string_in 输入/输出字符串
	 * \return 修剪之后字符串
	 */
	static std::string &LTrim(std::string &string_in);
	static std::wstring &LTrim(std::wstring &string_in);

	/**
	 * \brief 删除右边空白字符
	 * \param string_in 输入/输出字符串
	 * \return 修剪之后字符串
	 */
	static std::string &RTrim(std::string &string_in);
	static std::wstring &RTrim(std::wstring &string_in);

	/**
	 * \brief 删除两边空白字符
	 * \param string_in 输入/输出字符串
	 * \return 修剪之后字符串
	 */
	static std::string &Trim(std::string &string_in);
	static std::wstring &Trim(std::wstring &string_in);

	/**
	 * \brief 比较字符串是否相等
	 * \param l_string 字符串1
	 * \param r_string 字符串2
	 * \param ignore_case 忽略大小写
	 * \return 是否相等
	 */
	static bool Equals(std::string_view l_string, std::string_view r_string, bool ignore_case = true);
	static bool Equals(std::wstring_view l_string, std::wstring_view r_string, bool ignore_case = true);

	/**
	 * \brief 按照正则表达式拆分字符串
	 * \param input 要拆分的字符串
	 * \param regex 正则表达式
	 * \return 拆分的结果集
	 */
	static std::vector<std::string> Split(const std::string& input, const std::string &patten, bool skip_empty = true);
	static std::vector<std::wstring> Split(const std::wstring& input, const std::wstring &patten, bool skip_empty = true);

	/**
	 * \brief 按照分隔符拆分字符串
	 * \param input 要拆分的字符串
	 * \param delim 分隔符
	 * \return 拆分的结果集
	 */
	static std::vector<std::string> Split(const std::string &input, const char delim, bool skip_empty = true);
	static std::vector<std::wstring> Split(const std::wstring &input, const wchar_t delim, bool skip_empty = true);

	/**
	 * \brief 按照分隔符连接字符串
	 * \param strings 要连接的字符串
	 * \param delim 分隔符
	 * \return 连接之后的字符串
	 */
	static std::string Join(const std::vector<std::string> &strings, std::string_view delim);
	static std::wstring Join(const std::vector<std::wstring> &strings, std::wstring_view delim);

	/**
	 * \brief 按照分隔符连接字符串
	 * \param strings 要连接的字符串
	 * \param delim 分隔符
	 * \return 连接之后的字符串
	 */
	static std::string Join(const std::vector<std::string> &strings, const char delim);
	static std::wstring Join(const std::vector<std::wstring> &strings, const wchar_t delim);
};