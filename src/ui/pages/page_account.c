// ============================================================================
// 账号管理页面（主页面 + 登录 + 注册）
// ============================================================================

#include "pages.h"

/** g_accountSubPage - 账号管理子页面状态（0=主页，1=登录页，2=注册页） */
static int g_accountSubPage = 0;

/** g_loginForm - 登录表单状态 */
static struct { char username[32]; char password[64]; UITextBoxState userState; UITextBoxState passState; bool initialized; } g_loginForm = {0};

/** g_regForm - 注册表单状态 */
static struct { char username[32]; char password[64]; char confirm[64]; UITextBoxState userState; UITextBoxState passState; UITextBoxState confirmState; bool initialized; } g_regForm = {0};

void MenuAccount_Show(void) {
    if (g_accountSubPage == 1) { MenuLogin_Show(); return; }
    if (g_accountSubPage == 2) { MenuRegister_Show(); return; }
    Rectangle contentRect = {250, 80, SCREEN_WIDTH - 270, SCREEN_HEIGHT - 100};
    UILayout layout = UIBeginLayout(contentRect, UI_DIR_VERTICAL, 30, 50);
    Rectangle titleRect = UILayoutNext(&layout, -1, 60);
    Vector2 tSize = MeasureTextAuto(u8"账号管理", 42, 1);
    DrawTextAuto(u8"账号管理", (Vector2){contentRect.x + 50, titleRect.y}, 42, 1, STYLE->theme.textPrimary);
    Rectangle panelRect = UILayoutNext(&layout, -1, 260);
    DrawRectangleRounded(panelRect, 0.1f, 12, STYLE->theme.panelBg);
    UILayout panelLayout = UIBeginLayout(panelRect, UI_DIR_VERTICAL, 25, 40);
    if (ACCOUNT.isLoggedIn) {
        Rectangle userSection = UILayoutNext(&panelLayout, -1, 100);
        DrawCircleLines(userSection.x + 40, userSection.y + 40, 30, STYLE->theme.primary);
        DrawTextAuto(Account_GetCurrentUser(), (Vector2){userSection.x + 90, userSection.y + 25}, 36, 1, STYLE->theme.primary);
        Rectangle infoSection = UILayoutNext(&panelLayout, -1, 60);
        char infoText[256]; int idx = Account_GetCurrentIndex();
        if (idx >= 0) { char createdStr[64]; strftime(createdStr, sizeof(createdStr), "%Y-%m-%d", localtime(&ACCOUNT.users[idx].createdTime)); snprintf(infoText, sizeof(infoText), u8"注册时间: %s  |  学习数据: progress_%s.txt", createdStr, Account_GetCurrentUser()); }
        else snprintf(infoText, sizeof(infoText), "%s", u8"当前用户");
        DrawTextAuto(infoText, (Vector2){infoSection.x, infoSection.y}, 20, 1, STYLE->theme.textSecondary);
        Rectangle btnSection = UILayoutNext(&panelLayout, -1, 60);
        UILayout btnLayout = UIBeginLayout(btnSection, UI_DIR_HORIZONTAL, 25, 0);
        Rectangle logoutBtn = UILayoutNext(&btnLayout, 160, 50);
        if (UIButton(u8"登出", logoutBtn, STYLE, UI_STATE, 300)) { Account_Logout(); setProgressFilePath("./data/progress.txt"); loadProgress(); Plan_SetFilePath("./data/plans.txt"); snprintf(LOGIN_MSG, 128, "%s", u8"已登出"); }
        Rectangle switchBtn = UILayoutNext(&btnLayout, 200, 50);
        if (UIButton(u8"切换账号", switchBtn, STYLE, UI_STATE, 301)) { Account_Logout(); setProgressFilePath("./data/progress.txt"); loadProgress(); Plan_SetFilePath("./data/plans.txt"); g_accountSubPage = 1; }
    } else {
        Rectangle welcomeSection = UILayoutNext(&panelLayout, -1, 80);
        DrawTextAuto(u8"欢迎使用背单词软件", (Vector2){welcomeSection.x, welcomeSection.y}, 32, 1, STYLE->theme.primary);
        DrawTextAuto(u8"创建账号或登录后，学习进度将独立保存", (Vector2){welcomeSection.x, welcomeSection.y + 45}, 20, 1, STYLE->theme.textSecondary);
        Rectangle btnSection = UILayoutNext(&panelLayout, -1, 70);
        UILayout btnLayout = UIBeginLayout(btnSection, UI_DIR_HORIZONTAL, 30, 50);
        Rectangle loginBtn = UILayoutNext(&btnLayout, 230, 55);
        if (UIButton(u8"登录", loginBtn, STYLE, UI_STATE, 302)) { g_accountSubPage = 1; memset(&g_loginForm, 0, sizeof(g_loginForm)); g_loginForm.initialized = true; g_loginForm.userState.hasFocus = true; snprintf(g_app.loginMsg, 128, "%s", ""); }
        Rectangle registerBtn = UILayoutNext(&btnLayout, 230, 55);
        if (UIButton(u8"注册", registerBtn, STYLE, UI_STATE, 303)) { g_accountSubPage = 2; memset(&g_regForm, 0, sizeof(g_regForm)); g_regForm.initialized = true; g_regForm.userState.hasFocus = true; snprintf(g_app.loginMsg, 128, "%s", ""); }
    }
    Rectangle userListLabel = UILayoutNext(&layout, -1, 35);
    DrawTextAuto(u8"全部注册用户", (Vector2){userListLabel.x, userListLabel.y}, 24, 1, STYLE->theme.textSecondary);
    Rectangle userListPanel = UILayoutNext(&layout, -1, -1);
    DrawRectangleRounded(userListPanel, 0.1f, 12, STYLE->theme.panelBg);
    float listContentH = ACCOUNT.userCount * 45.0f + 20;
    UIScrollView userSV = {0}; userSV.viewport = userListPanel; userSV.contentSize = (Vector2){userListPanel.width - 30, listContentH};
    UIBeginScrollView(&userSV, userListPanel, userSV.contentSize);
    for (int i = 0; i < ACCOUNT.userCount; i++) {
        Rectangle rowRect = {userListPanel.x + 15, userListPanel.y + 10 + i * 45 - userSV.scrollOffset.y, userListPanel.width - 30, 40};
        if (i % 2 == 0) DrawRectangleRec(rowRect, Fade(STYLE->theme.inputBg, 0.5f));
        DrawTextAuto(ACCOUNT.users[i].username, (Vector2){rowRect.x + 10, rowRect.y + 10}, 22, 1, STYLE->theme.textPrimary);
        char timeStr[64];
        if (ACCOUNT.users[i].createdTime > 0) strftime(timeStr, sizeof(timeStr), "%Y-%m-%d", localtime(&ACCOUNT.users[i].createdTime));
        else snprintf(timeStr, sizeof(timeStr), "%s", u8"未知");
        DrawTextAuto(timeStr, (Vector2){rowRect.x + 200, rowRect.y + 10}, 20, 1, STYLE->theme.textSecondary);
        if (ACCOUNT.users[i].lastLoginTime > 0) { char loginStr[64]; strftime(loginStr, sizeof(loginStr), "%Y-%m-%d %H:%M", localtime(&ACCOUNT.users[i].lastLoginTime)); DrawTextAuto(loginStr, (Vector2){rowRect.x + 400, rowRect.y + 10}, 20, 1, STYLE->theme.textSecondary); }
        if (i == Account_GetCurrentIndex()) DrawTextAuto(u8"[当前]", (Vector2){rowRect.x + rowRect.width - 70, rowRect.y + 10}, 20, 1, STYLE->theme.primary);
    }
    UIEndScrollView(&userSV, STYLE, UI_STATE);
}

