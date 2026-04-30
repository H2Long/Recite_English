// ============================================================================
// Raylib 单词背诵 UI 组件库实现
// 功能：提供完整的 UI 组件，包括按钮、输入框、闪卡、滚动视图等
// ============================================================================

#include "raylib_word_ui.h"
#include <string.h>

// 外部函数：测量混合文本宽度
extern Vector2 MeasureTextAuto(const char* text, float fontSize, float spacing);
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

// ============================================================================
// 字体设置
// ============================================================================

// 全局字体变量
static Font g_chFont = {0};     // 中文字体
static Font g_enFont = {0};     // 英文字体
static Font g_latinFont = {0};  // IPA/音标字体

/**
 * 设置中英文字体
 * @param chineseFont 中文字体
 * @param englishFont 英文字体
 */
void UISetFonts(Font chineseFont, Font englishFont) {
    g_chFont = chineseFont;
    g_enFont = englishFont;
}

/**
 * 设置 IPA 音标字体
 * @param latinFont IPA 音标字体
 */
void UISetLatinFont(Font latinFont) {
    g_latinFont = latinFont;
}

// ============================================================================
// UTF-8 辅助函数
// ============================================================================

/**
 * 获取 UTF-8 字符的字节长度
 * @param c 首字节
 * @return 字符占用的字节数（1-4）
 */
static int utf8_char_len(unsigned char c) {
    if (c < 0x80) return 1;           // ASCII: 1字节
    if ((c & 0xE0) == 0xC0) return 2; // 2字节字符
    if ((c & 0xF0) == 0xE0) return 3; // 3字节字符（中文）
    if ((c & 0xF8) == 0xF0) return 4; // 4字节字符
    return 1;
}

/**
 * 检查字符是否是 IPA/Latin 扩展字符
 * @param c Unicode 码点
 * @return 是否为 IPA 字符
 */
static inline bool isLatinExtended(int c) {
    // 检查多个 Unicode 范围
    if (c >= 0x00A0 && c <= 0x00FF) return true;  // Latin-1 Supplement
    if (c >= 0x0100 && c <= 0x017F) return true;  // Latin Extended-A
    if (c >= 0x0180 && c <= 0x024F) return true;  // Latin Extended-B
    if (c >= 0x0250 && c <= 0x02AF) return true;  // IPA Extensions（音标）
    if (c >= 0x02B0 && c <= 0x02FF) return true;  // Spacing Modifier Letters
    if (c >= 0x0300 && c <= 0x036F) return true;  // Combining Diacritical Marks
    if (c >= 0x0370 && c <= 0x03FF) return true;  // Greek and Coptic
    if (c >= 0x1D00 && c <= 0x1DBF) return true;  // Phonetic Extensions
    if (c >= 0x1E00 && c <= 0x1EFF) return true;  // Latin Extended Additional
    return false;
}

/**
 * 解析 UTF-8 字符串获取完整 Unicode 码点
 * @param text UTF-8 字符串指针
 * @return Unicode 码点
 */
static int utf8_to_codepoint(const unsigned char* text) {
    unsigned char byte = text[0];
    if (byte < 0x80) return byte;  // ASCII

    // 根据首字节判断字符长度
    if (byte < 0xE0) {
        // 2字节字符
        return ((byte & 0x1F) << 6) | (text[1] & 0x3F);
    }
    if (byte < 0xF0) {
        // 3字节字符（中文）
        return ((byte & 0x0F) << 12) | ((text[1] & 0x3F) << 6) | (text[2] & 0x3F);
    }
    // 4字节字符
    return ((byte & 0x07) << 18) | ((text[1] & 0x3F) << 12) | ((text[2] & 0x3F) << 6) | (text[3] & 0x3F);
}

/**
 * 根据字符类型选择合适的字体
 * @param text 字符指针
 * @return 应该使用的字体
 */
static Font UIGetFontForChar(const unsigned char* text) {
    unsigned char byte = text[0];

    // ASCII 字符使用英文字体
    if (byte < 0x80) return g_enFont.texture.id != 0 ? g_enFont : GetFontDefault();

    // 解析字符
    int codepoint = utf8_to_codepoint(text);

    // IPA/Latin 扩展字符优先使用 IPA 字体
    if (isLatinExtended(codepoint)) {
        if (g_latinFont.texture.id != 0) return g_latinFont;
        if (g_chFont.texture.id != 0) return g_chFont;
        return GetFontDefault();
    }

    // 其他字符（中文等）使用中文字体
    return g_chFont.texture.id != 0 ? g_chFont : GetFontDefault();
}

// ============================================================================
// 文本绘制
// ============================================================================

/**
 * 绘制单行文本（自动为每个字符选择正确字体）
 * 支持混合中英文和 IPA 音标
 * @param text 文本内容
 * @param pos 绘制位置（左上角）
 * @param fontSize 字体大小
 * @param spacing 字间距
 * @param tint 文字颜色
 */
