# 代码优化建议

> 本文档基于对整个项目源码的深入审查整理而成，按优先级排列。

---

## 目录

- [🔴 高危 — 建议立即修复](#-高危--建议立即修复)
- [🟡 中危 — 建议下次迭代修复](#-中危--建议下次迭代修复)
- [🟢 低危 — 建议长期优化](#-低危--建议长期优化)
- [💡 架构建议](#-架构建议)

---

## 🔴 高危 — 建议立即修复

### 1. 浅拷贝导致 Use-After-Free 风险

**文件：** `src/modules/words.c:583`

```c
g_words[i].entry = g_wordLibrary[i];  // 浅拷贝指针！
```

`WordEntry` 包含 5 个 `char*` 指针（word / phonetic / definition / example / exampleTranslation）。`g_words[i].entry = g_wordLibrary[i]` 是**结构体赋值**，两个数组的指针指向同一块 `strdup` 分配的内存。

当 `deleteWordFromLibrary()` 释放这些指针后，`g_words` 中的对应指针变成悬空指针（dangling pointer），后续访问会导致 **use-after-free**。

**修复方案：** `reloadWords()` 改为深拷贝，或删除单词后同时清理 `g_words` 中对应的进度条目。

---

### 2. `realloc` 返回值未检查 — OOM 时内存泄漏

**出现位置（3处）：**

| 文件 | 行号 | 代码 |
|------|------|------|
| `src/modules/words.c` | 128 | `g_wordLibrary = realloc(g_wordLibrary, ...)` |
| `src/modules/words.c` | 505 | `g_wordLibrary = realloc(g_wordLibrary, ...)` |
| `src/modules/fonts.c` | 392 | `allGlyphs = realloc(allGlyphs, ...)` |

`realloc` 返回 NULL 时，原始指针丢失，造成内存泄漏。

**修复方案：**
```c
WordEntry* tmp = (WordEntry*)realloc(g_wordLibrary, newSize);
if (tmp == NULL) {
    // 处理错误，g_wordLibrary 仍然有效
    return false;
}
g_wordLibrary = tmp;
```

---

### 3. 文件解析失败时内存泄漏

**文件：** `src/modules/words.c:140-148`

```c
char* fields[5] = {NULL, NULL, NULL, NULL, NULL};
// ... strdup 分配各字段 ...
if (fieldIdx >= 3) {
    // 使用 fields...
} // else: fieldIdx < 3 时，已 strdup 的 fields[0] 等无人释放
```

当一行数据少于 3 个字段时，`strdup` 分配的内存泄漏了。

**修复方案：** 添加 `else` 分支释放已分配的字段。

---

### 4. `void()` 函数指针禁用类型检查

**文件：** `src/core/tree_menu.h:23-24`, `tree_menu.c:20`

```c
void (*show)();          // C89 风格：参数列表未指定
void (*fun)();
```

C 语言中 `void()` 表示"未指定参数"（旧式风格），编译器不会检查参数。应改为：

```c
void (*show)(void);     // 明确声明：无参数
void (*fun)(void);
```

---

## 🟡 中危 — 建议下次迭代修复

### 5. `menu_callbacks.c` 过于庞大（2089 行）

单个文件包含所有 14 个页面的渲染逻辑，违背单一职责原则。

**建议：** 按功能拆分为多个文件：

```
src/ui/
├── menu_learn.c          ← 学单词页面
├── menu_card_review.c    ← 卡片背单词
├── menu_select_word.c    ← 选词背单词
├── menu_test.c           ← 测试模式
├── menu_plan.c           ← 学习计划 + 进度
├── menu_settings.c       ← 设置
├── menu_account.c        ← 账号/登录/注册
├── menu_word_manager.c   ← 词库管理
├── menu_search.c         ← 查找单词
└── menu_callbacks.c      ← 菜单系统辅助函数（InitMenuTree/DrawTreeMenu）
```

---

### 6. 初始化逻辑重复：`main.c` vs `AppState_Init`

**文件：** `src/main.c:64-83`, `src/core/app_state.c:31-66`

`main.c` 手动初始化 `REVIEW.reviewCount`、`TEST.testCount` 等字段，然后马上调用 `AppState_Init()` 又初始化一遍相同的字段。`main.c` 的值覆盖了 `AppState_Init()` 的值。

**建议：** 将初始化逻辑全部移到 `AppState_Init()` 中，`main.c` 只做业务相关的初始化。

---

### 7. 密码哈希不安全（djb2 算法）

**文件：** `src/modules/account.c:48-88`

使用 `djb2` 哈希（哈希表用）存储密码，未加盐（no salt），彩虹表可逆向。

**建议：** 对本地学习软件来说风险可控，代码注释已说明"非加密安全"。如果后续需要更强的安全性，建议使用 SHA-256 加盐。

---

### 8. 硬编码按钮 ID 可能冲突

**文件：** `src/ui/menu_callbacks.c` 中大量使用

```c
UIButton(u8"上一个", ..., 1);    // ID=1
UIButton(u8"下一个", ..., 2);
UIButton(u8"返回",   ..., 6);
UIButton(u8"搜索",   ..., 7);
```

这些 ID 分散在各个函数中，新增页面时容易重复，导致按钮状态错乱。

**建议：** 定义枚举：

```c
typedef enum {
    BTN_PREV = 1,
    BTN_NEXT,
    BTN_RESTART,
    BTN_NEXT_QUESTION,
    BTN_CLEAR_PROGRESS,
    BTN_BACK,
    BTN_SEARCH,
    // ...
} ButtonID;
```

---

### 9. 除零风险

**文件：** `src/ui/menu_callbacks.c:821`

```c
float prog = (float)active->studiedToday / active->dailyWordCount;
```

如果 `dailyWordCount` 为 0（创建计划时可能默认值未设置），得到 `inf`。

**建议：** 加保护：

```c
float prog = active->dailyWordCount > 0
    ? (float)active->studiedToday / active->dailyWordCount
    : 0.0f;
```

---

### 10. `strtok()` 非线程安全

**出现位置：**

| 文件 | 行号 |
|------|------|
| `src/modules/words.c` | 134, 237 |
| `src/modules/account.c` | 131 |
| `src/modules/plan.c` | 132 |

`strtok` 使用静态内部缓冲区，不可重入。虽然目前是单线程应用，但为未来兼容应考虑改用 `strtok_r`（POSIX）或 `strtok_s`（C11 / MSVC）。

---

### 11. 字体字形预加载重复计算

**文件：** `src/modules/fonts.c:348-386`

`loadFonts()` 对 `allChinese` 和 `wordChars` 分别调用两次 `LoadCodepoints()`（一次为中文字体，一次为合并字体），结果完全一样。

**建议：** 只需计算一次，结果复用。

---

### 12. 每帧重复 O(n) 遍历计算统计值

**文件：** `src/ui/menu_callbacks.c`

- `MenuHome_Show()` 第 65 行：遍历所有单词统计已掌握数
- `MenuLearn_Show()` 第 148 行：同样统计
- `MenuProgress_Show()` 第 1034 行：同样统计

每次重绘都 O(n) 扫描。这些页面不共存在同一个帧上，但每次进入页面都重新计算。

**建议：** 在 AppState 中缓存一个 `masteredCount`，在用户点击"认识"/"不认识"后增量更新，避免 O(n) 每帧扫描。

---

### 13. `config.h` 中有大量未使用的宏

| 宏 | 位置 | 状态 |
|----|------|------|
| `COLOR_BLACK` / `WHITE` / `RED` 等 | config.h:24-33 | **未使用**（实际颜色来自 raylib_word_ui.c） |
| `THEME_PRIMARY` / `SUCCESS` 等 | config.h:36-40 | **未使用** |
| `OPACITY_DIM` / `NORMAL` / `BRIGHT` | config.h:43-45 | **未使用** |
| `TEST_TIME_LIMIT` | config.h:92 | **未使用（保留供未来开发）** |

**建议：** 删除或标记为 future use。

---

### 14. 全局变量 `g_learnScrollOffset` 已死代码

**文件：** `src/modules/words.c:41-42`

```c
float g_learnScrollOffset = 0;  // 当前未使用，由 LearnState 管理
```

注释已说明未使用，应删除。

---

### 15. `AppState_Reset()` 未在任何地方调用

**文件：** `src/core/app_state.c:75-94`

定义了完整的 Reset 函数但从未被调用，是死代码。

**建议：** 如果以后计划添加"重新开始学习"功能可保留，否则删除。

---

### 16. `src/` 根目录存在空文件

```
src/account.h       (0 字节)  ← 空的，实际在 src/modules/account.h
src/plan.c          (0 字节)  ← 空的
src/plan.h          (0 字节)  ← 空的
src/words.c         (0 字节)  ← 空的
src/fonts.c         (0 字节)  ← 空的
```

这些是文件重组时残留的，应删除。

---

## 🟢 低危 — 建议长期优化

### 17. 命名风格不统一

混用三种风格：
- **PascalCase：** `AppState_Init`, `MenuHome_Show`, `UIButton`
- **snake_case：** `loadWordsFromFile`, `shuffleArray`
- **匈牙利前缀：** `g_app`, `g_wordCount`（`g_` 表示全局）
- **混合：** `g_pPlanState`, `g_wmState`

**建议：** 选一种风格全局统一。公共 API 用 `PascalCase`，内部静态函数用 `snake_case`。

---

### 18. 变量命名过于简略

**文件：** `src/ui/menu_callbacks.c`

- `cr` → `contentRect`
- `sv` → `scrollView`
- `bl` → `buttonLayout`
- `ft` → `formTitle`
- `l1` / `i1` / `l2` / `i2` → `labelRect` / `inputRect`

**建议：** 避免单字母/两字母缩写，除非是局部循环变量。

---

### 19. `main.c` 未使用 `config.h` 中的 `WINDOW_TITLE`

```c
// config.h 第 16 行已定义：
#define WINDOW_TITLE  u8"背单词软件"

// 但 main.c 第 41 行硬编码：
InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, u8"背单词软件");
```

**建议：** 改为 `InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);`

---

### 20. `localtime()` 非线程安全

**出现位置：**
- `src/modules/fonts.c:549`
- `src/ui/menu_callbacks.c:1444, 1541, 1553`
- `src/modules/plan.c:103`

**建议：** 使用 `localtime_r()`（POSIX）或 `localtime_s()`（MSVC）替换。

---

### 21. `atoi`/`atol` 无法检测解析错误

**出现位置：** `src/modules/account.c:141-144`

```c
u->createdTime = (time_t)atol(fields[2]);
```

解析失败时返回 0，无法区分"值是 0"和"解析失败"。

**建议：** 使用 `strtol()/strtoll()` 配合 `errno` 检查。

---

### 22. `hash_string` 使用 `int c` 可能导致 UB

**文件：** `src/modules/account.c:51`

```c
int c;
while ((c = *str++)) {
```

当 `char` 为有符号类型时，高 ASCII 字符（≥ 0x80）转为负数，可能影响哈希结果。

**建议：** `unsigned char c;`

---

### 23. 搜索逻辑重复

**文件：** `src/ui/menu_callbacks.c:1817-1836`

搜索按钮和自动搜索的触发逻辑几乎完全一样，重复约 10 行代码。

**建议：** 提取为公共函数：

```c
static void triggerSearch(void) {
    const char* query = SEARCH.searchBar.textState.buffer;
    SEARCH.searchResultCount = searchWordsByRegex(query, ...);
    if (SEARCH.searchResultCount == 0) {
        SEARCH.searchResultCount = searchWordsSimple(query, ...);
    }
}
```

---

### 24. 登录表单每帧双拷贝

**文件：** `src/ui/menu_callbacks.c:1605-1607`

```c
strncpy(g_loginForm.userState.buffer, g_loginForm.username, ...);  // 拷入
UITextBox(&g_loginForm.userState, ...);
strncpy(g_loginForm.username, g_loginForm.userState.buffer, ...);  // 拷出
```

每帧将 username 从 struct 拷到 textbox buffer，再从 buffer 拷回 struct。

**建议：** 直接用 `UITextBoxState.buffer` 作为唯一数据源，不需要 struct 中的 `username`/`password` 字段。

---

### 25. `build.sh` 中的 Linux 特定命令

**文件：** `build.sh:45,81,85`

```bash
ldconfig -p    # Linux 专用
nproc          # Linux 专用（macOS 用 sysctl -n hw.logicalcpu）
chmod +x       # Linux/Unix 可用
ldd            # Linux/Unix 可用
```

**建议：** 添加 macOS 兼容判断。

---

### 26. `UIWordListView` 与学单词模式的列表高度不一致

- `UIWordListView()` 使用 `itemHeight = 50.0f`
- `MenuLearn_Show()` 使用 `60.0f`

导致视觉上不一致。

**建议：** 统一定义一个列表项高度的常量。

---

### 27. 硬编码坐标过多

几乎所有页面使用硬编码像素坐标（如 `Rectangle cardRect = {SCREEN_WIDTH/2 - 220, 160, 440, 264}`），窗口大小改变时布局错乱。

**建议：** 使用相对布局计算，定义侧边栏宽度、顶部栏高度、卡片比例等常量到 `config.h`。

---

### 28. `saveProgress()` 返回值被忽略

**文件：** `src/ui/menu_callbacks.c:375,392`

```c
saveProgress();  // 返回值是 void，但实际写入可能失败
```

用户学习进度可能在静默中丢失。

**建议：** `saveProgress()` 改为 `bool` 返回值，调用处检查。

---

## 💡 架构建议

### 建议一：页面拆分

将 `menu_callbacks.c` 按页面拆分成独立文件，每个文件对应一个页面：

```
src/ui/pages/
├── page_home.c
├── page_learn.c
├── page_card_review.c
├── page_select_word.c
├── page_test.c
├── page_search.c
├── page_plan_root.c
├── page_plan_manager.c
├── page_progress.c
├── page_settings.c
├── page_account.c
├── page_login.c
├── page_register.c
├── page_word_manager.c
└── pages.h            ← 统一导出所有页面函数
```

### 建议二：配置文件统一

将 `config.h`、`account.h` 中的常量（`MAX_USERS`、`MAX_WORDS`、`ACCOUNT_FILE` 等）统一到一个配置文件中，避免分散定义。

### 建议三：测试框架

当前项目没有任何单元测试。建议为以下核心模块添加测试：
- `words.c` — 单词加载、搜索、进度管理
- `account.c` — 注册/登录逻辑
- `plan.c` — 计划创建/日期检测

可以使用简单的 C 测试框架（如 [Unity](https://github.com/ThrowTheSwitch/Unity) 或 [CTest](https://cmake.org/cmake/help/latest/manual/ctest.1.html)）。

### 建议四：错误处理统一

目前错误处理方式不一致：
- 有些地方返回 `bool`
- 有些地方返回 `void` 并 `printf`
- 有些地方什么都不做

建议统一错误处理策略：核心函数返回错误码，上层决定如何处理。

---

> 文档版本：v1.0 — 基于代码提交 c57e9e0 的审查
