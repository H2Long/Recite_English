// 账号系统 — 注册/登录/登出, djb2哈希存储密码

#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <stdbool.h>
#include <time.h>

#define MAX_USERS     50
#define MAX_USERNAME  32
#define MAX_PASSWORD  64
#define ACCOUNT_FILE  "./data/accounts.txt"

typedef struct {
    char username[MAX_USERNAME];
    char passwordHash[MAX_PASSWORD];     // djb2哈希值
    time_t createdTime;
    time_t lastLoginTime;
    int selectWordCorrect;               // 选词背单词累计正确数
    int selectWordTotal;                 // 选词背单词累计总题数
} User;

typedef struct {
    User users[MAX_USERS];
    int userCount;
    int currentUserIndex;                // -1=未登录
    bool isLoggedIn;
} AccountState;

void Account_SetState(AccountState* state);
void Account_Init(void);
void Account_Save(void);

bool Account_Register(const char* username, const char* password);
bool Account_Login(const char* username, const char* password);
void Account_Logout(void);

bool Account_IsLoggedIn(void);
const char* Account_GetCurrentUser(void);
int Account_GetCurrentIndex(void);

void Account_GetProgressPath(char* buffer, int size);
void Account_GetPlanPath(char* buffer, int size);

#endif
