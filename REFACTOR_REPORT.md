# 项目变更记录

> 本文档记录项目的所有重要变更，包括功能增减、架构调整、重构等。
> 每次重大修改后必须更新此文档。

---

## 版本信息

| 项目 | 内容 |
|------|------|
| 项目名称 | raylib-word |
| 当前版本 | v5.0.0 |
| 创建日期 | 2025-01-26 |
| 最近更新 | 2026-04-29 |

---

## 更新日志

### v5.0.0 (2026-04-30) - 代码架构重构与质量优化

#### 重构内容

##### 1. PersistentScrollView —— 滚动视图位置自动持久化

**新增文件**：`raylib_word_ui.h/c`

```c
// 新增结构体
typedef struct {
    UIScrollView sv;         // 内部的滚动视图
    float* persistedOffset;  // 指向外部持久化偏移量
} PersistentScrollView;

// 新增 API
void UIBeginPersistentScrollView(PersistentScrollView* psv, Rectangle viewport,
                                 Vector2 contentSize, float* offsetPtr);
void UIEndPersistentScrollView(PersistentScrollView* psv, UIStyle* style, UIState* state);
```

**解决的问题**：开发中多个页面（词库管理列表、侧边导航栏、计划列表）出现滚动位置每帧重置的 Bug，因为 `UIScrollView` 是局部变量。

**旧写法**（每处手动管理）：
```c
static float g_scroll = 0.0f;
UIScrollView sv = {0};
sv.scrollOffset.y = g_scroll;
// ... 绘制 ...
UIEndScrollView(&sv, STYLE, UI_STATE);
g_scroll = sv.scrollOffset.y;
```

**新写法**（封装自动化）：
```c
static float g_scroll = 0.0f;
PersistentScrollView psv = {0};
UIBeginPersistentScrollView(&psv, viewport, contentSize, &g_scroll);
// ... 绘制 ...
UIEndPersistentScrollView(&psv, STYLE, UI_STATE);
```

##### 2. 改进建议文档

**新增文件**：`improvement_suggestions.md` 第 11 章

新增 10 条基于实际 Bug 经验的代码质量优化建议：
1. 统一状态绑定模式（防止重复 Bug）
2. 滚动视图位置持久化封装
3. 全局调试标记系统
4. 输入框焦点管理优化
5. 字体预加载自动化
6. 字符串操作安全封装
7. 菜单注册模式简化
8. 构建脚本增强
9. 统一消息提示组件
10. 本地化/配置分离

##### 3. 调试日志文档

**新增文件**：`BACKSPACE_SUPPORT_REPORT.md`

新增「经验 12：计划数据不显示」章节，详细记录了与账号系统相同的状态绑定 Bug 的发现、诊断和修复过程。

---

#### 当前代码架构与层级

```
┌─────────────────────────────────────────────────────────┐
│                     表现层 (UI)                          │
│  main.c           ─ 主循环、登录遮罩、页面调度            │
│  menu_callbacks.c ─ 所有菜单页面渲染函数                 │
│  fonts.c          ─ 字体加载、混合文本绘制               │
│  raylib_word_ui.c ─ UI 组件库（按钮/输入框/滚动视图）    │
│  tree_menu.c      ─ 树形菜单导航系统                    │
├─────────────────────────────────────────────────────────┤
│                     业务层 (Service)                    │
│  words.c          ─ 单词加载、进度管理、搜索             │
│  plan.c           ─ 学习计划管理（独立库）               │
│  account.c        ─ 账号管理系统（独立库）               │
├─────────────────────────────────────────────────────────┤
│                     数据层 (Data)                       │
│  app_state.c      ─ 统一状态管理（AppState）             │
│                    └─ 状态绑定：account/plan 注册到此处  │
│                    └─ 菜单系统、主题、各模式状态         │
│  config.h         ─ 配置常量（窗口/学习参数/UI 尺寸）    │
│  words.txt        ─ 单词数据文件                        │
│  accounts.txt     ─ 账号数据文件                        │
│  plans.txt        ─ 学习计划数据文件                     │
│  progress.txt     ─ 学习进度文件                        │
├─────────────────────────────────────────────────────────┤
│                     独立库模块                          │
│  account.c/h      ─ 账号库（注册/登录/登出/状态绑定）    │
│  plan.c/h         ─ 学习计划库（创建/删除/激活/持久化）  │
│  raylib_word_ui   ─ UI 组件库（按钮/输入框/滚动/闪卡）  │
└─────────────────────────────────────────────────────────┘
```

**各层职责说明**：

| 层级 | 职责 | 依赖关系 |
|------|------|----------|
| 表现层 | 用户界面渲染、事件处理 | 依赖业务层获取数据 |
| 业务层 | 核心业务逻辑（单词管理/计划/账号） | 独立模块，通过 API 暴露功能 |
| 数据层 | 状态管理、配置、文件持久化 | 被所有层引用 |
| 独立库 | 可复用的功能模块 | 通过 `SetState` 绑定到数据层 |

**关键设计模式**：

