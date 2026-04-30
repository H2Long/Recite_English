// ============================================================================
// 账号管理系统 - 库头文件
// 功能：提供完整的用户账号管理功能，包括注册、登录/登出、
//       多用户学习进度隔离（每个用户独立进度文件/计划文件）
//
// 设计模式：外部状态绑定
//   本模块采用"外部状态绑定"模式。通过 Account_SetState() 将模块状态
//   绑定到 AppState 中的 account 字段。如果不调用 SetState，
//   模块使用内部静态备用状态运行。
//
// 密码策略：
//   使用 djb2 哈希算法存储密码哈希值，不保存明文。
//   注意：djb2 是非加密哈希（适用于哈希表），不适合高安全性场景，
//   但对于本地单机应用已足够防止无意中的密码窥视。
//
// 数据文件格式 (ACCOUNT_FILE = "./data/accounts.txt")：
//   username|passwordHash|createdTime|lastLoginTime|selectWordCorrect|selectWordTotal
//   每行一个用户，'|' 分隔。文件首次运行时自动创建。
// ============================================================================

#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <stdbool.h>  /* bool/true/false */
#include <time.h>     /* time_t 时间戳类型 */

// ============================================================================
// 常量定义
// ============================================================================

#define MAX_USERS        50      /* 用户数组最大容量，超出时注册返回 false */
#define MAX_USERNAME     32      /* 用户名最大长度（含 '\0'） */
#define MAX_PASSWORD     64      /* 密码哈希字符串最大长度（含 '\0'），djb2 输出约 20 字符 */
#define ACCOUNT_FILE     "./data/accounts.txt"   /* 账号数据持久化文件路径 */

// ============================================================================
// 数据结构
// ============================================================================

/**
 * User - 单用户信息结构体
 *
 * 字段顺序与 ACCOUNT_FILE 中的顺序一致。
 * 修改此结构体时需同步修改 Account_Init() 和 Account_Save()。
 */
typedef struct {
    char username[MAX_USERNAME];          /* 用户名：唯一标识，不能与现有用户重复 */
    char passwordHash[MAX_PASSWORD];      /* 密码哈希值：djb2 算法，存储为十进制字符串 */
    time_t createdTime;                   /* 注册时间：Unix 时间戳（秒），0 表示未知 */
    time_t lastLoginTime;                 /* 上次登录时间：Unix 时间戳，0 表示从未登录 */
    int selectWordCorrect;               /* "选词背单词"模式累计正确数（跨会话） */
    int selectWordTotal;                 /* "选词背单词"模式累计总题数（跨会话） */
} User;

/**
 * AccountState - 账号系统整体状态
 *
 * 管理所有用户数组和当前登录会话。
 * 支持单用户登录（同一时间一个用户在线）。
 */
typedef struct {
    User users[MAX_USERS];                /* 用户数组：固定容量线性存储 */
    int userCount;                        /* 当前用户总数（0 ~ MAX_USERS） */
    int currentUserIndex;                 /* 当前登录用户索引（-1 = 未登录） */
    bool isLoggedIn;                      /* 是否已登录（true 时 currentUserIndex >= 0） */
} AccountState;

// ============================================================================
// 公共接口
// ============================================================================

void Account_SetState(AccountState* state);                    /* 绑定状态到 AppState */
void Account_Init(void);                                       /* 从文件加载用户数据 */
void Account_Save(void);                                       /* 保存用户数据到文件 */

bool Account_Register(const char* username, const char* password);  /* 注册新用户 */
bool Account_Login(const char* username, const char* password);     /* 登录验证 */
void Account_Logout(void);                                          /* 登出 */

bool Account_IsLoggedIn(void);                    /* 检查是否已登录 */
const char* Account_GetCurrentUser(void);         /* 获取当前用户名 */
int Account_GetCurrentIndex(void);                /* 获取当前用户索引 */

void Account_GetProgressPath(char* buffer, int size);  /* 获取用户专属进度文件路径 */
void Account_GetPlanPath(char* buffer, int size);       /* 获取用户专属计划文件路径 */

#endif // ACCOUNT_H
