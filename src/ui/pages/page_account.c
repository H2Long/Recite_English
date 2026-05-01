#include "pages.h"

// 0=主页 1=登录页 2=注册页
static int g_accountSubPage = 0;

// 登录表单
static struct {
    char username[32], password[64];
    UITextBoxState userState, passState;
    bool ready;
} g_loginForm = {0};

// 注册表单
static struct {
    char username[32], password[64], confirm[64];
    UITextBoxState userState, passState, confirmState;
    bool ready;
} g_regForm = {0};

void MenuAccount_Show(void) {
    if(g_accountSubPage == 1) {
        MenuLogin_Show();
        return;
    }
    else if(g_accountSubPage == 2) {
        MenuRegister_Show();
        return;
    }

    Rectangle contentRect = {250, 80, SCREEN_WIDTH - 270, SCREEN_HEIGHT - 100};
    UILayout layout = UIBeginLayout(contentRect, UI_DIR_VERTICAL, 30, 50);

    // 标题
    Rectangle titleRect = UILayoutNext(&layout, -1, 60);
    DrawTextAuto(u8"账号管理", (Vector2){contentRect.x + 50, titleRect.y}, 42, 1, STYLE->theme.textPrimary);

    // 用户信息面板
    Rectangle panelRect = UILayoutNext(&layout, -1, 260);
    DrawRectangleRounded(panelRect, 0.1f, 12, STYLE->theme.panelBg);
    UILayout panelLayout = UIBeginLayout(panelRect, UI_DIR_VERTICAL, 25, 40);

    if(ACCOUNT.isLoggedIn) {
        // 头像 + 用户名
        Rectangle userSection = UILayoutNext(&panelLayout, -1, 100);
        DrawCircleLines(userSection.x + 40, userSection.y + 40, 30, STYLE->theme.primary);
        DrawTextAuto(Account_GetCurrentUser(),
            (Vector2){userSection.x + 90, userSection.y + 25}, 36, 1, STYLE->theme.primary);

        // 注册时间 + 数据文件
        Rectangle infoSection = UILayoutNext(&panelLayout, -1, 60);
        int idx = Account_GetCurrentIndex();
        char info[256];
        if(idx >= 0) {
            char timeStr[64];
            strftime(timeStr, sizeof(timeStr), "%Y-%m-%d",
                localtime(&ACCOUNT.users[idx].createdTime));
            snprintf(info, sizeof(info),
                u8"注册时间: %s  |  学习数据: progress_%s.txt",
                timeStr, Account_GetCurrentUser());
        }
        else {
            strcpy(info, u8"当前用户");
        }
        DrawTextAuto(info, (Vector2){infoSection.x, infoSection.y}, 20, 1,
            STYLE->theme.textSecondary);

        // 登出 / 切换账号
        Rectangle buttonSection = UILayoutNext(&panelLayout, -1, 60);
        UILayout buttonLayout = UIBeginLayout(buttonSection, UI_DIR_HORIZONTAL, 25, 0);

        if(UIButton(u8"登出", UILayoutNext(&buttonLayout, 160, 50), STYLE, UI_STATE, 300)) {
            Account_Logout();
            setProgressFilePath("./data/progress.txt");
            loadProgress();
            Plan_SetFilePath("./data/plans.txt");
            strcpy(LOGIN_MSG, u8"已登出");
        }
        if(UIButton(u8"切换账号", UILayoutNext(&buttonLayout, 200, 50), STYLE, UI_STATE, 301)) {
            Account_Logout();
            setProgressFilePath("./data/progress.txt");
            loadProgress();
            Plan_SetFilePath("./data/plans.txt");
            g_accountSubPage = 1;
        }
    }
    else {
        // 欢迎信息
        Rectangle welcomeSection = UILayoutNext(&panelLayout, -1, 80);
        DrawTextAuto(u8"欢迎使用背单词软件", (Vector2){welcomeSection.x, welcomeSection.y},
            32, 1, STYLE->theme.primary);
        DrawTextAuto(u8"创建账号或登录后，学习进度将独立保存",
            (Vector2){welcomeSection.x, welcomeSection.y + 45}, 20, 1,
            STYLE->theme.textSecondary);

        // 登录 / 注册按钮
        Rectangle buttonSection = UILayoutNext(&panelLayout, -1, 70);
        UILayout buttonLayout = UIBeginLayout(buttonSection, UI_DIR_HORIZONTAL, 30, 50);

        if(UIButton(u8"登录", UILayoutNext(&buttonLayout, 230, 55), STYLE, UI_STATE, 302)) {
            g_accountSubPage = 1;
            memset(&g_loginForm, 0, sizeof(g_loginForm));
            g_loginForm.ready = true;
            g_loginForm.userState.hasFocus = true;
            LOGIN_MSG[0] = '\0';
        }
        if(UIButton(u8"注册", UILayoutNext(&buttonLayout, 230, 55), STYLE, UI_STATE, 303)) {
            g_accountSubPage = 2;
            memset(&g_regForm, 0, sizeof(g_regForm));
            g_regForm.ready = true;
            g_regForm.userState.hasFocus = true;
            LOGIN_MSG[0] = '\0';
        }
    }

    // 全部注册用户列表
    Rectangle userListTitle = UILayoutNext(&layout, -1, 35);
    DrawTextAuto(u8"全部注册用户", (Vector2){userListTitle.x, userListTitle.y},
        24, 1, STYLE->theme.textSecondary);

    Rectangle userListPanel = UILayoutNext(&layout, -1, -1);
    DrawRectangleRounded(userListPanel, 0.1f, 12, STYLE->theme.panelBg);

    UIScrollView scrollView = {0};
    scrollView.viewport = userListPanel;
    scrollView.contentSize = (Vector2){userListPanel.width - 30,
        ACCOUNT.userCount * 45.0f + 20};
    UIBeginScrollView(&scrollView, userListPanel, scrollView.contentSize);

    for (int i = 0; i < ACCOUNT.userCount; i++) {
        Rectangle rowRect = {userListPanel.x + 15,
            userListPanel.y + 10 + i * 45 - scrollView.scrollOffset.y,
            userListPanel.width - 30, 40};

        if(i % 2 == 0) {
            DrawRectangleRec(rowRect, Fade(STYLE->theme.inputBg, 0.5f));
        }
        DrawTextAuto(ACCOUNT.users[i].username,
            (Vector2){rowRect.x + 10, rowRect.y + 10}, 22, 1, STYLE->theme.textPrimary);

        char timeStr[64];
        if(ACCOUNT.users[i].createdTime > 0) {
            strftime(timeStr, sizeof(timeStr), "%Y-%m-%d",
                localtime(&ACCOUNT.users[i].createdTime));
        }
        else {
            strcpy(timeStr, u8"未知");
        }
        DrawTextAuto(timeStr, (Vector2){rowRect.x + 200, rowRect.y + 10},
            20, 1, STYLE->theme.textSecondary);

        if(ACCOUNT.users[i].lastLoginTime > 0) {
            char loginStr[64];
            strftime(loginStr, sizeof(loginStr), "%Y-%m-%d %H:%M",
                localtime(&ACCOUNT.users[i].lastLoginTime));
            DrawTextAuto(loginStr, (Vector2){rowRect.x + 400, rowRect.y + 10},
                20, 1, STYLE->theme.textSecondary);
        }
        if(i == Account_GetCurrentIndex()) {
            DrawTextAuto(u8"[当前]", (Vector2){rowRect.x + rowRect.width - 70, rowRect.y + 10},
                20, 1, STYLE->theme.primary);
        }
    }
    UIEndScrollView(&scrollView, STYLE, UI_STATE);
}

