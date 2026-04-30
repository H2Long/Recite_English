# 代码架构与阅读指南（完全版）

> **本文档写给谁看：** 你是编程新手，拿到了这个背单词软件的源码，想从头到尾搞懂它是怎么工作的，然后能够自己修改或添加功能。
>
> **本文档怎么用：** 我建议你打开电脑，一边看文档一边打开对应的源码文件对照阅读。看不懂的段落跳过，先看整体框架，再回头细看。

---

## 目录

- [第一章：软件全景（3 分钟热身）](#第一章软件全景3-分钟热身)
- [第二章：启动流程 — 从 main() 开始](#第二章启动流程--从-main-开始)
- [第三章：主循环 — 软件的核心心脏](#第三章主循环--软件的核心心脏)
- [第四章：三层架构详解](#第四章三层架构详解)
- [第五章：core/ 层 — 基础设施](#第五章core-层--基础设施)
- [第六章：modules/ 层 — 业务功能库](#第六章modules-层--业务功能库)
- [第七章：ui/ 层 — 用户界面](#第七章ui-层--用户界面)
- [第八章：页面文件详解（12 个文件逐个说明）](#第八章页面文件详解12-个文件逐个说明)
- [第九章：数据持久化 — 文件读写机制](#第九章数据持久化--文件读写机制)
- [第十章：核心概念总结](#第十章核心概念总结)
- [第十一章：阅读路线图](#第十一章阅读路线图)
- [第十二章：常见修改场景（附代码）](#第十二章常见修改场景附代码)

---

## 第一章：软件全景（3 分钟热身）

### 1.1 这是个什么软件？

这是一个**背单词的桌面软件**，用 C 语言 + raylib 图形库写的。它提供了四种学习模式：

| 模式 | 做什么 | 像什么 |
|------|--------|--------|
| 学单词 | 滚动浏览单词列表，看释义和例句 | 像翻电子词典 |
| 卡片背单词 | 先看单词猜释义，再翻面确认 | 像纸质闪卡 |
| 选词背单词 | 看中文释义，从 4 个选项中选出正确单词 | 像选择题 |
| 测试 | 看英文单词，从 4 个选项中选出正确释义 | 像考试 |

### 1.2 整个项目有多少个文件？

```
Recite_English/
├── src/                        # 源码目录（约 30 个文件）
│   ├── main.c                  ─ 程序入口（~210 行）
│   ├── core/                   ─ 3 个文件（配置 + 状态 + 菜单）
│   ├── modules/                ─ 4 组文件（账号/计划/单词/字体）
│   └── ui/                     ─ 14 个文件（组件 + 菜单 + 12个页面）
├── data/                       # 数据文件
│   ├── words.txt               ─ 单词库（可编辑）
│   ├── accounts.txt            ─ 账号数据
│   ├── progress.txt            ─ 学习进度
│   ├── plans.txt               ─ 学习计划
│   └── fonts/                  ─ 字体文件（3 个）
├── docs/                       # 文档
└── CMakeLists.txt + build.sh   # 编译脚本
```

**重要：** 所有代码都在 `src/` 里，其他目录是数据、文档、编译配置。

### 1.3 一段话概括这个软件怎么工作

```
启动 → 加载词库/账号/进度/字体 → 进入主循环：
    每 1/60 秒：
        检测鼠标/键盘 → 擦掉屏幕 → 重新画全部界面 → 等下一帧
        其中"画界面"就是：画左侧菜单 + 画当前页面
    关闭窗口 → 保存进度 → 退出
```

**最核心的思维转变：** 这个软件不像微信或浏览器那样"界面上有按钮，点击后按钮变了"。它是：
1. 每帧都擦掉整个屏幕
2. 每帧都重新画所有东西
3. 界面"变化"只是因为**下一帧画了不同的内容**

怎么做到"画不同的内容"？因为**状态变量变了**。比如 `learnIndex` 从 0 变成 1，下一帧就会画第 2 个单词。

---

## 第二章：启动流程 — 从 main() 开始

打开 `src/main.c` 文件，这就是程序的入口。`main()` 函数可以分成 4 个阶段：

### 阶段 1：创建窗口（第 41~42 行）

```c
InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);  // 创建 1600×1000 的窗口
SetTargetFPS(60);                                         // 设定每秒 60 帧
```

这里的 `SCREEN_WIDTH` 和 `SCREEN_HEIGHT` 来自 `config.h`：

```c
#define SCREEN_WIDTH  1600
#define SCREEN_HEIGHT 1000
```

### 阶段 2：初始化数据（第 47~61 行）

这是最关键的阶段，按顺序加载所有数据：

```c
// 第 1 步：加载单词库（从 data/words.txt 读到内存）
loadWordsFromFile(WORDS_FILE_PATH);      // 填充 g_wordLibrary 数组

// 第 2 步：加载账号（从 data/accounts.txt 读到内存）
Account_Init();                           // 填充 AccountState 中的 users 数组

// 第 3 步：加载学习计划（从 data/plans.txt 读到内存）
Plan_Init();                              // 填充 PlanState 中的 plans 数组

// 第 4 步：初始化单词学习进度（从 data/progress.txt 恢复）
initWords();                              // 创建 g_words 数组并从文件恢复进度

// 第 5 步：初始化随机数（用于打乱题目顺序）
srand(...);

// 第 6 步：加载字体文件
loadFonts();                              // 加载中/英/IPA 三种字体

// 第 7 步：把字体设置到 UI 系统
UISetFonts(g_mergedFont, g_englishFont);
UISetLatinFont(g_latinFont);

// 第 8 步：初始化全局状态
AppState_Init();                          // 把所有状态变量设为初始值
g_app.loginRequired = true;               // 要求用户登录

// 第 9 步：创建菜单树（12 个页面形成树形结构）
InitMenuTree();
```

**初始化顺序为什么重要？** 因为后面的一些步骤依赖前面的数据。例如：
- `initWords()` 依赖 `loadWordsFromFile()`（需要先有单词数据）
- `AppState_Init()` 依赖 Account（需要把 AccountState 绑定到 AppState）
- `InitMenuTree()` 依赖 AppState（因为菜单树存在 AppState 中）

### 阶段 3：准备学习数据（第 63~91 行）

初始化完成后，还需要准备好第一次进入各个学习模式时的数据：

```c
// 准备闪卡复习：找到所有 lastReview == 0 的单词（还没复习过的）
REVIEW.reviewCount = 0;
for (int j = 0; j < g_wordProgressCount; j++) {
    if (g_words[j].progress.lastReview == 0) {
        REVIEW.reviewIndices[REVIEW.reviewCount++] = j;
    }
}
shuffleArray(REVIEW.reviewIndices, REVIEW.reviewCount);  // 打乱顺序
```

同时设置字号：
```c
STYLE->fontSizeSmall = 22.0f;
STYLE->fontSizeNormal = 28.0f;
STYLE->fontSizeLarge = 46.0f;
```

### 阶段 4：进入主循环（第 95~197 行）

```c
while (!WindowShouldClose()) {
    // 每帧都执行这里面的代码
}
```

关于主循环的详细说明，见下一章。

### 阶段 5：退出清理（第 202~206 行）

```c
AppState_Deinit();    // 释放菜单树内存
unloadFonts();        // 卸载字体
freeWordLibrary();    // 释放单词库
CloseWindow();        // 关闭窗口
```

---

## 第三章：主循环 — 软件的核心心脏

主循环是软件最关键的代码。每一帧（~16 毫秒）都执行以下 6 件事：

### 3.1 主循环的 6 个步骤（图解）

```
┌──────────────────────────────────────────────────┐
│             第 N 帧开始                            │
│                                                    │
│  ① UIBegin()      → 记录鼠标位置、检测按键         │
│                                                    │
│  ② BeginDrawing() → 告诉 raylib"我要开始画了"     │
│                                                    │
│  ③ ClearBackground() → 清屏（擦掉上一帧的画）     │
│                                                    │
│  ④ 画顶部标题栏 + 右上角用户信息                   │
│                                                    │
│  ⑤ DrawTreeMenu()  → 画左侧导航菜单               │
│                                                    │
│  ⑥ CURRENT_MENU->show() → 画当前页面              │
│     ↓                                              │
│     (如果未登录) 画登录遮罩层                        │
│                                                    │
│  ⑦ EndDrawing()    → 通知 raylib"画完了，显示吧"   │
│                                                    │
│  ⑧ UIEnd()         → 清理临时状态，准备下一帧      │
│                                                    │
│             第 N+1 帧开始                           │
└──────────────────────────────────────────────────┘
```

### 3.2 第①步 UIBegin() 到底做了什么？

打开 `raylib_word_ui.c`，找到 `UIBegin()` 函数：

```c
void UIBegin(UIState* state) {
    // 记录当前鼠标位置（用于检测悬停、点击）
    state->mousePos = GetMousePosition();
    
    // 检测鼠标按键状态
    state->mouseDown = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
    state->mousePressed = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    state->mouseReleased = IsMouseButtonReleased(MOUSE_LEFT_BUTTON);
    
    // 检测键盘输入
    state->keyPressed = GetKeyPressed();
    state->charPressed = GetCharPressed();
    
    // 重置热区 ID（每次重新计数）
    state->hotItem = 0;
}
```

**为什么需要这个函数？** raylib 的输入检测是"瞬时"的。`IsMouseButtonDown()` 只在调用那一瞬间返回 true。如果没有 `UIBegin()` 统一记录状态，后面画按钮时再去调用这些函数，不同按钮之间的调用时序会导致状态不一致。

### 3.3 第④步：画标题栏

```c
// 画一个蓝色背景条
Rectangle topBar = {0, 0, SCREEN_WIDTH, 60};
DrawRectangleRec(topBar, STYLE->theme.panelBg);

// 在中间画"背单词软件"几个大字
DrawTextAuto(u8"背单词软件", ..., 40, 1, STYLE->theme.primary);

// 右上角画用户信息（"未登录"或用户名）
// 如果点击了，跳转到账号管理页面
```

### 3.4 第⑤步：画左侧菜单

`DrawTreeMenu()` 在 `menu_callbacks.c` 中。它会：

1. 如果当前不是根菜单（比如在子页面），显示 **"返回"按钮** 和页面名称
2. 遍历当前菜单节点的子节点，画出每个子菜单项（如"学单词""背单词"等）
3. 如果子节点有 `show` 函数指针，点击时跳转到该页面

**侧边菜单的"返回"工作原理：**

```c
// 当用户点击"学单词"按钮时：
if (UIButton("学单词", itemRect, STYLE, UI_STATE, 200 + i)) {
    // 1. 把当前页面（主菜单）压入栈
    StackPush(AppState_GetMenuStack(), child);
    // 2. 把当前页面设为"学单词"
    g_app.currentMenu = child;
}

// 当用户点击"返回"按钮时：
if (UIButton(u8"返回", backRect, STYLE, UI_STATE, 6)) {
    // 1. 从栈中弹出上一个页面
    MENU* prev = StackPop(AppState_GetMenuStack());
    // 2. 设置当前页面为上一个页面
    g_app.currentMenu = g_app.menuStack.menuStack[g_app.menuStack.Stacktop];
}
```

### 3.5 第⑥步：画当前页面

```c
if (CURRENT_MENU->show != NULL) {
    CURRENT_MENU->show();  // 调用当前页面的渲染函数
}
```

这里的 `CURRENT_MENU` 是宏，展开为 `(*AppState_GetCurrentMenu())`，也就是 `g_app.currentMenu`。

- 如果 `g_app.currentMenu` 指向主菜单节点 → 调用 `MenuHome_Show()`
- 如果指向学单词节点 → 调用 `MenuLearn_Show()`
- 以此类推

### 3.6 第⑥步的"登录遮罩层"

主循环最后有一个 `if (g_app.loginRequired && !Account_IsLoggedIn())` 判断：

```c
if (g_app.loginRequired && !Account_IsLoggedIn()) {
    // 如果当前页面不是账号页面，画一个半透明遮罩+提示
    // 提示："请先登录"
    // 两个按钮："登录" 和 "注册账号"
    // 点击后导航到账号管理页面
}
```

这就是为什么刚启动时所有页面都被挡住——**必须先登录才能使用**。

### 3.7 第⑧步 UIEnd()

```c
void UIEnd(UIState* state) {
    // 把上一帧的 pressed 状态清除
    // 有些状态是"这一帧有效"的，需要在这里清理
}
```

---

## 第四章：三层架构详解

```
src/
├── main.c              ← 入口（不属于任何层）
├── core/               ← 🏗️ 核心框架层（不依赖任何业务）
│   ├── config.h        ─ 全局常量
│   ├── app_state.c/h   ─ 统一状态管理
│   └── tree_menu.c/h   ─ 菜单树 + 导航栈
├── modules/            ← 📦 业务逻辑层（不依赖 UI）
│   ├── account.c/h     ─ 账号系统
│   ├── plan.c/h        ─ 学习计划
│   ├── words.c/h       ─ 单词数据+进度
│   └── fonts.c/h       ─ 字体渲染
└── ui/                 ← 🎨 界面表现层（依赖 core + modules）
    ├── raylib_word_ui.c/h  ─ UI 组件库
    ├── menu_callbacks.c/h  ─ 菜单系统
    └── pages/              ─ 12 个页面文件
```

### 4.1 为什么分三层？

**最简单的类比：餐厅**

| 层 | 类比 | 职责 |
|---|------|------|
| `core/` | **餐厅的建筑** | 墙、水管、电路——支持一切的基础设施 |
| `modules/` | **厨房** | 做菜——纯业务逻辑，不关心菜怎么端上桌 |
| `ui/` | **餐厅前台** | 服务员、菜单——用户能看到和交互的东西 |

**核心规则：** 箭头方向是"依赖方向"——`core/` 不依赖 `modules/` 和 `ui/`，`modules/` 不依赖 `ui/`。

### 4.2 依赖关系图解

```
main.c
  │
  ├──→ core/app_state.h    (需要 app_state.h 中的 AppState 类型)
  ├──→ modules/words.h     (需要 words.h 中的 g_words 等)
  ├──→ modules/fonts.h     (需要 fonts.h 中的字体函数)
  ├──→ ui/menu_callbacks.h (需要 InitMenuTree 函数)
  ├──→ modules/account.h   (需要 Account_Init 等)
  └──→ modules/plan.h      (需要 Plan_Init 等)


page_xxx.c （以 page_home.c 为例）
  │
  └──→ pages.h
         ├──→ raylib.h             (raylib 图形库)
         ├──→ raylib_word_ui.h      (UIButton/UILayout 等)
         ├──→ tree_menu.h           (MENU 类型)
         ├──→ words.h               (g_words/g_wordProgressCount)
         ├──→ fonts.h               (MeasureTextAuto/DrawTextAuto)
         ├──→ app_state.h           (AppState_GetStyle 等)
         ├──→ config.h              (SCREEN_WIDTH/SCREEN_HEIGHT)
         ├──→ account.h             (Account_IsLoggedIn)
         └──→ plan.h                (Plan_GetActive)


menu_callbacks.c
  │
  ├──→ menu_callbacks.h    (自己的头文件)
  ├──→ app_state.h         (访问 AppState)
  └──→ pages.h             (获取所有页面函数指针)


words.c
  │
  ├──→ words.h             (自己的头文件)
  └──→ config.h            (使用 WORDS_FILE_PATH 等常量)
```

**观察到什么？** 每个文件只包含它真正需要的头文件。`pages.h` 充当了"总入口"，让页面文件只需要一个 `#include "pages.h"` 就能访问所有功能。

---

## 第五章：core/ 层 — 基础设施

### 5.1 config.h — 全局常量清单

这个文件定义了整个程序用的**所有常量**。没有它，程序寸步难行。

**常量分类：**

| 分类 | 示例 | 含义 | 修改后影响 |
|------|------|------|-----------|
| 窗口大小 | `SCREEN_WIDTH = 1600` | 窗口宽 | 界面布局会乱（硬编码坐标） |
| 主题颜色 | `THEME_PRIMARY = dodger blue` | 主色调 | 按钮/标题等颜色改变 |
| 学习参数 | `MASTERED_THRESHOLD = 3` | 认识3次算掌握 | 学习策略改变 |
| UI 尺寸 | `CARD_WIDTH = 600` | 闪卡宽度 | 卡片大小改变 |
| 测试设置 | `TEST_MAX_QUESTIONS = 10` | 最多10题 | 测试题目数 |
| 文件路径 | `WORDS_FILE_PATH` | 词库文件路径 | 文件位置改变 |

**新手常犯的错误：** 修改 `SCREEN_WIDTH` 发现窗口确实变了，但按钮位置全乱套了。这是因为**按钮坐标是硬编码的**，没有根据窗口尺寸动态计算。

### 5.2 app_state.h — 程序状态的"总清单"

这个文件定义了 `AppState` 结构体。它是**整个程序运行时的状态全集**：

```c
typedef struct {
    UIStyle style;             // UI 样式（主题/字体/字号）
    UIState uiState;           // UI 交互状态（鼠标/键盘/热区）
    bool isDarkMode;           // 当前是否为深色模式
    
    MENU* rootMenu;            // 根菜单节点（菜单树的根）
    MENU* currentMenu;         // 当前正在显示的菜单
    MenuStack menuStack;       // 导航栈（用于"返回"功能）
    
    LearnState learn;          // 学单词模式：当前索引、过滤器、滚动位置
    ReviewState review;         // 卡片背单词：待复习列表、翻面状态
    TestState test;            // 测试模式：题目列表、答题统计
    SelectWordState selectWord; // 选词背单词：题目列表、答题统计
    SearchState search;         // 查找单词：搜索输入、结果列表
    
    AccountState account;      // 账号系统：用户列表、登录状态
    char loginMsg[128];        // 登录提示消息
    
    PlanState plan;            // 学习计划：计划列表、当前计划
    
    bool loginRequired;        // 是否要求登录
} AppState;
```

**每个子状态的具体字段：**

**LearnState（学单词）：**
| 字段 | 类型 | 默认值 | 含义 |
|------|------|--------|------|
| `learnIndex` | int | 0 | 当前在列表中选择的单词索引 |
| `learnFilterUnknown` | bool | false | 是否只显示未掌握的单词 |
| `learnScrollOffset` | float | 0 | 列表滚动位置（跨帧保存） |

**ReviewState（卡片背单词）：**
| 字段 | 类型 | 含义 |
|------|------|------|
| `reviewIndices[100]` | int[] | 待复习的单词在 g_words 中的索引 |
| `reviewCount` | int | 待复习单词的数量 |
| `currentReviewIdx` | int | 当前复习到第几个 |
| `flashcardFace` | CardFace | 闪卡当前显示正面还是背面 |
| `flashcardAnimTime` | float | 翻转动画进度 |
| `knownInSession` | int | 本轮认识了多少个 |
| `unknownInSession` | int | 本轮不认识多少个 |

**TestState（测试模式）和 SelectWordState（选词背单词）结构基本相同：**
| 字段 | 类型 | 含义 |
|------|------|------|
| `testIndices[100]` | int[] | 测试单词的索引 |
| `testCount` | int | 总题数 |
| `currentTestIdx` | int | 当前第几题 |
| `testCorrect` | int | 答对多少题 |
| `testTotal` | int | 已答多少题 |
| `selectedAnswer` | int | 用户选了哪个选项（-1=未选） |
| `answerResult` | int | 答题结果（1=对/0=错/-1=未答） |
| `currentCorrectIdx` | int | 正确答案是哪个选项 |
| `wrongOptionsUsed[100]` | bool[] | 标记已用过哪些单词做错误选项 |

### 5.3 app_state.c — 状态的初始化和访问

这个文件提供两个关键功能：

**① 初始化全部状态**
```c
void AppState_Init(void) {
    // 初始化样式（主题、字体大小等）
    UIStyleInit(&g_app.style);
    
    // 设置主题（浅色模式）
    g_app.isDarkMode = false;
    g_app.style.theme = UIThemeLight();
    
    // 绑定模块状态到 AppState
    Account_SetState(&g_app.account);  // account 模块的状态存在这里
    Plan_SetState(&g_app.plan);        // plan 模块的状态存在这里
    
    // 初始化学习模式状态为安全默认值
    memset(&g_app.learn, 0, sizeof(LearnState));
    memset(&g_app.review, 0, sizeof(ReviewState));
    // ... 依此类推
}
```

**② 提供访问器函数**
```c
// 每个子状态都有一个 getter 函数
UIStyle* AppState_GetStyle(void) { return &g_app.style; }
UIState* AppState_GetUIState(void) { return &g_app.uiState; }
LearnState* AppState_GetLearnState(void) { return &g_app.learn; }
// ... 每个字段都有对应的 Getter
```

**为什么需要这些 Getter？** 因为 `pages.h` 中的宏需要它们：

```c
#define LEARN (*AppState_GetLearnState())
// 这样写 LEARN.learnIndex 就等价于 (*AppState_GetLearnState()).learnIndex
// 也就等价于 g_app.learn.learnIndex
```

### 5.4 tree_menu.h — 菜单树结构

菜单用**树形结构**组织：

```
[主菜单] ← 根节点
  ├── [学单词]       ← 子节点1（无下级）
  ├── [背单词]       ← 子节点2
  │   ├── [卡片背单词]  ← 子节点2的子节点
  │   ├── [选词背单词]
  │   └── [测试模式]
  ├── [查找单词]
  ├── [学习计划]
  │   ├── [计划管理]
  │   └── [学习进度]
  ├── [设置]
  ├── [账号管理]
  └── [词库管理]
```

每个节点是一个 `MENU` 结构体：

```c
typedef struct menu {
    void (*show)(void);           // 指向页面渲染函数
    void (*fun)(void);            // 保留备用
    struct menu* child[8];        // 子节点数组（最多8个）
    struct menu* parent;          // 父节点指针
    int currentindex;             // 在父节点中的索引
    int childindex;               // 子节点数量
} MENU;
```

**关键设计：函数指针**

`show` 是一个**函数指针**。C 语言的函数指针可以指向任何一个函数，只要参数和返回值匹配。这里所有页面函数都是 `void func(void)` 类型。

所以这一行：
```c
MENU* menuLearn = CreatMenuTreeNode(NULL, MenuLearn_Show);
```
创建了一个菜单节点，它的 `show` 指向 `MenuLearn_Show` 函数。当用户点击"学单词"菜单项时，程序执行：
```c
g_app.currentMenu->show();  // 等价于 MenuLearn_Show();
```

**导航栈（实现"返回"功能）：**

```c
typedef struct MenuStack {
    MENU* menuStack[10];  // 栈数组，最多 10 层
    int Stacktop;          // 栈顶索引（-1 = 空）
} MenuStack;
```

- **进入子页面时**：`StackPush(&menuStack, currentMenu)` — 把当前页面记下来
- **点击"返回"时**：`StackPop(&menuStack)` — 取出上一个页面

### 5.5 tree_menu.c — 菜单树的创建

`InitMenuTree()` 函数创建了完整的菜单树。关键代码：

```c
void InitMenuTree(void) {
    // 1. 创建每个页面节点（第二个参数是页面渲染函数）
    MENU* menuLearn = CreatMenuTreeNode(NULL, MenuLearn_Show);
    MENU* menuCardReview = CreatMenuTreeNode(NULL, MenuCardReview_Show);
    // ...
    
    // 2. 组织父子关系
    // 背单词（父节点）→ 卡片/选词/测试（子节点）
    MENU* menuReviewRoot = CreatMenuTreeNode(NULL, MenuReviewRoot_Show);
    ConnectMenuTree(menuReviewRoot, menuCardReview);
    ConnectMenuTree(menuReviewRoot, menuSelectWord);
    ConnectMenuTree(menuReviewRoot, menuTest);
    
    // 3. 创建根节点，连接所有一级菜单
    g_app.rootMenu = CreatMenuTreeNode(NULL, MenuHome_Show);
    ConnectMenuTree(g_app.rootMenu, menuLearn);
    ConnectMenuTree(g_app.rootMenu, menuReviewRoot);
    // ...
}
```

---

## 第六章：modules/ 层 — 业务功能库

### 6.1 四个模块的定位

| 模块 | 管理的数据 | 涉及的全局变量 | 功能 |
|------|-----------|---------------|------|
| account | 用户列表、登录状态 | 通过 `AppState.account` | 注册/登录/登出/多用户 |
| plan | 学习计划列表 | 通过 `AppState.plan` | 创建/切换/跨日推进 |
| words | 单词库、学习进度 | `g_wordLibrary`, `g_words[]` | 加载/进度/搜索/词库管理 |
| fonts | 字体句柄 | `g_chineseFont` 等 | 加载/混合文本渲染 |

### 6.2 account.h/c — 账号系统

**数据结构：**
```c
typedef struct {
    char username[MAX_USERNAME];       // 用户名
    char passwordHash[MAX_PASSWORD];   // 密码哈希值（djb2 算法）
    time_t createdTime;                // 注册时间
    time_t lastLoginTime;              // 最后登录时间
    int selectWordCorrect;            // 选词背单词累计正确数
    int selectWordTotal;              // 选词背单词累计总题数
} User;

typedef struct {
    User users[MAX_USERS];     // 用户数组（最多 50 个）
    int userCount;             // 当前用户数
    int currentUserIndex;      // 当前登录的用户索引
    bool isLoggedIn;           // 是否已登录
} AccountState;
```

**函数列表：**

| 函数 | 作用 | 你在使用时需注意 |
|------|------|----------------|
| `Account_Init()` | 从 `accounts.txt` 加载用户数据 | 必须在初始化阶段调用 |
| `Account_Save()` | 保存用户数据到文件 | 注册/登录后自动调用，一般不用手动调 |
| `Account_Register()` | 注册新用户 | 用户名/密码不能为空，不能重复 |
| `Account_Login()` | 登录验证 | 密码会哈希后比较，不是明文比较 |
| `Account_Logout()` | 登出 | 调用后需要手动切换进度文件路径 |
| `Account_IsLoggedIn()` | 检查登录状态 | 返回 bool |
| `Account_GetCurrentUser()` | 获取当前用户名 | 未登录返回空字符串 |
| `Account_GetCurrentIndex()` | 获取当前用户索引 | 未登录返回 -1 |
| `Account_GetProgressPath()` | 获取用户专属进度文件路径 | 如 `./progress_hhlong.txt` |
| `Account_GetPlanPath()` | 获取用户专属计划文件路径 | 如 `./plans_hhlong.txt` |
| `Account_SetState()` | 绑定外部状态 | 由 AppState_Init 自动调用 |

**数据流示例（登录过程）：**

```
用户输入用户名 "hhlong" 和密码 "abc123"
    ↓
Account_Login("hhlong", "abc123")
    ↓
find_user("hhlong") → 查找 users 数组 → 找到索引 1
    ↓
hash_password("abc123") → 计算哈希 → "1234567890"
    ↓
strcmp(users[1].passwordHash, "1234567890") → 匹配成功
    ↓
currentUserIndex = 1
isLoggedIn = true
users[1].lastLoginTime = time(NULL)
Account_Save()  // 持久化
    ↓
返回 true
```

**账号数据文件格式**（`data/accounts.txt`）：
```
username|passwordHash|createdTime|lastLoginTime|correctCount|totalCount
admin|1234567890|1700000000|1700000001|15|20
hhlong|9876543210|1700000002|1700000003|8|10
```

### 6.3 plan.h/c — 学习计划

**数据结构：**
```c
typedef struct {
    char name[PLAN_NAME_MAX];      // 计划名称（如"一周入门"）
    int dailyWordCount;            // 每日学习目标（如 20 个）
    int totalDays;                 // 计划总天数（如 7 天）
    int currentDay;                // 当前进度（第几天）
    time_t createdAt;              // 创建时间
    time_t lastStudyDate;          // 最后学习日期（yyyymmdd 格式整数）
    int studiedToday;              // 今日已学单词数
} LearningPlan;
```

**函数列表：**

| 函数 | 作用 | 关键逻辑 |
|------|------|---------|
| `Plan_Init()` | 从文件加载计划或创建默认计划 | 文件不存在时自动创建 4 个默认计划 |
| `Plan_Save()` | 保存计划到文件 | 每次修改后调用 |
| `Plan_Create()` | 创建新计划 | name/daily/total 三个参数 |
| `Plan_Delete()` | 删除计划 | 不能删除当前激活的计划 |
| `Plan_Activate()` | 激活一个计划 | 只能有一个激活计划 |
| `Plan_GetActive()` | 获取当前激活的计划 | 没有激活时返回 NULL |
| `Plan_CheckNewDay()` | 检查是否是新的一天 | **核心逻辑**——跨日时自动推进 `currentDay` |
| `Plan_AddStudiedToday()` | 增加今日学习计数 | 学了新单词后调用 |

**"跨日检查"的核心逻辑：**

```c
void Plan_CheckNewDay(void) {
    LearningPlan* p = Plan_GetActive();  // 获取当前激活的计划
    if (p == NULL) return;
    
    int today = get_today_date();        // 获取今天的 yyyymmdd（如 20260430）
    
    if (p->lastStudyDate != today) {     // 上次学习不是今天？
        // → 新的一天！
        if (p->lastStudyDate > 0) {
            p->currentDay++;             // 计划推进一天
        }
        p->studiedToday = 0;            // 重置今日学习计数
        p->lastStudyDate = today;       // 更新学习日期
        Plan_Save();                     // 持久化
    }
}
```

**这个逻辑什么时候被调用？** 在页面的渲染函数中，如 `page_plan.c` 和 `page_card_review.c` 中，每次进入时都会调用 `Plan_CheckNewDay()` 检查。

### 6.4 words.h/c — 单词系统（核心模块）

这是**最复杂的模块**。它管理两组数据：

**① g_wordLibrary — 原始词库（动态数组）**
```c
WordEntry* g_wordLibrary = NULL;  // 动态分配，自动扩容
int g_wordCount = 0;              // 词库中有几个单词
```

`WordEntry` 结构体（在 `raylib_word_ui.h` 中定义）：
```c
typedef struct {
    char* word;                  // 单词文本
    char* phonetic;              // 音标
    char* definition;            // 释义
    char* example;               // 例句
    char* exampleTranslation;    // 例句翻译
} WordEntry;
```

**② g_words[] — 含学习进度的单词数组（定长 100）**
```c
WordWithProgress g_words[100];  // 定长数组
int g_wordProgressCount = 0;    // 有效单词数
```

`WordWithProgress` 结构体（在 `words.h` 中定义）：
```c
typedef struct {
    WordEntry entry;          // 单词数据（与 g_wordLibrary 共享指针！）
    WordProgress progress;    // 学习进度
} WordWithProgress;

typedef struct {
    int wordIndex;             // 在 g_wordLibrary 中的索引
    int knownCount;            // 认识次数
    int unknownCount;          // 不认识次数
    time_t lastReview;         // 上次复习时间戳
    bool mastered;             // 是否已掌握（knownCount >= 3）
} WordProgress;
```

**两个数组的关系（核心数据流）：**

```
words.txt 文件
    │
    ▼
loadWordsFromFile()  ──→  g_wordLibrary（动态，可增删改）
                                │
                                ▼
                          initWords()  ──→  g_words[100]（含进度）
                                │
                                ▼
                          progress.txt  ←── saveProgress()
                          (进度持久化)      loadProgress()
```

**注意：** `g_words[i].entry` 是**浅拷贝**——它直接复制了 `g_wordLibrary[i]` 的指针，两者指向同一块内存。这意味着一方修改后另一方也会看到变更。（这也是 CODE_IMPROVE.md 中提到的风险点。）

**函数列表：**

| 函数 | 作用 | 调用时机 |
|------|------|---------|
| `loadWordsFromFile()` | 从 words.txt 加载到 g_wordLibrary | 启动时 |
| `freeWordLibrary()` | 释放 g_wordLibrary 内存 | 退出时 |
| `initWords()` | 从 g_wordLibrary 填充 g_words 并加载进度 | 启动时 |
| `saveProgress()` | 保存 g_words 的进度到文件 | 每次学习操作后 |
| `loadProgress()` | 从文件恢复进度到 g_words | 启动时 |
| `clearProgress()` | 清空所有进度 | 用户点击"清除进度" |
| `addWordToLibrary()` | 添加单词到词库 | 词库管理页面 |
| `editWordInLibrary()` | 编辑词库中的单词 | 词库管理页面 |
| `deleteWordFromLibrary()` | 删除词库中的单词 | 词库管理页面 |
| `reloadWords()` | 词库修改后同步到 g_words | 词库管理页面 |
| `saveWordsToFile()` | g_wordLibrary 保存到 words.txt | 词库增删改后 |
| `searchWordsByRegex()` | 通配符模式搜索单词 | 查找单词页面 |
| `searchWordsSimple()` | 简单子串搜索 | 词库管理页面 |
| `shuffleArray()` | 打乱数组 | 初始化/开始学习时 |
| `getMasteryColor()` | 根据掌握度返回颜色 | 学单词页面列表 |

### 6.5 fonts.h/c — 字体渲染

这个模块管理四种字体和混合文本渲染。

**四种字体：**

| 字体变量 | 来源文件 | 渲染的字符 |
|---------|---------|-----------|
| `g_chineseFont` | `NotoSansCJK.otf` | 中文字符（CJK 区块） |
| `g_englishFont` | `DejaVuSans.ttf` | 英文字符（ASCII） |
| `g_latinFont` | `DejaVuSans.ttf` | IPA 音标字符 |
| `g_mergedFont` | 基于 `g_chineseFont` 合并 | UI 统一文字 |

**混合文本渲染原理：**

`DrawTextAuto()` 函数逐字符遍历文本，根据 Unicode 码点选择字体：

```
文本："hello 世界 [həˈloʊ]"
        ↓↓↓   ↓↓   ↓↓↓↓↓↓↓
    英文字体  中文字体   IPA 字体

逐字符判断：
  'h' (0x0068) → ASCII → 用 g_englishFont
  'e' (0x0065) → ASCII → 用 g_englishFont
  ' ' (0x0020) → ASCII → 用 g_englishFont
  '世' (0x4E16) → CJK → 用 g_mergedFont (含中文)
  '界' (0x754C) → CJK → 用 g_mergedFont
  '[' (0x005B) → ASCII → 用 g_englishFont
  'h' (0x0068) → ASCII → 用 g_englishFont
  'ə' (0x0259) → IPA → 用 g_latinFont
  ...
```

**函数列表：**

| 函数 | 作用 | 参数 | 返回值 |
|------|------|------|--------|
| `loadFonts()` | 加载三种字体 | 无 | 无（设置全局字体变量） |
| `unloadFonts()` | 卸载字体释放 GPU 内存 | 无 | 无 |
| `DrawTextAuto()` | 混合字体绘制文本 | 文本、位置、字号、间距、颜色 | 无 |
| `MeasureTextAuto()` | 测量混合文本宽度 | 文本、字号、间距 | Vector2（宽+高） |
| `formatTime()` | 将时间戳格式化为日期字符串 | 时间戳 | 字符串（如 "2026-04-30"） |
| `fileExists()` | 检查文件是否存在 | 文件路径 | bool |

---

## 第七章：ui/ 层 — 用户界面

### 7.1 raylib_word_ui.h/c — UI 组件库

这是一个**自制的 UI 组件库**，提供了所有图形界面的基础元素。

**布局系统：**

```c
// 创建一个垂直布局容器，间距 30，内边距 50
UILayout layout = UIBeginLayout(contentRect, UI_DIR_VERTICAL, 30, 50);

// 获取下一个控件的位置（宽度=-1 表示填满容器，高度=60）
Rectangle titleRect = UILayoutNext(&layout, -1, 60);
// 在这个位置画标题
DrawTextAuto("标题", titleRect, ...);

// 再获取下一个控件位置
Rectangle descRect = UILayoutNext(&layout, -1, 50);
// 画描述
DrawTextAuto("描述", descRect, ...);
```

**按钮系统：**

```c
// UIButton 接受：文本、位置、样式、交互状态、唯一 ID
// 返回值：用户在这一帧点击了按钮 → true
if (UIButton("开始", btnRect, STYLE, UI_STATE, 100)) {
    // 只有在用户点击的这一帧才会进入这里
    doSomething();
}
```

按钮的 ID 为什么重要？同一帧可能画了多个按钮，ID 用来区分当前鼠标悬停/点击的是哪个按钮：

```c
int id = 100;
for (int i = 0; i < 5; i++) {
    if (UIButton(..., id + i)) {  // 每个按钮 ID 不同：100, 101, 102, 103, 104
        // 如果 ID=102 的按钮被点击，这里 i==2
    }
}
```

**文本框系统：**
```c
UITextBoxState myBox = {0};  // 初始化为 0（buffer 为空，cursor=0）

// 每次帧调用，如果点击了文本框，它会获得焦点并且可以输入
UITextBox(&myBox, rect, STYLE, UI_STATE, false);  // false = 不是密码模式

// 获取用户输入的内容
const char* input = myBox.buffer;
```

**闪卡系统：**
```c
int result = UIFlashCard(entry, rect, &face, STYLE, UI_STATE, &animTime);
// result 返回值：
//   0 = 没有操作
//   1 = 用户点击"认识"按钮
//   2 = 用户点击"不认识"按钮
//   3 = 用户点击"翻转"按钮
```

**核心组件列表：**

| 组件函数 | 作用 | 返回值 |
|---------|------|--------|
| `UIButton()` | 普通按钮 | bool（是否被点击） |
| `UIButtonEx()` | 扩展按钮（可禁用） | bool |
| `UICheckbox()` | 复选框 | bool（状态是否改变） |
| `UILabel()` | 纯文字标签 | void |
| `UITextBox()` | 文本输入框 | bool（是否获得焦点） |
| `UIFlashCard()` | 闪卡组件 | int（操作类型） |
| `UIMultipleChoice()` | 选择题组件 | int（选中选项索引） |
| `UIWordCard()` | 单词详情卡片 | void |
| `UIWordListView()` | 单词列表视图 | int（被点击的索引） |
| `UISearchBar()` | 搜索栏 | void |

### 7.2 menu_callbacks.c — 菜单系统

这个文件实现了三个功能：

1. **`InitMenuTree()`** — 创建 12 个页面的菜单树（见 5.5 节）
2. **`GetMenuItemText()`** — 根据函数指针返回菜单名称文本
3. **`DrawTreeMenu()`** — 绘制左侧导航栏

**`GetMenuItemText()` 如何知道菜单名称？**
```c
const char* GetMenuItemText(MENU* menu) {
    // 通过比较 show 函数指针来判断是哪个页面
    if (menu->show == MenuHome_Show) return u8"主菜单";
    if (menu->show == MenuLearn_Show) return u8"学单词";
    if (menu->show == MenuCardReview_Show) return u8"卡片背单词";
    // ...依此类推
}
```

---

## 第八章：页面文件详解（12 个文件逐个说明）

每个页面文件都遵循相同的"三步曲"结构：

```c
void MenuXXX_Show(void) {
    // 第 1 步：定义页面可用区域（扣除左侧菜单 + 顶部栏）
    Rectangle contentRect = {250, 80, SCREEN_WIDTH - 270, SCREEN_HEIGHT - 100};
    
    // 第 2 步：创建布局容器
    UILayout layout = UIBeginLayout(contentRect, UI_DIR_VERTICAL, 30, 50);
    
    // 第 3 步：在布局中依次画元素
    Rectangle titleRect = UILayoutNext(&layout, -1, 60);
    DrawTextAuto(u8"标题", titleRect, ...);
    
    Rectangle btnRect = UILayoutNext(&layout, 120, 50);
    if (UIButton(u8"按钮", btnRect, ...)) {
        // 处理点击
    }
}
```

### 8.1 page_home.c — 主菜单

**函数：** `MenuHome_Show()`

**做什么：** 显示欢迎信息、学习统计（总词数/已掌握/学习中）、三个功能入口卡片。

**特殊点：** 三个卡片按钮的点击处理——通过菜单栈导航：

```c
if (UIButton(u8"开始", goBtn, STYLE, UI_STATE, 100 + i) && modes[i].target != NULL) {
    StackPush(AppState_GetMenuStack(), modes[i].target);  // 记录当前页
    CURRENT_MENU = modes[i].target;                       // 跳转到目标页
}
```

### 8.2 page_learn.c — 学单词

**函数：** `MenuLearn_Show()`

**布局：** 水平两栏——左侧 350px 单词列表，右侧剩余空间为详情。

**涉及的状态：** `LEARN`（`LearnState`结构体）

```c
// 列表项点击时：
if (UIListItem(g_words[i].entry.word, itemRect, STYLE, UI_STATE))
    LEARN.learnIndex = i;  // 更新选中的单词索引

// 滚动位置跨帧保存：
LEARN.learnScrollOffset = sv.scrollOffset.y;

// "上一个""下一个"按钮：
if (UIButton(u8"上一个", ..., STYLE, UI_STATE, 1) && LEARN.learnIndex > 0)
    LEARN.learnIndex--;
```

### 8.3 page_review_root.c — 背单词父页

**函数：** `MenuReviewRoot_Show()`

**做什么：** 简单的选择入口页——三个按钮分别跳到卡片/选词/测试模式。

### 8.4 page_card_review.c — 卡片背单词

**函数：** `MenuCardReview_Show()`

**做什么：** 闪卡翻转（先看单词猜释义，翻面确认）。

**两阶段逻辑：**

```
阶段 1：学习中
    ┌────────────────┐
    │    abandon      │  ← 显示单词
    │   [翻转] 按钮   │
    └────────────────┘
         ↓ 点击翻转
    ┌────────────────┐
    │    abandon      │
    │  v. 放弃；抛弃   │  ← 显示释义
    │  [认识] [不认识] │  ← 选择 → 更新进度
    └────────────────┘

阶段 2：学习完毕（所有单词都过了一遍）
    显示结果统计：
    本轮认识: 15 个  不认识: 3 个
    [再学一轮] [返回]
```

**处理"认识/不认识"的代码：**

```c
if (result == 1) {  // 认识
    g_words[wordIdx].progress.knownCount++;
    if (g_words[wordIdx].progress.knownCount >= MASTERED_THRESHOLD) {
        g_words[wordIdx].progress.mastered = true;
    }
    REVIEW.knownInSession++;
} else if (result == 2) {  // 不认识
    g_words[wordIdx].progress.unknownCount++;
    REVIEW.unknownInSession++;
    // 把它放到队列末尾，后续再复习
}

g_words[wordIdx].progress.lastReview = time(NULL);
saveProgress();  // 立即保存进度
```

### 8.5 page_select_word.c — 选词背单词

**函数：** `MenuSelectWord_Show()`

**做什么：** 显示中文释义，从 4 个选项中选出正确的英文单词。

**出题逻辑：**
```c
// 从 g_words 中随机选一个作为正确答案
int correctIdx = TEST.testIndices[TEST.currentTestIdx];

// 从其余单词中选 3 个作为干扰项（不能重复）
int wrongCount = 0;
for (int i = 0; i < g_wordProgressCount && wrongCount < 3; i++) {
    if (i == correctIdx) continue;
    if (TEST.wrongOptionsUsed[i]) continue;  // 已用过，不能再用
    wrongOptions[wrongCount++] = i;  // 加入选项
}
```

**答题处理：**
```c
// 用户选择了选项 selectedAnswer
// 而正确答案是 currentCorrectIdx
if (selectedAnswer == currentCorrectIdx) {
    // 正确！count++；更新账号累计统计
    TEST.testCorrect++;
    Account_Save();
} else {
    // 错误
}
TEST.testTotal++;
TEST.answerResult = (selectedAnswer == currentCorrectIdx) ? 1 : 0;
```

### 8.6 page_test.c — 测试模式

**函数：** `MenuTest_Show()`

**和选词背单词的区别：** 选词是**看释义选单词**，测试是**看单词选释义**。其他逻辑类似。

### 8.7 page_search.c — 查找单词

**函数：** `MenuSearch_Show()`

**做什么：** 输入搜索关键词，显示匹配的单词列表，点击某个单词显示详情。

**搜索实现：** 调用 `searchWordsByRegex()` 进行通配符搜索，结果存入 `SEARCH` 状态。

### 8.8 page_plan.c — 学习计划（3 个函数）

这个文件包含 3 个页面函数：

**① `MenuPlanRoot_Show()`** — 显示当前计划信息 + 进度条
```c
// 获取当前激活的计划
LearningPlan* plan = Plan_GetActive();
if (plan != NULL) {
    // 显示：计划名称、第X天/共Y天、每日目标、今日已学
    // 进度条：currentDay / totalDays
    // 如果今日已学 ≥ 每日目标，显示"✅ 已完成"
}
```

**② `MenuPlanManager_Show()`** — 计划管理（创建/选择/删除）

创建新计划时需要填写三个字段：
```c
snprintf(name, ..., planNameInput.buffer);     // 计划名称
int daily = atoi(dailyInput.buffer);           // 每日目标词数
int total = atoi(totalInput.buffer);           // 总天数
Plan_Create(name, daily, total);
```

**③ `MenuProgress_Show()`** — 学习进度统计

显示所有单词及其掌握状态，提供"清除进度"按钮。

### 8.9 page_settings.c — 设置

**函数：** `MenuSettings_Show()`

最简单的一个页面。显示：
- 深色模式切换（调用 `AppState_ToggleDarkMode()`）
- 版本信息（写死的字符串）

```c
// 深色模式切换
UICheckbox(u8"深色模式", rect, &g_app.isDarkMode, STYLE, UI_STATE);
if (g_app.isDarkMode) {
    STYLE->theme = UIThemeDark();
} else {
    STYLE->theme = UIThemeLight();
}
```

### 8.10 page_account.c — 账号管理（3 个函数）

**① `MenuAccount_Show()`** — 账号管理主页
- 未登录：显示两个按钮"去登录""去注册"
- 已登录：显示当前用户名、累计答题统计、"登出""切换账号"按钮、"删除账号"按钮

**② `MenuLogin_Show()`** — 登录表单
```c
char username[MAX_USERNAME];  // 用户名输入
char password[MAX_PASSWORD];  // 密码输入（掩码显示）
// 点击"登录"按钮
Account_Login(username, password);
// 登录成功后切换进度/计划文件路径
setProgressFilePath(progressPath);
Plan_SetFilePath(planPath);
```

**③ `MenuRegister_Show()`** — 注册表单
```c
char username[MAX_USERNAME];  // 用户名
char password[MAX_PASSWORD];  // 密码
char confirm[MAX_PASSWORD];   // 确认密码
// 检查两次密码是否一致
if (strcmp(password, confirm) == 0) {
    Account_Register(username, password);
}
```

### 8.11 page_word_manager.c — 词库管理

**函数：** `MenuWordManager_Show()`

这是**交互最复杂的页面**，因为它支持 CRUD（创建、读取、更新、删除）。

**状态变量：** 用 `static` 结构体持久化：

```c
static struct {
    int selectedIdx;                // 当前选中的单词
    int searchResults[MAX_WORDS];   // 搜索结果索引
    int searchCount;                // 搜索结果数
    char searchQuery[64];           // 搜索关键词
    UITextBoxState searchBox;       // 搜索框
    UITextBoxState fieldStates[5];  // 5 个编辑字段的文本框
    bool isAdding;                  // 是否在添加模式
    char msg[128];                  // 操作消息
    int msgTimer;                   // 消息计时
} g_wmState = {0};
```

**搜索功能：** 不区分大小写的子串匹配（`strstr` + `tolower`）。
**添加功能：** 填写 5 个字段（单词/音标/释义/例句/例句翻译）→ 点击"添加"→ `addWordToLibrary()` → `reloadWords()`。
**编辑功能：** 选择单词 → 修改字段 → 点击"保存"→ `editWordInLibrary()` → `reloadWords()`。
**删除功能：** 选择单词 → 点击"删除"→ 确认 → `deleteWordFromLibrary()` → `reloadWords()`。

---

## 第九章：数据持久化 — 文件读写机制

### 9.1 所有数据文件一览

| 文件 | 格式 | 由谁读写 | 在哪读取 | 在哪写入 |
|------|------|---------|---------|---------|
| `data/words.txt` | `word\|phonetic\|def\|ex\|ext\n` | words.c | `loadWordsFromFile()` | `saveWordsToFile()` |
| `data/progress.txt` | `word\|kCnt\|uCnt\|review\n` | words.c | `loadProgress()` | `saveProgress()` |
| `data/accounts.txt` | `usr\|pwd\|ctime\|ltime\|corr\|tot\n` | account.c | `Account_Init()` | `Account_Save()` |
| `data/plans.txt` | `name\|daily\|...\|isActive\n` | plan.c | `Plan_Init()` | `Plan_Save()` |

### 9.2 多用户隔离

不同用户的进度和计划是分开的：

```
未登录时：
    progress.txt         ← 公共进度
    plans.txt            ← 公共计划

登录后（用户名为 hhlong）：
    progress_hhlong.txt  ← 用户专属进度
    plans_hhlong.txt     ← 用户专属计划
```

路径由 `Account_GetProgressPath()` 和 `Account_GetPlanPath()` 生成：

```c
void Account_GetProgressPath(char* buffer, int size) {
    if (s->isLoggedIn) {
        snprintf(buffer, size, "./progress_%s.txt", 
                 s->users[s->currentUserIndex].username);
    } else {
        snprintf(buffer, size, "%s", "./progress.txt");
    }
}
```

### 9.3 什么时候数据被写入文件？

| 操作 | 写入的文件 | 调用的函数 |
|------|-----------|-----------|
| 认识/不认识一个单词 | `progress.txt` | `saveProgress()` |
| 退出程序 | `progress.txt` 未保存（**注意：**当前版本退出时没有显式调用 saveProgress！） | — |
| 注册新账号 | `accounts.txt` | `Account_Save()` |
| 登录/登出 | `accounts.txt`（更新 lastLoginTime） | `Account_Save()` |
| 创建/修改计划 | `plans.txt` | `Plan_Save()` |
| 添加/编辑/删除单词 | `words.txt` | `saveWordsToFile()` |
| 清除所有进度 | `progress.txt` | `saveProgress()` |

**这是目前的一个潜在问题：** 程序退出时没有显式调用 `saveProgress()`，如果用户在学习过程中直接关闭窗口，最后几次的学习记录可能丢失。

---

## 第十章：核心概念总结

### 10.1 "立即模式"UI 总结

```
传统软件（保留模式）：
   内存中有一棵"UI 树"（按钮A、标签B、输入框C……）
   只有状态变化时才重绘对应的部分
   代码复杂，但性能好

本软件（立即模式）：
   没有 UI 树
   每帧完全擦掉重画
   代码简单直观，但性能取决于绘制开销
```

**你的思维应该这样切换：**
- ❌ ~~"按钮被点击后，按钮变成灰色"~~
- ✅ "这一帧，因为 `isDarkMode` 变成了 `true`，所以画按钮时用了深色主题的颜色"

### 10.2 全局变量体系

```
全局变量的三种类型：

1. g_app (AppState) — 集中管理的全局状态
   所有"页面间共享"的状态都存在这里
   通过 Getter 函数访问：AppState_GetStyle(), AppState_GetLearnState() 等

2. 文件级 static 变量
   只在单个 .c 文件中使用
   例如：page_word_manager.c 中的 g_wmState
    
3. 真正的全局变量（不带 static）
   例如：words.c 中的 g_words[], g_wordLibrary
   多个 .c 文件通过 extern 声明访问
```

### 10.3 数据流向总结

```
用户操作 → UIBegin() 检测到事件 → 页面函数判断事件
    → 修改状态变量 → 下一帧 → 状态变量改变 → 画出不同内容
    
具体例子（用户点击"下一个"按钮）：
    ① UIBegin() 检测 mousePressed = true
    ② MenuLearn_Show() 中 UIButton("下一个", ...) 返回 true
    ③ LEARN.learnIndex++（状态改变）
    ④ 下一帧：learnIndex 为 1，右侧显示第 2 个单词
```

### 10.4 命名约定速查

| 前缀 | 含义 | 例子 |
|------|------|------|
| `g_` | 全局变量 | `g_words`, `g_wordLibrary`, `g_app` |
| `_` 开头 | 习惯用的间隔符 | `g_loginForm`, `g_wmState` |
| 全大写 | 宏/常量 | `SCREEN_WIDTH`, `MASTERED_THRESHOLD` |
| 首字母大写函数 | 公共 API | `Account_Login()`, `Plan_Create()` |
| 小写函数 | 内部/辅助 | `find_user()`, `get_today_date()` |
| `MenuXxx_Show` | 页面函数 | `MenuHome_Show()`, `MenuLearn_Show()` |

---

## 第十一章：阅读路线图

这是针对新手的推荐阅读顺序（从易到难）：

### 第一阶段：建立整体认知（30 分钟）

| 步骤 | 读什么 | 关注什么 |
|------|--------|---------|
| 1 | `docs/README.md` | 软件有哪些功能 |
| 2 | `src/core/config.h` | 所有可配置的常量 |
| 3 | `src/main.c` | 启动流程、主循环结构、退出清理 |
| 4 | `src/core/app_state.h` | 全局状态有哪些字段 |

### 第二阶段：理解"页面"是什么（30 分钟）

| 步骤 | 读什么 | 关注什么 |
|------|--------|---------|
| 5 | `src/ui/pages/page_home.c` | 最简单的页面，理解"三步曲"结构 |
| 6 | `src/ui/pages/page_settings.c` | 最简单的功能页面 |
| 7 | `src/ui/pages/page_learn.c` | 中等复杂度：列表+详情+滚动+过滤器 |

此时你应该已经理解了"每帧重新画全部内容"的概念。

### 第三阶段：理解数据和持久化（30 分钟）

| 步骤 | 读什么 | 关注什么 |
|------|--------|---------|
| 8 | `src/modules/words.h` | g_wordLibrary 和 g_words 的关系 |
| 9 | `src/modules/words.c` 的 saveProgress/loadProgress | 怎么从文件存/取数据 |
| 10 | `src/modules/account.h` 和 account.c 的 Login/Register | 账号系统的完整流程 |

### 第四阶段：深入复杂页面（40 分钟）

| 步骤 | 读什么 | 关注什么 |
|------|--------|---------|
| 11 | `src/ui/pages/page_card_review.c` | 两阶段逻辑（学习中→完成） |
| 12 | `src/ui/pages/page_test.c` 或 `page_select_word.c` | 出题逻辑（正确选项+干扰项） |
| 13 | `src/ui/pages/page_word_manager.c` | CRUD 操作 + 搜索功能 |

### 第五阶段：理解基础设施（30 分钟）

| 步骤 | 读什么 | 关注什么 |
|------|--------|---------|
| 14 | `src/ui/menu_callbacks.c` 的 InitMenuTree() | 菜单树怎么构建 |
| 15 | `src/core/tree_menu.h` | 函数指针和导航栈 |
| 16 | `src/core/app_state.c` 的 AppState_Init() | 状态怎么初始化 |
| 17 | `src/modules/fonts.c` 的 loadFonts() | 字体加载策略 |

### 第六阶段：深入 UI 组件（可选）

| 步骤 | 读什么 | 关注什么 |
|------|--------|---------|
| 18 | `src/ui/raylib_word_ui.h` | 所有 UI 组件接口 |
| 19 | `src/ui/raylib_word_ui.c` 的 UIButton() | 按钮怎么检测点击 |
| 20 | `src/ui/raylib_word_ui.c` 的 UITextBox() | 文本框怎么处理 UTF-8 |

---

## 第十二章：常见修改场景（附代码）

### 场景 1：修改主菜单上的文字

**改哪里：** `src/ui/pages/page_home.c`

```c
// 找到这行（约 27 行）：
DrawTextAuto(u8"欢迎使用背单词软件！", ...);
// 改成：
DrawTextAuto(u8"欢迎使用英语学习系统！", ...);
```

### 场景 2：修改"认识 3 次算掌握"为"5 次"

**改哪里：** `src/core/config.h`

```c
#define MASTERED_THRESHOLD     3   // 改成 5
```

### 场景 3：添加一个新单词到词库（不写代码）

打开 `data/words.txt`，在末尾添加一行：

```
memo|ˈmemoʊ|n. 备忘录|I wrote a memo.|我写了一份备忘录。
```

格式：`单词|音标|释义|例句|例句翻译`

### 场景 4：改变窗口大小

**改哪里：** `src/core/config.h`

```c
#define SCREEN_WIDTH  1600   // 改成 1920
#define SCREEN_HEIGHT 1000   // 改成 1080
```

⚠️ **注意：** 按钮坐标是硬编码的，改窗口大小会导致布局错乱。这需要修改每个页面文件中的 `contentRect` 定义。

### 场景 5：修改默认字体大小

**改哪里：** `src/main.c`（约 88~90 行）

```c
STYLE->fontSizeSmall = 22.0f;      // 小号字体
STYLE->fontSizeNormal = 28.0f;     // 普通字体（改成 32.0f 会变大）
STYLE->fontSizeLarge = 46.0f;      // 大号字体
```

### 场景 6：修改主色调颜色

**改哪里：** `src/core/config.h`

```c
#define THEME_PRIMARY (Color){ 30, 144, 255, 255}  // 改成红色：(Color){220, 50, 50, 255}
```

### 场景 7：新增一个简单的页面

假设你想创建一个"关于我们"页面。

**第 1 步：创建文件 `src/ui/pages/page_about.c`**

```c
#include "pages.h"

void MenuAbout_Show(void) {
    Rectangle contentRect = {250, 80, SCREEN_WIDTH - 270, SCREEN_HEIGHT - 100};
    UILayout layout = UIBeginLayout(contentRect, UI_DIR_VERTICAL, 30, 50);

    Rectangle titleRect = UILayoutNext(&layout, -1, 60);
    DrawTextAuto(u8"关于本软件", (Vector2){titleRect.x, titleRect.y}, 36, 1, STYLE->theme.primary);

    Rectangle descRect = UILayoutNext(&layout, -1, 100);
    UIDrawTextRec(u8"版本 5.0\n\n这是一个用 raylib 开发的背单词软件。", 
                  descRect, 24, 1, true, STYLE->theme.textSecondary);
}
```

**第 2 步：在 `pages.h` 中添加声明**

在 `void MenuWordManager_Show(void);` 后面添加：

```c
void MenuAbout_Show(void);
```

**第 3 步：在 `menu_callbacks.c` 的 `InitMenuTree()` 中添加节点**

在 `ConnectMenuTree(g_app.rootMenu, menuWordManager);` 后面添加：

```c
MENU* menuAbout = CreatMenuTreeNode(NULL, MenuAbout_Show);
ConnectMenuTree(g_app.rootMenu, menuAbout);
```

**第 4 步：在 `GetMenuItemText()` 中添加名称**

```c
if (menu->show == MenuAbout_Show) return u8"关于";
```

**第 5 步：在 `CMakeLists.txt` 中添加文件**

在 `src/ui/pages/page_account.c` 后面添加：

```cmake
src/ui/pages/page_about.c
```

### 场景 8：给某个按钮换位置

页面中的所有控件都是通过 `UILayoutNext()` 定位的，按"添加顺序"从上到下排列。如果要改变顺序，只需调整代码顺序。

```c
// 原来是：标题 → 描述 → 按钮
// 要改成：按钮 → 标题 → 描述
Rectangle btnRect = UILayoutNext(&layout, 120, 50);
Rectangle titleRect = UILayoutNext(&layout, -1, 60);
Rectangle descRect = UILayoutNext(&layout, -1, 50);
```

### 场景 9：修改默认的 10 个测试题数量

**改哪里：** `src/core/config.h`

```c
#define TEST_MAX_QUESTIONS  10   // 改成 20
```

### 场景 10：禁掉"强制登录"

**改哪里：** `src/main.c`

```c
// 将第 58 行：
g_app.loginRequired = true;
// 改为：
g_app.loginRequired = false;
```

这样启动后就不会弹出登录遮罩了，所有人都可以直接使用。

---

### 附录：文件依赖速查表

| 文件 | 自己定义的函数/变量 | 被谁使用 |
|------|-------------------|---------|
| `config.h` | 所有常量 | 几乎所有文件 |
| `app_state.h` | `AppState`, `g_app` | 几乎所有文件 |
| `app_state.c` | `AppState_Init()`, `AppState_GetXxx()` | `main.c`, 所有页面文件 |
| `tree_menu.h` | `MENU`, `MenuStack` | `app_state.h`, `menu_callbacks.c`, 页面文件 |
| `tree_menu.c` | `CreatMenuTreeNode()`, `ConnectMenuTree()`, `StackPush/Pop` | `menu_callbacks.c` |
| `words.h` | `g_words[]`, `g_wordLibrary` | 几乎所有页面文件 |
| `words.c` | `load/saveProgress()`, `add/edit/deleteWord()` | 页面文件 + `main.c` |
| `fonts.h` | `g_xxxFont`, `DrawTextAuto()`, `MeasureTextAuto()` | 所有页面文件 + `main.c` |
| `fonts.c` | `loadFonts()`, `unloadFonts()`, `formatTime()` | `main.c` |
| `account.h` | `AccountState`, `User` | 账号相关页面 |
| `account.c` | `Account_Login/Logout/Register()` | 账号页面 + `main.c` |
| `plan.h` | `PlanState`, `LearningPlan` | 计划相关页面 |
| `plan.c` | `Plan_Create/Activate/CheckNewDay()` | 计划页面 + `main.c` |
| `raylib_word_ui.h` | `UIButton()`, `UITextBox()`, `UILayout` 等 | 所有页面文件 |
| `raylib_word_ui.c` | UI 组件实现, 主题, UTF-8 工具函数 | 所有页面文件 |
| `menu_callbacks.h` | 菜单树的 `extern` 声明 | `main.c` |
| `menu_callbacks.c` | `InitMenuTree()`, `DrawTreeMenu()` | `main.c` |
| `pages.h` | 宏 + 页面函数声明 | 所有页面文件 |
| `page_*.c` | `MenuXxx_Show()` 各一个 | 通过函数指针被调用 |
| `main.c` | `main()` | 无（这是入口） |

---

> 文档版本：v2.0（完全版 — 面向新手）
>
> 如果你读到这里还有不清楚的地方，欢迎告诉我具体是哪个文件/哪个函数让你困惑，我会进一步解释。
