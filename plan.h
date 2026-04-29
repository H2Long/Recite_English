// ============================================================================
// 学习计划管理系统 - 库头文件
// 功能：学习计划的创建、管理、保存；与背单词模式交互
// ============================================================================

#ifndef PLAN_H
#define PLAN_H

#include <stdbool.h>
#include <time.h>

// ============================================================================
// 常量定义
// ============================================================================

#define MAX_PLANS         20
#define PLAN_NAME_MAX     64
#define PLAN_FILE         "./plans.txt"

// ============================================================================
// 数据结构
// ============================================================================

/**
 * 学习计划
 */
typedef struct {
    char name[PLAN_NAME_MAX];       // 计划名称
    int dailyWordCount;             // 每天背单词数
    int totalDays;                  // 总天数
    int currentDay;                 // 当前第几天（0-based）
    time_t createdAt;               // 创建时间
    time_t lastStudyDate;           // 上次学习日期（用于检测新的一天）
    int studiedToday;               // 今天已学单词数
} LearningPlan;

/**
 * 学习计划系统状态
 */
typedef struct {
    LearningPlan plans[MAX_PLANS];  // 所有计划
    int planCount;                  // 计划总数
    int activePlanIndex;            // 当前激活的计划索引（-1=无）
} PlanState;

// ============================================================================
// 默认计划配置
// ============================================================================

typedef struct {
    const char* name;
    int daily;
    int days;
} DefaultPlan;

// ============================================================================
// 学习计划库公共接口
// ============================================================================

/**
 * 初始化学习计划系统
 * 从文件加载计划 + 添加默认计划
 */
void Plan_Init(void);

/**
 * 保存计划到文件
 */
void Plan_Save(void);

/**
 * 创建新计划
 * @param name 计划名称
 * @param daily 每天单词数
 * @param days 总天数
 * @return true 创建成功
 */
bool Plan_Create(const char* name, int daily, int days);

/**
 * 删除计划
 * @param index 计划索引
 * @return true 删除成功
 */
bool Plan_Delete(int index);

/**
 * 设置激活计划
 * @param index 计划索引（-1 = 取消激活）
 */
void Plan_SetActive(int index);

/**
 * 获取当前激活计划
 * @return 计划指针，无激活返回 NULL
 */
LearningPlan* Plan_GetActive(void);

/**
 * 获取激活计划索引
 */
int Plan_GetActiveIndex(void);

/**
 * 增加今日已学单词数
 * @param count 今日已学单词数（累加模式）
 */
void Plan_AddStudiedToday(int count);

/**
 * 检查并重置每日计数（检测是否是新的一天）
 */
void Plan_CheckNewDay(void);

/**
 * 获取今日剩余应学单词数
 * @return 剩余数，无激活计划返回 0
 */
int Plan_GetRemainingToday(void);

/**
 * 添加默认计划（首次运行时）
 */
void Plan_AddDefaults(void);

#endif // PLAN_H
