# BACKSPACE 退格键支持功能

## 一、需求描述

用户在搜索框中输入单词后，需要使用 BACKSPACE 键删除已输入的字符。

## 二、涉及的源码文件

| 文件 | 作用 |
|------|------|
| `raylib_word_ui.c` | 文本输入框组件 `UITextBox()`、退格删除函数 `utf8_delete_left()` |
| `raylib_word_ui.h` | `UITextBoxState` 结构体定义 |
| `menu_callbacks.c` | 搜索页面的 `MenuSearch_Show()` 调用 `UISearchBar()` |

## 三、修改过程与逐步分析

### 第 1 步：确认代码中已有 BACKSPACE 处理逻辑

阅读 `raylib_word_ui.c:691` 的 `UITextBox` 函数，发现原本就有退格键处理：

```c
if (key == KEY_BACKSPACE) {
    utf8_delete_left(state->buffer, &state->cursor);
}
```

其中 `key = uistate->keyPressed = GetKeyPressed()`。

### 第 2 步：确认焦点（Focus）是否正确设置

输入框需要"获得焦点"才能接收键盘输入。焦点通过**鼠标点击**来设置：

```c
bool UITextBox(UITextBoxState* state, Rectangle rect, ...) {
    int id = UIGetID(state->buffer);   // 原始代码：用 buffer 内容生成 ID
    bool isInside = CheckCollisionPointRec(uistate->mousePos, rect);

    if (uistate->mousePressed) {
        uistate->focusItem = isInside ? id : 0;
        state->hasFocus = (uistate->focusItem == id);
    }
}
```

#### 潜在 Bug：空 buffer 时 ID 为 0

`UIGetID()` 根据字符串内容计算哈希值：

```c
int UIGetID(const char* label) {
    int id = 0;
    while (*label) id = id * 31 + (*label++);
    return id;
}
```

当 `buffer` 为空时 `label[0] == '\0'`，`while` 循环不执行，`id` 为 **0**。而 `focusItem` 初始值也是 **0**。

所以空 buffer 时 `state->hasFocus = (0 == 0) = true` — **无意中"永久获得焦点"**，这不是正确的做法。

#### 修复：改用 state 指针生成唯一 ID

```c
int id = (int)(unsigned long)(void*)state;   // 用 state 地址，保证唯一且不变
```

### 第 3 步：添加视觉调试确认焦点状态

为了确认焦点是否真的被设置，在绘制时根据 `isInside` 和 `hasFocus` 改变背景颜色：

```c
Color bgColor = style->theme.inputBg;
if (isInside) bgColor = RED;       // 鼠标悬停→红色
if (hasFocus) bgColor = BLUE;      // 获得焦点→蓝色
```

**测试结论**：点击输入框后变蓝色 → 焦点设置成功。

### 第 4 步：添加视觉调试确认退格键检测

在键盘处理分支中添加标记变量 `backspaceHit`，当退格键被检测到时在屏幕左上角画一个绿色方块：

```c
bool backspaceHit = false;

if (key == KEY_BACKSPACE || IsKeyPressed(KEY_BACKSPACE)) {
    utf8_delete_left(state->buffer, &state->cursor);
    backspaceHit = true;
}

// 绘制阶段
if (backspaceHit) DrawRectangle(10, 10, 50, 50, GREEN);
```

**测试结论**：按退格键后左上角出现绿色方块 → 退格键检测成功。

### 第 5 步：发现真正的 Bug — `utf8_delete_left` 边界条件错误

```c
// 原始代码    (x)
while (prev_byte < byte_offset) {
    int len = utf8_char_len((unsigned char)buffer[prev_byte]);
    if (prev_byte + len > byte_offset) break;   // ← Bug 在这里！
    prev_byte += len;
    prev_index++;
}
memmove(buffer + prev_byte, buffer + byte_offset, ...);
```

#### 逐行追踪