1. **状态绑定模式**（`account.c` / `plan.c`）
   - 库内部通过 `getState()` 获取状态指针
   - `SetState()` 绑定到 `AppState` 的字段
   - 未绑定时使用内部备用状态

2. **菜单树模式**（`tree_menu.c`）
   - 树形菜单节点通过函数指针关联渲染函数
   - 菜单栈实现"返回"导航
   - `GetMenuItemText` 通过函数指针匹配判断菜单名

3. **滚动视图持久化**（`PersistentScrollView`）
   - 封装 `UIScrollView` + 外部 `float*`
   - 自动读写滚动位置，消除手动管理

---

### v4.1.1 (2026-04-29) - 计划数据绑定修复 + 删除按钮

#### 修复内容

1. **计划数据不显示** — `plan.c`
   - 根因：与账号系统相同的 Bug（经验 12），`plan.c` 内部 `static PlanState` 与 `g_app.plan` 不同步
   - 修复：添加 `Plan_SetState()` + `getPlanState()` 模式，在 `AppState_Init()` 中绑定

2. **字体补充** — `fonts.c`
   - allChinese 新增 5 个汉字：周半巩阶刺

#### 新增内容

1. **计划删除按钮** — `menu_callbacks.c`
   - 每个计划项右上角添加红色 ✕ 删除按钮
   - 悬停时背景变红，点击调用 `Plan_Delete(i)`

#### 文件变更

| 文件 | 变更 |
|------|------|
| `plan.h/c` | 新增 `Plan_SetState()` 状态绑定 |
| `app_state.c` | 添加 `Plan_SetState(&g_app.plan)` 绑定调用 |
| `menu_callbacks.c` | 计划列表添加 ✕ 删除按钮 |
| `fonts.c` | allChinese 补充周半巩阶刺 |
| `BACKSPACE_SUPPORT_REPORT.md` | 新增经验 12 |

---

### v4.1.0 (2026-04-29) - 登录强制 + 计划多用户支持

#### 新增内容

1. **登录强制** - `main.c`
   - 程序启动后弹出登录遮罩层（半透明黑色背景 + 提示面板）
   - 未登录状态下无法操作任何功能，所有页面被覆盖
   - 登录成功或注册成功后自动解除锁定
   - 提示面板包含"登录"和"注册账号"两个按钮

2. **学习计划按用户隔离**
   - 每个用户的计划独立保存：`plans_<用户名>.txt`
   - 登录后自动切换到对应用户的计划文件
   - 登出后恢复默认 `plans.txt`
   - `plan.h/c` 新增 `Plan_SetFilePath()` 动态切换路径
   - `account.h/c` 新增 `Account_GetPlanPath()` 获取用户计划路径

3. **计划列表视觉优化** - `menu_callbacks.c`
   - 激活计划：蓝色边框 + 左侧指示条 + "▶ 当前使用"标签
   - 未激活计划：鼠标悬停浅灰背景
   - 每项增加进度条显示完成百分比

#### 相关文件变更
   - `app_state.h` - 新增 `loginRequired` 字段
   - `main.c` - 初始化 `loginRequired=true`，主循环中绘制遮罩层
   - `menu_callbacks.c` - 登录/注册成功后设置 `loginRequired=false`、切换计划路径、视觉优化

#### 修复内容

1. **登录遮罩行为优化** - `main.c`
   - 遮罩仅在非账号页面显示（点击登录/注册后自动隐藏，进入账号页面）
   - 未登录时点击其他模块遮罩重新弹出
   - 移除 ✕ 关闭按钮，只能通过登录/注册成功解除
   - `plan.h/c` - 新增 `Plan_SetFilePath()`，路径动态化
   - `account.h/c` - 新增 `Account_GetPlanPath()`

---

### v4.0.0 (2026-04-29) - 学习计划管理系统

#### 新增内容

1. **独立学习计划库** - `plan.c` / `plan.h`
   - 独立模块，提供完整的计划管理 API
   - 计划数据结构：名称、每日单词数、总天数、当前进度
   - 自动检测新的一天并重置每日计数
   - 数据持久化到 `plans.txt`

2. **四项默认计划** - 首次运行自动创建
   | 计划名 | 每日词数 | 天数 |
   |--------|----------|------|
   | 一周入门计划 | 10 | 7 |
   | 半月巩固计划 | 15 | 15 |
   | 三十天进阶计划 | 20 | 30 |
   | 六十天冲刺计划 | 30 | 60 |

3. **计划管理页面** - `MenuPlanManager_Show()`
   - 左侧：可滚动计划列表（显示名称、参数、当前天数）
   - 右侧：创建新计划表单（名称 + 每日词数 + 总天数）
   - 点击列表项直接激活，点击 ✕ 删除
   - "使用默认"一键恢复四项默认计划

4. **学习计划父页面** - `MenuPlanRoot_Show()`
   - 当前激活计划卡片（名称 + 天数 + 今日进度条）
   - "计划管理"和"学习进度"两个功能入口

5. **菜单结构重构**
   - 根菜单新增"学习计划"（替换原"学习进度"位置）
   - "学习进度"移入"学习计划"子菜单
   - 学习计划 → 计划管理、学习进度

