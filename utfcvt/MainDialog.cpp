#include "MainDialog.h"

#include <CommCtrl.h>
#include <Richedit.h>
#include <shlobj_core.h>
#include <shlwapi.h>
#include <tchar.h>
#include <windowsx.h>
#include <pplawait.h>

#include "Converter.h"
#include "DropTarget.h"
#include "Language.h"
#include "Resource.h"
#include "StringUtils.h"

#define UM_APPEND_TEXT (WM_USER + 1)

static Concurrency::cancellation_token_source convert_cts_;
static Concurrency::task<void> convert_task_;

static FolderDropTarget *drop_target_ = nullptr;

INT_PTR CALLBACK MainDialog::WindowProc(HWND dlg, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_INITDIALOG:
	{
		OnInit(dlg, *((CvtConf *)lp));
		// DragAcceptFiles(dlg, TRUE); // 开启文件拖拽接收
		drop_target_ = new (std::nothrow) FolderDropTarget(dlg);
		RegisterDragDrop(dlg, drop_target_);
		drop_target_->OnFolderDropped = [dlg](const std::wstring &folder)
		{
			SetWindowText(GetDlgItem(dlg, IDC_SOURCE_FOLDER), folder.c_str());
		};
		return (INT_PTR)TRUE;
	}
	case WM_DESTROY:
	{
		if (drop_target_)
		{
			RevokeDragDrop(dlg);
			drop_target_->Release();
		}
		PostQuitMessage(0);
		return (INT_PTR)TRUE;
	}
	case WM_CLOSE:
	{
		DestroyWindow(dlg);
		return (INT_PTR)TRUE;
	}
	// case WM_DROPFILES:
	// {
	// 	HDROP hdrop = (HDROP)wp;
	// 	wchar_t path[MAX_PATH]{};
	// 	DragQueryFileW(hdrop, 0, path, MAX_PATH);
	// 	DragFinish(hdrop);
	// 	DWORD attr = GetFileAttributesW(path);
	// 	if (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY))
	// 	{
	// 		// 是文件夹 → 允许
	// 		SetWindowText(GetDlgItem(dlg, IDC_SOURCE_FOLDER), path);
	// 		return (INT_PTR)TRUE;
	// 	}
	// 	break;
	// }
	case WM_COMMAND:
	{
		WORD id = LOWORD(wp);
		HWND control = (HWND)lp;
		if (HIWORD(wp) == EN_CHANGE)
		{
			if (id != IDC_SOURCE_FOLDER)
				break;

			int length = GetWindowTextLength(control) + 1; // 终止符
			std::vector<TCHAR> text;
			if (length > 0)
			{
				text.resize(length);
				GetWindowText(control, text.data(), length);
			}
			EnableWindow(GetDlgItem(dlg, IDC_SOURCE_CONVERT), PathFileExists(text.data()));
			return (INT_PTR)TRUE;
		}
		OnCommand(dlg, id, control);
		return (INT_PTR)TRUE;
	}
	case UM_APPEND_TEXT:
	{
		HWND logger = GetDlgItem(dlg, IDC_OUTPUT_LOGGER);
		AppendText(logger, (LPCWSTR)wp, (COLORREF)lp);
		return (INT_PTR)TRUE;
	}
	default:
		break;
	}
	return (INT_PTR)FALSE;
}

