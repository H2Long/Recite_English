// ============================================================================
// 学习计划管理系统 - 库实现
// 功能：学习计划的创建、持久化、激活管理
// ============================================================================

#include "plan.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// 内部状态
// ============================================================================

// g_pPlanState: 指向外部状态的指针（由 AppState 通过 Plan_SetState 传入）
// 如果为 NULL，库使用内部备用状态 g_planInternal
static PlanState* g_pPlanState = NULL;
static PlanState g_planInternal;            // 内部备用状态（无外部绑定时使用）
static char g_planFilePath[256] = "./data/plans.txt";  // 计划文件路径（支持运行时切换）

/**
 * Plan_SetState - 绑定外部状态指针
 * 
 * 将 plan 模块与 AppState 中的 PlanState 关联起来。
 * 如果不调用此函数，库会自动使用内部静态 PlanState。
 * 此设计使 plan 模块可以独立运行，也可以集成到统一状态管理中。
 * 
 * @param state 外部 PlanState 指针（传入 &g_app.plan）
 */
void Plan_SetState(PlanState* state) {
    g_pPlanState = state;
}

/**
 * getPlanState - 获取当前有效的计划状态指针
 * 
 * 优先返回外部绑定的状态，未绑定时返回内部备用状态。
 * 
 * @return PlanState* 当前有效的计划状态指针
 */
static PlanState* getPlanState(void) {
    return g_pPlanState ? g_pPlanState : &g_planInternal;
}

/**
 * Plan_SetFilePath - 设置计划文件路径（用于多用户切换）
 * 
 * 切换用户时调用此函数更新计划文件路径，
 * 设置后会自动调用 Plan_Init() 从新路径重新加载计划数据。
 * 
 * @param path 新的计划文件路径（如 "./data/plans_hhlong.txt"）
 */
void Plan_SetFilePath(const char* path) {
    strncpy(g_planFilePath, path, sizeof(g_planFilePath) - 1);
    g_planFilePath[sizeof(g_planFilePath) - 1] = '\0';
    printf("INFO: Plan file set to: %s\n", g_planFilePath);
    // 重新从新路径加载
    Plan_Init();
}

// ============================================================================
// 默认计划
// ============================================================================

static const DefaultPlan g_defaultPlans[] = {
    {u8"一周入门计划",   10, 7},
    {u8"半月巩固计划",   15, 15},
    {u8"三十天进阶计划", 20, 30},
    {u8"六十天冲刺计划", 30, 60},
};
static const int g_defaultPlanCount = sizeof(g_defaultPlans) / sizeof(g_defaultPlans[0]);

// ============================================================================
// 内部辅助
// ============================================================================

/**
 * find_plan - 按名称查找计划索引
 * 
 * @param name 计划名称
 * @return int 计划索引，-1 表示未找到
 */
static int find_plan(const char* name) {
    for (int i = 0; i < getPlanState()->planCount; i++) {
        if (strcmp(getPlanState()->plans[i].name, name) == 0) return i;
    }
    return -1;
}

/**
 * get_today_date - 获取今天的日期整数值
 * 
 * 将当前日期编码为 yyyymmdd 格式的整数，
 * 用于比较两个日期是否为同一天（判断"新的一天"）。
 * 例如 2026年4月30日 → 20260430
 * 
 * 注意：使用 tm_mon 时需 +1，因为 tm_mon 范围是 0~11
 * 
 * @return int 今天的日期编码（yyyymmdd）
 */
static int get_today_date(void) {
    time_t now = time(NULL);
    struct tm* tm = localtime(&now);
    return tm->tm_year * 10000 + tm->tm_mon * 100 + tm->tm_mday;
}

// ============================================================================
// 公共接口实现
// ============================================================================

void Plan_Init(void) {
    getPlanState()->planCount = 0;
    getPlanState()->activePlanIndex = -1;

    FILE* fp = fopen(g_planFilePath, "r");
    if (fp == NULL) {
        printf("INFO: No plan file found, adding default plans\n");
        Plan_AddDefaults();
        Plan_Save();
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') line[len-1] = '\0';
        if (strlen(line) == 0) continue;

        // format: name|daily|totalDays|currentDay|createdAt|lastStudyDate|studiedToday|isActive
        char* fields[8] = {NULL};
        int fieldIdx = 0;
        char* token = strtok(line, "|");
        while (token != NULL && fieldIdx < 8) {
            fields[fieldIdx++] = token;
            token = strtok(NULL, "|");
        }
        if (fieldIdx >= 3 && getPlanState()->planCount < MAX_PLANS) {
            LearningPlan* p = &getPlanState()->plans[getPlanState()->planCount];
            strncpy(p->name, fields[0], PLAN_NAME_MAX - 1);
            p->dailyWordCount = atoi(fields[1]);
            p->totalDays = atoi(fields[2]);
            p->currentDay = fieldIdx >= 4 ? atoi(fields[3]) : 0;
            p->createdAt = fieldIdx >= 5 ? (time_t)atol(fields[4]) : time(NULL);
            p->lastStudyDate = fieldIdx >= 6 ? (time_t)atol(fields[5]) : 0;
            p->studiedToday = fieldIdx >= 7 ? atoi(fields[6]) : 0;
            if (fieldIdx >= 8 && atoi(fields[7]) == 1) {
                getPlanState()->activePlanIndex = getPlanState()->planCount;
            }
            getPlanState()->planCount++;
        }
    }
    fclose(fp);
    printf("INFO: Loaded %d plans\n", getPlanState()->planCount);
}

