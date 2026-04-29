# 项目变更记录

> 本文档记录项目的所有重要变更，包括功能增减、架构调整、重构等。
> 每次重大修改后必须更新此文档。

---

## 版本信息

| 项目 | 内容 |
|------|------|
| 项目名称 | raylib-word |
| 当前版本 | v2.0.0 |
| 创建日期 | 2025-01-26 |
| 最近更新 | 2026-04-29 |

---

## 更新日志

### v2.0.0 (2026-04-29) - 账号管理系统

#### 新增内容

1. **独立账号管理库** - `account.c` / `account.h`
   - 独立模块，可脱离主程序单独编译使用
   - 提供完整的用户管理 API：`Account_Init`、`Account_Register`、`Account_Login`、`Account_Logout`
   - 用户数据持久化存储于 `accounts.txt`（管道分隔格式）
   - 密码使用 djb2 哈希存储，支持最多 50 个用户

2. **多用户学习进度隔离** - `words.c`
   - 新增 `setProgressFilePath()` 函数，支持动态切换进度文件
   - 每用户独立进度文件：`progress_<username>.txt`
   - 登录后自动加载对应用户进度，登出恢复默认

3. **账号管理 UI** - `menu_callbacks.c`
   - **账号页面** `MenuAccount_Show()`：显示用户信息、注册时间、登出/切换账号
   - **登录页面** `MenuLogin_Show()`：用户名/密码输入、登录按钮、跳转注册
   - **注册页面** `MenuRegister_Show()`：用户名/密码/确认密码输入、注册按钮
   - 登录/注册提示信息反馈

4. **导航菜单扩展** - `tree_menu.h`
   - `MAX_CHILD_NUM` 从 5 增加到 8
   - 侧边导航新增"账号管理"菜单项

#### 文件清单

| 文件 | 类型 | 说明 |
|------|------|------|
| `account.h` | 新增 | 账号库头文件，定义 User/AccountState 结构和公共 API |
| `account.c` | 新增 | 账号库实现，用户注册/登录/登出/数据持久化 |
| `app_state.h` | 修改 | 添加 AccountState 字段和访问器声明 |
| `app_state.c` | 修改 | 添加账号状态访问器实现 |
| `words.h` | 修改 | 添加 setProgressFilePath 声明 |
| `words.c` | 修改 | 动态进度文件路径支持 |
| `menu_callbacks.c` | 修改 | 新增三个账号页面函数 |
| `menu_callbacks.h` | 修改 | 新增账号页面函数声明 |
| `main.c` | 修改 | 添加 Account_Init 调用 |
| `CMakeLists.txt` | 修改 | 添加 account.c/h |
| `tree_menu.h` | 修改 | MAX_CHILD_NUM 5→8 |

---

### v1.5.0 (2026-04-29) - 退格键支持与光标修正

#### 新增内容

1. **退格键持续删除** - `raylib_word_ui.c`
   - 搜索框支持 BACKSPACE 键删除已输入字符
   - 采用 `GetKeyPressed()` + `IsKeyPressed()` 双重检测确保可靠性

2. **字体扩展** - `fonts.c`
   - `allChinese` 字符串新增约 300 个常用汉字
   - 新增中文标点符号：。，、：；？！“”‘’（）【】《》——……

#### 修复内容

1. **`utf8_delete_left` 边界条件修复** - `raylib_word_ui.c:344`
   - while 循环中 `>` 改为 `>=`，修复光标位于字符串末尾时删除无效的 bug
   - 原因：`prev_byte + len == byte_offset` 时循环多走一步，导致 `memmove` 从自己拷贝到自己

2. **文本框焦点 ID 修复** - `raylib_word_ui.c:692`
   - 将 `UIGetID(state->buffer)` 改为 `(int)(unsigned long)(void*)state`
   - 原因：空 buffer 时 `UIGetID("")` 返回 0，与 `focusItem` 初值相同，造成永久焦点假象

3. **光标位置精度修复** - `raylib_word_ui.c`
   - 光标绘制时 `MeasureTextEx(style->font, ...)` 改为 `MeasureTextAuto(...)`
   - 原因：单一字体测量混合文本（中/英/IPA）导致光标偏移

---

### v1.4.0 (2025-01-26) - 深色模式支持

#### 新增内容

1. **新增设置页面** - `MenuSettings_Show()`
   - 主题设置区域，支持深色/浅色模式切换
   - 关于信息区域，显示版本信息
   - 复选框点击实时切换主题，无需重启

