// ============================================================================
// 账号管理页面
// 功能：
//   MenuAccount_Show  — 主页面：用户信息 / 登录入口 + 全部用户列表
//   MenuLogin_Show    — 登录子页面：用户名 + 密码
//   MenuRegister_Show — 注册子页面：用户名 + 密码 + 确认密码
// ============================================================================

#include "pages.h"

// g_accountSubPage - 子页面状态（0=主页，1=登录页，2=注册页）
static int g_accountSubPage = 0;

// g_loginForm - 登录表单状态
static struct { char username[32]; char password[64];
    UITextBoxState userState, passState; bool initialized; } g_loginForm = {0};

// g_regForm - 注册表单状态
static struct { char username[32]; char password[64]; char confirm[64];
    UITextBoxState userState, passState, confirmState; bool initialized; } g_regForm = {0};

/**
 * MenuAccount_Show - 账号管理主页面
 *
 * 已登录：显示头像占位 + 用户名 + 注册时间 + 登出/切换按钮
 * 未登录：显示欢迎语 + 登录/注册按钮
 * 底部：全部注册用户列表（用户名 / 注册时间 / 上次登录 / 当前标记）
 */
void MenuAccount_Show(void) {
    if (g_accountSubPage == 1) { MenuLogin_Show(); return; }
    if (g_accountSubPage == 2) { MenuRegister_Show(); return; }

    Rectangle cr = {250, 80, SCREEN_WIDTH - 270, SCREEN_HEIGHT - 100};
    UILayout layout = UIBeginLayout(cr, UI_DIR_VERTICAL, 30, 50);
    Vector2 ts = MeasureTextAuto(u8"账号管理", 42, 1);
    DrawTextAuto(u8"账号管理", (Vector2){cr.x + 50, (UILayoutNext(&layout, -1, 60)).y}, 42, 1, STYLE->theme.textPrimary);

    Rectangle pr = UILayoutNext(&layout, -1, 260);
    DrawRectangleRounded(pr, 0.1f, 12, STYLE->theme.panelBg);
    UILayout pl = UIBeginLayout(pr, UI_DIR_VERTICAL, 25, 40);

    if (ACCOUNT.isLoggedIn) {
        Rectangle us = UILayoutNext(&pl, -1, 100);
        DrawCircleLines(us.x + 40, us.y + 40, 30, STYLE->theme.primary);
        DrawTextAuto(Account_GetCurrentUser(), (Vector2){us.x + 90, us.y + 25}, 36, 1, STYLE->theme.primary);
        Rectangle is = UILayoutNext(&pl, -1, 60);
        int idx = Account_GetCurrentIndex();
        char info[256];
        if (idx >= 0) { char cs[64]; strftime(cs, sizeof(cs), "%Y-%m-%d", localtime(&ACCOUNT.users[idx].createdTime));
            snprintf(info, sizeof(info), u8"注册时间: %s  |  学习数据: progress_%s.txt", cs, Account_GetCurrentUser()); }
        else snprintf(info, sizeof(info), "%s", u8"当前用户");
        DrawTextAuto(info, (Vector2){is.x, is.y}, 20, 1, STYLE->theme.textSecondary);

        Rectangle bs = UILayoutNext(&pl, -1, 60);
        UILayout bl2 = UIBeginLayout(bs, UI_DIR_HORIZONTAL, 25, 0);
        if (UIButton(u8"登出", UILayoutNext(&bl2, 160, 50), STYLE, UI_STATE, 300))
            { Account_Logout(); setProgressFilePath("./data/progress.txt"); loadProgress(); Plan_SetFilePath("./data/plans.txt"); snprintf(LOGIN_MSG, 128, "%s", u8"已登出"); }
        if (UIButton(u8"切换账号", UILayoutNext(&bl2, 200, 50), STYLE, UI_STATE, 301))
            { Account_Logout(); setProgressFilePath("./data/progress.txt"); loadProgress(); Plan_SetFilePath("./data/plans.txt"); g_accountSubPage = 1; }
    } else {
        Rectangle ws = UILayoutNext(&pl, -1, 80);
        DrawTextAuto(u8"欢迎使用背单词软件", (Vector2){ws.x, ws.y}, 32, 1, STYLE->theme.primary);
        DrawTextAuto(u8"创建账号或登录后，学习进度将独立保存", (Vector2){ws.x, ws.y + 45}, 20, 1, STYLE->theme.textSecondary);
        Rectangle bs = UILayoutNext(&pl, -1, 70);
        UILayout bl2 = UIBeginLayout(bs, UI_DIR_HORIZONTAL, 30, 50);
        if (UIButton(u8"登录", UILayoutNext(&bl2, 230, 55), STYLE, UI_STATE, 302))
            { g_accountSubPage = 1; memset(&g_loginForm, 0, sizeof(g_loginForm)); g_loginForm.initialized = true; g_loginForm.userState.hasFocus = true; snprintf(g_app.loginMsg, 128, "%s", ""); }
        if (UIButton(u8"注册", UILayoutNext(&bl2, 230, 55), STYLE, UI_STATE, 303))
            { g_accountSubPage = 2; memset(&g_regForm, 0, sizeof(g_regForm)); g_regForm.initialized = true; g_regForm.userState.hasFocus = true; snprintf(g_app.loginMsg, 128, "%s", ""); }
    }

    // 全部注册用户列表
    DrawTextAuto(u8"全部注册用户", (Vector2){(UILayoutNext(&layout, -1, 35)).x, (UILayoutNext(&layout, -1, 35)).y}, 24, 1, STYLE->theme.textSecondary);
    Rectangle up = UILayoutNext(&layout, -1, -1);
    DrawRectangleRounded(up, 0.1f, 12, STYLE->theme.panelBg);
    UIScrollView usv = {0}; usv.viewport = up;
    usv.contentSize = (Vector2){up.width - 30, ACCOUNT.userCount * 45.0f + 20};
    UIBeginScrollView(&usv, up, usv.contentSize);

    for (int i = 0; i < ACCOUNT.userCount; i++) {
        Rectangle rr2 = {up.x + 15, up.y + 10 + i * 45 - usv.scrollOffset.y, up.width - 30, 40};
        if (i % 2 == 0) DrawRectangleRec(rr2, Fade(STYLE->theme.inputBg, 0.5f));
        DrawTextAuto(ACCOUNT.users[i].username, (Vector2){rr2.x + 10, rr2.y + 10}, 22, 1, STYLE->theme.textPrimary);
        char ts2[64];
        if (ACCOUNT.users[i].createdTime > 0) strftime(ts2, sizeof(ts2), "%Y-%m-%d", localtime(&ACCOUNT.users[i].createdTime));
        else snprintf(ts2, sizeof(ts2), "%s", u8"未知");
        DrawTextAuto(ts2, (Vector2){rr2.x + 200, rr2.y + 10}, 20, 1, STYLE->theme.textSecondary);
        if (ACCOUNT.users[i].lastLoginTime > 0) {
            char ls[64]; strftime(ls, sizeof(ls), "%Y-%m-%d %H:%M", localtime(&ACCOUNT.users[i].lastLoginTime));
            DrawTextAuto(ls, (Vector2){rr2.x + 400, rr2.y + 10}, 20, 1, STYLE->theme.textSecondary);
        }
        if (i == Account_GetCurrentIndex())
            DrawTextAuto(u8"[当前]", (Vector2){rr2.x + rr2.width - 70, rr2.y + 10}, 20, 1, STYLE->theme.primary);
    }
    UIEndScrollView(&usv, STYLE, UI_STATE);
}

