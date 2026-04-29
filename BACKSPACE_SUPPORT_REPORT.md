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

## 五、经验总结

| 经验 | 说明 |
|------|------|
| 1. 分步调试 | 先用视觉标记确认焦点（点击变蓝），再用视觉标记确认按键检测（绿方块），定位问题所在层 |
| 2. 边界条件 | `>` 和 `>=` 的边界差异在光标位于字符串末尾时暴露，这类问题在 ASCII 文本中不会出现异常，但在中文等多字节字符场景更突出 |
| 3. `GetKeyPressed` vs `IsKeyPressed` | `GetKeyPressed()` 基于事件队列，只返回一次；`IsKeyPressed()` 基于状态检测。同时使用两者可以提高按键捕获的可靠性 |
| 4. 哈希 ID 陷阱 | `UIGetID("")` 对空字符串返回 0，与 `focusItem` 的初始值相同，导致永久焦点假象。用指针地址生成 ID 更可靠 |
| 5. 从简到繁 | 先使用简单的 `strlen` 删除验证流程，确认删除本身能工作后，再调试 UTF-8 函数的细节 |
