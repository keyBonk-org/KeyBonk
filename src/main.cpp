// === ./src/main.cpp ===
// 主程序存放的文件
// 包含消息循环，作为入口

#include <filesystem>
#include <thread>
#include <algorithm>
#include "global.hpp"
#include "debug.hpp"
#include "window_manager.hpp"
#include "hook/keyboard_hook.hpp"
#include "hook/mouse_hook.hpp"
#include "functions/files.hpp"
#include "resources.hpp"
#include "functions/background.hpp"
#include "audio-player/audioPlayer.hpp"

// 自定义消息
#define WM_WINDOW_HAS_CREAT (WM_APP + 4) // 窗口以及创建，由后面打开的进程发送到当前窗口

// 检查软件是否重复启动
// 检查标准为能否根据“窗口类名+窗口标题”的组合查询到主窗口的句柄
BOOL IsInstanceAlreadyRunning(LPCTSTR windowClass, LPCTSTR windowTitle)
{
    HWND f_hWnd = FindWindow(windowClass, windowTitle);
    if (f_hWnd)
    {
        // 找到窗口，激活它
        PostMessageW(f_hWnd, WM_WINDOW_HAS_CREAT, 0, 0); // 发送窗口消息给已经存在的窗口
        return TRUE;
    }
    return FALSE;
}

void preloadAudio()
{
    using keybonk::global;
    std::filesystem::path audioLibPath(global.audioLibPath);
    if (!std::filesystem::exists(audioLibPath) || !std::filesystem::is_directory(audioLibPath))
        return;

    for (const auto &entry : std::filesystem::recursive_directory_iterator(audioLibPath))
    {
        // 只处理普通文件且扩展名为 .wav
        if (!std::filesystem::is_regular_file(entry) || entry.path().extension() != ".wav")
            continue;

        // 获取不带扩展名的文件名
        std::wstring stem = entry.path().stem().wstring();

        // 检查文件名是否为纯数字（并转成整数）
        if (stem.empty() || !std::all_of(stem.begin(), stem.end(), ::iswdigit))
            continue; // 不是纯数字，舍弃

        int key = std::stoi(stem); // 转成整数（假设不会溢出）

        // 检查是否为合法的VK键值
        if (key < 0 || key > 255)
            continue;

        // 调用 preloadAudio，获取返回值
        yumo::readySign ready(false); // 用于接收预加载完成的信号
        std::wstring fullPath = entry.path().wstring();
        size_t audioID = yumo::preloadAudio(fullPath.c_str(), &ready); // 假设成功，内部处理错误

        while (ready == false)
        {
            // 等待预加载完成
            Sleep(10); // 休眠10毫秒，避免CPU占用过高
        }
        {
            std::lock_guard<std::mutex> lock(global.mutex); // 加锁，确保线程安全
            // 存入 map
            global.audioList[key] = audioID; // 或者用 insert，但 [] 会覆盖，通常没问题
        }
    }
    global.audioPreloadReady.store(true, std::memory_order_release); // 设置预加载完成标志
}