void UIDrawText(const char* text, Vector2 pos, float fontSize, float spacing, Color tint) {
    if (!text || text[0] == '\0') return;

    float x = pos.x;
    while (*text) {
        unsigned char uc = *(unsigned char*)text;

        // 处理空格
        if (uc == ' ' || uc == '\t') {
            Font spFont = g_enFont.texture.id != 0 ? g_enFont : GetFontDefault();
            x += MeasureTextEx(spFont, " ", fontSize, spacing).x;
            text++;
            continue;
        }

        // 获取字符应使用的字体
        Font currentFont = UIGetFontForChar((const unsigned char*)text);
        int charLen = utf8_char_len(uc);

        // 提取字符
        char segment[5] = {0};
        for (int i = 0; i < charLen && *text; i++) {
            segment[i] = *text++;
        }

        // 绘制字符
        DrawTextEx(currentFont, segment, (Vector2){ x, pos.y }, fontSize, spacing, tint);

        // 更新位置
        Vector2 s = MeasureTextEx(currentFont, segment, fontSize, spacing);
        x += s.x;
    }
}

/**
 * 绘制文本（带矩形裁剪和自动换行）
 * 支持混合中英文和 IPA 音标
 * @param text 文本内容
 * @param rec 绘制区域
 * @param fontSize 字体大小
 * @param spacing 字间距
 * @param wordWrap 是否自动换行
 * @param tint 文字颜色
 */
void UIDrawTextRec(const char* text, Rectangle rec, float fontSize, float spacing, bool wordWrap, Color tint) {
    if (!text || text[0] == '\0') return;

    // 开启裁剪模式，只绘制区域内的内容
    BeginScissorMode((int)rec.x, (int)rec.y, (int)rec.width, (int)rec.height);

    float textWidth = rec.width;
    float lineY = rec.y;
    float x = rec.x;

    // 逐字符处理
    while (*text && lineY < rec.y + rec.height) {
        unsigned char uc = *(unsigned char*)text;

        // 处理空格
        if (uc == ' ' || uc == '\t') {
            Font spFont = g_enFont.texture.id != 0 ? g_enFont : GetFontDefault();
            x += MeasureTextEx(spFont, " ", fontSize, spacing).x;
            text++;
            continue;
        }

        // 处理换行符
        if (uc == '\n') {
            text++;
            lineY += fontSize + spacing;
            x = rec.x;
            continue;
        }

        // 获取字符应使用的字体
        Font currentFont = UIGetFontForChar((const unsigned char*)text);
        if (currentFont.texture.id == 0) currentFont = GetFontDefault();

        // 收集相同字体的连续字符
        char segment[256] = {0};
        int segLen = 0;
        float segWidth = 0;
        Font segFont = currentFont;

        while (*text && segLen < 250) {
            Font charFont = UIGetFontForChar((const unsigned char*)text);
            if (charFont.texture.id == 0) charFont = GetFontDefault();

            // 如果字体改变，停止收集
            if (charFont.texture.id != segFont.texture.id && segLen > 0) break;

            segFont = charFont;
            int len = utf8_char_len(uc);
            for (int i = 0; i < len && *text; i++) {
                segment[segLen++] = *text++;
            }
            uc = *(unsigned char*)text;
            Vector2 s = MeasureTextEx(segFont, segment, fontSize, spacing);
            segWidth = s.x;
        }

        // 检查是否需要换行
        if (x != rec.x && x + segWidth > rec.x + textWidth) {
            lineY += fontSize + spacing;
            x = rec.x;
        }

        // 绘制文本片段
        DrawTextEx(segFont, segment, (Vector2){ x, lineY }, fontSize, spacing, tint);
        x += segWidth;
    }

    EndScissorMode();
}

// ============================================================================
// UTF-8 字符串辅助函数
// ============================================================================

/**
 * 计算 UTF-8 字符串的字符数（不是字节数）
 * @param s UTF-8 字符串
 * @return 字符数量
 */
static int utf8_strlen(const char* s) {
    int count = 0;
    while (*s) {
        s += utf8_char_len((unsigned char)*s);
        count++;
    }
    return count;
}

/**
 * 将字符索引转换为字节偏移
 * @param s UTF-8 字符串
 * @param index 字符索引
 * @return 字节偏移量
 */
static int utf8_index_to_byte(const char* s, int index) {
    int byte_offset = 0;
    int char_count = 0;
    while (s[byte_offset] && char_count < index) {
        byte_offset += utf8_char_len((unsigned char)s[byte_offset]);
        char_count++;
    }
    return byte_offset;
}

/**
 * 插入 Unicode 码点到字符串
 * @param buffer 目标缓冲区
 * @param bufsize 缓冲区大小
 * @param cursor_char_index 光标位置（字符索引）
 * @param codepoint 要插入的 Unicode 码点
 * @return 插入的字节数，0表示失败
 */
