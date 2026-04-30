// ============================================================================
// 菜单回调函数实现
// 功能：各菜单页面的显示逻辑
// 重构后：使用 AppState 访问器获取状态，不再直接访问散落的全局变量
// ============================================================================

#include "menu_callbacks.h"
#include <ctype.h>

// 访问 words.c 中的单词库（g_wordLibrary 在 words.c 中定义）
extern WordEntry* g_wordLibrary;

// ============================================================================
// 宏定义：简化访问器调用
// ============================================================================

#define STYLE    (AppState_GetStyle())
#define UI_STATE (AppState_GetUIState())
#define CURRENT_MENU (*AppState_GetCurrentMenu())
#define MENU_STACK   (AppState_GetMenuStack())
#define LEARN    (*AppState_GetLearnState())
#define REVIEW   (*AppState_GetReviewState())
#define TEST     (*AppState_GetTestState())
#define SEARCH   (*AppState_GetSearchState())
#define ACCOUNT  (*AppState_GetAccountState())
#define SELECT_WORD (*AppState_GetSelectWordState())
#define PLAN_STATE (*AppState_GetPlanState())

// 账号菜单节点（供 main.c 右上角点击跳转使用）
MENU* g_accountMenuNode = NULL;
#define LOGIN_MSG (AppState_GetLoginMsg())

// ============================================================================
// 主菜单显示
// 包含欢迎信息、统计信息和功能卡片入口
// ============================================================================
void MenuHome_Show(void) {
    // 内容区域（右侧主区域）
    Rectangle contentRect = {250, 80, SCREEN_WIDTH - 270, SCREEN_HEIGHT - 100};
    UILayout layout = UIBeginLayout(contentRect, UI_DIR_VERTICAL, 30, 50);

    // --------------------------------------------------------------------
    // 欢迎信息区域
    // --------------------------------------------------------------------
    Rectangle welcomeRect = UILayoutNext(&layout, -1, 90);
    const char* welcome = u8"欢迎使用背单词软件！";
    Vector2 wSize = MeasureTextAuto(welcome, 52, 1);
    DrawTextAuto(welcome, (Vector2){SCREEN_WIDTH/2 - wSize.x/2, welcomeRect.y}, 52, 1, STYLE->theme.primary);

    // 副标题
    Rectangle descRect = UILayoutNext(&layout, -1, 60);
    const char* desc = u8"选择左侧导航栏中的模式开始学习";
    Vector2 dSize = MeasureTextAuto(desc, 28, 1);
    DrawTextAuto(desc, (Vector2){SCREEN_WIDTH/2 - dSize.x/2, descRect.y}, 28, 1, STYLE->theme.textSecondary);

    // --------------------------------------------------------------------
    // 统计信息卡片
    // --------------------------------------------------------------------
    Rectangle statsRect = UILayoutNext(&layout, -1, 100);
    DrawRectangleRounded(statsRect, 0.1f, 12, STYLE->theme.panelBg);

    char statsText[256];
    int mastered = 0;
    // 统计已掌握的单词数
    for (int i = 0; i < g_wordProgressCount; i++) {
        if (g_words[i].progress.mastered) mastered++;
    }
    snprintf(statsText, sizeof(statsText),
        u8"总单词数: %d  |  已掌握: %d  |  学习中: %d",
        g_wordProgressCount, mastered, g_wordProgressCount - mastered);

    // 绘制统计文本
    UILayout statsLayout = UIBeginLayout(statsRect, UI_DIR_HORIZONTAL, 0, 25);
    Rectangle statsTextRect = UILayoutNext(&statsLayout, -1, -1);
    DrawTextAuto(statsText, (Vector2){statsTextRect.x, statsTextRect.y + 28}, 22, 1, STYLE->theme.textPrimary);

    // --------------------------------------------------------------------
    // 功能卡片（学单词、背单词、查找单词）
    // --------------------------------------------------------------------
    Rectangle cardsRect = UILayoutNext(&layout, -1, 160);
    UILayout cardsLayout = UIBeginLayout(cardsRect, UI_DIR_HORIZONTAL, 30, 0);

    // 模式配置
    struct { const char* title; const char* desc; MENU* target; Color color; } modes[] = {
        {u8"学单词", u8"详细学习每个单词的释义和用法", NULL, (Color){70, 130, 180, 255}},
        {u8"背单词", u8"包含卡片背单词、选词背单词、测试三种模式", NULL, (Color){60, 179, 113, 255}},
        {u8"查找单词", u8"正则表达式快速查找", NULL, (Color){138, 43, 226, 255}}
    };

    // 关联菜单节点
    for (int i = 0; i < g_app.rootMenu->childindex && i < 3; i++) {
        modes[i].target = g_app.rootMenu->child[i];
    }

    // 绘制卡片
    for (int i = 0; i < 3; i++) {
        Rectangle cardRect = UILayoutNext(&cardsLayout, 240, -1);

        // 卡片背景和边框
        DrawRectangleRounded(cardRect, 0.1f, 12, STYLE->theme.panelBg);
        DrawRectangleRoundedLines(cardRect, 0.1f, 12, modes[i].color);

        // 标题
        Vector2 titleS = MeasureTextAuto(modes[i].title, 32, 1);
        DrawTextAuto(modes[i].title, (Vector2){cardRect.x + 15, cardRect.y + 15}, 32, 1, modes[i].color);

        // 描述
        Rectangle descArea = {cardRect.x + 15, cardRect.y + 55, cardRect.width - 30, cardRect.height - 80};
        UIDrawTextRec(modes[i].desc, descArea, 18, 1, true, STYLE->theme.textSecondary);

        // "开始"按钮
        Rectangle goBtn = {cardRect.x + cardRect.width - 100, cardRect.y + cardRect.height - 55, 85, 45};
        // 使用唯一 ID (100+i) 避免按钮状态冲突
        if (UIButton(u8"开始", goBtn, STYLE, UI_STATE, 100 + i) && modes[i].target != NULL) {
            StackPush(AppState_GetMenuStack(), modes[i].target);
            CURRENT_MENU = modes[i].target;
        }
    }
}

// ============================================================================
// 学单词模式
// 左侧：可滚动的单词列表
// 右侧：选中单词的详细信息
// ============================================================================
void MenuLearn_Show(void) {
    Rectangle contentRect = {250, 80, SCREEN_WIDTH - 270, SCREEN_HEIGHT - 100};
    UILayout layout = UIBeginLayout(contentRect, UI_DIR_HORIZONTAL, 25, 0);

    // --------------------------------------------------------------------
    // 左侧：单词列表
    // --------------------------------------------------------------------
    Rectangle listArea = UILayoutNext(&layout, 350, -1);
    DrawRectangleRounded(listArea, 0.1f, 8, STYLE->theme.panelBg);

    // 过滤器复选框
    Rectangle filterRect = {listArea.x + 15, listArea.y + 15, 220, 35};
    UICheckbox(u8"只显示未掌握", filterRect, &LEARN.learnFilterUnknown, STYLE, UI_STATE);

    // 滚动视图区域
    Rectangle scrollView = {listArea.x, listArea.y + 60, listArea.width, listArea.height - 60};
    UIScrollView sv = {0};
    // 使用状态变量保存滚动位置，避免每次重绘时跳回顶部
    sv.scrollOffset.y = LEARN.learnScrollOffset;

    // 计算可见单词数量
    int masteredCount = 0;
    for (int i = 0; i < g_wordProgressCount; i++) {
        if (g_words[i].progress.mastered) masteredCount++;
    }
    int visibleCount = LEARN.learnFilterUnknown ?
        (g_wordProgressCount - masteredCount) : g_wordProgressCount;

    // 设置内容大小并开始滚动视图
    sv.contentSize = (Vector2){scrollView.width, visibleCount * 60.0f};
    UIBeginScrollView(&sv, scrollView, sv.contentSize);

    // 绘制列表项
    int listIndex = 0;
    for (int i = 0; i < g_wordProgressCount; i++) {
        // 过滤器：跳过已掌握的单词
        if (LEARN.learnFilterUnknown && g_words[i].progress.mastered) continue;

        // 计算列表项位置（考虑滚动偏移）
        Rectangle itemRect = {scrollView.x, scrollView.y + listIndex * 60 - sv.scrollOffset.y, scrollView.width, 60};

        // 熟练度指示条
        Color indicatorColor = getMasteryColor(
            (float)g_words[i].progress.knownCount /
            (g_words[i].progress.knownCount + g_words[i].progress.unknownCount + 1)
        );
        DrawRectangleRec((Rectangle){itemRect.x + 5, itemRect.y + 12, 5, itemRect.height - 24}, indicatorColor);

        // 高亮当前选中项
        if (i == LEARN.learnIndex) {
            DrawRectangleRec(itemRect, Fade(STYLE->theme.primary, 0.2f));
        }

        // 列表项点击处理
        if (UIListItem(g_words[i].entry.word, itemRect, STYLE, UI_STATE)) {
            LEARN.learnIndex = i;
        }
        listIndex++;
    }

    // 结束滚动视图
    UIEndScrollView(&sv, STYLE, UI_STATE);
    // 保存滚动位置
    LEARN.learnScrollOffset = sv.scrollOffset.y;

    // --------------------------------------------------------------------
    // 右侧：单词详情
    // --------------------------------------------------------------------
    Rectangle detailArea = UILayoutNext(&layout, -1, -1);
    WordEntry* entry = &g_words[LEARN.learnIndex].entry;
    WordProgress* progress = &g_words[LEARN.learnIndex].progress;

    DrawRectangleRounded(detailArea, 0.1f, 12, STYLE->theme.panelBg);

    // 使用垂直布局组织内容
    UILayout detailLayout = UIBeginLayout(detailArea, UI_DIR_VERTICAL, 20, 30);

    // 单词
    Rectangle wordRect = UILayoutNext(&detailLayout, -1, 70);
    DrawTextAuto(entry->word, (Vector2){wordRect.x, wordRect.y}, 56, 1, STYLE->theme.primary);

    // 音标
    if (entry->phonetic && *entry->phonetic) {
        Rectangle phoRect = UILayoutNext(&detailLayout, -1, 45);
        DrawTextAuto(entry->phonetic, (Vector2){wordRect.x, phoRect.y}, 32, 1, STYLE->theme.textSecondary);
    }

    // 分隔线
    Rectangle lineRect = UILayoutNext(&detailLayout, -1, 3);
    DrawLineEx((Vector2){lineRect.x, lineRect.y}, (Vector2){lineRect.x + lineRect.width, lineRect.y}, 1, STYLE->theme.inputBorder);

    // 释义标签
    Rectangle defLabelRect = UILayoutNext(&detailLayout, -1, 40);
    DrawTextAuto(u8"释义", (Vector2){defLabelRect.x, defLabelRect.y}, 28, 1, STYLE->theme.textSecondary);

    // 释义内容
    Rectangle defRect = UILayoutNext(&detailLayout, -1, 80);
    UIDrawTextRec(entry->definition, defRect, 30, 1, true, STYLE->theme.textPrimary);

    // 例句标签
    Rectangle exLabelRect = UILayoutNext(&detailLayout, -1, 40);
    DrawTextAuto(u8"例句", (Vector2){exLabelRect.x, exLabelRect.y}, 28, 1, STYLE->theme.textSecondary);

    // 例句内容
    if (entry->example && *entry->example) {
        Rectangle exRect = UILayoutNext(&detailLayout, -1, 90);
        char exampleFull[512];
        snprintf(exampleFull, sizeof(exampleFull), u8"例: %s", entry->example);
        UIDrawTextRec(exampleFull, exRect, 26, 1, true, STYLE->theme.textSecondary);
    }

    // 学习状态统计
    Rectangle stateRect = UILayoutNext(&detailLayout, -1, 90);
    DrawRectangleRounded(stateRect, 0.1f, 8, STYLE->theme.inputBg);

    UILayout stateLayout = UIBeginLayout(stateRect, UI_DIR_HORIZONTAL, 40, 20);
    Rectangle stateItem = UILayoutNext(&stateLayout, -1, -1);

    char stateText[256];
    snprintf(stateText, sizeof(stateText),
        u8"认识: %d次  |  不认识: %d次  |  状态: %s  |  上次复习: %s",
        progress->knownCount, progress->unknownCount,
        progress->mastered ? u8"已掌握" : u8"学习中",
        formatTime(progress->lastReview));
    UIDrawTextRec(stateText, (Rectangle){stateItem.x, stateItem.y + 20, stateItem.width, stateItem.height}, 22, 1, true, STYLE->theme.textPrimary);

    // 导航按钮
    Rectangle navRect = UILayoutNext(&detailLayout, -1, 60);
    UILayout navLayout = UIBeginLayout(navRect, UI_DIR_HORIZONTAL, 25, 0);

    Rectangle prevBtn = UILayoutNext(&navLayout, 120, 50);
    Rectangle nextBtn = UILayoutNext(&navLayout, 120, 50);

    if (UIButton(u8"上一个", prevBtn, STYLE, UI_STATE, 1) && LEARN.learnIndex > 0) {
        LEARN.learnIndex--;
    }
    if (UIButton(u8"下一个", nextBtn, STYLE, UI_STATE, 2) && LEARN.learnIndex < g_wordProgressCount - 1) {
        LEARN.learnIndex++;
    }
}