假设 buffer = `"abc"`，cursor = 3（末尾），全部为 ASCII（每个字符 1 字节）：

| 迭代 | `prev_byte` | `buffer[prev_byte]` | `len` | `prev_byte+len > byte_offset(3)` | 执行动作 |
|------|-------------|---------------------|-------|----------------------------------|---------|
| 1 | 0 | 'a' = 97 | 1 | `1 > 3` ? ❌ | `prev_byte=1, prev_index=1` |
| 2 | 1 | 'b' = 98 | 1 | `2 > 3` ? ❌ | `prev_byte=2, prev_index=2` |
| 3 | 2 | 'c' = 99 | 1 | `3 > 3` ? ❌ | `prev_byte=3, prev_index=3` |
| 退出 | `3 < 3` = false | | | | |

**最终 `prev_byte = 3`，与 `byte_offset = 3` 完全相等。**

```c
memmove(buffer + 3, buffer + 3, strlen(buffer + 3) + 1);
// 从自己拷贝到自己 → 没有任何变化！
```

#### Bug 原因

条件 `prev_byte + len > byte_offset` 只有在 **超过** 边界时才停止，但光标恰好在字符末尾时 `prev_byte + len == byte_offset`，条件不成立，导致循环多走了一步，让 `prev_byte` 追上了 `byte_offset`。

#### 修复方案

将 `>` 改为 `>=`，当 `prev_byte + len` 等于 `byte_offset` 时立即停止：

```c
// 修复后    (✓)
while (prev_byte < byte_offset) {
    int len = utf8_char_len((unsigned char)buffer[prev_byte]);
    if (prev_byte + len >= byte_offset) break;   // ← >=
    prev_byte += len;
    prev_index++;
}
```

#### 修复后追踪

| 迭代 | `prev_byte` | `len` | `prev_byte+len >= 3` ? | 执行动作 |
|------|-------------|-------|------------------------|---------|
| 1 | 0 | 1 | `1 >= 3` ? ❌ | `prev_byte=1, prev_index=1` |
| 2 | 1 | 1 | `2 >= 3` ? ❌ | `prev_byte=2, prev_index=2` |
| 3 | 2 | 1 | `3 >= 3` ? ✅ **break** | 停止 |

最终 `prev_byte = 2, prev_index = 2`，找到字符 `'c'` 的起始位置。

```c
memmove(buffer + 2, buffer + 3, strlen(buffer + 3) + 1);  // 正确删除 'c'
// buffer 变为 "ab\0"
```

## 四、完整的 `UITextBox` 键盘处理逻辑

```c
if (state->hasFocus) {
    int key = uistate->keyPressed;

    // 退格键 — 使用 GetKeyPressed() + IsKeyPressed() 双重检测
    if (key == KEY_BACKSPACE || IsKeyPressed(KEY_BACKSPACE)) {
        utf8_delete_left(state->buffer, &state->cursor);
    }

    // 其他键
    if (key == KEY_DELETE) {
        utf8_delete_right(state->buffer, &state->cursor);
    } else if (key == KEY_LEFT) {
        if (state->cursor > 0) state->cursor--;
    } else if (key == KEY_RIGHT) {
        int total = utf8_strlen(state->buffer);
        if (state->cursor < total) state->cursor++;
    } else if (uistate->charPressed >= 32) {
        utf8_insert_codepoint(state->buffer, sizeof(state->buffer),
                              &state->cursor, uistate->charPressed);
    }
}
```

## 六、后续修复经验

> 以下是基于 BACKSPACE 功能之后，在项目其他模块中遇到的类似 Bug 和修复经验，供学习参考。

---

### 经验 6：光标位置偏移 — 混合字体测量

**场景**：搜索框输入中文或混合文本时，光标（闪烁的竖线）位置与字符的实际位置不匹配，每次移动的距离不准确。

**Bug 代码**（`raylib_word_ui.c` 光标绘制部分）：