static int utf8_insert_codepoint(char* buffer, int bufsize, int* cursor_char_index, int codepoint) {
    int byte_offset = utf8_index_to_byte(buffer, *cursor_char_index);
    int old_len = (int)strlen(buffer);

    // 将码点转换为 UTF-8
    char utf8[5] = {0};
    int utf8_len = 0;
    if (codepoint < 0x80) {
        utf8[0] = (char)codepoint;
        utf8_len = 1;
    } else if (codepoint < 0x800) {
        utf8[0] = (char)(0xC0 | (codepoint >> 6));
        utf8[1] = (char)(0x80 | (codepoint & 0x3F));
        utf8_len = 2;
    } else if (codepoint < 0x10000) {
        utf8[0] = (char)(0xE0 | (codepoint >> 12));
        utf8[1] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
        utf8[2] = (char)(0x80 | (codepoint & 0x3F));
        utf8_len = 3;
    } else {
        utf8[0] = (char)(0xF0 | (codepoint >> 18));
        utf8[1] = (char)(0x80 | ((codepoint >> 12) & 0x3F));
        utf8[2] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
        utf8[3] = (char)(0x80 | (codepoint & 0x3F));
        utf8_len = 4;
    }

    // 检查空间
    if (old_len + utf8_len >= bufsize) return 0;

    // 插入字符
    memmove(buffer + byte_offset + utf8_len, buffer + byte_offset, old_len - byte_offset + 1);
    memcpy(buffer + byte_offset, utf8, utf8_len);
    (*cursor_char_index)++;
    return utf8_len;
}

/**
 * 向左删除字符（退格）
 * @param buffer 目标缓冲区
 * @param cursor_char_index 光标位置（会被更新）
 */
static void utf8_delete_left(char* buffer, int* cursor_char_index) {
    if (*cursor_char_index <= 0) return;

    int byte_offset = utf8_index_to_byte(buffer, *cursor_char_index);
    int prev_byte = 0;
    int prev_index = 0;

    // 找到前一个字符的起始位置
    while (prev_byte < byte_offset) {
        int len = utf8_char_len((unsigned char)buffer[prev_byte]);
        if (prev_byte + len >= byte_offset) break;
        prev_byte += len;
        prev_index++;
    }

    // 删除字符
    memmove(buffer + prev_byte, buffer + byte_offset, strlen(buffer + byte_offset) + 1);
    *cursor_char_index = prev_index;
}

/**
 * 向右删除字符（Delete键）
 * @param buffer 目标缓冲区
 * @param cursor_char_index 光标位置
 */
static void utf8_delete_right(char* buffer, int* cursor_char_index) {
    int byte_offset = utf8_index_to_byte(buffer, *cursor_char_index);
    if (buffer[byte_offset] == '\0') return;

    int next_len = utf8_char_len((unsigned char)buffer[byte_offset]);
    memmove(buffer + byte_offset, buffer + byte_offset + next_len, strlen(buffer + byte_offset + next_len) + 1);
}

// ============================================================================
// 主题
// ============================================================================

/**
 * 获取浅色主题
 * @return UITheme 结构
 */
UITheme UIThemeLight(void) {
    return (UITheme){
        .primary = { 70, 130, 180, 255 },        // 蓝色主色调
        .primaryHover = { 100, 149, 237, 255 },  // 悬停状态
        .primaryPressed = { 65, 105, 225, 255 }, // 按下状态
        .secondary = { 211, 211, 211, 255 },    // 灰色次要色
        .background = { 245, 245, 245, 255 },    // 浅灰背景
        .panelBg = { 255, 255, 255, 255 },       // 白色面板
        .textPrimary = { 30, 30, 30, 255 },      // 深色文字
        .textSecondary = { 120, 120, 120, 255 }, // 浅色文字
        .inputBg = { 255, 255, 255, 255 },       // 输入框背景
        .inputBorder = { 200, 200, 200, 255 },   // 输入框边框
        .error = { 220, 20, 60, 255 },           // 错误红色
        .success = { 50, 205, 50, 255 }          // 成功绿色
    };
}

/**
 * 获取深色主题
 * @return UITheme 结构
 */
UITheme UIThemeDark(void) {
    return (UITheme){
        .primary = { 70, 130, 180, 255 },        // 蓝色主色调
        .primaryHover = { 100, 149, 237, 255 },  // 悬停状态
        .primaryPressed = { 65, 105, 225, 255 }, // 按下状态
        .secondary = { 80, 80, 80, 255 },        // 深灰次要色
        .background = { 30, 30, 30, 255 },        // 深色背景
        .panelBg = { 45, 45, 45, 255 },          // 深灰面板
        .textPrimary = { 240, 240, 240, 255 },  // 浅色文字
        .textSecondary = { 180, 180, 180, 255 },// 灰色文字
        .inputBg = { 60, 60, 60, 255 },          // 输入框背景
        .inputBorder = { 100, 100, 100, 255 },   // 输入框边框
        .error = { 255, 80, 80, 255 },           // 错误红色
        .success = { 80, 200, 80, 255 }          // 成功绿色
    };
}

// ============================================================================
// 样式与字体管理
// ============================================================================

/**
 * 初始化 UI 样式为默认值
 * @param style 样式指针
 */
void UIStyleInit(UIStyle* style) {
    *style = (UIStyle){
        .theme = UIThemeLight(),       // 默认使用浅色主题
        .font = GetFontDefault(),      // 默认字体
        .fontSizeSmall = 16.0f,        // 小号字体
        .fontSizeNormal = 20.0f,        // 普通字体
        .fontSizeLarge = 28.0f,        // 大号字体
        .spacing = 8.0f,               // 默认间距
        .cornerRadius = 6              // 默认圆角
    };
}

/**
 * 加载自定义字体
 * @param style 样式指针
 * @param filename 字体文件路径
 */