void MenuLogin_Show(void) {
    if (!g_loginForm.initialized) { memset(&g_loginForm, 0, sizeof(g_loginForm)); g_loginForm.initialized = true; g_loginForm.userState.hasFocus = true; }
    Rectangle contentRect = {250, 80, SCREEN_WIDTH - 270, SCREEN_HEIGHT - 100};
    UILayout layout = UIBeginLayout(contentRect, UI_DIR_VERTICAL, 30, 50);
    Rectangle titleRect = UILayoutNext(&layout, -1, 60);
    Vector2 tSize = MeasureTextAuto(u8"用户登录", 42, 1);
    DrawTextAuto(u8"用户登录", (Vector2){SCREEN_WIDTH/2 - tSize.x/2, titleRect.y}, 42, 1, STYLE->theme.primary);
    Rectangle panelRect = {SCREEN_WIDTH/2 - 250, 200, 500, 400};
    DrawRectangleRounded(panelRect, 0.1f, 12, STYLE->theme.panelBg);
    UILayout formLayout = UIBeginLayout(panelRect, UI_DIR_VERTICAL, 20, 40);
    Rectangle userLabel = UILayoutNext(&formLayout, -1, 35); DrawTextAuto(u8"用户名", (Vector2){userLabel.x, userLabel.y}, 24, 1, STYLE->theme.textSecondary);
    Rectangle userInput = UILayoutNext(&formLayout, -1, 50); strncpy(g_loginForm.userState.buffer, g_loginForm.username, sizeof(g_loginForm.userState.buffer) - 1);
    UITextBox(&g_loginForm.userState, userInput, STYLE, UI_STATE, false); strncpy(g_loginForm.username, g_loginForm.userState.buffer, 31);
    Rectangle passLabel = UILayoutNext(&formLayout, -1, 35); DrawTextAuto(u8"密码", (Vector2){passLabel.x, passLabel.y}, 24, 1, STYLE->theme.textSecondary);
    Rectangle passInput = UILayoutNext(&formLayout, -1, 50); strncpy(g_loginForm.passState.buffer, g_loginForm.password, sizeof(g_loginForm.passState.buffer) - 1);
    UITextBox(&g_loginForm.passState, passInput, STYLE, UI_STATE, true); strncpy(g_loginForm.password, g_loginForm.passState.buffer, 63);
    Rectangle msgRect = UILayoutNext(&formLayout, -1, 35);
    if (strlen(LOGIN_MSG) > 0) DrawTextAuto(LOGIN_MSG, (Vector2){msgRect.x, msgRect.y}, 20, 1, (strstr(LOGIN_MSG, "成功") || strstr(LOGIN_MSG, "欢迎")) ? STYLE->theme.success : STYLE->theme.error);
    Rectangle btnSection = UILayoutNext(&formLayout, -1, 55);
    UILayout btnLayout = UIBeginLayout(btnSection, UI_DIR_HORIZONTAL, 20, 0);
    Rectangle loginBtn = UILayoutNext(&btnLayout, 140, 50);
    if (UIButton(u8"登录", loginBtn, STYLE, UI_STATE, 310)) {
        if (strlen(g_loginForm.username) > 0 && strlen(g_loginForm.password) > 0) {
            if (Account_Login(g_loginForm.username, g_loginForm.password)) {
                char progressPath[256], planPath[256];
                Account_GetProgressPath(progressPath, sizeof(progressPath)); setProgressFilePath(progressPath); loadProgress();
                Account_GetPlanPath(planPath, sizeof(planPath)); Plan_SetFilePath(planPath);
                snprintf(LOGIN_MSG, 128, u8"欢迎回来，%s！", Account_GetCurrentUser());
                memset(&g_loginForm, 0, sizeof(g_loginForm)); g_loginForm.initialized = true; g_accountSubPage = 0;
                g_app.loginRequired = false; CURRENT_MENU = g_app.rootMenu;
            } else snprintf(LOGIN_MSG, 128, "%s", u8"登录失败：用户名或密码错误");
        } else snprintf(LOGIN_MSG, 128, "%s", u8"请输入用户名和密码");
    }
    Rectangle regBtn = UILayoutNext(&btnLayout, 140, 50);
    if (UIButton(u8"注册账号", regBtn, STYLE, UI_STATE, 311)) { g_accountSubPage = 2; memset(&g_regForm, 0, sizeof(g_regForm)); g_regForm.initialized = true; g_regForm.userState.hasFocus = true; snprintf(g_app.loginMsg, 128, "%s", ""); }
    Rectangle backBtn = UILayoutNext(&btnLayout, 120, 50);
    if (UIButton(u8"返回", backBtn, STYLE, UI_STATE, 312)) { g_accountSubPage = 0; memset(&g_loginForm, 0, sizeof(g_loginForm)); g_loginForm.initialized = true; snprintf(g_app.loginMsg, 128, "%s", ""); }
}

