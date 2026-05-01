// 立即模式UI组件库 — 按钮/输入框/卡片/滚动视图/主题

#ifndef RAYLIB_WORD_UI_H
#define RAYLIB_WORD_UI_H

#include "raylib.h"
#include <stdbool.h>

// 布局方向
typedef enum { UI_DIR_HORIZONTAL = 0, UI_DIR_VERTICAL = 1 } UIDirection;

// 布局容器
typedef struct {
    Rectangle container;
    UIDirection direction;
    float spacing, padding;
    Vector2 cursor;
} UILayout;

// 滚动视图
typedef struct {
    Rectangle viewport;
    Vector2 contentSize;
    Vector2 scrollOffset;
    bool showScrollbar;
} UIScrollView;

typedef struct {
    UIScrollView sv;
    float* persistedOffset;
} PersistentScrollView;

// 文本框状态
typedef struct {
    char buffer[1024];
    int cursor;
    bool hasFocus;
    float backspaceHoldTime;
    float lastBackspaceDelete;
} UITextBoxState;

// 搜索栏
typedef struct {
    UITextBoxState textState;
    bool searchTriggered;
} SearchBarState;

// 闪卡面
typedef enum { CARD_FRONT = 0, CARD_BACK = 1 } CardFace;

// 主题颜色
typedef struct {
    Color primary, primaryHover, primaryPressed;
    Color secondary;
    Color background, panelBg;
    Color textPrimary, textSecondary;
    Color inputBg, inputBorder;
    Color error, success;
} UITheme;

// UI样式
typedef struct {
    UITheme theme;
    Font font;
    float fontSizeSmall, fontSizeNormal, fontSizeLarge;
    float spacing;
    int cornerRadius;
} UIStyle;

// UI交互状态
typedef struct {
    Vector2 mousePos;
    bool mouseDown, mousePressed, mouseReleased;
    int hotItem, activeItem, focusItem;
    int keyPressed, charPressed;
} UIState;

// 单词条目
typedef struct {
    char* word;
    char* phonetic;
    char* definition;
    char* example;
    char* exampleTranslation;
} WordEntry;

// 字体设置
void UISetFonts(Font chineseFont, Font englishFont);
void UISetLatinFont(Font latinFont);

// 样式/主题
void UIStyleInit(UIStyle* style);
UITheme UIThemeLight(void);
UITheme UIThemeDark(void);

// 状态管理
void UIBegin(UIState* state);
void UIEnd(UIState* state);
int UIGetID(const char* label);

// 布局
UILayout UIBeginLayout(Rectangle container, UIDirection dir, float spacing, float padding);
Rectangle UILayoutNext(UILayout* layout, float width, float height);

// 基础控件
bool UIButton(const char* label, Rectangle rect, UIStyle* style, UIState* state, int id);
bool UIButtonEx(const char* label, Rectangle rect, UIStyle* style, UIState* state, bool enabled, int id);
bool UICheckbox(const char* label, Rectangle rect, bool* checked, UIStyle* style, UIState* state);
void UILabel(const char* text, Rectangle rect, UIStyle* style, Color color);

// 滚动视图
void UIBeginScrollView(UIScrollView* scroll, Rectangle viewport, Vector2 contentSize);
void UIEndScrollView(UIScrollView* scroll, UIStyle* style, UIState* state);
bool UIListItem(const char* text, Rectangle itemRect, UIStyle* style, UIState* state);

// 文本输入
bool UITextBox(UITextBoxState* state, Rectangle rect, UIStyle* style, UIState* uistate, bool password);

// 文本绘制
void UIDrawText(const char* text, Vector2 pos, float fontSize, float spacing, Color tint);
void UIDrawTextRec(const char* text, Rectangle rec, float fontSize, float spacing, bool wordWrap, Color tint);

// 单词卡片/闪卡
void UIWordCard(WordEntry* entry, Rectangle rect, UIStyle* style);
int UIFlashCard(WordEntry* entry, Rectangle rect, CardFace* face, UIStyle* style, UIState* state, float* animTime);

// 选择题
int UIMultipleChoice(const char* question, const char* options[], int optionCount,
                     int correctIndex, Rectangle rect, UIStyle* style, UIState* state);

// 搜索栏
void UISearchBar(SearchBarState* sb, Rectangle rect, UIStyle* style, UIState* state);

// 单词列表
int UIWordListView(WordEntry* words, int count, int* selectedIndex,
                   UIScrollView* scroll, UIStyle* style, UIState* state);

// 持久化滚动
void UIBeginPersistentScrollView(PersistentScrollView* psv, Rectangle viewport, Vector2 contentSize, float* offsetPtr);
void UIEndPersistentScrollView(PersistentScrollView* psv, UIStyle* style, UIState* state);

#endif
