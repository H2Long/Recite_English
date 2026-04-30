# 背单词软件 v5.0.0 - API 函数指南

> 本文档按模块（文件）分类，列出所有公共函数的签名、功能说明、参数和返回值。

---

## 目录

1. [src/main.c — 程序入口与主循环](#1-srcmainc--程序入口与主循环)
2. [src/core/config.h — 配置常量](#2-srccoreconfigh--配置常量)
3. [src/core/app_state.c/h — 统一状态管理](#3-srccoreapp_statech--统一状态管理)
4. [src/core/tree_menu.c/h — 树形菜单系统](#4-srccoretree_menuch--树形菜单系统)
5. [src/modules/account.c/h — 账号管理系统](#5-srcmodulesaccountch--账号管理系统)
6. [src/modules/plan.c/h — 学习计划系统](#6-srcmodulesplanch--学习计划系统)
7. [src/modules/words.c/h — 单词管理模块](#7-srcmoduleswordsch--单词管理模块)
8. [src/modules/fonts.c/h — 字体渲染模块](#8-srcmodulesfontsch--字体渲染模块)
9. [src/ui/raylib_word_ui.c/h — UI 组件库](#9-srcuiraylib_word_uich--ui-组件库)
10. [src/ui/menu_callbacks.c/h — 菜单页面回调](#10-srcuimenu_callbacksch--菜单页面回调)

---

## 1. src/main.c — 程序入口与主循环

### `int main(void)`

**功能**：程序入口。初始化 raylib 窗口、加载数据、进入主循环、退出清理。

**流程**：
1. 创建窗口 (1600×1000, 60 FPS)
2. 初始化：加载单词 → 加载账号 → 加载计划 → 初始化进度 → 加载字体 → 初始化 UI → 初始化状态 → 构建菜单树
3. 进入主循环（每帧：更新UI → 绘制标题栏 → 绘制侧边菜单 → 绘制页面 → 登录遮罩）
4. 退出清理：释放状态 → 卸载字体 → 释放单词库 → 关闭窗口

**返回值**：0（正常退出）

### 宏定义

| 宏 | 展开为 | 用途 |
|----|--------|------|
| `STYLE` | `AppState_GetStyle()` | 获取 UI 样式指针 |
| `UI_STATE` | `AppState_GetUIState()` | 获取 UI 交互状态 |
| `CURRENT_MENU` | `(*AppState_GetCurrentMenu())` | 获取/设置当前菜单 |
| `LEARN` | `(*AppState_GetLearnState())` | 学单词状态 |
| `REVIEW` | `(*AppState_GetReviewState())` | 背单词状态 |
| `TEST` | `(*AppState_GetTestState())` | 测试状态 |
| `SELECT_WORD` | `(*AppState_GetSelectWordState())` | 选词状态 |

---

## 2. src/core/config.h — 配置常量

本文件定义了所有硬编码常量，**不包含函数**。主要分组如下：

### 窗口配置
| 宏 | 值 | 说明 |
|----|-----|------|
| `SCREEN_WIDTH` | 1600 | 窗口宽度（像素） |
| `SCREEN_HEIGHT` | 1000 | 窗口高度（像素） |
| `WINDOW_TITLE` | "背单词软件" | 窗口标题 |
| `TARGET_FPS` | 60 | 目标帧率 |

### 学习参数
| 宏 | 值 | 说明 |
|----|-----|------|
| `MASTERED_THRESHOLD` | 3 | 认识几次算"已掌握"（艾宾浩斯简化） |
| `DAILY_GOAL_DEFAULT` | 20 | 每日学习目标 |
| `REVIEW_INTERVAL_DAYS` | 1 | 复习间隔 |
| `MAX_WRONG_COUNT` | 5 | 错误几次后重点标记 |

### UI 布局
| 宏 | 值 | 说明 |
|----|-----|------|
| `FONT_SIZE_TITLE` | 32 | 标题字体 |
| `FONT_SIZE_NORMAL` | 20 | 正文字体 |
| `FONT_SIZE_SMALL` | 16 | 小字体 |
| `CARD_WIDTH/HEIGHT` | 600×400 | 单词卡片尺寸 |
| `BUTTON_HEIGHT_NORMAL` | 50 | 按钮高度 |

### 测试配置
| 宏 | 值 | 说明 |
|----|-----|------|
| `TEST_MAX_QUESTIONS` | 10 | 最大题目数 |
| `OPTIONS_COUNT` | 4 | 选项数量 |

### 文件路径
| 宏 | 值 | 说明 |
|----|-----|------|
| `WORDS_FILE_PATH` | `"./data/words.txt"` | 单词数据文件 |
| `PROGRESS_FILE_PATH` | `"./data/progress.txt"` | 学习进度文件 |
| `FONT_FILE_PATH` | `"./data/fonts/NotoSansCJK.otf"` | 中文字体文件 |

---

## 3. src/core/app_state.c/h — 统一状态管理

### 数据结构

#### `AppState` — 全局应用状态结构体
```c
typedef struct {
    UIStyle style;             // UI 样式（主题、字体、字号）
    UIState uiState;           // UI 交互状态（鼠标、键盘）
    bool isDarkMode;           // 深色模式开关
    MENU* rootMenu;            // 根菜单节点
    MENU* currentMenu;         // 当前显示的菜单
    MenuStack menuStack;       // 菜单栈（"返回"导航）
    LearnState learn;          // 学单词模式状态
    ReviewState review;        // 背单词模式状态
    TestState test;            // 测试模式状态
    SelectWordState selectWord; // 选词背单词状态
    SearchState search;        // 查找单词状态
    AccountState account;      // 账号状态
    char loginMsg[128];        // 登录/注册提示信息
    PlanState plan;            // 学习计划状态
    bool loginRequired;        // 是否强制登录
} AppState;
```

### 公共函数

#### `void AppState_Init(void)`
- **功能**：初始化所有子状态为默认值，绑定账号和计划系统状态。

#### `void AppState_Reset(void)`
- **功能**：重置各学习模式的状态（不清除菜单树），用于重新开始学习。

#### `void AppState_Deinit(void)`
- **功能**：释放菜单树内存，清理应用状态。

#### `UIStyle* AppState_GetStyle(void)`
- **功能**：获取 UI 样式指针。
- **返回值**：指向 `g_app.style` 的指针。

#### `UIState* AppState_GetUIState(void)`
- **功能**：获取 UI 交互状态指针。

#### `MENU** AppState_GetCurrentMenu(void)`
- **功能**：获取当前菜单指针的指针。通过解引用可获取/设置当前菜单。

#### `MenuStack* AppState_GetMenuStack(void)`
- **功能**：获取菜单栈指针。

#### `LearnState* AppState_GetLearnState(void)`
- **功能**：获取学单词模式状态指针。

#### `ReviewState* AppState_GetReviewState(void)`
- **功能**：获取背单词模式状态指针。

#### `TestState* AppState_GetTestState(void)`
- **功能**：获取测试模式状态指针。

#### `SearchState* AppState_GetSearchState(void)`
- **功能**：获取查找单词模式状态指针。

#### `SelectWordState* AppState_GetSelectWordState(void)`
- **功能**：获取选词背单词模式状态指针。

#### `PlanState* AppState_GetPlanState(void)`
- **功能**：获取学习计划状态指针。

#### `AccountState* AppState_GetAccountState(void)`
- **功能**：获取账号状态指针。

#### `char* AppState_GetLoginMsg(void)`
- **功能**：获取登录/注册提示消息缓冲区指针。

#### `bool AppState_IsDarkMode(void)`
- **功能**：检查当前是否为深色模式。
- **返回值**：`true` 深色模式，`false` 浅色模式。

#### `void AppState_SetDarkMode(bool isDark)`
- **功能**：设置深色/浅色模式，自动更新 `style.theme`。

#### `void AppState_ToggleDarkMode(void)`
- **功能**：切换深色/浅色模式。

---

## 4. src/core/tree_menu.c/h — 树形菜单系统

### 数据结构

#### `MENU` — 菜单节点
```c
typedef struct menu {
    void (*show)();                    // 页面渲染函数
    void(*fun)();                      // 页面操作函数（当前未使用）
    struct menu* child[8];             // 子节点数组（最多8个）
    struct menu* parent;               // 父节点
    int currentindex;                  // 在父节点中的索引
    int childindex;                    // 已连接的子节点数
} MENU;
```

#### `MenuStack` — 菜单导航栈
```c
typedef struct {
    MENU* menuStack[10];  // 栈数组（最大10层）
    int Stacktop;         // 栈顶索引（-1=空栈）
} MenuStack;
```

### 公共函数

#### `MENU* CreatMenuTreeNode(void(*fun)(), void (*show)())`
- **功能**：创建新的菜单树节点。
- **参数**：
  - `fun` — 操作函数指针（可为 NULL）
  - `show` — 页面渲染函数指针
- **返回值**：新创建的菜单节点指针，失败返回 NULL。

#### `int ConnectMenuTree(MENU* parentNode, MENU* childNode)`
- **功能**：将子节点连接到父节点，建立双向链接。
- **参数**：
  - `parentNode` — 父节点
  - `childNode` — 子节点
- **返回值**：0 成功，-1 失败（参数无效或子节点已满）。

#### `void StackInit(MenuStack* MenuBack)`
- **功能**：初始化菜单栈，清空所有元素并设置栈顶为 -1。

#### `void StackPush(MenuStack* MenuBack, MENU* MenuCurrent)`
- **功能**：将菜单压入栈中（记录访问历史，用于"返回"功能）。
- **参数**：
  - `MenuBack` — 目标栈
  - `MenuCurrent` — 要压入的菜单节点

#### `MENU* StackPop(MenuStack* MenuBack)`
- **功能**：从栈中弹出菜单（返回上一个页面）。
- **参数**：
  - `MenuBack` — 目标栈
- **返回值**：弹出的菜单节点，栈空返回 NULL。

---

## 5. src/modules/account.c/h — 账号管理系统

### 数据结构

#### `User` — 用户信息
```c
typedef struct {
    char username[32];          // 用户名
    char passwordHash[64];      // 密码哈希值（djb2 算法）
    time_t createdTime;         // 注册时间
    time_t lastLoginTime;       // 上次登录时间
    int selectWordCorrect;     // 选词模式答对数
    int selectWordTotal;       // 选词模式总题数
} User;
```

#### `AccountState` — 账号系统状态
```c
typedef struct {
    User users[50];       // 用户数组（最多50个）
    int userCount;        // 用户总数
    int currentUserIndex; // 当前登录用户索引（-1=未登录）
    bool isLoggedIn;      // 是否已登录
} AccountState;
```

### 公共函数

#### `void Account_SetState(AccountState* state)`
- **功能**：绑定外部状态指针到 AppState。如果不调用，库使用内部备用状态。
- **参数**：`state` — 外部 `AccountState` 指针（传入 `&g_app.account`）

#### `void Account_Init(void)`
- **功能**：初始化账号系统，从 `accounts.txt` 加载用户数据。文件不存在时创建默认账号（admin/admin）。

#### `void Account_Save(void)`
- **功能**：将内存中的用户数据保存到 `accounts.txt` 文件。格式：`username|hash|createdTime|lastLoginTime|crt|total`

#### `bool Account_Register(const char* username, const char* password)`
- **功能**：注册新用户。
- **参数**：
  - `username` — 用户名（不能为空，不能重复，最长31字符）
  - `password` — 密码（不能为空）
- **返回值**：`true` 注册成功，`false` 失败（用户名重复/无效/已满）。
- **副作用**：自动保存到文件，打印 INFO 日志。

#### `bool Account_Login(const char* username, const char* password)`
- **功能**：用户登录验证。
- **参数**：
  - `username` — 用户名
  - `password` — 明文密码（内部哈希后比对）
- **返回值**：`true` 登录成功，`false` 失败。
- **副作用**：登录成功自动更新 `lastLoginTime` 并保存。

#### `void Account_Logout(void)`
- **功能**：用户登出，清除 `currentUserIndex` 和 `isLoggedIn`。

#### `bool Account_IsLoggedIn(void)`
- **功能**：检查当前是否已登录。
- **返回值**：`true` 已登录。

#### `const char* Account_GetCurrentUser(void)`
- **功能**：获取当前登录用户名。
- **返回值**：用户名字符串，未登录返回空字符串 `""`。

#### `int Account_GetCurrentIndex(void)`
- **功能**：获取当前登录用户索引。
- **返回值**：用户索引（0 ~ userCount-1），未登录返回 -1。

#### `void Account_GetProgressPath(char* buffer, int size)`
- **功能**：获取当前用户的专属进度文件路径。
- **参数**：
  - `buffer` — 输出缓冲区
  - `size` — 缓冲区大小
- **输出示例**：`"./progress_hhlong.txt"`（已登录）或 `"./progress.txt"`（未登录）

#### `void Account_GetPlanPath(char* buffer, int size)`
- **功能**：获取当前用户的专属计划文件路径。
- **参数**：
  - `buffer` — 输出缓冲区
  - `size` — 缓冲区大小
- **输出示例**：`"./plans_hhlong.txt"`（已登录）或 `"./plans.txt"`（未登录）

---

## 6. src/modules/plan.c/h — 学习计划系统

### 数据结构

#### `LearningPlan` — 学习计划
```c
typedef struct {
    char name[64];           // 计划名称
    int dailyWordCount;      // 每日单词数
    int totalDays;           // 总天数
    int currentDay;          // 当前天数（0-based）
    time_t createdAt;        // 创建时间
    time_t lastStudyDate;    // 上次学习日期
    int studiedToday;        // 今日已学单词数
} LearningPlan;
```

#### `PlanState` — 计划系统状态
```c
typedef struct {
    LearningPlan plans[20];  // 所有计划（最多20个）
    int planCount;           // 计划总数
    int activePlanIndex;     // 当前激活计划索引（-1=无）
} PlanState;
```

### 公共函数

#### `void Plan_SetState(PlanState* state)`
- **功能**：绑定外部计划状态指针到 AppState。

#### `void Plan_SetFilePath(const char* path)`
- **功能**：更改计划文件路径并重新加载（用于多用户切换）。
- **参数**：`path` — 新路径（如 `"./data/plans_hhlong.txt"`）

#### `void Plan_Init(void)`
- **功能**：初始化计划系统。从文件加载计划数据，文件不存在时添加4个默认计划并保存。

**默认计划**：

| 名称 | 每日词数 | 天数 |
|------|---------|------|
| 一周入门计划 | 10 | 7 |
| 半月巩固计划 | 15 | 15 |
| 三十天进阶计划 | 20 | 30 |
| 六十天冲刺计划 | 30 | 60 |

#### `void Plan_Save(void)`
- **功能**：将计划数据保存到文件。格式：`name|daily|totalDays|currentDay|createdAt|lastStudyDate|studiedToday|isActive`

#### `bool Plan_Create(const char* name, int daily, int days)`
- **功能**：创建新学习计划。
- **参数**：
  - `name` — 计划名称（不能为空，不能重名）
  - `daily` — 每日单词数（>0）
  - `days` — 总天数（>0）
- **返回值**：`true` 创建成功。

#### `bool Plan_Delete(int index)`
- **功能**：删除指定索引的计划。如果删除的是激活计划，自动取消激活。
- **返回值**：`true` 删除成功。

#### `void Plan_SetActive(int index)`
- **功能**：设置激活计划。激活时自动检查并更新每日状态。传入 -1 取消激活。

#### `LearningPlan* Plan_GetActive(void)`
- **功能**：获取当前激活的学习计划。
- **返回值**：计划指针，无激活返回 NULL。

#### `int Plan_GetActiveIndex(void)`
- **功能**：获取激活计划索引。
- **返回值**：索引，-1 表示无激活计划。

#### `void Plan_AddStudiedToday(int count)`
- **功能**：增加今日已学单词数（累加），更新 `lastStudyDate`。

#### `void Plan_CheckNewDay(void)`
- **功能**：检查是否为新的一天。如果是，累加 `currentDay`，重置今日计数。

#### `int Plan_GetRemainingToday(void)`
- **功能**：获取今日剩余应学单词数（`dailyWordCount - studiedToday`）。

#### `void Plan_AddDefaults(void)`
- **功能**：添加4个默认计划，自动激活第一个。

---

## 7. src/modules/words.c/h — 单词管理模块

### 全局变量

| 变量 | 类型 | 说明 |
|------|------|------|
| `g_words[MAX_WORDS]` | `WordWithProgress[]` | 含学习进度的单词数组（核心数据） |
| `g_wordLibrary` | `WordEntry*` | 单词库动态数组（支持增删改） |
| `g_wordProgressCount` | `int` | g_words 中有效单词数 |
| `g_wordCount` | `int` | g_wordLibrary 中单词总数 |

### 公共函数

#### `void loadWordsFromFile(const char* filename)`
- **功能**：从文件加载单词到 `g_wordLibrary`。文件格式：`word|phonetic|definition|example|exampleTranslation`
- **参数**：`filename` — 单词数据文件路径
- **文件不存在时**：使用内置10个默认单词（abandon ~ accomplish）

#### `void freeWordLibrary(void)`
- **功能**：释放 `g_wordLibrary` 所有内存。
- **调用时机**：程序退出时。

#### `void initWords(void)`
- **功能**：初始化单词进度。将 `g_wordLibrary` 复制到 `g_words`，设置默认识别次数，然后从文件恢复学习进度。

#### `void shuffleArray(int *array, int count)`
- **功能**：Fisher-Yates 洗牌算法，随机打乱数组顺序。
- **用途**：用于随机出题和随机单词顺序。

#### `void setProgressFilePath(const char* path)`
- **功能**：设置学习进度文件路径（多用户切换时使用）。

#### `void saveProgress(void)`
- **功能**：保存学习进度到文件。格式：`word|knownCount|unknownCount|lastReview`

#### `void loadProgress(void)`
- **功能**：从文件加载学习进度。认识次数 ≥ 3 的单词自动标记为"已掌握"。

#### `void clearProgress(void)`
- **功能**：清除所有单词的学习进度，重置为初始状态。

#### `int searchWordsByRegex(const char* pattern, int* results, int maxResults)`
- **功能**：使用正则表达式搜索单词（不区分大小写）。
- **参数**：
  - `pattern` — 正则表达式模式（如 `"ab.*"`）
  - `results` — 输出数组，存放匹配单词的索引
  - `maxResults` — 最大结果数
- **返回值**：匹配到的单词数量。
- **回退**：正则编译失败时自动降级为简单子串匹配。

#### `int searchWordsSimple(const char* query, int* results, int maxResults)`
- **功能**：简单的模糊搜索，不区分大小写的子串匹配。
- **参数**：
  - `query` — 搜索关键词
  - `results` — 输出数组
  - `maxResults` — 最大结果数

#### `bool saveWordsToFile(const char* filename)`
- **功能**：将 `g_wordLibrary` 保存到文件。
- **返回值**：`true` 保存成功。

#### `bool addWordToLibrary(const char* word, const char* phonetic, const char* definition, const char* example, const char* exampleTranslation)`
- **功能**：添加新单词到词库。自动扩容，自动保存文件。
- **注意**：添加后需调用 `reloadWords()` 同步到 `g_words`。

#### `bool editWordInLibrary(int index, const char* word, const char* phonetic, const char* definition, const char* example, const char* exampleTranslation)`
- **功能**：编辑词库中指定索引的单词。自动保存文件。

#### `bool deleteWordFromLibrary(int index)`
- **功能**：删除词库中指定索引的单词。后续单词前移填补空缺。

#### `void reloadWords(void)`
- **功能**：将 `g_wordLibrary` 的最新数据同步到 `g_words`。新增索引初始化默认进度，已有进度保留。

---

## 8. src/modules/fonts.c/h — 字体渲染模块

### 全局字体变量

| 变量 | 说明 |
|------|------|
| `g_chineseFont` | 中文字体（NotoSansCJK） |
| `g_englishFont` | 英文字体（DejaVu Sans） |
| `g_latinFont` | IPA 音标字体 |
| `g_mergedFont` | 合并字体（中+IPA+ASCII，用于 UI） |

### 公共函数

#### `void loadFonts(void)`
- **功能**：加载所有字体。按优先级依次尝试多个候选路径：
  1. **英文字体**：DejaVu Sans → DejaVu Sans Mono
  2. **IPA 字体**：IPA 日文字体 → Noto → DejaVu → 默认字体
  3. **中文字体**：NotoSansCJK.otf → DroidSansFallback → simhei.ttf
  4. **合并字体**：基于中文字体合并 IPA 和 ASCII 字形

#### `void unloadFonts(void)`
- **功能**：卸载已加载的自定义字体（跳过默认字体）。

#### `Vector2 MeasureTextAuto(const char* text, float fontSize, float spacing)`
- **功能**：测量混合文本（中/英/IPA）的渲染宽度。自动识别字符类型选择对应字体。
- **返回值**：`{ width, fontSize }`

#### `void DrawTextAuto(const char* text, Vector2 pos, float fontSize, float spacing, Color tint)`
- **功能**：绘制单行混合文本，自动为每个字符选择正确字体。
- **字符识别规则**：
  - ASCII (0x00-0x7F) → 英文字体
  - IPA/Latin 扩展 (0x00A0-0x036F 等) → IPA 字体
  - 中文/其他 → 合并字体

#### `Color getMasteryColor(float mastery)`
- **功能**：根据熟练度返回颜色。
- **规则**：
  - ≥ 80%：绿色 `(50, 205, 50)` — 已掌握
  - ≥ 50%：橙色 `(255, 165, 0)` — 学习中
  - < 50%：红色 `(220, 20, 60)` — 待学习

#### `const char* formatTime(time_t timestamp)`
- **功能**：格式化时间戳为可读字符串（"月-日 时:分"）。
- **特殊值**：`timestamp == 0` 返回 `"从未"`。

---

## 9. src/ui/raylib_word_ui.c/h — UI 组件库

### 数据结构

| 结构体 | 说明 |
|--------|------|
| `UITheme` | 主题颜色配置（primary/background/textPrimary 等12色） |
| `UIStyle` | UI 样式（theme + font + fontSize + spacing + cornerRadius） |
| `UIState` | 交互状态（mousePos, mouseDown, keyboard 等） |
| `UILayout` | 布局容器（方向/间距/光标位置） |
| `UIScrollView` | 滚动视图状态（视口/内容/偏移/滚动条） |
| `PersistentScrollView` | 持久化滚动视图（自动保存滚动位置） |
| `UITextBoxState` | 文本输入框状态（缓冲区/光标/焦点） |
| `SearchBarState` | 搜索栏状态（文本框 + 触发标记） |
| `WordEntry` | 单词条目（word/phonetic/definition/example） |

### 公共函数

#### 字体设置

| 函数 | 说明 |
|------|------|
| `void UISetFonts(Font chineseFont, Font englishFont)` | 设置 UI 使用的中文和英文字体 |
| `void UISetLatinFont(Font latinFont)` | 设置 IPA 音标字体 |

#### 样式管理

| 函数 | 说明 |
|------|------|
| `void UIStyleInit(UIStyle* style)` | 初始化样式为默认值（浅色主题、默认字体） |
| `void UILoadFont(UIStyle* style, const char* filename)` | 加载自定义字体 |
| `void UIUnloadFont(UIStyle* style)` | 卸载自定义字体 |

#### 主题

| 函数 | 说明 |
|------|------|
| `UITheme UIThemeLight(void)` | 获取浅色主题（白底蓝字） |
| `UITheme UIThemeDark(void)` | 获取深色主题（黑底蓝字） |

#### UI 状态

| 函数 | 说明 |
|------|------|
| `void UIBegin(UIState* state)` | 开始一帧：更新鼠标位置、按键状态、重置悬停 |
| `void UIEnd(UIState* state)` | 结束一帧：鼠标未按下时清除活动状态 |
| `int UIGetID(const char* label)` | 根据标签字符串生成唯一 ID（djb2 hash） |

#### 布局系统

| 函数 | 说明 |
|------|------|
| `UILayout UIBeginLayout(Rectangle container, UIDirection dir, float spacing, float padding)` | 开始布局，指定容器、方向、间距、内边距 |
| `Rectangle UILayoutNext(UILayout* layout, float width, float height)` | 获取下一个元素的位置和大小。width/height 为 -1 时自动填满剩余空间 |

#### 基础控件

| 函数 | 说明 |
|------|------|
| `bool UIButton(const char* label, Rectangle rect, UIStyle* style, UIState* state, int id)` | 普通按钮，返回是否被点击 |
| `bool UIButtonEx(const char* label, Rectangle rect, UIStyle* style, UIState* state, bool enabled, int id)` | 扩展按钮，支持禁用状态 |
| `bool UICheckbox(const char* label, Rectangle rect, bool* checked, UIStyle* style, UIState* state)` | 复选框，点击切换选中状态 |
| `void UILabel(const char* text, Rectangle rect, UIStyle* style, Color color)` | 文本标签，自动换行 |

#### 滚动视图

| 函数 | 说明 |
|------|------|
| `void UIBeginScrollView(UIScrollView* scroll, Rectangle viewport, Vector2 contentSize)` | 开始滚动视图（开启裁剪模式） |
| `void UIEndScrollView(UIScrollView* scroll, UIStyle* style, UIState* state)` | 结束滚动视图（处理滚轮、绘制滚动条） |
| `bool UIListItem(const char* text, Rectangle itemRect, UIStyle* style, UIState* state)` | 可点击的列表项，返回是否被点击 |
| `void UIBeginPersistentScrollView(PersistentScrollView* psv, Rectangle viewport, Vector2 contentSize, float* offsetPtr)` | 开始持久化滚动视图（自动保存偏移） |
| `void UIEndPersistentScrollView(PersistentScrollView* psv, UIStyle* style, UIState* state)` | 结束持久化滚动视图（回写偏移） |

#### 文本输入

| 函数 | 说明 |
|------|------|
| `bool UITextBox(UITextBoxState* state, Rectangle rect, UIStyle* style, UIState* uistate, bool password)` | 文本输入框。支持：UTF-8中文、退格/Delete、左右光标移动、密码模式（显示*号）。返回是否按下回车 |

#### 文本绘制

| 函数 | 说明 |
|------|------|
| `void UIDrawText(const char* text, Vector2 pos, float fontSize, float spacing, Color tint)` | 绘制单行文本（自动选择字体） |
| `void UIDrawTextRec(const char* text, Rectangle rec, float fontSize, float spacing, bool wordWrap, Color tint)` | 绘制文本（矩形裁剪 + 自动换行） |

#### 高级组件

| 函数 | 说明 |
|------|------|
| `void UIWordCard(WordEntry* entry, Rectangle rect, UIStyle* style)` | 绘制单词卡片（单词→音标→分隔线→释义→例句） |
| `int UIFlashCard(WordEntry* entry, Rectangle rect, CardFace* face, UIStyle* style, UIState* state, float* animTime)` | 闪卡组件。支持点击翻转动画、认识/不认识按钮。返回值：0=无操作, 1=认识, 2=不认识, 3=翻转 |
| `int UIMultipleChoice(const char* question, const char* options[], int optionCount, int correctIndex, Rectangle rect, UIStyle* style, UIState* state)` | 选择题组件。绘制题干 + A/B/C/D 选项，返回用户选择的索引 |
| `void UISearchBar(SearchBarState* sb, Rectangle rect, UIStyle* style, UIState* state)` | 搜索栏组件（输入框 + 搜索按钮） |
| `int UIWordListView(WordEntry* words, int count, int* selectedIndex, UIScrollView* scroll, UIStyle* style, UIState* state)` | 可滚动的单词列表视图，返回被点击的单词索引 |

---

## 10. src/ui/menu_callbacks.c/h — 菜单页面回调

### 页面渲染函数（共14个）

| 函数 | 页面 | 说明 |
|------|------|------|
| `MenuHome_Show()` | **主菜单** | 欢迎信息 + 统计卡片 + 三个功能入口（学单词/背单词/查找单词） |
| `MenuLearn_Show()` | **学单词** | 左侧可滚动单词列表 + 右侧详情（单词/音标/释义/例句/学习统计）+ 上下导航 |
| `MenuReviewRoot_Show()` | **背单词（父页）** | 三种模式入口卡片：卡片背单词/选词背单词/测试 |
| `MenuCardReview_Show()` | **卡片背单词** | 闪卡翻转：正面显示单词，背面显示释义，认识3次标记已掌握，显示本轮统计 |
| `MenuSelectWord_Show()` | **选词背单词** | 看释义选英文单词，四选一，按账号统计得分 |
| `MenuTest_Show()` | **测试模式** | 看单词选释义，四选一，完成后显示正确率和评价 |
| `MenuPlanRoot_Show()` | **学习计划（父页）** | 显示当前激活计划信息和进度条，两个子入口 |
| `MenuPlanManager_Show()` | **计划管理** | 左侧可滚动计划列表（点击激活/删除），右侧创建表单 |
| `MenuProgress_Show()` | **学习进度** | 统计卡片（总数/已掌握/学习中）+ 所有单词详细进度列表 |
| `MenuSettings_Show()` | **设置** | 深色模式切换 + 关于信息 |
| `MenuAccount_Show()` | **账号管理** | 已登录显示用户信息/登出，未登录显示登录/注册入口 + 用户列表 |
| `MenuLogin_Show()` | **登录** | 用户名/密码输入，登录验证，切换注册页 |
| `MenuRegister_Show()` | **注册** | 用户名/密码/确认密码输入，注册验证 |
| `MenuSearch_Show()` | **查找单词** | 正则/模糊搜索，滚动显示每个结果的单词/音标/释义/例句 |

### 菜单系统辅助函数

| 函数 | 说明 |
|------|------|
| `void InitMenuTree(void)` | 初始化菜单树，创建所有节点并建立父子关系。根节点为主菜单，包含7个子节点：学单词/背单词/查找/学习计划/设置/账号/词库 |
| `void FreeMenuTree(void)` | 释放菜单树内存 |
| `const char* GetMenuItemText(MENU* menu)` | 根据菜单节点的 show 函数指针获取对应的中文名称 |
| `void DrawTreeMenu(Rectangle menuRect)` | 绘制左侧树形导航菜单。非根节点显示标题+返回按钮，子节点可滚动点击切换 |

### 页面流程图

```
主菜单
├── 学单词
├── 背单词
│   ├── 卡片背单词
│   ├── 选词背单词
│   └── 测试模式
├── 查找单词
├── 学习计划
│   ├── 计划管理
│   └── 学习进度
├── 设置
├── 账号管理（含登录/注册子页）
└── 词库管理
```
