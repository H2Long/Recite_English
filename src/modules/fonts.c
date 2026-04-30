// ============================================================================
// 字体渲染模块
// 功能：加载管理四种字体（中文/英文/IPA/合并），实现混合文本逐字符渲染
//
// 设计决策：
//   维护四种独立的字体句柄，而不是使用单一合并字体。原因是：
//   1. 中文字体文件大(~16MB)，合入英文字体会导致不必要的内存占用
//   2. 英文字体（如 DejaVu Sans）对 ASCII 和拉丁字符渲染效果更好
//   3. 不同平台字体路径不同，分开加载可以逐个尝试直到成功
//
// 混合文本渲染策略 (DrawTextAuto/MeasureTextAuto)：
//   遍历文本中的每个 UTF-8 字符，根据其 Unicode 码点范围选择字体：
//   - 0x0000 ~ 0x007F (ASCII) → g_englishFont
//   - 0x00A0 ~ 0x1EFF (Latin/IPA 扩展) → g_latinFont
//   - 其它（中文等 CJK 字符）→ g_mergedFont
//
// 字体加载优先级 (loadFonts)：
//   1. data/fonts/ 下的打包字体（保证跨平台一致性）
//   2. 各平台系统字体（作为后备）
//   3. raylib 内置默认字体（最终回退）
//
// 依赖：
//   - fonts.h: 函数声明
//   - words.h: 引用 g_words 以提取中文字符预加载字形
// ============================================================================

#include "fonts.h"
#include "words.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// 字体
Font g_chineseFont;   // 中文字体（NotoSansCJK.otf 等），负责渲染所有 CJK 字符
Font g_englishFont;   // 英文字体（DejaVu Sans 等），负责渲染 ASCII 字符
Font g_latinFont;     // IPA/音标字体（IPA 日文字体或 DejaVu），负责渲染 IPA 扩展字符
Font g_mergedFont;    // 合并字体（基于中文字体合并 IPA 和 ASCII 字形），用于 UI 统一绘制

// ============================================================================
// 字体管理模块
// 功能：加载中英文/IPA字体，自动选择字体绘制混合文本
// ============================================================================

#include "fonts.h"
#include "words.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// 全局字体变量
Font g_chineseFont;    // 中文字体（NotoSansCJK）
Font g_englishFont;    // 英文字体（DejaVu）
Font g_latinFont;      // IPA/音标字体
Font g_mergedFont;     // 合并字体（中+IPA+ASCII，用于UI）

// 检查文件是否存在
static bool fileExists(const char *path) {
    FILE *f = fopen(path, "rb");
    if (f) { fclose(f); return true; }
    return false;
}

static void appendCodepoints(int **array, int *count, int *capacity, const int *src, int srcCount) {
    if (srcCount <= 0) return;
    if (*array == NULL) {
        *capacity = (srcCount > 16) ? srcCount : 16;
        *array = (int*)malloc(*capacity * sizeof(int));
        *count = 0;
    } else if (*count + srcCount > *capacity) {
        *capacity = (*count + srcCount) * 2;
        *array = (int*)realloc(*array, *capacity * sizeof(int));
    }
    memcpy(*array + *count, src, srcCount * sizeof(int));
    *count += srcCount;
}

// 从文本中提取中文字符到缓冲区
static void extractChineseChars(const char* text, char* buffer, int* bufLen, int maxLen) {
    for (int j = 0; text && text[j] && *bufLen < maxLen; j++) {
        unsigned char c = text[j];
        if (c >= 0x80) {
            int len = (c < 0xE0) ? 2 : (c < 0xF0) ? 3 : 4;
            bool found = false;
            for (int k = 0; k < *bufLen; k++) {
                bool match = true;
                for (int m = 0; m < len && k + m < *bufLen; m++) {
                    if (((unsigned char*)buffer)[k + m] != ((unsigned char*)text)[j + m]) {
                        match = false;
                        break;
                    }
                }
                if (match) { found = true; break; }
            }
            if (!found) {
                for (int m = 0; m < len && *bufLen < maxLen; m++) {
                    buffer[(*bufLen)++] = text[j + m];
                }
            }
            j += len - 1;
        }
    }
}