void UILoadFont(UIStyle* style, const char* filename) {
    if (style->font.texture.id != GetFontDefault().texture.id) {
        UnloadFont(style->font);
    }
    style->font = LoadFont(filename);
    // 设置字体纹理过滤为双线性，提高渲染质量
    SetTextureFilter(style->font.texture, TEXTURE_FILTER_BILINEAR);
}

/**
 * 卸载自定义字体
 * @param style 样式指针
 */
void UIUnloadFont(UIStyle* style) {
    if (style->font.texture.id != GetFontDefault().texture.id) {
        UnloadFont(style->font);
        style->font = GetFontDefault();
    }
}

// ============================================================================
// UI 状态管理
// ============================================================================

/**
 * 开始一帧的 UI 更新
 * 更新鼠标和键盘状态
 * @param state UI 状态指针
 */
void UIBegin(UIState* state) {
    state->mousePos = GetMousePosition();
    state->mouseDown = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    state->mousePressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    state->mouseReleased = IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
    state->keyPressed = GetKeyPressed();
    state->charPressed = GetCharPressed();
    state->hotItem = 0;  // 重置悬停状态
}

/**
 * 结束一帧的 UI 更新
 * @param state UI 状态指针
 */
void UIEnd(UIState* state) {
    // 如果鼠标没有按下，清除活动状态
    if (!state->mouseDown) state->activeItem = 0;
}

/**
 * 根据标签生成唯一 ID
 * 用于区分同名按钮等控件
 * @param label 控件标签
 * @return 唯一 ID
 */
int UIGetID(const char* label) {
    int id = 0;
    while (*label) id = id * 31 + (*label++);
    return id;
}

// ============================================================================
// 布局系统
// ============================================================================

/**
 * 开始一个新的布局
 * @param container 容器区域
 * @param dir 布局方向（水平/垂直）
 * @param spacing 元素间距
 * @param padding 内边距
 * @return 布局结构
 */
UILayout UIBeginLayout(Rectangle container, UIDirection dir, float spacing, float padding) {
    UILayout layout = {
        .container = container,
        .direction = dir,
        .spacing = spacing,
        .padding = padding,
        .cursor = { padding, padding }  // 初始光标位置
    };
    return layout;
}

/**
 * 获取下一个布局区域
 * 会自动更新内部光标位置
 * @param layout 布局指针
 * @param width 元素宽度（-1表示填满剩余空间）
 * @param height 元素高度（-1表示填满剩余空间）
 * @return 下一个元素的位置和大小
 */
Rectangle UILayoutNext(UILayout* layout, float width, float height) {
    Rectangle rect = {0};
    float containerW = layout->container.width - 2 * layout->padding;
    float containerH = layout->container.height - 2 * layout->padding;

    if (layout->direction == UI_DIR_VERTICAL) {
        // 垂直布局
        if (width < 0) width = containerW;  // 默认宽度填满
        if (height < 0) height = containerH - layout->cursor.y;  // 默认高度填满剩余
        rect.x = layout->container.x + layout->padding + layout->cursor.x;
        rect.y = layout->container.y + layout->padding + layout->cursor.y;
        rect.width = width;
        rect.height = height;
        layout->cursor.y += height + layout->spacing;  // 移动光标
    } else {
        // 水平布局
        if (width < 0) width = containerW - layout->cursor.x;  // 默认宽度填满剩余
        if (height < 0) height = containerH;  // 默认高度填满
        rect.x = layout->container.x + layout->padding + layout->cursor.x;
        rect.y = layout->container.y + layout->padding + layout->cursor.y;
        rect.width = width;
        rect.height = height;
        layout->cursor.x += width + layout->spacing;  // 移动光标
    }
    return rect;
}

// ============================================================================
// 基础控件
// ============================================================================

/**
 * 绘制按钮（扩展版本，支持禁用状态）
 * @param label 按钮文字
 * @param rect 按钮区域
 * @param style 样式指针
 * @param state 状态指针
 * @param enabled 是否启用
 * @param id 唯一 ID
 * @return 按钮是否被点击
 */
bool UIButtonEx(const char* label, Rectangle rect, UIStyle* style, UIState* state, bool enabled, int id) {
    bool isInside = CheckCollisionPointRec(state->mousePos, rect);

    // 根据状态选择颜色
    Color bg = enabled ? style->theme.primary : style->theme.secondary;
    Color textColor = enabled ? style->theme.textPrimary : style->theme.textSecondary;

    // 处理悬停和按下状态
    if (enabled && isInside) {
        state->hotItem = id;
        if (state->mousePressed) state->activeItem = id;
    }

    if (state->activeItem == id && enabled) bg = style->theme.primaryPressed;  // 按下状态
    else if (state->hotItem == id && enabled) bg = style->theme.primaryHover;   // 悬停状态

    // 绘制按钮背景
    DrawRectangleRounded(rect, style->cornerRadius / rect.height * 10, 8, bg);
    DrawRectangleRoundedLines(rect, style->cornerRadius / rect.height * 10, 8, Fade(BLACK, 0.2f));

    // 绘制文字
    Vector2 textSize = MeasureTextEx(style->font, label, style->fontSizeNormal, 1);
    Vector2 textPos = {
        rect.x + (rect.width - textSize.x) / 2,
        rect.y + (rect.height - textSize.y) / 2
    };
    UIDrawText(label, textPos, style->fontSizeNormal, 1, textColor);

    // 检测点击
    bool clicked = false;
    if (enabled && state->mouseReleased && state->hotItem == id && state->activeItem == id) {
        clicked = true;
    }
    return clicked;
}

