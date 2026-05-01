// 学习计划 — 创建/激活/跨日自动推进

#include "plan.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static PlanState* g_pPlanState = NULL;
static PlanState g_planInternal;
static char g_planFilePath[256] = "./data/plans.txt";

void Plan_SetState(PlanState* state) { g_pPlanState = state; }

static PlanState* getState(void) { return g_pPlanState ? g_pPlanState : &g_planInternal; }

void Plan_SetFilePath(const char* path) {
    strcpy(g_planFilePath, path);
    Plan_Init();
}

static const DefaultPlan g_defaultPlans[] = {
    {u8"一周入门计划", 10, 7}, {u8"半月巩固计划", 15, 15},
    {u8"三十天进阶计划", 20, 30}, {u8"六十天冲刺计划", 30, 60},
};

static int find_plan(const char* name) {
    PlanState* state = getState();
    for (int i = 0; i < state->planCount; i++) {
        if(strcmp(state->plans[i].name, name) == 0) { return i; }
    }
    return -1;
}

static int get_today_date(void) {
    time_t now = time(NULL);
    struct tm* tm = localtime(&now);
    return tm->tm_year * 10000 + tm->tm_mon * 100 + tm->tm_mday;
}

void Plan_Init(void) {
    PlanState* state = getState();
    state->planCount = 0;
    state->activePlanIndex = -1;
    FILE* fp = fopen(g_planFilePath, "r");
    if(fp == NULL) {
        Plan_AddDefaults();
        Plan_Save();
        return ;
    }
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        int len = strlen(line);
        if(len > 0 && line[len-1] == '\n') { line[len-1] = '\0'; }
        if(strlen(line) == 0) { continue; }
        char* f[8] = {NULL};
        int fi = 0;
        char* tok = strtok(line, "|");
        while (tok && fi < 8) { f[fi++] = tok; tok = strtok(NULL, "|"); }
        if(fi >= 3 && state->planCount < MAX_PLANS) {
            LearningPlan* p = &state->plans[state->planCount];
            strncpy(p->name, f[0], PLAN_NAME_MAX - 1);
            p->dailyWordCount = atoi(f[1]);
            p->totalDays = atoi(f[2]);
            p->currentDay = fi >= 4 ? atoi(f[3]) : 0;
            p->createdAt = fi >= 5 ? (time_t)atol(f[4]) : time(NULL);
            p->lastStudyDate = fi >= 6 ? (time_t)atol(f[5]) : 0;
            p->studiedToday = fi >= 7 ? atoi(f[6]) : 0;
            if(fi >= 8 && atoi(f[7]) == 1) { state->activePlanIndex = state->planCount; }
            state->planCount++;
        }
    }
    fclose(fp);
}

void Plan_Save(void) {
    PlanState* state = getState();
    FILE* fp = fopen(g_planFilePath, "w");
    if(fp == NULL) { return ; }
    for (int i = 0; i < state->planCount; i++) {
        LearningPlan* p = &state->plans[i];
        fprintf(fp, "%s|%d|%d|%d|%ld|%ld|%d|%d\n",
            p->name, p->dailyWordCount, p->totalDays, p->currentDay,
            (long)p->createdAt, (long)p->lastStudyDate, p->studiedToday,
            (i == state->activePlanIndex) ? 1 : 0);
    }
    fclose(fp);
}

bool Plan_Create(const char* name, int daily, int days) {
    if(name == NULL || strlen(name) == 0) { return false; }
    if(daily <= 0 || days <= 0) { return false; }
    PlanState* state = getState();
    if(state->planCount >= MAX_PLANS) { return false; }
    if(find_plan(name) >= 0) { return false; }
    LearningPlan* p = &state->plans[state->planCount];
    strncpy(p->name, name, PLAN_NAME_MAX - 1);
    p->dailyWordCount = daily;
    p->totalDays = days;
    p->currentDay = 0;
    p->createdAt = time(NULL);
    p->lastStudyDate = get_today_date();
    p->studiedToday = 0;
    state->planCount++;
    Plan_Save();
    return true;
}

bool Plan_Delete(int index) {
    PlanState* state = getState();
    if(index < 0 || index >= state->planCount) { return false; }
    if(state->activePlanIndex == index) { state->activePlanIndex = -1; }
    else if(state->activePlanIndex > index) { state->activePlanIndex--; }
    for (int i = index; i < state->planCount - 1; i++) { state->plans[i] = state->plans[i+1]; }
    state->planCount--;
    Plan_Save();
    return true;
}

void Plan_SetActive(int index) {
    PlanState* state = getState();
    state->activePlanIndex = (index >= 0 && index < state->planCount) ? index : -1;
    if(state->activePlanIndex >= 0) { Plan_CheckNewDay(); }
    Plan_Save();
}

LearningPlan* Plan_GetActive(void) {
    PlanState* state = getState();
    if(state->activePlanIndex < 0) { return NULL; }
    return &state->plans[state->activePlanIndex];
}

int Plan_GetActiveIndex(void) { return getState()->activePlanIndex; }

void Plan_AddStudiedToday(int count) {
    LearningPlan* p = Plan_GetActive();
    if(p == NULL) { return ; }
    p->studiedToday += count;
    p->lastStudyDate = get_today_date();
    Plan_Save();
}

void Plan_CheckNewDay(void) {
    LearningPlan* p = Plan_GetActive();
    if(p == NULL) { return ; }
    int today = get_today_date();
    if(p->lastStudyDate != today) {
        if(p->lastStudyDate > 0) { p->currentDay++; }
        p->studiedToday = 0;
        p->lastStudyDate = today;
        Plan_Save();
    }
}

int Plan_GetRemainingToday(void) {
    LearningPlan* p = Plan_GetActive();
    if(p == NULL) { return 0; }
    int r = p->dailyWordCount - p->studiedToday;
    return r > 0 ? r : 0;
}

void Plan_AddDefaults(void) {
    PlanState* state = getState();
    for (int i = 0; i < 4 && state->planCount < MAX_PLANS; i++) {
        LearningPlan* p = &state->plans[state->planCount];
        strncpy(p->name, g_defaultPlans[i].name, PLAN_NAME_MAX - 1);
        p->dailyWordCount = g_defaultPlans[i].daily;
        p->totalDays = g_defaultPlans[i].days;
        p->currentDay = 0;
        p->createdAt = time(NULL);
        p->lastStudyDate = 0;
        p->studiedToday = 0;
        state->planCount++;
    }
    if(state->planCount > 0) { Plan_SetActive(0); }
}
