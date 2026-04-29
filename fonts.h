#ifndef FONTS_H
#define FONTS_H

#include "raylib.h"
#include <time.h>

// ============================================================================
// 字体变量（由 fonts.c 提供）
// ============================================================================

extern Font g_chineseFont;   // 中文字体（NotoSansCJK）
extern Font g_englishFont;   // 英文字体（DejaVu）
extern Font g_latinFont;     // IPA/音标字体
extern Font g_mergedFont;    // 合并字体（用于UI统一绘制）

// ============================================================================
// 函数声明
// ============================================================================

void loadFonts(void);                                    // 加载所有字体
void unloadFonts(void);                                  // 卸载字体

// 测量混合文本的宽度（自动识别中/英/IPA字符）
Vector2 MeasureTextAuto(const char* text, float fontSize, float spacing);

// 绘制单行文本（自动选择字体）
void DrawTextAuto(const char* text, Vector2 pos, float fontSize, float spacing, Color tint);

// 根据熟练度获取颜色（绿色>=80%，橙色>=50%，红色<50%）
Color getMasteryColor(float mastery);

// 格式化时间戳为 "月-日 时:分" 格式（0返回"从未"）
const char* formatTime(time_t timestamp);

#endif // FONTS_H
