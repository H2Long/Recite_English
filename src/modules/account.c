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
    // ---- 参数合法性校验 ----
    if (username == NULL || password == NULL) return false;
    if (strlen(username) == 0 || strlen(password) == 0) return false;
    if (strlen(username) >= MAX_USERNAME || strlen(password) >= MAX_PASSWORD) return false;
    if (s->userCount >= MAX_USERS) return false;          // 用户数已达上限
    if (find_user(username) >= 0) return false;            // 用户名已存在

    // ---- 在 users 数组末尾添加新用户 ----
    User* u = &s->users[s->userCount];
    strncpy(u->username, username, MAX_USERNAME - 1);      // 复制用户名（安全截断）
    hash_password(password, u->passwordHash, MAX_PASSWORD); // 计算并存储密码哈希
    u->createdTime = time(NULL);                            // 记录当前时间为注册时间
    u->lastLoginTime = 0;                                   // 新账号从未登录过
    s->userCount++;

    Account_Save();  // 持久化到文件
    printf("INFO: User '%s' registered successfully\n", username);
    return true;
}

/**
 * Account_Login - 用户登录验证
 *
 * 验证流程：
 *   1. 按用户名查找用户（find_user）
 *   2. 对输入的明文密码进行哈希
 *   3. 比对哈希值与数据库中存储的哈希
 *   4. 匹配则设置登录状态，更新 lastLoginTime
 *
 * 注意：密码在网络中或内存中以明文形式短暂存在（参数传入），
 * 只有存储和比对时使用哈希值。在本地单机环境中这是可接受的。
 *
 * @param username 用户名
 * @param password 明文密码（函数内部立即哈希）
 * @return true 登录成功，false 登录失败（用户名不存在或密码错误）
 */
bool Account_Login(const char* username, const char* password) {
    AccountState* s = getState();
    if (username == NULL || password == NULL) return false;

    // 步骤1：查找用户。find_user 返回索引或 -1
    int idx = find_user(username);
    if (idx < 0) return false;  // 用户名不存在

    // 步骤2：哈希输入的密码，准备比对
    char hash[MAX_PASSWORD];
    hash_password(password, hash, MAX_PASSWORD);

    // 步骤3：比对哈希。字符串完全匹配才算通过
    if (strcmp(s->users[idx].passwordHash, hash) != 0) {
        return false;  // 密码错误
    }

    // 步骤4：认证通过，更新登录状态
    s->currentUserIndex = idx;      // 设置为当前用户
    s->isLoggedIn = true;           // 标记为已登录
    s->users[idx].lastLoginTime = time(NULL);  // 记录本次登录时间
    Account_Save();                 // 持久化（保存最新登录时间）
    printf("INFO: User '%s' logged in\n", username);
    return true;
}

/**
 * Account_Logout - 用户登出
 *
 * 清除当前登录状态，不影响其他用户数据。
 * 登出后 isLoggedIn = false，currentUserIndex = -1。
 * 注意：登出不会关闭用户已打开的学习进度文件，
 * 调用方需额外调用 setProgressFilePath("./data/progress.txt") 切换回默认进度。
 */
void Account_Logout(void) {
    AccountState* s = getState();
    if (s->currentUserIndex >= 0) {
        printf("INFO: User '%s' logged out\n",
               s->users[s->currentUserIndex].username);
    }
    s->currentUserIndex = -1;  // 清除当前用户索引
    s->isLoggedIn = false;      // 标记为未登录
}

/**
 * Account_IsLoggedIn - 检查当前是否已登录
 *
 * 简单返回 isLoggedIn 字段的值。配合 Account_GetCurrentIndex()
 * 可获取当前登录用户的具体信息（如用户名、注册时间等）。
 *
 * @return true 当前有用户在线，false 未登录
 */
bool Account_IsLoggedIn(void) {
    return getState()->isLoggedIn;
}

/**
 * Account_GetCurrentUser - 获取当前登录用户名
 *
 * @return 用户名字符串指针（指向 users 数组中的 username 字段）
 *         未登录时返回空字符串 ""（而非 NULL，方便调用方无需判空）
 */
const char* Account_GetCurrentUser(void) {
    AccountState* s = getState();
    if (!s->isLoggedIn || s->currentUserIndex < 0) {
        return "";  // 未登录时返回空字符串，而非 NULL
    }
    return s->users[s->currentUserIndex].username;
}

/**
 * Account_GetCurrentIndex - 获取当前登录用户在 users 数组中的索引
 *
 * @return 用户索引（0 ~ userCount-1），未登录返回 -1
 */
int Account_GetCurrentIndex(void) {
    AccountState* s = getState();
    return s->isLoggedIn ? s->currentUserIndex : -1;
}

/**
 * Account_GetProgressPath - 获取当前用户的专属进度文件路径
 *
 * 登录用户：progress_<用户名>.txt（如 progress_hhlong.txt）
 * 未登录：  progress.txt（默认公共进度）
 *
 * 此路径用于 words.c 中的 setProgressFilePath()，实现多用户进度隔离。
 *
 * @param buffer 输出缓冲区，存放生成的路径字符串
 * @param size   缓冲区大小（应至少 256 字节）
 */
void Account_GetProgressPath(char* buffer, int size) {
    AccountState* s = getState();
    if (s->isLoggedIn) {
        snprintf(buffer, size, "./progress_%s.txt",
                 s->users[s->currentUserIndex].username);
    } else {
        snprintf(buffer, size, "%s", "./progress.txt");
    }
}

/**
 * Account_GetPlanPath - 获取当前用户的专属计划文件路径
 *
 * 登录用户：plans_<用户名>.txt（如 plans_hhlong.txt）
 * 未登录：  plans.txt（默认公共计划）
 *
 * 此路径用于 plan.c 中的 Plan_SetFilePath()，实现多用户计划隔离。
 *
 * @param buffer 输出缓冲区
 * @param size   缓冲区大小
 */
void Account_GetPlanPath(char* buffer, int size) {
    AccountState* s = getState();
    if (s->isLoggedIn) {
        snprintf(buffer, size, "./plans_%s.txt",
                 s->users[s->currentUserIndex].username);
    } else {
        snprintf(buffer, size, "%s", "./plans.txt");
    }
}
