#pragma once

#include <filesystem>

/**
 * \brief 系统实用类
 * \author sf
 * \date 2026年5月20日
 */
class SystemUtils final
{
public:
	/**
	 * \brief 获取当前执行文件所在文件
	 * \return 当前执行文件所在文件
	 */
	static std::filesystem::path GetExecutableFolder();
};