// ============================================================================
// 背单词软件 - 主程序
// 功能：学单词、背单词（闪卡）、测试（选择题）、学习进度管理
// 依赖库：raylib (图形渲染)
// ============================================================================

#include "raylib.h"
#include "raylib_word_ui.h"
#include "tree_menu.h"
#include "words.h"
#include "fonts.h"
#include "menu_callbacks.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

// ============================================================================
// 窗口尺寸常量定义
// 注意：修改此处会同时影响所有页面的布局，请参考 README 调整其他相关参数
// ============================================================================
#define SCREEN_WIDTH 1600   // 窗口宽度（像素）
#define SCREEN_HEIGHT 1000  // 窗口高度（像素）

// ============================================================================
// 全局变量定义
// 这些变量在整个程序生命周期内保持状态
// ============================================================================

// UI 样式和状态
UIStyle g_style;      // UI 主题样式（颜色、字体大小等）
UIState g_uiState;    // UI 交互状态（鼠标位置、按键状态等）

// 菜单系统
MENU* g_rootMenu = NULL;       // 根菜单节点
MENU* g_currentMenu = NULL;    // 当前显示的菜单
MenuStack g_menuStack;         // 菜单栈（用于"返回"功能）

// 学单词模式状态
int g_learnIndex = 0;           // 当前选中的单词索引
bool g_learnFilterUnknown = false;  // 是否只显示未掌握的单词

// 背单词模式（闪卡）状态
int g_reviewIndices[MAX_WORDS];     // 复习单词的索引数组
int g_reviewCount = 0;               // 待复习单词数量
int g_currentReviewIdx = 0;          // 当前复习到第几个
CardFace g_flashcardFace = CARD_FRONT;  // 闪卡当前显示正面还是背面
float g_flashcardAnimTime = 0.0f;    // 闪卡翻转动画时间
int g_knownInSession = 0;           // 本轮认识的数量
int g_unknownInSession = 0;          // 本轮不认识的的数量

// 测试模式状态
int g_testIndices[MAX_WORDS];        // 测试单词的索引数组
int g_testCount = 0;                 // 测试题总数
int g_currentTestIdx = 0;            // 当前测试到第几题
int g_testCorrect = 0;               // 正确答案数
int g_testTotal = 0;                 // 已答题总数
int g_selectedAnswer = -1;           // 用户选择的答案
int g_answerResult = -1;             // 答题结果（1=正确，0=错误，-1=未答）
int g_currentCorrectIdx = 0;         // 正确答案的选项索引
bool g_wrongOptionsUsed[MAX_WORDS];  // 标记已使用的错误选项

// ============================================================================
// 主程序入口
// ============================================================================

int main(void) {
    // 初始化 raylib 窗口
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, u8"背单词软件");
    SetTargetFPS(60);  // 设置目标帧率

    // --------------------------------------------------------------------
    // 初始化阶段
    // --------------------------------------------------------------------
    loadWordsFromFile("./words.txt");  // 从文件加载单词数据
    initWords();                       // 初始化单词进度
    srand(time(NULL));                 // 初始化随机数种子
    loadFonts();                       // 加载中英文字体
    UISetFonts(g_mergedFont, g_englishFont);  // 设置 UI 使用的字体
    UISetLatinFont(g_latinFont);       // 设置 IPA 音标字体

    // 初始化菜单树
    InitMenuTree();

    // 初始化各模式状态
    g_learnIndex = 0;
    g_learnFilterUnknown = false;
    g_flashcardFace = CARD_FRONT;
    g_flashcardAnimTime = 0.0f;
    g_knownInSession = 0;
    g_unknownInSession = 0;

    // 准备闪卡复习（只显示未复习过的单词）
    g_reviewCount = 0;
    for (int j = 0; j < g_wordProgressCount; j++) {
        if (g_words[j].progress.lastReview == 0) {
            g_reviewIndices[g_reviewCount++] = j;
        }
    }
    shuffleArray(g_reviewIndices, g_reviewCount);  // 随机打乱顺序
    g_currentReviewIdx = 0;

    // 准备测试模式（最多10题）
    g_testCount = (g_wordProgressCount < 10) ? g_wordProgressCount : 10;
    for (int j = 0; j < g_wordProgressCount; j++) {
        g_testIndices[j] = j;
    }
    shuffleArray(g_testIndices, g_wordProgressCount);
    g_currentTestIdx = 0;
    g_testCorrect = 0;
    g_testTotal = 0;
    g_selectedAnswer = -1;
    g_answerResult = -1;
    memset(g_wrongOptionsUsed, 0, sizeof(g_wrongOptionsUsed));

    // 初始化 UI 样式（字体大小在此调整）
    UIStyleInit(&g_style);
    g_style.font = g_mergedFont;
    // 以下是字体大小参数，可根据需要调整
    g_style.fontSizeSmall = 22.0f;    // 小号字体（如选项标签）
    g_style.fontSizeNormal = 28.0f;   // 普通字体（如按钮文字）
    g_style.fontSizeLarge = 46.0f;    // 大号字体（如单词显示）

    // --------------------------------------------------------------------
    // 主循环
    // --------------------------------------------------------------------
    while (!WindowShouldClose()) {
        // 更新 UI 状态
        UIBegin(&g_uiState);

        BeginDrawing();
        ClearBackground(g_style.theme.background);

        // 绘制顶部标题栏
        Rectangle topBar = {0, 0, SCREEN_WIDTH, 60};
        DrawRectangleRec(topBar, g_style.theme.panelBg);
        DrawLineEx((Vector2){0, 60}, (Vector2){SCREEN_WIDTH, 60}, 1, g_style.theme.inputBorder);

        // 标题文字
        const char* topTitle = u8"背单词软件";
        Vector2 titleSize = MeasureTextAuto(topTitle, 40, 1);
        DrawTextAuto(topTitle, (Vector2){SCREEN_WIDTH/2 - titleSize.x/2, 8}, 40, 1, g_style.theme.primary);

        // 绘制左侧树形导航菜单
        Rectangle menuRect = {10, 70, 230, SCREEN_HEIGHT - 80};
        DrawTreeMenu(menuRect);

        // 根据当前菜单显示对应页面
        if (g_currentMenu->show != NULL) {
            g_currentMenu->show();
        }

        EndDrawing();
        UIEnd(&g_uiState);
    }

    // --------------------------------------------------------------------
    // 清理阶段
    // --------------------------------------------------------------------
    FreeMenuTree();      // 释放菜单树内存
    unloadFonts();       // 卸载字体
    freeWordLibrary();   // 释放单词库内存
    CloseWindow();       // 关闭 raylib 窗口

    return 0;
}