void MenuLogin_Show(void) {
    if(!g_loginForm.ready) {
        memset(&g_loginForm, 0, sizeof(g_loginForm));
        g_loginForm.ready = true;
        g_loginForm.userState.hasFocus = true;
    }

    Rectangle contentRect = {250, 80, SCREEN_WIDTH - 270, SCREEN_HEIGHT - 100};
    UILayout layout = UIBeginLayout(contentRect, UI_DIR_VERTICAL, 30, 50);

    // 标题
    Vector2 titleSize = MeasureTextAuto(u8"用户登录", 42, 1);
    Rectangle titleRect = UILayoutNext(&layout, -1, 60);
    DrawTextAuto(u8"用户登录", (Vector2){SCREEN_WIDTH/2 - titleSize.x/2, titleRect.y},
        42, 1, STYLE->theme.primary);

    // 表单面板
    Rectangle panelRect = {SCREEN_WIDTH/2 - 250, 200, 500, 400};
    DrawRectangleRounded(panelRect, 0.1f, 12, STYLE->theme.panelBg);
    UILayout formLayout = UIBeginLayout(panelRect, UI_DIR_VERTICAL, 20, 40);

    // 用户名
    Rectangle userLabelRect = UILayoutNext(&formLayout, -1, 35);
    DrawTextAuto(u8"用户名", (Vector2){userLabelRect.x, userLabelRect.y},
        24, 1, STYLE->theme.textSecondary);
    Rectangle userInput = UILayoutNext(&formLayout, -1, 50);
    strcpy(g_loginForm.userState.buffer, g_loginForm.username);
    UITextBox(&g_loginForm.userState, userInput, STYLE, UI_STATE, false);
    strcpy(g_loginForm.username, g_loginForm.userState.buffer);

    // 密码
    Rectangle passLabelRect = UILayoutNext(&formLayout, -1, 35);
    DrawTextAuto(u8"密码", (Vector2){passLabelRect.x, passLabelRect.y},
        24, 1, STYLE->theme.textSecondary);
    Rectangle passInput = UILayoutNext(&formLayout, -1, 50);
    strcpy(g_loginForm.passState.buffer, g_loginForm.password);
    UITextBox(&g_loginForm.passState, passInput, STYLE, UI_STATE, true);
    strcpy(g_loginForm.password, g_loginForm.passState.buffer);

    // 提示消息
    Rectangle msgSection = UILayoutNext(&formLayout, -1, 35);
    if(LOGIN_MSG[0]) {
        bool ok = strstr(LOGIN_MSG, u8"成功") || strstr(LOGIN_MSG, u8"欢迎");
        DrawTextAuto(LOGIN_MSG, (Vector2){msgSection.x, msgSection.y}, 20, 1,
            ok ? STYLE->theme.success : STYLE->theme.error);
    }

    // 按钮行
    Rectangle buttonSection = UILayoutNext(&formLayout, -1, 55);
    UILayout buttonLayout = UIBeginLayout(buttonSection, UI_DIR_HORIZONTAL, 20, 0);

    if(UIButton(u8"登录", UILayoutNext(&buttonLayout, 140, 50), STYLE, UI_STATE, 310)) {
        if(!g_loginForm.username[0] || !g_loginForm.password[0]) {
            strcpy(LOGIN_MSG, u8"请输入用户名和密码");
        }
        else if(Account_Login(g_loginForm.username, g_loginForm.password)) {
            char progressPath[256], planPath[256];
            Account_GetProgressPath(progressPath, sizeof(progressPath));
            setProgressFilePath(progressPath);
            loadProgress();
            Account_GetPlanPath(planPath, sizeof(planPath));
            Plan_SetFilePath(planPath);
            snprintf(LOGIN_MSG, 128, u8"欢迎回来，%s！", Account_GetCurrentUser());
            memset(&g_loginForm, 0, sizeof(g_loginForm));
            g_loginForm.ready = true;
            g_accountSubPage = 0;
            g_app.loginRequired = false;
            CURRENT_MENU = g_app.rootMenu;
        }
        else {
            strcpy(LOGIN_MSG, u8"登录失败：用户名或密码错误");
        }
    }
    if(UIButton(u8"注册账号", UILayoutNext(&buttonLayout, 140, 50), STYLE, UI_STATE, 311)) {
        g_accountSubPage = 2;
        memset(&g_regForm, 0, sizeof(g_regForm));
        g_regForm.ready = true;
        g_regForm.userState.hasFocus = true;
        LOGIN_MSG[0] = '\0';
    }
    if(UIButton(u8"返回", UILayoutNext(&buttonLayout, 120, 50), STYLE, UI_STATE, 312)) {
        g_accountSubPage = 0;
        memset(&g_loginForm, 0, sizeof(g_loginForm));
        g_loginForm.ready = true;
        LOGIN_MSG[0] = '\0';
    }
}

