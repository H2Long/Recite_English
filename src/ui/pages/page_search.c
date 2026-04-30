// ============================================================================
// 查找单词页面
// 功能：用户输入关键词，程序在单词库中搜索匹配结果，滚动展示每个单词的详情
// 搜索方式：通配符模式（^开头/$结尾/*任意/?单字符）或简单子串匹配
// ============================================================================

#include "pages.h"

/**
 * MenuSearch_Show - 渲染"查找单词"页面
 *
 * 页面分为上下两个区域：
 *   1. 上半部分（固定高度 130px）：搜索栏
 *      - 标题文字"查找单词"（引导用户）
 *      - 语法提示文字（教会用户如何使用通配符）
 *      - 搜索输入框（UISearchBar 组件：左侧输入框 + 右侧搜索按钮）
 *   2. 下半部分（填满剩余空间）：搜索结果
 *      - 三种状态：无输入提示 / 无结果提示 / 有结果展示
 *      - 有结果时：每 230px 高度展示一个单词，包括单词、音标、释义、例句、翻译
 *
 * 搜索触发时机：
 *   - 用户点击"搜索"按钮时
 *   - 输入框内容发生变化时（自动搜索，无需手动点击）
 */
void MenuSearch_Show(void) {
    /*
     * 内容区域定义：
     *   - 左侧偏移 250px：为侧边导航栏留出空间（230px 导航栏 + 20px 间距）
     *   - 顶部偏移 80px：为顶部标题栏留出空间（60px 标题栏 + 20px 间距）
     *   - 宽度 = 屏幕宽度 - 左侧偏移 - 右侧边距(20px)
     *   - 高度 = 屏幕高度 - 顶部偏移 - 底部边距(20px)
     *   这些硬编码值在所有页面中保持一致，确保布局统一。
     */
    Rectangle cr = {250, 80, SCREEN_WIDTH - 270, SCREEN_HEIGHT - 100};
    /*
     * UIBeginLayout 创建垂直布局容器：
     *   - 方向 UI_DIR_VERTICAL：元素从上到下排列
     *   - 间距 25px：每个子元素之间的间隔
     *   - 内边距 30px：容器内部的左右边距
     */
    UILayout layout = UIBeginLayout(cr, UI_DIR_VERTICAL, 25, 30);

    // ==================================================================
    // 第一部分：搜索栏区域（固定高度 130px）
    // 包含标题、语法提示、搜索输入框三行
    // ==================================================================
    Rectangle sp = UILayoutNext(&layout, -1, 130);
    DrawRectangleRounded(sp, 0.1f, 12, STYLE->theme.panelBg);  // 圆角白色背景面板

    /*
     * 标题文字："查找单词"
     * 位置：面板左上角 (sp.x + 25, sp.y + 15)，与面板左侧边界保持 25px 间距
     * 字号 28px，使用主色调突出显示
     */
    DrawTextAuto(u8"查找单词", (Vector2){sp.x + 25, sp.y + 15}, 28, 1, STYLE->theme.textPrimary);

    /*
     * 语法提示文字：
     * 位置：在标题下方 40px 处（标题高约 30px + 10px 间距）
     * 字号 18px，使用次要文字颜色（较淡）
     * 内容提示用户可以使用类似正则表达式的语法
     */
    DrawTextAuto(u8"支持正则表达式，例如: ab.*  或  ^acc.*",
        (Vector2){sp.x + 25, sp.y + 55}, 18, 1, STYLE->theme.textSecondary);

    /*
     * 搜索输入框：
     * 位置：在提示文字下方 30px 处（sp.y + 85）
     * 宽度：面板宽度 - 150px（预留约 100px 给搜索按钮 + 50px 间距）
     * 高度：45px（标准输入框高度）
     * UISearchBar 内部包含一个 UITextBox（输入框）和一个 UIButton（搜索按钮）
     * 搜索状态存储在 SEARCH.searchBar 中，跨帧持久化
     */
    Rectangle inputRect = {sp.x + 25, sp.y + 85, sp.width - 150, 45};
    UISearchBar(&SEARCH.searchBar, inputRect, STYLE, UI_STATE);

    /*
     * 搜索按钮位置：在输入框右侧
     *   btnR.x = sp.x + sp.width - 115（面板右边界向左 115px）
     *   btnR.y = sp.y + 85（与输入框同一行）
     *   宽度 90px，高度 45px（与输入框等高）
     */
    static int lastLen = 0;  // 上一次的输入框内容长度，用于检测变化
    Rectangle btnR = {sp.x + sp.width - 115, sp.y + 85, 90, 45};

    /*
     * 方式一：点击"搜索"按钮触发搜索
     * UIButton 返回 true 表示按钮被点击
     * 按钮 ID = 7，在整个项目中保持唯一，避免与其他按钮的交互状态冲突
     */
    if (UIButton(u8"搜索", btnR, STYLE, UI_STATE, 7)) {
        const char* q = SEARCH.searchBar.textState.buffer;
        if (strlen(q) > 0) {  // 搜索词不为空时才执行搜索
            /*
             * 搜索策略：先尝试通配符模式匹配（searchWordsByRegex）
             * 这个函数支持 ^ $ * ? 语法，内部做了跨平台兼容处理（不用 POSIX regex.h）
             * 如果没有匹配结果，回退到简单的子串匹配（searchWordsSimple）
             * 这种"先精确后模糊"的两阶段策略，让用户同时获得两种搜索方式的好处
             */
            SEARCH.searchResultCount = searchWordsByRegex(q, SEARCH.searchResults, MAX_WORDS);
            if (SEARCH.searchResultCount == 0)
                SEARCH.searchResultCount = searchWordsSimple(q, SEARCH.searchResults, MAX_WORDS);
        }
    }

    /*
     * 方式二：输入框内容变化时自动搜索
     * 通过比较当前内容长度与 lastLen（上一次记录的长度）来检测变化
     * 注意：这不能检测"替换字符但长度不变"的情况，但已覆盖大部分常见操作
     * 与按钮搜索使用完全相同的搜索逻辑（代码重复，可考虑抽取为公共函数）
     */
    int curLen = strlen(SEARCH.searchBar.textState.buffer);
    if (curLen > 0 && curLen != lastLen) {
        const char* q = SEARCH.searchBar.textState.buffer;
        SEARCH.searchResultCount = searchWordsByRegex(q, SEARCH.searchResults, MAX_WORDS);
        if (SEARCH.searchResultCount == 0)
            SEARCH.searchResultCount = searchWordsSimple(q, SEARCH.searchResults, MAX_WORDS);
    }
    lastLen = curLen;  // 更新上次长度记录，供下一帧比较

    // ==================================================================
    // 第二部分：搜索结果区域（填满剩余高度）
    // 三种显示状态，通过 if-else if-else 分支选择
    // ==================================================================
    Rectangle rp = UILayoutNext(&layout, -1, -1);
    /* -1 表示填满剩余空间 */
    DrawRectangleRounded(rp, 0.1f, 12, STYLE->theme.panelBg);

    /*
     * 状态一：有搜索关键词但结果为 0 → 显示"没有找到"提示
     * 判断条件：searchResultCount == 0 且 输入框不为空
     * 注意：输入框为空时 searchResultCount 也是 0，但属于"未搜索"状态（走到 else 分支）
     */
    if (SEARCH.searchResultCount == 0 && strlen(SEARCH.searchBar.textState.buffer) > 0) {
        /* 提示面板居中显示，简洁的"没有找到"信息 */
        Rectangle mr = {rp.x + 30, rp.y + 30, rp.width - 60, 60};
        DrawRectangleRounded(mr, 0.1f, 8, STYLE->theme.inputBg);
        Vector2 ms = MeasureTextAuto(u8"没有找到匹配的单词", 26, 1);
        /* 文字在面板中居中（水平居中 = 面板中点 - 文字宽度的一半） */
        DrawTextAuto(u8"没有找到匹配的单词",
            (Vector2){rp.x + rp.width/2 - ms.x/2, rp.y + rp.height/2 - ms.y/2}, 26, 1, STYLE->theme.textSecondary);

    /*
     * 状态二：搜索结果 > 0 → 滚动列表展示每个匹配单词的详细信息
     */
    } else if (SEARCH.searchResultCount > 0) {
        /* 显示搜索结果计数："找到 X 个结果" */
        char ct[64];
        snprintf(ct, sizeof(ct), u8"找到 %d 个结果", SEARCH.searchResultCount);
        DrawTextAuto(ct, (Vector2){rp.x + 25, rp.y + 15}, 22, 1, STYLE->theme.textSecondary);

        /*
         * 可滚动的结果列表：
         * 起始位置从计数文字下方开始（rp.y + 55），高度填满剩余空间（rp.height - 70）
         * 内容总高度 = 结果数 × 每个结果项高度（230px，含间距）
         * 每个结果项内部布局：
         *   0~15px 上边距
         *   15~55px  单词（36px 字号，主色调）
         *   55~85px  音标（24px 字号，次要色）
         *   85~105px "释义"标签
         *   105~140px 释义内容
         *   140~175px "例句"标签 + 例句
         *   175~215px "翻译"标签 + 翻译（如果有）
         */
        Rectangle lr = {rp.x, rp.y + 55, rp.width, rp.height - 70};
        UIScrollView sv = {0};  /* 初始化为零值，scrollOffset 默认为 0 */
        sv.viewport = lr;
        sv.contentSize = (Vector2){lr.width - 30, SEARCH.searchResultCount * 235.0f};
        /* UIBeginScrollView 会在 viewport 区域内开启裁剪模式 */
        UIBeginScrollView(&sv, lr, sv.contentSize);

        /* 遍历每个搜索结果，逐个绘制详情卡片 */
        for (int i = 0; i < SEARCH.searchResultCount; i++) {
            /* 从 SEARCH.searchResults 数组中取第 i 个结果的单词索引 */
            WordEntry* e = &g_words[SEARCH.searchResults[i]].entry;
            /*
             * 每个结果项的位置：
             *   x = 列表左边界 + 20px 边距
             *   y = 列表顶部 + i * 230 - scrollOffset（考虑滚动偏移）
             *   宽度 = 列表宽度 - 40px（左右各 20px 边距）
             *   高度 = 215px（留 15px 作为项间距）
             */
            Rectangle ir = {lr.x + 20, lr.y + i * 230 - sv.scrollOffset.y, lr.width - 40, 215};
            DrawRectangleRounded(ir, 0.1f, 8, STYLE->theme.inputBg);

            /* 第 1 行：单词本身，36px 大号字体，主色调 */
            DrawTextAuto(e->word, (Vector2){ir.x + 20, ir.y + 15}, 36, 1, STYLE->theme.primary);

            /* 第 2 行：音标（如果存在），24px，次要色 */
            if (e->phonetic && *e->phonetic)
                DrawTextAuto(e->phonetic, (Vector2){ir.x + 20, ir.y + 55}, 24, 1, STYLE->theme.textSecondary);

            /* 第 3 行："释义"标签 + 释义内容 */
            DrawTextAuto(u8"释义", (Vector2){ir.x + 20, ir.y + 85}, 18, 1, STYLE->theme.textSecondary);
            UIDrawTextRec(e->definition,
                (Rectangle){ir.x + 20, ir.y + 105, ir.width - 40, 35}, 22, 1, true, STYLE->theme.textPrimary);

            /* 第 4 行："例句"标签 + 例句内容（如果存在） */
            if (e->example && *e->example) {
                DrawTextAuto(u8"例句", (Vector2){ir.x + 20, ir.y + 140}, 18, 1, STYLE->theme.textSecondary);
                UIDrawTextRec(e->example,
                    (Rectangle){ir.x + 75, ir.y + 140, ir.width - 95, 35}, 20, 1, true, STYLE->theme.textSecondary);

                /* 第 5 行："翻译"标签 + 翻译内容（如果存在例句翻译） */
                if (e->exampleTranslation && *e->exampleTranslation) {
                    DrawTextAuto(u8"翻译", (Vector2){ir.x + 20, ir.y + 175}, 18, 1, (Color){100, 149, 237, 255});
                    UIDrawTextRec(e->exampleTranslation,
                        (Rectangle){ir.x + 75, ir.y + 175, ir.width - 95, 35}, 20, 1, true, STYLE->theme.textPrimary);
                }
            }
        }
        /* UIEndScrollView 负责处理鼠标滚轮事件和绘制滚动条 */
        UIEndScrollView(&sv, STYLE, UI_STATE);

    /*
     * 状态三：搜索框为空（初始状态）→ 提示用户输入
     */
    } else {
        Rectangle mr = {rp.x + 30, rp.y + 30, rp.width - 60, 60};
        DrawRectangleRounded(mr, 0.1f, 8, STYLE->theme.inputBg);
        Vector2 ms = MeasureTextAuto(u8"请在上方输入要查找的单词", 24, 1);
        DrawTextAuto(u8"请在上方输入要查找的单词",
            (Vector2){rp.x + rp.width/2 - ms.x/2, rp.y + rp.height/2 - ms.y/2}, 24, 1, STYLE->theme.textSecondary);
    }
}
