// ============================================================================
// 字体渲染模块头文件
// 功能：声明字体变量和混合文本绘制函数
//
// 四种字体：
//   g_chineseFont  — 中文字体（NotoSansCJK.otf），渲染全部 CJK 字符
//   g_englishFont  — 英文字体（DejaVu Sans），渲染 ASCII 字符
//   g_latinFont    — IPA 音标字体（IPA 日文字体/DejaVu），渲染 IPA 扩展字符
//   g_mergedFont   — 合并字体（基于中文字体合并 IPA+ASCII），用于 UI 统一绘制
//
// 混合文本函数：
//   DrawTextAuto()     — 绘制单行文本，自动为每个字符选择字体
//   MeasureTextAuto()  — 测量混合文本的渲染宽度
// ============================================================================

#ifndef FONTS_H
#define FONTS_H

#include "raylib.h"  /* Font / Vector2 / Color 类型 */
#include <time.h>    /* time_t */

// ============================================================================
// 字体变量（由 fonts.c 提供，外部可访问）
// ============================================================================

extern Font g_chineseFont;   /* 中文字体：NotoSansCJK.otf 或其他 */
extern Font g_englishFont;   /* 英文字体：DejaVu Sans 或其他 */
extern Font g_latinFont;     /* IPA 音标字体：IPA 日文/DejaVu 等 */
extern Font g_mergedFont;    /* 合并字体：中文 + IPA + ASCII，用于 UIDrawText() */

// ============================================================================
// 函数声明
// ============================================================================

void loadFonts(void);                                  /* 加载所有字体（按优先级尝试多个候选路径） */
void unloadFonts(void);                                /* 卸载字体 */

Vector2 MeasureTextAuto(const char* text, float fontSize, float spacing);  /* 测量混合文本宽度 */
void DrawTextAuto(const char* text, Vector2 pos, float fontSize, float spacing, Color tint);  /* 绘制混合文本 */

Color getMasteryColor(float mastery);                  /* 根据熟练度获取颜色（绿/橙/红） */
const char* formatTime(time_t timestamp);              /* 格式化时间戳（"月-日 时:分"或"从未"） */

#endif // FONTS_H