// ============================================================================
// 背单词模式（父页面 - 显示三个子模式入口）
// ============================================================================
void MenuReviewRoot_Show(void) {
    Rectangle contentRect = {250, 80, SCREEN_WIDTH - 270, SCREEN_HEIGHT - 100};
    UILayout layout = UIBeginLayout(contentRect, UI_DIR_VERTICAL, 30, 50);

    // 页面标题
    Rectangle titleRect = UILayoutNext(&layout, -1, 60);
    const char* title = u8"背单词";
    Vector2 tSize = MeasureTextAuto(title, 42, 1);
    DrawTextAuto(title, (Vector2){contentRect.x + 50, titleRect.y}, 42, 1, STYLE->theme.primary);

    // 说明文字
    Rectangle descRect = UILayoutNext(&layout, -1, 50);
    const char* desc = u8"选择一种背单词模式开始学习";
    Vector2 dSize = MeasureTextAuto(desc, 24, 1);
    DrawTextAuto(desc, (Vector2){contentRect.x + 50, descRect.y}, 24, 1, STYLE->theme.textSecondary);

    // 三个模式卡片
    Rectangle cardsRect = UILayoutNext(&layout, -1, 300);
    UILayout cardsLayout = UIBeginLayout(cardsRect, UI_DIR_HORIZONTAL, 30, 0);

    struct { const char* title; const char* desc; Color color; } modes[] = {
        {u8"卡片背单词", u8"闪卡翻转记忆\n正面显示单词\n背面显示释义", (Color){60, 179, 113, 255}},
        {u8"选词背单词", u8"看释义选单词\n四选一选择题\n得分按账号统计", (Color){70, 130, 180, 255}},
        {u8"测试模式", u8"看单词选释义\n四选一选择题\n完成后显示正确率", (Color){255, 140, 0, 255}},
    };

    for (int i = 0; i < 3; i++) {
        Rectangle cardRect = UILayoutNext(&cardsLayout, 320, -1);
        DrawRectangleRounded(cardRect, 0.1f, 12, STYLE->theme.panelBg);
        DrawRectangleRoundedLines(cardRect, 0.1f, 12, modes[i].color);

        // 标题
        Vector2 titleS = MeasureTextAuto(modes[i].title, 32, 1);
        DrawTextAuto(modes[i].title, (Vector2){cardRect.x + 20, cardRect.y + 20}, 32, 1, modes[i].color);

        // 描述
        Rectangle descArea = {cardRect.x + 20, cardRect.y + 70, cardRect.width - 40, cardRect.height - 130};
        UIDrawTextRec(modes[i].desc, descArea, 22, 1, true, STYLE->theme.textSecondary);

        // 进入按钮
        Rectangle goBtn = {cardRect.x + cardRect.width / 2 - 60, cardRect.y + cardRect.height - 70, 120, 50};
        if (UIButton(modes[i].title, goBtn, STYLE, UI_STATE, 500 + i)) {
            if (i < g_app.currentMenu->childindex) {
                MENU* target = g_app.currentMenu->child[i];
                if (target != NULL) {
                    StackPush(AppState_GetMenuStack(), g_app.currentMenu);
                    CURRENT_MENU = target;
                }
            }
        }
    }
}

// ============================================================================
// 卡片背单词模式（原闪卡复习）
// 使用闪卡翻转学习：正面显示单词，背面显示释义
// 认识3次后标记为"已掌握"
// ============================================================================
void MenuCardReview_Show(void) {
    // 所有单词都已掌握
    if (REVIEW.reviewCount == 0) {
        Rectangle msgRect = {SCREEN_WIDTH/2 - 250, SCREEN_HEIGHT/2 - 60, 500, 120};
        DrawRectangleRounded(msgRect, 0.1f, 12, STYLE->theme.panelBg);
        const char* msg = u8"恭喜！所有单词都已掌握！";
        Vector2 mSize = MeasureTextAuto(msg, 34, 1);
        DrawTextAuto(msg, (Vector2){SCREEN_WIDTH/2 - mSize.x/2, SCREEN_HEIGHT/2 - 20}, 34, 1, STYLE->theme.success);
    } else {
        // --------------------------------------------------------------------
        // 进度条
        // --------------------------------------------------------------------
        Rectangle progressBar = {SCREEN_WIDTH/2 - 250, 100, 500, 30};
        DrawRectangleRounded(progressBar, 0.2f, 5, STYLE->theme.inputBg);
        float progress = (float)REVIEW.currentReviewIdx / REVIEW.reviewCount;
        DrawRectangleRounded((Rectangle){progressBar.x, progressBar.y, progressBar.width * progress, progressBar.height}, 0.2f, 5, STYLE->theme.primary);

        // 进度文字
        char progressText[64];
        snprintf(progressText, sizeof(progressText), u8"%d / %d", REVIEW.currentReviewIdx + 1, REVIEW.reviewCount);
        Vector2 pSize = MeasureTextAuto(progressText, 22, 1);
        DrawTextAuto(progressText, (Vector2){SCREEN_WIDTH/2 - pSize.x/2, progressBar.y + 35}, 22, 1, STYLE->theme.textSecondary);

        // --------------------------------------------------------------------
        // 闪卡
        // --------------------------------------------------------------------
        Rectangle cardRect = {SCREEN_WIDTH/2 - 220, 160, 440, 264};
        int action = UIFlashCard(&g_words[REVIEW.reviewIndices[REVIEW.currentReviewIdx]].entry, cardRect,
                                &REVIEW.flashcardFace, STYLE, UI_STATE, &REVIEW.flashcardAnimTime);

        // 处理"认识"按钮
        if (action == 1) {
            g_words[REVIEW.reviewIndices[REVIEW.currentReviewIdx]].progress.knownCount++;
            g_words[REVIEW.reviewIndices[REVIEW.currentReviewIdx]].progress.lastReview = time(NULL);
            // 认识3次后标记为已掌握
            if (g_words[REVIEW.reviewIndices[REVIEW.currentReviewIdx]].progress.knownCount >= 3) {
                g_words[REVIEW.reviewIndices[REVIEW.currentReviewIdx]].progress.mastered = true;
            }
            REVIEW.knownInSession++;
            // 重置卡片到正面
            REVIEW.flashcardFace = CARD_FRONT;
            REVIEW.flashcardAnimTime = 0.0f;
            // 移动到下一个单词
            REVIEW.currentReviewIdx++;
            if (REVIEW.currentReviewIdx >= REVIEW.reviewCount) {
                REVIEW.currentReviewIdx = REVIEW.reviewCount - 1;
            }
            saveProgress();
        }
        // 处理"不认识"按钮
        else if (action == 2) {
            g_words[REVIEW.reviewIndices[REVIEW.currentReviewIdx]].progress.unknownCount++;
            g_words[REVIEW.reviewIndices[REVIEW.currentReviewIdx]].progress.lastReview = time(NULL);
            // 重置掌握状态
            g_words[REVIEW.reviewIndices[REVIEW.currentReviewIdx]].progress.mastered = false;
            REVIEW.unknownInSession++;
            // 重置卡片到正面
            REVIEW.flashcardFace = CARD_FRONT;
            REVIEW.flashcardAnimTime = 0.0f;
            // 移动到下一个单词
            REVIEW.currentReviewIdx++;
            if (REVIEW.currentReviewIdx >= REVIEW.reviewCount) {
                REVIEW.currentReviewIdx = REVIEW.reviewCount - 1;
            }
            saveProgress();
        }

        // --------------------------------------------------------------------
        // 本轮统计
        // --------------------------------------------------------------------
        Rectangle statsRect = {SCREEN_WIDTH/2 - 180, 470, 360, 65};
        DrawRectangleRounded(statsRect, 0.1f, 8, STYLE->theme.panelBg);
        char sessionText[128];
        snprintf(sessionText, sizeof(sessionText), u8"本轮认识: %d  |  不认识: %d", REVIEW.knownInSession, REVIEW.unknownInSession);
        Vector2 sSize = MeasureTextAuto(sessionText, 26, 1);
        DrawTextAuto(sessionText, (Vector2){SCREEN_WIDTH/2 - sSize.x/2, statsRect.y + 18}, 26, 1, STYLE->theme.textPrimary);

        // --------------------------------------------------------------------
        // 提示信息
        // --------------------------------------------------------------------
        Rectangle tipRect = {SCREEN_WIDTH/2 - 220, 545, 440, 50};
        const char* tip = u8"点击卡片翻转 | 底部按钮标记认识/不认识";
        Vector2 tSize = MeasureTextAuto(tip, 20, 1);
        DrawTextAuto(tip, (Vector2){SCREEN_WIDTH/2 - tSize.x/2, tipRect.y + 12}, 20, 1, STYLE->theme.textSecondary);
    }
}

// ============================================================================
// 选词背单词模式（看释义选英文单词）
// 显示汉语释义，从4个选项中选正确的英文单词
// 按用户账号统计得分
// ============================================================================
void MenuSelectWord_Show(void) {
    // 测试完成或无单词
    if (SELECT_WORD.selectCount == 0 || SELECT_WORD.currentSelectIdx >= SELECT_WORD.selectCount) {
        // 准备或重新准备题目
        if (SELECT_WORD.selectCount == 0) {
            SELECT_WORD.selectCount = (g_wordProgressCount < 15) ? g_wordProgressCount : 15;
            for (int j = 0; j < g_wordProgressCount; j++) {
                SELECT_WORD.selectIndices[j] = j;
            }
            shuffleArray(SELECT_WORD.selectIndices, g_wordProgressCount);
            SELECT_WORD.currentSelectIdx = 0;
            SELECT_WORD.selectCorrect = 0;
            SELECT_WORD.selectTotal = 0;
            SELECT_WORD.selectedAnswer = -1;
            SELECT_WORD.answerResult = -1;
        }

        // 显示结果
        if (SELECT_WORD.selectTotal > 0) {
            Rectangle resultRect = {SCREEN_WIDTH/2 - 250, SCREEN_HEIGHT/2 - 200, 500, 400};
            DrawRectangleRounded(resultRect, 0.1f, 12, STYLE->theme.panelBg);

            const char* title = u8"练习完成！";
            Vector2 tSize = MeasureTextAuto(title, 46, 1);
            DrawTextAuto(title, (Vector2){SCREEN_WIDTH/2 - tSize.x/2, resultRect.y + 35}, 46, 1, STYLE->theme.primary);

            float score = SELECT_WORD.selectTotal > 0 ?
                (float)SELECT_WORD.selectCorrect / SELECT_WORD.selectTotal * 100 : 0;
            char scoreText[128];
            snprintf(scoreText, sizeof(scoreText), u8"正确率: %.0f%% (%d/%d)",
                     score, SELECT_WORD.selectCorrect, SELECT_WORD.selectTotal);
            Vector2 sSize = MeasureTextAuto(scoreText, 34, 1);
            Color scoreColor = score >= 80 ? STYLE->theme.success :
                (score >= 60 ? (Color){255, 165, 0, 255} : STYLE->theme.error);
            DrawTextAuto(scoreText, (Vector2){SCREEN_WIDTH/2 - sSize.x/2, resultRect.y + 110},
                         34, 1, scoreColor);

            // 更新用户统计
            if (Account_IsLoggedIn()) {
                int idx = Account_GetCurrentIndex();
                if (idx >= 0) {
                    ACCOUNT.users[idx].selectWordCorrect += SELECT_WORD.selectCorrect;
                    ACCOUNT.users[idx].selectWordTotal += SELECT_WORD.selectTotal;
                    Account_Save();
                }
                char accountStats[128];
                snprintf(accountStats, sizeof(accountStats),
                         u8"账号累计: %d/%d (%.0f%%)",
                         ACCOUNT.users[Account_GetCurrentIndex()].selectWordCorrect,
                         ACCOUNT.users[Account_GetCurrentIndex()].selectWordTotal,
                         ACCOUNT.users[Account_GetCurrentIndex()].selectWordTotal > 0 ?
                         (float)ACCOUNT.users[Account_GetCurrentIndex()].selectWordCorrect /
                         ACCOUNT.users[Account_GetCurrentIndex()].selectWordTotal * 100 : 0.0f);
                Vector2 aSize = MeasureTextAuto(accountStats, 22, 1);
                DrawTextAuto(accountStats, (Vector2){SCREEN_WIDTH/2 - aSize.x/2, resultRect.y + 165},
                             22, 1, STYLE->theme.textSecondary);
            }

            // 评价
            const char* comment = "";
            if (score >= 90) comment = u8"太棒了！继续保持！";
            else if (score >= 70) comment = u8"不错！再接再厉！";
            else if (score >= 50) comment = u8"需要多复习哦！";
            else comment = u8"建议先去学单词模式";
            Vector2 cSize = MeasureTextAuto(comment, 28, 1);
            DrawTextAuto(comment, (Vector2){SCREEN_WIDTH/2 - cSize.x/2, resultRect.y + 210},
                         28, 1, STYLE->theme.textSecondary);

            // 再来一次按钮
            Rectangle restartBtn = {SCREEN_WIDTH/2 - 90, resultRect.y + 280, 180, 60};
            if (UIButton(u8"再来一次", restartBtn, STYLE, UI_STATE, 8)) {
                SELECT_WORD.selectCount = 0;
                SELECT_WORD.currentSelectIdx = 0;
                SELECT_WORD.selectCorrect = 0;
                SELECT_WORD.selectTotal = 0;
                SELECT_WORD.selectedAnswer = -1;
                SELECT_WORD.answerResult = -1;
                memset(SELECT_WORD.wrongOptionsUsed, 0, sizeof(SELECT_WORD.wrongOptionsUsed));
            }
            return;
        }
    }

    // --------------------------------------------------------------------
    // 显示当前题目
    // --------------------------------------------------------------------
    int wordIdx = SELECT_WORD.selectIndices[SELECT_WORD.currentSelectIdx];
    WordEntry* currentWord = &g_words[wordIdx].entry;

    // 显示释义（题干）
    Rectangle qRect = {SCREEN_WIDTH/2 - 250, 100, 500, 120};
    DrawRectangleRounded(qRect, 0.1f, 8, STYLE->theme.panelBg);

    DrawTextAuto(u8"请选择正确的英文单词", (Vector2){SCREEN_WIDTH/2 - 160, qRect.y + 12},
                 20, 1, STYLE->theme.textSecondary);

    // 显示汉语释义
    Rectangle defRect = {qRect.x + 25, qRect.y + 40, qRect.width - 50, 65};
    UIDrawTextRec(currentWord->definition, defRect, 32, 1, true, STYLE->theme.textPrimary);

    // 生成选项（4个英文单词）
    const char* options[4] = {0};
    static int wrongOptions[3] = {0};
    static int wrongCount = 0;
    static int lastSelectIdx = -1;

    if (lastSelectIdx != SELECT_WORD.currentSelectIdx) {
        SELECT_WORD.currentCorrectIdx = rand() % 4;
        wrongCount = 0;
        for (int i = 0; i < g_wordProgressCount && wrongCount < 3; i++) {
            if (i != wordIdx && !SELECT_WORD.wrongOptionsUsed[i]) {
                wrongOptions[wrongCount++] = i;
                SELECT_WORD.wrongOptionsUsed[i] = true;
            }
        }
        while (wrongCount < 3) {
            int idx = rand() % g_wordProgressCount;
            if (idx != wordIdx) wrongOptions[wrongCount++] = idx;
        }
        lastSelectIdx = SELECT_WORD.currentSelectIdx;
    }

    options[SELECT_WORD.currentCorrectIdx] = currentWord->word;
    int optIdx = 0;
    for (int i = 0; i < 4; i++) {
        if (i != SELECT_WORD.currentCorrectIdx && optIdx < wrongCount) {
            options[i] = g_words[wrongOptions[optIdx++]].entry.word;
        }
    }

    // 绘制选项
    UILayout optLayout = UIBeginLayout((Rectangle){SCREEN_WIDTH/2 - 250, 230, 500, 420}, UI_DIR_VERTICAL, 15, 0);
    for (int i = 0; i < 4; i++) {
        Rectangle optRect = UILayoutNext(&optLayout, -1, 85);
        Color bgColor = STYLE->theme.panelBg;
        Color borderColor = STYLE->theme.inputBorder;

        if (SELECT_WORD.answerResult != -1) {
            if (i == SELECT_WORD.currentCorrectIdx) {
                bgColor = Fade(STYLE->theme.success, 0.3f);
                borderColor = STYLE->theme.success;
            } else if (i == SELECT_WORD.selectedAnswer) {
                bgColor = Fade(STYLE->theme.error, 0.3f);
                borderColor = STYLE->theme.error;
            }
        } else if (SELECT_WORD.selectedAnswer == i) {
            bgColor = Fade(STYLE->theme.primary, 0.3f);
            borderColor = STYLE->theme.primary;
        }

        DrawRectangleRounded(optRect, 0.1f, 8, bgColor);
        DrawRectangleRoundedLines(optRect, 0.1f, 8, borderColor);

        char optLabel[32];
        snprintf(optLabel, sizeof(optLabel), "%c. ", 'A' + i);
        Vector2 labelSize = MeasureTextAuto(optLabel, 24, 1);
        DrawTextAuto(optLabel, (Vector2){optRect.x + 18, optRect.y + 22}, 24, 1, STYLE->theme.primary);

        Rectangle textRect = {optRect.x + 18 + labelSize.x, optRect.y + 18,
                              optRect.width - 36 - labelSize.x, optRect.height - 30};
        UIDrawTextRec(options[i], textRect, 30, 1, true, STYLE->theme.textPrimary);

        if (SELECT_WORD.answerResult == -1 &&
            CheckCollisionPointRec(UI_STATE->mousePos, optRect) && UI_STATE->mouseReleased) {
            SELECT_WORD.selectedAnswer = i;
            SELECT_WORD.selectTotal++;
            if (i == SELECT_WORD.currentCorrectIdx) {
                SELECT_WORD.selectCorrect++;
                SELECT_WORD.answerResult = 1;
            } else {
                SELECT_WORD.answerResult = 0;
            }
        }
    }

    // 进度
    Rectangle progressRect = {SCREEN_WIDTH/2 - 150, 660, 300, 40};
    char progressText[64];
    snprintf(progressText, sizeof(progressText), u8"进度: %d / %d  正确率: %d%%",
             SELECT_WORD.currentSelectIdx + 1, SELECT_WORD.selectCount,
             SELECT_WORD.selectTotal > 0 ?
             SELECT_WORD.selectCorrect * 100 / SELECT_WORD.selectTotal : 0);
    Vector2 pSize = MeasureTextAuto(progressText, 22, 1);
    DrawTextAuto(progressText, (Vector2){SCREEN_WIDTH/2 - pSize.x/2, progressRect.y + 8},
                 22, 1, STYLE->theme.textSecondary);

    // 下一题
    if (SELECT_WORD.answerResult != -1) {
        Rectangle nextBtn = {SCREEN_WIDTH/2 - 75, 710, 150, 55};
        if (UIButton(u8"下一题", nextBtn, STYLE, UI_STATE, 9)) {
            SELECT_WORD.currentSelectIdx++;
            SELECT_WORD.selectedAnswer = -1;
            SELECT_WORD.answerResult = -1;
        }
    }
}

