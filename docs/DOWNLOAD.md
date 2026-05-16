# 下载安装指南

> 背单词软件 — 轻量级桌面英语单词记忆应用

---

## 目录

- [下载](#下载)
- [Linux 安装](#linux-安装)
- [Windows 安装](#windows-安装)
- [常见问题](#常见问题)

---

## 下载

### 方式一：GitHub Releases（推荐）

前往 [GitHub Releases 页面](https://github.com/H2Long/Recite_English/releases)，下载最新版本：

| 平台 | 文件 | 说明 |
|------|------|------|
| Linux | `背单词软件-x86_64.AppImage` | 单文件，无需安装，双击运行 |
| Windows | `背单词软件-Setup-v5.0.0.exe` | 标准安装程序，带安装向导 |

### 方式二：从源码编译

如果你需要最新开发版本，可以自行编译。参见下方各平台的编译说明。

---

## Linux 安装

### 方法 A：AppImage（推荐，最简单）

**第 1 步：下载**

从 GitHub Releases 下载 `背单词软件-x86_64.AppImage`

**第 2 步：添加执行权限**

打开终端，进入下载目录，执行：

```bash
chmod +x 背单词软件-x86_64.AppImage
```

或者在文件管理器中：右键 → 属性 → 权限 → 勾选"允许作为程序执行"

**第 3 步：运行**

```bash
./背单词软件-x86_64.AppImage
```

或直接双击文件图标。

> AppImage 是单个文件，不需要安装，不修改系统，删除文件即卸载。

### 方法 B：从源码编译

**第 1 步：安装依赖**

```bash
# Ubuntu / Debian
sudo apt install cmake ninja-build libraylib-dev

# Fedora
sudo dnf install cmake ninja-build raylib-devel

# Arch Linux
sudo pacman -S cmake ninja raylib
```

**第 2 步：克隆项目**

```bash
git clone https://github.com/H2Long/Recite_English.git
cd Recite_English
```

**第 3 步：编译运行**

```bash
# 一键编译
./build.sh build

# 运行
./build/main_c
```

**第 4 步（可选）：打包成 AppImage**

```bash
./build_appimage.sh
# 产出: build/背单词软件-x86_64.AppImage
```

---

## Windows 安装

### 方法 A：安装程序（推荐，最简单）

**第 1 步：下载**

从 GitHub Releases 下载 `背单词软件-Setup-v5.0.0.exe`

**第 2 步：运行安装程序**

双击 `背单词软件-Setup-v5.0.0.exe`，按向导操作：

1. 点击"下一步"接受安装协议
2. 选择安装目录（默认 `C:\Program Files\ReciteEnglish`）
3. 点击"安装"
4. 安装完成后，桌面和开始菜单会出现"背单词软件"快捷方式

**第 3 步：运行**

- 双击桌面的"背单词软件"图标
- 或从开始菜单找到"背单词软件"

**卸载**：控制面板 → 程序和功能 → 找到"背单词软件" → 卸载

### 方法 B：从源码编译（MSYS2，推荐开发者）

**第 1 步：安装 MSYS2**

下载安装 [MSYS2](https://www.msys2.org/)

**第 2 步：安装工具链**

打开 MSYS2 UCRT64 终端，执行：

```bash
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-raylib
```

**第 3 步：克隆并编译**

```bash
git clone https://github.com/H2Long/Recite_English.git
cd Recite_English
mkdir build && cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

**第 4 步：运行**

```bash
./main_c
```

或回到项目根目录，把 `data/` 文件夹和 `build/main_c.exe` 放在一起，双击 `main_c.exe`。

### 方法 C：从源码编译（Visual Studio 2022）

**第 1 步：安装 Visual Studio 2022**

下载 [Visual Studio 2022 Community](https://visualstudio.microsoft.com/zh-hans/downloads/)，安装时勾选"使用 C++ 的桌面开发"。

**第 2 步：安装 CMake**

下载 [CMake](https://cmake.org/download/)，安装时勾选"Add CMake to system PATH"。

**第 3 步：安装 raylib（vcpkg）**

打开管理员 PowerShell：

```powershell
cd D:\dev
git clone https://github.com/microsoft/vcpkg
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg install raylib:x64-windows-static
.\vcpkg integrate install
```

**第 4 步：编译**

打开"Developer PowerShell for VS 2022"：

```powershell
cd D:\dev\Recite_English
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

**第 5 步：运行**

```powershell
.\Release\main_c.exe
```

---

## 常见问题

### Linux

**Q: 双击 AppImage 没反应？**

在终端中运行，查看错误信息：

```bash
chmod +x 背单词软件-x86_64.AppImage
./背单词软件-x86_64.AppImage
```

**Q: 提示 "FUSE is not installed"？**

AppImage 需要 FUSE 支持：

```bash
# Ubuntu / Debian
sudo apt install fuse libfuse2

# Fedora
sudo dnf install fuse-libs
```

**Q: 中文显示为方框？**

AppImage 自带字体，不需要额外安装。如果从源码编译，请确认 `data/fonts/NotoSansCJK.otf` 存在。

### Windows

**Q: 提示 "VCRUNTIME140.dll 丢失"？**

安装 [Visual C++ Redistributable](https://aka.ms/vs/17/release/vc_redist.x64.exe)

**Q: 安装程序被杀毒软件拦截？**

这是因为程序没有数字签名。选择"允许运行"即可。如果你不放心，可以从源码编译。

**Q: 如何自定义词库？**

编辑安装目录下的 `data/words.txt`，格式为：

```
word|音标|释义|例句|例句翻译
```

每行一个单词，用 `|` 分隔各字段。

---

## 数据文件说明

| 文件 | 说明 |
|------|------|
| `data/words.txt` | 单词库，纯文本，可用记事本编辑 |
| `data/accounts.txt` | 账号数据 |
| `data/progress*.txt` | 学习进度（每个用户独立） |
| `data/plans*.txt` | 学习计划（每个用户独立） |
| `data/fonts/` | 字体文件 |

> 提示：备份 `data/` 文件夹即可迁移所有数据到新电脑。