/**
 * 绘制按钮（普通版本，始终启用）
 * @param label 按钮文字
 * @param rect 按钮区域
 * @param style 样式指针
 * @param state 状态指针
 * @param id 唯一 ID
 * @return 按钮是否被点击
 */
bool UIButton(const char* label, Rectangle rect, UIStyle* style, UIState* state, int id) {
    return UIButtonEx(label, rect, style, state, true, id);
}

/**
 * 绘制复选框
 * @param label 标签文字
 * @param rect 复选框区域
 * @param checked 是否选中
 * @param style 样式指针
 * @param state 状态指针
 * @return 状态是否改变
 */
bool UICheckbox(const char* label, Rectangle rect, bool* checked, UIStyle* style, UIState* state) {
    int id = UIGetID(label);
    bool isInside = CheckCollisionPointRec(state->mousePos, rect);

    // 处理交互
    if (isInside) {
        state->hotItem = id;
        if (state->mousePressed) state->activeItem = id;
    }

    // 绘制背景
    Color bg = style->theme.inputBg;
    if (*checked) bg = style->theme.primary;

    DrawRectangleRounded(rect, style->cornerRadius / rect.height * 10, 8, bg);
    DrawRectangleRoundedLines(rect, style->cornerRadius / rect.height * 10, 8, style->theme.inputBorder);

    // 绘制勾选标记
    if (*checked) {
        Vector2 center = { rect.x + rect.width/2, rect.y + rect.height/2 };
        DrawLineEx((Vector2){ center.x - 6, center.y }, (Vector2){ center.x - 2, center.y + 5 }, 3, WHITE);
        DrawLineEx((Vector2){ center.x - 2, center.y + 5 }, (Vector2){ center.x + 6, center.y - 5 }, 3, WHITE);
    }

    // 绘制标签
    if (label && *label) {
        Vector2 textPos = { rect.x + rect.width + 8, rect.y + (rect.height - style->fontSizeNormal)/2 };
        UIDrawText(label, textPos, style->fontSizeNormal, 1, style->theme.textPrimary);
    }

    // 检测点击
    if (state->mouseReleased && state->hotItem == id && state->activeItem == id) {
        *checked = !*checked;
        return true;
    }
    return false;
}

/**
 * 绘制文本标签
 * @param text 文本内容
 * @param rect 标签区域
 * @param style 样式指针
 * @param color 文字颜色
 */
void UILabel(const char* text, Rectangle rect, UIStyle* style, Color color) {
    UIDrawTextRec(text, rect, style->fontSizeNormal, 1, true, color);
}

// ============================================================================
// 输入框
// ============================================================================

/**
 * 文本输入框
 * 支持 UTF-8 中文输入和退格键持续删除
 * @param state 输入框状态
 * @param rect 输入框区域
 * @param style 样式指针
 * @param uistate UI 状态指针
 * @param password 是否密码模式
 * @return 是否按下回车
 */
bool UITextBox(UITextBoxState* state, Rectangle rect, UIStyle* style, UIState* uistate, bool password) {
    // 使用 state 指针生成唯一ID
    int id = (int)(unsigned long)(void*)state;
    bool isInside = CheckCollisionPointRec(uistate->mousePos, rect);

    // 处理焦点 - 鼠标点击设置焦点
    if (uistate->mousePressed) {
        if (isInside) {
            uistate->focusItem = id;
            state->hasFocus = true;
            state->cursor = utf8_strlen(state->buffer);
        } else if (state->hasFocus) {
            // 如果当前输入框有焦点但鼠标没点中它，清除焦点
            // 注意：不能依赖 uistate->focusItem 比较，因为后续处理的
            // 输入框会覆盖 uistate->focusItem 的值
            state->hasFocus = false;
        }
    }
    
    // 更新悬停状态
    if (isInside) uistate->hotItem = id;

    // 键盘输入处理 - 只有获得焦点时才处理
    if (state->hasFocus) {
        int key = uistate->keyPressed;
        
        // 处理退格键
        if (key == KEY_BACKSPACE || IsKeyPressed(KEY_BACKSPACE)) {
            utf8_delete_left(state->buffer, &state->cursor);
        }
        
        // 处理其他键
        if (key == KEY_DELETE) {
            utf8_delete_right(state->buffer, &state->cursor);
        } else if (key == KEY_LEFT) {
            if (state->cursor > 0) state->cursor--;
        } else if (key == KEY_RIGHT) {
            int total = utf8_strlen(state->buffer);
            if (state->cursor < total) state->cursor++;
        } else if (uistate->charPressed >= 32) {
            utf8_insert_codepoint(state->buffer, sizeof(state->buffer), &state->cursor, uistate->charPressed);
        }
    }

    // 绘制输入框背景
    DrawRectangleRounded(rect, style->cornerRadius / rect.height * 10, 8, style->theme.inputBg);
    Color borderColor = state->hasFocus ? style->theme.primary : style->theme.inputBorder;
    DrawRectangleRoundedLines(rect, style->cornerRadius / rect.height * 10, 8, borderColor);

    // 处理密码模式
    const char* displayText = state->buffer;
    char passwordBuf[1024] = {0};
    if (password && *displayText) {
        int len = utf8_strlen(displayText);
        memset(passwordBuf, '*', len);
        passwordBuf[len] = '\0';
        displayText = passwordBuf;
    }

    // 绘制文本
    Rectangle textRect = { rect.x + 8, rect.y, rect.width - 16, rect.height };
    UIDrawTextRec(displayText, textRect, style->fontSizeNormal, 1, false, style->theme.textPrimary);

    // 绘制光标
    if (state->hasFocus) {
        int byte_offset = utf8_index_to_byte(state->buffer, state->cursor);
        char before[1024];
        strncpy(before, state->buffer, byte_offset);
        before[byte_offset] = '\0';
        Vector2 textSizeBefore = MeasureTextAuto(before, style->fontSizeNormal, 1);
        float cursorX = rect.x + 8 + textSizeBefore.x;
        DrawLineEx((Vector2){ cursorX, rect.y + 4 }, (Vector2){ cursorX, rect.y + rect.height - 4 }, 2, style->theme.textPrimary);
    }

    return (state->hasFocus && uistate->keyPressed == KEY_ENTER);
}

