// ============================================================================
// Raylib 单词背诵 UI 组件库头文件
// 功能：定义 UI 组件接口、数据结构和主题
// ============================================================================

#ifndef RAYLIB_WORD_UI_H
#define RAYLIB_WORD_UI_H

#include "raylib.h"
#include <stdbool.h>

// ============================================================================
// 布局方向枚举
// ============================================================================

typedef enum {
    UI_DIR_HORIZONTAL = 0,  // 水平布局
    UI_DIR_VERTICAL = 1     // 垂直布局
} UIDirection;

// ============================================================================
// 布局结构
// ============================================================================

// 布局容器结构
// 用于管理 UI 元素的自动排列
typedef struct {
    Rectangle container;    // 容器矩形区域
    UIDirection direction;   // 布局方向
    float spacing;          // 元素间距
    float padding;          // 内边距
    Vector2 cursor;         // 下一个元素的起始位置
} UILayout;

// ============================================================================
// 滚动视图结构
// ============================================================================

// 滚动视图状态
typedef struct {
    Rectangle viewport;      // 可视区域
    Vector2 contentSize;     // 内容实际大小
    Vector2 scrollOffset;   // 滚动偏移量
    bool showScrollbar;     // 是否显示滚动条
} UIScrollView;

// ============================================================================
// 文本框状态结构
// ============================================================================

// 文本输入框状态
typedef struct {
    char buffer[1024];       // 文本缓冲区
    int cursor;             // 光标位置（字符索引）
    bool hasFocus;          // 是否获得焦点
    float backspaceHoldTime;    // 退格键按住时间
    float lastBackspaceDelete;  // 上次删除时间
} UITextBoxState;

// ============================================================================
// 搜索栏状态结构
// ============================================================================

// 搜索栏状态
typedef struct {
    UITextBoxState textState;    // 文本框状态
    bool searchTriggered;         // 搜索是否被触发
} SearchBarState;

// ============================================================================
// 闪卡状态枚举
// ============================================================================

// 闪卡显示面
typedef enum {
    CARD_FRONT = 0,  // 正面（显示单词）
    CARD_BACK = 1    // 背面（显示释义）
} CardFace;

// ============================================================================
// 主题颜色结构
// ============================================================================

// UI 主题颜色配置
typedef struct {
    Color primary;        // 主色调（按钮、标题等）
    Color primaryHover;   // 主色调悬停状态
    Color primaryPressed; // 主色调按下状态
    Color secondary;      // 次要色
    Color background;    // 背景色
    Color panelBg;       // 面板背景色
    Color textPrimary;   // 主要文字颜色
    Color textSecondary; // 次要文字颜色
    Color inputBg;       // 输入框背景色
    Color inputBorder;   // 输入框边框色
    Color error;         // 错误/失败颜色
    Color success;       // 成功颜色
} UITheme;

// ============================================================================
// UI 样式结构
// ============================================================================

// UI 样式配置
typedef struct {
    UITheme theme;           // 主题颜色
    Font font;               // 默认字体
    float fontSizeSmall;     // 小号字体大小
    float fontSizeNormal;    // 普通字体大小
    float fontSizeLarge;     // 大号字体大小
    float spacing;           // 默认间距
    int cornerRadius;         // 圆角半径
} UIStyle;

// ============================================================================
// UI 状态结构
// ============================================================================

// UI 交互状态
typedef struct {
    Vector2 mousePos;        // 鼠标位置
    bool mouseDown;          // 鼠标按下
    bool mousePressed;       // 鼠标刚按下
    bool mouseReleased;      // 鼠标刚释放
    int hotItem;             // 当前悬停的控件ID
    int activeItem;          // 当前按下的控件ID
    int focusItem;           // 当前焦点的控件ID
    int keyPressed;          // 按下的键
    int charPressed;        // 按下的字符
} UIState;

// ============================================================================
// 单词数据结构
// ============================================================================

// 单词条目（来自 words.txt）
typedef struct {
    char* word;              // 单词
    char* phonetic;          // 音标
    char* definition;       // 释义
    char* example;           // 例句
    char* exampleTranslation; // 例句翻译
} WordEntry;

// ============================================================================
// 函数声明
// ============================================================================

// 字体设置
void UISetFonts(Font chineseFont, Font englishFont);    // 设置中英文字体
void UISetLatinFont(Font latinFont);                     // 设置 IPA 音标字体

// 样式管理
void UIStyleInit(UIStyle* style);                       // 初始化样式为默认值
void UILoadFont(UIStyle* style, const char* filename); // 加载自定义字体
void UIUnloadFont(UIStyle* style);                      // 卸载字体

// 主题管理
UITheme UIThemeLight(void);    // 获取浅色主题
UITheme UIThemeDark(void);     // 获取深色主题

// 状态管理
void UIBegin(UIState* state);  // 开始一帧（更新鼠标和键盘状态）
void UIEnd(UIState* state);    // 结束一帧

// ID 生成
int UIGetID(const char* label);  // 根据标签生成唯一ID

// 布局系统
UILayout UIBeginLayout(Rectangle container, UIDirection dir, float spacing, float padding);  // 开始布局
Rectangle UILayoutNext(UILayout* layout, float width, float height);  // 获取下一个布局区域

// 基础控件
bool UIButton(const char* label, Rectangle rect, UIStyle* style, UIState* state, int id);  // 普通按钮
bool UIButtonEx(const char* label, Rectangle rect, UIStyle* style, UIState* state, bool enabled, int id);  // 扩展按钮（支持禁用）
bool UICheckbox(const char* label, Rectangle rect, bool* checked, UIStyle* style, UIState* state);  // 复选框
void UILabel(const char* text, Rectangle rect, UIStyle* style, Color color);  // 标签

// 滚动视图
void UIBeginScrollView(UIScrollView* scroll, Rectangle viewport, Vector2 contentSize);  // 开始滚动视图
void UIEndScrollView(UIScrollView* scroll, UIStyle* style, UIState* state);  // 结束滚动视图
bool UIListItem(const char* text, Rectangle itemRect, UIStyle* style, UIState* state);  // 列表项

// 文本输入
bool UITextBox(UITextBoxState* state, Rectangle rect, UIStyle* style, UIState* uistate, bool password);  // 文本输入框

// 文本绘制（需配合 fonts.h 中的函数使用）
void UIDrawText(const char* text, Vector2 pos, float fontSize, float spacing, Color tint);  // 绘制单行文本
void UIDrawTextRec(const char* text, Rectangle rec, float fontSize, float spacing, bool wordWrap, Color tint);  // 绘制多行文本（自动换行）

// 单词卡片
void UIWordCard(WordEntry* entry, Rectangle rect, UIStyle* style);  // 显示单词卡片

// 闪卡
int UIFlashCard(WordEntry* entry, Rectangle rect, CardFace* face, UIStyle* style, UIState* state, float* animTime);  // 闪卡组件

// 选择题
int UIMultipleChoice(const char* question, const char* options[], int optionCount,
                     int correctIndex, Rectangle rect, UIStyle* style, UIState* state);  // 选择题组件

// 搜索栏
void UISearchBar(SearchBarState* sb, Rectangle rect, UIStyle* style, UIState* state);  // 搜索栏

// 单词列表视图
int UIWordListView(WordEntry* words, int count, int* selectedIndex,
                   UIScrollView* scroll, UIStyle* style, UIState* state);  // 单词列表

#endif // RAYLIB_WORD_UI_H