// ============================================================================
// 测试模式
// 选择题测试：显示单词，从4个选项中选择正确释义
// 测试完成后显示正确率和评价
// ============================================================================
void MenuTest_Show(void) {
    // 测试完成，显示结果
    if (TEST.testCount == 0 || TEST.currentTestIdx >= TEST.testCount) {
        Rectangle resultRect = {SCREEN_WIDTH/2 - 250, SCREEN_HEIGHT/2 - 180, 500, 360};
        DrawRectangleRounded(resultRect, 0.1f, 12, STYLE->theme.panelBg);

        // 标题
        const char* title = u8"测试完成！";
        Vector2 tSize = MeasureTextAuto(title, 46, 1);
        DrawTextAuto(title, (Vector2){SCREEN_WIDTH/2 - tSize.x/2, resultRect.y + 35}, 46, 1, STYLE->theme.primary);

        // 正确率
        char scoreText[128];
        float score = TEST.testTotal > 0 ? (float)TEST.testCorrect / TEST.testTotal * 100 : 0;
        snprintf(scoreText, sizeof(scoreText), u8"正确率: %.0f%% (%d/%d)", score, TEST.testCorrect, TEST.testTotal);
        Vector2 sSize = MeasureTextAuto(scoreText, 34, 1);
        Color scoreColor = score >= 80 ? STYLE->theme.success : (score >= 60 ? (Color){255, 165, 0, 255} : STYLE->theme.error);
        DrawTextAuto(scoreText, (Vector2){SCREEN_WIDTH/2 - sSize.x/2, resultRect.y + 110}, 34, 1, scoreColor);

        // 评价
        const char* comment = "";
        if (score >= 90) comment = u8"太棒了！继续保持！";
        else if (score >= 70) comment = u8"不错！再接再厉！";
        else if (score >= 50) comment = u8"需要多复习哦！";
        else comment = u8"建议先去学单词模式";
        Vector2 cSize = MeasureTextAuto(comment, 28, 1);
        DrawTextAuto(comment, (Vector2){SCREEN_WIDTH/2 - cSize.x/2, resultRect.y + 170}, 28, 1, STYLE->theme.textSecondary);

        // 重新测试按钮
        Rectangle restartBtn = {SCREEN_WIDTH/2 - 90, resultRect.y + 250, 180, 60};
        if (UIButton(u8"再来一次", restartBtn, STYLE, UI_STATE, 3)) {
            // 重置测试状态
            for (int j = 0; j < g_wordProgressCount; j++) TEST.testIndices[j] = j;
            shuffleArray(TEST.testIndices, g_wordProgressCount);
            TEST.currentTestIdx = 0;
            TEST.testCorrect = 0;
            TEST.testTotal = 0;
            TEST.selectedAnswer = -1;
            TEST.answerResult = -1;
            memset(TEST.wrongOptionsUsed, 0, sizeof(TEST.wrongOptionsUsed));
        }
    } else {
        // --------------------------------------------------------------------
        // 显示当前题目
        // --------------------------------------------------------------------
        int wordIdx = TEST.testIndices[TEST.currentTestIdx];
        WordEntry* currentWord = &g_words[wordIdx].entry;

        // 题干区域
        Rectangle qRect = {SCREEN_WIDTH/2 - 250, 100, 500, 110};
        DrawRectangleRounded(qRect, 0.1f, 8, STYLE->theme.panelBg);

        char question[128];
        snprintf(question, sizeof(question), u8"单词 \"%s\" 的正确释义是？", currentWord->word);
        Vector2 qSize = MeasureTextAuto(question, 32, 1);
        DrawTextAuto(question, (Vector2){SCREEN_WIDTH/2 - qSize.x/2, qRect.y + 35}, 32, 1, STYLE->theme.textPrimary);

        // 生成选项
        const char* options[4] = {0};

        // 使用静态变量避免重复生成
        static int wrongOptions[3] = {0};
        static int wrongCount = 0;
        static int lastTestIdx = -1;

        // 题目改变时重新生成选项
        bool needRegenerate = (lastTestIdx != TEST.currentTestIdx);
        if (needRegenerate) {
            // 随机确定正确答案位置
            TEST.currentCorrectIdx = rand() % 4;

            // 选择3个错误的选项
            wrongCount = 0;
            for (int i = 0; i < g_wordProgressCount && wrongCount < 3; i++) {
                if (i != wordIdx && !TEST.wrongOptionsUsed[i]) {
                    wrongOptions[wrongCount++] = i;
                    TEST.wrongOptionsUsed[i] = true;
                }
            }
            // 如果单词库太小，补充随机错误选项
            while (wrongCount < 3) {
                int idx = rand() % g_wordProgressCount;
                if (idx != wordIdx) {
                    wrongOptions[wrongCount++] = idx;
                }
            }
            lastTestIdx = TEST.currentTestIdx;
        }

        // 填充选项
        options[TEST.currentCorrectIdx] = currentWord->definition;
        int optIdx = 0;
        for (int i = 0; i < 4; i++) {
            if (i != TEST.currentCorrectIdx && optIdx < wrongCount) {
                options[i] = g_words[wrongOptions[optIdx++]].entry.definition;
            }
        }

        // --------------------------------------------------------------------
        // 绘制选项
        // --------------------------------------------------------------------
        UILayout optLayout = UIBeginLayout((Rectangle){SCREEN_WIDTH/2 - 250, 220, 500, 420}, UI_DIR_VERTICAL, 15, 0);

        for (int i = 0; i < 4; i++) {
            Rectangle optRect = UILayoutNext(&optLayout, -1, 85);

            // 根据答题状态选择颜色
            Color bgColor = STYLE->theme.panelBg;
            Color borderColor = STYLE->theme.inputBorder;

            if (TEST.answerResult != -1) {
                // 显示结果后
                if (i == TEST.currentCorrectIdx) {
                    bgColor = Fade(STYLE->theme.success, 0.3f);
                    borderColor = STYLE->theme.success;
                } else if (i == TEST.selectedAnswer) {
                    bgColor = Fade(STYLE->theme.error, 0.3f);
                    borderColor = STYLE->theme.error;
                }
            } else if (TEST.selectedAnswer == i) {
                // 选择但未确认
                bgColor = Fade(STYLE->theme.primary, 0.3f);
                borderColor = STYLE->theme.primary;
            }

            // 绘制选项背景
            DrawRectangleRounded(optRect, 0.1f, 8, bgColor);
            DrawRectangleRoundedLines(optRect, 0.1f, 8, borderColor);

            // 选项标签 (A. B. C. D.)
            char optLabel[32];
            snprintf(optLabel, sizeof(optLabel), "%c. ", 'A' + i);
            Vector2 labelSize = MeasureTextAuto(optLabel, 24, 1);
            DrawTextAuto(optLabel, (Vector2){optRect.x + 18, optRect.y + 22}, 24, 1, STYLE->theme.primary);

            // 选项内容
            Rectangle textRect = {optRect.x + 18 + labelSize.x, optRect.y + 18, optRect.width - 36 - labelSize.x, optRect.height - 30};
            UIDrawTextRec(options[i], textRect, 30, 1, true, STYLE->theme.textPrimary);

            // 点击处理
            if (TEST.answerResult == -1 && CheckCollisionPointRec(UI_STATE->mousePos, optRect)) {
                if (UI_STATE->mouseReleased) {
                    TEST.selectedAnswer = i;
                    TEST.testTotal++;
                    if (i == TEST.currentCorrectIdx) {
                        TEST.testCorrect++;
                        TEST.answerResult = 1;
                    } else {
                        TEST.answerResult = 0;
                    }
                }
            }
        }

        // --------------------------------------------------------------------
        // 进度和下一题
        // --------------------------------------------------------------------
        Rectangle progressRect = {SCREEN_WIDTH/2 - 150, 660, 300, 40};
        char progressText[64];
        snprintf(progressText, sizeof(progressText), u8"进度: %d / %d  正确率: %d%%",
            TEST.currentTestIdx + 1, TEST.testCount, TEST.testTotal > 0 ? TEST.testCorrect * 100 / TEST.testTotal : 0);
        Vector2 pSize = MeasureTextAuto(progressText, 22, 1);
        DrawTextAuto(progressText, (Vector2){SCREEN_WIDTH/2 - pSize.x/2, progressRect.y + 8}, 22, 1, STYLE->theme.textSecondary);

        // 下一题按钮
        if (TEST.answerResult != -1) {
            Rectangle nextBtn = {SCREEN_WIDTH/2 - 75, 710, 150, 55};
            if (UIButton(u8"下一题", nextBtn, STYLE, UI_STATE, 4)) {
                TEST.currentTestIdx++;
                TEST.selectedAnswer = -1;
                TEST.answerResult = -1;
            }
        }
    }
}

