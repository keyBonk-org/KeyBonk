# 设置代码页为UTF-8以支持中文路径和输出
$(shell chcp 65001 >nul)

# 编译架构选择，默认64位
ARCH ?= 64
# 编译cpp文件时的调试选项
DEBUG ?= -DKB_DEBUG

# 使用cmd为shell并禁止所有隐式规则
SHELL         := cmd
.SUFFIXES:

ifeq ($(ARCH),32)
CXX      = i686-w64-mingw32-g++
WINDRES  = windres
WINDRES_FLAG = -F pe-i386 -o
ARCH_TEXT= x86
else
CXX      = g++
WINDRES  = windres
WINDRES_FLAG = -F pe-x86-64 -o
ARCH_TEXT= x64
endif

# 基本变量
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic -O2 -DUNICODE -D_WIN32_WINNT=0x0601
LDFLAGS  = -mwindows -municode -std=c++17
LDLIBS   = -luser32 -lgdi32 -lole32 -lgdiplus -lwinmm -lmsacm32 -lcomdlg32

# 预编译头文件设置
PCH_FILE := pch.hpp

# 目录变量
SRC_DIR   := src
INC_DIR   := include
RES_DIR   := resource
BUILD_BASE:= build
LIB_DIR   := libs
BUILD_DIR := $(BUILD_BASE)/$(ARCH)
OBJ_DIR   := $(BUILD_DIR)/obj
BIN       := $(BUILD_DIR)/KeyBonk.exe
INCLUDE_DIRS:= -I$(INC_DIR) -I$(LIB_DIR) -I$(RES_DIR)

# 预编译头文件设置（依赖于OBJ_DIR）
PCH_DIR  := $(OBJ_DIR)
PCH_OBJ  := $(PCH_DIR)/$(PCH_FILE).gch

# 源文件列表 
CXX_SRCS := $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/windows/*.cpp) $(wildcard $(SRC_DIR)/hook/*.cpp) $(wildcard $(SRC_DIR)/functions/*.cpp)
RES_SRC  := $(RES_DIR)/resources.rc

# 库文件目录
LIB_INCLUDE := libs
LIB_DOWNLOAD_PATH := $(BUILD_DIR)/libs
AUDIO_LIB_REPO    := keybonk-org/audio-player
AUDIO_LIB := $(LIB_DOWNLOAD_PATH)/audioPlayer_$(ARCH_TEXT).a

# 自动推导对象 
CXX_OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(CXX_SRCS))
RES_OBJ  := $(OBJ_DIR)/rc/resources.o

# 默认目标（64位debug模式）
.PHONY: all clean help run release release64 release32 installer installer64 installer32
all: $(BIN)
	@echo 调试版本构建完成: $@

# 链接 
$(BIN): $(CXX_OBJS) $(RES_OBJ) $(AUDIO_LIB) | $(BUILD_DIR)/bin/default
	@echo 正在链接生成可执行文件 $@ ...
	@$(CXX) $(LDFLAGS) $^ -o $@ $(LDLIBS)

# 预编译头文件生成
$(PCH_OBJ): $(INC_DIR)/$(PCH_FILE) | $(PCH_DIR)
	@echo 正在生成预编译头文件 $@
	@if not exist "$(PCH_DIR)" mkdir "$(PCH_DIR)"
	@$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) -c $(INC_DIR)/$(PCH_FILE) -o $@

# 编译对象文件（使用预编译头）
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR) $(PCH_OBJ)
	@echo 正在将源码文件 $< 编译到 $@
	@if not exist "$(dir $@)" mkdir "$(dir $@)"
	@$(CXX) $(CXXFLAGS) $(DEBUG) $(INCLUDE_DIRS) -include $(PCH_FILE) -MMD -MP -c $< -o $@

# 资源文件 
$(RES_OBJ): $(RES_SRC) ./include/globalDevelopmentControl.hpp | $(OBJ_DIR)/rc
	@echo 正在将RC文件 "$<" 编译到 $@ ...
	@$(WINDRES) $< $(WINDRES_FLAG) $@

# 音频库下载
$(AUDIO_LIB):
	@echo 正在下载/更新 audio-player 库 ...
	@powershell -ExecutionPolicy Bypass -File "download_audio_lib.ps1" -Repo "$(AUDIO_LIB_REPO)" -Arch "$(ARCH_TEXT)" -DownloadDir "$(LIB_DOWNLOAD_PATH)" -TargetFile "$(AUDIO_LIB)"
	@echo 库已就绪。

# 自动依赖 
-include $(CXX_OBJS:.o=.d)

# 目录创建（对象文件目录和资源对象目录）
$(OBJ_DIR):
	@echo 正在创建对象文件目录 "$(OBJ_DIR)"
	@if not exist "$(OBJ_DIR)" mkdir "$(OBJ_DIR)"

$(OBJ_DIR)/rc:
	@echo 正在创建资源对象目录 "$(OBJ_DIR)/rc"
	@if not exist "$(OBJ_DIR)/rc" mkdir "$(OBJ_DIR)/rc"

# 资源文件复制
$(BUILD_DIR)/bin/default:
	@echo 正在将资源文件复制到 "$@":
	@if not exist "$@" mkdir "$@"
	
	@echo == 正在将 "resource\audios" 复制到 "$@\audios\"
	@xcopy /E /Y "resource\audios" "$@\audios\\" >nul
	
	@echo == 正在将 "resource\background.png" 复制到 "$@\imgs"
	@xcopy /Y "resource\background.png" "$@\imgs\\" >nul

	@echo == 正在将 "resource\icon-org.png" 复制到 "$@"
	@xcopy /Y "resource\icon-org.png" "$@" >nul
	
	@echo 资源复制完毕

# 文件清理
clean:
	@echo 正在清理build目录 ...
	@if exist "$(BUILD_BASE)" rmdir /S /Q "$(BUILD_BASE)"
	@echo 正在清理预编译头文件 ...
	@if exist "$(OBJ_DIR)\$(PCH_FILE).gch" del /Q "$(OBJ_DIR)\$(PCH_FILE).gch"
	@echo 清理完成

# 帮助 
help:
	@type .\docs\makefileHelper.txt

# 运行编译结果（默认64位）
run: $(BIN)
	@echo [运行] 运行 $(BIN)
	@$(BIN)

# 构建发布版（64+32）
release: clean release64 release32
	@echo 所有发布版本构建完成

# 发布版64位构建
release64:
	@echo 正在构建64位发布版本 ...
	@$(MAKE) ARCH=64 all DEBUG=

# 发布版32位构建
release32:
	@echo 正在构建32位发布版本 ...
	@$(MAKE) ARCH=32 all DEBUG=

# 构建安装包（64+32）
installer: clean release64 installer64 release32 installer32
	@echo 所有安装程序构建完成

# 64位安装包构建
installer64: installer.iss release64
	@echo 正在构建64位安装程序 ...
	@start cmd /c chcp 936 ^&^& iscc /DMyAppArch=64 installer.iss
	@echo 64位安装程序编译完毕

# 32位安装包构建
installer32: installer.iss release32
	@echo 正在构建32位安装程序 ...
	@start cmd /c chcp 936 ^&^& iscc /DMyAppArch=32 installer.iss
	@echo 32位安装程序编译完毕