void MainDialog::OnInit(HWND dlg, const CvtConf &config)
{
	// 国际化支持
	auto &lang = Language::GetInstance();
	SetWindowText(dlg, lang.GetStaticStr(TEXT("Title"), TEXT("多字节转UNICODE")).c_str());
	SetWindowText(GetDlgItem(dlg, IDC_LBL_FOLDER), lang.GetStaticStr(TEXT("Folder"), TEXT("源目录：")).c_str());
	SetWindowText(GetDlgItem(dlg, IDC_SOURCE_BROWSE), lang.GetStaticStr(TEXT("Browse"), TEXT("浏览...(&B)")).c_str());
	SetWindowText(GetDlgItem(dlg, IDC_LBL_EXTENSION), lang.GetStaticStr(TEXT("Extension"), TEXT("源扩展名：")).c_str());
	SetWindowText(GetDlgItem(dlg, IDC_LBL_EXCLUDES), lang.GetStaticStr(TEXT("Exclude"), TEXT("排除目录：")).c_str());
	SetWindowText(GetDlgItem(dlg, IDC_LBL_SRC_ENC), lang.GetStaticStr(TEXT("SrcEncoding"), TEXT("源编码：")).c_str());
	SetWindowText(GetDlgItem(dlg, IDC_SUBDIR_SCANNING), lang.GetStaticStr(TEXT("SubDirScan"), TEXT("扫描子目录")).c_str());
	SetWindowText(GetDlgItem(dlg, IDC_LBL_TGT_ENC), lang.GetStaticStr(TEXT("TgtEncoding"), TEXT("目标编码：")).c_str());
	SetWindowText(GetDlgItem(dlg, IDC_TYPE_CASTING), lang.GetStaticStr(TEXT("SkipUnicode"), TEXT("跳过UNICODE")).c_str());
	SetWindowText(GetDlgItem(dlg, IDC_SOURCE_CONVERT), lang.GetStaticStr(TEXT("StartConvert"), TEXT("开始转换(&C)")).c_str());

	HWND folder = GetDlgItem(dlg, IDC_SOURCE_FOLDER);
	HWND extension = GetDlgItem(dlg, IDC_SOURCE_EXTENSION);
	HWND excludes = GetDlgItem(dlg, IDC_SOURCE_EXCLUDES);
	HWND encoding = GetDlgItem(dlg, IDC_SOURCE_ENCODING);
	HWND decoding = GetDlgItem(dlg, IDC_TARGET_ENCODING);
	HWND subdir = GetDlgItem(dlg, IDC_SUBDIR_SCANNING);
	HWND casting = GetDlgItem(dlg, IDC_TYPE_CASTING);
	HWND logger = GetDlgItem(dlg, IDC_OUTPUT_LOGGER);
	bool auto_start = false;

	if (config.command_line)
	{
		TCHAR buffer[64]{0};
		_stprintf_s(buffer, TEXT("--- 当前编码(%d) ---"), config.code_page);
		ComboBox_AddString(encoding, buffer);
		auto_start = PathFileExists(config.root_folder.c_str());
	}
	else
	{
		ComboBox_AddString(encoding, lang.GetDynamicStr(TEXT("SysEncoding"), TEXT("--- 系统编码(默认) ---")).c_str());
		ComboBox_AddString(encoding, lang.GetDynamicStr(TEXT("ChsEncoding"), TEXT("--- 中文简体(936) ---")).c_str());
		ComboBox_AddString(encoding, lang.GetDynamicStr(TEXT("ChtEncoding"), TEXT("--- 中文繁体(950) ---")).c_str());
	}

	ComboBox_AddString(decoding, lang.GetDynamicStr(TEXT("Utf8"), TEXT("--- UTF-8 ---")).c_str());
	ComboBox_AddString(decoding, lang.GetDynamicStr(TEXT("Utf8Bom"), TEXT("--- UTF-8 BOM ---")).c_str());
	ComboBox_AddString(decoding, lang.GetDynamicStr(TEXT("Utf16Le"), TEXT("--- UTF-16 LE ---")).c_str());

	SetWindowText(folder, config.root_folder.c_str());
	SetWindowText(extension, StringUtils::Join(config.extensions, TEXT('|')).c_str());
	SetWindowText(excludes, StringUtils::Join(config.excludes, TEXT('|')).c_str());
	Button_SetCheck(subdir, config.scan_subfolders);
	Button_SetCheck(casting, config.skip_unicode);

	ComboBox_SetCurSel(encoding, Config::CodePageToIndex(config.code_page));
	ComboBox_SetCurSel(decoding, config.save_encode);

	SendMessage(logger, EM_SETREADONLY, TRUE, 0); // 只读模式

	convert_cts_.cancel();
	if (auto_start)
	{
		OnConvertClicked(dlg, const_cast<CvtConf &>(config));
	}
}

void MainDialog::OnCommand(HWND dlg, WORD id, HWND control)
{
	switch (id)
	{
	case IDCANCEL:
	{
		EndDialog(dlg, id);
		break;
	}
	case IDC_SOURCE_BROWSE:
	{
		OnBrowseClicked(dlg);
		break;
	}
	case IDC_SOURCE_CONVERT:
	{
		CvtConf config;
		OnConvertClicked(dlg, config);
		break;
	}
	default:
		break;
	}
}

void MainDialog::AppendText(HWND rich, const std::wstring &text, COLORREF color, bool newline, bool scroll)
{
	CHARFORMAT2W cf{};
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_COLOR;
	cf.crTextColor = color;
	SendMessage(rich, EM_SETSEL, -1, -1); // 移动光标到末尾
	SendMessage(rich, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf); // 设置颜色
	SendMessage(rich, EM_REPLACESEL, FALSE, (LPARAM)text.c_str()); // 插入文本

	if (newline)
	{
		SendMessage(rich, EM_REPLACESEL, FALSE, (LPARAM)TEXT("\r\n")); // 插入换行符
	}
	if (scroll)
	{
		SendMessage(rich, EM_SCROLLCARET, 0, 0);// 滚动到底部
	}
}