// 从单词释义和音标中提取中文字符
static void extractWordsChineseChars(char* buffer, int* bufLen, int maxLen) {
    for (int i = 0; i < g_wordCount && i < MAX_WORDS; i++) {
        extractChineseChars(g_words[i].entry.definition, buffer, bufLen, maxLen);
        extractChineseChars(g_words[i].entry.phonetic, buffer, bufLen, maxLen);
    }
}

// 检查字符是否是 IPA/Latin 扩展字符
static inline bool isLatinExtended(int c) {
    if (c >= 0x00A0 && c <= 0x00FF) return true;
    if (c >= 0x0250 && c <= 0x02AF) return true;
    if (c >= 0x0100 && c <= 0x017F) return true;
    if (c >= 0x0180 && c <= 0x024F) return true;
    if (c >= 0x1E00 && c <= 0x1EFF) return true;
    if (c >= 0x0370 && c <= 0x03FF) return true;
    if (c >= 0x1D00 && c <= 0x1D7F) return true;
    if (c >= 0x1D80 && c <= 0x1DBF) return true;
    if (c >= 0x02B0 && c <= 0x02FF) return true;
    if (c >= 0x0300 && c <= 0x036F) return true;
    return false;
}

// 加载字体
// 按顺序尝试加载：
// 1. 优先用 data/fonts/ 下的打包字体（确保跨平台一致性）
// 2. 回退到各平台系统字体
// 3. 最终回退到 raylib 默认字体
void loadFonts(void) {
    g_mergedFont = GetFontDefault();
    g_latinFont = GetFontDefault();
    
    // ====================================================================
    // 英文字体候选列表
    // 优先使用打包的 data/fonts/ 目录，再回退到系统字体
    // ====================================================================
    const char* englishCandidates[] = {
        // 打包字体（保证跨平台一致性）
        "./data/fonts/DejaVuSans.ttf",
        "./data/fonts/DejaVuSansMono.ttf",
#if defined(_WIN32)
        // Windows 系统字体
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/times.ttf",
        "C:/Windows/Fonts/calibri.ttf",
        "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/verdana.ttf",
        "C:/Windows/Fonts/consola.ttf",
        "C:/Windows/Fonts/cour.ttf",
#elif defined(__APPLE__)
        // macOS 系统字体
        "/System/Library/Fonts/Helvetica.ttc",
        "/System/Library/Fonts/Times.ttc",
        "/System/Library/Fonts/Arial.ttf",
        "/System/Library/Fonts/Courier.ttc",
        "/System/Library/Fonts/Menlo.ttc",
        "/Library/Fonts/Arial.ttf",
        "/Library/Fonts/Times New Roman.ttf",
#else
        // Linux 系统字体
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf",
#endif
    };
    int englishCount = sizeof(englishCandidates) / sizeof(englishCandidates[0]);
    for (int i = 0; i < englishCount; i++) {
        if (fileExists(englishCandidates[i])) {
            int asciiGlyphs[127 - 32];
            for (int c = 32; c < 127; c++) asciiGlyphs[c - 32] = c;
            Font f = LoadFontEx(englishCandidates[i], 32, asciiGlyphs, 95);
            if (f.texture.id != 0) {
                g_englishFont = f;
                printf("INFO: English font loaded: %s\n", englishCandidates[i]);
                break;
            }
        }
    }
    if (g_englishFont.texture.id == 0) {
        g_englishFont = GetFontDefault();
    }
    
    // ====================================================================
    // IPA/音标字体候选列表
    // 优先使用打包的 data/fonts/ 目录，再回退到各平台系统字体
    // ====================================================================
    const char* ipaCandidates[] = {
        // 打包字体（保证跨平台一致性）
        "./data/fonts/DejaVuSans.ttf",
        "./data/fonts/DejaVuSansMono.ttf",
#if defined(_WIN32)
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/times.ttf",
        "C:/Windows/Fonts/calibri.ttf",
        "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/consola.ttf",
        "C:/Windows/Fonts/seguiemj.ttf",
#elif defined(__APPLE__)
        "/System/Library/Fonts/Helvetica.ttc",
        "/System/Library/Fonts/Times.ttc",
        "/System/Library/Fonts/Menlo.ttc",
        "/System/Library/Fonts/Apple Symbols.ttf",
        "/Library/Fonts/Arial.ttf",
#else
        "/usr/share/fonts/opentype/ipafont-mincho/ipam.ttf",
        "/usr/share/fonts/opentype/ipafont-mincho/ipamp.ttf",
        "/usr/share/fonts/opentype/ipaexfont-mincho/ipaexm.ttf",
        "/usr/share/fonts/opentype/ipafont-gothic/ipag.ttf",
        "/usr/share/fonts/opentype/ipafont-gothic/ipagp.ttf",
        "/usr/share/fonts/opentype/ipaexfont-gothic/ipaexg.ttf",
        "/usr/share/fonts/truetype/fonts-japanese-mincho.ttf",
        "/usr/share/fonts/truetype/fonts-japanese-gothic.ttf",
        "/usr/share/texmf/fonts/opentype/public/lm/lmroman10-regular.otf",
        "/usr/share/texmf/fonts/opentype/public/lm/lmroman12-regular.otf",
        "/usr/share/texmf/fonts/opentype/public/lm/lmroman17-regular.otf",
        "/usr/share/texmf/fonts/opentype/public/lm/lmromandunh10-regular.otf",
        "/usr/share/texmf/fonts/opentype/public/lm/lmromancaps10-regular.otf",
        "/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf",
        "/usr/share/fonts/truetype/noto/NotoSans-Bold.ttf",
        "/usr/share/fonts/truetype/noto/NotoSansMono-Regular.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSerif.ttf",
        "/usr/share/fonts/truetype/msttcorefonts/Arial.ttf",
        "/usr/share/fonts/truetype/msttcorefonts/Times_New_Roman.ttf",
#endif
    };
    int ipaCount = sizeof(ipaCandidates) / sizeof(ipaCandidates[0]);
    for (int i = 0; i < ipaCount; i++) {
        if (fileExists(ipaCandidates[i])) {
            int ipaGlyphs[2500];
            int ipaGlyphCount = 0;
            for (int c = 0x00A0; c <= 0x00FF && ipaGlyphCount < 100; c++) ipaGlyphs[ipaGlyphCount++] = c;
            for (int c = 0x0100; c <= 0x017F && ipaGlyphCount < 300; c++) ipaGlyphs[ipaGlyphCount++] = c;
            for (int c = 0x0180; c <= 0x024F && ipaGlyphCount < 500; c++) ipaGlyphs[ipaGlyphCount++] = c;
            for (int c = 0x0250; c <= 0x02AF && ipaGlyphCount < 700; c++) ipaGlyphs[ipaGlyphCount++] = c;
            for (int c = 0x02B0; c <= 0x02FF && ipaGlyphCount < 850; c++) ipaGlyphs[ipaGlyphCount++] = c;
            for (int c = 0x0300; c <= 0x036F && ipaGlyphCount < 1050; c++) ipaGlyphs[ipaGlyphCount++] = c;
            for (int c = 0x0370; c <= 0x03FF && ipaGlyphCount < 1250; c++) ipaGlyphs[ipaGlyphCount++] = c;
            for (int c = 0x1D00; c <= 0x1DBF && ipaGlyphCount < 1500; c++) ipaGlyphs[ipaGlyphCount++] = c;
            for (int c = 0x1E00; c <= 0x1EFF && ipaGlyphCount < 1700; c++) ipaGlyphs[ipaGlyphCount++] = c;
            for (int c = 0x1F00; c <= 0x1FFF && ipaGlyphCount < 2000; c++) ipaGlyphs[ipaGlyphCount++] = c;
            
            Font f = LoadFontEx(ipaCandidates[i], 32, ipaGlyphs, ipaGlyphCount);
            if (f.texture.id != 0 && f.recs != NULL) {
                g_latinFont = f;
                printf("INFO: IPA font loaded: %s with %d glyphs\n", ipaCandidates[i], ipaGlyphCount);
                break;
            }
        }
    }
    
    if (g_latinFont.texture.id == 0 || g_latinFont.recs == NULL) {
        g_latinFont = GetFontDefault();
        printf("WARNING: IPA font load failed, using default\n");
    }
    
    // ====================================================================
    // 中文字体候选列表（按平台区分）
    // ====================================================================
#if defined(_WIN32)
    const char* candidates[] = {
        "./data/fonts/NotoSansCJK.otf",
        "C:/Windows/Fonts/msyh.ttc",        // 微软雅黑
        "C:/Windows/Fonts/msyhbd.ttc",      // 微软雅黑加粗
        "C:/Windows/Fonts/simsun.ttc",      // 宋体
        "C:/Windows/Fonts/simhei.ttf",      // 黑体
        "C:/Windows/Fonts/yahei.ttf",
        "./simhei.ttf",
        "./SimHei.ttf",
    };
#elif defined(__APPLE__)
    const char* candidates[] = {
        "./data/fonts/NotoSansCJK.otf",
        "/System/Library/Fonts/PingFang.ttc",       // 苹方
        "/System/Library/Fonts/STHeiti Light.ttc",  // 华文黑体
        "/System/Library/Fonts/STHeiti Medium.ttc",
        "/Library/Fonts/Arial Unicode.ttf",
        "/System/Library/Fonts/AppleSDGothicNeo.ttc",
    };
#else
    const char* candidates[] = {
        "./data/fonts/NotoSansCJK.otf",
        "/usr/share/fonts/truetype/droid/DroidSansFallbackFull.ttf",
        "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc",
        "/usr/share/fonts/truetype/wqy/wqy-microhei.ttc",
        "/usr/share/fonts/truetype/wqy/wqy-zenhei.ttc",
        "./simhei.ttf",
        "./SimHei.ttf",
    };
#endif

    int chineseCount = sizeof(candidates) / sizeof(candidates[0]);
    const char *fontPath = NULL;
    for (int i = 0; i < chineseCount; i++) {
        if (fileExists(candidates[i])) {
            fontPath = candidates[i];
            break;
        }
    }

    g_chineseFont = GetFontDefault();
    if (fontPath != NULL) {
        const char* allChinese = 
            "主菜单学单词背单词测试进度开始上一个下一个认识不认识！"
            "释义例句总单词数已掌握学习中认识次不认识次状态上次复习"
            "恭喜所有单词都已掌握再来一次下一题清除进度正确率建议先去学单词模式"
            "点击卡片翻转底部按钮标记认识或不认识本轮认识返回"
            "欢迎使用背单词软件选择左侧导航栏中的模式开始学习"
            "详细学习每个单词的释义和用法使用闪卡翻转记忆选择题检验学习成果"
            "只显示未掌握来一次卡片详细释义是什么用法得分太棒继续保持不错接再厉需要多复习"
            "得分不错再来建议先去学模式刷记择题检验成果翻转底按钮标记本轮认主菜单"
            "设置主题当前：深浅色模式基于构建，支持中英文混合显示版本关"
            "查找单词搜索输入没有找到请输入要查找的单词汉语解释音标注音例句翻译结果。则面"
            "永远不要放弃你的梦想她有说三种语言的能力每个人都注意到了他的缺席这是绝对必要的"
            "植物从土壤中吸收水分这个概念太抽象了我们拥有丰富的资源她有优秀的学业记录"
            "汽车开始加速我们完成了任务这个时钟显示准确的时间你能够实现你的目标"
            "他承认了他的错误她学会了一项新技能我们必须适应变化这些食物足够十个人吃请调节音量"
            "她在行政部门工作我钦佩你的勇气军队开始前进她热爱冒险天气很不利"
            "他们在网上做广告他给了我好的建议我们提倡和平天气影响情绪我负担不起"
            "让我们讨论议程他变得具有攻击性我同意你的看法警报响了这是一张很棒的专辑"
            "他不喝酒始终保持警觉这对双胞胎看起来很像她还活着吗不允许吸烟我想一个人待着"
            "学习字母表我们需要改变计划多么惊人的景色绝对必要的你的能够学习优秀记录"
            "学会适应变化食物足够调节音量钦佩你的部队前进不利的做广告"
            "给我建议提倡和平影响负担议程攻击性警报不允许保持警觉"
            "双胞胎学习改变计划惊人绝对优秀能够永远每人都时钟目标错误技能适应"
            "调节优部进利告给议态警止持"
            "一二三四五六七八九十百千万亿男女老师朋友父母兄弟姐妹丈夫妻子"
            "吃饭喝水睡觉走路说话读写问答哭笑跑跳坐站躺春夏秋冬"
            "红黄蓝绿黑白灰紫橙天地水火花草鱼鸟鸡狗猪猫兔龙蛇牛羊虫"
            "东西南北前后左右里外中间旁边正反内外上下大小长短高矮宽窄"
            "厚薄深浅粗细轻重快慢早晚冷暖凉热干湿软硬酸甜苦辣咸香"
            "因为所以虽然但是而且或者然而无论如果即使既然只要除非"
            "把被让叫给对向跟与同从在到由关于按照根据除了"
            "已经虽然因为所以但是而且或者无论非常特别比较更加最太很真够极"
            "的得了着过吧呢啊吗呀哪么"
            "次又再也还还是就是可是只是但是就能才刚"
            "先生小姐孩子同学同事亲戚邻居街道医院商店市场公园"
            "身体头部眼睛耳鼻口手脚腿胳膊肩膀肚子背腰皮肤血骨"
            "春夏秋冬春节日年岁月份星期时分秒上午中午下午"
            "国家城市道路街道楼房门窗桌椅床柜箱包纸张笔袋"
            "手机电脑网络书籍信息数据文件程序软件硬件网页邮件"
            "唱歌跳舞音乐绘画体育科学历史地理数学语文英语"
            "喜欢快乐高兴难过悲伤愤怒害怕紧张放松安静热闹"
            "美丽漂亮好看可爱帅气英俊聪明优秀努力勤奋认真仔细"
            "这些那些每个什么怎么为什么多少几哪谁"
            "欢迎感谢帮助支持保护关心照顾提醒注意思考理解。，、：；？！“”‘’（）【】《》——……·～—／＠＃￥％＆＊＋＝－　"
            "拿放置搬推拉抬提抓握抱扔投接踢跨踩穿脱戴挂晒洗刷擦扫收拾整理"
            "买卖付找换退租借休息劳动研究讨论交流沟通忘记丢失找见"
            "送给寄取收提贡献享邀请参加组织管理选择判断决定安排准备计划"
            "生产消费运输存储保护修理制造创作发明报告演讲采访调查"
            "治疗护理检查诊断驾驶乘坐旅行旅游"
            "机场车站码头港口办公室教室会议室图书馆厨房卧室客厅浴室阳台"
            "银行邮政保险税务报纸杂志电视广播飞机火车轮船汽车"
            "森林草原沙漠海洋河流湖泊太阳月亮星星宇宙银河"
            "幸福幸运快乐痛苦简单复杂容易困难安静吵闹热闹干净脏乱拥挤"
            "新鲜陈旧古老现代年轻年老成熟幼稚明亮黑暗光明昏暗坚固脆弱"
            "危险安全稳定动荡富裕贫穷繁荣荒凉先进落后原始"
            "各某另其些点多少量全部剩余此而之其且或与否于毕将曾业己"
            "账号密确登录注册请败存效户名切换失独等待片看词库添删辑编框周半巩阶刺";

        char wordChars[8192] = {0};
        int wcLen = 0;
        extractWordsChineseChars(wordChars, &wcLen, 8000);

        int uiGlyphCount = 0;
        int *uiGlyphs = LoadCodepoints(allChinese, &uiGlyphCount);
        int wordGlyphCount = 0;
        int *wordGlyphs = LoadCodepoints(wordChars, &wordGlyphCount);
        
        int *glyphs = NULL;
        int glyphCount = 0;
        int glyphCapacity = 0;
        appendCodepoints(&glyphs, &glyphCount, &glyphCapacity, uiGlyphs, uiGlyphCount);
        appendCodepoints(&glyphs, &glyphCount, &glyphCapacity, wordGlyphs, wordGlyphCount);
        UnloadCodepoints(uiGlyphs);
        UnloadCodepoints(wordGlyphs);

        int asciiGlyphs[127 - 32];
        for (int c = 32; c < 127; c++) asciiGlyphs[c - 32] = c;
        appendCodepoints(&glyphs, &glyphCount, &glyphCapacity, asciiGlyphs, 127 - 32);

        Font tmp = LoadFontEx(fontPath, 32, glyphs, glyphCount);
        if (tmp.texture.id != 0) {
            g_chineseFont = tmp;
            printf("INFO: Chinese font loaded: %s with %d glyphs\n", fontPath, glyphCount);
        } else {
            printf("WARNING: Chinese font load failed: %s\n", fontPath);
        }
        free(glyphs);

        // 加载合并字体（中文 + IPA + ASCII）
        int mergeCount = 0;
        int *mergeGlyphs = LoadCodepoints(allChinese, &mergeCount);
        int wordGlyphsMergedCount = 0;
        int *wordGlyphsMerged = LoadCodepoints(wordChars, &wordGlyphsMergedCount);
        
        int *allGlyphs = NULL;
        int allCount = 0;
        int allCapacity = 0;
        appendCodepoints(&allGlyphs, &allCount, &allCapacity, mergeGlyphs, mergeCount);
        appendCodepoints(&allGlyphs, &allCount, &allCapacity, wordGlyphsMerged, wordGlyphsMergedCount);
        UnloadCodepoints(mergeGlyphs);
        UnloadCodepoints(wordGlyphsMerged);

        Font defaultFont = GetFontDefault();
        int defaultGlyphCount = defaultFont.glyphCount;
        
        int newCapacity = allCount + defaultGlyphCount + 2000;
        allGlyphs = (int*)realloc(allGlyphs, newCapacity * sizeof(int));
        
        for (int i = 0; i < defaultGlyphCount; i++) {
            bool exists = false;
            int cp = defaultFont.glyphs[i].value;
            for (int j = 0; j < allCount; j++) {
                if (allGlyphs[j] == cp) { exists = true; break; }
            }
            if (!exists) {
                allGlyphs[allCount++] = cp;
            }
        }
        
        int ipaRanges[][2] = {
            {0x00A0, 0x00FF}, {0x0100, 0x017F}, {0x0180, 0x024F},
            {0x0250, 0x02AF}, {0x02B0, 0x02FF}, {0x0300, 0x036F},
            {0x0370, 0x03FF}, {0x1D00, 0x1DBF}, {0x1D80, 0x1DBF},
            {0x1E00, 0x1EFF}, {0x2000, 0x206F}, {0x2070, 0x209F},
        };
        int ipaRangeCount = sizeof(ipaRanges) / sizeof(ipaRanges[0]);
        for (int r = 0; r < ipaRangeCount; r++) {
            for (int cp = ipaRanges[r][0]; cp <= ipaRanges[r][1]; cp++) {
                bool exists = false;
                for (int j = 0; j < allCount; j++) {
                    if (allGlyphs[j] == cp) { exists = true; break; }
                }
                if (!exists) {
                    allGlyphs[allCount++] = cp;
                }
            }
        }

        Font merged = LoadFontEx(fontPath, 32, allGlyphs, allCount);
        if (merged.texture.id != 0) {
            g_mergedFont = merged;
            printf("INFO: Merged font loaded with %d glyphs\n", allCount);
        } else {
            printf("WARNING: Merged font load failed, using English font\n");
            g_mergedFont = g_englishFont;
        }
        free(allGlyphs);
    }
}

