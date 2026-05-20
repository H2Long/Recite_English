# 背单词软件 (Recite English)

基于 C + raylib 的桌面端英语单词记忆应用。轻量、单文件、跨平台。

## 功能

- **卡片背单词** — 闪卡模式，显示单词/音标/释义/例句，支持翻转
- **选词背单词** — 从词库中自选单词进行学习
- **测试模式** — 选择题形式的单词测验
- **单词搜索** — 关键词搜索词库
- **学习计划** — 设定每日学习目标
- **账号系统** — 多用户切换，独立保存学习进度
- **词库管理** — 内置 220 个 CET-4 高频词汇，可自定义词库文件

## 下载

前往 [Releases](https://github.com/H2Long/Recite_English/releases) 下载最新版本：

| 平台 | 文件 |
|------|------|
| Linux | `ReciteEnglish-x86_64.AppImage` |
| Windows | `ReciteEnglish-windows-x86_64.zip` |

### Linux 运行

```bash
chmod +x ReciteEnglish-x86_64.AppImage
./ReciteEnglish-x86_64.AppImage
```

如果无法启动，安装 libfuse2：

```bash
sudo apt install libfuse2
```

## 构建

### 依赖

- CMake >= 3.10
- GCC / Clang
- [raylib](https://github.com/raysan5/raylib) >= 6.0（静态链接）

### 编译

```bash
git clone https://github.com/H2Long/Recite_English.git
cd Recite_English
mkdir build && cd build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja
./main_c
```

### 打包 AppImage

```bash
./build_appimage.sh
# 产出: build/背单词软件-x86_64.AppImage
```

## 项目结构

```
src/
├── main.c                  # 入口
├── core/                   # 应用状态、树形菜单
├── modules/                # 账号、单词、计划、字体
└── ui/
    ├── raylib_word_ui.c    # UI 组件库（按钮/卡片/搜索框/滚动视图）
    ├── menu_callbacks.c    # 菜单路由
    └── pages/              # 各页面实现
data/
├── words.txt               # 词库（可自定义）
├── fonts/                  # 字体文件
├── accounts.txt            # 账号数据
├── plans.txt               # 学习计划
└── progress.txt            # 学习进度
```

## 用户数据

AppImage 运行时，用户进度保存在 `~/.local/share/recite-english/data/`。更新 AppImage 版本不会丢失学习数据。

## 技术栈

- **语言**: C11
- **图形**: raylib 6.0（OpenGL 后端）
- **构建**: CMake + Ninja
- **CI/CD**: GitHub Actions（Linux AppImage / Windows zip）