```c
Vector2 textSizeBefore = MeasureTextEx(style->font, before, style->fontSizeNormal, 1);
float cursorX = rect.x + 8 + textSizeBefore.x;
```

**问题分析**：`MeasureTextEx(style->font, ...)` 使用**单一字体**测量文本宽度。但应用中支持混合字体（中文字体 `g_chFont`、英文字体 `g_enFont`、IPA 音标字体 `g_latinFont`），每种字体的字符宽度不同。

例如字符串 `"hello世界"`：
- `"hello"` 用英文字体测量 → 宽 60px
- `"世界"` 用中文字体测量 → 每个字宽 24px × 2 = 48px
- 实际总宽 = 108px
- 但用单一字体测量 → 所有字符用同一种字体 → 120px（中文字符被当作英文测量）

所以光标位置偏差了 12px，并且每个字符误差累积，越往后偏差越大。

**修复**：

```c
Vector2 textSizeBefore = MeasureTextAuto(before, style->fontSizeNormal, 1);
```

`MeasureTextAuto()` 在 `fonts.c` 中实现，它会逐字符判断是中/英/IPA 并选择对应字体测量：

```c
Vector2 MeasureTextAuto(const char* text, float fontSize, float spacing) {
    Vector2 result = {0, fontSize};
    while (*text) {
        unsigned char uc = *(unsigned char*)text;
        Font currentFont = getFontForChar(text);
        int charLen = getUtf8Len(uc);
        char seg[5] = {0};
        for (int i = 0; i < charLen; i++) seg[i] = *text++;
        result.x += MeasureTextEx(currentFont, seg, fontSize, spacing).x;
    }
    return result;
}
```

**经验**：涉及多字体渲染的应用，文本宽度测量必须使用与绘制相同的字体选择逻辑。单一字体测量混合文本必然产生误差。

---

### 经验 7：多输入框焦点清除 — focusItem 被后续组件覆盖

**场景**：词库管理页面有 5 个输入框（单词、音标、释义、例句、例句翻译），点击输入框 A 后再点击输入框 B，A 的焦点没有被清除，导致多个输入框同时显示焦点状态（高亮边框）。

**Bug 代码**（`raylib_word_ui.c` 焦点处理）：

```c
if (uistate->mousePressed) {
    if (isInside) {
        uistate->focusItem = id;    // 设置焦点
        state->hasFocus = true;
    } else {
        // 点击其他地方，清除焦点
        if (uistate->focusItem == id) {    // ← 依赖 focusItem 比较
            uistate->focusItem = 0;
            state->hasFocus = false;
        }
    }
}
```

**逐帧追踪**：词库管理页面的 5 个输入框依次调用 `UITextBox()`，每帧按 field[0]→field[1]→...→field[4] 顺序执行。

假设用户先点击 field[3]（例句），再点击 field[2]（释义）：
```
帧 N:  用户点击 field[3]
  field[0]: mousePressed=false，无操作
  field[1]: mousePressed=false，无操作
  field[2]: mousePressed=false，无操作
  field[3]: mousePressed=true, isInside=true
            → focusItem = field3_id,  hasFocus[3] = true
  field[4]: mousePressed=false，无操作

帧 N+1: 用户点击 field[2]
  field[0]: mousePressed=true, isInside=false
            focusItem(现在是 field3_id) == field0_id? → false → 不操作
  field[1]: 同上 → false → 不操作
  field[2]: mousePressed=true, isInside=true
            → focusItem = field2_id,  hasFocus[2] = true  ← ✅
  field[3]: mousePressed=true, isInside=false
            focusItem(现在是 field2_id) == field3_id? → false → 不操作 ← ❌
            hasFocus[3] 仍然是 true!
  field[4]: 同上 → field[4] 焦点不变
```

**结果**：field[2]、field[3]、field[4] 全部 `hasFocus = true`，三个输入框同时显示高亮边框。

