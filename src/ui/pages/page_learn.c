// ============================================================================
// 学单词页面
// 功能：左侧可滚动的单词列表 + 右侧详情面板
// 布局：
//   ┌─────────────┬─────────────────────────────┐
//   │  ☐只显示未掌握│                             │
//   │ ─────────── │      单词（大号）             │
//   │  abandon    │      音标                    │
//   │  ability    │      ───────                 │
//   │  ...        │      释义                    │
//   │  (滚动列表)  │      例句                    │
//   │             │      ───────                 │
//   │             │      学习统计                 │
//   │             │   [上一个]   [下一个]          │
//   └─────────────┴─────────────────────────────┘
// ============================================================================

#include "pages.h"

/**
 * MenuLearn_Show - 渲染学单词页面
 *
 * 左侧列表每行左侧有彩色指示条（绿=已掌握，橙=学习中，红=待复习）。
 * 点击列表项切换右侧详情。选中项高亮显示。
 * "只显示未掌握"复选框过滤已掌握的单词。
 * 滚动位置通过 LEARN.learnScrollOffset 跨帧持久化。
 */
void MenuLearn_Show(void) {
    Rectangle contentRect = {250, 80, SCREEN_WIDTH - 270, SCREEN_HEIGHT - 100};
    UILayout layout = UIBeginLayout(contentRect, UI_DIR_HORIZONTAL, 25, 0);

    // ------------------------------------------------------------------
    // 左侧：单词列表（350px 宽）
    // ------------------------------------------------------------------
    Rectangle listArea = UILayoutNext(&layout, 350, -1);
    DrawRectangleRounded(listArea, 0.1f, 8, STYLE->theme.panelBg);

    // 过滤器复选框
    UICheckbox(u8"只显示未掌握",
        (Rectangle){listArea.x + 15, listArea.y + 15, 220, 35},
        &LEARN.learnFilterUnknown, STYLE, UI_STATE);

    // 滚动列表区域（从复选框下方开始）
    Rectangle scrollView = {listArea.x, listArea.y + 60,
                            listArea.width, listArea.height - 60};
    UIScrollView sv = {0};
    sv.scrollOffset.y = LEARN.learnScrollOffset;  // 恢复上次的滚动位置

    // 计算可见单词数（过滤后）
    int masteredCount = 0;
    for (int i = 0; i < g_wordProgressCount; i++)
        if (g_words[i].progress.mastered) masteredCount++;

    int visibleCount = LEARN.learnFilterUnknown ?
        (g_wordProgressCount - masteredCount) : g_wordProgressCount;

    // 渲染列表
    sv.contentSize = (Vector2){scrollView.width, visibleCount * 60.0f};
    UIBeginScrollView(&sv, scrollView, sv.contentSize);

    int listIndex = 0;
    for (int i = 0; i < g_wordProgressCount; i++) {
        if (LEARN.learnFilterUnknown && g_words[i].progress.mastered) continue;

        Rectangle itemRect = {scrollView.x,
            scrollView.y + listIndex * 60 - sv.scrollOffset.y,
            scrollView.width, 60};

        // 左侧熟练度色条：绿/橙/红
        Color indicatorColor = getMasteryColor(
            (float)g_words[i].progress.knownCount /
            (g_words[i].progress.knownCount + g_words[i].progress.unknownCount + 1));
        DrawRectangleRec((Rectangle){itemRect.x + 5, itemRect.y + 12,
                                      5, itemRect.height - 24}, indicatorColor);

        if (i == LEARN.learnIndex)
            DrawRectangleRec(itemRect, Fade(STYLE->theme.primary, 0.2f));
        if (UIListItem(g_words[i].entry.word, itemRect, STYLE, UI_STATE))
            LEARN.learnIndex = i;
        listIndex++;
    }
    UIEndScrollView(&sv, STYLE, UI_STATE);
    LEARN.learnScrollOffset = sv.scrollOffset.y;  // 保存滚动位置

    // ------------------------------------------------------------------
    // 右侧：单词详情（单词/音标/释义/例句/学习统计/导航按钮）
    // ------------------------------------------------------------------
    Rectangle detailArea = UILayoutNext(&layout, -1, -1);
    WordEntry* entry = &g_words[LEARN.learnIndex].entry;
    WordProgress* progress = &g_words[LEARN.learnIndex].progress;

    DrawRectangleRounded(detailArea, 0.1f, 12, STYLE->theme.panelBg);
    UILayout dl = UIBeginLayout(detailArea, UI_DIR_VERTICAL, 20, 30);

    // 单词
    Rectangle wr = UILayoutNext(&dl, -1, 70);
    DrawTextAuto(entry->word, (Vector2){wr.x, wr.y}, 56, 1, STYLE->theme.primary);

    // 音标
    if (entry->phonetic && *entry->phonetic) {
        Rectangle pr = UILayoutNext(&dl, -1, 45);
        DrawTextAuto(entry->phonetic, (Vector2){wr.x, pr.y}, 32, 1, STYLE->theme.textSecondary);
    }

    // 分隔线
    Rectangle lr = UILayoutNext(&dl, -1, 3);
    DrawLineEx((Vector2){lr.x, lr.y}, (Vector2){lr.x + lr.width, lr.y}, 1, STYLE->theme.inputBorder);

    // 释义
    Rectangle dlr = UILayoutNext(&dl, -1, 40);
    DrawTextAuto(u8"释义", (Vector2){dlr.x, dlr.y}, 28, 1, STYLE->theme.textSecondary);
    Rectangle dr = UILayoutNext(&dl, -1, 80);
    UIDrawTextRec(entry->definition, dr, 30, 1, true, STYLE->theme.textPrimary);

    // 例句
    Rectangle elr = UILayoutNext(&dl, -1, 40);
    DrawTextAuto(u8"例句", (Vector2){elr.x, elr.y}, 28, 1, STYLE->theme.textSecondary);
    if (entry->example && *entry->example) {
        Rectangle er = UILayoutNext(&dl, -1, 90);
        char exampleFull[512];
        snprintf(exampleFull, sizeof(exampleFull), u8"例: %s", entry->example);
        UIDrawTextRec(exampleFull, er, 26, 1, true, STYLE->theme.textSecondary);
    }

    // 学习状态统计
    Rectangle sr = UILayoutNext(&dl, -1, 90);
    DrawRectangleRounded(sr, 0.1f, 8, STYLE->theme.inputBg);
    UILayout sl = UIBeginLayout(sr, UI_DIR_HORIZONTAL, 40, 20);
    Rectangle si = UILayoutNext(&sl, -1, -1);
    char stateText[256];
    snprintf(stateText, sizeof(stateText),
        u8"认识: %d次  |  不认识: %d次  |  状态: %s  |  上次复习: %s",
        progress->knownCount, progress->unknownCount,
        progress->mastered ? u8"已掌握" : u8"学习中",
        formatTime(progress->lastReview));
    UIDrawTextRec(stateText, (Rectangle){si.x, si.y + 20, si.width, si.height},
                  22, 1, true, STYLE->theme.textPrimary);

    // 导航按钮
    Rectangle nr = UILayoutNext(&dl, -1, 60);
    UILayout nl = UIBeginLayout(nr, UI_DIR_HORIZONTAL, 25, 0);
    if (UIButton(u8"上一个", UILayoutNext(&nl, 120, 50), STYLE, UI_STATE, 1) && LEARN.learnIndex > 0)
        LEARN.learnIndex--;
    if (UIButton(u8"下一个", UILayoutNext(&nl, 120, 50), STYLE, UI_STATE, 2) && LEARN.learnIndex < g_wordProgressCount - 1)
        LEARN.learnIndex++;
}
