// ============================================================================
// 账号管理系统 - 库头文件
// 功能：用户注册、登录、登出、多用户学习进度管理
// ============================================================================

#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <stdbool.h>
#include <time.h>

// ============================================================================
// 常量定义
// ============================================================================

#define MAX_USERS        50      // 最大用户数
#define MAX_USERNAME     32      // 用户名最大长度
#define MAX_PASSWORD     64      // 密码最大长度
#define ACCOUNT_FILE     "./accounts.txt"   // 账号数据文件

// ============================================================================
// 数据结构
// ============================================================================

/**
 * 用户信息
 */
typedef struct {
    char username[MAX_USERNAME];          // 用户名
    char passwordHash[MAX_PASSWORD];      // 密码哈希值
    time_t createdTime;                   // 注册时间
    time_t lastLoginTime;                 // 上次登录时间
} User;

/**
 * 账号系统状态
 */
typedef struct {
    User users[MAX_USERS];                // 用户数组
    int userCount;                        // 当前用户总数
    int currentUserIndex;                 // 当前登录用户索引（-1表示未登录）
    bool isLoggedIn;                      // 是否已登录
} AccountState;

// ============================================================================
// 账号库公共接口
// ============================================================================

/**
 * 设置账号状态指针（绑定到 AppState 的状态）
 * @param state 外部 AccountState 指针，传入 &g_app.account
 */
void Account_SetState(AccountState* state);

/**
 * 初始化账号系统
 * 从账号文件加载用户数据
 */
void Account_Init(void);

/**
 * 保存账号数据到文件
 */
void Account_Save(void);

/**
 * 用户注册
 * @param username 用户名（不能为空，不能重复）
 * @param password 密码（不能为空）
 * @return true 注册成功，false 注册失败
 */
bool Account_Register(const char* username, const char* password);

/**
 * 用户登录
 * @param username 用户名
 * @param password 密码
 * @return true 登录成功，false 登录失败
 */
bool Account_Login(const char* username, const char* password);

/**
 * 用户登出
 */
void Account_Logout(void);

/**
 * 检查是否已登录
 * @return true 已登录
 */
bool Account_IsLoggedIn(void);

/**
 * 获取当前登录用户名
 * @return 用户名，未登录返回空字符串
 */
const char* Account_GetCurrentUser(void);

/**
 * 获取当前用户索引
 * @return 索引，未登录返回 -1
 */
int Account_GetCurrentIndex(void);

/**
 * 获取当前用户的进度文件路径
 * @param buffer 输出缓冲区
 * @param size 缓冲区大小
 */
void Account_GetProgressPath(char* buffer, int size);

#endif // ACCOUNT_H
