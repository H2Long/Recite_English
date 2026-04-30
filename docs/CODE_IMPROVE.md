# 代码优化建议

> 本文档基于对整个项目源码的全面审查整理而成。
> **已按修改难度从易到难排序**，方便你人工逐步修复。

---

## 目录

- [🟢 非常简单（5分钟内可改完）](#-非常简单5分钟内可改完)
- [🟡 简单（10~15分钟）](#-简单1015分钟)
- [🟠 中等（30分钟左右）](#-中等30分钟左右)
- [🔴 困难（1小时以上）](#-困难1小时以上)
- [💡 架构建议（长期规划）](#-架构建议长期规划)

---

## 🟢 非常简单（5分钟内可改完）

### 1. 删除 `g_learnScrollOffset` 死代码

**文件：** `src/modules/words.c` 第 41~42 行

```c
float g_learnScrollOffset = 0;  // 当前未使用，由 LearnState 管理
```

注释已经说明了它没被使用。直接删除这两行。

---

### 2. 删除 `AppState_Reset()` 死代码（或确认保留）

**文件：** `src/core/app_state.c` 第 75~94 行

`AppState_Reset()` 定义了完整的重置逻辑，但在整个项目中**没有任何地方调用它**。

**选择方案：**
- 如果以后会用到 → 保留不动
- 如果不会用到 → 删除函数和它在 `app_state.h` 中的声明

---

### 3. 删除 `config.h` 中未使用的宏

**文件：** `src/core/config.h`

以下宏定义了但从未在任何 `.c` 文件中使用：

```c
// 第 24~33 行 — 基础颜色（实际颜色来自 raylib_word_ui.c 的 Theme 结构体）
#define COLOR_BLACK        ...
#define COLOR_WHITE        ...
#define COLOR_RED          ...

// 第 36~40 行 — 主题颜色（未被引用）
#define THEME_PRIMARY      ...
#define THEME_SUCCESS      ...

// 第 43~45 行 — 透明度预设
#define OPACITY_DIM        0.2f
#define OPACITY_NORMAL     0.5f
#define OPACITY_BRIGHT     0.8f

// 第 92 行 — 测试时间限制（保留供未来用）
#define TEST_TIME_LIMIT     0
```

**操作：** 直接删除除 `TEST_TIME_LIMIT`（可能以后会用到）外的所有未使用宏。

---

### 4. 修复 `hash_string` 中 `int c` → `unsigned char c`

**文件：** `src/modules/account.c` 第 51 行

```c
int c;
while ((c = *str++)) {
```

当 `char` 为有符号类型时，高 ASCII 字符（≥ 0x80）会变成负数，影响哈希结果。

**修复：** 改为 `unsigned char c;`

---

### 5. 修复 `WINDOW_TITLE` 宏使用（已修复）

**文件：** `src/main.c` 第 40 行

`config.h` 中定义了 `#define WINDOW_TITLE u8"背单词软件"`，但 `main.c` 之前是硬编码的字符串。

✅ **这个已经改好了，不用动。**

---

### 6. 删除 `src/` 下残留的空文件

**文件：**
- ~~`src/account.h`~~ ✅ 已删除
- ~~`src/config.h`~~ ✅ 已删除
- ~~`src/fonts.c`~~ ✅ 已删除
- ~~`src/plan.c`~~ ✅ 已删除
- ~~`src/plan.h`~~ ✅ 已删除
- ~~`src/words.c`~~ ✅ 已删除

✅ **这些已经删除了，不用再管。**

---

### 7. 给新拆分的页面文件补充头注释

**文件：** `src/ui/pages/` 下的 11 个新文件

刚拆分的页面文件缺少详细的函数级注释。每个文件已经有一个文件头注释：
```c
// ============================================================================
// 主菜单页面
// 包含欢迎信息、统计卡片和三个功能入口
// ============================================================================
```

建议为每个页面函数加上 Doxygen 风格的入参/返回值注释。

---

### 8. 修复 `scene_` 文件名简洁性（可选）

当前页面文件名为 `page_home.c`、`page_learn.c` 等，命名清晰，无需修改。

---

## 🟡 简单（10~15分钟）

### 9. 将硬编码按钮 ID 提取为枚举

**文件：** `src/ui/pages/` 中多处出现

目前按钮 ID 是散落的魔法数字：
```c
UIButton(u8"上一个", ..., 1);
UIButton(u8"下一个", ..., 2);
UIButton(u8"返回",   ..., 6);
UIButton(u8"搜索",   ..., 7);
```

这些 ID 如果重复会导致按钮状态错乱。

**修复方案：** 在 `pages.h` 中添加枚举：

```c
// pages.h 末尾添加
typedef enum {
    BTN_PREV = 1,
    BTN_NEXT,
    BTN_RESTART_TEST,
    BTN_NEXT_QUESTION,
    BTN_CLEAR_PROGRESS,
    BTN_BACK,
    BTN_SEARCH,
    BTN_RESTART_SELECT,
    BTN_NEXT_CHOICE,
    BTN_RESTART,
    // 页面级别的按钮 100+...
    BTN_HOME_START = 100,
    BTN_REVIEW_MODE = 500,
    BTN_WORD_MANAGER = 600,
    BTN_PLAN = 700,
    BTN_LOGIN = 800,
} ButtonID;
```

然后全局搜索替换所有魔法数字。由于按钮 ID 不能重复，这个方法也更容易发现冲突。

---

### 10. 从 `MenuHome_Show` 提取搜索触发函数（消除重复代码）

**文件：** `src/ui/pages/page_search.c` 第 30~37 行

搜索按钮和自动搜索的代码几乎完全一样：

```c
// 按钮搜索（第 30~37 行）
if (UIButton(u8"搜索", btnRect, STYLE, UI_STATE, 7)) {
    const char* query = SEARCH.searchBar.textState.buffer;
    if (strlen(query) > 0) {
        SEARCH.searchResultCount = searchWordsByRegex(query, ...);
        if (SEARCH.searchResultCount == 0)
            SEARCH.searchResultCount = searchWordsSimple(query, ...);
    }
}
// 自动搜索（第 38~45 行）-- 几乎一样的逻辑
int currentLen = strlen(SEARCH.searchBar.textState.buffer);
if (currentLen > 0 && currentLen != lastSearchLen) {
    const char* query = SEARCH.searchBar.textState.buffer;
    SEARCH.searchResultCount = searchWordsByRegex(query, ...);
    if (SEARCH.searchResultCount == 0)
        SEARCH.searchResultCount = searchWordsSimple(query, ...);
}
```

**修复方案：** 提取为一个 `static void doSearch(void)` 函数：

```c
static void doSearch(void) {
    const char* query = SEARCH.searchBar.textState.buffer;
    if (strlen(query) == 0) return;
    SEARCH.searchResultCount = searchWordsByRegex(query, ...);
    if (SEARCH.searchResultCount == 0)
        SEARCH.searchResultCount = searchWordsSimple(query, ...);
}
```

然后在按钮回调和自动搜索处都调用 `doSearch()`。

---

### 11. 统一列表项高度

**当前不一致情况：**
- `UIWordListView()` 使用 `itemHeight = 50.0f`（`raylib_word_ui.c` 第 1114 行）
- `MenuLearn_Show()` 使用 `60.0f`（`page_learn.c` 第 33 行）
- `MenuWordManager_Show()` 使用 `40.0f`（`page_word_manager.c` 第 23 行）

这导致不同页面的列表视觉上高度不一致。

**修复方案：** 在 `config.h` 中添加：
```c
#define LIST_ITEM_HEIGHT   50    // 列表项标准高度
```

然后所有页面使用这个宏。

---

### 12. 从 `MenuHome_Show` 移除每帧 O(n) 的已掌握统计

**文件：** `src/ui/pages/page_home.c` 第 29~32 行

每次重绘都遍历全部 100 个单词统计已掌握数。在 `page_learn.c` 和 `page_plan.c` 中也有同样的遍历。

**快速修复方案（5分钟）：** 不需要大的架构改动，保持现状即可。因为单词数上限 100，O(n) 遍历的性能损耗可忽略不计。

---

## 🟠 中等（30分钟左右）

### 13. 修复 `strtok()` 非线程安全

**出现位置（3个文件）：**

| 文件 | 行号 |
|------|------|
| `src/modules/words.c` | 第 134, 237 行 |
| `src/modules/account.c` | 第 131 行 |
| `src/modules/plan.c` | 第 132 行 |

**修复方案：** 使用条件编译选择线程安全的版本：

```c
// 在 words.c 顶部添加
#ifdef _MSC_VER
    #define STRTOK strtok_s
#else
    #define STRTOK strtok_r
#endif

// 使用
char* token = STRTOK(line, "|", &saveptr);
```

完整的修改涉及在 3 个文件中添加 `saveptr` 声明和条件编译宏，大约需要 20 分钟。

---

### 14. 修复 `localtime()` 非线程安全

**出现位置：**

| 文件 | 行号 | 用途 |
|------|------|------|
| `src/modules/fonts.c` | 第 549 行 | 格式化时间戳 |
| `src/modules/plan.c` | 第 103 行 | 获取当天日期 |
| `src/ui/pages/page_account.c` | 第 69, 75, 77 行 | 显示用户注册/登录时间 |

**修复方案：** 使用条件编译：

```c
// 替换
struct tm *tm_info = localtime(&timestamp);

// 为
struct tm tm_buf;
#ifdef _MSC_VER
    localtime_s(&tm_buf, &timestamp);
#else
    localtime_r(&timestamp, &tm_buf);
#endif
```

---

### 15. 修复函数指针 `void()` → `void(void)`

**文件：** `src/core/tree_menu.h` 第 23~24 行，`tree_menu.c` 第 20 行

```c
void (*show)();    // 应改为 void (*show)(void);
void (*fun)();
```

C 语言中空括号表示"未指定参数"（旧式风格），编译器不会做类型检查。

**操作：** 在 3 个位置加上 `void`：
1. `tree_menu.h:23` — `void (*show)(void);`
2. `tree_menu.h:24` — `void(*fun)(void);`
3. `tree_menu.c:20` — `MENU* CreatMenuTreeNode(void(*fun)(void), void (*show)(void))`

---

### 16. 防止除零：`dailyWordCount` 可能为 0

**文件：** `src/ui/pages/page_plan.c` 第 39 行

```c
float prog = (float)active->studiedToday / active->dailyWordCount;
```

如果 `dailyWordCount` 为 0，结果是 `inf`。

**修复：**
```c
float prog = active->dailyWordCount > 0
    ? (float)active->studiedToday / active->dailyWordCount
    : 0.0f;
```

---

## 🔴 困难（1小时以上）

### 17. `realloc` 返回值未检查（3处）

**文件：**
- `src/modules/words.c` 第 128, 505 行
- `src/modules/fonts.c` 第 392 行

```c
g_wordLibrary = realloc(g_wordLibrary, newSize);
```

如果 `realloc` 返回 NULL（内存不足），原始指针丢失，造成泄漏。

**修复方案（比较复杂）：** 需要用临时变量保存返回值：

```c
WordEntry* tmp = realloc(g_wordLibrary, newSize);
if (tmp == NULL) {
    // 处理错误，g_wordLibrary 仍然有效
    printf("ERROR: 内存不足\n");
    return false;
}
g_wordLibrary = tmp;
```

涉及修改两个文件中的 3 处代码，约需 30 分钟，还需要考虑出错时的回退逻辑。

---

### 18. 浅拷贝导致 Use-After-Free 风险

**文件：** `src/modules/words.c` 第 583 行

```c
g_words[i].entry = g_wordLibrary[i];  // 浅拷贝指针
```

`WordEntry` 包含 `char*` 指针。两个数组共享同一块 `strdup` 分配的内存。当 `deleteWordFromLibrary()` 释放这些指针后，`g_words` 中的指针变成悬空指针。

**修复方案（比较复杂）：** 改为深拷贝，或确保删除后立即同步。

深拷贝实现：
```c
// reloadWords() 中
g_words[i].entry.word = strdup(g_wordLibrary[i].word);
g_words[i].entry.phonetic = strdup(g_wordLibrary[i].phonetic);
// ... 其他字段
```

配套还需要在 `AppState_Deinit()` 或适当的地方释放深拷贝的内存。

---

### 19. 文件解析失败时内存泄漏

**文件：** `src/modules/words.c` 第 140~148 行

```c
if (fieldIdx >= 3) {
    // 使用 fields...
} // 没有 else 分支：如果 fieldIdx < 3，strdup 的 fields[0~2] 泄漏了
```

**修复方案：** 添加 else 分支释放已分配的字段。

---

### 20. 初始化逻辑重复：`main.c` vs `AppState_Init()`

`main.c` 手动初始化 REVIEW、TEST 等状态 → 立即调用 `AppState_Init()` 又初始化一遍 → `main.c` 的值覆盖后者。虽然最终结果正确，但逻辑混乱。

**修复方案：** 将 REVIEW 和 TEST 的初始化统一移到 `AppState_Init()` 中，`main.c` 只保留业务逻辑（如从文件加载数据）。涉及较大改动。

---

### 21. 字体字形预加载双重计算

**文件：** `src/modules/fonts.c`

对 `allChinese` 和 `wordChars` 分别调用两次 `LoadCodepoints()`（一次为中文字体，一次为合并字体），结果完全一样。

**修复方案：** 将第一次 `LoadCodepoints` 的结果保存下来复用。涉及 `loadFonts()` 函数的结构调整。

---

### 22. 密码哈希安全性（djb2 算法）

**文件：** `src/modules/account.c` 第 48~88 行

使用 djb2 哈希（哈希表用的非加密算法），未加盐（no salt），彩虹表可逆向。

**修复方案：** 对于本地学习软件风险可控（代码注释已说明"非加密安全"）。如果要改进，需要：
1. 引入 SHA-256 库
2. 加盐机制
3. 重新设计账号文件格式（需兼容旧数据迁移）

**建议：** 保持现状，文档已声明仅适用于本地环境。

---

## 💡 架构建议（长期规划）

### 23. 统一信息源：登录表单双拷贝

**文件：** `src/ui/pages/page_account.c`

每帧将 username 从 struct 拷到 textbox buffer，再从 buffer 拷回 struct，重复劳动。

**改进方案：** 直接用 `UITextBoxState.buffer` 作为唯一数据源，删除 struct 中冗余的 `username`/`password` 字段。需要改动 MenuLogin_Show 和 MenuRegister_Show 两个函数。

### 24. `config.h` 中的硬编码颜色未使用

✅ 已列入 #3。

### 25. 命名风格统一

当前混用 `AppState_Init`（PascalCase）、`loadWordsFromFile`（snake_case）、`g_` 前缀（匈牙利）。属于长期规范，建议在大型重构时统一清理。

### 26. 单元测试

目前没有任何测试。建议后期使用 [Unity](https://github.com/ThrowTheSwitch/Unity) 测试框架为核心模块（account、words、plan）添加测试。

---

> 文档版本：v2.0 — 已按修复难度重新排序