// ============================================================================
// 学习计划管理（父页面）
// ============================================================================
void MenuPlanRoot_Show(void) {
    Rectangle cr = {250, 80, SCREEN_WIDTH - 270, SCREEN_HEIGHT - 100};
    UILayout layout = UIBeginLayout(cr, UI_DIR_VERTICAL, 30, 50);

    Rectangle titleR = UILayoutNext(&layout, -1, 60);
    DrawTextAuto(u8"学习计划", (Vector2){cr.x + 50, titleR.y}, 42, 1, STYLE->theme.primary);

    // 当前激活计划信息
    LearningPlan* active = Plan_GetActive();
    if (active != NULL) {
        Rectangle infoR = UILayoutNext(&layout, -1, 80);
        DrawRectangleRounded(infoR, 0.1f, 8, STYLE->theme.panelBg);
        char info[256];
        snprintf(info, sizeof(info), u8"当前计划: %s  |  第 %d/%d 天  |  今日 %d/%d 词",
                 active->name, active->currentDay + 1, active->totalDays,
                 active->studiedToday, active->dailyWordCount);
        DrawTextAuto(info, (Vector2){infoR.x + 20, infoR.y + 25}, 22, 1, STYLE->theme.primary);
        // 进度条
        float prog = (float)active->studiedToday / active->dailyWordCount;
        if (prog > 1.0f) prog = 1.0f;
        Rectangle barBg = {infoR.x + 20, infoR.y + 55, infoR.width - 40, 12};
        DrawRectangleRounded(barBg, 0.5f, 4, STYLE->theme.inputBg);
        DrawRectangleRounded((Rectangle){barBg.x, barBg.y, barBg.width * prog, barBg.height},
                             0.5f, 4, STYLE->theme.success);
    } else {
        Rectangle noPlanR = UILayoutNext(&layout, -1, 60);
        DrawTextAuto(u8"暂未选择学习计划，请先创建或选择一个计划",
                     (Vector2){noPlanR.x, noPlanR.y}, 22, 1, STYLE->theme.textSecondary);
    }

    // 功能卡片
    Rectangle cardsR = UILayoutNext(&layout, -1, 200);
    UILayout cardsL = UIBeginLayout(cardsR, UI_DIR_HORIZONTAL, 30, 0);

    struct { const char* t; const char* d; Color c; } cards[] = {
        {u8"计划管理", u8"创建、选择、删除\n学习计划", (Color){70, 130, 180, 255}},
        {u8"学习进度", u8"查看每个单词的\n学习掌握情况", (Color){60, 179, 113, 255}},
    };

    for (int i = 0; i < 2; i++) {
        Rectangle cardR = UILayoutNext(&cardsL, 320, -1);
        DrawRectangleRounded(cardR, 0.1f, 12, STYLE->theme.panelBg);
        DrawRectangleRoundedLines(cardR, 0.1f, 12, cards[i].c);
        Vector2 ts = MeasureTextAuto(cards[i].t, 32, 1);
        DrawTextAuto(cards[i].t, (Vector2){cardR.x + 20, cardR.y + 20}, 32, 1, cards[i].c);
        Rectangle descA = {cardR.x + 20, cardR.y + 70, cardR.width - 40, cardR.height - 130};
        UIDrawTextRec(cards[i].d, descA, 22, 1, true, STYLE->theme.textSecondary);
        Rectangle goBtn = {cardR.x + cardR.width/2 - 60, cardR.y + cardR.height - 70, 120, 50};
        if (UIButton(cards[i].t, goBtn, STYLE, UI_STATE, 700 + i)) {
            if (i < g_app.currentMenu->childindex) {
                MENU* target = g_app.currentMenu->child[i];
                if (target) { StackPush(AppState_GetMenuStack(), g_app.currentMenu); CURRENT_MENU = target; }
            }
        }
    }
}

// ============================================================================
// 学习计划管理页面
// ============================================================================

// 计划管理表单状态
static struct {
    char name[64];
    int daily;
    int days;
    UITextBoxState nameState;
    UITextBoxState dailyState;
    UITextBoxState daysState;
    bool isAdding;
    char msg[128];
    bool initialized;
} g_planForm = {0};

void MenuPlanManager_Show(void) {
    if (!g_planForm.initialized) {
        memset(&g_planForm, 0, sizeof(g_planForm));
        g_planForm.initialized = true;
        g_planForm.daily = 10;
        g_planForm.days = 7;
        snprintf(g_planForm.nameState.buffer, 1023, "%s", u8"我的计划");
    }

    Rectangle cr = {250, 80, SCREEN_WIDTH - 270, SCREEN_HEIGHT - 100};
    UILayout layout = UIBeginLayout(cr, UI_DIR_HORIZONTAL, 20, 25);

    // ============ 左侧：计划列表 ============
    Rectangle listR = UILayoutNext(&layout, 320, -1);
    DrawRectangleRounded(listR, 0.1f, 8, STYLE->theme.panelBg);

    Rectangle listTitle = {listR.x + 15, listR.y + 15, listR.width - 30, 35};
    DrawTextAuto(u8"我的计划", (Vector2){listTitle.x, listTitle.y}, 24, 1, STYLE->theme.textPrimary);

    static float planListScroll = 0.0f;
    float totalH = PLAN_STATE.planCount * 60.0f + 10;
    UIScrollView sv = {0};
    Rectangle listArea = {listR.x, listR.y + 55, listR.width, listR.height - 55};
    sv.viewport = listArea;
    sv.scrollOffset.y = planListScroll;
    sv.contentSize = (Vector2){listArea.width - 20, totalH};
    UIBeginScrollView(&sv, listArea, sv.contentSize);

    for (int i = 0; i < PLAN_STATE.planCount; i++) {
        Rectangle item = {listArea.x + 10, listArea.y + 5 + i * 60 - sv.scrollOffset.y, listArea.width - 20, 55};
        bool isActive = (i == Plan_GetActiveIndex());

        // 背景色：激活用蓝色，悬停用浅灰色
        if (isActive) {
            DrawRectangleRounded(item, 0.08f, 6, Fade(STYLE->theme.primary, 0.15f));
            DrawRectangleRoundedLines(item, 0.08f, 6, STYLE->theme.primary);
            // 左侧指示条（激活标记）
            DrawRectangleRec((Rectangle){item.x, item.y, 4, item.height}, STYLE->theme.primary);
        } else {
            bool hover = CheckCollisionPointRec(UI_STATE->mousePos, item);
            if (hover) DrawRectangleRec(item, Fade(STYLE->theme.primary, 0.06f));
        }

        // 点击激活
        if (CheckCollisionPointRec(UI_STATE->mousePos, item) && UI_STATE->mouseReleased && !isActive) {
            Plan_SetActive(i);
        }

        // 计划名 + 激活标记
        float nameX = item.x + 18;
        DrawTextAuto(PLAN_STATE.plans[i].name, (Vector2){nameX, item.y + 4}, 22, 1,
                     isActive ? STYLE->theme.primary : STYLE->theme.textPrimary);
        if (isActive) {
            DrawTextAuto(u8"▶ 当前使用", (Vector2){item.x + item.width - 100, item.y + 4}, 16, 1, STYLE->theme.primary);
        }

        // 计划参数 + 进度
        char sub[128];
        LearningPlan* p = &PLAN_STATE.plans[i];
        float dayProg = p->totalDays > 0 ? (float)(p->currentDay + 1) / p->totalDays : 0;
        if (dayProg > 1.0f) dayProg = 1.0f;
        snprintf(sub, sizeof(sub), u8"%d词/天 × %d天  进度 %d%%",
                 p->dailyWordCount, p->totalDays, (int)(dayProg * 100));
        DrawTextAuto(sub, (Vector2){nameX, item.y + 29}, 14, 1, STYLE->theme.textSecondary);

        // 进度条（小）
        Rectangle bar = {nameX, item.y + 48, item.width - nameX + item.x - listArea.x - 50, 4};
        DrawRectangleRec(bar, Fade(STYLE->theme.textSecondary, 0.2f));
        DrawRectangleRec((Rectangle){bar.x, bar.y, bar.width * dayProg, bar.height},
                         isActive ? STYLE->theme.primary : STYLE->theme.success);

        // 删除按钮（红色 ✕）
        Rectangle delBtn = {item.x + item.width - 28, item.y + 4, 24, 22};
        bool delHover = CheckCollisionPointRec(UI_STATE->mousePos, delBtn);
        if (delHover) DrawRectangleRounded(delBtn, 0.2f, 4, Fade(RED, 0.2f));
        DrawTextAuto(u8"✕", (Vector2){delBtn.x + 6, delBtn.y + 2}, 16, 1, delHover ? RED : STYLE->theme.textSecondary);
        if (delHover && UI_STATE->mouseReleased) {
            Plan_Delete(i);
        }
    }
    UIEndScrollView(&sv, STYLE, UI_STATE);
    planListScroll = sv.scrollOffset.y;

    // ============ 右侧：创建/编辑表单 ============
    Rectangle formR = UILayoutNext(&layout, -1, -1);
    DrawRectangleRounded(formR, 0.1f, 8, STYLE->theme.panelBg);
    UILayout fl = UIBeginLayout(formR, UI_DIR_VERTICAL, 15, 30);

    Rectangle ft = UILayoutNext(&fl, -1, 40);
    DrawTextAuto(u8"创建新计划", (Vector2){ft.x, ft.y}, 28, 1, STYLE->theme.primary);

    // 计划名称
    Rectangle l1 = UILayoutNext(&fl, -1, 28);
    DrawTextAuto(u8"计划名称", (Vector2){l1.x, l1.y}, 20, 1, STYLE->theme.textSecondary);
    Rectangle i1 = UILayoutNext(&fl, -1, 42);
    UITextBox(&g_planForm.nameState, i1, STYLE, UI_STATE, false);

    // 每日单词数
    Rectangle l2 = UILayoutNext(&fl, -1, 28);
    DrawTextAuto(u8"每日单词数", (Vector2){l2.x, l2.y}, 20, 1, STYLE->theme.textSecondary);
    Rectangle i2 = UILayoutNext(&fl, -1, 42);
    UITextBox(&g_planForm.dailyState, i2, STYLE, UI_STATE, false);

    // 总天数
    Rectangle l3 = UILayoutNext(&fl, -1, 28);
    DrawTextAuto(u8"总天数", (Vector2){l3.x, l3.y}, 20, 1, STYLE->theme.textSecondary);
    Rectangle i3 = UILayoutNext(&fl, -1, 42);
    UITextBox(&g_planForm.daysState, i3, STYLE, UI_STATE, false);

    // 提示消息
    if (strlen(g_planForm.msg) > 0) {
        Rectangle mr = UILayoutNext(&fl, -1, 30);
        Color mc = STYLE->theme.success;
        if (strstr(g_planForm.msg, "失败")) mc = STYLE->theme.error;
        DrawTextAuto(g_planForm.msg, (Vector2){mr.x, mr.y}, 18, 1, mc);
    }

    // 按钮
    Rectangle btnR = UILayoutNext(&fl, -1, 50);
    UILayout bl = UIBeginLayout(btnR, UI_DIR_HORIZONTAL, 20, 0);

    Rectangle createBtn = UILayoutNext(&bl, 150, 45);
    if (UIButton(u8"创建计划", createBtn, STYLE, UI_STATE, 710)) {
        const char* name = g_planForm.nameState.buffer;
        int daily = atoi(g_planForm.dailyState.buffer);
        int days = atoi(g_planForm.daysState.buffer);
        if (strlen(name) == 0) snprintf(g_planForm.msg, sizeof(g_planForm.msg), "%s", u8"请输入计划名称");
        else if (daily <= 0 || days <= 0) snprintf(g_planForm.msg, sizeof(g_planForm.msg), "%s", u8"请输入有效的数字");
        else if (Plan_Create(name, daily, days)) {
            snprintf(g_planForm.msg, sizeof(g_planForm.msg), u8"计划创建成功！");
            memset(&g_planForm.nameState, 0, sizeof(UITextBoxState));
            memset(&g_planForm.dailyState, 0, sizeof(UITextBoxState));
            memset(&g_planForm.daysState, 0, sizeof(UITextBoxState));
        } else snprintf(g_planForm.msg, sizeof(g_planForm.msg), u8"创建失败：计划名已存在或已满");
    }

    Rectangle useBtn = UILayoutNext(&bl, 150, 45);
    if (UIButton(u8"使用默认", useBtn, STYLE, UI_STATE, 711)) {
        Plan_AddDefaults();
        snprintf(g_planForm.msg, sizeof(g_planForm.msg), u8"已添加默认计划");
    }
}