// 主程序
int WINAPI wWinMain(HINSTANCE hInstance, [[maybe_unused]] HINSTANCE hPrevInstance, [[maybe_unused]] PWSTR pCmdLine, int nCmdShow)
{
    using keybonk::global;
    global.fullDebugFilePath = new wchar_t[MAX_PATH]{};
    GetExeRelativePath(L"./log.txt", global.fullDebugFilePath, MAX_PATH);

    // =============================================================
    // ||                    重复启动检测                          ||
    // =============================================================

    if (IsInstanceAlreadyRunning(L"KeyBonk主窗口", L"KeyBonk主窗口"))
    {
        // 输出错误日志
        debug::logOutput(
            L"[初始化]KeyBonk主窗口重复创建,软件退出"
            L"\n    - hInstance:",
            std::to_wstring(reinterpret_cast<long long>(hInstance)).c_str(),
            L"\n    - CmdShow:", std::to_wstring(nCmdShow).c_str(), L"\n");
        return 0;
    }

    // =============================================================
    // ||                    准备启动信息                          ||
    // =============================================================

    debug::logProgramStartTime();

    // 保存一些参数到全局
    global.hInstance = hInstance;
    global.nCmdShow = nCmdShow;

    // 读取配置项文件中的record（上次退出时记录）部分
    // 格式化出ini文件的完整路径
    global.fullIniFilePath = new wchar_t[MAX_PATH]{};
    GetExeRelativePath(L"./config.ini", global.fullIniFilePath, MAX_PATH);
    // win-x和win-y 上次退出时的窗口位置
    int windowPositionX = GetPrivateProfileInt(L"record", L"win-x", 100, global.fullIniFilePath);
    int windowPositionY = GetPrivateProfileInt(L"record", L"win-y", 100, global.fullIniFilePath);
    // 把上次退出记录的静音情况读到Mute和MuteMouse里
    global.Mute = bool(GetPrivateProfileInt(L"record", L"mute", 0, global.fullIniFilePath));
    global.MuteMouse = bool(GetPrivateProfileInt(L"record", L"mute-m", 0, global.fullIniFilePath));

    // 读取设置信息
    GetPrivateProfileString(L"settings", L"lib", L".\\bin\\default", global.audioLibPath, MAX_PATH, global.fullIniFilePath);

    // =============================================================
    // ||                   初始化库与窗口                         ||
    // =============================================================

    // 初始化COM库（其实这是一个很久的未来才会有的功能要用的初始化，只是提前写了）
    global.hrMain = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    if (FAILED(global.hrMain))
    {
        debug::logOutput(L"[初始化]初始化COM库异常\n    - HRESULT：", std::to_wstring(global.hrMain).c_str(), L"\n");
        MessageBoxExW(
            NULL, L"错误：00001，初始化COM库时发生异常，请检查系统相关文件是否完好",
            L"KB - 运行时发生错误", MB_OK | MB_ICONEXCLAMATION, 0); // 消息框提示出错
        return 0;
    }
    global.comInitialized = true;

    // 注册窗口类
    const wchar_t CLASS_NAME[] = L"KeyBonk主窗口";
    WNDCLASSEX wc = {};             // 用0初始化整个WindowClass
    wc.cbSize = sizeof(WNDCLASSEX); // 设置结构体大小
    wc.lpfnWndProc = WindowProc;    // 指定WindowProc函数
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME; // 窗口类名称
    wc.hIcon = (HICON)LoadImage(global.hInstance, MAKEINTRESOURCE(IDI_MY_ICON), IMAGE_ICON, 64, 64, 0);
    wc.hIconSm = (HICON)LoadImage(global.hInstance, MAKEINTRESOURCE(IDI_MY_ICON), IMAGE_ICON, 64, 64, 0); // 小图标（窗口标题栏）
    RegisterClassEx(&wc);                                                                                 // 注册

    // 创建窗口
    global.hwnd = CreateWindowExW(
        WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW, // 支持透明，设置置顶，隐藏软件图标
        CLASS_NAME,                                       // 窗口类
        L"KeyBonk主窗口",                                 // 窗口文本
        WS_POPUP | WS_VISIBLE | WS_CLIPCHILDREN,          // 窗口风格
                                                          // 大小+位置
        windowPositionX, windowPositionY, 160, 180,
        NULL,      // 父窗口
        NULL,      // 菜单
        hInstance, // 示例句柄
        NULL       // 附带的软件数据
    );

    // 创建失败则提示并返回，结束运行
    if (global.hwnd == NULL)
    {
        debug::logOutput(L"[初始化]主窗口创建异常\n");
        debug::logWinError(GetLastError());
        MessageBoxExW(
            NULL, L"错误：00002，创建窗口时发生异常，请检查系统各项设置是否正常",
            L"KB - 运行时发生错误", MB_OK | MB_ICONEXCLAMATION, 0); // 消息框提示出错
        return 0;
    }

    // 初始化GDI+
    Gdiplus::GpStatus GDIpStatus; // 接收GDI+库的状态（错误码）
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    GDIpStatus = Gdiplus::GdiplusStartup(&global.gdiplusToken, &gdiplusStartupInput, NULL);
    if (GDIpStatus != Gdiplus::Ok)
    { // 如果存在问题
        debug::logOutput(L"[初始化]GDI+初始化异常\n    - GDI+状态码：", std::to_wstring(GDIpStatus).c_str(), L"\n");
        MessageBoxExW(
            NULL, L"错误：00003，初始化GDI+库时发生异常",
            L"KB - 运行时发生错误", MB_OK | MB_ICONEXCLAMATION, 0); // 消息框提示出错
        return 0;
    }

    // =============================================================
    // ||                    绘制背景图片                          ||
    // =============================================================

    // todo 改一改钩子回调，用来实现图片切换

    wchar_t *imgPath = new wchar_t[MAX_PATH]{};
    wchar_t *exePath = new wchar_t[MAX_PATH]{};
    GetExeDirectory(exePath, MAX_PATH);
    swprintf_s(imgPath, MAX_PATH, L"%ls\\%ls\\imgs", exePath, global.audioLibPath);
    delete[] exePath;

    try
    {
        global.bg_opt.emplace(global.hwnd, imgPath, global.nCmdShow);
    }
    catch (const keybonk::exception &e)
    {
        typedef keybonk::exception::type eType;

        switch (e.getType())
        {
        case eType::FileNotFound:
        {
            debug::logOutput(L"[初始化]背景图片文件未找到\n",
                             L"    - 图片文件夹路径：", imgPath, L"\n");
            MessageBoxExW(
                NULL, L"错误：00004，背景图片文件未找到，请检查相关文件是否存在",
                L"KB - 运行时发生错误", MB_OK | MB_ICONEXCLAMATION, 0); // 消息框提示出错}
            break;
        }
        case eType::InvalidInput:
        {
            debug::logOutput(L"[初始化]背景图片尺寸异常\n",
                             L"    - 图片文件夹路径：", imgPath, L"\n");
            MessageBoxExW(
                NULL, L"错误：00006，背景图片尺寸异常，请检查相关文件是否正确",
                L"KB - 运行时发生错误", MB_OK | MB_ICONEXCLAMATION, 0); // 消息框提示出错
            break;
        }
        default:
            debug::logOutput(L"[初始化]背景图片加载发生未知错误\n",
                             L"    - 图片文件夹路径：", imgPath, L"\n");
            MessageBoxExW(
                NULL, L"错误：00007，背景图片加载发生未知错误，请检查相关文件是否正确",
                L"KB - 运行时发生错误", MB_OK | MB_ICONEXCLAMATION, 0); // 消息框提示出错
            break;
        }
        delete[] imgPath;
        return 0;
    }
    global.bg_opt->resetToDefault();

    // =============================================================
    // ||                     预处理音频                           ||
    // =============================================================

    // 准备预加载线程
    std::thread preloadAudioThread(preloadAudio);

    preloadAudioThread.detach(); // 分离线程，让它在后台运行

    // =============================================================
    // ||                      钩子安装                            ||
    // =============================================================

    // 安装键盘钩子
    global.KeyboardHook = SetWindowsHookExW(
        WH_KEYBOARD_LL, // 低级键盘钩子
        // 似乎也能用WH_KEYBOARD，但低级钩子用起来更稳定、简单些
        LowLevelKeyboardProc, // 传递回调函数地址
        GetModuleHandle(NULL),
        0);
    if (global.KeyboardHook == NULL)
    { // 没拿到句柄则失败
        debug::logOutput(L"[初始化]键盘钩子安装失败\n");
        debug::logWinError(GetLastError());
        MessageBoxExW(
            NULL, L"错误：00005，钩子安装失败，请检查杀毒软件是否关闭",
            L"KB - 运行时发生错误", MB_OK | MB_ICONEXCLAMATION, 0); // 消息框提示出错
        // 释放已分配的资源
        return 0;
    }

    // 鼠标钩子
    global.MouseHook = SetWindowsHookExW(
        WH_MOUSE_LL,       // 低级鼠标钩子
        LowLevelMouseProc, // 传递回调函数地址
        GetModuleHandle(NULL),
        0);
    if (global.MouseHook == NULL)
    { // 没拿到句柄则失败
        debug::logOutput(L"[初始化]鼠标钩子安装失败\n");
        debug::logWinError(GetLastError());
        MessageBoxExW(
            NULL, L"错误：00005，钩子安装失败，请检查杀毒软件是否关闭",
            L"KB - 运行时发生错误", MB_OK | MB_ICONEXCLAMATION, 0); // 消息框提示出错
        // 释放已分配的资源
        return 0;
    }

    // =============================================================
    // ||                      消息循环                            ||
    // =============================================================

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}