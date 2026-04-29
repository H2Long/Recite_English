#include "fonts.h"
#include "words.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// 字体
Font g_chineseFont;
Font g_englishFont;
Font g_latinFont;
Font g_mergedFont;

// ============================================================================
// 字体管理模块
// 功能：加载中英文/IPA字体，自动选择字体绘制混合文本
// ============================================================================

#include "fonts.h"
#include "words.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// 全局字体变量
Font g_chineseFont;    // 中文字体（NotoSansCJK）
Font g_englishFont;    // 英文字体（DejaVu）
Font g_latinFont;      // IPA/音标字体
Font g_mergedFont;     // 合并字体（中+IPA+ASCII，用于UI）

// 检查文件是否存在
static bool fileExists(const char *path) {
    FILE *f = fopen(path, "rb");
    if (f) { fclose(f); return true; }
    return false;
}

static void appendCodepoints(int **array, int *count, int *capacity, const int *src, int srcCount) {
    if (srcCount <= 0) return;
    if (*array == NULL) {
        *capacity = (srcCount > 16) ? srcCount : 16;
        *array = (int*)malloc(*capacity * sizeof(int));
        *count = 0;
    } else if (*count + srcCount > *capacity) {
        *capacity = (*count + srcCount) * 2;
        *array = (int*)realloc(*array, *capacity * sizeof(int));
    }
    memcpy(*array + *count, src, srcCount * sizeof(int));
    *count += srcCount;
}

// 从文本中提取中文字符到缓冲区
static void extractChineseChars(const char* text, char* buffer, int* bufLen, int maxLen) {
    for (int j = 0; text && text[j] && *bufLen < maxLen; j++) {
        unsigned char c = text[j];
        if (c >= 0x80) {
            int len = (c < 0xE0) ? 2 : (c < 0xF0) ? 3 : 4;
            bool found = false;
            for (int k = 0; k < *bufLen; k++) {
                bool match = true;
                for (int m = 0; m < len && k + m < *bufLen; m++) {
                    if (((unsigned char*)buffer)[k + m] != ((unsigned char*)text)[j + m]) {
                        match = false;
                        break;
                    }
                }
                if (match) { found = true; break; }
            }
            if (!found) {
                for (int m = 0; m < len && *bufLen < maxLen; m++) {
                    buffer[(*bufLen)++] = text[j + m];
                }
            }
            j += len - 1;
        }
    }
}

// 从单词释义和音标中提取中文字符
static void extractWordsChineseChars(char* buffer, int* bufLen, int maxLen) {
    for (int i = 0; i < g_wordCount && i < MAX_WORDS; i++) {
        extractChineseChars(g_words[i].entry.definition, buffer, bufLen, maxLen);
        extractChineseChars(g_words[i].entry.phonetic, buffer, bufLen, maxLen);
    }
}

// 检查字符是否是 IPA/Latin 扩展字符
static inline bool isLatinExtended(int c) {
    if (c >= 0x00A0 && c <= 0x00FF) return true;
    if (c >= 0x0250 && c <= 0x02AF) return true;
    if (c >= 0x0100 && c <= 0x017F) return true;
    if (c >= 0x0180 && c <= 0x024F) return true;
    if (c >= 0x1E00 && c <= 0x1EFF) return true;
    if (c >= 0x0370 && c <= 0x03FF) return true;
    if (c >= 0x1D00 && c <= 0x1D7F) return true;
    if (c >= 0x1D80 && c <= 0x1DBF) return true;
    if (c >= 0x02B0 && c <= 0x02FF) return true;
    if (c >= 0x0300 && c <= 0x036F) return true;
    return false;
}