void MainDialog::OnBrowseClicked(HWND dlg)
{
	auto &lang = Language::GetInstance();
	HWND folder = GetDlgItem(dlg, IDC_SOURCE_FOLDER);
	int length = GetWindowTextLength(folder) + 1;
	std::vector<TCHAR> text;
	if (length > 0)
	{
		text.resize(length);
		GetWindowText(folder, text.data(), length);
	}

	auto title = lang.GetDynamicStr(TEXT("BrowseTitle"), TEXT("绍峰网络工作室"));

	BROWSEINFO browse_info{};
	browse_info.lpfn = nullptr;
	browse_info.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
	browse_info.hwndOwner = dlg;
	browse_info.lpszTitle = title.c_str();
	browse_info.lParam = (LPARAM)text.data();

	LPITEMIDLIST pidl = nullptr;
	if ((pidl = SHBrowseForFolder(&browse_info)) != nullptr)
	{
		wchar_t path[MAX_PATH];
		if (SHGetPathFromIDList(pidl, path))
		{
			SetWindowText(folder, path);
			ILFree(pidl);
		}
	}
}

void MainDialog::OnConvertClicked(HWND dlg, CvtConf config)
{
	auto &lang = Language::GetInstance();

	EnableWindow(GetDlgItem(dlg, IDC_SOURCE_CONVERT), false);
	if (convert_cts_.get_token().is_canceled())
	{
		convert_cts_ = Concurrency::cancellation_token_source();
	}
	else
	{
		convert_cts_.cancel();
		return;
	}

	HWND convert = GetDlgItem(dlg, IDC_SOURCE_CONVERT);
	HWND progress = GetDlgItem(dlg, IDC_SOURCE_PROGRESS);
	HWND logger = GetDlgItem(dlg, IDC_OUTPUT_LOGGER);

	// 开启跑马灯模式
	LONG_PTR style = GetWindowLongPtr(progress, GWL_STYLE);
	SetWindowLongPtr(progress, GWL_STYLE, style | PBS_MARQUEE);
	SendMessage(progress, (UINT)PBM_SETMARQUEE, (WPARAM)TRUE, (LPARAM)10);

	if (!config.command_line)
	{
		HWND folder = GetDlgItem(dlg, IDC_SOURCE_FOLDER);
		HWND encoding = GetDlgItem(dlg, IDC_SOURCE_ENCODING);
		HWND extension = GetDlgItem(dlg, IDC_SOURCE_EXTENSION);
		HWND exclude = GetDlgItem(dlg, IDC_SOURCE_EXCLUDES);
		HWND subdir = GetDlgItem(dlg, IDC_SUBDIR_SCANNING);
		HWND decoding = GetDlgItem(dlg, IDC_TARGET_ENCODING);
		HWND casting = GetDlgItem(dlg, IDC_TYPE_CASTING);

		int length = GetWindowTextLength(folder) + 1;
		std::vector<TCHAR> text;
		if (length > 0)
		{
			text.resize(length);
			GetWindowText(folder, text.data(), length);
		}
		config.root_folder = text.data();
		length = GetWindowTextLength(extension) + 1;
		if (length > 0)
		{
			text.resize(length);
			GetWindowText(extension, text.data(), length);
		}
		config.extensions = StringUtils::Split(text.data(), TEXT('\x7C'));
		length = GetWindowTextLength(exclude) + 1;
		if (length > 0)
		{
			text.resize(length);
			GetWindowText(exclude, text.data(), length);
		}
		config.code_page = Config::IndexToCodePage(ComboBox_GetCurSel(encoding));
		config.excludes = StringUtils::Split(text.data(), TEXT('\x7C'));
		config.scan_subfolders = (Button_GetCheck(subdir) == BST_CHECKED);
		config.save_encode = SaveEncode(ComboBox_GetCurSel(decoding));
		config.skip_unicode = (Button_GetCheck(casting) == BST_CHECKED);

		CvtConf curConfig;
		Config::LoadFromIni(curConfig); // Keep current language
		config.language = curConfig.language;

		Config::SaveToIni(config);
	}

	// 相对路径转绝对路径
	for (auto &exclude : config.excludes)
	{
		if (exclude.find(TEXT(":\\")) == -1)
		{
			exclude = config.root_folder + (exclude.front() == TEXT('\\') ? TEXT("") : TEXT("\\")) + exclude;
		}
	}

	convert_task_ = Concurrency::create_task([convert, progress, logger, dlg, config]()
											 {
			bool open_dlg = false;
			auto& lang = Language::GetInstance();

			if (PathFileExists(config.root_folder.c_str()))
			{
				int file_count{ 0 };
				EnableWindow(convert, true);

				Edit_SetText(logger, TEXT(""));
				SetWindowText(convert, lang.GetStaticStr(TEXT("StopConvert"), TEXT("停止转换(&S)")).c_str());
				for (const auto& file : Converter::GetFiles(config))
				{
					if (convert_cts_.get_token().is_canceled())
					{
						break;
					}
					++file_count;
					auto text = file.substr(config.root_folder.size() + 1);
					SetWindowText(dlg, text.c_str());
				}
				SendMessage(progress, PBM_SETRANGE32, 0, file_count);
				SendMessage(progress, PBM_SETPOS, 0, 0);

				// 关闭跑马灯模式
				LONG_PTR style = GetWindowLongPtr(progress, GWL_STYLE);
				SendMessage(progress, (UINT)PBM_SETMARQUEE, (WPARAM)FALSE, (LPARAM)0);
				SetWindowLongPtr(progress, GWL_STYLE, style & ~PBS_MARQUEE);

				if (!convert_cts_.get_token().is_canceled())
				{
					std::vector<Concurrency::task<bool>> tasks;
					for (const auto& file : Converter::GetFiles(config))
					{
						tasks.emplace_back(Concurrency::create_task([&, file]()->bool
							{
								auto text = file.substr(config.root_folder.size() + 1);
								SetWindowText(dlg, text.c_str());
								if (!convert_cts_.get_token().is_canceled())
								{
									CvtResult ret = Converter::ToUnicode(file, config).get();
									switch (ret)
									{
									case CvtResult::Success:
										SendMessage(dlg, UM_APPEND_TEXT, (WPARAM)(lang.GetDynamicStr( TEXT("ConvertSuccess"), TEXT("转换成功: ")) + text).c_str(), kGreenColor);
										break;
									case CvtResult::Ignore:
										SendMessage(dlg, UM_APPEND_TEXT, (WPARAM)(lang.GetDynamicStr( TEXT("ConvertIgnore"), TEXT("跳过文件: ")) + text).c_str(), kOrangeColor);
										break;
									case CvtResult::Invalid:
										SendMessage(dlg, UM_APPEND_TEXT, (WPARAM)(lang.GetDynamicStr( TEXT("ConvertInvalid"), TEXT("无效编码: ")) + text).c_str(), kRedColor);
										break;
									default:
										break;
									}
									SendMessage(progress, PBM_DELTAPOS, 1, 0);
									return true;
								}
								return false;
							}));
					}
					when_all(begin(tasks), end(tasks)).wait();

					open_dlg = (!convert_cts_.get_token().is_canceled());
				}
				SetWindowText(dlg, lang.GetStaticStr(TEXT("Title"), TEXT("多字节转UNICODE")).c_str());
			}
			EnableWindow(convert, true);
			SetWindowText(convert, lang.GetStaticStr(TEXT("StartConvert"), TEXT("开始转换(&C)")).c_str());
			convert_cts_.cancel();
			if (open_dlg && MessageBox(dlg, lang.GetDynamicStr(TEXT("FinishMsg"), TEXT("是否要打开转换目录？")).c_str(), lang.GetDynamicStr( TEXT("FinishTitle"), TEXT("【转换完成】")).c_str(), MB_ICONINFORMATION | MB_YESNO) == IDYES)
			{ // 打开转换目录
				OpenFolder(config.root_folder);
			} });
}

bool MainDialog::OpenFolder(const std::wstring &folder)
{
	if (SUCCEEDED(CoInitializeEx(nullptr, 0)))
	{
		PIDLIST_ABSOLUTE pidl;
		if (SUCCEEDED(SHParseDisplayName(folder.c_str(), nullptr, &pidl, 0, nullptr)))
		{
			ITEMIDLIST idl{};
			LPCITEMIDLIST apidl[] = {&idl};
			SHOpenFolderAndSelectItems(pidl, _countof(apidl), apidl, 0);
			ILFree(pidl);
		}
		CoUninitialize();
	}
	return false;
}
