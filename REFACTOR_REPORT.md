# 项目变更记录

> 本文档记录项目的所有重要变更，包括功能增减、架构调整、重构等。
> 每次重大修改后必须更新此文档。

---

## 版本信息

| 项目 | 内容 |
|------|------|
| 项目名称 | raylib-word |
| 当前版本 | v1.4.0 |
| 创建日期 | 2025-01-26 |
| 最近更新 | 2025-01-26 |

---

## 更新日志

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
