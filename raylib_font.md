# Raylib 中同时显示中文、英文和 IPA 音标

## 问题背景

在使用 Raylib 开发背单词软件时，遇到一个问题：卡片组件 `UIFlashCard` 点击后，英文单词和 IPA 音标显示为方块（豆腐块），只有中文释义正常显示。

数据显示为 `主主主` 而不是正常的文字。

## 问题分析

### 根本原因

Raylib 的 `DrawTextEx` 函数使用单一字体渲染文本。如果字体不包含某个字符的 glyph，该字符就会显示为方块或问号。

在背单词应用中，文本包含三种类型的字符：
1. **英文字母** (ASCII) - 如 `a-z, A-Z`
2. **中文** (CJK) - 如 `释义、认识`
3. **IPA 音标** (Latin Extended) - 如 `æ, ʃ, ʊ, ə, θ`

### 原始代码的问题

原始代码使用简单的 ASCII/非 ASCII 判断来选择字体：

```c
if (c < 0x80) {
    return englishFont;  // ASCII 使用英文字体
} else {
    return chineseFont;  // 非 ASCII 全部用中文字体
}
```

这个逻辑的问题是：**IPA 音标字符是多字节 UTF-8 字符（`c >= 0x80`），但它们不是中文字符**。把它们交给中文字体渲染，自然会显示为方块。

## 解决方案

### 核心思路

根据字符的 Unicode 码点范围，精确判断字符类型，选择最合适的字体：

```
ASCII (U+0000 - U+007F)           → 英文字体 (DejaVu Sans)
Latin Extended / IPA (U+0080+)    → IPA 专用字体 (ipam.ttf)
CJK / 中文 (U+4E00 - U+9FFF)      → 中文字体 (NotoSansCJK)
```

### 关键代码实现

#### 1. 字符分类函数

```c
// 检查字符是否是 IPA/Latin 扩展字符（需要用专用字体渲染）
static inline bool isLatinExtended(int c) {
    // Latin-1 Supplement: U+00A0-U+00FF (包含 æ, ð, ø, ß 等)
    if (c >= 0x00A0 && c <= 0x00FF) return true;
    // IPA Extensions: U+0250-U+02AF
    if (c >= 0x0250 && c <= 0x02AF) return true;
    // Latin Extended-A: U+0100-U+017F
    if (c >= 0x0100 && c <= 0x017F) return true;
    // Latin Extended-B: U+0180-U+024F
    if (c >= 0x0180 && c <= 0x024F) return true;
    // Latin Extended Additional: U+1E00-U+1EFF
    if (c >= 0x1E00 && c <= 0x1EFF) return true;
    // Greek and Coptic: U+0370-U+03FF (用于音标)
    if (c >= 0x0370 && c <= 0x03FF) return true;
    // Phonetic Extensions: U+1D00-U+1D7F
    if (c >= 0x1D00 && c <= 0x1D7F) return true;
    // Phonetic Extensions Supplement: U+1D80-U+1DBF
    if (c >= 0x1D80 && c <= 0x1DBF) return true;
    // Spacing Modifier Letters: U+02B0-U+02FF
    if (c >= 0x02B0 && c <= 0x02FF) return true;
    // Combining Diacritical Marks: U+0300-U+036F
    if (c >= 0x0300 && c <= 0x036F) return true;
    return false;
}
```

#### 2. UTF-8 解析函数

```c
// 解析 UTF-8 获取完整码点
static int utf8_to_codepoint(const unsigned char* text) {
    unsigned char byte = text[0];
    if (byte < 0x80) return byte;
    if (byte < 0xE0) return ((byte & 0x1F) << 6) | (text[1] & 0x3F);
    if (byte < 0xF0) return ((byte & 0x0F) << 12) | ((text[1] & 0x3F) << 6) | (text[2] & 0x3F);
    return ((byte & 0x07) << 18) | ((text[1] & 0x3F) << 12) | ((text[2] & 0x3F) << 6) | (text[3] & 0x3F);
}
```

#### 3. 根据字符选择字体

```c
// 获取单个字符应该使用的字体（根据字符类型）
static Font getFontForChar(const unsigned char* text) {
    unsigned char byte = text[0];
    if (byte < 0x80) return g_englishFont;  // ASCII → 英文字体

    // 解析 UTF-8 获取完整码点
    int codepoint = utf8_to_codepoint(text);

    if (isLatinExtended(codepoint)) {
        return g_latinFont;  // IPA/Latin 扩展 → IPA 专用字体
    }
    return g_mergedFont;  // CJK 和其他 → 合并字体
}
```

#### 4. 逐字符绘制函数

