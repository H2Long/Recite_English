// 字体管理 — 中/英/IPA/合并四种字体, 混合文本绘制

#ifndef FONTS_H
#define FONTS_H

#include "raylib.h"
#include <time.h>

extern Font g_chineseFont;
extern Font g_englishFont;
extern Font g_latinFont;
extern Font g_mergedFont;

void loadFonts(void);
void unloadFonts(void);

Vector2 MeasureTextAuto(const char* text, float fontSize, float spacing);
void DrawTextAuto(const char* text, Vector2 pos, float fontSize, float spacing, Color tint);

Color getMasteryColor(float mastery);
const char* formatTime(time_t timestamp);

#endif
