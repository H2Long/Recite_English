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

/**
 * getState - 获取当前有效的账号状态指针
 * 
 * 优先返回外部绑定的状态（由 AppState 通过 Account_SetState 传入），
 * 如果未绑定则使用内部备用状态。这种设计使 account.c 既可以作为
 * 独立库运行（使用内部状态），也可以集成到 AppState 中（使用外部状态）。
 * 
 * @return AccountState* 当前有效的账号状态指针
 */
static AccountState* getState(void) {
    return g_pState ? g_pState : &g_internalState;
}

/**
 * hash_string - djb2 字符串哈希算法
 * 
 * 经典的字符串哈希算法，由 Daniel J. Bernstein 提出。
 * 特点：简单快速、分布均匀、冲突率低。
 * 用于密码哈希存储，注意此算法不是加密安全的（不适合生产环境密码存储），
 * 但对于本地单机应用来说足够使用。
 * 
 * 算法原理：hash(i) = hash(i-1) * 33 + str[i]
 * 
 * @param str 要哈希的字符串
 * @return unsigned long 哈希值
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
 * find_user - 按用户名查找用户索引
 * 
 * 遍历用户数组进行精确匹配。
 * 
 * @param username 要查找的用户名
 * @return int 用户索引（0 ~ userCount-1），-1 表示未找到
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
 * hash_password - 生成密码的哈希字符串
 * 
 * 将明文密码通过 djb2 哈希算法转换为哈希字符串，
 * 存储到输出缓冲区中。密码不以明文形式存储。
 * 
 * @param password 明文密码
 * @param output 输出缓冲区（存放哈希字符串）
 * @param outputSize 输出缓冲区大小
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

        // 格式: username|passwordHash|createdTime|lastLoginTime|selectWordCorrect|selectWordTotal
        char* fields[6] = {NULL, NULL, NULL, NULL, NULL, NULL};
        int fieldIdx = 0;
        char* token = strtok(line, "|");
        while (token != NULL && fieldIdx < 6) {
            fields[fieldIdx++] = token;
            token = strtok(NULL, "|");
        }

        if (fieldIdx >= 4 && s->userCount < MAX_USERS) {
            User* u = &s->users[s->userCount];
            strncpy(u->username, fields[0], MAX_USERNAME - 1);
            strncpy(u->passwordHash, fields[1], MAX_PASSWORD - 1);
            u->createdTime = (time_t)atol(fields[2]);
            u->lastLoginTime = (time_t)atol(fields[3]);
            u->selectWordCorrect = (fieldIdx >= 5) ? atoi(fields[4]) : 0;
            u->selectWordTotal = (fieldIdx >= 6) ? atoi(fields[5]) : 0;
            s->userCount++;
        }
    }
    fclose(fp);
    printf("INFO: Loaded %d user(s) from %s\n", s->userCount, ACCOUNT_FILE);
}

/**
 * Account_Save - 保存所有用户账号数据到文件
 * 
 * 将当前内存中的用户列表写入 ACCOUNT_FILE，
 * 格式：username|passwordHash|createdTime|lastLoginTime|selectWordCorrect|selectWordTotal
 * 每行一个用户。此函数在账号注册、登录后自动调用。
 */
void Account_Save(void) {
    AccountState* s = getState();
    FILE* fp = fopen(ACCOUNT_FILE, "w");
    if (fp == NULL) {
        printf("WARNING: Cannot save accounts to %s\n", ACCOUNT_FILE);
        return;
    }

    for (int i = 0; i < s->userCount; i++) {
        User* u = &s->users[i];
        fprintf(fp, "%s|%s|%ld|%ld|%d|%d\n",
                u->username,
                u->passwordHash,
                (long)u->createdTime,
                (long)u->lastLoginTime,
                u->selectWordCorrect,
                u->selectWordTotal);
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

void Account_GetPlanPath(char* buffer, int size) {
    AccountState* s = getState();
    if (s->isLoggedIn) {
        snprintf(buffer, size, "./plans_%s.txt",
                 s->users[s->currentUserIndex].username);
    } else {
        snprintf(buffer, size, "%s", "./plans.txt");
    }
}
