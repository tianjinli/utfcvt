#pragma once

#include <Windows.h>

#include <functional>
#include <string>
#include <vector>

class FolderDropTarget : public IDropTarget
{
public:
    FolderDropTarget(HWND host_wnd, bool multiple = false) : host_wnd_(host_wnd), reference_(1), dir_count_(false), multiple_(multiple) {}

    // IUnknown 保持你的原样，写得很规范
    IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv) override
    {
        if (riid == IID_IUnknown || riid == IID_IDropTarget)
        {
            *ppv = static_cast<IDropTarget *>(this);
            AddRef();
            return S_OK;
        }
        *ppv = nullptr;
        return E_NOINTERFACE;
    }

    IFACEMETHODIMP_(ULONG)
    AddRef() override { return InterlockedIncrement(&reference_); }
    IFACEMETHODIMP_(ULONG)
    Release() override
    {
        ULONG res = InterlockedDecrement(&reference_);
        if (res == 0)
            delete this;
        return res;
    }

    // IDropTarget
    IFACEMETHODIMP DragEnter(IDataObject *data_obj, DWORD, POINTL, DWORD *effect) override
    {
        CountDirFile(data_obj);
        *effect = IsValidDrop() ? DROPEFFECT_COPY : DROPEFFECT_NONE;
        return S_OK;
    }

    IFACEMETHODIMP DragOver(DWORD, POINTL, DWORD *effect) override
    {
        *effect = IsValidDrop() ? DROPEFFECT_COPY : DROPEFFECT_NONE;
        return S_OK;
    }

    IFACEMETHODIMP DragLeave() override
    {
        // 离开时清空状态
        dir_count_ = 0;
        file_count_ = 0;
        return S_OK;
    }

    IFACEMETHODIMP Drop(IDataObject *data_obj, DWORD, POINTL, DWORD *effect) override
    {
        // 双重防御
        if (!IsValidDrop())
        {
            *effect = DROPEFFECT_NONE;
            return S_OK;
        }

        if (OnFolderDropped)
        {
            auto folders = ExtractPaths(data_obj);
            for (const auto &folder : folders)
            {
                OnFolderDropped(folder);
            }
        }

        *effect = DROPEFFECT_COPY;
        return S_OK;
    }

private:
    bool IsValidDrop() const { return (((multiple_ && dir_count_ > 0) || (!multiple_ && dir_count_ == 1)) && dir_count_ == file_count_); }

    void CountDirFile(IDataObject *data_obj)
    {
        FORMATETC fmt = {CF_HDROP, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
        STGMEDIUM stg{};

        dir_count_ = 0;
        file_count_ = 0;
        if (FAILED(data_obj->GetData(&fmt, &stg)))
            return;

        HDROP hdrop = (HDROP)stg.hGlobal;
        file_count_ = DragQueryFileW(hdrop, 0xFFFFFFFF, nullptr, 0); // 获取多选总数
        for (UINT i = 0; i < file_count_; ++i)
        {
            wchar_t path[MAX_PATH]{};
            DragQueryFileW(hdrop, i, path, MAX_PATH);

            DWORD attr = GetFileAttributesW(path);
            if (attr & FILE_ATTRIBUTE_DIRECTORY)
            {
                ++dir_count_;
                break;
            }
        }

        ReleaseStgMedium(&stg);
    }

    // 提取所有被拖放出来的合法路径容器
    std::vector<std::wstring> ExtractPaths(IDataObject *data_obj)
    {
        std::vector<std::wstring> paths;
        FORMATETC fmt = {CF_HDROP, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
        STGMEDIUM stg{};

        if (SUCCEEDED(data_obj->GetData(&fmt, &stg)))
        {
            HDROP hdrop = (HDROP)stg.hGlobal;
            UINT file_count = DragQueryFileW(hdrop, 0xFFFFFFFF, nullptr, 0);
            for (UINT i = 0; i < file_count; ++i)
            {
                wchar_t path[MAX_PATH]{};
                DragQueryFileW(hdrop, i, path, MAX_PATH);
                paths.emplace_back(path);
            }
            ReleaseStgMedium(&stg);
        }
        return paths;
    }

public:
    std::function<void(const std::wstring &)> OnFolderDropped;

private:
    HWND host_wnd_ = nullptr;
    LONG reference_ = 0;
    uint32_t dir_count_ = 0;  // 文件夹数量
    uint32_t file_count_ = 0; // 拖放项数量
    bool multiple_ = false;
};