// 加载字体
// 按顺序尝试加载：
// 1. 英文字体：DejaVu 系列
// 2. IPA 字体：日文字体 > Linux 字体 > Noto > DejaVu
// 3. 中文字体：NotoSansCJK > DroidSansFallback > 黑体
// 4. 合并字体：中文 + IPA + ASCII，用于统一绘制
void loadFonts(void) {
    g_mergedFont = GetFontDefault();
    g_latinFont = GetFontDefault();
    
    const char* englishCandidates[] = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
    };
    for (int i = 0; i < 3; i++) {
        if (fileExists(englishCandidates[i])) {
            int asciiGlyphs[127 - 32];
            for (int c = 32; c < 127; c++) asciiGlyphs[c - 32] = c;
            Font f = LoadFontEx(englishCandidates[i], 32, asciiGlyphs, 95);
            if (f.texture.id != 0) {
                g_englishFont = f;
                printf("INFO: English font loaded: %s\n", englishCandidates[i]);
                break;
            }
        }
    }
    if (g_englishFont.texture.id == 0) {
        g_englishFont = GetFontDefault();
    }
    
    const char* ipaCandidates[] = {
        "/usr/share/fonts/opentype/ipafont-mincho/ipam.ttf",
        "/usr/share/fonts/opentype/ipafont-mincho/ipamp.ttf",
        "/usr/share/fonts/opentype/ipaexfont-mincho/ipaexm.ttf",
        "/usr/share/fonts/opentype/ipafont-gothic/ipag.ttf",
        "/usr/share/fonts/opentype/ipafont-gothic/ipagp.ttf",
        "/usr/share/fonts/opentype/ipaexfont-gothic/ipaexg.ttf",
        "/usr/share/fonts/truetype/fonts-japanese-mincho.ttf",
        "/usr/share/fonts/truetype/fonts-japanese-gothic.ttf",
        "/usr/share/texmf/fonts/opentype/public/lm/lmroman10-regular.otf",
        "/usr/share/texmf/fonts/opentype/public/lm/lmroman12-regular.otf",
        "/usr/share/texmf/fonts/opentype/public/lm/lmroman17-regular.otf",
        "/usr/share/texmf/fonts/opentype/public/lm/lmromandunh10-regular.otf",
        "/usr/share/texmf/fonts/opentype/public/lm/lmromancaps10-regular.otf",
        "/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf",
        "/usr/share/fonts/truetype/noto/NotoSans-Bold.ttf",
        "/usr/share/fonts/truetype/noto/NotoSansMono-Regular.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSerif.ttf",
        "/usr/share/fonts/truetype/msttcorefonts/Arial.ttf",
        "/usr/share/fonts/truetype/msttcorefonts/Times_New_Roman.ttf",
    };
    int ipaCount = sizeof(ipaCandidates) / sizeof(ipaCandidates[0]);
    for (int i = 0; i < ipaCount; i++) {
        if (fileExists(ipaCandidates[i])) {
            int ipaGlyphs[2500];
            int ipaGlyphCount = 0;
            for (int c = 0x00A0; c <= 0x00FF && ipaGlyphCount < 100; c++) ipaGlyphs[ipaGlyphCount++] = c;
            for (int c = 0x0100; c <= 0x017F && ipaGlyphCount < 300; c++) ipaGlyphs[ipaGlyphCount++] = c;
            for (int c = 0x0180; c <= 0x024F && ipaGlyphCount < 500; c++) ipaGlyphs[ipaGlyphCount++] = c;
            for (int c = 0x0250; c <= 0x02AF && ipaGlyphCount < 700; c++) ipaGlyphs[ipaGlyphCount++] = c;
            for (int c = 0x02B0; c <= 0x02FF && ipaGlyphCount < 850; c++) ipaGlyphs[ipaGlyphCount++] = c;
            for (int c = 0x0300; c <= 0x036F && ipaGlyphCount < 1050; c++) ipaGlyphs[ipaGlyphCount++] = c;
            for (int c = 0x0370; c <= 0x03FF && ipaGlyphCount < 1250; c++) ipaGlyphs[ipaGlyphCount++] = c;
            for (int c = 0x1D00; c <= 0x1DBF && ipaGlyphCount < 1500; c++) ipaGlyphs[ipaGlyphCount++] = c;
            for (int c = 0x1E00; c <= 0x1EFF && ipaGlyphCount < 1700; c++) ipaGlyphs[ipaGlyphCount++] = c;
            for (int c = 0x1F00; c <= 0x1FFF && ipaGlyphCount < 2000; c++) ipaGlyphs[ipaGlyphCount++] = c;
            
            Font f = LoadFontEx(ipaCandidates[i], 32, ipaGlyphs, ipaGlyphCount);
            if (f.texture.id != 0 && f.recs != NULL) {
                g_latinFont = f;
                printf("INFO: IPA font loaded: %s with %d glyphs\n", ipaCandidates[i], ipaGlyphCount);
                break;
            }
        }
    }
    
    if (g_latinFont.texture.id == 0 || g_latinFont.recs == NULL) {
        for (int i = 0; i < 3; i++) {
            if (fileExists(englishCandidates[i])) {
                int ipaGlyphs[1300];
                int ipaGlyphCount = 0;
                for (int c = 0x0250; c <= 0x02AF && ipaGlyphCount < 100; c++) ipaGlyphs[ipaGlyphCount++] = c;
                for (int c = 0x02B0; c <= 0x02FF && ipaGlyphCount < 200; c++) ipaGlyphs[ipaGlyphCount++] = c;
                for (int c = 0x0300; c <= 0x036F && ipaGlyphCount < 400; c++) ipaGlyphs[ipaGlyphCount++] = c;
                for (int c = 0x0370; c <= 0x03FF && ipaGlyphCount < 600; c++) ipaGlyphs[ipaGlyphCount++] = c;
                for (int c = 0x1D00; c <= 0x1DBF && ipaGlyphCount < 900; c++) ipaGlyphs[ipaGlyphCount++] = c;
                for (int c = 0x1E00; c <= 0x1EFF && ipaGlyphCount < 1100; c++) ipaGlyphs[ipaGlyphCount++] = c;
                
                Font f = LoadFontEx(englishCandidates[i], 32, ipaGlyphs, ipaGlyphCount);
                if (f.texture.id != 0 && f.recs != NULL) {
                    g_latinFont = f;
                    printf("INFO: IPA font (DejaVu fallback) loaded: %s with %d glyphs\n", englishCandidates[i], ipaGlyphCount);
                    break;
                }
            }
        }
    }
    
    if (g_latinFont.texture.id == 0 || g_latinFont.recs == NULL) {
        g_latinFont = GetFontDefault();
        printf("WARNING: IPA font load failed, using default\n");
    }
    
    // 加载中文字体
    const char* candidates[] = {
        "./NotoSansCJK.otf",
        "/usr/share/fonts/truetype/droid/DroidSansFallbackFull.ttf",
        "./simhei.ttf",
        "./SimHei.ttf",
    };

    const char *fontPath = NULL;
    for (int i = 0; i < 4; i++) {
        if (fileExists(candidates[i])) {
            fontPath = candidates[i];
            break;
        }
    }

    g_chineseFont = GetFontDefault();
    if (fontPath != NULL) {
        const char* allChinese = 
            "主菜单学单词背单词测试进度开始上一个下一个认识不认识"
            "释义例句总单词数已掌握学习中认识次不认识次状态上次复习"
            "恭喜所有单词都已掌握再来一次下一题清除进度正确率建议先去学单词模式"
            "点击卡片翻转底部按钮标记认识或不认识本轮认识返回"
            "欢迎使用背单词软件选择左侧导航栏中的模式开始学习"
            "详细学习每个单词的释义和用法使用闪卡翻转记忆选择题检验学习成果"
            "只显示未掌握来一次卡片详细释义用法得分太棒继续保持不错接再厉需要多复习"
            "得分不错再来建议先去学模式刷记择题检验成果翻转底按钮标记本轮认主菜单"
            "设置主题当前浅色模式基于构建支持混合显示版本";

        char wordChars[8192] = {0};
        int wcLen = 0;
        extractWordsChineseChars(wordChars, &wcLen, 8000);

        int uiGlyphCount = 0;
        int *uiGlyphs = LoadCodepoints(allChinese, &uiGlyphCount);
        int wordGlyphCount = 0;
        int *wordGlyphs = LoadCodepoints(wordChars, &wordGlyphCount);
        
        int *glyphs = NULL;
        int glyphCount = 0;
        int glyphCapacity = 0;
        appendCodepoints(&glyphs, &glyphCount, &glyphCapacity, uiGlyphs, uiGlyphCount);
        appendCodepoints(&glyphs, &glyphCount, &glyphCapacity, wordGlyphs, wordGlyphCount);
        UnloadCodepoints(uiGlyphs);
        UnloadCodepoints(wordGlyphs);

        int asciiGlyphs[127 - 32];
        for (int c = 32; c < 127; c++) asciiGlyphs[c - 32] = c;
        appendCodepoints(&glyphs, &glyphCount, &glyphCapacity, asciiGlyphs, 127 - 32);

        Font tmp = LoadFontEx(fontPath, 32, glyphs, glyphCount);
        if (tmp.texture.id != 0) {
            g_chineseFont = tmp;
            printf("INFO: Chinese font loaded: %s with %d glyphs\n", fontPath, glyphCount);
        } else {
            printf("WARNING: Chinese font load failed: %s\n", fontPath);
        }
        free(glyphs);

        // 加载合并字体（中文 + IPA + ASCII）
        int mergeCount = 0;
        int *mergeGlyphs = LoadCodepoints(allChinese, &mergeCount);
        int wordGlyphsMergedCount = 0;
        int *wordGlyphsMerged = LoadCodepoints(wordChars, &wordGlyphsMergedCount);
        
        int *allGlyphs = NULL;
        int allCount = 0;
        int allCapacity = 0;
        appendCodepoints(&allGlyphs, &allCount, &allCapacity, mergeGlyphs, mergeCount);
        appendCodepoints(&allGlyphs, &allCount, &allCapacity, wordGlyphsMerged, wordGlyphsMergedCount);
        UnloadCodepoints(mergeGlyphs);
        UnloadCodepoints(wordGlyphsMerged);

        Font defaultFont = GetFontDefault();
        int defaultGlyphCount = defaultFont.glyphCount;
        
        int newCapacity = allCount + defaultGlyphCount + 2000;
        allGlyphs = (int*)realloc(allGlyphs, newCapacity * sizeof(int));
        
        for (int i = 0; i < defaultGlyphCount; i++) {
            bool exists = false;
            int cp = defaultFont.glyphs[i].value;
            for (int j = 0; j < allCount; j++) {
                if (allGlyphs[j] == cp) { exists = true; break; }
            }
            if (!exists) {
                allGlyphs[allCount++] = cp;
            }
        }
        
        int ipaRanges[][2] = {
            {0x00A0, 0x00FF}, {0x0100, 0x017F}, {0x0180, 0x024F},
            {0x0250, 0x02AF}, {0x02B0, 0x02FF}, {0x0300, 0x036F},
            {0x0370, 0x03FF}, {0x1D00, 0x1DBF}, {0x1D80, 0x1DBF},
            {0x1E00, 0x1EFF}, {0x2000, 0x206F}, {0x2070, 0x209F},
        };
        int ipaRangeCount = sizeof(ipaRanges) / sizeof(ipaRanges[0]);
        for (int r = 0; r < ipaRangeCount; r++) {
            for (int cp = ipaRanges[r][0]; cp <= ipaRanges[r][1]; cp++) {
                bool exists = false;
                for (int j = 0; j < allCount; j++) {
                    if (allGlyphs[j] == cp) { exists = true; break; }
                }
                if (!exists) {
                    allGlyphs[allCount++] = cp;
                }
            }
        }

        Font merged = LoadFontEx(fontPath, 32, allGlyphs, allCount);
        if (merged.texture.id != 0) {
            g_mergedFont = merged;
            printf("INFO: Merged font loaded with %d glyphs\n", allCount);
        } else {
            printf("WARNING: Merged font load failed, using English font\n");
            g_mergedFont = g_englishFont;
        }
        free(allGlyphs);
    }
}

