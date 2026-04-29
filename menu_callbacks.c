// ============================================================================
// 菜单回调函数实现
// 功能：各菜单页面的显示逻辑
// 重构后：使用 AppState 访问器获取状态，不再直接访问散落的全局变量
// ============================================================================

#include "menu_callbacks.h"

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
    // 功能卡片（学单词、背单词、测试）
    // --------------------------------------------------------------------
    Rectangle cardsRect = UILayoutNext(&layout, -1, 160);
    UILayout cardsLayout = UIBeginLayout(cardsRect, UI_DIR_HORIZONTAL, 25, 0);

    // 模式配置
    struct { const char* title; const char* desc; MENU* target; Color color; } modes[] = {
        {u8"学单词", u8"详细学习每个单词的释义和用法", NULL, (Color){70, 130, 180, 255}},
        {u8"背单词", u8"使用闪卡翻转记忆", NULL, (Color){60, 179, 113, 255}},
        {u8"测试", u8"选择题检验学习成果", NULL, (Color){255, 140, 0, 255}}
    };

    // 关联菜单节点
    for (int i = 0; i < g_app.rootMenu->childindex && i < 3; i++) {
        modes[i].target = g_app.rootMenu->child[i];
    }

    // 绘制卡片
    for (int i = 0; i < 3; i++) {
        Rectangle cardRect = UILayoutNext(&cardsLayout, 320, -1);

        // 卡片背景和边框
        DrawRectangleRounded(cardRect, 0.1f, 12, STYLE->theme.panelBg);
        DrawRectangleRoundedLines(cardRect, 0.1f, 12, modes[i].color);

        // 标题
        Vector2 titleS = MeasureTextAuto(modes[i].title, 36, 1);
        DrawTextAuto(modes[i].title, (Vector2){cardRect.x + 20, cardRect.y + 20}, 36, 1, modes[i].color);

        // 描述
        Rectangle descArea = {cardRect.x + 20, cardRect.y + 70, cardRect.width - 40, cardRect.height - 100};
        UIDrawTextRec(modes[i].desc, descArea, 22, 1, true, STYLE->theme.textSecondary);

        // "开始"按钮
        Rectangle goBtn = {cardRect.x + cardRect.width - 130, cardRect.y + cardRect.height - 65, 110, 50};
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
// 背单词模式
// 使用闪卡翻转学习：正面显示单词，背面显示释义
// 认识3次后标记为"已掌握"
// ============================================================================
void MenuReview_Show(void) {
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
    const char* aboutText = u8"背单词软件 v1.3.0\n基于 raylib 构建，支持中英文混合显示";
    UIDrawTextRec(aboutText, aboutContent, 18, 1, true, STYLE->theme.textSecondary);
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
    MENU* menuReview = CreatMenuTreeNode(NULL, MenuReview_Show);
    MENU* menuTest = CreatMenuTreeNode(NULL, MenuTest_Show);
    MENU* menuProgress = CreatMenuTreeNode(NULL, MenuProgress_Show);
    MENU* menuSettings = CreatMenuTreeNode(NULL, MenuSettings_Show);

    // 创建根菜单（主页面）
    g_app.rootMenu = CreatMenuTreeNode(NULL, MenuHome_Show);

    // 连接子菜单到根菜单
    ConnectMenuTree(g_app.rootMenu, menuLearn);
    ConnectMenuTree(g_app.rootMenu, menuReview);
    ConnectMenuTree(g_app.rootMenu, menuTest);
    ConnectMenuTree(g_app.rootMenu, menuProgress);
    ConnectMenuTree(g_app.rootMenu, menuSettings);

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
    if (menu->show == MenuReview_Show) return u8"背单词";
    if (menu->show == MenuTest_Show) return u8"测试";
    if (menu->show == MenuProgress_Show) return u8"学习进度";
    if (menu->show == MenuSettings_Show) return u8"设置";
    return "";
}

/**
 * 绘制树形菜单（左侧导航栏）
 * @param menuRect 菜单区域
 */
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

    // 绘制子菜单项
    if (g_app.currentMenu->childindex > 0) {
        Rectangle itemsRect = UILayoutNext(&layout, -1, -1);
        UILayout itemsLayout = UIBeginLayout(itemsRect, UI_DIR_VERTICAL, 10, 0);

        for (int i = 0; i < g_app.currentMenu->childindex; i++) {
            MENU* child = g_app.currentMenu->child[i];
            if (child == NULL) continue;

            // 菜单项按钮，使用唯一 ID (200+i)
            Rectangle itemRect = UILayoutNext(&itemsLayout, -1, 55);
            if (UIButton(GetMenuItemText(child), itemRect, STYLE, UI_STATE, 200 + i)) {
                if (child->show != NULL) {
                    StackPush(AppState_GetMenuStack(), child);
                    g_app.currentMenu = child;
                }
            }
        }
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