void MenuRegister_Show(void) {
    if (!g_regForm.initialized) { memset(&g_regForm, 0, sizeof(g_regForm)); g_regForm.initialized = true; g_regForm.userState.hasFocus = true; }
    Rectangle contentRect = {250, 80, SCREEN_WIDTH - 270, SCREEN_HEIGHT - 100};
    UILayout layout = UIBeginLayout(contentRect, UI_DIR_VERTICAL, 30, 50);
    Rectangle titleRect = UILayoutNext(&layout, -1, 60);
    Vector2 tSize = MeasureTextAuto(u8"用户注册", 42, 1);
    DrawTextAuto(u8"用户注册", (Vector2){SCREEN_WIDTH/2 - tSize.x/2, titleRect.y}, 42, 1, STYLE->theme.primary);
    Rectangle panelRect = {SCREEN_WIDTH/2 - 250, 200, 500, 450};
    DrawRectangleRounded(panelRect, 0.1f, 12, STYLE->theme.panelBg);
    UILayout formLayout = UIBeginLayout(panelRect, UI_DIR_VERTICAL, 18, 40);
    Rectangle userLabel = UILayoutNext(&formLayout, -1, 35); DrawTextAuto(u8"用户名", (Vector2){userLabel.x, userLabel.y}, 24, 1, STYLE->theme.textSecondary);
    Rectangle userInput = UILayoutNext(&formLayout, -1, 50); strncpy(g_regForm.userState.buffer, g_regForm.username, sizeof(g_regForm.userState.buffer) - 1);
    UITextBox(&g_regForm.userState, userInput, STYLE, UI_STATE, false); strncpy(g_regForm.username, g_regForm.userState.buffer, 31);
    Rectangle passLabel = UILayoutNext(&formLayout, -1, 35); DrawTextAuto(u8"密码", (Vector2){passLabel.x, passLabel.y}, 24, 1, STYLE->theme.textSecondary);
    Rectangle passInput = UILayoutNext(&formLayout, -1, 50); strncpy(g_regForm.passState.buffer, g_regForm.password, sizeof(g_regForm.passState.buffer) - 1);
    UITextBox(&g_regForm.passState, passInput, STYLE, UI_STATE, true); strncpy(g_regForm.password, g_regForm.passState.buffer, 63);
    Rectangle confirmLabel = UILayoutNext(&formLayout, -1, 35); DrawTextAuto(u8"确认密码", (Vector2){confirmLabel.x, confirmLabel.y}, 24, 1, STYLE->theme.textSecondary);
    Rectangle confirmInput = UILayoutNext(&formLayout, -1, 50); strncpy(g_regForm.confirmState.buffer, g_regForm.confirm, sizeof(g_regForm.confirmState.buffer) - 1);
    UITextBox(&g_regForm.confirmState, confirmInput, STYLE, UI_STATE, true); strncpy(g_regForm.confirm, g_regForm.confirmState.buffer, 63);
    Rectangle msgRect = UILayoutNext(&formLayout, -1, 35);
    if (strlen(LOGIN_MSG) > 0) DrawTextAuto(LOGIN_MSG, (Vector2){msgRect.x, msgRect.y}, 20, 1, strstr(LOGIN_MSG, "成功") ? STYLE->theme.success : STYLE->theme.error);
    Rectangle btnSection = UILayoutNext(&formLayout, -1, 55);
    UILayout btnLayout = UIBeginLayout(btnSection, UI_DIR_HORIZONTAL, 20, 0);
    Rectangle regBtn = UILayoutNext(&btnLayout, 140, 50);
    if (UIButton(u8"注册", regBtn, STYLE, UI_STATE, 320)) {
        if (strlen(g_regForm.username) == 0) snprintf(LOGIN_MSG, 128, "%s", u8"请输入用户名");
        else if (strlen(g_regForm.password) == 0) snprintf(LOGIN_MSG, 128, "%s", u8"请输入密码");
        else if (strcmp(g_regForm.password, g_regForm.confirm) != 0) snprintf(LOGIN_MSG, 128, "%s", u8"两次密码输入不一致");
        else if (Account_Register(g_regForm.username, g_regForm.password)) {
            Account_Login(g_regForm.username, g_regForm.password);
            char progressPath[256], planPath[256];
            Account_GetProgressPath(progressPath, sizeof(progressPath)); setProgressFilePath(progressPath); loadProgress();
            Account_GetPlanPath(planPath, sizeof(planPath)); Plan_SetFilePath(planPath);
            snprintf(LOGIN_MSG, 128, u8"注册成功，欢迎 %s！", g_regForm.username);
            memset(&g_regForm, 0, sizeof(g_regForm)); g_regForm.initialized = true; g_accountSubPage = 0;
            g_app.loginRequired = false; CURRENT_MENU = g_app.rootMenu;
        } else snprintf(LOGIN_MSG, 128, "%s", u8"注册失败：用户名已存在或无效");
    }
    Rectangle loginBtn = UILayoutNext(&btnLayout, 140, 50);
    if (UIButton(u8"返回登录", loginBtn, STYLE, UI_STATE, 321)) { g_accountSubPage = 1; memset(&g_loginForm, 0, sizeof(g_loginForm)); g_loginForm.initialized = true; g_loginForm.userState.hasFocus = true; snprintf(g_app.loginMsg, 128, "%s", ""); }
    Rectangle backBtn = UILayoutNext(&btnLayout, 120, 50);
    if (UIButton(u8"返回", backBtn, STYLE, UI_STATE, 322)) { g_accountSubPage = 0; memset(&g_regForm, 0, sizeof(g_regForm)); g_regForm.initialized = true; snprintf(g_app.loginMsg, 128, "%s", ""); }
}