// ============================================================================
// 进度模式
// 显示学习统计和每个单词的详细进度
// ============================================================================
void MenuProgress_Show(void) {
    UILayout layout = UIBeginLayout((Rectangle){250, 80, SCREEN_WIDTH - 270, SCREEN_HEIGHT - 100}, UI_DIR_VERTICAL, 20, 25);

    // --------------------------------------------------------------------
    // 统计卡片
    // --------------------------------------------------------------------
    Rectangle statsCard = UILayoutNext(&layout, -1, 130);
    DrawRectangleRounded(statsCard, 0.1f, 12, STYLE->theme.panelBg);

    int mastered = 0, learning = 0;
    for (int i = 0; i < g_wordProgressCount; i++) {
        if (g_words[i].progress.mastered) mastered++;
        else learning++;
    }

    // 三个统计数据并排显示
    UILayout statsLayout = UIBeginLayout(statsCard, UI_DIR_HORIZONTAL, 0, 0);
    Rectangle stat1 = UILayoutNext(&statsLayout, statsCard.width / 3, -1);
    Rectangle stat2 = UILayoutNext(&statsLayout, statsCard.width / 3, -1);
    Rectangle stat3 = UILayoutNext(&statsLayout, statsCard.width / 3, -1);

    char statText[64];
    Vector2 sSize;

    // 总单词数
    snprintf(statText, sizeof(statText), u8"总单词数\n%d", g_wordProgressCount);
    sSize = MeasureTextAuto(statText, 30, 1);
    DrawTextAuto(statText, (Vector2){stat1.x + stat1.width/2 - sSize.x/2, stat1.y + 35}, 30, 1, STYLE->theme.textPrimary);

    // 已掌握
    snprintf(statText, sizeof(statText), u8"已掌握\n%d", mastered);
    sSize = MeasureTextAuto(statText, 30, 1);
    DrawTextAuto(statText, (Vector2){stat2.x + stat2.width/2 - sSize.x/2, stat2.y + 35}, 30, 1, STYLE->theme.success);

    // 学习中
    snprintf(statText, sizeof(statText), u8"学习中\n%d", learning);
    sSize = MeasureTextAuto(statText, 30, 1);
    DrawTextAuto(statText, (Vector2){stat3.x + stat3.width/2 - sSize.x/2, stat3.y + 35}, 30, 1, (Color){255, 165, 0, 255});

    // 清除进度按钮
    Rectangle resetBtn = UILayoutNext(&layout, -1, 55);
    resetBtn.x = SCREEN_WIDTH - 400;
    resetBtn.width = 200;
    if (UIButton(u8"清除进度", resetBtn, STYLE, UI_STATE, 5)) {
        clearProgress();
    }

    // --------------------------------------------------------------------
    // 进度列表
    // --------------------------------------------------------------------
    Rectangle listCard = UILayoutNext(&layout, -1, -1);
    DrawRectangleRounded(listCard, 0.1f, 12, STYLE->theme.panelBg);

    UILayout listLayout = UIBeginLayout(listCard, UI_DIR_VERTICAL, 6, 18);

    // 表头
    Rectangle headerRect = UILayoutNext(&listLayout, -1, 45);
    DrawTextAuto(u8"单词", (Vector2){headerRect.x + 15, headerRect.y + 10}, 22, 1, STYLE->theme.textSecondary);
    DrawTextAuto(u8"认识", (Vector2){headerRect.x + 240, headerRect.y + 10}, 22, 1, STYLE->theme.textSecondary);
    DrawTextAuto(u8"不认识", (Vector2){headerRect.x + 350, headerRect.y + 10}, 22, 1, STYLE->theme.textSecondary);
    DrawTextAuto(u8"状态", (Vector2){headerRect.x + 500, headerRect.y + 10}, 22, 1, STYLE->theme.textSecondary);
    DrawTextAuto(u8"上次复习", (Vector2){headerRect.x + 620, headerRect.y + 10}, 22, 1, STYLE->theme.textSecondary);
    DrawLineEx((Vector2){headerRect.x, headerRect.y + 40}, (Vector2){headerRect.x + headerRect.width, headerRect.y + 40}, 1, STYLE->theme.inputBorder);

    // 滚动区域
    Rectangle scrollArea = UILayoutNext(&listLayout, -1, -1);
    UIScrollView sv = {0};
    sv.viewport = scrollArea;
    sv.contentSize = (Vector2){scrollArea.width, g_wordProgressCount * 50.0f};
    UIBeginScrollView(&sv, scrollArea, sv.contentSize);

    // 绘制每个单词的进度
    for (int i = 0; i < g_wordProgressCount; i++) {
        Rectangle rowRect = {scrollArea.x, scrollArea.y + i * 50 - sv.scrollOffset.y, scrollArea.width, 50};

        // 斑马纹背景
        if (i % 2 == 0) {
            DrawRectangleRec(rowRect, Fade(STYLE->theme.inputBg, 0.5f));
        }

        // 单词
        DrawTextAuto(g_words[i].entry.word, (Vector2){rowRect.x + 15, rowRect.y + 14}, 22, 1, STYLE->theme.textPrimary);

        // 认识次数
        char countText[32];
        snprintf(countText, sizeof(countText), "%d", g_words[i].progress.knownCount);
        DrawTextAuto(countText, (Vector2){rowRect.x + 250, rowRect.y + 14}, 22, 1, STYLE->theme.success);

        // 不认识次数
        snprintf(countText, sizeof(countText), "%d", g_words[i].progress.unknownCount);
        DrawTextAuto(countText, (Vector2){rowRect.x + 360, rowRect.y + 14}, 22, 1, STYLE->theme.error);

        // 状态
        const char* status = g_words[i].progress.mastered ? u8"已掌握" : u8"学习中";
        Color statusColor = g_words[i].progress.mastered ? STYLE->theme.success : (Color){255, 165, 0, 255};
        DrawTextAuto(status, (Vector2){rowRect.x + 500, rowRect.y + 14}, 22, 1, statusColor);

        // 上次复习时间
        DrawTextAuto(formatTime(g_words[i].progress.lastReview), (Vector2){rowRect.x + 620, rowRect.y + 14}, 20, 1, STYLE->theme.textSecondary);
    }

    UIEndScrollView(&sv, STYLE, UI_STATE);
}

// ============================================================================
// 设置页面
// ============================================================================

/**
 * 设置页面
 * 提供主题切换（深色/浅色模式）等设置选项
 */
void MenuSettings_Show(void) {
    static bool lastDarkMode = false;  // 记录上次的模式状态
    
    Rectangle contentRect = {250, 80, SCREEN_WIDTH - 270, SCREEN_HEIGHT - 100};
    UILayout layout = UIBeginLayout(contentRect, UI_DIR_VERTICAL, 30, 50);

    // 页面标题
    Rectangle titleRect = UILayoutNext(&layout, -1, 60);
    const char* title = u8"设置";
    Vector2 tSize = MeasureTextAuto(title, 42, 1);
    DrawTextAuto(title, (Vector2){contentRect.x + 50, titleRect.y}, 42, 1, STYLE->theme.textPrimary);

    // 设置面板
    Rectangle panelRect = UILayoutNext(&layout, -1, -1);
    DrawRectangleRounded(panelRect, 0.1f, 12, STYLE->theme.panelBg);

    UILayout settingsLayout = UIBeginLayout(panelRect, UI_DIR_VERTICAL, 20, 30);

    // --------------------------------------------------------------------
    // 主题设置
    // --------------------------------------------------------------------
    Rectangle themeSection = UILayoutNext(&settingsLayout, -1, 120);

    // 主题标题
    const char* themeTitle = u8"主题设置";
    DrawTextAuto(themeTitle, (Vector2){themeSection.x + 30, themeSection.y + 10}, 28, 1, STYLE->theme.textPrimary);

    // 深色模式复选框
    Rectangle checkboxRect = {themeSection.x + 30, themeSection.y + 55, 300, 40};
    if (UICheckbox(u8"深色模式", checkboxRect, &g_app.isDarkMode, STYLE, UI_STATE)) {
        // 复选框状态改变时，实时更新主题
        AppState_SetDarkMode(g_app.isDarkMode);
    }

    // 主题切换提示
    Rectangle hintRect = {themeSection.x + 30, themeSection.y + 100, 500, 30};
    const char* hint = AppState_IsDarkMode() ? u8"当前：深色模式" : u8"当前：浅色模式";
    DrawTextAuto(hint, (Vector2){hintRect.x, hintRect.y}, 18, 1, STYLE->theme.textSecondary);

    // --------------------------------------------------------------------
    // 关于信息
    // --------------------------------------------------------------------
    Rectangle aboutSection = UILayoutNext(&settingsLayout, -1, 120);

    // 关于标题
    const char* aboutTitle = u8"关于";
    DrawTextAuto(aboutTitle, (Vector2){aboutSection.x + 30, aboutSection.y + 10}, 28, 1, STYLE->theme.textPrimary);

    // 关于内容
    Rectangle aboutContent = {aboutSection.x + 30, aboutSection.y + 50, 600, 60};
    const char* aboutText = u8"背单词软件 v5.0.0\n基于 raylib 构建，支持中英文混合显示";
    UIDrawTextRec(aboutText, aboutContent, 18, 1, true, STYLE->theme.textSecondary);
}

// ============================================================================
// 词库管理页面
// ============================================================================

// 词库管理状态
static struct {
    int selectedIdx;                // 当前选中的单词索引 (-1=无)
    int searchResults[MAX_WORDS];   // 搜索结果索引
    int searchCount;                // 搜索结果数
    char searchQuery[64];           // 搜索关键词
    UITextBoxState searchBox;       // 搜索框状态
    UITextBoxState fieldStates[5];  // 编辑框状态
    bool isAdding;                  // 是否在添加模式
    char msg[128];                  // 提示消息
    int msgTimer;                   // 消息显示计时
} g_wmState = {0};

// 词库管理页面 - 搜索单词列表
static void wmDoSearch(void) {
    g_wmState.searchCount = 0;
    const char* q = g_wmState.searchBox.buffer;
    if (strlen(q) == 0) {
        for (int i = 0; i < g_wordCount; i++) g_wmState.searchResults[g_wmState.searchCount++] = i;
        return;
    }
    char qLower[64];
    strncpy(qLower, q, sizeof(qLower)-1);
    for (int j = 0; qLower[j]; j++) qLower[j] = tolower(qLower[j]);
    for (int i = 0; i < g_wordCount && g_wmState.searchCount < MAX_WORDS; i++) {
        char wLower[256];
        strncpy(wLower, g_wordLibrary[i].word, sizeof(wLower)-1);
        for (int j = 0; wLower[j]; j++) wLower[j] = tolower(wLower[j]);
        if (strstr(wLower, qLower) != NULL) g_wmState.searchResults[g_wmState.searchCount++] = i;
    }
}

