# 打包操作文档

> 本文档教你如何把这个背单词软件编译打包，分发给 Windows 和 Linux 用户。

---

## 目录

- [目录](#目录)
- [一、项目结构速览](#一项目结构速览)
- [二、Linux 平台打包](#二linux-平台打包)
  - [2.1 安装依赖](#21-安装依赖)
  - [2.2 一键打包](#22-一键打包)
  - [2.3 分步手动打包](#23-分步手动打包)
- [三、Windows 平台打包](#三windows-平台打包)
  - [3.1 安装 Visual Studio 生成工具](#31-安装-visual-studio-生成工具)
  - [3.2 安装 CMake](#32-安装-cmake)
  - [3.3 安装 raylib (vcpkg)](#33-安装-raylib-vcpkg)
  - [3.4 编译](#34-编译)
- [四、分发说明](#四分发说明)
- [五、常见问题](#五常见问题)

---

## 一、项目结构速览

```
main.c/
├── build.sh              ← Linux 一键打包脚本
├── CMakeLists.txt        ← 跨平台构建配置（已适配 Windows）
├── src/                  ← 源代码
│   ├── main.c
│   ├── core/             ←   核心框架
│   ├── modules/          ←   业务模块
│   └── ui/               ←   UI 层
├── data/                 ← 数据文件（打包时会自动包含）
│   ├── words.txt         ←   单词库
│   ├── accounts.txt      ←   账号
│   └── fonts/            ←   字体
│       ├── DejaVuSans.ttf       ← 英文 + IPA 音标
│       ├── DejaVuSansMono.ttf   ← 等宽后备
│       └── NotoSansCJK.otf      ← 中文
└── docs/                 ← 文档
```

---

## 二、Linux 平台打包

### 2.1 安装依赖

```bash
# Ubuntu / Debian
sudo apt install cmake ninja-build libraylib-dev

# Fedora
sudo dnf install cmake ninja-build raylib-devel

# Arch Linux
sudo pacman -S cmake ninja raylib
```

验证安装：

```bash
cmake --version
ninja --version
ls /usr/local/lib/libraylib.a    # 应该能看到文件
```

### 2.2 一键打包（推荐）

项目根目录有 `build.sh` 脚本，一条命令完成全部工作：

```bash
# 只编译（检查有没有错误）
./build.sh build

# 编译 + 打包到 build/dist/ 目录
./build.sh install

# 编译 + 打包成 .tar.gz 压缩包（可以直接发给别人）
./build.sh package

# 清理构建产物
./build.sh clean
```

`package` 命令执行后会在 `build/` 目录生成：

```
build/背单词软件-v5.0.0-linux.tar.gz    ← 这就是分发包
```

### 2.3 分步手动打包（想自己控制时用）

```bash
# 1. 创建构建目录
mkdir build && cd build

# 2. 配置（cmake 会自动找 raylib）
cmake .. -DCMAKE_BUILD_TYPE=Release

# 3. 编译
cmake --build . -j$(nproc)

# 4. 检查是否静态链接（没有 libraylib 依赖才行）
ldd main_c | grep raylib
# 应该输出空，或显示 "not a dynamic executable"

# 5. 创建分发包
cd ..
mkdir -p dist
cp build/main_c dist/
cp -r data dist/
chmod +x dist/main_c

# 6. 打包
tar czf build/背单词软件-v5.0.0-linux.tar.gz dist/
```

---

## 三、Windows 平台打包

### 3.1 安装 Visual Studio 生成工具

**方法 A：完整 VS 2022（推荐新手）**
1. 访问 https://visualstudio.microsoft.com/zh-hans/downloads/
2. 下载 **Visual Studio 2022 Community**（免费）
3. 运行安装程序，勾选 **"使用 C++ 的桌面开发"**（Desktop development with C++）
4. 安装完成后重启电脑

**方法 B：仅生成工具（体积小）**
1. 访问 https://visualstudio.microsoft.com/zh-hans/downloads/
2. 找到 **Visual Studio 2022 生成工具**（Build Tools）
3. 安装时勾选 **"使用 C++ 的桌面开发"**

验证安装：打开 **"Developer PowerShell for VS 2022"**（开始菜单搜索），输入：
```powershell
cl --version
```

### 3.2 安装 CMake

1. 访问 https://cmake.org/download/
2. 下载 **cmake-x.x-win64-x64.msi**（Windows x64 版本）
3. 安装时勾选 **"Add CMake to system PATH"**
4. 安装完成后，打开新终端验证：
```powershell
cmake --version
```

### 3.3 安装 raylib (使用 vcpkg)

打开 PowerShell（管理员模式），逐条执行：

```powershell
# 切换到你的工作目录（比如 D:\dev）
cd D:\dev

# 克隆 vcpkg（微软的 C++ 包管理器）
git clone https://github.com/microsoft/vcpkg
cd vcpkg

# 执行安装脚本
.\bootstrap-vcpkg.bat

# 安装 raylib 静态库（推荐，编译出的 exe 不需要额外 dll）
.\vcpkg install raylib:x64-windows-static

# 告诉 CMake 以后可以自动找到 vcpkg 安装的包
.\vcpkg integrate install
```

> ⏳ 第一次安装 vcpkg 和 raylib 可能需要 5~10 分钟（下载编译），耐心等待即可。

### 3.4 编译

**注意**：必须在 **"Developer PowerShell for VS 2022"** 中执行，普通 PowerShell 不行。

```powershell
# 打开 "Developer PowerShell for VS 2022"
# 进入项目目录
cd D:\dev\main.c

# 创建构建目录
mkdir build
cd build

# 配置（cmake 自动通过 vcpkg 找到 raylib）
cmake .. -DCMAKE_BUILD_TYPE=Release

# 编译
cmake --build . --config Release
```

编译成功后，可执行文件在：

```
build\Release\main_c.exe
```

验证是否静态链接（不需要 raylib.dll）：

```powershell
# 用 Dependencies 工具检查，或者直接把 exe 复制到另一台电脑试试
# 如果不放心，可以用这个命令看有没有 .dll 依赖
dumpbin /dependents build\Release\main_c.exe | findstr raylib
# 应该没有输出（没有 raylib 相关 dll）
```

### 3.5 打包分发给 Windows 用户

```powershell
# 在 build 目录外执行
mkdir dist
copy build\Release\main_c.exe dist\
xcopy /E data dist\data\

# 打包成 zip（用系统自带的压缩或 7zip）
# 最终文件结构：
dist\
├── main_c.exe
├── data\
│   ├── words.txt
│   ├── accounts.txt
│   ├── plans*.txt
│   └── fonts\
│       ├── DejaVuSans.ttf
│       ├── DejaVuSansMono.ttf
│       └── NotoSansCJK.otf
```

---

## 四、分发说明

### 给用户的文件结构

无论是 Linux 还是 Windows，最终发给用户的就是一个文件夹（或压缩包），结构都一样：

```
背单词软件-v5.0.0/
├── main_c (或 main_c.exe)    ← 主程序
├── 使用说明.txt
└── data/
    ├── words.txt             ← 词库（用户可用记事本编辑）
    ├── accounts.txt          ← 账号
    └── fonts/                ← 字体（不要动）
```

### 用户运行方式

| 平台 | 操作 |
|------|------|
| **Linux** | 终端进入目录，执行 `./main_c` |
| **Windows** | 双击 `main_c.exe` |

不需要安装任何东西，不需要装 raylib，解压即用。

### 注意事项

1. **Linux 上静态链接**：CMakeLists.txt 会自动优先找 `libraylib.a` 做静态链接，生成的 `main_c` 不依赖 raylib.so，大小约 1.2MB
2. **Windows 上静态链接**：用 `raylib:x64-windows-static` 包即可，生成的 `main_c.exe` 不依赖 raylib.dll
3. **如果 raylib 版本不同**：确保 vcpkg 安装的 raylib 版本与你开发环境一致（目前用的是 raylib 6.0）

---

## 五、常见问题

### Q: 编译时报 "找不到 raylib.h"

**原因**：cmake 没有找到 raylib 头文件。

**解决方法**：
- **Linux**：`sudo apt install libraylib-dev`
- **Windows**：确认 vcpkg 已安装并且 `vcpkg integrate install` 已执行
- 也可以手动指定路径：`cmake .. -DCMAKE_PREFIX_PATH=/path/to/raylib`

### Q: 编译时报 "multiple definition of `Lerp`"

**原因**：raylib_word_ui.c 中定义的 `Lerp` 函数与 raylib 静态库中的同名函数冲突。

**解决方法**：打开 `src/ui/raylib_word_ui.c`，将 `float Lerp(...)` 改为 `static float Lerp(...)`。

### Q: 运行时报 "cannot open shared object file: libraylib.so"

**原因**：没有做静态链接，编译时链接了动态库。

**解决方法**：
- **Linux**：检查 `/usr/local/lib/libraylib.a` 是否存在，重新运行 `cmake ..` 会优先用静态库
- **Windows**：用 vcpkg 安装 `raylib:x64-windows-static` 重编

### Q: Windows 上 "VCRUNTIME140.dll 丢失"

**原因**：缺少 VC++ 运行库。

**解决方法**：安装 Visual C++ Redistributable：
https://aka.ms/vs/17/release/vc_redist.x64.exe

### Q: 中文显示为方框

**原因**：`data/fonts/NotoSansCJK.otf` 字体文件缺失。

**解决方法**：确认 `data/fonts/` 目录下有 `NotoSansCJK.otf` 文件。如果没有，从项目仓库下载或从系统复制一份中文字体（如 `simhei.ttf`）放进去。