// 卸载字体（程序退出时调用）
void unloadFonts(void) {
    if (g_chineseFont.texture.id != GetFontDefault().texture.id) {
        UnloadFont(g_chineseFont);
    }
    if (g_mergedFont.texture.id != GetFontDefault().texture.id && 
        g_mergedFont.texture.id != g_chineseFont.texture.id) {
        UnloadFont(g_mergedFont);
    }
}

// 获取 UTF-8 字符的字节长度
static int getUtf8Len(unsigned char byte) {
    if (byte < 0x80) return 1;
    if (byte < 0xE0) return 2;
    if (byte < 0xF0) return 3;
    return 4;
}

// 获取单个字符应该使用的字体
// ASCII字符 -> 英文字体
// IPA/Latin扩展字符 -> IPA字体
// 中文/其他 -> 合并字体
static Font getFontForChar(const char* text) {
    unsigned char byte = *(unsigned char*)text;
    if (byte < 0x80) return g_englishFont;
    
    int codepoint;
    if (byte < 0xE0) {
        codepoint = ((byte & 0x1F) << 6) | ((unsigned char)text[1] & 0x3F);
    } else if (byte < 0xF0) {
        codepoint = ((byte & 0x0F) << 12) | (((unsigned char)text[1] & 0x3F) << 6) | ((unsigned char)text[2] & 0x3F);
    } else {
        codepoint = ((byte & 0x07) << 18) | (((unsigned char)text[1] & 0x3F) << 12) | (((unsigned char)text[2] & 0x3F) << 6) | ((unsigned char)text[3] & 0x3F);
    }
    
    if (isLatinExtended(codepoint)) return g_latinFont;
    return g_mergedFont;
}