/**
 * MenuLogin_Show - 登录子页面
 * 用户名输入 + 密码输入（密文）→ 登录/注册账号/返回按钮
 * 登录成功后自动加载用户专属的进度和计划文件
 */
void MenuLogin_Show(void) {
    if (!g_loginForm.initialized)
        { memset(&g_loginForm, 0, sizeof(g_loginForm)); g_loginForm.initialized = true; g_loginForm.userState.hasFocus = true; }

    Rectangle cr = {250, 80, SCREEN_WIDTH - 270, SCREEN_HEIGHT - 100};
    UILayout layout = UIBeginLayout(cr, UI_DIR_VERTICAL, 30, 50);
    Vector2 ts = MeasureTextAuto(u8"用户登录", 42, 1);
    DrawTextAuto(u8"用户登录", (Vector2){SCREEN_WIDTH/2 - ts.x/2, (UILayoutNext(&layout, -1, 60)).y}, 42, 1, STYLE->theme.primary);

    Rectangle pr = {SCREEN_WIDTH/2 - 250, 200, 500, 400};
    DrawRectangleRounded(pr, 0.1f, 12, STYLE->theme.panelBg);
    UILayout fl = UIBeginLayout(pr, UI_DIR_VERTICAL, 20, 40);

    DrawTextAuto(u8"用户名", (Vector2){(UILayoutNext(&fl, -1, 35)).x, (UILayoutNext(&fl, -1, 35)).y}, 24, 1, STYLE->theme.textSecondary);
    Rectangle ui = UILayoutNext(&fl, -1, 50);
    strncpy(g_loginForm.userState.buffer, g_loginForm.username, sizeof(g_loginForm.userState.buffer) - 1);
    UITextBox(&g_loginForm.userState, ui, STYLE, UI_STATE, false);
    strncpy(g_loginForm.username, g_loginForm.userState.buffer, 31);

    DrawTextAuto(u8"密码", (Vector2){(UILayoutNext(&fl, -1, 35)).x, (UILayoutNext(&fl, -1, 35)).y}, 24, 1, STYLE->theme.textSecondary);
    Rectangle pi = UILayoutNext(&fl, -1, 50);
    strncpy(g_loginForm.passState.buffer, g_loginForm.password, sizeof(g_loginForm.passState.buffer) - 1);
    UITextBox(&g_loginForm.passState, pi, STYLE, UI_STATE, true);
    strncpy(g_loginForm.password, g_loginForm.passState.buffer, 63);

    Rectangle mr2 = UILayoutNext(&fl, -1, 35);
    if (strlen(LOGIN_MSG) > 0) DrawTextAuto(LOGIN_MSG, (Vector2){mr2.x, mr2.y}, 20, 1,
        (strstr(LOGIN_MSG, "成功") || strstr(LOGIN_MSG, "欢迎")) ? STYLE->theme.success : STYLE->theme.error);

    Rectangle bs = UILayoutNext(&fl, -1, 55);
    UILayout bl2 = UIBeginLayout(bs, UI_DIR_HORIZONTAL, 20, 0);

    if (UIButton(u8"登录", UILayoutNext(&bl2, 140, 50), STYLE, UI_STATE, 310)) {
        if (strlen(g_loginForm.username) > 0 && strlen(g_loginForm.password) > 0) {
            if (Account_Login(g_loginForm.username, g_loginForm.password)) {
                char pp[256], plp[256];
                Account_GetProgressPath(pp, sizeof(pp)); setProgressFilePath(pp); loadProgress();
                Account_GetPlanPath(plp, sizeof(plp)); Plan_SetFilePath(plp);
                snprintf(LOGIN_MSG, 128, u8"欢迎回来，%s！", Account_GetCurrentUser());
                memset(&g_loginForm, 0, sizeof(g_loginForm)); g_loginForm.initialized = true;
                g_accountSubPage = 0; g_app.loginRequired = false; CURRENT_MENU = g_app.rootMenu;
            } else snprintf(LOGIN_MSG, 128, "%s", u8"登录失败：用户名或密码错误");
        } else snprintf(LOGIN_MSG, 128, "%s", u8"请输入用户名和密码");
    }
    if (UIButton(u8"注册账号", UILayoutNext(&bl2, 140, 50), STYLE, UI_STATE, 311))
        { g_accountSubPage = 2; memset(&g_regForm, 0, sizeof(g_regForm)); g_regForm.initialized = true; g_regForm.userState.hasFocus = true; snprintf(g_app.loginMsg, 128, "%s", ""); }
    if (UIButton(u8"返回", UILayoutNext(&bl2, 120, 50), STYLE, UI_STATE, 312))
        { g_accountSubPage = 0; memset(&g_loginForm, 0, sizeof(g_loginForm)); g_loginForm.initialized = true; snprintf(g_app.loginMsg, 128, "%s", ""); }
}

