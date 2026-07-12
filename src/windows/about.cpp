// ===./src/about.cpp===
// 关于窗口相关功能实现

#include "windows/about.hpp"
#include "global.hpp"
#include "globalDevelopmentControl.hpp"
#include "resources.hpp"

// 打开“关于”窗口
void aboutWindowOpen()
{
    using keybonk::global;

    // 检查关于窗口是否已经存在（窗口只能存在一次）
    if (global.hwndAbout != NULL && IsWindow(global.hwndAbout))
    {
        // 检查窗口是否被最小化或隐藏
        if (IsIconic(global.hwndAbout))
        {
            // 恢复窗口
            ShowWindow(global.hwndAbout, SW_RESTORE);
        }
        else if (!IsWindowVisible(global.hwndAbout))
        {
            // 显示窗口
            ShowWindow(global.hwndAbout, SW_SHOW);
        }
        // 激活窗口并设置为前台
        SetForegroundWindow(global.hwndAbout);
        return;
    }

    // 注册窗口类
    const wchar_t CLASS_NAME[] = L"KeyBonk“关于”窗口";
    WNDCLASSEX wc = {};                // 用0初始化整个WindowClass
    wc.cbSize = sizeof(WNDCLASSEX);    // 设置结构体大小
    wc.lpfnWndProc = WindowProc_about; // 指定WindowProc_about函数
    wc.hInstance = global.hInstance;
    wc.lpszClassName = CLASS_NAME; // 窗口类名称
    wc.hIcon = (HICON)LoadImage(global.hInstance, MAKEINTRESOURCE(IDI_MY_ICON), IMAGE_ICON, 64, 64, 0);
    wc.hIconSm = (HICON)LoadImage(global.hInstance, MAKEINTRESOURCE(IDI_MY_ICON), IMAGE_ICON, 64, 64, 0); // 小图标（窗口标题栏）
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClassEx(&wc); // 注册

    global.hwndAbout = CreateWindowExW(
        WS_EX_APPWINDOW, CLASS_NAME, L"关于 KeyBonk",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 600, 400,
        NULL, NULL, global.hInstance, NULL);

    // 创建失败则提示并返回，结束运行
    if (global.hwndAbout == NULL)
    {
        MessageBoxExW(
            NULL, L"错误：00002，创建窗口时发生异常，请检查系统各项设置是否正常",
            L"KB - 运行时发生错误", MB_OK | MB_ICONEXCLAMATION, 0); // 消息框提示出错
    }
    ShowWindow(global.hwndAbout, global.nCmdShow);
    UpdateWindow(global.hwndAbout);
}

// “关于”窗口使用：控件ID定义
// （这些内容仅在此处出现，不独立放入头文件）
#define IDC_LOGO 1001          // 显示软件logo的图片框
#define IDC_SOFTWARE_NAME 1002 // 软件名（标签）
#define IDC_VERSION 1003       // 软件版本（标签）
#define IDC_AUTHOR 1004        // 作者（标签）
#define IDC_DESCRIPTION 1005   // 描述（标签）
#define IDC_CLOSE_BUTTON 1006  // 关闭按钮
#define IDC_WEBSITE_LINK 1007  // 网页链接（标签）

