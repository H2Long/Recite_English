// ============================================================================
// 配置文件头文件
// 功能：集中定义所有硬编码的常量，包括窗口尺寸、颜色、主题、学习参数等
// 使用方法：在其他 .c/.h 文件中 #include "config.h" 即可使用这些常量
// ============================================================================

#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// 窗口配置
// ============================================================================

#define SCREEN_WIDTH  1600   // 窗口宽度（像素）
#define SCREEN_HEIGHT 1000   // 窗口高度（像素）
#define WINDOW_TITLE  u8"背单词软件"
#define TARGET_FPS    60     // 目标帧率

// ============================================================================
// 颜色主题配置
// ============================================================================

// 基础颜色
#define COLOR_BLACK        (Color){  0,   0,   0, 255}
#define COLOR_WHITE        (Color){255, 255, 255, 255}
#define COLOR_RED          (Color){255,   0,   0, 255}
#define COLOR_GREEN        (Color){  0, 128,   0, 255}
#define COLOR_BLUE         (Color){  0,   0, 255, 255}
#define COLOR_YELLOW       (Color){255, 255,   0, 255}
#define COLOR_ORANGE       (Color){255, 165,   0, 255}
#define COLOR_GRAY         (Color){128, 128, 128, 255}
#define COLOR_LIGHTGRAY    (Color){200, 200, 200, 255}
#define COLOR_DARKGRAY     (Color){ 64,  64,  64, 255}

// 主题颜色
#define THEME_PRIMARY      (Color){ 30, 144, 255, 255}  // 主色调 - Dodger Blue
#define THEME_SUCCESS      (Color){  0, 200,  83, 255}  // 成功色 - 绿色
#define THEME_WARNING      (Color){255, 165,   0, 255}  // 警告色 - 橙色
#define THEME_DANGER       (Color){255,  69,  58, 255}  // 危险色 - 红色
#define THEME_INFO         (Color){ 90, 200, 255, 255}  // 信息色 - 天蓝色

// 透明度预设
#define OPACITY_DIM        0.2f   // 暗淡
#define OPACITY_NORMAL     0.5f   // 普通
#define OPACITY_BRIGHT     0.8f   // 明亮

// ============================================================================
// 学习参数配置
// ============================================================================

#define MASTERED_THRESHOLD     3       // 认识几次算"已掌握"（艾宾浩斯简化：3次）
#define DEFAULT_CARD_DELAY_MS  2000    // 默认卡片翻页延迟（毫秒）
#define DAILY_GOAL_DEFAULT     20      // 默认每日学习目标（个）
#define REVIEW_INTERVAL_DAYS   1       // 默认复习间隔（天）
#define MAX_WRONG_COUNT        5       // 错误几次后重点标记

// ============================================================================
// UI 布局配置
// ============================================================================

// 边距和间距
#define PADDING_SMALL   10      // 小间距
#define PADDING_MEDIUM  20      // 中间距
#define PADDING_LARGE   40      // 大间距

// 圆角
#define CORNER_RADIUS_SMALL   5       // 小圆角
#define CORNER_RADIUS_MEDIUM  10       // 中圆角
#define CORNER_RADIUS_LARGE   20       // 大圆角

// 字体大小
#define FONT_SIZE_TITLE   32      // 标题字体大小
#define FONT_SIZE_NORMAL  20      // 正常字体大小
#define FONT_SIZE_SMALL   16      // 小字体大小
#define FONT_SIZE_TINY    14      // 最小字体大小

// 按钮尺寸
#define BUTTON_HEIGHT_NORMAL   50      // 普通按钮高度
#define BUTTON_HEIGHT_LARGE    70      // 大按钮高度
#define BUTTON_MIN_WIDTH       120     // 按钮最小宽度

// 卡片配置
#define CARD_WIDTH       600      // 单词卡片宽度
#define CARD_HEIGHT      400      // 单词卡片高度
#define CARD_PADDING     30       // 卡片内边距

// ============================================================================
// 测试模式配置
// ============================================================================

#define TEST_MAX_QUESTIONS  10      // 测试最大题目数
#define TEST_TIME_LIMIT     0       // 测试时间限制（0=无限制）
#define OPTIONS_COUNT       4       // 选项数量（选择题）

// ============================================================================
// 动画配置
// ============================================================================

#define ANIMATION_SPEED_NORMAL  0.3f   // 正常动画速度（秒）
#define ANIMATION_SPEED_FAST    0.1f   // 快速动画速度（秒）
#define ANIMATION_SPEED_SLOW    0.5f   // 慢速动画速度（秒）

// ============================================================================
// 文件路径配置
// ============================================================================

#define WORDS_FILE_PATH     "./data/words.txt"       // 单词数据文件路径
#define PROGRESS_FILE_PATH  "./data/progress.txt"   // 学习进度文件路径
#define FONT_FILE_PATH      "./data/fonts/NotoSansCJK.otf" // 中文字体文件路径

#endif // CONFIG_H
