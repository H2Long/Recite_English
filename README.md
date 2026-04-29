# 背单词软件

基于 Raylib 的桌面背单词应用，支持学单词、背单词（闪卡）、测试三种学习模式。

## 目录结构

```
.
├── main.c              # 主程序入口
├── menu_callbacks.c/h  # 菜单页面回调函数
├── raylib_word_ui.c/h # UI 组件库
├── words.c/h           # 单词管理和进度持久化
├── tree_menu.c/h       # 树形菜单系统
├── fonts.c/h           # 字体加载和混合文本绘制
├── words.txt           # 单词数据文件
└── progress.txt        # 学习进度文件（自动生成）
```

## 核心模块说明

### 1. main.c - 主程序入口

**重要函数：**

| 函数 | 说明 |
|------|------|
| `main()` | 程序入口，初始化窗口、加载数据、主循环 |

---

### 2. raylib_word_ui.c/h - UI 组件库

提供基础的 UI 控件，包括：

| 函数 | 说明 | 关键参数 |
|------|------|----------|
| `UIButton()` | 绘制按钮 | `label` 按钮文字, `rect` 区域 |
| `UICheckbox()` | 复选框 | `label` 标签, `checked` 是否选中 |
| `UITextBox()` | 文本输入框 | `state` 输入状态, `rect` 区域 |
| `UIFlashCard()` | 闪卡组件 | `entry` 单词, `face` 显示哪面, `animTime` 动画 |
| `UIMultipleChoice()` | 选择题 | `question` 题干, `options` 选项数组 |

---

### 3. menu_callbacks.c/h - 菜单页面

| 页面函数 | 说明 |
|----------|------|
| `MenuHome_Show()` | 主页：欢迎信息、统计卡片、功能入口 |
| `MenuLearn_Show()` | 学单词：左侧单词列表 + 右侧详情 |
| `MenuReview_Show()` | 背单词：闪卡翻转学习 |
| `MenuTest_Show()` | 测试：选择题答题 |
| `MenuProgress_Show()` | 进度：学习统计和单词进度列表 |

---

## 可调整参数详细清单

### 一、窗口尺寸配置

**文件：** `menu_callbacks.h` 或 `main.c`（两处都需保持一致）

```c
// 定义位置：menu_callbacks.h 第 25-26 行
#define SCREEN_WIDTH 1600   // 窗口宽度（像素）
#define SCREEN_HEIGHT 1000  // 窗口高度（像素）
```

修改说明：
- 增大窗口：需调整各页面元素位置避免留白
- 减小窗口：需调整布局参数避免元素溢出

---

### 二、字体大小配置

#### 2.1 UI 组件默认字体大小

**文件：** `main.c` 第 121-123 行

```c
g_style.fontSizeSmall = 22.0f;    // 小号字体（如复选框标签）
g_style.fontSizeNormal = 28.0f;   // 普通字体（按钮文字、列表项）
g_style.fontSizeLarge = 46.0f;    // 大号字体（单词显示）
```

#### 2.2 测试模式选项字体大小

**文件：** `menu_callbacks.c` 第 476 行

```c
UIDrawTextRec(options[i], textRect, 30, 1, true, g_style.theme.textPrimary);
//                                   ^^ 字体大小，默认 30
```

#### 2.3 学单词详情页字体

**文件：** `menu_callbacks.c`

| 行号 | 内容 | 变量/参数 | 默认值 |
|------|------|-----------|--------|
| 180 | 单词本身 | `MeasureTextAuto()` | 56 |
| 185 | 音标 | `MeasureTextAuto()` | 32 |
| 194 | "释义"标签 | `MeasureTextAuto()` | 28 |
| 198 | 释义内容 | `UIDrawTextRec()` fontSize | 30 |
| 202 | "例句"标签 | `MeasureTextAuto()` | 28 |
| 209 | 例句内容 | `UIDrawTextRec()` fontSize | 26 |
| 225 | 学习状态统计 | `UIDrawTextRec()` fontSize | 22 |

---

### 三、侧边导航栏配置

**文件：** `main.c` 第 146 行

```c
Rectangle menuRect = {10, 70, 230, SCREEN_HEIGHT - 80};
//                     ^  ^  ^^^  ^^^^^^^^^^^^^^^^^^^^
//                     |  |  |    |
//                     |  |  |    高度 = 窗口高度 - 上下边距
//                     |  |  导航栏宽度
//                     |  距顶部距离
//                     距左侧距离
```

**主内容区域起始位置**（需与导航栏宽度保持一致）：
```c
// 位置：menu_callbacks.c 各页面函数
Rectangle contentRect = {250, 80, SCREEN_WIDTH - 270, SCREEN_HEIGHT - 100};
//                       ^^^^ 左侧留白 = 导航栏宽度 + 间距（230 + 20）
```

---

### 四、闪卡相关配置