// ============================================================================
// 滚动视图与列表
// ============================================================================

/**
 * 开始滚动视图
 * @param scroll 滚动视图状态
 * @param viewport 可视区域
 * @param contentSize 内容实际大小
 */
void UIBeginScrollView(UIScrollView* scroll, Rectangle viewport, Vector2 contentSize) {
    scroll->viewport = viewport;
    scroll->contentSize = contentSize;
    scroll->showScrollbar = (contentSize.y > viewport.height);

    // 限制滚动范围
    if (scroll->scrollOffset.y < 0) scroll->scrollOffset.y = 0;
    float maxY = contentSize.y - viewport.height;
    if (maxY < 0) maxY = 0;
    if (scroll->scrollOffset.y > maxY) scroll->scrollOffset.y = maxY;

    // 开启裁剪模式
    BeginScissorMode((int)viewport.x, (int)viewport.y, (int)viewport.width, (int)viewport.height);
}

/**
 * 结束滚动视图
 * 处理鼠标滚轮和滚动条绘制
 * @param scroll 滚动视图状态
 * @param style 样式指针
 * @param state UI 状态指针
 */
void UIEndScrollView(UIScrollView* scroll, UIStyle* style, UIState* state) {
    EndScissorMode();

    // 处理鼠标滚轮
    if (CheckCollisionPointRec(state->mousePos, scroll->viewport)) {
        float wheel = GetMouseWheelMove();
        if (wheel != 0) scroll->scrollOffset.y -= wheel * 30;
    }

    // 再次限制滚动范围
    float maxY = scroll->contentSize.y - scroll->viewport.height;
    if (maxY < 0) maxY = 0;
    if (scroll->scrollOffset.y < 0) scroll->scrollOffset.y = 0;
    if (scroll->scrollOffset.y > maxY) scroll->scrollOffset.y = maxY;

    // 绘制滚动条
    if (scroll->showScrollbar) {
        float barHeight = scroll->viewport.height * (scroll->viewport.height / scroll->contentSize.y);
        float barY = scroll->viewport.y + (scroll->scrollOffset.y / scroll->contentSize.y) * scroll->viewport.height;
        DrawRectangleRec((Rectangle){ scroll->viewport.x + scroll->viewport.width - 8, barY, 6, barHeight },
                        Fade(style->theme.textSecondary, 0.5f));
    }
}

/**
 * 绘制列表项

// ============================================================================
// 持久化滚动视图（自动保存滚动位置）
// ============================================================================

void UIBeginPersistentScrollView(PersistentScrollView* psv, Rectangle viewport, Vector2 contentSize, float* offsetPtr) {
    psv->persistedOffset = offsetPtr;
    psv->sv.scrollOffset.y = *offsetPtr;
    UIBeginScrollView(&psv->sv, viewport, contentSize);
}

void UIEndPersistentScrollView(PersistentScrollView* psv, UIStyle* style, UIState* state) {
    UIEndScrollView(&psv->sv, style, state);
    *psv->persistedOffset = psv->sv.scrollOffset.y;
}
 * @param text 显示文字
 * @param itemRect 项的区域
 * @param style 样式指针
 * @param state UI 状态指针
 * @return 是否被点击
 */
bool UIListItem(const char* text, Rectangle itemRect, UIStyle* style, UIState* state) {
    int id = UIGetID(text);
    bool isInside = CheckCollisionPointRec(state->mousePos, itemRect);

    // 处理交互
    if (isInside) {
        state->hotItem = id;
        if (state->mousePressed) state->activeItem = id;
    }

    // 绘制背景
    Color bg = (state->activeItem == id) ? style->theme.primaryPressed :
               (state->hotItem == id) ? style->theme.primaryHover : BLANK;
    if (bg.a > 0) DrawRectangleRec(itemRect, bg);

    // 绘制文字
    Vector2 textPos = { itemRect.x + 8, itemRect.y + (itemRect.height - style->fontSizeNormal)/2 };
    UIDrawText(text, textPos, style->fontSizeNormal, 1, style->theme.textPrimary);

    return (state->mouseReleased && state->hotItem == id && state->activeItem == id);
}