/**
 * MenuRegister_Show - 注册子页面
 * 用户名 + 密码 + 确认密码 → 注册 / 返回登录 / 返回按钮
 * 注册成功后自动登录，加载初始化户的进度和计划文件
 */
void MenuRegister_Show(void) {
    if (!g_regForm.initialized)
        { memset(&g_regForm, 0, sizeof(g_regForm)); g_regForm.initialized = true; g_regForm.userState.hasFocus = true; }

    Rectangle cr = {250, 80, SCREEN_WIDTH - 270, SCREEN_HEIGHT - 100};
    UILayout layout = UIBeginLayout(cr, UI_DIR_VERTICAL, 30, 50);
    Vector2 ts = MeasureTextAuto(u8"用户注册", 42, 1);
    DrawTextAuto(u8"用户注册", (Vector2){SCREEN_WIDTH/2 - ts.x/2, (UILayoutNext(&layout, -1, 60)).y}, 42, 1, STYLE->theme.primary);

    Rectangle pr = {SCREEN_WIDTH/2 - 250, 200, 500, 450};
    DrawRectangleRounded(pr, 0.1f, 12, STYLE->theme.panelBg);
    UILayout fl = UIBeginLayout(pr, UI_DIR_VERTICAL, 18, 40);

    DrawTextAuto(u8"用户名", (Vector2){(UILayoutNext(&fl, -1, 35)).x, (UILayoutNext(&fl, -1, 35)).y}, 24, 1, STYLE->theme.textSecondary);
    Rectangle ui = UILayoutNext(&fl, -1, 50);
    strncpy(g_regForm.userState.buffer, g_regForm.username, sizeof(g_regForm.userState.buffer) - 1);
    UITextBox(&g_regForm.userState, ui, STYLE, UI_STATE, false);
    strncpy(g_regForm.username, g_regForm.userState.buffer, 31);

    DrawTextAuto(u8"密码", (Vector2){(UILayoutNext(&fl, -1, 35)).x, (UILayoutNext(&fl, -1, 35)).y}, 24, 1, STYLE->theme.textSecondary);
    Rectangle pi = UILayoutNext(&fl, -1, 50);
    strncpy(g_regForm.passState.buffer, g_regForm.password, sizeof(g_regForm.passState.buffer) - 1);
    UITextBox(&g_regForm.passState, pi, STYLE, UI_STATE, true);
    strncpy(g_regForm.password, g_regForm.passState.buffer, 63);

    DrawTextAuto(u8"确认密码", (Vector2){(UILayoutNext(&fl, -1, 35)).x, (UILayoutNext(&fl, -1, 35)).y}, 24, 1, STYLE->theme.textSecondary);
    Rectangle ci = UILayoutNext(&fl, -1, 50);
    strncpy(g_regForm.confirmState.buffer, g_regForm.confirm, sizeof(g_regForm.confirmState.buffer) - 1);
    UITextBox(&g_regForm.confirmState, ci, STYLE, UI_STATE, true);
    strncpy(g_regForm.confirm, g_regForm.confirmState.buffer, 63);

    Rectangle mr2 = UILayoutNext(&fl, -1, 35);
    if (strlen(LOGIN_MSG) > 0) DrawTextAuto(LOGIN_MSG, (Vector2){mr2.x, mr2.y}, 20, 1,
        strstr(LOGIN_MSG, "成功") ? STYLE->theme.success : STYLE->theme.error);

    Rectangle bs = UILayoutNext(&fl, -1, 55);
    UILayout bl2 = UIBeginLayout(bs, UI_DIR_HORIZONTAL, 20, 0);

    if (UIButton(u8"注册", UILayoutNext(&bl2, 140, 50), STYLE, UI_STATE, 320)) {
        if (strlen(g_regForm.username) == 0) snprintf(LOGIN_MSG, 128, "%s", u8"请输入用户名");
        else if (strlen(g_regForm.password) == 0) snprintf(LOGIN_MSG, 128, "%s", u8"请输入密码");
        else if (strcmp(g_regForm.password, g_regForm.confirm) != 0) snprintf(LOGIN_MSG, 128, "%s", u8"两次密码输入不一致");
        else if (Account_Register(g_regForm.username, g_regForm.password)) {
            Account_Login(g_regForm.username, g_regForm.password);
            char pp[256], plp[256];
            Account_GetProgressPath(pp, sizeof(pp)); setProgressFilePath(pp); loadProgress();
            Account_GetPlanPath(plp, sizeof(plp)); Plan_SetFilePath(plp);
            snprintf(LOGIN_MSG, 128, u8"注册成功，欢迎 %s！", g_regForm.username);
            memset(&g_regForm, 0, sizeof(g_regForm)); g_regForm.initialized = true;
            g_accountSubPage = 0; g_app.loginRequired = false; CURRENT_MENU = g_app.rootMenu;
        } else snprintf(LOGIN_MSG, 128, "%s", u8"注册失败：用户名已存在或无效");
    }
    if (UIButton(u8"返回登录", UILayoutNext(&bl2, 140, 50), STYLE, UI_STATE, 321))
        { g_accountSubPage = 1; memset(&g_loginForm, 0, sizeof(g_loginForm)); g_loginForm.initialized = true; g_loginForm.userState.hasFocus = true; snprintf(g_app.loginMsg, 128, "%s", ""); }
    if (UIButton(u8"返回", UILayoutNext(&bl2, 120, 50), STYLE, UI_STATE, 322))
        { g_accountSubPage = 0; memset(&g_regForm, 0, sizeof(g_regForm)); g_regForm.initialized = true; snprintf(g_app.loginMsg, 128, "%s", ""); }
}