#### 4.1 闪卡整体布局

**文件：** `menu_callbacks.c` 第 259-268 行

```c
// 进度条位置
Rectangle progressBar = {SCREEN_WIDTH/2 - 250, 100, 500, 30};
//                       x坐标                    宽度

// 进度文字位置
Vector2 pSize = MeasureTextAuto(progressText, 22, 1);
DrawTextAuto(progressText, (Vector2){SCREEN_WIDTH/2 - pSize.x/2, progressBar.y + 35}, ...);
//                                                                         ^^^^^^^^ 距进度条下方距离
```

#### 4.2 闪卡卡片尺寸和位置

**文件：** `menu_callbacks.c` 第 273 行

```c
Rectangle cardRect = {SCREEN_WIDTH/2 - 220, 160, 440, 264};
//                    x坐标           ^^^^^  ^^^ ^^^^ ^^^
//                    |               |      |   |    |
//                    |               |      |   |    卡片高度
//                    |               |      |   卡片宽度
//                    |               |      距顶部距离
//                    |               水平居中偏移
//                    水平居中：SCREEN_WIDTH/2 - 卡片宽度/2
```

#### 4.3 闪卡按钮位置

**文件：** `raylib_word_ui.c` 第 977 行

```c
Rectangle btnRow = { rect.x, rect.y + rect.height + 40, 220, 40 };
//                                    ^^^^^^^^^^^^^^^^ 按钮与卡片间距
//                                                      增加数值按钮下移
```

#### 4.4 本轮统计区域位置

**文件：** `menu_callbacks.c` 第 317 行

```c
Rectangle statsRect = {SCREEN_WIDTH/2 - 180, 470, 360, 65};
//                     x坐标                    ^^^^ 宽度 ^^^^ 高度
```

#### 4.5 提示文字位置

**文件：** `menu_callbacks.c` 第 327 行

```c
Rectangle tipRect = {SCREEN_WIDTH/2 - 220, 545, 440, 50};
//                    x坐标                   ^^^^ 宽度
```

---

### 五、测试模式配置

#### 5.1 题干区域

**文件：** `menu_callbacks.c` 第 388-394 行

```c
Rectangle qRect = {SCREEN_WIDTH/2 - 250, 100, 500, 110};
//                  x坐标                 ^^^^ 宽度 ^^^^ 高度

char question[128];
Vector2 qSize = MeasureTextAuto(question, 32, 1);
//                                         ^^ 题干字体大小
```

#### 5.2 选项列表区域

**文件：** `menu_callbacks.c` 第 440 行

```c
UILayout optLayout = UIBeginLayout((Rectangle){SCREEN_WIDTH/2 - 250, 220, 500, 420}, ...);
//                                                        x坐标     y坐标  宽度  高度
```

#### 5.3 单个选项样式

**文件：** `menu_callbacks.c` 第 443 行

```c
Rectangle optRect = UILayoutNext(&optLayout, -1, 85);
//                                               ^^ 选项高度，默认 85
```

#### 5.4 选项标签字体

**文件：** `menu_callbacks.c` 第 471-476 行

```c
snprintf(optLabel, sizeof(optLabel), "%c. ", 'A' + i);
Vector2 labelSize = MeasureTextAuto(optLabel, 24, 1);
//                                         ^^ 标签(A/B/C/D)字体大小

UIDrawTextRec(options[i], textRect, 30, 1, true, ...);
//                                ^^ 选项内容字体大小
```

#### 5.5 进度显示位置

**文件：** `menu_callbacks.c` 第 496-501 行

```c
Rectangle progressRect = {SCREEN_WIDTH/2 - 150, 660, 300, 40};
//                         x坐标                 ^^^^ 宽度
```

#### 5.6 下一题按钮位置

**文件：** `menu_callbacks.c` 第 505 行

```c
Rectangle nextBtn = {SCREEN_WIDTH/2 - 75, 710, 150, 55};
//                    x坐标                 ^^^ 宽度 ^^^ 高度
```

---

### 六、学单词模式配置

#### 6.1 单词列表

**文件：** `menu_callbacks.c` 第 108 行

```c
Rectangle listArea = UILayoutNext(&layout, 350, -1);
//                                          ^^^ 列表宽度
```

**复选框位置：**
```c
// 第 112 行
Rectangle filterRect = {listArea.x + 15, listArea.y + 15, 220, 35};
//                                       ^^^^ x偏移  ^^^^ y偏移  ^^^^ 宽度
```

**列表项高度：**
```c
// 第 140 行
Rectangle itemRect = {scrollView.x, scrollView.y + listIndex * 60 - sv.scrollOffset.y, ...};
//                                                          ^^ 每项高度 60
```

#### 6.2 导航按钮

**文件：** `menu_callbacks.c` 第 228-232 行