void MenuWordManager_Show(void) {
    Rectangle cr = {250, 80, SCREEN_WIDTH - 270, SCREEN_HEIGHT - 100};
    UILayout layout = UIBeginLayout(cr, UI_DIR_VERTICAL, 20, 25);

    Rectangle titleR = UILayoutNext(&layout, -1, 50);
    DrawTextAuto(u8"词库管理", (Vector2){cr.x + 25, titleR.y}, 36, 1, STYLE->theme.primary);

    // 搜索栏
    Rectangle searchR = UILayoutNext(&layout, -1, 45);
    Rectangle searchInput = {searchR.x, searchR.y, searchR.width - 90, 40};
    UITextBox(&g_wmState.searchBox, searchInput, STYLE, UI_STATE, false);
    Rectangle searchBtn = {searchR.x + searchR.width - 80, searchR.y, 75, 40};
    if (UIButton(u8"搜索", searchBtn, STYLE, UI_STATE, 600)) wmDoSearch();

    // 自动搜索（初始为-1确保首次进入时触发）
    static int lastSearchLen = -1;
    int curLen = strlen(g_wmState.searchBox.buffer);
    if (curLen != lastSearchLen) { wmDoSearch(); lastSearchLen = curLen; }

    // 主内容区：左侧列表 + 右侧编辑
    Rectangle mainR = UILayoutNext(&layout, -1, -1);
    UILayout mainL = UIBeginLayout(mainR, UI_DIR_HORIZONTAL, 15, 0);

    // 左侧：单词列表（用静态变量保存滚动位置，与学单词模式相同）
    Rectangle listR = UILayoutNext(&mainL, 280, -1);
    DrawRectangleRounded(listR, 0.1f, 8, STYLE->theme.panelBg);

    static float g_wmListScroll = 0.0f;
    float listH = g_wmState.searchCount * 40.0f + 10;
    UIScrollView sv = {0};
    sv.viewport = listR;
    sv.scrollOffset.y = g_wmListScroll;
    sv.contentSize = (Vector2){listR.width - 20, listH};
    UIBeginScrollView(&sv, listR, sv.contentSize);

    for (int i = 0; i < g_wmState.searchCount; i++) {
        int idx = g_wmState.searchResults[i];
        Rectangle item = {listR.x + 10, listR.y + 5 + i * 40 - sv.scrollOffset.y, listR.width - 20, 36};
        if (idx == g_wmState.selectedIdx) DrawRectangleRec(item, Fade(STYLE->theme.primary, 0.25f));
        if (CheckCollisionPointRec(UI_STATE->mousePos, item)) {
            if (!(idx == g_wmState.selectedIdx)) DrawRectangleRec(item, Fade(STYLE->theme.primary, 0.1f));
            if (UI_STATE->mouseReleased) {
                g_wmState.selectedIdx = idx;
                g_wmState.isAdding = false;
                // 填充编辑框
                for (int f = 0; f < 5; f++) memset(&g_wmState.fieldStates[f], 0, sizeof(UITextBoxState));
                strncpy(g_wmState.fieldStates[0].buffer, g_wordLibrary[idx].word ? g_wordLibrary[idx].word : "", 1023);
                strncpy(g_wmState.fieldStates[1].buffer, g_wordLibrary[idx].phonetic ? g_wordLibrary[idx].phonetic : "", 1023);
                strncpy(g_wmState.fieldStates[2].buffer, g_wordLibrary[idx].definition ? g_wordLibrary[idx].definition : "", 1023);
                strncpy(g_wmState.fieldStates[3].buffer, g_wordLibrary[idx].example ? g_wordLibrary[idx].example : "", 1023);
                strncpy(g_wmState.fieldStates[4].buffer, g_wordLibrary[idx].exampleTranslation ? g_wordLibrary[idx].exampleTranslation : "", 1023);
                snprintf(g_wmState.msg, sizeof(g_wmState.msg), "%s", "");
            }
        }
        DrawTextAuto(g_wordLibrary[idx].word, (Vector2){item.x + 8, item.y + 8}, 20, 1, STYLE->theme.textPrimary);
    }
    UIEndScrollView(&sv, STYLE, UI_STATE);
    g_wmListScroll = sv.scrollOffset.y;  // 保存滚动位置（与学单词模式相同）

    // 右侧：编辑/详情区域
    Rectangle editR = UILayoutNext(&mainL, -1, -1);
    DrawRectangleRounded(editR, 0.1f, 8, STYLE->theme.panelBg);
    UILayout editL = UIBeginLayout(editR, UI_DIR_VERTICAL, 12, 20);

    Rectangle editTitle = UILayoutNext(&editL, -1, 35);
    DrawTextAuto(g_wmState.isAdding ? u8"添加新单词" : (g_wmState.selectedIdx >= 0 ? u8"编辑单词" : u8"请选择一个单词"),
                 (Vector2){editTitle.x, editTitle.y}, 24, 1, STYLE->theme.primary);

    const char* labels[5] = {u8"单词", u8"音标", u8"释义", u8"例句", u8"例句翻译"};
    for (int f = 0; f < 5; f++) {
        Rectangle lbR = UILayoutNext(&editL, -1, 28);
        DrawTextAuto(labels[f], (Vector2){lbR.x, lbR.y}, 18, 1, STYLE->theme.textSecondary);
        Rectangle inpR = UILayoutNext(&editL, -1, 38);
        UITextBox(&g_wmState.fieldStates[f], inpR, STYLE, UI_STATE, false);
    }

    // 按钮区
    Rectangle btnR = UILayoutNext(&editL, -1, 50);
    UILayout btnL = UIBeginLayout(btnR, UI_DIR_HORIZONTAL, 10, 0);

    // 提示消息
    if (strlen(g_wmState.msg) > 0) {
        Rectangle msgR = UILayoutNext(&editL, -1, 30);
        Color mc = STYLE->theme.success;
        if (strstr(g_wmState.msg, "失败") || strstr(g_wmState.msg, "错误")) mc = STYLE->theme.error;
        DrawTextAuto(g_wmState.msg, (Vector2){msgR.x, msgR.y}, 18, 1, mc);
    }

    // 保存按钮
    Rectangle saveBtn = UILayoutNext(&btnL, 120, 42);
    if (UIButton(u8"保存", saveBtn, STYLE, UI_STATE, 601)) {
        // 空字段自动填入"无"
        for (int f = 1; f < 5; f++) {
            if (strlen(g_wmState.fieldStates[f].buffer) == 0) {
                strncpy(g_wmState.fieldStates[f].buffer, u8"无", 1023);
            }
        }
        if (g_wmState.isAdding) {
            if (addWordToLibrary(g_wmState.fieldStates[0].buffer,
                g_wmState.fieldStates[1].buffer, g_wmState.fieldStates[2].buffer,
                g_wmState.fieldStates[3].buffer, g_wmState.fieldStates[4].buffer)) {
                snprintf(g_wmState.msg, sizeof(g_wmState.msg), u8"添加成功！");
                g_wmState.isAdding = false;
                reloadWords();
                wmDoSearch();
            } else snprintf(g_wmState.msg, sizeof(g_wmState.msg), u8"添加失败：单词不能为空");
        } else if (g_wmState.selectedIdx >= 0) {
            if (editWordInLibrary(g_wmState.selectedIdx,
                g_wmState.fieldStates[0].buffer, g_wmState.fieldStates[1].buffer,
                g_wmState.fieldStates[2].buffer, g_wmState.fieldStates[3].buffer,
                g_wmState.fieldStates[4].buffer)) {
                snprintf(g_wmState.msg, sizeof(g_wmState.msg), u8"保存成功！");
                reloadWords();
                wmDoSearch();
            } else snprintf(g_wmState.msg, sizeof(g_wmState.msg), u8"保存失败");
        }
    }

    Rectangle addBtn = UILayoutNext(&btnL, 120, 42);
    if (UIButton(u8"新增", addBtn, STYLE, UI_STATE, 602)) {
        g_wmState.isAdding = true;
        g_wmState.selectedIdx = -1;
        for (int f = 0; f < 5; f++) { memset(&g_wmState.fieldStates[f], 0, sizeof(UITextBoxState)); }
        snprintf(g_wmState.msg, sizeof(g_wmState.msg), "%s", "");
    }

    Rectangle delBtn = UILayoutNext(&btnL, 120, 42);
    if (UIButton(u8"删除", delBtn, STYLE, UI_STATE, 603)) {
        if (g_wmState.selectedIdx >= 0 && !g_wmState.isAdding) {
            if (deleteWordFromLibrary(g_wmState.selectedIdx)) {
                snprintf(g_wmState.msg, sizeof(g_wmState.msg), u8"删除成功！");
                g_wmState.selectedIdx = -1;
                reloadWords();
                wmDoSearch();
                for (int f = 0; f < 5; f++) memset(&g_wmState.fieldStates[f], 0, sizeof(UITextBoxState));
            } else snprintf(g_wmState.msg, sizeof(g_wmState.msg), u8"删除失败");
        } else snprintf(g_wmState.msg, sizeof(g_wmState.msg), u8"请先选择一个单词");
    }
}

// ============================================================================
// 账号管理页面
// ============================================================================

// 子页面状态（0=主页面，1=登录页，2=注册页）
static int g_accountSubPage = 0;

// 登录表单状态
static struct {
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
    UITextBoxState userState;
    UITextBoxState passState;
    bool initialized;
} g_loginForm = {0};

// 注册表单状态
static struct {
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
    char confirm[MAX_PASSWORD];
    UITextBoxState userState;
    UITextBoxState passState;
    UITextBoxState confirmState;
    bool initialized;
} g_regForm = {0};

/**
 * 账号管理页面
 * 支持子页面：主页面、登录、注册
 */
void MenuAccount_Show(void) {
    // 子页面状态显示
    if (g_accountSubPage == 1) {
        MenuLogin_Show();
        return;
    }
    if (g_accountSubPage == 2) {
        MenuRegister_Show();
        return;
    }

    Rectangle contentRect = {250, 80, SCREEN_WIDTH - 270, SCREEN_HEIGHT - 100};
    UILayout layout = UIBeginLayout(contentRect, UI_DIR_VERTICAL, 30, 50);

    // 页面标题
    Rectangle titleRect = UILayoutNext(&layout, -1, 60);
    const char* title = u8"账号管理";
    Vector2 tSize = MeasureTextAuto(title, 42, 1);
    DrawTextAuto(title, (Vector2){contentRect.x + 50, titleRect.y}, 42, 1, STYLE->theme.textPrimary);

    // 账号面板（固定高度，给用户列表留空间）
    Rectangle panelRect = UILayoutNext(&layout, -1, 260);
    DrawRectangleRounded(panelRect, 0.1f, 12, STYLE->theme.panelBg);

    UILayout panelLayout = UIBeginLayout(panelRect, UI_DIR_VERTICAL, 25, 40);

    if (ACCOUNT.isLoggedIn) {
        // 已登录状态：显示用户信息
        Rectangle userSection = UILayoutNext(&panelLayout, -1, 100);
        
        // 用户头像占位
        DrawCircleLines(userSection.x + 40, userSection.y + 40, 30, STYLE->theme.primary);
        DrawTextAuto(Account_GetCurrentUser(),
                     (Vector2){userSection.x + 90, userSection.y + 25}, 36, 1, STYLE->theme.primary);
        
        // 用户信息
        Rectangle infoSection = UILayoutNext(&panelLayout, -1, 60);
        char infoText[256];
        int idx = Account_GetCurrentIndex();
        if (idx >= 0) {
            char createdStr[64];
            strftime(createdStr, sizeof(createdStr), "%Y-%m-%d",
                     localtime(&ACCOUNT.users[idx].createdTime));
            snprintf(infoText, sizeof(infoText),
                     u8"注册时间: %s  |  学习数据: progress_%s.txt",
                     createdStr, Account_GetCurrentUser());
        } else {
            snprintf(infoText, sizeof(infoText), "%s", u8"当前用户");
        }
        DrawTextAuto(infoText, (Vector2){infoSection.x, infoSection.y}, 20, 1, STYLE->theme.textSecondary);

        // 登出按钮
        Rectangle btnSection = UILayoutNext(&panelLayout, -1, 60);
        UILayout btnLayout = UIBeginLayout(btnSection, UI_DIR_HORIZONTAL, 25, 0);

        Rectangle logoutBtn = UILayoutNext(&btnLayout, 160, 50);
        if (UIButton(u8"登出", logoutBtn, STYLE, UI_STATE, 300)) {
            Account_Logout();
            setProgressFilePath("./progress.txt");
            loadProgress();
            Plan_SetFilePath("./plans.txt");
            snprintf(LOGIN_MSG, 128, "%s", u8"已登出");
        }

        Rectangle switchBtn = UILayoutNext(&btnLayout, 200, 50);
        if (UIButton(u8"切换账号", switchBtn, STYLE, UI_STATE, 301)) {
            Account_Logout();
            setProgressFilePath("./progress.txt");
            loadProgress();
            Plan_SetFilePath("./plans.txt");
            g_accountSubPage = 1;
        }
    } else {
        // 未登录状态：显示登录/注册入口
        Rectangle welcomeSection = UILayoutNext(&panelLayout, -1, 80);
        DrawTextAuto(u8"欢迎使用背单词软件", (Vector2){welcomeSection.x, welcomeSection.y},
                     32, 1, STYLE->theme.primary);
        DrawTextAuto(u8"创建账号或登录后，学习进度将独立保存", 
                     (Vector2){welcomeSection.x, welcomeSection.y + 45},
                     20, 1, STYLE->theme.textSecondary);

        // 操作按钮
        Rectangle btnSection = UILayoutNext(&panelLayout, -1, 70);
        UILayout btnLayout = UIBeginLayout(btnSection, UI_DIR_HORIZONTAL, 30, 50);

        Rectangle loginBtn = UILayoutNext(&btnLayout, 230, 55);
        if (UIButton(u8"登录", loginBtn, STYLE, UI_STATE, 302)) {
            g_accountSubPage = 1;
            memset(&g_loginForm, 0, sizeof(g_loginForm));
            g_loginForm.initialized = true;
            g_loginForm.userState.hasFocus = true;
            snprintf(g_app.loginMsg, 128, "%s", "");
        }

        Rectangle registerBtn = UILayoutNext(&btnLayout, 230, 55);
        if (UIButton(u8"注册", registerBtn, STYLE, UI_STATE, 303)) {
            g_accountSubPage = 2;
            memset(&g_regForm, 0, sizeof(g_regForm));
            g_regForm.initialized = true;
            g_regForm.userState.hasFocus = true;
            snprintf(g_app.loginMsg, 128, "%s", "");
        }
    }

    // --------------------------------------------------------------------
    // 全部注册用户列表（展示所有已注册账号）
    // --------------------------------------------------------------------
    Rectangle userListLabel = UILayoutNext(&layout, -1, 35);
    DrawTextAuto(u8"全部注册用户", (Vector2){userListLabel.x, userListLabel.y}, 24, 1, STYLE->theme.textSecondary);

    Rectangle userListPanel = UILayoutNext(&layout, -1, -1);
    DrawRectangleRounded(userListPanel, 0.1f, 12, STYLE->theme.panelBg);

    // 计算列表内容高度
    float listContentH = ACCOUNT.userCount * 45.0f + 20;
    UIScrollView userSV = {0};
    userSV.viewport = userListPanel;
    userSV.contentSize = (Vector2){userListPanel.width - 30, listContentH};
    UIBeginScrollView(&userSV, userListPanel, userSV.contentSize);

    for (int i = 0; i < ACCOUNT.userCount; i++) {
        Rectangle rowRect = {userListPanel.x + 15,
                             userListPanel.y + 10 + i * 45 - userSV.scrollOffset.y,
                             userListPanel.width - 30, 40};

        // 斑马纹背景
        if (i % 2 == 0) {
            DrawRectangleRec(rowRect, Fade(STYLE->theme.inputBg, 0.5f));
        }

        // 用户名
        DrawTextAuto(ACCOUNT.users[i].username,
                     (Vector2){rowRect.x + 10, rowRect.y + 10},
                     22, 1, STYLE->theme.textPrimary);

        // 注册时间
        char timeStr[64];
        if (ACCOUNT.users[i].createdTime > 0) {
            strftime(timeStr, sizeof(timeStr), "%Y-%m-%d",
                     localtime(&ACCOUNT.users[i].createdTime));
        } else {
            snprintf(timeStr, sizeof(timeStr), "%s", u8"未知");
        }
        DrawTextAuto(timeStr,
                     (Vector2){rowRect.x + 200, rowRect.y + 10},
                     20, 1, STYLE->theme.textSecondary);

        // 上次登录时间
        if (ACCOUNT.users[i].lastLoginTime > 0) {
            char loginStr[64];
            strftime(loginStr, sizeof(loginStr), "%Y-%m-%d %H:%M",
                     localtime(&ACCOUNT.users[i].lastLoginTime));
            DrawTextAuto(loginStr,
                         (Vector2){rowRect.x + 400, rowRect.y + 10},
                         20, 1, STYLE->theme.textSecondary);
        }

        // 当前登录标记
        if (i == Account_GetCurrentIndex()) {
            DrawTextAuto(u8"[当前]",
                         (Vector2){rowRect.x + rowRect.width - 70, rowRect.y + 10},
                         20, 1, STYLE->theme.primary);
        }
    }

    UIEndScrollView(&userSV, STYLE, UI_STATE);
}

// ============================================================================
// 登录页面
// ============================================================================

/**
 * 登录页面（作为账号页面的子页面）
 */
