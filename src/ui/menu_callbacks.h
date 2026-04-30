// ============================================================================
// 菜单回调函数头文件
// 功能：声明各菜单页面的显示函数
// 重构后：通过 AppState 统一访问状态，不再使用散落的全局变量
// ============================================================================

#ifndef MENU_CALLBACKS_H
#define MENU_CALLBACKS_H

#include "raylib.h"
#include "raylib_word_ui.h"
#include "tree_menu.h"
#include "words.h"
#include "fonts.h"
#include "app_state.h"
#include "config.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ============================================================================
// 菜单回调函数声明
// ============================================================================

/**
 * 主菜单显示
 * 包含欢迎信息、统计信息和功能卡片入口
 */
void MenuHome_Show(void);

/**
 * 学单词模式
 * 左侧：可滚动的单词列表
 * 右侧：选中单词的详细信息（单词、音标、释义、例句）
 */
void MenuLearn_Show(void);

/**
 * 背单词模式（父页面 - 显示三个子模式入口）
 */
void MenuReviewRoot_Show(void);

/**
 * 卡片背单词（原闪卡复习）
 * 使用闪卡翻转学习：正面显示单词，背面显示释义
 * 认识3次后标记为"已掌握"
 */
void MenuCardReview_Show(void);

/**
 * 选词背单词
 * 显示汉语释义，从4个英文选项中选正确单词
 * 按用户账号统计得分
 */
void MenuSelectWord_Show(void);

/**
 * 测试模式
 * 选择题测试：显示单词，从4个选项中选择正确释义
 * 测试完成后显示正确率和评价
 */
void MenuTest_Show(void);

/**
 * 查找单词模式
 * 使用正则表达式或模糊搜索查找单词，显示详细信息
 */
void MenuSearch_Show(void);

/**
 * 学习计划管理（父页面 - 包含计划和进度）
 */
void MenuPlanRoot_Show(void);

/**
 * 学习计划列表管理
 * 创建、选择、删除学习计划
 */
void MenuPlanManager_Show(void);

/**
 * 进度模式
 * 显示学习统计和每个单词的详细进度
 */
void MenuProgress_Show(void);

/**
 * 设置页面
 * 提供主题切换（深色/浅色模式）等设置选项
 */
void MenuSettings_Show(void);

/**
 * 账号管理页面
 * 用户注册、登录、切换账号
 */
void MenuAccount_Show(void);

/**
 * 词库管理页面
 * 添加、编辑、删除单词
 */
void MenuWordManager_Show(void);

/**
 * 登录页面
 * 用户登录表单
 */
void MenuLogin_Show(void);

/**
 * 注册页面
 * 新用户注册表单
 */
void MenuRegister_Show(void);

// ============================================================================
// 菜单系统辅助函数
// ============================================================================

/**
 * 初始化菜单树
 * 创建所有菜单节点并建立父子关系
 */
void InitMenuTree(void);

/**
 * 释放菜单树内存
 */
void FreeMenuTree(void);

/**
 * 获取菜单项文本
 * 根据菜单的显示函数确定菜单名称
 * @param menu 菜单节点
 * @return 菜单显示名称
 */
const char* GetMenuItemText(MENU* menu);

/**
 * 绘制树形菜单（左侧导航栏）
 * @param menuRect 菜单区域
 */
void DrawTreeMenu(Rectangle menuRect);

// 账号菜单节点（供外部页面点击跳转使用）
extern MENU* g_accountMenuNode;

#endif // MENU_CALLBACKS_H
