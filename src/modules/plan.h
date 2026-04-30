// ============================================================================
// 学习计划管理系统 - 库头文件
// 功能：学习计划的创建、持久化、激活管理、每日进度追踪
//
// 设计说明：
//   与 account 模块类似，plan 模块也采用"外部状态绑定"模式。
//   通过 Plan_SetState() 绑定到 AppState 中的 plan 字段。
//   如果不调用 SetState，模块使用内部备用状态。
//
// 计划自动推进机制：
//   每天首次使用应用时，Plan_CheckNewDay() 会自动检测新的一天，
//   将 currentDay++、studiedToday=0。此函数在 Plan_SetActive()
//   和 Plan_Init() 中自动调用。
// ============================================================================

#ifndef PLAN_H
#define PLAN_H

#include <stdbool.h>  /* bool 类型 */
#include <time.h>     /* time_t 类型 */

// ============================================================================
// 常量定义
// ============================================================================

#define MAX_PLANS         20      /* 计划数组最大容量 */
#define PLAN_NAME_MAX     64      /* 计划名称最大长度（含 '\0'） */
#define PLAN_FILE_DEFAULT "./data/plans.txt"  /* 计划数据文件默认路径 */

// ============================================================================
// 数据结构
// ============================================================================

/**
 * LearningPlan - 单个学习计划结构体
 *
 * 每个计划定义了一个学习周期，包括每日目标词数和总天数。
 * currentDay 和 studiedToday 在 Plan_CheckNewDay() 中自动推进。
 */
typedef struct {
    char name[PLAN_NAME_MAX];       /* 计划名称（如"一周入门计划"） */
    int dailyWordCount;             /* 每日目标单词数 */
    int totalDays;                  /* 计划总天数 */
    int currentDay;                 /* 当前第几天（0-based） */
    time_t createdAt;               /* 创建时间（Unix 时间戳） */
    time_t lastStudyDate;           /* 上次学习日期（yyyymmdd 整数，用于跨日检测） */
    int studiedToday;               /* 今日已学单词数 */
} LearningPlan;

/**
 * PlanState - 学习计划系统整体状态
 */
typedef struct {
    LearningPlan plans[MAX_PLANS];  /* 所有学习计划数组 */
    int planCount;                  /* 计划总数 */
    int activePlanIndex;            /* 当前激活的计划索引（-1 = 无激活） */
} PlanState;

/**
 * DefaultPlan - 默认计划模板
 * 用于在首次运行时创建预设计划。
 */
typedef struct {
    const char* name;  /* 计划名称 */
    int daily;          /* 每日词数 */
    int days;           /* 总天数 */
} DefaultPlan;

// ============================================================================
// 公共接口
// ============================================================================

void Plan_SetState(PlanState* state);              /* 绑定状态到 AppState */
void Plan_SetFilePath(const char* path);            /* 设置计划文件路径（多用户切换） */

void Plan_Init(void);                               /* 初始化计划系统 */
void Plan_Save(void);                               /* 保存计划到文件 */

bool Plan_Create(const char* name, int daily, int days);  /* 创建新计划 */
bool Plan_Delete(int index);                               /* 删除计划 */
void Plan_SetActive(int index);                            /* 激活计划 */

LearningPlan* Plan_GetActive(void);          /* 获取当前激活的计划 */
int Plan_GetActiveIndex(void);              /* 获取激活计划索引 */

void Plan_AddStudiedToday(int count);        /* 增加今日已学单词数 */
void Plan_CheckNewDay(void);                 /* 检测并处理跨日 */
int Plan_GetRemainingToday(void);            /* 获取今日剩余应学词数 */
void Plan_AddDefaults(void);                 /* 添加 4 个默认计划 */

#endif // PLAN_H