// ============================================================================
// 单词卡片
// ============================================================================

/**
 * 绘制单词卡片
 * @param entry 单词条目
 * @param rect 卡片区域
 * @param style 样式指针
 */
void UIWordCard(WordEntry* entry, Rectangle rect, UIStyle* style) {
    DrawRectangleRounded(rect, 0.1f, 8, style->theme.panelBg);
    DrawRectangleRoundedLines(rect, 0.1f, 8, style->theme.inputBorder);

    // 使用垂直布局
    UILayout layout = UIBeginLayout(rect, UI_DIR_VERTICAL, 4, 12);

    // 单词
    Rectangle wordRect = UILayoutNext(&layout, -1, style->fontSizeLarge + 8);
    UIDrawText(entry->word, (Vector2){ wordRect.x, wordRect.y }, style->fontSizeLarge, 1, style->theme.textPrimary);

    // 音标
    if (entry->phonetic && *entry->phonetic) {
        Rectangle phoRect = UILayoutNext(&layout, -1, style->fontSizeNormal + 4);
        UIDrawText(entry->phonetic, (Vector2){ phoRect.x, phoRect.y }, style->fontSizeNormal, 1, style->theme.textSecondary);
    }

    // 分隔线
    Rectangle lineRect = UILayoutNext(&layout, -1, 2);
    DrawLineEx((Vector2){ lineRect.x, lineRect.y }, (Vector2){ lineRect.x + lineRect.width, lineRect.y }, 1, style->theme.inputBorder);

    // 释义
    Rectangle defRect = UILayoutNext(&layout, -1, style->fontSizeNormal * 2);
    UIDrawTextRec(entry->definition, defRect, style->fontSizeNormal, 1, true, style->theme.textPrimary);

    // 例句
    if (entry->example && *entry->example) {
        Rectangle exRect = UILayoutNext(&layout, -1, style->fontSizeNormal * 3);
        UIDrawTextRec(TextFormat("例: %s", entry->example), exRect, style->fontSizeNormal, 1, true, style->theme.textSecondary);
    }
}

// ============================================================================
// 闪卡
// ============================================================================

/**
 * 线性插值
 * @param a 起始值
 * @param b 结束值
 * @param t 插值因子（0-1）
 */
float Lerp(float a, float b, float t) { return a + (b - a) * t; }

/**
 * 绘制闪卡组件
 * 支持点击翻转显示释义
 * @param entry 单词条目
 * @param rect 卡片区域
 * @param face 当前显示的面
 * @param style 样式指针
 * @param state UI 状态指针
 * @param animTime 动画时间指针
 * @return 0=无操作，1=认识，2=不认识，3=翻转
 */
int UIFlashCard(WordEntry* entry, Rectangle rect, CardFace* face, UIStyle* style, UIState* state, float* animTime) {
    const float animSpeed = 5.0f;

    // 动画插值
    float target = (*face == CARD_FRONT) ? 0.0f : 1.0f;
    *animTime = Lerp(*animTime, target, GetFrameTime() * animSpeed);
    if (fabsf(*animTime - target) < 0.01f) *animTime = target;

    // 缩放动画效果
    float scale = 1.0f - fabsf(*animTime - 0.5f) * 0.2f;
    Rectangle drawRect = {
        rect.x + rect.width * (1 - scale) / 2,
        rect.y + rect.height * (1 - scale) / 2,
        rect.width * scale,
        rect.height * scale
    };

    // 绘制卡片背景
    DrawRectangleRounded(drawRect, 0.1f, 8, style->theme.panelBg);
    DrawRectangleRoundedLines(drawRect, 0.1f, 8, style->theme.inputBorder);

    // 判断显示哪一面（动画 < 0.5 为正面）
    bool showFront = (*animTime < 0.5f);

    if (showFront) {
        // 正面：单词居中显示
        float fontSize = style->fontSizeLarge * 1.2f;
        Vector2 wordSize = MeasureTextEx(style->font, entry->word, fontSize, 1);
        Vector2 wordPos = {
            drawRect.x + (drawRect.width - wordSize.x) / 2,
            drawRect.y + (drawRect.height - wordSize.y) / 2
        };
        UIDrawText(entry->word, wordPos, fontSize, 1, style->theme.textPrimary);
    } else {
        // 背面：释义和音标分上下显示
        float defFontSize = style->fontSizeLarge * 1.2f;
        float phoFontSize = style->fontSizeNormal;

        // 释义区域：卡片上半部分
        float defHeight = drawRect.height * 0.6f;
        Rectangle defRect = {
            drawRect.x + 15,
            drawRect.y + 10,
            drawRect.width - 30,
            defHeight - 20
        };
        UIDrawTextRec(entry->definition, defRect, defFontSize, 1, true, style->theme.textPrimary);

        // 音标区域：卡片下半部分
        if (entry->phonetic && *entry->phonetic) {
            Vector2 phoSize = MeasureTextEx(style->font, entry->phonetic, phoFontSize, 1);
            Vector2 phoPos = {
                drawRect.x + (drawRect.width - phoSize.x) / 2,
                drawRect.y + drawRect.height * 0.75f
            };
            UIDrawText(entry->phonetic, phoPos, phoFontSize, 1, style->theme.textSecondary);
        }
    }

    // 检测点击翻转
    int action = 0;
    bool isInside = CheckCollisionPointRec(state->mousePos, drawRect);
    if (isInside && state->mouseReleased) {
        *face = (*face == CARD_FRONT) ? CARD_BACK : CARD_FRONT;
        action = 3;
    }

    // 动画完成后显示按钮
    if (*animTime > 0.9f && *face == CARD_BACK) {
        // 绘制"认识"和"不认识"按钮
        Rectangle btnRow = { rect.x, rect.y + rect.height + 40, 220, 40 };
        if (UIButton("认识", btnRow, style, state, 100)) action = 1;
        btnRow.x = rect.x + rect.width - 220;
        if (UIButton("不认识", btnRow, style, state, 101)) action = 2;
    }

    return action;
}