```c
Rectangle navRect = UILayoutNext(&detailLayout, -1, 60);
//                                              ^^ 导航区域高度

Rectangle prevBtn = UILayoutNext(&navLayout, 120, 50);
//                                         ^^^ 宽度  ^^ 高度
Rectangle nextBtn = UILayoutNext(&navLayout, 120, 50);
```

---

### 七、进度页面配置

#### 7.1 统计卡片

**文件：** `menu_callbacks.c` 第 525 行

```c
Rectangle statsCard = UILayoutNext(&layout, -1, 130);
//                                              ^^^ 卡片高度
```

#### 7.2 清除进度按钮

**文件：** `menu_callbacks.c` 第 559-561 行

```c
Rectangle resetBtn = UILayoutNext(&layout, -1, 55);
resetBtn.x = SCREEN_WIDTH - 400;   // x 位置
resetBtn.width = 200;              // 按钮宽度
```

#### 7.3 进度列表表头

**文件：** `menu_callbacks.c` 第 575-581 行

```c
Rectangle headerRect = UILayoutNext(&listLayout, -1, 45);
//                                              ^^ 表头高度
```

#### 7.4 进度列表项高度

**文件：** `menu_callbacks.c` 第 592 行

```c
Rectangle rowRect = {scrollArea.x, scrollArea.y + i * 50 - sv.scrollOffset.y, ...};
//                                                          ^^ 每行高度 50
```

---

### 八、主页配置

#### 8.1 功能卡片

**文件：** `menu_callbacks.c` 第 55-93 行

```c
// 卡片容器
Rectangle cardsRect = UILayoutNext(&layout, -1, 160);
//                                              ^^^ 卡片容器高度

// 单个卡片宽度
Rectangle cardRect = UILayoutNext(&cardsLayout, 320, -1);
//                                         ^^^ 卡片宽度

// 开始按钮
Rectangle goBtn = {cardRect.x + cardRect.width - 130, cardRect.y + cardRect.height - 65, 110, 50};
//                                                          x偏移          y偏移          ^^^ 宽度 ^^^ 高度
```

#### 8.2 标题字体

**文件：** `menu_callbacks.c` 第 79-80 行

```c
Vector2 titleS = MeasureTextAuto(modes[i].title, 36, 1);
//                                         ^^ 卡片标题字体
```

---

### 九、UI 主题颜色配置

**文件：** `raylib_word_ui.c` 第 375-410 行

```c
UITheme UIThemeLight(void) {
    return (UITheme){
        .primary = { 70, 130, 180, 255 },        // 主色调（按钮）
        .primaryHover = { 100, 149, 237, 255 },  // 悬停状态
        .primaryPressed = { 65, 105, 225, 255 }, // 按下状态
        .secondary = { 211, 211, 211, 255 },     // 次要色
        .background = { 245, 245, 245, 255 },   // 背景色
        .panelBg = { 255, 255, 255, 255 },      // 面板背景
        .textPrimary = { 30, 30, 30, 255 },    // 主要文字
        .textSecondary = { 120, 120, 120, 255 },// 次要文字
        .inputBg = { 255, 255, 255, 255 },      // 输入框背景
        .inputBorder = { 200, 200, 200, 255 },  // 输入框边框
        .error = { 220, 20, 60, 255 },          // 错误/失败（红色）
        .success = { 50, 205, 50, 255 }         // 成功（绿色）
    };
}
```

颜色格式：`(R, G, B, A)`，范围 0-255

---

### 十、单词数据格式

`words.txt` 文件格式（每行一个单词，字段用 `|` 分隔）：

```
单词|音标|词性释义|例句
```

示例：
```
advance|[əd'væns]|v. 前进；进步|The army began to advance.
adventure|[əd'ventʃər]|n. 冒险|She loves adventure.
```

---

## 快速调整参考表

| 调整项 | 文件 | 行号 | 变量/参数 |
|--------|------|------|-----------|
| 窗口尺寸 | menu_callbacks.h | 25-26 | `SCREEN_WIDTH/HEIGHT` |
| UI默认字体 | main.c | 121-123 | `fontSizeSmall/Normal/Large` |
| 测试选项字体 | menu_callbacks.c | 476 | 第三个参数 |
| 侧边栏宽度 | main.c | 146 | `230` |
| 闪卡尺寸 | menu_callbacks.c | 273 | `cardRect` |
| 闪卡按钮位置 | raylib_word_ui.c | 977 | `rect.y + rect.height + 40` |
| 测试选项高度 | menu_callbacks.c | 443 | `UILayoutNext` 第三个参数 |
| 列表项高度 | menu_callbacks.c | 140 | `listIndex * 60` |
| 主题颜色 | raylib_word_ui.c | 375-410 | `UITheme` 结构体 |

---

## 编译运行

```bash
mkdir build && cd build
cmake ..
make
./main_c
```

需要安装 Raylib 库和 CMake。