2. **主题切换功能** - `app_state.c/h`
   - 新增 `isDarkMode` 字段记录当前主题
   - 新增 `AppState_IsDarkMode()` 获取当前模式
   - 新增 `AppState_ToggleDarkMode()` 切换模式
   - 新增 `AppState_SetDarkMode(bool)` 设置模式

3. **菜单系统更新**
   - 设置页面作为根菜单的子项
   - 左侧导航栏显示"设置"菜单项
   - 支持通过 `GetMenuItemText()` 获取菜单名称

#### 深色模式主题色

| 颜色角色 | 浅色模式 | 深色模式 |
|----------|----------|----------|
| 背景色 | #F5F5F5 | #1E1E1E |
| 面板色 | #FFFFFF | #2D2D2D |
| 主文字 | #1E1E1E | #F0F0F0 |
| 次文字 | #787878 | #B4B4B4 |
| 错误色 | #DC143C | #FF5050 |
| 成功色 | #32CD32 | #50C850 |

---

### v1.3.0 (2025-01-26) - 配置常量统一

#### 修改内容

1. **新增 `config.h`** - 集中管理所有硬编码常量
   - 窗口配置（尺寸、标题、帧率）
   - 颜色主题（基础色、主题色、透明度）
   - 学习参数（掌握阈值、延迟、每日目标）
   - UI 布局（边距、圆角、字体大小、按钮尺寸）
   - 测试配置（最大题目数、选项数）
   - 文件路径（单词文件、进度文件、字体文件）

2. **修改 `app_state.h`**
   - 移除重复的 `SCREEN_WIDTH`/`SCREEN_HEIGHT` 定义
   - 改为 `#include "config.h"`

3. **修改 `menu_callbacks.h`**
   - 移除重复的 `SCREEN_WIDTH`/`SCREEN_HEIGHT` 定义
   - 改为 `#include "config.h"`

#### 配置常量分类

```c
// config.h 中的配置分类

// 窗口配置
#define SCREEN_WIDTH  1600
#define SCREEN_HEIGHT 1000

// 颜色主题
#define COLOR_RED     (Color){255,   0,   0, 255}
#define THEME_SUCCESS (Color){  0, 200,  83, 255}
#define THEME_PRIMARY (Color){ 30, 144, 255, 255}

// 学习参数
#define MASTERED_THRESHOLD     3       // 认识3次算已掌握
#define DEFAULT_CARD_DELAY_MS  2000    // 卡片延迟2秒
#define DAILY_GOAL_DEFAULT     20      // 每日目标20个

// UI 布局
#define PADDING_MEDIUM    20
#define CORNER_RADIUS_SMALL  5
#define FONT_SIZE_NORMAL  20
```

#### 使用方法

```c
// 在任何 .c/.h 文件中，只需包含 config.h 即可使用所有配置
#include "config.h"

// 使用示例
InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
DrawRectangle(0, 0, 100, 100, THEME_PRIMARY);
if (knownCount >= MASTERED_THRESHOLD) { ... }
```

---

### v1.2.0 (2025-01-26) - 统一状态管理重构

#### 修改内容

1. **新增 `app_state.c/h`** - 统一状态管理模块
   - 将散落在 main.c 和 menu_callbacks.h 中的全局变量整合到 AppState 结构体
   - 提供访问器函数访问各子模块状态
   - 结构体组成：
     - `style`: UI 样式（颜色、字体大小等）
     - `uiState`: UI 交互状态（鼠标位置、按键状态）
     - `rootMenu/currentMenu/menuStack`: 菜单系统状态
     - `learn`: 学单词模式状态（当前索引、过滤器、滚动位置）
     - `review`: 背单词模式状态（复习列表、闪卡状态、本轮统计）
     - `test`: 测试模式状态（题目列表、答题状态、正确率）

2. **修改 `menu_callbacks.c/h`**
   - 移除 extern 全局变量声明（从约 25 个减少到 0 个）
   - 改用访问器函数获取状态
   - 定义简化宏：
     ```c
     #define STYLE    (AppState_GetStyle())
     #define LEARN    (*AppState_GetLearnState())
     #define REVIEW   (*AppState_GetReviewState())
     #define TEST     (*AppState_GetTestState())
     ```

3. **修改 `main.c`**
   - 移除所有散落的全局变量定义
   - 使用 `AppState_Init()` 初始化所有状态
   - 使用 `AppState_Deinit()` 清理资源
   - 代码量从 168 行减少到 120 行

4. **保留不变**
   - `fonts.c/h` - 字体渲染逻辑完全不变
   - `words.c/h` - 单词数据管理完全不变
   - `raylib_word_ui.c/h` - UI 组件完全不变
   - `tree_menu.c/h` - 菜单树系统完全不变

#### 当前代码架构