// 卸载字体（程序退出时调用）
void unloadFonts(void) {
    if (g_chineseFont.texture.id != GetFontDefault().texture.id) {
        UnloadFont(g_chineseFont);
    }
    if (g_mergedFont.texture.id != GetFontDefault().texture.id && 
        g_mergedFont.texture.id != g_chineseFont.texture.id) {
        UnloadFont(g_mergedFont);
    }
}

// 获取 UTF-8 字符的字节长度
static int getUtf8Len(unsigned char byte) {
    if (byte < 0x80) return 1;
    if (byte < 0xE0) return 2;
    if (byte < 0xF0) return 3;
    return 4;
}

// 获取单个字符应该使用的字体
// ASCII字符 -> 英文字体
// IPA/Latin扩展字符 -> IPA字体
// 中文/其他 -> 合并字体
static Font getFontForChar(const char* text) {
    unsigned char byte = *(unsigned char*)text;
    if (byte < 0x80) return g_englishFont;
    
    int codepoint;
    if (byte < 0xE0) {
        codepoint = ((byte & 0x1F) << 6) | ((unsigned char)text[1] & 0x3F);
    } else if (byte < 0xF0) {
        codepoint = ((byte & 0x0F) << 12) | (((unsigned char)text[1] & 0x3F) << 6) | ((unsigned char)text[2] & 0x3F);
    } else {
        codepoint = ((byte & 0x07) << 18) | (((unsigned char)text[1] & 0x3F) << 12) | (((unsigned char)text[2] & 0x3F) << 6) | ((unsigned char)text[3] & 0x3F);
    }
    
    if (isLatinExtended(codepoint)) return g_latinFont;
    return g_mergedFont;
}