// 关于窗口消息处理
LRESULT CALLBACK WindowProc_about(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    using keybonk::global;

    switch (uMsg)
    {
    // 窗口创建完毕，绘制组件
    case WM_CREATE:
    {
        // 获取客户区大小
        RECT rect;
        GetClientRect(hwnd, &rect);
        int clientWidth = rect.right - rect.left;  // 客户区宽度
        int clientHeight = rect.bottom - rect.top; // 客户区高度

        // 获取程序图标位置
        auto tmp_path = std::make_unique<wchar_t[]>(MAX_PATH);
        swprintf_s(tmp_path.get(),
                   MAX_PATH,
                   L"%ls\\icon-org.png",
                   global.audioLibPath);

        // 创建Logo图标（静态图片控件）
        HWND hLogo = CreateWindowExW(
            0, L"STATIC", L"",
            WS_CHILD | WS_VISIBLE | SS_BITMAP | SS_CENTERIMAGE,
            20, 20, 96, 96,
            hwnd, (HMENU)IDC_LOGO, global.hInstance, NULL);

        // 使用GDI+加载PNG图片作为Logo
        Gdiplus::Image *pLogoImage = Gdiplus::Image::FromFile(tmp_path.get());
        if (pLogoImage && pLogoImage->GetLastStatus() == Gdiplus::Ok)
        {
            // 创建一个位图来绘制图片
            HDC hdc = GetDC(NULL);
            HDC hMemDC = CreateCompatibleDC(hdc);
            HBITMAP hBitmap = CreateCompatibleBitmap(hdc, 96, 96);
            SelectObject(hMemDC, hBitmap);

            // 绘制背景为白色（与窗口颜色一致）
            RECT rect = {0, 0, 96, 96};
            FillRect(hMemDC, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));

            // 绘制PNG图片
            Gdiplus::Graphics graphics(hMemDC);
            graphics.DrawImage(pLogoImage, 0, 0, 96, 96);

            // 设置位图到静态控件
            SendMessageW(hLogo, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);

            // 释放资源
            delete pLogoImage;
            DeleteDC(hMemDC);
            ReleaseDC(NULL, hdc);
        }

        // 创建软件名称文本
        CreateWindowExW(0, L"STATIC", L"KeyBonk",
                        WS_CHILD | WS_VISIBLE | SS_CENTER | SS_NOPREFIX,
                        130, 20, clientWidth - 150, 30,
                        hwnd, (HMENU)IDC_SOFTWARE_NAME, global.hInstance, NULL);

        // 创建软件版本文本
        CreateWindowExW(0, L"STATIC", L"版本: v" w_appVersion,
                        WS_CHILD | WS_VISIBLE | SS_LEFT | SS_NOPREFIX,
                        130, 60, clientWidth - 150, 20,
                        hwnd, (HMENU)IDC_VERSION, global.hInstance, NULL);

        // 创建软件作者文本
        CreateWindowExW(0, L"STATIC", L"作者: 小狄同学呀",
                        WS_CHILD | WS_VISIBLE | SS_LEFT | SS_NOPREFIX,
                        130, 90, clientWidth - 150, 20,
                        hwnd, (HMENU)IDC_AUTHOR, global.hInstance, NULL);

        // 创建软件介绍文本
        CreateWindowExW(0, L"STATIC",
                        L"    KeyBonk是一款键盘音效软件，为了让您收获更优秀的按键效果。"
                        L"软件基于旧版由易语言开发的软件坤音键盘进行重开发，运行效率等方面均有所提升\r\n"
                        L"    当前版本的变化：" versionChange,
                        WS_CHILD | WS_VISIBLE | SS_LEFT | SS_NOPREFIX,
                        20, 130, clientWidth - 40, 120, // 增加高度以容纳更多内容
                        hwnd, (HMENU)IDC_DESCRIPTION, global.hInstance, NULL);

        // 创建个人网站链接
        CreateWindowExW(WS_EX_TRANSPARENT, L"STATIC", L"访问项目官网: https://keyBonk-org.github.io/",
                        WS_CHILD | WS_VISIBLE | SS_CENTER | SS_NOTIFY | SS_NOPREFIX,
                        20, 270, clientWidth - 40, 30, // 向下移动位置
                        hwnd, (HMENU)IDC_WEBSITE_LINK, global.hInstance, NULL);

        // 创建关闭按钮
        CreateWindowExW(0, L"BUTTON", L"关闭",
                        WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                        clientWidth - 100, clientHeight - 50, 80, 30,
                        hwnd, (HMENU)IDC_CLOSE_BUTTON, global.hInstance, NULL);

        // 设置文本控件的字体
        // 软件名的字体设置
        HFONT hFont = CreateFontW(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                  DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
                                  CLEARTYPE_QUALITY, VARIABLE_PITCH, L"微软雅黑");
        SendMessageW(GetDlgItem(hwnd, IDC_SOFTWARE_NAME), WM_SETFONT, (WPARAM)hFont, TRUE);

        // 其余文本组件的字体设置
        HFONT hNormalFont = CreateFontW(24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                        DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
                                        CLEARTYPE_QUALITY, VARIABLE_PITCH, L"微软雅黑");
        SendMessageW(GetDlgItem(hwnd, IDC_VERSION), WM_SETFONT, (WPARAM)hNormalFont, TRUE);
        SendMessageW(GetDlgItem(hwnd, IDC_AUTHOR), WM_SETFONT, (WPARAM)hNormalFont, TRUE);
        SendMessageW(GetDlgItem(hwnd, IDC_DESCRIPTION), WM_SETFONT, (WPARAM)hNormalFont, TRUE);
        SendMessageW(GetDlgItem(hwnd, IDC_WEBSITE_LINK), WM_SETFONT, (WPARAM)hNormalFont, TRUE);

        // 关闭按钮字体设置
        SendMessageW(GetDlgItem(hwnd, IDC_CLOSE_BUTTON), WM_SETFONT, (WPARAM)hNormalFont, TRUE);

        // 设置链接文本颜色为蓝色
        // 使用WM_CTLCOLORSTATIC消息来处理文本颜色，这里先设置控件的背景模式
        SetWindowLongW(GetDlgItem(hwnd, IDC_WEBSITE_LINK), GWL_STYLE,
                       GetWindowLongW(GetDlgItem(hwnd, IDC_WEBSITE_LINK), GWL_STYLE) | SS_NOTIFY);

        return 0;
    }

    case WM_SYSCOMMAND:
    {
        // 移除自定义的最小化处理，使用默认行为，让窗口正常显示在任务栏
        break;
    }

    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
        case IDC_CLOSE_BUTTON:
            // 关闭按钮点击事件
            DestroyWindow(hwnd);
            return 0;

        case IDC_WEBSITE_LINK:
            // 网站链接点击事件
            ShellExecuteW(NULL, L"open", L"https://keyBonk-org.github.io/", NULL, NULL, SW_SHOWNORMAL);
            return 0;
        }
        break;
    }

    case WM_DESTROY:
        DestroyWindow(global.hwndAbout);
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
        EndPaint(hwnd, &ps);
    }
        return 0;

    case WM_SETCURSOR:
    {
        // 为网站链接添加手型光标
        if (LOWORD(lParam) == HTCLIENT)
        {
            HWND hControl = (HWND)wParam;
            if (hControl == GetDlgItem(hwnd, IDC_WEBSITE_LINK))
            {
                SetCursor(LoadCursor(NULL, IDC_HAND));
                return TRUE;
            }
        }
        break;
    }

    case WM_LBUTTONDOWN:
    {
        // 处理链接的点击事件（确保SS_NOTIFY生效）
        POINT pt;
        GetCursorPos(&pt);
        ScreenToClient(hwnd, &pt);

        HWND hLink = GetDlgItem(hwnd, IDC_WEBSITE_LINK); // 获取链接窗口句柄
        RECT linkRect;                                   // 用于存放链接位置的变量
        GetWindowRect(hLink, &linkRect);
        ScreenToClient(hwnd, (POINT *)&linkRect);
        ScreenToClient(hwnd, (POINT *)&linkRect.right);

        if (PtInRect(&linkRect, pt)) // 如果点击在链接的位置内
        {
            // 打开网站
            ShellExecuteW(NULL, L"open", L"https://keyBonk-org.github.io/", NULL, NULL, SW_SHOWNORMAL);
        }
        break;
    }

    case WM_CTLCOLORSTATIC:
    {
        HWND hWndCtrl = (HWND)lParam;
        HDC hdcCtrl = (HDC)wParam;

        if (hWndCtrl == GetDlgItem(hwnd, IDC_WEBSITE_LINK))
        {
            // 设置文本颜色为蓝色
            SetTextColor(hdcCtrl, RGB(0, 0, 255));
            // 设置背景模式为透明
            SetBkMode(hdcCtrl, TRANSPARENT);
            // 返回与窗口相同的背景刷子
            return (LRESULT)(HBRUSH)GetStockObject(WHITE_BRUSH);
        }
        else
        {
            // 对于其他所有文本控件，设置与窗口相同的背景色
            SetBkColor(hdcCtrl, RGB(255, 255, 255));
            SetTextColor(hdcCtrl, RGB(0, 0, 0));
            // 返回与窗口相同的背景刷子
            return (LRESULT)(HBRUSH)GetStockObject(WHITE_BRUSH);
        }
        break;
    }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}