```c
// 绘制混合文本（自动为每个字符选择正确字体）
static void DrawTextAuto(const char* text, Vector2 pos, float fontSize, float spacing, Color tint) {
    if (!text || text[0] == '\0') return;

    float x = pos.x;
    while (*text) {
        unsigned char uc = *(unsigned char*)text;

        // 跳过空白
        if (uc == ' ' || uc == '\t') {
            x += MeasureTextEx(g_englishFont, " ", fontSize, spacing).x;
            text++;
            continue;
        }

        // 获取当前字符的字体
        Font currentFont = getFontForChar((const unsigned char*)text);

        // 计算字符长度并提取
        int charLen = (uc < 0x80) ? 1 : (uc < 0xE0) ? 2 : (uc < 0xF0) ? 3 : 4;
        char segment[5] = {0};
        for (int i = 0; i < charLen && *text; i++) {
            segment[i] = *text++;
        }

        // 绘制
        DrawTextEx(currentFont, segment, (Vector2){ x, pos.y }, fontSize, spacing, tint);
        Vector2 s = MeasureTextEx(currentFont, segment, fontSize, spacing);
        x += s.x;
    }
}
```

### 字体加载策略

#### 英文字体
加载 DejaVu Sans，只包含 ASCII 字符（32-126）：

```c
int asciiGlyphs[127 - 32];
for (int c = 32; c < 127; c++) asciiGlyphs[c - 32] = c;
Font englishFont = LoadFontEx("DejaVuSans.ttf", 32, asciiGlyphs, 95);
```

#### IPA 音标字体
专门加载支持 IPA 字符的字体（如 IPAMincho）：

```c
int ipaGlyphs[2500];
int ipaGlyphCount = 0;

// U+0250 - U+02AF: IPA Extensions
for (int c = 0x0250; c <= 0x02AF; c++) {
    ipaGlyphs[ipaGlyphCount++] = c;
}
// ... 其他 IPA 字符范围
Font latinFont = LoadFontEx("ipam.ttf", 32, ipaGlyphs, ipaGlyphCount);
```

#### 中文字体 + 扩展字符
加载包含中文和 Latin Extended 的合并字体：

```c
// 加载中文字体时，包含所有 IPA 字符范围
int allGlyphs[5000];
int allCount = 0;

// 添加所有 IPA 字符范围
int ipaRanges[][2] = {
    {0x00A0, 0x00FF},  // Latin-1 Supplement
    {0x0100, 0x017F},  // Latin Extended-A
    {0x0180, 0x024F},  // Latin Extended-B
    {0x0250, 0x02AF},  // IPA Extensions
    {0x02B0, 0x02FF},  // Spacing Modifier Letters
    {0x0370, 0x03FF},  // Greek and Coptic
    {0x1D00, 0x1DBF},  // Phonetic Extensions
    // ...
};
Font mergedFont = LoadFontEx("NotoSansCJK.otf", 32, allGlyphs, allCount);
```

## 架构设计

### 字体变量

```c
// main.c
Font g_chineseFont;     // 纯中文字体（用于回退）
Font g_englishFont;      // ASCII 英文字体
Font g_latinFont;        // IPA 音标专用字体
Font g_mergedFont;       // 合并字体（中文 + Latin Extended）

// raylib_word_ui.c (UI组件库)
Font g_chFont;           // 中文/合并字体
Font g_enFont;           // 英文字体
Font g_latinFont;        // IPA 音标字体
```

### 初始化流程

```c
// main.c
loadFonts();                                    // 加载所有字体
UISetFonts(g_mergedFont, g_englishFont);       // 设置UI组件库字体
UISetLatinFont(g_latinFont);                    // 设置UI组件库IPA字体
```

## 常见问题

### Q: 为什么 IPA 字符显示为方块？

**A:** 原因可能是：
1. 使用了错误的字体（把 IPA 字符交给中文字体渲染）
2. 字体文件本身不包含这些字符
3. 字体加载时没有请求这些字符的 glyph

**解决方法：** 确保 IPA 字符使用专门加载了 IPA 字符的字体（如 ipam.ttf）。

### Q: 如何调试字体问题？

**A:** 在代码中添加字体加载信息输出：

```c
printf("INFO: English font loaded: %s with %d glyphs\n",
       fontPath, englishFont.glyphCount);
printf("INFO: IPA font loaded: %s with %d glyphs\n",
       ipaFontPath, ipaFont.glyphCount);
```

### Q: 哪些字符范围属于 IPA 音标？

**A:** 主要的 IPA 字符范围包括：

| 范围 | 名称 | 示例字符 |
|------|------|----------|
| U+00A0-U+00FF | Latin-1 Supplement | æ, ð, ø, ß |
| U+0250-U+02AF | IPA Extensions | ɑ, ə, ʃ, ʊ, θ |
| U+02B0-U+02FF | Spacing Modifier Letters | ʰ, ʷ, ː |
| U+0370-U+03FF | Greek (音标用) | θ, χ, φ |
| U+1D00-U+1DBF | Phonetic Extensions | ᴄ, ᴅ, ꜰ |

## 总结

Raylib 中同时显示中文、英文和 IPA 音标的关键是：

1. **精确分类字符**：不能简单用 ASCII/非 ASCII 判断
2. **选择合适字体**：每种字符使用支持它的专用字体
3. **逐字符渲染**：混合文本需要逐字符选择字体并绘制
4. **正确加载 glyph**：确保字体加载时包含了所有需要的字符
