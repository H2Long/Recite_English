// 学习计划 — 创建/激活/跨日自动推进

#ifndef PLAN_H
#define PLAN_H

#include <stdbool.h>
#include <time.h>

#define MAX_PLANS      20
#define PLAN_NAME_MAX  64
#define PLAN_FILE_DEFAULT "./data/plans.txt"

typedef struct {
    char name[PLAN_NAME_MAX];
    int dailyWordCount;          // 每日目标词数
    int totalDays;               // 总天数
    int currentDay;              // 当前第几天(0-based)
    time_t createdAt;
    time_t lastStudyDate;        // 上次学习日期(yyyymmdd)
    int studiedToday;            // 今日已学
} LearningPlan;

typedef struct {
    LearningPlan plans[MAX_PLANS];
    int planCount;
    int activePlanIndex;         // -1=无激活
} PlanState;

typedef struct {
    const char* name;
    int daily;
    int days;
} DefaultPlan;

void Plan_SetState(PlanState* state);
void Plan_SetFilePath(const char* path);
void Plan_Init(void);
void Plan_Save(void);

bool Plan_Create(const char* name, int daily, int days);
bool Plan_Delete(int index);
void Plan_SetActive(int index);

LearningPlan* Plan_GetActive(void);
int Plan_GetActiveIndex(void);
void Plan_AddStudiedToday(int count);
void Plan_CheckNewDay(void);
int Plan_GetRemainingToday(void);
void Plan_AddDefaults(void);

#endif