void MenuLogin_Show(void) {
    if (!g_loginForm.initialized) {
        memset(&g_loginForm, 0, sizeof(g_loginForm));
        g_loginForm.initialized = true;
        g_loginForm.userState.hasFocus = true;
    }

    Rectangle contentRect = {250, 80, SCREEN_WIDTH - 270, SCREEN_HEIGHT - 100};
    UILayout layout = UIBeginLayout(contentRect, UI_DIR_VERTICAL, 30, 50);

    // 页面标题
    Rectangle titleRect = UILayoutNext(&layout, -1, 60);
    const char* title = u8"用户登录";
    Vector2 tSize = MeasureTextAuto(title, 42, 1);
    DrawTextAuto(title, (Vector2){SCREEN_WIDTH/2 - tSize.x/2, titleRect.y}, 42, 1, STYLE->theme.primary);

    // 登录面板
    Rectangle panelRect = {SCREEN_WIDTH/2 - 250, 200, 500, 400};
    DrawRectangleRounded(panelRect, 0.1f, 12, STYLE->theme.panelBg);

    UILayout formLayout = UIBeginLayout(panelRect, UI_DIR_VERTICAL, 20, 40);

    // 用户名输入
    Rectangle userLabel = UILayoutNext(&formLayout, -1, 35);
    DrawTextAuto(u8"用户名", (Vector2){userLabel.x, userLabel.y}, 24, 1, STYLE->theme.textSecondary);

    Rectangle userInput = UILayoutNext(&formLayout, -1, 50);
    strncpy(g_loginForm.userState.buffer, g_loginForm.username, sizeof(g_loginForm.userState.buffer) - 1);
    UITextBox(&g_loginForm.userState, userInput, STYLE, UI_STATE, false);
    strncpy(g_loginForm.username, g_loginForm.userState.buffer, MAX_USERNAME - 1);

    // 密码输入
    Rectangle passLabel = UILayoutNext(&formLayout, -1, 35);
    DrawTextAuto(u8"密码", (Vector2){passLabel.x, passLabel.y}, 24, 1, STYLE->theme.textSecondary);

    Rectangle passInput = UILayoutNext(&formLayout, -1, 50);
    strncpy(g_loginForm.passState.buffer, g_loginForm.password, sizeof(g_loginForm.passState.buffer) - 1);
    UITextBox(&g_loginForm.passState, passInput, STYLE, UI_STATE, true);
    strncpy(g_loginForm.password, g_loginForm.passState.buffer, MAX_PASSWORD - 1);

    // 提示信息
    Rectangle msgRect = UILayoutNext(&formLayout, -1, 35);
    if (strlen(LOGIN_MSG) > 0) {
        Color msgColor = STYLE->theme.error;
        if (strstr(LOGIN_MSG, "成功") || strstr(LOGIN_MSG, "欢迎"))
            msgColor = STYLE->theme.success;
        DrawTextAuto(LOGIN_MSG, (Vector2){msgRect.x, msgRect.y}, 20, 1, msgColor);
    }

    // 按钮区域
    Rectangle btnSection = UILayoutNext(&formLayout, -1, 55);
    UILayout btnLayout = UIBeginLayout(btnSection, UI_DIR_HORIZONTAL, 20, 0);

    Rectangle loginBtn = UILayoutNext(&btnLayout, 140, 50);
    if (UIButton(u8"登录", loginBtn, STYLE, UI_STATE, 310)) {
        if (strlen(g_loginForm.username) > 0 && strlen(g_loginForm.password) > 0) {
            if (Account_Login(g_loginForm.username, g_loginForm.password)) {
                char progressPath[256], planPath[256];
                Account_GetProgressPath(progressPath, sizeof(progressPath));
                setProgressFilePath(progressPath);
                loadProgress();
                Account_GetPlanPath(planPath, sizeof(planPath));
                Plan_SetFilePath(planPath);
                snprintf(LOGIN_MSG, 128, u8"欢迎回来，%s！", Account_GetCurrentUser());
                memset(&g_loginForm, 0, sizeof(g_loginForm));
                g_loginForm.initialized = true;
                g_accountSubPage = 0;
                g_app.loginRequired = false;
                CURRENT_MENU = g_app.rootMenu;
            } else {
                snprintf(LOGIN_MSG, 128, "%s", u8"登录失败：用户名或密码错误");
            }
        } else {
            snprintf(LOGIN_MSG, 128, "%s", u8"请输入用户名和密码");
        }
    }

    Rectangle regBtn = UILayoutNext(&btnLayout, 140, 50);
    if (UIButton(u8"注册账号", regBtn, STYLE, UI_STATE, 311)) {
        g_accountSubPage = 2;
        memset(&g_regForm, 0, sizeof(g_regForm));
        g_regForm.initialized = true;
        g_regForm.userState.hasFocus = true;
        snprintf(g_app.loginMsg, 128, "%s", "");
    }

    Rectangle backBtn = UILayoutNext(&btnLayout, 120, 50);
    if (UIButton(u8"返回", backBtn, STYLE, UI_STATE, 312)) {
        g_accountSubPage = 0;
        memset(&g_loginForm, 0, sizeof(g_loginForm));
        g_loginForm.initialized = true;
        snprintf(g_app.loginMsg, 128, "%s", "");
    }
}

// ============================================================================
// 注册页面
// ============================================================================

/**
 * 注册页面（作为账号页面的子页面）
 */
void MenuRegister_Show(void) {
    if (!g_regForm.initialized) {
        memset(&g_regForm, 0, sizeof(g_regForm));
        g_regForm.initialized = true;
        g_regForm.userState.hasFocus = true;
    }

    Rectangle contentRect = {250, 80, SCREEN_WIDTH - 270, SCREEN_HEIGHT - 100};
    UILayout layout = UIBeginLayout(contentRect, UI_DIR_VERTICAL, 30, 50);

    // 页面标题
    Rectangle titleRect = UILayoutNext(&layout, -1, 60);
    const char* title = u8"用户注册";
    Vector2 tSize = MeasureTextAuto(title, 42, 1);
    DrawTextAuto(title, (Vector2){SCREEN_WIDTH/2 - tSize.x/2, titleRect.y}, 42, 1, STYLE->theme.primary);

    // 注册面板
    Rectangle panelRect = {SCREEN_WIDTH/2 - 250, 200, 500, 450};
    DrawRectangleRounded(panelRect, 0.1f, 12, STYLE->theme.panelBg);

    UILayout formLayout = UIBeginLayout(panelRect, UI_DIR_VERTICAL, 18, 40);

    // 用户名
    Rectangle userLabel = UILayoutNext(&formLayout, -1, 35);
    DrawTextAuto(u8"用户名", (Vector2){userLabel.x, userLabel.y}, 24, 1, STYLE->theme.textSecondary);

    Rectangle userInput = UILayoutNext(&formLayout, -1, 50);
    strncpy(g_regForm.userState.buffer, g_regForm.username, sizeof(g_regForm.userState.buffer) - 1);
    UITextBox(&g_regForm.userState, userInput, STYLE, UI_STATE, false);
    strncpy(g_regForm.username, g_regForm.userState.buffer, MAX_USERNAME - 1);

    // 密码
    Rectangle passLabel = UILayoutNext(&formLayout, -1, 35);
    DrawTextAuto(u8"密码", (Vector2){passLabel.x, passLabel.y}, 24, 1, STYLE->theme.textSecondary);

    Rectangle passInput = UILayoutNext(&formLayout, -1, 50);
    strncpy(g_regForm.passState.buffer, g_regForm.password, sizeof(g_regForm.passState.buffer) - 1);
    UITextBox(&g_regForm.passState, passInput, STYLE, UI_STATE, true);
    strncpy(g_regForm.password, g_regForm.passState.buffer, MAX_PASSWORD - 1);

    // 确认密码
    Rectangle confirmLabel = UILayoutNext(&formLayout, -1, 35);
    DrawTextAuto(u8"确认密码", (Vector2){confirmLabel.x, confirmLabel.y}, 24, 1, STYLE->theme.textSecondary);

    Rectangle confirmInput = UILayoutNext(&formLayout, -1, 50);
    strncpy(g_regForm.confirmState.buffer, g_regForm.confirm, sizeof(g_regForm.confirmState.buffer) - 1);
    UITextBox(&g_regForm.confirmState, confirmInput, STYLE, UI_STATE, true);
    strncpy(g_regForm.confirm, g_regForm.confirmState.buffer, MAX_PASSWORD - 1);

    // 提示信息
    Rectangle msgRect = UILayoutNext(&formLayout, -1, 35);
    if (strlen(LOGIN_MSG) > 0) {
        Color msgColor = STYLE->theme.error;
        if (strstr(LOGIN_MSG, "成功")) msgColor = STYLE->theme.success;
        DrawTextAuto(LOGIN_MSG, (Vector2){msgRect.x, msgRect.y}, 20, 1, msgColor);
    }

    // 按钮区域
    Rectangle btnSection = UILayoutNext(&formLayout, -1, 55);
    UILayout btnLayout = UIBeginLayout(btnSection, UI_DIR_HORIZONTAL, 20, 0);

    Rectangle regBtn = UILayoutNext(&btnLayout, 140, 50);
    if (UIButton(u8"注册", regBtn, STYLE, UI_STATE, 320)) {
        if (strlen(g_regForm.username) == 0) {
            snprintf(LOGIN_MSG, 128, "%s", u8"请输入用户名");
        } else if (strlen(g_regForm.password) == 0) {
            snprintf(LOGIN_MSG, 128, "%s", u8"请输入密码");
        } else if (strcmp(g_regForm.password, g_regForm.confirm) != 0) {
            snprintf(LOGIN_MSG, 128, "%s", u8"两次密码输入不一致");
        } else if (Account_Register(g_regForm.username, g_regForm.password)) {
            Account_Login(g_regForm.username, g_regForm.password);
            char progressPath[256], planPath[256];
            Account_GetProgressPath(progressPath, sizeof(progressPath));
            setProgressFilePath(progressPath);
            loadProgress();
            Account_GetPlanPath(planPath, sizeof(planPath));
            Plan_SetFilePath(planPath);
            snprintf(LOGIN_MSG, 128, u8"注册成功，欢迎 %s！", g_regForm.username);
            memset(&g_regForm, 0, sizeof(g_regForm));
            g_regForm.initialized = true;
            g_accountSubPage = 0;
            g_app.loginRequired = false;
            CURRENT_MENU = g_app.rootMenu;
        } else {
            snprintf(LOGIN_MSG, 128, "%s", u8"注册失败：用户名已存在或无效");
        }
    }

    Rectangle loginBtn = UILayoutNext(&btnLayout, 140, 50);
    if (UIButton(u8"返回登录", loginBtn, STYLE, UI_STATE, 321)) {
        g_accountSubPage = 1;
        memset(&g_loginForm, 0, sizeof(g_loginForm));
        g_loginForm.initialized = true;
        g_loginForm.userState.hasFocus = true;
        snprintf(g_app.loginMsg, 128, "%s", "");
    }

    Rectangle backBtn = UILayoutNext(&btnLayout, 120, 50);
    if (UIButton(u8"返回", backBtn, STYLE, UI_STATE, 322)) {
        g_accountSubPage = 0;
        memset(&g_regForm, 0, sizeof(g_regForm));
        g_regForm.initialized = true;
        snprintf(g_app.loginMsg, 128, "%s", "");
    }
}