void Plan_Save(void) {
    FILE* fp = fopen(g_planFilePath, "w");
    if (fp == NULL) { printf("WARNING: Cannot save plans\n"); return; }
    for (int i = 0; i < getPlanState()->planCount; i++) {
        LearningPlan* p = &getPlanState()->plans[i];
        fprintf(fp, "%s|%d|%d|%d|%ld|%ld|%d|%d\n",
                p->name, p->dailyWordCount, p->totalDays, p->currentDay,
                (long)p->createdAt, (long)p->lastStudyDate, p->studiedToday,
                (i == getPlanState()->activePlanIndex) ? 1 : 0);
    }
    fclose(fp);
}

bool Plan_Create(const char* name, int daily, int days) {
    if (name == NULL || strlen(name) == 0 || daily <= 0 || days <= 0) return false;
    if (getPlanState()->planCount >= MAX_PLANS) return false;
    if (find_plan(name) >= 0) return false;

    LearningPlan* p = &getPlanState()->plans[getPlanState()->planCount];
    strncpy(p->name, name, PLAN_NAME_MAX - 1);
    p->dailyWordCount = daily;
    p->totalDays = days;
    p->currentDay = 0;
    p->createdAt = time(NULL);
    p->lastStudyDate = get_today_date();
    p->studiedToday = 0;
    getPlanState()->planCount++;
    Plan_Save();
    return true;
}

bool Plan_Delete(int index) {
    if (index < 0 || index >= getPlanState()->planCount) return false;
    if (getPlanState()->activePlanIndex == index) getPlanState()->activePlanIndex = -1;
    else if (getPlanState()->activePlanIndex > index) getPlanState()->activePlanIndex--;
    for (int i = index; i < getPlanState()->planCount - 1; i++) {
        getPlanState()->plans[i] = getPlanState()->plans[i + 1];
    }
    getPlanState()->planCount--;
    Plan_Save();
    return true;
}

void Plan_SetActive(int index) {
    getPlanState()->activePlanIndex = (index >= 0 && index < getPlanState()->planCount) ? index : -1;
    if (getPlanState()->activePlanIndex >= 0) {
        Plan_CheckNewDay();
    }
    Plan_Save();
}

/**
 * Plan_GetActive - 获取当前激活的学习计划
 * 
 * @return LearningPlan* 当前激活计划的指针，无激活计划时返回 NULL
 */
LearningPlan* Plan_GetActive(void) {
    if (getPlanState()->activePlanIndex < 0) return NULL;
    return &getPlanState()->plans[getPlanState()->activePlanIndex];
}

int Plan_GetActiveIndex(void) {
    return getPlanState()->activePlanIndex;
}

void Plan_AddStudiedToday(int count) {
    LearningPlan* p = Plan_GetActive();
    if (p != NULL) {
        p->studiedToday += count;
        p->lastStudyDate = get_today_date();
        Plan_Save();
    }
}

void Plan_CheckNewDay(void) {
    LearningPlan* p = Plan_GetActive();
    if (p == NULL) return;
    int today = get_today_date();
    if (p->lastStudyDate != today) {
        // 新的一天
        if (p->lastStudyDate > 0) {
            p->currentDay++;
        }
        p->studiedToday = 0;
        p->lastStudyDate = today;
        Plan_Save();
    }
}

int Plan_GetRemainingToday(void) {
    LearningPlan* p = Plan_GetActive();
    if (p == NULL) return 0;
    int remaining = p->dailyWordCount - p->studiedToday;
    return remaining > 0 ? remaining : 0;
}

void Plan_AddDefaults(void) {
    for (int i = 0; i < g_defaultPlanCount && getPlanState()->planCount < MAX_PLANS; i++) {
        LearningPlan* p = &getPlanState()->plans[getPlanState()->planCount];
        strncpy(p->name, g_defaultPlans[i].name, PLAN_NAME_MAX - 1);
        p->dailyWordCount = g_defaultPlans[i].daily;
        p->totalDays = g_defaultPlans[i].days;
        p->currentDay = 0;
        p->createdAt = time(NULL);
        p->lastStudyDate = 0;
        p->studiedToday = 0;
        getPlanState()->planCount++;
    }
    if (getPlanState()->planCount > 0) Plan_SetActive(0);
}
