// ===./include/about.hpp===
// 关于窗口相关功能声明

#ifndef ABOUT_H
#define ABOUT_H

#include <windows.h>
#include <gdiplus.h>
#include <memory>
#include "global.hpp"

// 函数声明
void aboutWindowOpen();                                                                // 打开关于窗口
LRESULT CALLBACK WindowProc_about(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam); // 关于窗口消息处理

#endif // ABOUT_H