```
┌─────────────────────────────────────────────────────────┐
│                        main.c                            │
│                   (入口、初始化、主循环)                   │
│             使用 AppState_Init() 初始化状态                │
└─────────────────────────┬───────────────────────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────────┐
│                    app_state.c/h                         │
│              (统一状态管理 - 全局变量整合)                  │
├─────────────┬─────────────┬─────────────┬───────────────┤
│ LearnState   │ ReviewState │ TestState   │ MenuState     │
│ learnIndex   │ reviewIndices│ testIndices│ rootMenu      │
│ filterUnknown │ flashcardFace│ testCorrect│ currentMenu   │
│ scrollOffset │ animTime     │ selectedAns │ menuStack     │
└─────────────┴─────────────┴─────────────┴───────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────────┐
│                menu_callbacks.c/h                        │
│               (菜单业务逻辑 - 页面渲染)                    │
│              使用宏访问状态: LEARN.learnIndex 等           │
├─────────────┬─────────────┬─────────────┬───────────────┤
│ MenuHome    │ MenuLearn   │ MenuReview  │ MenuTest      │
└─────────────┴─────────────┴─────────────┴───────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────────┐
│                raylib_word_ui.c/h                        │
│                  (UI 组件库 - 布局、交互)                  │
├─────────────┬─────────────┬─────────────┬───────────────┤
│ UILayout    │ UIButton    │ UICheckbox  │ UIScrollView  │
│ UITextBox   │ UICard      │ UIMultiChoice                │
└─────────────┴─────────────┴─────────────┴───────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────────┐
│                    fonts.c/h                            │
│              (字体渲染 - 混合文本 UTF-8)                   │
│                   【本次重构未修改】                       │
├─────────────┬─────────────┬─────────────┬───────────────┤
│ g_chFont    │ g_enFont    │ g_latinFont │ g_mergedFont  │
└─────────────┴─────────────┴─────────────┴───────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────────┐
│                    tree_menu.c/h                         │
│                    (树形菜单系统)                         │
└─────────────────────────────────────────────────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────────┐
│                    words.c/h                            │
│                  (单词数据管理)                          │
└─────────────────────────────────────────────────────────┘
```

#### 架构层级说明

| 层级 | 模块 | 职责 | 变更 |
|------|------|------|------|
| L1 | `main.c` | 程序入口，初始化/主循环/清理 | 不变 |
| L2 | `config.h` | 配置常量集中管理 | 新增 |
| L2 | `app_state.c/h` | 统一状态管理，协调各子系统 | v1.2 新增 |
| L3 | `menu_callbacks.c/h` | 业务逻辑，页面实现 | v1.2 重构 |
| L4 | `raylib_word_ui.c/h` | 基础 UI 组件库 | 不变 |
| L5 | `fonts.c/h` | 字体资源管理，文本渲染 | 不变 |
| L6 | `tree_menu.c/h` | 菜单导航系统 | 不变 |
| L7 | `words.c/h` | 数据层，单词管理 | 不变 |

---

### v1.0.0 (2025-01-26) - 初始版本

#### 新增内容

| 文件 | 说明 |
|------|------|
| `main.c` | 主程序入口，包含窗口初始化和主循环 |
| `words.c/h` | 单词数据管理，包括加载、进度跟踪 |
| `fonts.c/h` | 字体加载和混合文本渲染 |
| `menu_callbacks.c/h` | 四个菜单页面（学/背/测/进度） |
| `raylib_word_ui.c/h` | UI 组件库（按钮、卡片、滚动视图等） |
| `tree_menu.c/h` | 树形菜单系统 |
| `CMakeLists.txt` | CMake 构建配置 |

#### 功能特性

- 学单词模式：单词列表 + 详细信息
- 背单词模式：闪卡翻转，3次认识标记已掌握
- 测试模式：选择题，统计正确率
- 进度管理：学习统计 + 进度持久化
- 主题支持：浅色/深色模式
- 混合字体：中文/英文/IPA音标

---

## 后续更新模板

> 每次重大修改后，在上方添加新的版本块：

```markdown
### vX.Y.Z (YYYY-MM-DD) - [版本说明]

#### 修改内容
- [修改了什么]

#### 新增内容
- [新增了什么]

#### 删除内容
- [删除了什么]
```

---

## 注意事项

1. **提交前更新**：每次 `git commit` 前，确保此文档已更新
2. **版本号规则**：
   - 主版本号：重大架构调整
   - 次版本号：新增功能/较大改动
   - 修订号：bug修复/小改动
3. **文档位置**：此文档应与代码一同提交，保持版本历史
4. **配置修改**：修改 `config.h` 中的常量后，建议记录变更原因
