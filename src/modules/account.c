// 账号系统 — 注册/登录/登出, djb2哈希

#include "account.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static AccountState* g_pState = NULL;
static AccountState g_internalState;

static AccountState* getState(void) { return g_pState ? g_pState : &g_internalState; }

// djb2 哈希
static unsigned long hash_string(const char* str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) { hash = ((hash << 5) + hash) + c; }
    return hash;
}

static int find_user(const char* username) {
    AccountState* state = getState();
    for (int i = 0; i < state->userCount; i++) {
        if(strcmp(state->users[i].username, username) == 0) { return i; }
    }
    return -1;
}

void Account_SetState(AccountState* state) { g_pState = state; }

void Account_Init(void) {
    AccountState* state = getState();
    state->userCount = 0;
    state->currentUserIndex = -1;
    state->isLoggedIn = false;

    FILE* fp = fopen(ACCOUNT_FILE, "r");
    if(fp == NULL) {
        Account_Register("admin", "admin");
        Account_Save();
        return ;
    }
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        int len = strlen(line);
        if(len > 0 && line[len-1] == '\n') { line[len-1] = '\0'; }
        if(strlen(line) == 0) { continue; }
        char* f[6] = {NULL};
        int fi = 0;
        char* tok = strtok(line, "|");
        while (tok && fi < 6) { f[fi++] = tok; tok = strtok(NULL, "|"); }
        if(fi >= 4 && state->userCount < MAX_USERS) {
            User* u = &state->users[state->userCount];
            strncpy(u->username, f[0], MAX_USERNAME - 1);
            strncpy(u->passwordHash, f[1], MAX_PASSWORD - 1);
            u->createdTime = (time_t)atol(f[2]);
            u->lastLoginTime = (time_t)atol(f[3]);
            u->selectWordCorrect = fi >= 5 ? atoi(f[4]) : 0;
            u->selectWordTotal = fi >= 6 ? atoi(f[5]) : 0;
            state->userCount++;
        }
    }
    fclose(fp);
}

void Account_Save(void) {
    AccountState* state = getState();
    FILE* fp = fopen(ACCOUNT_FILE, "w");
    if(fp == NULL) { return ; }
    for (int i = 0; i < state->userCount; i++) {
        User* u = &state->users[i];
        fprintf(fp, "%s|%s|%ld|%ld|%d|%d\n",
            u->username, u->passwordHash, (long)u->createdTime,
            (long)u->lastLoginTime, u->selectWordCorrect, u->selectWordTotal);
    }
    fclose(fp);
}

bool Account_Register(const char* username, const char* password) {
    if(username == NULL || password == NULL) { return false; }
    if(strlen(username) == 0 || strlen(password) == 0) { return false; }
    AccountState* state = getState();
    if(state->userCount >= MAX_USERS) { return false; }
    if(find_user(username) >= 0) { return false; }
    User* u = &state->users[state->userCount];
    strncpy(u->username, username, MAX_USERNAME - 1);
    unsigned long h = hash_string(password);
    snprintf(u->passwordHash, MAX_PASSWORD, "%lu", h);
    u->createdTime = time(NULL);
    u->lastLoginTime = 0;
    u->selectWordCorrect = 0;
    u->selectWordTotal = 0;
    state->userCount++;
    Account_Save();
    return true;
}

bool Account_Login(const char* username, const char* password) {
    if(username == NULL || password == NULL) { return false; }
    int idx = find_user(username);
    if(idx < 0) { return false; }
    char hash[MAX_PASSWORD];
    unsigned long h = hash_string(password);
    snprintf(hash, MAX_PASSWORD, "%lu", h);
    AccountState* state = getState();
    if(strcmp(state->users[idx].passwordHash, hash) != 0) { return false; }
    state->currentUserIndex = idx;
    state->isLoggedIn = true;
    state->users[idx].lastLoginTime = time(NULL);
    Account_Save();
    return true;
}

void Account_Logout(void) {
    AccountState* state = getState();
    state->currentUserIndex = -1;
    state->isLoggedIn = false;
}

bool Account_IsLoggedIn(void) { return getState()->isLoggedIn; }

const char* Account_GetCurrentUser(void) {
    AccountState* state = getState();
    if(!state->isLoggedIn || state->currentUserIndex < 0) { return ""; }
    return state->users[state->currentUserIndex].username;
}

int Account_GetCurrentIndex(void) {
    return getState()->isLoggedIn ? getState()->currentUserIndex : -1;
}

void Account_GetProgressPath(char* buf, int size) {
    AccountState* state = getState();
    if(state->isLoggedIn) {
        snprintf(buf, size, "./progress_%s.txt", state->users[state->currentUserIndex].username);
    }
    else { strcpy(buf, "./progress.txt"); }
}

void Account_GetPlanPath(char* buf, int size) {
    AccountState* state = getState();
    if(state->isLoggedIn) {
        snprintf(buf, size, "./plans_%s.txt", state->users[state->currentUserIndex].username);
    }
    else { strcpy(buf, "./plans.txt"); }
}
