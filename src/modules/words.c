// 单词管理 — 词库加载/进度/搜索/CRUD

#include "words.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _MSC_VER
#define strdup _strdup
#endif

WordEntry* g_wordLibrary = NULL;
static int g_wordLibraryCapacity = 0;
WordWithProgress g_words[MAX_WORDS];
int g_wordProgressCount = 0;
int g_wordCount = 0;
static char g_progressFilePath[256] = "./data/progress.txt";

void setProgressFilePath(const char* path) { strcpy(g_progressFilePath, path); }

void shuffleArray(int *array, int count) {
    for (int i = count - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int tmp = array[i];
        array[i] = array[j];
        array[j] = tmp;
    }
}

void loadWordsFromFile(const char* filename) {
    FILE* fp = fopen(filename, "r");
    if(fp == NULL) {
        const char* def[][5] = {
            {"abandon", "[ə'bændən]", "v. 放弃；抛弃", "Never abandon your dreams.", u8"永远不要放弃你的梦想。"},
            {"ability", "[ə'bɪləti]", "n. 能力", "She has the ability to speak three languages.", u8"她有说三种语言的能力。"},
            {"absence", "['æbsəns]", "n. 缺席；缺乏", "His absence was noticed by everyone.", u8"每个人都注意到了他的缺席。"},
            {"absolute", "['æbsəluːt]", "adj. 绝对的", "It is an absolute necessity.", u8"这是绝对必要的。"},
            {"absorb", "[əb'zɔːrb]", "v. 吸收；理解", "Plants absorb water from the soil.", u8"植物从土壤中吸收水分。"},
            {"abstract", "['æbstrækt]", "adj. 抽象的", "The concept is too abstract.", u8"这个概念太抽象了。"},
            {"abundant", "[ə'bʌndənt]", "adj. 丰富的", "We have abundant resources.", u8"我们拥有丰富的资源。"},
            {"academic", "[ˌækə'demɪk]", "adj. 学术的", "She has excellent academic records.", u8"她有优秀的学业记录。"},
            {"accelerate", "[ək'seləreɪt]", "v. 加速", "The car began to accelerate.", u8"汽车开始加速。"},
            {"accomplish", "[ə'kʌmplɪʃ]", "v. 完成", "We accomplished our mission.", u8"我们完成了任务。"},
        };
        g_wordLibraryCapacity = 20;
        g_wordLibrary = (WordEntry*)malloc(g_wordLibraryCapacity * sizeof(WordEntry));
        for (int i = 0; i < 10; i++) {
            g_wordLibrary[i].word = strdup(def[i][0]);
            g_wordLibrary[i].phonetic = strdup(def[i][1]);
            g_wordLibrary[i].definition = strdup(def[i][2]);
            g_wordLibrary[i].example = strdup(def[i][3]);
            g_wordLibrary[i].exampleTranslation = strdup(def[i][4]);
        }
        g_wordCount = 10;
        return ;
    }

    char line[1024];
    g_wordLibraryCapacity = 50;
    g_wordLibrary = (WordEntry*)malloc(g_wordLibraryCapacity * sizeof(WordEntry));
    g_wordCount = 0;
    while (fgets(line, sizeof(line), fp)) {
        int len = strlen(line);
        if(len > 0 && line[len-1] == '\n') { line[len-1] = '\0'; }
        if(len > 1 && line[len-2] == '\r') { line[len-2] = '\0'; }
        if(strlen(line) == 0) { continue; }
        if(g_wordCount >= g_wordLibraryCapacity) {
            g_wordLibraryCapacity *= 2;
            g_wordLibrary = (WordEntry*)realloc(g_wordLibrary, g_wordLibraryCapacity * sizeof(WordEntry));
        }
        char* f[5] = {NULL};
        int fi = 0;
        char* tok = strtok(line, "|");
        while (tok && fi < 5) { f[fi++] = strdup(tok); tok = strtok(NULL, "|"); }
        if(fi >= 3) {
            g_wordLibrary[g_wordCount].word = f[0];
            g_wordLibrary[g_wordCount].phonetic = f[1];
            g_wordLibrary[g_wordCount].definition = f[2];
            g_wordLibrary[g_wordCount].example = fi >= 4 ? f[3] : "";
            g_wordLibrary[g_wordCount].exampleTranslation = fi >= 5 ? f[4] : "";
            g_wordCount++;
        }
    }
    fclose(fp);
}

void freeWordLibrary(void) {
    if(g_wordLibrary == NULL) { return ; }
    for (int i = 0; i < g_wordCount; i++) {
        free((void*)g_wordLibrary[i].word);
        free((void*)g_wordLibrary[i].phonetic);
        free((void*)g_wordLibrary[i].definition);
        free((void*)g_wordLibrary[i].example);
        free((void*)g_wordLibrary[i].exampleTranslation);
    }
    free(g_wordLibrary);
    g_wordLibrary = NULL;
    g_wordCount = 0;
}

