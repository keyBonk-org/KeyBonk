// ===./include/global.hpp===
// 全局变量和共享定义

#ifndef GLOBAL_H
#define GLOBAL_H

#include <windows.h>
#include <gdiplus.h>
#include <optional>
#include <map>
#include <mutex>
#include <atomic>
#include "functions/background.hpp"

namespace keybonk
{
    struct resource_manager
    {
    public:
        // 全局变量定义
        ULONG_PTR gdiplusToken = 0;           // GDI+的token
        bool comInitialized = false;          // COM库是否初始化成功
        wchar_t *fullIniFilePath = nullptr;   // 配置文件完整路径
        wchar_t *fullDebugFilePath = nullptr; // 日志文件完整路径
        HWND hwnd = NULL;                     // 主窗口句柄
        HWND hwndAbout = NULL;                // "关于"窗口句柄
        HWND hwndSetting = NULL;              // “设置”窗口句柄
        bool Mute = false;                    // 键盘是否静音
        bool MuteMouse = false;               // 鼠标是否静音
        bool WindowPenetrate = false;         // 窗口穿透
        NOTIFYICONDATA nid = {};              // 任务栏通知区域图标状态
        wchar_t audioLibPath[MAX_PATH];       // 音频库位置
        bool minimum = false;
        HINSTANCE hInstance;
        HHOOK KeyboardHook = nullptr; // 钩子句柄
        HHOOK MouseHook = nullptr;    // 钩子句柄
        HBITMAP hBmp = nullptr;       // 存储背景图片的位图
        HDC hdcScreen = nullptr;      // 主窗口屏幕DC
        HDC memDC = nullptr;          // 主窗口内存DC
        HBITMAP hOldBmp = nullptr;    // 主窗口内存DC默认位图
        int nCmdShow;
        HRESULT hrMain;                             // 接受Windows函数的返回结果
        std::optional<keybonk::background> bg_opt;  // 背景对象，在main初始化
        std::map<int, size_t> audioList;            // 音频库文件大小
        std::mutex mutex;                           // 互斥锁
        std::atomic<bool> audioPreloadReady{false}; // 用于接收预加载完成的信号
        // 全局资源释放
        ~resource_manager();
    };
    inline resource_manager global;
}
#endif // GLOBAL_H