void MenuRegister_Show(void) {
    if(!g_regForm.ready) {
        memset(&g_regForm, 0, sizeof(g_regForm));
        g_regForm.ready = true;
        g_regForm.userState.hasFocus = true;
    }

    Rectangle contentRect = {250, 80, SCREEN_WIDTH - 270, SCREEN_HEIGHT - 100};
    UILayout layout = UIBeginLayout(contentRect, UI_DIR_VERTICAL, 30, 50);

    // 标题
    Vector2 titleSize = MeasureTextAuto(u8"用户注册", 42, 1);
    Rectangle titleRect = UILayoutNext(&layout, -1, 60);
    DrawTextAuto(u8"用户注册", (Vector2){SCREEN_WIDTH/2 - titleSize.x/2, titleRect.y},
        42, 1, STYLE->theme.primary);

    // 表单面板
    Rectangle panelRect = {SCREEN_WIDTH/2 - 250, 200, 500, 450};
    DrawRectangleRounded(panelRect, 0.1f, 12, STYLE->theme.panelBg);
    UILayout formLayout = UIBeginLayout(panelRect, UI_DIR_VERTICAL, 18, 40);

    // 用户名
    Rectangle userLabelRect = UILayoutNext(&formLayout, -1, 35);
    DrawTextAuto(u8"用户名", (Vector2){userLabelRect.x, userLabelRect.y},
        24, 1, STYLE->theme.textSecondary);
    Rectangle userInput = UILayoutNext(&formLayout, -1, 50);
    strcpy(g_regForm.userState.buffer, g_regForm.username);
    UITextBox(&g_regForm.userState, userInput, STYLE, UI_STATE, false);
    strcpy(g_regForm.username, g_regForm.userState.buffer);

    // 密码
    Rectangle passLabelRect = UILayoutNext(&formLayout, -1, 35);
    DrawTextAuto(u8"密码", (Vector2){passLabelRect.x, passLabelRect.y},
        24, 1, STYLE->theme.textSecondary);
    Rectangle passInput = UILayoutNext(&formLayout, -1, 50);
    strcpy(g_regForm.passState.buffer, g_regForm.password);
    UITextBox(&g_regForm.passState, passInput, STYLE, UI_STATE, true);
    strcpy(g_regForm.password, g_regForm.passState.buffer);

    // 确认密码
    Rectangle confirmLabelRect = UILayoutNext(&formLayout, -1, 35);
    DrawTextAuto(u8"确认密码", (Vector2){confirmLabelRect.x, confirmLabelRect.y},
        24, 1, STYLE->theme.textSecondary);
    Rectangle confirmInput = UILayoutNext(&formLayout, -1, 50);
    strcpy(g_regForm.confirmState.buffer, g_regForm.confirm);
    UITextBox(&g_regForm.confirmState, confirmInput, STYLE, UI_STATE, true);
    strcpy(g_regForm.confirm, g_regForm.confirmState.buffer);

    // 提示消息
    Rectangle msgSection = UILayoutNext(&formLayout, -1, 35);
    if(LOGIN_MSG[0]) {
        DrawTextAuto(LOGIN_MSG, (Vector2){msgSection.x, msgSection.y}, 20, 1,
            strstr(LOGIN_MSG, u8"成功") ? STYLE->theme.success : STYLE->theme.error);
    }

    // 按钮行
    Rectangle buttonSection = UILayoutNext(&formLayout, -1, 55);
    UILayout buttonLayout = UIBeginLayout(buttonSection, UI_DIR_HORIZONTAL, 20, 0);

    if(UIButton(u8"注册", UILayoutNext(&buttonLayout, 140, 50), STYLE, UI_STATE, 320)) {
        if(!g_regForm.username[0]) {
            strcpy(LOGIN_MSG, u8"请输入用户名");
        }
        else if(!g_regForm.password[0]) {
            strcpy(LOGIN_MSG, u8"请输入密码");
        }
        else if(strcmp(g_regForm.password, g_regForm.confirm) != 0) {
            strcpy(LOGIN_MSG, u8"两次密码输入不一致");
        }
        else if(Account_Register(g_regForm.username, g_regForm.password)) {
            Account_Login(g_regForm.username, g_regForm.password);
            char progressPath[256], planPath[256];
            Account_GetProgressPath(progressPath, sizeof(progressPath));
            setProgressFilePath(progressPath);
            loadProgress();
            Account_GetPlanPath(planPath, sizeof(planPath));
            Plan_SetFilePath(planPath);
            snprintf(LOGIN_MSG, 128, u8"注册成功，欢迎 %s！", g_regForm.username);
            memset(&g_regForm, 0, sizeof(g_regForm));
            g_regForm.ready = true;
            g_accountSubPage = 0;
            g_app.loginRequired = false;
            CURRENT_MENU = g_app.rootMenu;
        }
        else {
            strcpy(LOGIN_MSG, u8"注册失败：用户名已存在或无效");
        }
    }
    if(UIButton(u8"返回登录", UILayoutNext(&buttonLayout, 140, 50), STYLE, UI_STATE, 321)) {
        g_accountSubPage = 1;
        memset(&g_loginForm, 0, sizeof(g_loginForm));
        g_loginForm.ready = true;
        g_loginForm.userState.hasFocus = true;
        LOGIN_MSG[0] = '\0';
    }
    if(UIButton(u8"返回", UILayoutNext(&buttonLayout, 120, 50), STYLE, UI_STATE, 322)) {
        g_accountSubPage = 0;
        memset(&g_regForm, 0, sizeof(g_regForm));
        g_regForm.ready = true;
        LOGIN_MSG[0] = '\0';
    }
}