// ============================================================================
// 选择题
// ============================================================================

/**
 * 绘制选择题组件
 * @param question 题干
 * @param options 选项数组
 * @param optionCount 选项数量
 * @param correctIndex 正确答案索引
 * @param rect 区域
 * @param style 样式指针
 * @param state UI 状态指针
 * @return 用户选择的选项索引，-1表示未选择
 */
int UIMultipleChoice(const char* question, const char* options[], int optionCount,
                     int correctIndex, Rectangle rect, UIStyle* style, UIState* state) {
    UILayout layout = UIBeginLayout(rect, UI_DIR_VERTICAL, 8, 8);

    // 绘制题干
    Rectangle qRect = UILayoutNext(&layout, -1, style->fontSizeNormal * 2);
    UIDrawTextRec(question, qRect, style->fontSizeNormal, 1, true, style->theme.textPrimary);

    int selected = -1;

    // 绘制选项
    for (int i = 0; i < optionCount; i++) {
        Rectangle optRect = UILayoutNext(&layout, -1, 40);
        bool isInside = CheckCollisionPointRec(state->mousePos, optRect);
        int id = UIGetID(options[i]);

        // 处理交互
        if (isInside) {
            state->hotItem = id;
            if (state->mousePressed) state->activeItem = id;
        }

        // 根据状态选择颜色
        Color bg = style->theme.panelBg;
        if (state->activeItem == id) bg = style->theme.primaryPressed;
        else if (state->hotItem == id) bg = style->theme.primaryHover;

        // 绘制选项背景
        DrawRectangleRounded(optRect, 0.2f, 8, bg);
        DrawRectangleRoundedLines(optRect, 0.2f, 8, style->theme.inputBorder);

        // 绘制选项标签
        char label[32];
        snprintf(label, sizeof(label), "%c. %s", 'A' + i, options[i]);
        Vector2 textPos = { optRect.x + 12, optRect.y + (optRect.height - style->fontSizeNormal)/2 };
        UIDrawText(label, textPos, style->fontSizeNormal, 1, style->theme.textPrimary);

        // 检测选择
        if (state->mouseReleased && state->hotItem == id && state->activeItem == id) selected = i;
    }

    return selected;
}

// ============================================================================
// 搜索栏
// ============================================================================

/**
 * 绘制搜索栏
 * @param sb 搜索栏状态
 * @param rect 区域
 * @param style 样式指针
 * @param state UI 状态指针
 */
void UISearchBar(SearchBarState* sb, Rectangle rect, UIStyle* style, UIState* state) {
    UILayout layout = UIBeginLayout(rect, UI_DIR_HORIZONTAL, 8, 0);
    Rectangle inputRect = UILayoutNext(&layout, rect.width - 70, rect.height);
    Rectangle btnRect = UILayoutNext(&layout, 60, rect.height);

    // 绘制输入框和按钮
    bool enter = UITextBox(&sb->textState, inputRect, style, state, false);
    bool clicked = UIButton("搜索", btnRect, style, state, 200);
    sb->searchTriggered = (enter || clicked);
}

// ============================================================================
// 单词列表视图
// ============================================================================

/**
 * 绘制可滚动的单词列表
 * @param words 单词数组
 * @param count 单词数量
 * @param selectedIndex 当前选中索引
 * @param scroll 滚动视图状态
 * @param style 样式指针
 * @param state UI 状态指针
 * @return 被点击的单词索引，-1表示无
 */
int UIWordListView(WordEntry* words, int count, int* selectedIndex,
                   UIScrollView* scroll, UIStyle* style, UIState* state) {
    int clickedIndex = -1;
    float itemHeight = 50.0f;

    // 设置内容大小并开始滚动视图
    scroll->contentSize = (Vector2){ scroll->viewport.width, itemHeight * count };
    UIBeginScrollView(scroll, scroll->viewport, scroll->contentSize);

    // 绘制列表项
    for (int i = 0; i < count; i++) {
        Rectangle itemRect = {
            scroll->viewport.x,
            scroll->viewport.y + i * itemHeight - scroll->scrollOffset.y,
            scroll->viewport.width,
            itemHeight
        };

        // 高亮选中项
        if (i == *selectedIndex) DrawRectangleRec(itemRect, Fade(style->theme.primary, 0.3f));

        // 绘制列表项
        if (UIListItem(words[i].word, itemRect, style, state)) {
            clickedIndex = i;
            *selectedIndex = i;
        }
    }

    UIEndScrollView(scroll, style, state);
    return clickedIndex;
}
