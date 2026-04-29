// ============================================================================
// 账号管理系统 - 库实现
// 功能：用户注册、登录认证、多用户进度文件管理
// ============================================================================

#include "account.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// 内部状态
// ============================================================================

static AccountState* g_pState = NULL;   // 指向外部状态（由 AppState 提供）
static AccountState g_internalState;    // 内部备用状态

// ============================================================================
// 内部辅助函数
// ============================================================================

static AccountState* getState(void) {
    return g_pState ? g_pState : &g_internalState;
}

/**
 * 简单字符串哈希（djb2 算法）
 * 用于密码存储，非加密安全但适合本地应用
 */
static unsigned long hash_string(const char* str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

/**
 * 查找用户索引
 * @param username 用户名
 * @return 索引，-1表示未找到
 */
static int find_user(const char* username) {
    AccountState* s = getState();
    for (int i = 0; i < s->userCount; i++) {
        if (strcmp(s->users[i].username, username) == 0) {
            return i;
        }
    }
    return -1;
}

/**
 * 生成密码哈希字符串
 */
static void hash_password(const char* password, char* output, int outputSize) {
    unsigned long h = hash_string(password);
    snprintf(output, outputSize, "%lu", h);
}

// ============================================================================
// 状态绑定
// ============================================================================

/**
 * 设置账号状态指针（绑定到外部状态）
 * 如果不需要外部状态，可以不调用此函数，库会使用内部备用状态
 */
void Account_SetState(AccountState* state) {
    g_pState = state;
}

// ============================================================================
// 账号库公共接口实现
// ============================================================================

void Account_Init(void) {
    AccountState* s = getState();
    s->userCount = 0;
    s->currentUserIndex = -1;
    s->isLoggedIn = false;

    FILE* fp = fopen(ACCOUNT_FILE, "r");
    if (fp == NULL) {
        // 首次使用，创建默认账号
        printf("INFO: No account file found, creating default account\n");
        Account_Register("admin", "admin");
        Account_Save();
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        // 去除换行符
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') line[len-1] = '\0';
        if (strlen(line) == 0) continue;

        // 格式: username|passwordHash|createdTime|lastLoginTime
        char* fields[4] = {NULL, NULL, NULL, NULL};
        int fieldIdx = 0;
        char* token = strtok(line, "|");
        while (token != NULL && fieldIdx < 4) {
            fields[fieldIdx++] = token;
            token = strtok(NULL, "|");
        }

        if (fieldIdx >= 4 && s->userCount < MAX_USERS) {
            User* u = &s->users[s->userCount];
            strncpy(u->username, fields[0], MAX_USERNAME - 1);
            strncpy(u->passwordHash, fields[1], MAX_PASSWORD - 1);
            u->createdTime = (time_t)atol(fields[2]);
            u->lastLoginTime = (time_t)atol(fields[3]);
            s->userCount++;
        }
    }
    fclose(fp);
    printf("INFO: Loaded %d user(s) from %s\n", s->userCount, ACCOUNT_FILE);
}

void Account_Save(void) {
    AccountState* s = getState();
    FILE* fp = fopen(ACCOUNT_FILE, "w");
    if (fp == NULL) {
        printf("WARNING: Cannot save accounts to %s\n", ACCOUNT_FILE);
        return;
    }

    for (int i = 0; i < s->userCount; i++) {
        User* u = &s->users[i];
        fprintf(fp, "%s|%s|%ld|%ld\n",
                u->username,
                u->passwordHash,
                (long)u->createdTime,
                (long)u->lastLoginTime);
    }

    fclose(fp);
}

bool Account_Register(const char* username, const char* password) {
    AccountState* s = getState();
    if (username == NULL || password == NULL) return false;
    if (strlen(username) == 0 || strlen(password) == 0) return false;
    if (strlen(username) >= MAX_USERNAME || strlen(password) >= MAX_PASSWORD) return false;
    if (s->userCount >= MAX_USERS) return false;
    if (find_user(username) >= 0) return false;  // 用户名已存在

    User* u = &s->users[s->userCount];
    strncpy(u->username, username, MAX_USERNAME - 1);
    hash_password(password, u->passwordHash, MAX_PASSWORD);
    u->createdTime = time(NULL);
    u->lastLoginTime = 0;
    s->userCount++;

    Account_Save();
    printf("INFO: User '%s' registered successfully\n", username);
    return true;
}

bool Account_Login(const char* username, const char* password) {
    AccountState* s = getState();
    if (username == NULL || password == NULL) return false;

    int idx = find_user(username);
    if (idx < 0) return false;

    char hash[MAX_PASSWORD];
    hash_password(password, hash, MAX_PASSWORD);

    if (strcmp(s->users[idx].passwordHash, hash) != 0) {
        return false;
    }

    s->currentUserIndex = idx;
    s->isLoggedIn = true;
    s->users[idx].lastLoginTime = time(NULL);
    Account_Save();
    printf("INFO: User '%s' logged in\n", username);
    return true;
}

void Account_Logout(void) {
    AccountState* s = getState();
    if (s->currentUserIndex >= 0) {
        printf("INFO: User '%s' logged out\n",
               s->users[s->currentUserIndex].username);
    }
    s->currentUserIndex = -1;
    s->isLoggedIn = false;
}

bool Account_IsLoggedIn(void) {
    return getState()->isLoggedIn;
}

const char* Account_GetCurrentUser(void) {
    AccountState* s = getState();
    if (!s->isLoggedIn || s->currentUserIndex < 0) {
        return "";
    }
    return s->users[s->currentUserIndex].username;
}

int Account_GetCurrentIndex(void) {
    AccountState* s = getState();
    return s->isLoggedIn ? s->currentUserIndex : -1;
}

void Account_GetProgressPath(char* buffer, int size) {
    AccountState* s = getState();
    if (s->isLoggedIn) {
        snprintf(buffer, size, "./progress_%s.txt",
                 s->users[s->currentUserIndex].username);
    } else {
        snprintf(buffer, size, "%s", "./progress.txt");
    }
}