#### 文件清单

| 文件 | 类型 | 说明 |
|------|------|------|
| `plan.h` | 新增 | 学习计划库头文件，定义 API |
| `plan.c` | 新增 | 学习计划库实现 |
| `app_state.h/c` | 修改 | 添加 PlanState |
| `menu_callbacks.h/c` | 修改 | 新增计划页面，重构菜单树 |
| `main.c` | 修改 | 添加 Plan_Init |
| `CMakeLists.txt` | 修改 | 添加 plan.c/h |

---

### v3.1.0 (2026-04-29) - 词库管理功能

#### 新增内容

1. **词库管理页面** - `MenuWordManager_Show()`
   - 侧边导航新增"词库管理"菜单项
   - 左侧可滚动单词列表（支持搜索过滤）
   - 右侧编辑/详情区域
   - 支持**新增单词**：填写单词、音标、释义、例句、例句翻译后保存
   - 支持**编辑单词**：点击单词列表中的单词，修改字段后保存
   - 支持**删除单词**：选中单词后删除
   - 所有修改实时保存到 `words.txt`

2. **词库管理 API** - `words.c/h`
   - `saveWordsToFile()` - 保存词库到文件
   - `addWordToLibrary()` - 添加单词
   - `editWordInLibrary()` - 编辑单词
   - `deleteWordFromLibrary()` - 删除单词
   - `reloadWords()` - 重新加载进度数组

#### 文件变更

| `menu_callbacks.h/c` | 新增 `MenuWordManager_Show`，InitMenuTree 添加词库管理节点 |
| `words.h` | 新增 5 个词库管理函数声明，`g_wordLibrary` 改为 extern |
| `words.c` | 新增 5 个词库管理函数实现，`g_wordLibrary` 改为非静态 |

#### 修复内容

1. **左侧导航栏不可滚动** - `DrawTreeMenu()`
   - 菜单项区域从 `UILayout` 改为 `UIScrollView` 滚动视图
   - 解决 7 个菜单项超出面板高度无法查看的问题

2. **UITextBox 焦点清除逻辑** - `raylib_word_ui.c`
   - 焦点检测从 `uistate->focusItem` 比较改为直接检查 `state->hasFocus`
   - 修复多个输入框时点击新输入框后旧输入框仍保留焦点的问题

3. **词库管理首屏不显示单词列表**
   - `lastSearchLen` 初始值 `0`→`-1`，确保首次进入时自动触发搜索

4. **字体补充** - `fonts.c`
   - allChinese 新增汉字：编、框|

---

### v3.0.0 (2026-04-29) - 背单词模式重构

#### 新增内容

1. **背单词模式拆分为三个子模式** - `menu_callbacks.c`
   - **背单词父页面** `MenuReviewRoot_Show()`：三个模式卡片入口（卡片背单词、选词背单词、测试）
   - **卡片背单词** `MenuCardReview_Show()`：原闪卡翻转模式（由 `MenuReview_Show` 更名）
   - **选词背单词** `MenuSelectWord_Show()`：看汉语释义选英文单词，四选一
   - **测试模式** `MenuTest_Show()`：从主菜单移至背单词子模式

2. **选词背单词模式** - 完整选择题流程
   - 显示汉语释义作为题干
   - 四个英文单词选项中选正确答案
   - 答对/答错即时颜色反馈
   - 进度条和正确率实时显示
   - 完成后显示得分评价

3. **用户按账号统计积分** - `account.h/c`
   - User 结构新增 `selectWordCorrect` / `selectWordTotal` 字段
   - 每次完成选词练习后自动累加到当前登录账号
   - 积分持久化保存到 `accounts.txt`
   - 完成页显示"账号累计"统计数据

#### 架构变更

| 旧结构 | 新结构 |
|--------|--------|
| 根菜单 → 背单词 | 根菜单 → 背单词（父节点） |
| 根菜单 → 测试 | → 卡片背单词（子节点） |
| | → 选词背单词（子节点） |
| | → 测试模式（子节点） |

- 主页面移除了"测试"卡片，改为三个卡片：学单词、背单词、查找单词
- 侧边栏点击"背单词"后显示三个子模式按钮
- `tree_menu.h` 的 `MAX_CHILD_NUM` 保持 8 不变

#### 新增/修改文件

| 文件 | 变更 |
|------|------|
| `menu_callbacks.h` | 新增 `MenuReviewRoot_Show`、`MenuSelectWord_Show` 声明；`MenuReview_Show`→`MenuCardReview_Show` |
| `menu_callbacks.c` | 新增三个函数，重命名一个函数，更新 InitMenuTree 和 GetMenuItemText，更新主页面卡片 |
| `app_state.h` | 新增 `SelectWordState` 结构体和字段 |
| `app_state.c` | 新增 `SelectWordState` 初始化+访问器 |
| `account.h` | User 新增 `selectWordCorrect` / `selectWordTotal` 字段 |
| `account.c` | 文件格式扩展为6字段，保存/加载积分数据 |
| `main.c` | 添加 `SELECT_WORD` 宏 |

---

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