**根因**：`uistate->focusItem` 是**共享状态**，被后面的输入框覆盖后，前面的输入框无法通过比较来清除焦点。

**修复**：直接用 `state->hasFocus` 判断，不依赖 `focusItem`：

```c
if (uistate->mousePressed) {
    if (isInside) {
        uistate->focusItem = id;
        state->hasFocus = true;
        state->cursor = utf8_strlen(state->buffer);
    } else if (state->hasFocus) {
        // 如果当前输入框有焦点但鼠标没点中它，直接清除
        state->hasFocus = false;
    }
}
```

**经验**：共享状态（`focusItem`）在依次调用多个组件时会被后续组件覆盖，导致前面的组件无法通过比较来检测状态变化。此时应使用每个组件自身的私有状态（`state->hasFocus`）来判断。

---

### 经验 8：滚动位置不持久 — 局部变量每帧重置

**场景**：词库管理页面的左侧单词列表和主侧边导航栏，用鼠标滚轮滚动后，下一帧又跳回顶部。

**Bug 代码**：

```c
// 词库管理页面左侧单词列表
UIScrollView sv = {0};           // 每帧初始化为 0
sv.viewport = listR;
sv.contentSize = ...;
UIBeginScrollView(&sv, listR, sv.contentSize);
// ... 绘制列表项 ...
UIEndScrollView(&sv, STYLE, UI_STATE);
// sv.scrollOffset.y 在这里被滚轮更新了，但 sv 是局部变量 → 下一帧丢弃
```

```c
// 侧边导航栏
UIScrollView sv = {0};           // 同样的问题
sv.viewport = itemsRect;
sv.contentSize = ...;
UIBeginScrollView(&sv, itemsRect, sv.contentSize);
// ...
UIEndScrollView(&sv, STYLE, UI_STATE);
```

**问题分析**：

`UIEndScrollView()` 内部通过 `GetMouseWheelMove()` 更新 `scroll->scrollOffset.y`，但这个修改只作用于传入的指针所指向的结构体。

由于 `sv` 是函数内的**局部变量**，每帧重新创建为 `{0}`，上一帧的滚动偏移在函数返回时就丢失了。

**与学单词模式的对比**（正确做法）：

```c
// 学单词模式 — 用 AppState 持久化滚动偏移
UIScrollView sv = {0};
sv.scrollOffset.y = LEARN.learnScrollOffset;      // ← 从持久状态读取
sv.contentSize = ...;
UIBeginScrollView(&sv, ...);
// ... 绘制 ...
UIEndScrollView(&sv, STYLE, UI_STATE);
LEARN.learnScrollOffset = sv.scrollOffset.y;       // ← 写回持久状态
```

**修复**：使用 `static` 变量持久化：

```c
// 词库管理页面
static float g_wmListScroll = 0.0f;
UIScrollView sv = {0};
sv.scrollOffset.y = g_wmListScroll;               // 读取
// ...
UIEndScrollView(&sv, STYLE, UI_STATE);
g_wmListScroll = sv.scrollOffset.y;               // 保存
```

```c
// 侧边导航栏
static float g_menuScrollOffset = 0.0f;
UIScrollView sv = {0};
sv.scrollOffset.y = g_menuScrollOffset;           // 读取
// ...
UIEndScrollView(&sv, STYLE, UI_STATE);
g_menuScrollOffset = sv.scrollOffset.y;           // 保存
```

**关于 `static` 变量**：
- **函数内 `static`**：作用域在函数内，生命周期为整个程序运行期
- **文件级 `static`**：作用域在当前 `.c` 文件，其他文件不可见
- **全局变量**（不加 `static`）：所有文件可见（用 `extern` 声明）

`static` 变量存储在**数据段**（.data/.bss），而不是栈上，所以函数返回后值不丢失。

