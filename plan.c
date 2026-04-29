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

static PlanState g_planState = {0};

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

static int find_plan(const char* name) {
    for (int i = 0; i < g_planState.planCount; i++) {
        if (strcmp(g_planState.plans[i].name, name) == 0) return i;
    }
    return -1;
}

static int get_today_date(void) {
    time_t now = time(NULL);
    struct tm* tm = localtime(&now);
    return tm->tm_year * 10000 + tm->tm_mon * 100 + tm->tm_mday;
}

// ============================================================================
// 公共接口实现
// ============================================================================

void Plan_Init(void) {
    g_planState.planCount = 0;
    g_planState.activePlanIndex = -1;

    FILE* fp = fopen(PLAN_FILE, "r");
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
        if (fieldIdx >= 3 && g_planState.planCount < MAX_PLANS) {
            LearningPlan* p = &g_planState.plans[g_planState.planCount];
            strncpy(p->name, fields[0], PLAN_NAME_MAX - 1);
            p->dailyWordCount = atoi(fields[1]);
            p->totalDays = atoi(fields[2]);
            p->currentDay = fieldIdx >= 4 ? atoi(fields[3]) : 0;
            p->createdAt = fieldIdx >= 5 ? (time_t)atol(fields[4]) : time(NULL);
            p->lastStudyDate = fieldIdx >= 6 ? (time_t)atol(fields[5]) : 0;
            p->studiedToday = fieldIdx >= 7 ? atoi(fields[6]) : 0;
            if (fieldIdx >= 8 && atoi(fields[7]) == 1) {
                g_planState.activePlanIndex = g_planState.planCount;
            }
            g_planState.planCount++;
        }
    }
    fclose(fp);
    printf("INFO: Loaded %d plans\n", g_planState.planCount);
}

void Plan_Save(void) {
    FILE* fp = fopen(PLAN_FILE, "w");
    if (fp == NULL) { printf("WARNING: Cannot save plans\n"); return; }
    for (int i = 0; i < g_planState.planCount; i++) {
        LearningPlan* p = &g_planState.plans[i];
        fprintf(fp, "%s|%d|%d|%d|%ld|%ld|%d|%d\n",
                p->name, p->dailyWordCount, p->totalDays, p->currentDay,
                (long)p->createdAt, (long)p->lastStudyDate, p->studiedToday,
                (i == g_planState.activePlanIndex) ? 1 : 0);
    }
    fclose(fp);
}

bool Plan_Create(const char* name, int daily, int days) {
    if (name == NULL || strlen(name) == 0 || daily <= 0 || days <= 0) return false;
    if (g_planState.planCount >= MAX_PLANS) return false;
    if (find_plan(name) >= 0) return false;

    LearningPlan* p = &g_planState.plans[g_planState.planCount];
    strncpy(p->name, name, PLAN_NAME_MAX - 1);
    p->dailyWordCount = daily;
    p->totalDays = days;
    p->currentDay = 0;
    p->createdAt = time(NULL);
    p->lastStudyDate = get_today_date();
    p->studiedToday = 0;
    g_planState.planCount++;
    Plan_Save();
    return true;
}

bool Plan_Delete(int index) {
    if (index < 0 || index >= g_planState.planCount) return false;
    if (g_planState.activePlanIndex == index) g_planState.activePlanIndex = -1;
    else if (g_planState.activePlanIndex > index) g_planState.activePlanIndex--;
    for (int i = index; i < g_planState.planCount - 1; i++) {
        g_planState.plans[i] = g_planState.plans[i + 1];
    }
    g_planState.planCount--;
    Plan_Save();
    return true;
}

void Plan_SetActive(int index) {
    g_planState.activePlanIndex = (index >= 0 && index < g_planState.planCount) ? index : -1;
    if (g_planState.activePlanIndex >= 0) {
        Plan_CheckNewDay();
    }
    Plan_Save();
}

LearningPlan* Plan_GetActive(void) {
    if (g_planState.activePlanIndex < 0) return NULL;
    return &g_planState.plans[g_planState.activePlanIndex];
}

int Plan_GetActiveIndex(void) {
    return g_planState.activePlanIndex;
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
    for (int i = 0; i < g_defaultPlanCount && g_planState.planCount < MAX_PLANS; i++) {
        LearningPlan* p = &g_planState.plans[g_planState.planCount];
        strncpy(p->name, g_defaultPlans[i].name, PLAN_NAME_MAX - 1);
        p->dailyWordCount = g_defaultPlans[i].daily;
        p->totalDays = g_defaultPlans[i].days;
        p->currentDay = 0;
        p->createdAt = time(NULL);
        p->lastStudyDate = 0;
        p->studiedToday = 0;
        g_planState.planCount++;
    }
    if (g_planState.planCount > 0) Plan_SetActive(0);
}