void initWords(void) {
    for (int i = 0; i < g_wordCount && i < MAX_WORDS; i++) {
        g_words[i].entry = g_wordLibrary[i];
        g_words[i].progress.wordIndex = i;
        g_words[i].progress.knownCount = 0;
        g_words[i].progress.unknownCount = 0;
        g_words[i].progress.lastReview = 0;
        g_words[i].progress.mastered = false;
        g_wordProgressCount++;
    }
    loadProgress();
}

void saveProgress(void) {
    FILE* fp = fopen(g_progressFilePath, "w");
    if(fp == NULL) { return ; }
    for (int i = 0; i < g_wordProgressCount; i++) {
        fprintf(fp, "%s|%d|%d|%ld\n",
            g_words[i].entry.word, g_words[i].progress.knownCount,
            g_words[i].progress.unknownCount, (long)g_words[i].progress.lastReview);
    }
    fclose(fp);
}

void loadProgress(void) {
    FILE* fp = fopen(g_progressFilePath, "r");
    if(fp == NULL) { return ; }
    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        int len = strlen(line);
        if(len > 0 && line[len-1] == '\n') { line[len-1] = '\0'; }
        if(strlen(line) == 0) { continue; }
        char* f[4] = {NULL};
        int fi = 0;
        char* tok = strtok(line, "|");
        while (tok && fi < 4) { f[fi++] = tok; tok = strtok(NULL, "|"); }
        if(fi >= 4) {
            for (int i = 0; i < g_wordProgressCount; i++) {
                if(strcmp(g_words[i].entry.word, f[0]) == 0) {
                    g_words[i].progress.knownCount = atoi(f[1]);
                    g_words[i].progress.unknownCount = atoi(f[2]);
                    g_words[i].progress.lastReview = (time_t)atol(f[3]);
                    if(g_words[i].progress.knownCount >= MASTERED_THRESHOLD) {
                        g_words[i].progress.mastered = true;
                    }
                    break;
                }
            }
        }
    }
    fclose(fp);
}

void clearProgress(void) {
    for (int i = 0; i < g_wordProgressCount; i++) {
        g_words[i].progress.knownCount = 0;
        g_words[i].progress.unknownCount = 0;
        g_words[i].progress.lastReview = 0;
        g_words[i].progress.mastered = false;
    }
    saveProgress();
}

// 通配符匹配 (支持 ^ $ * ?)
static bool matchPattern(const char* word, const char* pattern) {
    int pl = strlen(pattern);
    if(!strchr(pattern, '*') && !strchr(pattern, '?')
        && pattern[0] != '^' && pattern[pl-1] != '$') {
        char wlow[256], plow[256];
        strcpy(wlow, word);
        strcpy(plow, pattern);
        for (int i = 0; wlow[i]; i++) { wlow[i] = tolower(wlow[i]); }
        for (int i = 0; plow[i]; i++) { plow[i] = tolower(plow[i]); }
        return strstr(wlow, plow) != NULL;
    }
    bool start = (pattern[0] == '^');
    bool end = (pl > 0 && pattern[pl-1] == '$');
    const char* p = pattern + (start ? 1 : 0);
    int rl = pl - (start ? 1 : 0) - (end ? 1 : 0);
    char segs[16][64];
    int sc = 0;
    char buf[256];
    strncpy(buf, p, rl);
    buf[rl] = '\0';
    char* stok = strtok(buf, "*");
    while (stok && sc < 16) {
        strcpy(segs[sc], stok);
        for (int j = 0; segs[sc][j]; j++) { segs[sc][j] = tolower(segs[sc][j]); }
        sc++;
        stok = strtok(NULL, "*");
    }
    if(sc == 0) { return true; }
    bool sw = (p[0] == '*');
    bool ew = (rl > 0 && p[rl-1] == '*');
    char wlow[256];
    strcpy(wlow, word);
    for (int j = 0; wlow[j]; j++) { wlow[j] = tolower(wlow[j]); }
    int pos = 0;
    for (int s = 0; s < sc; s++) {
        char* found = strstr(wlow + pos, segs[s]);
        if(found == NULL) { return false; }
        if(s == 0 && !sw && found != wlow) { return false; }
        pos = (int)(found - wlow) + strlen(segs[s]);
    }
    if(!ew && pos != strlen(wlow)) { return false; }
    return true;
}

int searchWordsByRegex(const char* pattern, int* results, int max) {
    if(pattern == NULL || strlen(pattern) == 0) { return 0; }
    int cnt = 0;
    for (int i = 0; i < g_wordProgressCount && cnt < max; i++) {
        if(matchPattern(g_words[i].entry.word, pattern)) { results[cnt++] = i; }
    }
    return cnt;
}