**经验**：任何需要跨帧保持的状态（滚动位置、计数器、开关状态）都不能用局部变量。三种持久化选择：
1. **`static` 局部变量** — 简单，但只能在一个函数内使用
2. **结构体字段**（如 `AppState`）— 可被多个函数访问，推荐
3. **文件级 `static`** — 同一文件内多个函数共享

---

### 经验 9：账号库状态不同步 — 内部 static 与 AppState 分离

**场景**：账号管理页面始终显示"没有注册用户"，即使已经通过登录页面注册成功。

**架构分析**：

```c
// account.c — 内部有自己的状态
static AccountState g_accountState = {0};

void Account_Init(void) {
    // 从 accounts.txt 读取用户 → 存入 g_accountState
}

bool Account_Register(...) {
    // 注册 → 写入 g_accountState
    Account_Save();
}
```

```c
// app_state.h — AppState 也有自己的状态
typedef struct {
    AccountState account;   // 另一个 AccountState！
    // ...
} AppState;
```

```c
// menu_callbacks.c — 访问的是 AppState 里的 account
#define ACCOUNT (*AppState_GetAccountState())
```

**问题**：
- `Account_Init()` 把用户数据读入 `g_accountState`
- `Account_Register()` 把新用户写入 `g_accountState`  
- 但 `AppState_GetAccountState()` 返回的是 `&g_app.account`，一个**完全不同的内存区域**
- 所以 `ACCOUNT.userCount` 始终为 0，页面显示空白

**示意图**：
```
accounts.txt → account.c:Account_Init() → g_accountState (static)
                                                ↑ 不同步！
AppState_GetAccountState() → g_app.account → 始终为 {0}
```

**修复**：添加状态绑定机制，让 `account.c` 使用外部传入的状态：

```c
// account.c
static AccountState* g_pState = NULL;   // 指向外部状态

void Account_SetState(AccountState* state) {
    g_pState = state;                   // 绑定到 AppState
}

static AccountState* getState(void) {
    return g_pState ? g_pState : &g_internalState;  // 优先用外部状态
}

// 所有 Account_xxx 函数通过 getState() 获取状态指针
void Account_Init(void) {
    AccountState* s = getState();
    s->userCount = 0;
    // ... 读写 s 而非 g_accountState
}
```

```c
// app_state.c
void AppState_Init(void) {
    // ...
    Account_SetState(&g_app.account);   // 在初始化时绑定
}
```

**经验**：当库函数内部有 `static` 状态，而外部框架（`AppState`）也有一份状态时，必须确保两者指向同一块内存。常见的解决方案：
1. **传入指针**：库函数接受外部状态指针
2. **回调注册**：库提供 `SetState()` 函数供外部绑定
3. **直接使用外部状态**：库不维护内部状态，完全由调用方管理

---

### 经验 10：字符串字面量串联 — 遗漏分号

**场景**：在 `fonts.c` 的 `allChinese` 字符串末尾添加新汉字后编译报错 `expected ',' or ';' before 'char'`。

**Bug 代码**：

```c
const char* allChinese = 
    "..."
    "..."
    "..."      // ← 有分号，是最后一行
    "...";     // ← 这是最后一行
```

添加新行后：

```c
const char* allChinese = 
    "..."
    "..."
    "..."
    "..."      // ← 遗漏分号！
    
char wordChars[8192] = {0};  // 编译器认为上一个字符串还没结束
```

**C 语言规则**：相邻的字符串字面量自动串联（编译期行为）：

```c
"abc" "def"  →  "abcdef"
```

串联需要每行（最后一个除外）**不加分号**，只有**最后一行需要分号**结束语句：

```c
const char* s = 
    "第一行"     // ← 没有分号，与下一行串联
    "第二行"     // ← 没有分号，与下一行串联
    "最后一行";  // ← 有分号，语句结束
```

**修复**：确保最后一行有分号：

```c
            "各某另其些点...";   // ← 最后一行有分号
```

