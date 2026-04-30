# 代码架构与阅读指南

> 本文档面向**想要阅读和学习这个项目源码的新手开发者**。
> 我会从宏观到微观，告诉你整个软件是怎么运转的，从哪里开始看代码，以及怎么修改。

---

## 目录

1. [一句话说清这个软件怎么工作](#1-一句话说清这个软件怎么工作)
2. [项目文件结构（先记住这 4 个文件夹）](#2-项目文件结构先记住这-4-个文件夹)
3. [程序的"运行模式"——主循环](#3-程序的运行模式主循环)
4. [三层架构详解](#4-三层架构详解)
5. [模块功能总览](#5-模块功能总览)
6. [阅读路线图——从哪里开始看](#6-阅读路线图从哪里开始看)
7. [如何修改代码——常见需求的改法](#7-如何修改代码常见需求的改法)
8. [新手常问的问题](#8-新手常问的问题)

---

## 1. 一句话说清这个软件怎么工作

```
每 1/60 秒：
  1. 检测鼠标/键盘有没有操作
  2. 擦掉屏幕上的旧画面
  3. 重新画一遍全部内容（按钮、文字、卡片……）
  4. 等待下一次时钟滴答
```

这就是**"立即模式"（Immediate Mode）**UI 的核心思想：
> **不保存"界面上有什么"，每帧重新画一遍。**

对比你平时用的微信、浏览器等软件（"保留模式"）：
- 它们会在内存里维护一棵"界面树"，只在内容变化时才重绘
- 本软件每帧都从零开始绘制全部内容，代码更简单直接

**好处：** 代码写起来很直观——你想画什么就直接写绘制代码，不需要管理状态树
**坏处：** 如果你的绘制函数很慢，帧率会下降（但本软件界面简单，没这个问题）

---

## 2. 项目文件结构（先记住这 4 个文件夹）

```
Recite_English/
├── src/                    ← 📁 **源代码全部在这里**
│   ├── main.c              ← 程序的"大门口"，从这里启动
│   ├── core/               ← 🏗️ **核心框架**（底层基础设施）
│   ├── modules/            ← 📦 **业务模块**（独立的功能库）
│   └── ui/                 ← 🎨 **界面层**（按钮/页面等所有肉眼可见的东西）
├── data/                   ← 📁 词库、账号、字体等数据文件
├── docs/                   ← 📁 各类文档
├── CMakeLists.txt          ← 编译配置（告诉 CMake 怎么编译）
└── build.sh                ← 一键构建脚本（编译+打包）
```

### 4 个文件夹的职责分工

```
src/main.c
    │
    ▼
┌─────────────────────────────────────────────┐
│              core/（核心框架）                │
│  ┌──────────┬───────────┬──────────────┐    │
│  │config.h  │app_state  │tree_menu     │    │
│  │(配置常量) │(状态管理)  │(菜单树/导航栈)│    │
│  └──────────┴───────────┴──────────────┘    │
├─────────────────────────────────────────────┤
│             modules/（业务模块）              │
│  ┌──────────┬──────┬────────┬────────┐     │
│  │account   │plan  │words   │fonts   │     │
│  │(账号系统) │(计划)│(单词库) │(字体渲染)│     │
│  └──────────┴──────┴────────┴────────┘     │
├─────────────────────────────────────────────┤
│                ui/（界面层）                 │
│  ┌──────────────┬───────────────┐          │
│  │raylib_word_ui│menu_callbacks │          │
│  │(按钮/输入框  │(菜单系统 +     │          │
│  │ 闪卡/滚动等) │ 侧边栏绘制)    │          │
│  └──────────────┴───────────────┘          │
│  ┌───────────────────────────────────┐     │
│  │ pages/（12 个页面，每个一个文件）    │     │
│  │ page_home.c    ← 主菜单           │     │
│  │ page_learn.c   ← 学单词           │     │
│  │ page_card_review.c ← 卡片背单词    │     │
│  │ page_test.c    ← 测试模式          │     │
│  │ ……（共 12 个页面文件）              │     │
│  └───────────────────────────────────┘     │
└─────────────────────────────────────────────┘
```

**核心规则：** 箭头方向代表"依赖方向"
- `core/` 不依赖 `modules/` 和 `ui/`（纯粹的底层）
- `modules/` 不依赖 `ui/`（纯业务逻辑）
- `ui/` 依赖 `core/` 和 `modules/`（界面调用业务）

---

## 3. 程序的"运行模式"——主循环

这是整个软件最重要的运行逻辑，理解了它就读懂了 80% 的代码。

### 3.1 启动流程（main.c）

```
程序启动（main函数）
    │
    ├─ 1. 创建窗口（1600×1000，标题"背单词软件"）
    │
    ├─ 2. 初始化阶段（按顺序加载各项数据）
    │   ├── loadWordsFromFile()   → 从 words.txt 加载词库
    │   ├── Account_Init()        → 从 accounts.txt 加载账号
    │   ├── Plan_Init()           → 从 plans.txt 加载学习计划
    │   ├── initWords()           → 从 progress.txt 加载学习进度
    │   ├── loadFonts()           → 加载中/英/IPA 三种字体
    │   ├── AppState_Init()       → 初始化所有状态变量
    │   └── InitMenuTree()        → 创建菜单树（12 个页面节点）
    │
    ├─ 3. 进入主循环（每秒 60 次）
    │   │
    │   │   while (!WindowShouldClose()) {
    │   │
    │   ├──→ UIBegin()            → ① 读取鼠标键盘状态
    │   │
    │   ├──→ BeginDrawing()       → ② 开始绘制
    │   │   ├── ClearBackground() → 清屏
    │   │   ├── 绘制顶部标题栏     → 画"背单词软件"文字
    │   │   ├── 绘制右上角用户信息 → 显示"未登录"或用户名
    │   │   ├── DrawTreeMenu()    → 画左侧导航菜单
    │   │   └── CURRENT_MENU->show() → ③ **画当前页面**
    │   │                                 （12 个 show 函数之一）
    │   │
    │   ├──→ EndDrawing()
    │   └──→ UIEnd()              → ④ 清理临时状态
    │   │
    │   └── } 回到循环开始
    │
    └─ 4. 退出清理
        ├── AppState_Deinit()    → 释放菜单树内存
        ├── unloadFonts()        → 卸载字体
        ├── freeWordLibrary()    → 释放词库内存
        └── CloseWindow()        → 关闭窗口
```

### 3.2 "每帧重新画"到底是什么意思？

以"学单词"页面为例，想象代码执行过程：

```
第 1 帧（用户刚进入页面）：
    MenuLearn_Show() 被执行
    → 画左侧列表：abandon 高亮选中，ability，absence……挨个画出来
    → 画右侧详情：显示 abandon 的释义和例句
    → 画"上一个""下一个"按钮

第 2 帧（用户还没做任何操作）：
    MenuLearn_Show() 又被执行一次
    → 画左侧列表：abandon 仍然高亮……
    → 画右侧详情：一样的 abandon 信息
    → 画按钮

第 3 帧（用户点击了"下一个"按钮）：
    MenuLearn_Show() 再被执行
    → UIButton() 检测到点击 → LEARN.learnIndex 从 0 变为 1
    → 画左侧列表：ability 高亮选中
    → 画右侧详情：显示 ability 的释义
    → 画按钮
```

**关键发现：** "界面变化"本质上就是**每帧重新画时画了不同的内容**。你没有"改变界面"，你只是改变了一些变量（如 `learnIndex`），然后下一帧绘制时这些新值自然画出了新内容。

### 3.3 状态在哪里保存？

因为每帧都重新画，所以**状态必须保存在页面函数之外的"持久化"位置**，否则每帧都会丢失。

三种持久化方式：

| 方式 | 例子 | 生命周期 |
|------|------|---------|
| **AppState** 结构体字段 | `LEARN.learnIndex` 记录当前选中的单词 | 整个程序运行期间 |
| **static 局部变量** | `g_planForm` 记录计划管理表单输入 | 直到关闭程序 |
| **文本文件** | `progress.txt` 记录认识/不认识次数 | 跨会话 |

---

## 4. 三层架构详解

### 4.1 core/ 层（核心框架）

这层代码是整个程序的基础设施，它**不依赖其他任何业务模块**。

| 文件 | 一句话说明 |
|------|-----------|
| `config.h` | 所有常量的"总清单"——窗口大小、颜色、学习参数等 |
| `app_state.h/c` | 一个「大箱子」`AppState`，装着所有功能的状态 |
| `tree_menu.h/c` | 菜单树 + 导航栈——页面怎么跳转、怎么返回 |

**为什么需要 app_state？**
想象一个场景：你在"学单词"页面滑动到了第 5 个单词，切换到"背单词"页面背了几张卡，再切回"学单词"——如果 `learnIndex` 没有保存在一个全局位置，它就会回到 0，你得重新翻页。`AppState` 就是用于解决这种问题的"集中储物柜"。

### 4.2 modules/ 层（业务模块）

这层是**独立的业务功能库**，每个模块管理自己领域的数据。

| 文件 | 状态 | 数据文件 | 一句话说明 |
|------|------|---------|-----------|
| `account.h/c` | `AccountState` | `accounts.txt` | 用户注册登录，多用户隔离 |
| `plan.h/c` | `PlanState` | `plans.txt` | 每日目标管理，自动跨日 |
| `words.h/c` | `g_words[]` + `g_wordLibrary` | `words.txt`, `progress.txt` | 词库管理 + 学习进度 |
| `fonts.h/c` | 4 个 `Font` 变量 | 无（加载字体文件） | 中/英/IPA 混合文本渲染 |

**重要设计模式：外部状态绑定**
以 `account` 模块为例——它的状态存在 `AppState` 中，而不是模块内部：

```c
// app_state.c 中：
Account_SetState(&g_app.account);  // 告诉 account 模块：你的状态在 &g_app.account 这里

// account.c 内部：
static AccountState* g_pState = NULL;       // 指向 AppState 中的 account 字段
static AccountState g_internalState;        // 备用（当 g_pState==NULL 时使用）

static AccountState* getState(void) {
    return g_pState ? g_pState : &g_internalState;  // 优先使用外部绑定的状态
}
```

这种设计的好处：`AppState` 可以统一管理所有状态的初始化和清理，而模块自己不需要操心全局变量。

### 4.3 ui/ 层（界面层）

这层是**所有用户能看到的东西**。

| 文件 | 一句话说明 |
|------|-----------|
| `raylib_word_ui.h/c` | 自制的 UI 组件库：按钮、输入框、闪卡、滚动视图…… |
| `menu_callbacks.h/c` | 菜单树的创建 + 侧边导航栏的绘制 |
| `pages/pages.h` | 所有页面文件的公共头文件（集中包含 + 访问宏） |
| `pages/page_*.c` | 12 个具体页面，每个文件就是一个完整的页面 |

---

## 5. 模块功能总览

### 5.1 页面文件速查表

| # | 文件名 | 函数 | 页面 | 难度 |
|---|--------|------|------|------|
| 1 | `page_home.c` | `MenuHome_Show()` | 主菜单 | ⭐ 最简单，适合第一次读 |
| 2 | `page_learn.c` | `MenuLearn_Show()` | 学单词 | ⭐⭐ |
| 3 | `page_review_root.c` | `MenuReviewRoot_Show()` | 背单词（入口） | ⭐ |
| 4 | `page_card_review.c` | `MenuCardReview_Show()` | 卡片背单词 | ⭐⭐ |
| 5 | `page_select_word.c` | `MenuSelectWord_Show()` | 选词背单词 | ⭐⭐⭐ 有出题逻辑 |
| 6 | `page_test.c` | `MenuTest_Show()` | 测试模式 | ⭐⭐⭐ 同上 |
| 7 | `page_search.c` | `MenuSearch_Show()` | 查找单词 | ⭐⭐ |
| 8 | `page_plan.c` | 3 个函数 | 学习计划/进度 | ⭐⭐⭐ |
| 9 | `page_settings.c` | `MenuSettings_Show()` | 设置 | ⭐ 最简单 |
| 10 | `page_account.c` | 3 个函数 | 账号/登录/注册 | ⭐⭐ |
| 11 | `page_word_manager.c` | `MenuWordManager_Show()` | 词库管理 | ⭐⭐ |

### 5.2 12 个页面函数的共同模式

打开任何一个 `page_*.c` 文件，你会发现它们都遵循同样的结构：

```c
void MenuXXX_Show(void) {
    // 第 1 步：定义页面区域（左侧留 250px 给导航栏）
    Rectangle contentRect = {250, 80, SCREEN_WIDTH - 270, SCREEN_HEIGHT - 100};
    
    // 第 2 步：创建布局容器（垂直或水平）
    UILayout layout = UIBeginLayout(contentRect, UI_DIR_VERTICAL, 30, 50);
    
    // 第 3 步：按顺序画元素（每个元素占一行/一列）
    Rectangle titleRect = UILayoutNext(&layout, -1, 60);   // 标题
    DrawTextAuto(..., titleRect, ...);                      // 画标题文字
    
    Rectangle descRect = UILayoutNext(&layout, -1, 50);    // 描述
    DrawTextAuto(..., descRect, ...);                       // 画描述文字
    
    // 第 4 步：画控件（按钮、输入框等）
    Rectangle btnRect = UILayoutNext(&layout, 120, 50);
    if (UIButton("开始", btnRect, STYLE, UI_STATE, 100)) {
        // 用户点了按钮 → 执行某些操作
    }
}
```

---

## 6. 阅读路线图——从哪里开始看

如果你是新手，推荐按这个顺序阅读：

### 第 1 站：`src/main.c`（10 分钟）

这是程序的「大门」。从头到尾读一遍 `main()` 函数：
- 关注**初始化阶段**调用了哪些函数（这就是所有模块的"启动顺序"）
- 关注**主循环**里每帧做了什么
- 忽略具体实现细节

### 第 2 站：`src/core/config.h`（5 分钟）

看看所有可配置的常量。知道窗口多大、颜色是什么、学习阈值是多少。

### 第 3 站：`src/core/app_state.h`（10 分钟）

看 `AppState` 结构体有哪些字段——这就是整个程序运行时的"全集状态"。

### 第 4 站：`src/ui/pages/page_home.c`（15 分钟）

从最简单的页面开始。读完后你就能理解"页面函数"长什么样。

### 第 5 站：`src/core/app_state.c`（10 分钟）

看 `AppState_Init()` 和各个 `AppState_GetXXX()` 函数——理解状态怎么初始化和访问。

### 第 6 站：`src/ui/pages/page_learn.c`（20 分钟）

这是一个中等复杂度的页面。读完后你就能理解"列表 + 详情"模式。

### 第 7 站：`src/modules/words.c`（30 分钟）

这是核心业务模块。重点关注：
- `g_wordLibrary` 和 `g_words` 的关系（原始词库 vs 带进度数组）
- `loadWordsFromFile()` 怎么从文件解析数据
- `saveProgress()` / `loadProgress()` 怎么持久化学习进度

### 第 8 站：`src/ui/menu_callbacks.c`（15 分钟）

看 `InitMenuTree()` —— 这解释了所有页面是怎么组织成菜单树的。

### 第 9 站：`src/ui/raylib_word_ui.c`（可选，大文件）

当你需要理解某个 UI 控件（按钮、输入框、闪卡）的具体实现时再来查阅。

---

## 7. 如何修改代码——常见需求的改法

### 需求 1：修改主菜单上显示的文字

**改哪里：** `src/ui/pages/page_home.c` 中的 `MenuHome_Show()` 函数

```c
// 找到这行（第 14 行附近）：
DrawTextAuto(u8"欢迎使用背单词软件！", ...);
// 改成：
DrawTextAuto(u8"Hello World！", ...);
```

### 需求 2：添加一个新单词到词库

**方法 A——直接改文件**（最简单的）：
打开 `data/words.txt`，在末尾添加一行：
```
dictionary|ˈdɪkʃəneri|n. 字典；词典|I need a dictionary.|我需要一本字典。
```

**方法 B——通过程序界面**：
运行软件 → 进入"词库管理"页面 → 点击"新增" → 填写信息 → 保存。

### 需求 3：修改"认识 3 次算掌握"的阈值

**改哪里：** `src/core/config.h`

```c
#define MASTERED_THRESHOLD     3       // 改成 5 就变成认识 5 次才掌握
```

### 需求 4：改变窗口大小

**改哪里：** `src/core/config.h`

```c
#define SCREEN_WIDTH  1600   // 改成 1920
#define SCREEN_HEIGHT 1000   // 改成 1080
```

**但要注意：** 页面中大量硬编码的坐标不会自动适配新的窗口尺寸，需要逐个页面调整。

### 需求 5：添加一个"重置进度"的按钮

**找到现有类似功能：** 在 `page_plan.c` 中已经有"清除进度"按钮：

```c
// 在 page_plan.c 的 MenuProgress_Show() 函数中：
Rectangle resetBtn = UILayoutNext(&layout, -1, 55);
if (UIButton(u8"清除进度", resetBtn, STYLE, UI_STATE, 5)) {
    clearProgress();  // 调用 words.c 中的函数
}
```

你可以在其他页面的函数中仿照这个模式添加自己的按钮。

### 需求 6：给某个按钮换个颜色

**改哪里：** `src/ui/raylib_word_ui.c` 中的 `UIThemeLight()` 或 `UIThemeDark()`：

```c
// 浅色主题中的主色调：
.primary = { 70, 130, 180, 255 },  // 钢蓝色
// 改为红色：
.primary = { 220, 50, 50, 255 },
```

### 需求 7：新增一个页面

1. 在 `src/ui/pages/` 下创建 `page_myfeature.c`
2. 实现 `void MenuMyFeature_Show(void)` 函数
3. 在 `src/ui/pages/pages.h` 中添加函数声明
4. 在 `src/ui/menu_callbacks.c` 的 `InitMenuTree()` 中创建菜单节点并挂到菜单树上
5. 在 `CMakeLists.txt` 的 `add_executable` 中添加新文件

---

## 8. 新手常问的问题

### Q: 为什么所有变量都有 `g_` 前缀？

`g_` 表示"全局变量"（global），这是一个约定俗成的命名规则：
- `g_` 开头 → 全局变量（整个程序任何地方都能访问）
- `static` 不加 `g_` → 文件级变量（只能在当前 .c 文件中用）
- 无前缀 → 函数局部变量

例如：
```c
AppState g_app;              // g_ 表示全局，在 app_state.c 中定义
static int g_wordCount;      // static + g_ = 文件内全局（只在 words.c 中可见）
int learnIndex;              // 没有 g_ = 局部变量
```

### Q: `STYLE`、`LEARN` 这些大写的是啥？

它们是在 `pages.h` 中定义的**宏**：

```c
#define STYLE    (AppState_GetStyle())      // 每次使用 STYLE 都会调用函数
#define LEARN    (*AppState_GetLearnState())
```

所以你在代码里写：

```c
STYLE->fontSizeNormal = 28.0f;  // 等价于：
AppState_GetStyle()->fontSizeNormal = 28.0f;

LEARN.learnIndex = 3;           // 等价于：
(*AppState_GetLearnState()).learnIndex = 3;
```

### Q: 一个页面函数执行完后，下次再进来，之前的数据还在吗？

取决于数据保存在哪里：

| 数据位置 | 例子 | 下次进来还在吗？ |
|---------|------|----------------|
| `LEARN.learnIndex`（AppState 中） | 当前选中的单词 | ✅ 在 |
| `static` 变量（如 `g_planForm`） | 计划管理表单内容 | ✅ 在 |
| 函数内的局部变量 | `int sum = 0` | ❌ 被重新创建 |
| `progress.txt` 文件 | 学习进度 | ✅ 在 |

### Q: 为什么代码里没有 "class" 和 "对象"？

这个项目用纯 C 语言编写，没有 C++ 的 class 特性。
但C语言通过"结构体 + 函数指针"也可以实现面向对象的效果：

```c
// "对象" = 结构体
typedef struct { int knownCount; int unknownCount; ... } WordProgress;

// "方法" = 以结构体名为前缀的函数
void saveProgress(void);        // WordProgress 类的方法
void loadProgress(void);
```

### Q: 我想添加一个新功能，应该从哪个文件开始？

**黄金法则：先找到最类似的已有功能，复制粘贴再修改。**

| 你想做的 | 参考已有功能 |
|---------|------------|
| 加一个新页面 | 看 `page_settings.c`（最简单） |
| 加一个新的 UI 控件 | 看 `raylib_word_ui.c` 中的 `UIButton` |
| 加一个新的数据文件 | 看 `words.c` 的 `saveProgress` / `loadProgress` |
| 加一个新的状态变量 | 看 `app_state.h` 中的 `LearnState` 结构体 |

---

> 文档版本：v1.0 — 面向新手开发者的代码架构说明