int searchWordsSimple(const char* query, int* results, int max) {
    if(query == NULL || strlen(query) == 0) { return 0; }
    char ql[256];
    strcpy(ql, query);
    for (int i = 0; ql[i]; i++) { ql[i] = tolower(ql[i]); }
    int cnt = 0;
    for (int i = 0; i < g_wordProgressCount && cnt < max; i++) {
        char wl[256];
        strcpy(wl, g_words[i].entry.word);
        for (int j = 0; wl[j]; j++) { wl[j] = tolower(wl[j]); }
        if(strstr(wl, ql)) { results[cnt++] = i; }
    }
    return cnt;
}

bool saveWordsToFile(const char* filename) {
    FILE* fp = fopen(filename, "w");
    if(fp == NULL) { return false; }
    for (int i = 0; i < g_wordCount; i++) {
        fprintf(fp, "%s|%s|%s|%s|%s\n",
            g_wordLibrary[i].word ? g_wordLibrary[i].word : "",
            g_wordLibrary[i].phonetic ? g_wordLibrary[i].phonetic : "",
            g_wordLibrary[i].definition ? g_wordLibrary[i].definition : "",
            g_wordLibrary[i].example ? g_wordLibrary[i].example : "",
            g_wordLibrary[i].exampleTranslation ? g_wordLibrary[i].exampleTranslation : "");
    }
    fclose(fp);
    return true;
}

bool addWordToLibrary(const char* w, const char* ph, const char* def,
                      const char* ex, const char* ext) {
    if(w == NULL || strlen(w) == 0) { return false; }
    if(g_wordCount >= g_wordLibraryCapacity) {
        g_wordLibraryCapacity = g_wordLibraryCapacity == 0 ? 50 : g_wordLibraryCapacity * 2;
        g_wordLibrary = (WordEntry*)realloc(g_wordLibrary, g_wordLibraryCapacity * sizeof(WordEntry));
    }
    g_wordLibrary[g_wordCount].word = strdup(w);
    g_wordLibrary[g_wordCount].phonetic = strdup(ph ? ph : "");
    g_wordLibrary[g_wordCount].definition = strdup(def ? def : "");
    g_wordLibrary[g_wordCount].example = strdup(ex ? ex : "");
    g_wordLibrary[g_wordCount].exampleTranslation = strdup(ext ? ext : "");
    g_wordCount++;
    saveWordsToFile(WORDS_FILE_PATH);
    return true;
}

bool editWordInLibrary(int idx, const char* w, const char* ph,
                       const char* def, const char* ex, const char* ext) {
    if(idx < 0 || idx >= g_wordCount) { return false; }
    if(w == NULL || strlen(w) == 0) { return false; }
    free((void*)g_wordLibrary[idx].word);
    free((void*)g_wordLibrary[idx].phonetic);
    free((void*)g_wordLibrary[idx].definition);
    free((void*)g_wordLibrary[idx].example);
    free((void*)g_wordLibrary[idx].exampleTranslation);
    g_wordLibrary[idx].word = strdup(w);
    g_wordLibrary[idx].phonetic = strdup(ph ? ph : "");
    g_wordLibrary[idx].definition = strdup(def ? def : "");
    g_wordLibrary[idx].example = strdup(ex ? ex : "");
    g_wordLibrary[idx].exampleTranslation = strdup(ext ? ext : "");
    saveWordsToFile(WORDS_FILE_PATH);
    return true;
}

bool deleteWordFromLibrary(int idx) {
    if(idx < 0 || idx >= g_wordCount) { return false; }
    free((void*)g_wordLibrary[idx].word);
    free((void*)g_wordLibrary[idx].phonetic);
    free((void*)g_wordLibrary[idx].definition);
    free((void*)g_wordLibrary[idx].example);
    free((void*)g_wordLibrary[idx].exampleTranslation);
    for (int i = idx; i < g_wordCount - 1; i++) { g_wordLibrary[i] = g_wordLibrary[i+1]; }
    g_wordCount--;
    saveWordsToFile(WORDS_FILE_PATH);
    return true;
}

void reloadWords(void) {
    int old = g_wordProgressCount;
    g_wordProgressCount = 0;
    for (int i = 0; i < g_wordCount && i < MAX_WORDS; i++) {
        g_words[i].entry = g_wordLibrary[i];
        g_words[i].progress.wordIndex = i;
        if(i >= old) {
            g_words[i].progress.knownCount = 0;
            g_words[i].progress.unknownCount = 0;
            g_words[i].progress.lastReview = 0;
            g_words[i].progress.mastered = false;
        }
        g_wordProgressCount++;
    }
}