// ============================================================================
// 查找单词模式
// 使用正则表达式或模糊搜索查找单词，显示详细信息
// ============================================================================
void MenuSearch_Show(void) {
    Rectangle contentRect = {250, 80, SCREEN_WIDTH - 270, SCREEN_HEIGHT - 100};
    UILayout layout = UIBeginLayout(contentRect, UI_DIR_VERTICAL, 25, 30);

    // --------------------------------------------------------------------
    // 搜索框区域
    // --------------------------------------------------------------------
    Rectangle searchPanel = UILayoutNext(&layout, -1, 130);
    DrawRectangleRounded(searchPanel, 0.1f, 12, STYLE->theme.panelBg);

    // 搜索标题
    Rectangle titleRect = {searchPanel.x + 25, searchPanel.y + 15, 200, 40};
    DrawTextAuto(u8"查找单词", (Vector2){titleRect.x, titleRect.y}, 28, 1, STYLE->theme.textPrimary);

    // 搜索提示
    Rectangle hintRect = {searchPanel.x + 25, searchPanel.y + 55, 500, 30};
    DrawTextAuto(u8"支持正则表达式，例如: ab.*  或  ^acc.*", (Vector2){hintRect.x, hintRect.y}, 18, 1, STYLE->theme.textSecondary);

    // 搜索输入框
    Rectangle inputRect = {searchPanel.x + 25, searchPanel.y + 85, searchPanel.width - 150, 45};
    UISearchBar(&SEARCH.searchBar, inputRect, STYLE, UI_STATE);

    // 搜索按钮
    Rectangle btnRect = {searchPanel.x + searchPanel.width - 115, searchPanel.y + 85, 90, 45};
    static int lastSearchLen = 0;
    if (UIButton(u8"搜索", btnRect, STYLE, UI_STATE, 7)) {
        // 执行搜索
        const char* query = SEARCH.searchBar.textState.buffer;
        if (strlen(query) > 0) {
            // 尝试正则表达式搜索
            SEARCH.searchResultCount = searchWordsByRegex(query, SEARCH.searchResults, MAX_WORDS);
            // 如果正则失败或没结果，使用简单搜索
            if (SEARCH.searchResultCount == 0) {
                SEARCH.searchResultCount = searchWordsSimple(query, SEARCH.searchResults, MAX_WORDS);
            }
        }
    }

    // 检测到输入变化时自动搜索
    int currentLen = strlen(SEARCH.searchBar.textState.buffer);
    if (currentLen > 0 && currentLen != lastSearchLen) {
        const char* query = SEARCH.searchBar.textState.buffer;
        SEARCH.searchResultCount = searchWordsByRegex(query, SEARCH.searchResults, MAX_WORDS);
        if (SEARCH.searchResultCount == 0) {
            SEARCH.searchResultCount = searchWordsSimple(query, SEARCH.searchResults, MAX_WORDS);
        }
    }
    lastSearchLen = currentLen;

    // --------------------------------------------------------------------
    // 搜索结果区域
    // --------------------------------------------------------------------
    Rectangle resultPanel = UILayoutNext(&layout, -1, -1);
    DrawRectangleRounded(resultPanel, 0.1f, 12, STYLE->theme.panelBg);

    if (SEARCH.searchResultCount == 0 && strlen(SEARCH.searchBar.textState.buffer) > 0) {
        // 没有找到结果
        Rectangle msgRect = {resultPanel.x + 30, resultPanel.y + 30, resultPanel.width - 60, 60};
        DrawRectangleRounded(msgRect, 0.1f, 8, STYLE->theme.inputBg);
        const char* msg = u8"没有找到匹配的单词";
        Vector2 msgSize = MeasureTextAuto(msg, 26, 1);
        DrawTextAuto(msg, (Vector2){resultPanel.x + resultPanel.width/2 - msgSize.x/2, resultPanel.y + resultPanel.height/2 - msgSize.y/2}, 26, 1, STYLE->theme.textSecondary);
    } else if (SEARCH.searchResultCount > 0) {
        // 显示搜索结果数量
        Rectangle countRect = {resultPanel.x + 25, resultPanel.y + 15, 300, 35};
        char countText[64];
        snprintf(countText, sizeof(countText), u8"找到 %d 个结果", SEARCH.searchResultCount);
        DrawTextAuto(countText, (Vector2){countRect.x, countRect.y}, 22, 1, STYLE->theme.textSecondary);

        // 结果列表（可滚动）
        Rectangle listRect = {resultPanel.x, resultPanel.y + 55, resultPanel.width, resultPanel.height - 70};
        UIScrollView sv = {0};
        sv.viewport = listRect;
        sv.contentSize = (Vector2){listRect.width - 30, SEARCH.searchResultCount * 235.0f};
        UIBeginScrollView(&sv, listRect, sv.contentSize);

        // 绘制每个结果
        for (int i = 0; i < SEARCH.searchResultCount; i++) {
            int wordIdx = SEARCH.searchResults[i];
            WordEntry* entry = &g_words[wordIdx].entry;

            Rectangle itemRect = {listRect.x + 20, listRect.y + i * 230 - sv.scrollOffset.y, listRect.width - 40, 215};
            DrawRectangleRounded(itemRect, 0.1f, 8, STYLE->theme.inputBg);

            // 单词
            DrawTextAuto(entry->word, (Vector2){itemRect.x + 20, itemRect.y + 15}, 36, 1, STYLE->theme.primary);

            // 音标
            if (entry->phonetic && *entry->phonetic) {
                DrawTextAuto(entry->phonetic, (Vector2){itemRect.x + 20, itemRect.y + 55}, 24, 1, STYLE->theme.textSecondary);
            }

            // 释义标签和内容
            DrawTextAuto(u8"释义", (Vector2){itemRect.x + 20, itemRect.y + 85}, 18, 1, STYLE->theme.textSecondary);
            Rectangle defRect = {itemRect.x + 20, itemRect.y + 105, itemRect.width - 40, 35};
            UIDrawTextRec(entry->definition, defRect, 22, 1, true, STYLE->theme.textPrimary);

            // 例句标签和内容
            if (entry->example && *entry->example) {
                DrawTextAuto(u8"例句", (Vector2){itemRect.x + 20, itemRect.y + 140}, 18, 1, STYLE->theme.textSecondary);
                Rectangle exRect = {itemRect.x + 75, itemRect.y + 140, itemRect.width - 95, 35};
                UIDrawTextRec(entry->example, exRect, 20, 1, true, STYLE->theme.textSecondary);

                // 例句翻译
                if (entry->exampleTranslation && *entry->exampleTranslation) {
                    Rectangle transLabelRect = {itemRect.x + 20, itemRect.y + 175, 100, 30};
                    DrawTextAuto(u8"翻译", (Vector2){transLabelRect.x, transLabelRect.y}, 18, 1, (Color){100, 149, 237, 255});
                    Rectangle transRect = {itemRect.x + 75, itemRect.y + 175, itemRect.width - 95, 35};
                    UIDrawTextRec(entry->exampleTranslation, transRect, 20, 1, true, STYLE->theme.textPrimary);
                }
            }
        }

        UIEndScrollView(&sv, STYLE, UI_STATE);
    } else {
        // 初始状态，提示用户输入
        Rectangle msgRect = {resultPanel.x + 30, resultPanel.y + 30, resultPanel.width - 60, 60};
        DrawRectangleRounded(msgRect, 0.1f, 8, STYLE->theme.inputBg);
        const char* msg = u8"请在上方输入要查找的单词";
        Vector2 msgSize = MeasureTextAuto(msg, 24, 1);
        DrawTextAuto(msg, (Vector2){resultPanel.x + resultPanel.width/2 - msgSize.x/2, resultPanel.y + resultPanel.height/2 - msgSize.y/2}, 24, 1, STYLE->theme.textSecondary);
    }
}

// ============================================================================
// 菜单系统辅助函数
// ============================================================================

/**
 * 初始化菜单树
 * 创建所有菜单节点并建立父子关系
 */
void InitMenuTree(void) {
    // 创建子菜单
    MENU* menuLearn = CreatMenuTreeNode(NULL, MenuLearn_Show);
    MENU* menuSearch = CreatMenuTreeNode(NULL, MenuSearch_Show);
    // 学习计划（父节点 + 子页面）
    MENU* menuPlanRoot = CreatMenuTreeNode(NULL, MenuPlanRoot_Show);
    MENU* menuPlanManager = CreatMenuTreeNode(NULL, MenuPlanManager_Show);
    MENU* menuProgress = CreatMenuTreeNode(NULL, MenuProgress_Show);
    ConnectMenuTree(menuPlanRoot, menuPlanManager);
    ConnectMenuTree(menuPlanRoot, menuProgress);

    MENU* menuSettings = CreatMenuTreeNode(NULL, MenuSettings_Show);
    MENU* menuAccount = CreatMenuTreeNode(NULL, MenuAccount_Show);
    MENU* menuWordManager = CreatMenuTreeNode(NULL, MenuWordManager_Show);

    // 背单词模式（父节点 + 三个子模式）
    MENU* menuReviewRoot = CreatMenuTreeNode(NULL, MenuReviewRoot_Show);
    MENU* menuCardReview = CreatMenuTreeNode(NULL, MenuCardReview_Show);
    MENU* menuSelectWord = CreatMenuTreeNode(NULL, MenuSelectWord_Show);
    MENU* menuTest = CreatMenuTreeNode(NULL, MenuTest_Show);
    ConnectMenuTree(menuReviewRoot, menuCardReview);
    ConnectMenuTree(menuReviewRoot, menuSelectWord);
    ConnectMenuTree(menuReviewRoot, menuTest);

    // 创建根菜单（主页面）
    g_app.rootMenu = CreatMenuTreeNode(NULL, MenuHome_Show);

    // 连接子菜单到根菜单
    ConnectMenuTree(g_app.rootMenu, menuLearn);
    ConnectMenuTree(g_app.rootMenu, menuReviewRoot);
    ConnectMenuTree(g_app.rootMenu, menuSearch);
    ConnectMenuTree(g_app.rootMenu, menuPlanRoot);
    ConnectMenuTree(g_app.rootMenu, menuSettings);
    ConnectMenuTree(g_app.rootMenu, menuAccount);
    ConnectMenuTree(g_app.rootMenu, menuWordManager);
    // 保存账号菜单节点引用（供右上角点击跳转使用）
    g_accountMenuNode = menuAccount;

    // 初始化菜单栈
    StackInit(&g_app.menuStack);
    StackPush(&g_app.menuStack, g_app.rootMenu);
    g_app.currentMenu = g_app.rootMenu;
}

/**
 * 释放菜单树内存
 */
void FreeMenuTree(void) {
    if (g_app.rootMenu == NULL) return;

    // 释放所有子节点
    for (int i = 0; i < g_app.rootMenu->childindex; i++) {
        if (g_app.rootMenu->child[i]) {
            free(g_app.rootMenu->child[i]);
        }
    }
    // 释放根节点
    free(g_app.rootMenu);
    g_app.rootMenu = NULL;
    g_app.currentMenu = NULL;
}

/**
 * 获取菜单项文本
 * 根据菜单的显示函数确定菜单名称
 * @param menu 菜单节点
 * @return 菜单显示名称
 */
const char* GetMenuItemText(MENU* menu) {
    if (menu == NULL) return "";
    if (menu == g_app.rootMenu) return u8"主菜单";
    if (menu->show == MenuLearn_Show) return u8"学单词";
    if (menu->show == MenuReviewRoot_Show) return u8"背单词";
    if (menu->show == MenuCardReview_Show) return u8"卡片背单词";
    if (menu->show == MenuSelectWord_Show) return u8"选词背单词";
    if (menu->show == MenuTest_Show) return u8"测试";
    if (menu->show == MenuSearch_Show) return u8"查找单词";
    if (menu->show == MenuPlanRoot_Show) return u8"学习计划";
    if (menu->show == MenuPlanManager_Show) return u8"计划管理";
    if (menu->show == MenuProgress_Show) return u8"学习进度";
    if (menu->show == MenuSettings_Show) return u8"设置";
    if (menu->show == MenuAccount_Show) return u8"账号管理";
    if (menu->show == MenuWordManager_Show) return u8"词库管理";
    return "";
}

/**
 * 绘制树形菜单（左侧导航栏）
 * @param menuRect 菜单区域
 */
// 侧边菜单滚动位置（跨帧保存）
static float g_menuScrollOffset = 0.0f;

void DrawTreeMenu(Rectangle menuRect) {
    if (g_app.currentMenu == NULL) return;

    // 菜单背景
    DrawRectangleRounded(menuRect, 0.1f, 8, STYLE->theme.panelBg);

    UILayout layout = UIBeginLayout(menuRect, UI_DIR_VERTICAL, 12, 12);

    // 非根菜单显示标题和返回按钮
    if (g_app.currentMenu != g_app.rootMenu) {
        // 当前页面标题
        Rectangle titleRect = UILayoutNext(&layout, -1, 45);
        const char* title = GetMenuItemText(g_app.currentMenu);
        Vector2 tSize = MeasureTextAuto(title, 28, 1);
        DrawTextAuto(title, (Vector2){menuRect.x + menuRect.width/2 - tSize.x/2, titleRect.y + 8}, 28, 1, STYLE->theme.primary);

        // 返回按钮
        Rectangle backRect = UILayoutNext(&layout, -1, 50);
        if (UIButton(u8"返回", backRect, STYLE, UI_STATE, 6)) {
            MENU* prev = StackPop(AppState_GetMenuStack());
            if (prev != NULL && g_app.menuStack.Stacktop >= 0) {
                g_app.currentMenu = g_app.menuStack.menuStack[g_app.menuStack.Stacktop];
            } else {
                g_app.currentMenu = g_app.rootMenu;
            }
        }

        // 分隔线
        Rectangle lineRect = UILayoutNext(&layout, -1, 3);
        DrawLineEx((Vector2){lineRect.x, lineRect.y}, (Vector2){lineRect.x + lineRect.width, lineRect.y}, 1, STYLE->theme.inputBorder);
    }

    // 绘制子菜单项（可滚动）
    if (g_app.currentMenu->childindex > 0) {
        Rectangle itemsRect = UILayoutNext(&layout, -1, -1);

        // 滚动视图（使用静态变量保存滚轮位置，避免每帧重置）
        UIScrollView sv = {0};
        sv.viewport = itemsRect;
        sv.scrollOffset.y = g_menuScrollOffset;
        float totalH = g_app.currentMenu->childindex * 55.0f;
        sv.contentSize = (Vector2){itemsRect.width, totalH};
        UIBeginScrollView(&sv, itemsRect, sv.contentSize);

        for (int i = 0; i < g_app.currentMenu->childindex; i++) {
            MENU* child = g_app.currentMenu->child[i];
            if (child == NULL) continue;

            // 菜单项按钮，使用唯一 ID (200+i)
            Rectangle itemRect = {itemsRect.x, itemsRect.y + i * 55 - sv.scrollOffset.y, itemsRect.width, 55};
            if (UIButton(GetMenuItemText(child), itemRect, STYLE, UI_STATE, 200 + i)) {
                if (child->show != NULL) {
                    StackPush(AppState_GetMenuStack(), child);
                    g_app.currentMenu = child;
                }
            }
        }

        UIEndScrollView(&sv, STYLE, UI_STATE);
        g_menuScrollOffset = sv.scrollOffset.y;
    }

    // 根菜单显示标题
    if (g_app.currentMenu == g_app.rootMenu) {
        Rectangle welcomeRect = UILayoutNext(&layout, -1, 50);
        const char* welcome = u8"主菜单";
        Vector2 wSize = MeasureTextAuto(welcome, 30, 1);
        DrawTextAuto(welcome, (Vector2){menuRect.x + menuRect.width/2 - wSize.x/2, welcomeRect.y + 8}, 30, 1, STYLE->theme.primary);

        // 分隔线
        Rectangle lineRect = UILayoutNext(&layout, -1, 3);
        DrawLineEx((Vector2){lineRect.x, lineRect.y}, (Vector2){lineRect.x + lineRect.width, lineRect.y}, 1, STYLE->theme.inputBorder);
    }
}