// 测量混合文本的宽度
// 自动识别中/英/IPA字符，使用对应字体测量
Vector2 MeasureTextAuto(const char* text, float fontSize, float spacing) {
    Vector2 result = {0, fontSize};
    if (!text || text[0] == '\0') return result;
    
    while (*text) {
        unsigned char uc = *(unsigned char*)text;
        if (uc == ' ' || uc == '\t') {
            result.x += MeasureTextEx(g_englishFont, " ", fontSize, spacing).x;
            text++;
            continue;
        }
        if (uc == '\n') break;
        
        Font currentFont = getFontForChar(text);
        int charLen = getUtf8Len(uc);
        
        char segment[5] = {0};
        for (int i = 0; i < charLen && *text; i++) {
            segment[i] = *text++;
        }
        Vector2 s = MeasureTextEx(currentFont, segment, fontSize, spacing);
        result.x += s.x;
    }
    return result;
}

// 绘制单行文本（自动选择字体）
// 根据字符类型选择字体：中文字符用合并字体，英文/IPA用对应字体
void DrawTextAuto(const char* text, Vector2 pos, float fontSize, float spacing, Color tint) {
    if (!text || text[0] == '\0') return;
    
    float x = pos.x;
    while (*text) {
        unsigned char uc = *(unsigned char*)text;
        
        if (uc == ' ' || uc == '\t') {
            x += MeasureTextEx(g_englishFont, " ", fontSize, spacing).x;
            text++;
            continue;
        }
        if (uc == '\n') break;

        Font currentFont = getFontForChar(text);
        int charLen = getUtf8Len(uc);
        
        char segment[5] = {0};
        for (int i = 0; i < charLen && *text; i++) {
            segment[i] = *text++;
        }

        DrawTextEx(currentFont, segment, (Vector2){ x, pos.y }, fontSize, spacing, tint);
        Vector2 s = MeasureTextEx(currentFont, segment, fontSize, spacing);
        x += s.x;
    }
}

// 获取记忆状态的颜色
// 熟练度 >= 80%: 绿色（已掌握）
// 熟练度 >= 50%: 橙色（学习中）
// 其他: 红色（待学习）
Color getMasteryColor(float mastery) {
    if (mastery >= 0.8f) return (Color){50, 205, 50, 255};
    if (mastery >= 0.5f) return (Color){255, 165, 0, 255};
    return (Color){220, 20, 60, 255};
}

// 格式化时间戳为可读字符串
// 0 表示"从未复习"，否则显示 "月-日 时:分"
const char* formatTime(time_t timestamp) {
    static char buffer[64];
    if (timestamp == 0) return "从未";
    struct tm *tm_info = localtime(&timestamp);
    strftime(buffer, sizeof(buffer), "%m-%d %H:%M", tm_info);
    return buffer;
}
