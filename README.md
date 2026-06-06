<!-- markdownlint-disable -->

<div align="center">

<img alt="LOGO" src="./resource/icon.png" width="256" height="256" />

# 发声键盘 - KeyBonk

V 1.3.0.1 （基础版）

<br>
<div>
    <img alt="C++" src="https://img.shields.io/badge/C++-17-%2300599C?logo=cplusplus">
</div>
<div>
    <img alt="platform" src="https://img.shields.io/badge/platform-Windows-blueviolet">
</div>
<div>
    <img alt="license" src="https://img.shields.io/github/license/xiaoditx/KeyBonk">
    <img alt="commit" src="https://img.shields.io/github/commit-activity/m/xiaoditx/KeyBonk">
</div>
<div>
    <img alt="stars" src="https://img.shields.io/github/stars/xiaoditx/KeyBonk?style=social">
    <img alt="GitHub all releases" src="https://img.shields.io/github/downloads/xiaoditx/KeyBonk/total?style=social">
</div>
</div>
<br>
<div>
    <a href="#开发信息">贡献者帮助信息</a>
</div>
<br>

<!-- markdownlint-restore -->

基于曾经做过的唯一能入眼的软件“坤音键盘”的4.0版本重构的C++版本，原版公开了源码但没有传到GitHub，因为那是一个易语言项目，源码只有一个文件所以似乎没有什么开到GitHub的必要（况且GitHub默认不识别易语言，虽然我们可以手动设定但我不喜欢）

### 重构的变动

重构将语言换为了C++，使得运行效率得到了提升，同时抛弃了原本使用注册热键实现的极为愚蠢的方式，改用了键盘钩子作为了监视方法。

这是我接触Win32开发的第一个项目，算是我练手用的了，希望各位可以喜欢

另外，目前有使用Godot重构的打算，但考虑到C++版本还没写完（且我不会Godot），这个重构会暂缓。

本次重构衍生出了项目[audio-player](https://github.com/xiaoditx/audio-player)，但..额...好吧我似乎有点高估自己了，这个项目一直在卡着导致我现在没心思继续往下写了，后续更新随缘吧

### 安装

软件安装包在[Release](https://github.com/xiaoditx/KeyBonk/releases)页面，请选择你想要的版本，推荐使用最新版的前一个版本

软件仅支持Windows，提供64位和32位两个版本

### 使用说明

打开软件，软件将监测按键，默认检测`j`、`n`、`t`、`m`四个按键，按下播放对应音频

音频文件在`./bin/default/audios`下，文件名为想要监测的按键的虚拟键值+“.wav”（仅支持wav）。有关虚拟键值，想要获取可以使用我在BeepMusic中使用过的工具：[键值查询工具](https://github.com/xiaoditx/BeepMusic/tree/main/tools/XD%E5%BC%80%E5%8F%91%E5%B7%A5%E5%85%B7-T1)。也可以查阅[微软文档](https://learn.microsoft.com/zh-cn/windows/win32/inputdev/virtual-key-codes)

背景文件可更换，简单的方法是使用自己的图片替换掉`./bin/default/background.png`即可，目前测试仅证明了png有效。当前版本不支持自动检测图片尺寸，不支持颜色过滤自定义（当前版本将十进制色号13217535的颜色设为透明）

可以自建一个文件夹（推荐作为`./bin/`的子文件夹），在内部自行搭建audios文件夹并自行放置background.png，想要让软件识别该文件夹，当前版本唯一有效的方法是修改config.ini，在文件末尾添加：

```text
[settings]
lib=（你自己创建的文件夹路径比如.\bin\default）
```

### 开发信息

项目文档：[docs](./docs)

软件技术栈

- win32
- C++17
- GDI+
- ~~COM库~~（计划引入）

开发环境

- Windows10-x64
- MSYS2（MingGW64）
- VScode
- GNU Make（4.4.1）

## 贡献者

感谢社区的贡献者们（其实现在并没有）

<a href="https://github.com/xiaoditx/keybonk/graphs/contributors">
  <img src="https://contrib.rocks/image?repo=xiaoditx/keybonk" />
</a>

## 项目统计

<a href="https://www.star-history.com/#xiaoditx/keybonk&type=date&legend=top-left">
 <picture>
   <source media="(prefers-color-scheme: dark)" srcset="https://api.star-history.com/svg?repos=xiaoditx/keybonk&type=date&theme=dark&legend=top-left" />
   <source media="(prefers-color-scheme: light)" srcset="https://api.star-history.com/svg?repos=xiaoditx/keybonk&type=date&legend=top-left" />
   <img alt="Star History Chart" src="https://api.star-history.com/svg?repos=xiaoditx/keybonk&type=date&legend=top-left" />
 </picture>
</a>

![Alt](https://repobeats.axiom.co/api/embed/148eacdfa3e1268ce951923b74ab7f6ed8d8a9e7.svg "Repobeats analytics image")