**经验**：C 语言字符串字面量串联的常见错误 — 添加新行后忘记在最后一行加分号。编译器的报错位置（下一行 `char wordChars`）与实际错误位置（上一行末尾）不同，需要了解这个模式才能定位。

---

### 经验 11：菜单页面"闪灭" — 子页面未接入菜单栈

**场景**：点击"登录"按钮后看到登录页面一闪而过，立即被账号主页面覆盖。

**原因**：点击按钮时直接调用 `MenuLogin_Show()` 渲染登录界面，但 `CURRENT_MENU` 仍然是账号管理页面，下一帧主循环调用 `CURRENT_MENU->show()` 时又重新渲染了 `MenuAccount_Show()`。

**修复**：使用子页面状态变量，在 `MenuAccount_Show()` 入口判断：

```c
static int g_accountSubPage = 0;    // 0=主页, 1=登录, 2=注册

void MenuAccount_Show(void) {
    if (g_accountSubPage == 1) {
        MenuLogin_Show();    // 直接渲染登录页
        return;
    }
    if (g_accountSubPage == 2) {
        MenuRegister_Show();
        return;
    }
    // ... 账号主页面渲染 ...
}
```

**点击按钮时**只设置状态变量，不直接调用渲染函数：

```c
if (UIButton(u8"登录", loginBtn, STYLE, UI_STATE, 302)) {
    g_accountSubPage = 1;
    // MenuLogin_Show();  ✗ 直接调用会导致闪灭
    // 下一帧 MenuAccount_Show 检测到 subPage=1 自动调用 MenuLogin_Show
}
```

**经验**：`CURRENT_MENU` 控制每帧渲染哪个页面。修改显示必须同步更新 `CURRENT_MENU` 或使用父页面内的子状态控制。直接调用另一个页面的渲染函数而不切换菜单，会导致下一帧被覆盖。

---

## 七、最终经验总结表

| # | 问题 | 根因 | 修复方案 | 核心教训 |
|---|------|------|----------|----------|
| 1 | 退格键不删除字符 | `utf8_delete_left` 循环条件 `>` 应为 `>=` | 条件改为 `>=` | 边界条件差一错误 |
| 2 | 光标位置不准 | 用单一字体测量混合文本 | 改用 `MeasureTextAuto` | 多字体必须逐字符测量 |
| 3 | 文本框焦点 ID 冲突 | 空 buffer 使 `UIGetID` 返回 0 | 改用指针地址 | 哈希函数对空输入返回 0 的陷阱 |
| 4 | 多输入框焦点残留 | `focusItem` 被后续组件覆盖 | 改用 `hasFocus` 判断 | 共享状态被覆盖后的检测失效 |
| 5 | 滚动位置重置 | 局部变量每帧重建 | `static` 变量持久化 | 跨帧状态不能存局部变量 |
| 6 | 账号数据不显示 | 内部 `static` 与 `AppState` 不同步 | 状态绑定 `Account_SetState` | 两处独立内存的同步问题 |
| 7 | 编译报错 | 字符串串联遗漏分号 | 最后一行加分号 | 编译器报错行与实际错误行不同 |
| 8 | 登录页面闪灭 | 直接调用渲染未切换菜单 | 子页面状态变量 | 菜单导航必须走 `CURRENT_MENU` |

## 八、调试方法论

1. **视觉标记**：无需 printf/log，直接在屏幕上画有色方块/改变背景色来验证代码分支是否执行
2. **分步隔离**：先确认"键盘检测是否成功"（绿方块），再确认"删除函数是否执行"，逐步缩小范围
3. **逐帧追踪**：多组件依次调用时，手工逐帧模拟执行顺序，找出状态覆盖的时序问题
4. **对比法**：学单词模式滚动正常 → 对比其代码与词库管理页面代码，找差异
5. **最小复现**：先用简单实现（`strlen` 删除）验证流程正确性，再引入复杂实现（`utf8_delete_left`）