// 测量混合文本的宽度
// 自动识别中/英/IPA字符，使用对应字体测量
Vector2 MeasureTextAuto(const char* text, float fontSize, float spacing) {
    Vector2 result = {0, fontSize};
    if (!text || text[0] == '\0') return result;
    
    while (*text) {
        unsigned char uc = *(unsigned char*)text;
        if (uc == ' ' || uc == '\t') {
            result.x += MeasureTextEx(g_englishFont, " ", fontSize, spacing).x;
            text++;
            continue;
        }
        if (uc == '\n') break;
        
        Font currentFont = getFontForChar(text);
        int charLen = getUtf8Len(uc);
        
        char segment[5] = {0};
        for (int i = 0; i < charLen && *text; i++) {
            segment[i] = *text++;
        }
        Vector2 s = MeasureTextEx(currentFont, segment, fontSize, spacing);
        result.x += s.x;
    }
    return result;
}

// 绘制单行文本（自动选择字体）
// 根据字符类型选择字体：中文字符用合并字体，英文/IPA用对应字体
void DrawTextAuto(const char* text, Vector2 pos, float fontSize, float spacing, Color tint) {
    if (!text || text[0] == '\0') return;
    
    float x = pos.x;
    while (*text) {
        unsigned char uc = *(unsigned char*)text;
        
        if (uc == ' ' || uc == '\t') {
            x += MeasureTextEx(g_englishFont, " ", fontSize, spacing).x;
            text++;
            continue;
        }
        if (uc == '\n') break;

        Font currentFont = getFontForChar(text);
        int charLen = getUtf8Len(uc);
        
        char segment[5] = {0};
        for (int i = 0; i < charLen && *text; i++) {
            segment[i] = *text++;
        }

        DrawTextEx(currentFont, segment, (Vector2){ x, pos.y }, fontSize, spacing, tint);
        Vector2 s = MeasureTextEx(currentFont, segment, fontSize, spacing);
        x += s.x;
    }
}

// 获取记忆状态的颜色
// 熟练度 >= 80%: 绿色（已掌握）
// 熟练度 >= 50%: 橙色（学习中）
// 其他: 红色（待学习）
Color getMasteryColor(float mastery) {
    if (mastery >= 0.8f) return (Color){50, 205, 50, 255};
    if (mastery >= 0.5f) return (Color){255, 165, 0, 255};
    return (Color){220, 20, 60, 255};
}

// 格式化时间戳为可读字符串
// 0 表示"从未复习"，否则显示 "月-日 时:分"
const char* formatTime(time_t timestamp) {
    static char buffer[64];
    if (timestamp == 0) return "从未";
    struct tm *tm_info = localtime(&timestamp);
    strftime(buffer, sizeof(buffer), "%m-%d %H:%M", tm_info);
    return buffer;
}
