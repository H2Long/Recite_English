// 立即模式UI组件库 — 实现

#include "raylib_word_ui.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

extern Vector2 MeasureTextAuto(const char* text, float fontSize, float spacing);

static Font g_chFont = {0};
static Font g_enFont = {0};
static Font g_latinFont = {0};

void UISetFonts(Font cf, Font ef) { g_chFont = cf; g_enFont = ef; }
void UISetLatinFont(Font lf) { g_latinFont = lf; }

// UTF-8 辅助
static int utf8_char_len(unsigned char c) {
    if(c < 0x80) { return 1; }
    if((c & 0xE0) == 0xC0) { return 2; }
    if((c & 0xF0) == 0xE0) { return 3; }
    if((c & 0xF8) == 0xF0) { return 4; }
    return 1;
}

static inline bool isLatinExtended(int c) {
    if(c >= 0x00A0 && c <= 0x00FF) { return true; }
    if(c >= 0x0100 && c <= 0x017F) { return true; }
    if(c >= 0x0180 && c <= 0x024F) { return true; }
    if(c >= 0x0250 && c <= 0x02AF) { return true; }
    if(c >= 0x02B0 && c <= 0x02FF) { return true; }
    if(c >= 0x0300 && c <= 0x036F) { return true; }
    if(c >= 0x0370 && c <= 0x03FF) { return true; }
    if(c >= 0x1D00 && c <= 0x1DBF) { return true; }
    if(c >= 0x1E00 && c <= 0x1EFF) { return true; }
    return false;
}

static int utf8_to_codepoint(const unsigned char* text) {
    unsigned char b = text[0];
    if(b < 0x80) { return b; }
    if(b < 0xE0) { return ((b & 0x1F) << 6) | (text[1] & 0x3F); }
    if(b < 0xF0) { return ((b & 0x0F) << 12) | ((text[1] & 0x3F) << 6) | (text[2] & 0x3F); }
    return ((b & 0x07) << 18) | ((text[1] & 0x3F) << 12) | ((text[2] & 0x3F) << 6) | (text[3] & 0x3F);
}

static Font UIGetFontForChar(const unsigned char* text) {
    if(text[0] < 0x80) { return g_enFont.texture.id != 0 ? g_enFont : GetFontDefault(); }
    int cp = utf8_to_codepoint(text);
    if(isLatinExtended(cp)) {
        if(g_latinFont.texture.id != 0) { return g_latinFont; }
        if(g_chFont.texture.id != 0) { return g_chFont; }
    }
    return g_chFont.texture.id != 0 ? g_chFont : GetFontDefault();
}

static int utf8_strlen(const char* str) {
    int n = 0;
    while (*str) { str += utf8_char_len((unsigned char)*str); n++; }
    return n;
}

static int utf8_index_to_byte(const char* str, int char_idx) {
    int off = 0, cnt = 0;
    while (str[off] && cnt < char_idx) {
        off += utf8_char_len((unsigned char)str[off]);
        cnt++;
    }
    return off;
}

static int utf8_insert_codepoint(char* buf, int bufsize, int* cursor, int cp) {
    int off = utf8_index_to_byte(buf, *cursor);
    int old_len = strlen(buf);
    char u8[5] = {0};
    int u8len = 0;
    if(cp < 0x80)           { u8[0] = (char)cp; u8len = 1; }
    else if(cp < 0x800)     { u8[0] = 0xC0|(cp>>6); u8[1] = 0x80|(cp&0x3F); u8len = 2; }
    else if(cp < 0x10000)   { u8[0] = 0xE0|(cp>>12); u8[1] = 0x80|((cp>>6)&0x3F); u8[2] = 0x80|(cp&0x3F); u8len = 3; }
    else                    { u8[0] = 0xF0|(cp>>18); u8[1] = 0x80|((cp>>12)&0x3F); u8[2] = 0x80|((cp>>6)&0x3F); u8[3] = 0x80|(cp&0x3F); u8len = 4; }
    if(old_len + u8len >= bufsize) { return 0; }
    memmove(buf + off + u8len, buf + off, old_len - off + 1);
    memcpy(buf + off, u8, u8len);
    (*cursor)++;
    return u8len;
}

static void utf8_delete_left(char* buf, int* cursor) {
    if(*cursor <= 0) { return ; }
    int off = utf8_index_to_byte(buf, *cursor);
    int prev = 0;
    while (prev < off) {
        int len = utf8_char_len((unsigned char)buf[prev]);
        if(prev + len >= off) { break; }
        prev += len;
    }
    memmove(buf + prev, buf + off, strlen(buf + off) + 1);
    *cursor = utf8_strlen(buf) - utf8_strlen(buf + prev);
}

static void utf8_delete_right(char* buf, int* cursor) {
    int off = utf8_index_to_byte(buf, *cursor);
    if(buf[off] == '\0') { return ; }
    int next_len = utf8_char_len((unsigned char)buf[off]);
    memmove(buf + off, buf + off + next_len, strlen(buf + off + next_len) + 1);
}

// 主题
UITheme UIThemeLight(void) {
    return (UITheme){
        .primary = {70,130,180,255}, .primaryHover = {100,149,237,255},
        .primaryPressed = {65,105,225,255}, .secondary = {211,211,211,255},
        .background = {245,245,245,255}, .panelBg = {255,255,255,255},
        .textPrimary = {30,30,30,255}, .textSecondary = {120,120,120,255},
        .inputBg = {255,255,255,255}, .inputBorder = {200,200,200,255},
        .error = {220,20,60,255}, .success = {50,205,50,255}
    };
}

UITheme UIThemeDark(void) {
    return (UITheme){
        .primary = {70,130,180,255}, .primaryHover = {100,149,237,255},
        .primaryPressed = {65,105,225,255}, .secondary = {80,80,80,255},
        .background = {30,30,30,255}, .panelBg = {45,45,45,255},
        .textPrimary = {240,240,240,255}, .textSecondary = {180,180,180,255},
        .inputBg = {60,60,60,255}, .inputBorder = {100,100,100,255},
        .error = {255,80,80,255}, .success = {80,200,80,255}
    };
}

void UIStyleInit(UIStyle* style) {
    *style = (UIStyle){.theme = UIThemeLight(), .font = GetFontDefault(),
        .fontSizeSmall = 16, .fontSizeNormal = 20, .fontSizeLarge = 28,
        .spacing = 8, .cornerRadius = 6};
}

void UIBegin(UIState* state) {
    state->mousePos = GetMousePosition();
    state->mouseDown = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    state->mousePressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    state->mouseReleased = IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
    state->keyPressed = GetKeyPressed();
    state->charPressed = GetCharPressed();
    state->hotItem = 0;
}

void UIEnd(UIState* state) {
    if(!state->mouseDown) { state->activeItem = 0; }
}

int UIGetID(const char* label) {
    int id = 0;
    while (*label) { id = id * 31 + (*label++); }
    return id;
}

// 布局
UILayout UIBeginLayout(Rectangle container, UIDirection dir, float spacing, float padding) {
    return (UILayout){.container = container, .direction = dir,
        .spacing = spacing, .padding = padding, .cursor = {padding, padding}};
}

Rectangle UILayoutNext(UILayout* layout, float width, float height) {
    Rectangle r = {0};
    float cw = layout->container.width - 2 * layout->padding;
    float ch = layout->container.height - 2 * layout->padding;
    if(width < 0) { width = cw; }
    if(height < 0) { height = ch - layout->cursor.y; }
    if(layout->direction == UI_DIR_VERTICAL) {
        r.x = layout->container.x + layout->padding + layout->cursor.x;
        r.y = layout->container.y + layout->padding + layout->cursor.y;
        r.width = width; r.height = height;
        layout->cursor.y += height + layout->spacing;
    }
    else {
        if(width < 0) { width = cw - layout->cursor.x; }
        r.x = layout->container.x + layout->padding + layout->cursor.x;
        r.y = layout->container.y + layout->padding + layout->cursor.y;
        r.width = width; r.height = height;
        layout->cursor.x += width + layout->spacing;
    }
    return r;
}

// 按钮
bool UIButtonEx(const char* label, Rectangle rect, UIStyle* style, UIState* state, bool enabled, int id) {
    bool inside = CheckCollisionPointRec(state->mousePos, rect);
    Color bg = enabled ? style->theme.primary : style->theme.secondary;
    Color tc = enabled ? style->theme.textPrimary : style->theme.textSecondary;
    if(enabled && inside) {
        state->hotItem = id;
        if(state->mousePressed) { state->activeItem = id; }
    }
    if(state->activeItem == id && enabled) { bg = style->theme.primaryPressed; }
    else if(state->hotItem == id && enabled) { bg = style->theme.primaryHover; }
    float cr = style->cornerRadius / rect.height * 10;
    DrawRectangleRounded(rect, cr, 8, bg);
    DrawRectangleRoundedLines(rect, cr, 8, Fade(BLACK, 0.2f));
    Vector2 size = MeasureTextEx(style->font, label, style->fontSizeNormal, 1);
    UIDrawText(label, (Vector2){rect.x+(rect.width-size.x)/2, rect.y+(rect.height-size.y)/2},
        style->fontSizeNormal, 1, tc);
    return enabled && state->mouseReleased && state->hotItem == id && state->activeItem == id;
}

bool UIButton(const char* label, Rectangle rect, UIStyle* style, UIState* state, int id) {
    return UIButtonEx(label, rect, style, state, true, id);
}

bool UICheckbox(const char* label, Rectangle rect, bool* checked, UIStyle* style, UIState* state) {
    int id = UIGetID(label);
    bool inside = CheckCollisionPointRec(state->mousePos, rect);
    if(inside) {
        state->hotItem = id;
        if(state->mousePressed) { state->activeItem = id; }
    }
    Color bg = *checked ? style->theme.primary : style->theme.inputBg;
    float cr = style->cornerRadius / rect.height * 10;
    DrawRectangleRounded(rect, cr, 8, bg);
    DrawRectangleRoundedLines(rect, cr, 8, style->theme.inputBorder);
    if(*checked) {
        Vector2 c = {rect.x+rect.width/2, rect.y+rect.height/2};
        DrawLineEx((Vector2){c.x-6,c.y}, (Vector2){c.x-2,c.y+5}, 3, WHITE);
        DrawLineEx((Vector2){c.x-2,c.y+5}, (Vector2){c.x+6,c.y-5}, 3, WHITE);
    }
    if(label && *label) {
        UIDrawText(label, (Vector2){rect.x+rect.width+8, rect.y+(rect.height-style->fontSizeNormal)/2},
            style->fontSizeNormal, 1, style->theme.textPrimary);
    }
    if(state->mouseReleased && state->hotItem == id && state->activeItem == id) {
        *checked = !*checked;
        return true;
    }
    return false;
}

void UILabel(const char* text, Rectangle rect, UIStyle* style, Color color) {
    UIDrawTextRec(text, rect, style->fontSizeNormal, 1, true, color);
}

// 文本框
bool UITextBox(UITextBoxState* box, Rectangle rect, UIStyle* style, UIState* state, bool password) {
    int id = (int)(unsigned long)(void*)box;
    bool inside = CheckCollisionPointRec(state->mousePos, rect);
    if(state->mousePressed) {
        if(inside) {
            state->focusItem = id;
            box->hasFocus = true;
            box->cursor = utf8_strlen(box->buffer);
        }
        else if(box->hasFocus) { box->hasFocus = false; }
    }
    if(inside) { state->hotItem = id; }
    if(box->hasFocus) {
        int key = state->keyPressed;
        if(key == KEY_BACKSPACE) { utf8_delete_left(box->buffer, &box->cursor); }
        if(key == KEY_DELETE) { utf8_delete_right(box->buffer, &box->cursor); }
        if(key == KEY_LEFT && box->cursor > 0) { box->cursor--; }
        if(key == KEY_RIGHT && box->cursor < utf8_strlen(box->buffer)) { box->cursor++; }
        if(state->charPressed >= 32) {
            utf8_insert_codepoint(box->buffer, sizeof(box->buffer), &box->cursor, state->charPressed);
        }
    }
    float cr = style->cornerRadius / rect.height * 10;
    DrawRectangleRounded(rect, cr, 8, style->theme.inputBg);
    Color bc = box->hasFocus ? style->theme.primary : style->theme.inputBorder;
    DrawRectangleRoundedLines(rect, cr, 8, bc);
    const char* dt = box->buffer;
    char pwb[1024] = {0};
    if(password && *dt) {
        int n = utf8_strlen(dt);
        memset(pwb, '*', n);
        dt = pwb;
    }
    UIDrawTextRec(dt, (Rectangle){rect.x+8, rect.y, rect.width-16, rect.height},
        style->fontSizeNormal, 1, false, style->theme.textPrimary);
    if(box->hasFocus) {
        int off = utf8_index_to_byte(box->buffer, box->cursor);
        char before[1024];
        strncpy(before, box->buffer, off);
        before[off] = '\0';
        float cx = rect.x + 8 + MeasureTextAuto(before, style->fontSizeNormal, 1).x;
        DrawLineEx((Vector2){cx, rect.y+4}, (Vector2){cx, rect.y+rect.height-4}, 2, style->theme.textPrimary);
    }
    return box->hasFocus && state->keyPressed == KEY_ENTER;
}

// 文本绘制
void UIDrawText(const char* text, Vector2 pos, float font_size, float spacing, Color tint) {
    if(!text || text[0] == '\0') { return ; }
    float x = pos.x;
    while (*text) {
        unsigned char uc = *(unsigned char*)text;
        if(uc == ' ' || uc == '\t') {
            Font sf = g_enFont.texture.id != 0 ? g_enFont : GetFontDefault();
            x += MeasureTextEx(sf, " ", font_size, spacing).x;
            text++;
            continue;
        }
        Font cur = UIGetFontForChar((const unsigned char*)text);
        int len = utf8_char_len(uc);
        char seg[5] = {0};
        for (int i = 0; i < len && *text; i++) { seg[i] = *text++; }
        DrawTextEx(cur, seg, (Vector2){x, pos.y}, font_size, spacing, tint);
        x += MeasureTextEx(cur, seg, font_size, spacing).x;
    }
}

void UIDrawTextRec(const char* text, Rectangle rec, float font_size, float spacing, bool word_wrap, Color tint) {
    if(!text || text[0] == '\0') { return ; }
    BeginScissorMode((int)rec.x, (int)rec.y, (int)rec.width, (int)rec.height);
    float x = rec.x, y = rec.y;
    while (*text && y < rec.y + rec.height) {
        unsigned char uc = *(unsigned char*)text;
        if(uc == ' ' || uc == '\t') {
            Font sf = g_enFont.texture.id != 0 ? g_enFont : GetFontDefault();
            x += MeasureTextEx(sf, " ", font_size, spacing).x;
            text++;
            continue;
        }
        if(uc == '\n') {
            text++;
            y += font_size + spacing;
            x = rec.x;
            continue;
        }
        Font cur = UIGetFontForChar((const unsigned char*)text);
        if(cur.texture.id == 0) { cur = GetFontDefault(); }
        // 收集连续同字体字符
        char seg[256] = {0};
        int seg_len = 0;
        float seg_width = 0;
        Font seg_font = cur;
        while (*text && seg_len < 250) {
            Font cf = UIGetFontForChar((const unsigned char*)text);
            if(cf.texture.id == 0) { cf = GetFontDefault(); }
            if(cf.texture.id != seg_font.texture.id && seg_len > 0) { break; }
            seg_font = cf;
            int len = utf8_char_len(uc);
            for (int i = 0; i < len && *text; i++) { seg[seg_len++] = *text++; }
            uc = *(unsigned char*)text;
            seg_width = MeasureTextEx(seg_font, seg, font_size, spacing).x;
        }
        if(word_wrap && x != rec.x && x + seg_width > rec.x + rec.width) {
            y += font_size + spacing;
            x = rec.x;
        }
        DrawTextEx(seg_font, seg, (Vector2){x, y}, font_size, spacing, tint);
        x += seg_width;
    }
    EndScissorMode();
}

// 滚动视图
void UIBeginScrollView(UIScrollView* scroll, Rectangle viewport, Vector2 content_size) {
    scroll->viewport = viewport;
    scroll->contentSize = content_size;
    scroll->showScrollbar = (content_size.y > viewport.height);
    if(scroll->scrollOffset.y < 0) { scroll->scrollOffset.y = 0; }
    float max_y = content_size.y - viewport.height;
    if(max_y < 0) { max_y = 0; }
    if(scroll->scrollOffset.y > max_y) { scroll->scrollOffset.y = max_y; }
    BeginScissorMode((int)viewport.x, (int)viewport.y, (int)viewport.width, (int)viewport.height);
}

void UIEndScrollView(UIScrollView* scroll, UIStyle* style, UIState* state) {
    EndScissorMode();
    if(CheckCollisionPointRec(state->mousePos, scroll->viewport)) {
        float wheel = GetMouseWheelMove();
        if(wheel != 0) { scroll->scrollOffset.y -= wheel * 30; }
    }
    float max_y = scroll->contentSize.y - scroll->viewport.height;
    if(max_y < 0) { max_y = 0; }
    if(scroll->scrollOffset.y < 0) { scroll->scrollOffset.y = 0; }
    if(scroll->scrollOffset.y > max_y) { scroll->scrollOffset.y = max_y; }
    if(scroll->showScrollbar) {
        float bar_h = scroll->viewport.height * (scroll->viewport.height / scroll->contentSize.y);
        float bar_y = scroll->viewport.y + (scroll->scrollOffset.y / scroll->contentSize.y) * scroll->viewport.height;
        DrawRectangleRec((Rectangle){scroll->viewport.x+scroll->viewport.width-8, bar_y, 6, bar_h},
            Fade(style->theme.textSecondary, 0.5f));
    }
}

void UIBeginPersistentScrollView(PersistentScrollView* psv, Rectangle viewport,
                                 Vector2 content_size, float* offset) {
    psv->persistedOffset = offset;
    psv->sv.scrollOffset.y = *offset;
    UIBeginScrollView(&psv->sv, viewport, content_size);
}

void UIEndPersistentScrollView(PersistentScrollView* psv, UIStyle* style, UIState* state) {
    UIEndScrollView(&psv->sv, style, state);
    *psv->persistedOffset = psv->sv.scrollOffset.y;
}

bool UIListItem(const char* text, Rectangle rect, UIStyle* style, UIState* state) {
    int id = UIGetID(text);
    bool inside = CheckCollisionPointRec(state->mousePos, rect);
    if(inside) {
        state->hotItem = id;
        if(state->mousePressed) { state->activeItem = id; }
    }
    Color bg = state->activeItem == id ? style->theme.primaryPressed :
               state->hotItem == id ? style->theme.primaryHover : BLANK;
    if(bg.a > 0) { DrawRectangleRec(rect, bg); }
    UIDrawText(text, (Vector2){rect.x+8, rect.y+(rect.height-style->fontSizeNormal)/2},
        style->fontSizeNormal, 1, style->theme.textPrimary);
    return state->mouseReleased && state->hotItem == id && state->activeItem == id;
}

// 单词卡片
void UIWordCard(WordEntry* entry, Rectangle rect, UIStyle* style) {
    DrawRectangleRounded(rect, 0.1f, 8, style->theme.panelBg);
    DrawRectangleRoundedLines(rect, 0.1f, 8, style->theme.inputBorder);
    UILayout lay = UIBeginLayout(rect, UI_DIR_VERTICAL, 4, 12);
    Rectangle wr = UILayoutNext(&lay, -1, style->fontSizeLarge + 8);
    UIDrawText(entry->word, (Vector2){wr.x, wr.y}, style->fontSizeLarge, 1, style->theme.textPrimary);
    if(entry->phonetic && *entry->phonetic) {
        Rectangle pr = UILayoutNext(&lay, -1, style->fontSizeNormal + 4);
        UIDrawText(entry->phonetic, (Vector2){pr.x, pr.y}, style->fontSizeNormal, 1, style->theme.textSecondary);
    }
    Rectangle lr = UILayoutNext(&lay, -1, 2);
    DrawLineEx((Vector2){lr.x,lr.y}, (Vector2){lr.x+lr.width,lr.y}, 1, style->theme.inputBorder);
    Rectangle dr = UILayoutNext(&lay, -1, style->fontSizeNormal*2);
    UIDrawTextRec(entry->definition, dr, style->fontSizeNormal, 1, true, style->theme.textPrimary);
    if(entry->example && *entry->example) {
        Rectangle er = UILayoutNext(&lay, -1, style->fontSizeNormal*3);
        char buf[512];
        snprintf(buf, sizeof(buf), "例: %s", entry->example);
        UIDrawTextRec(buf, er, style->fontSizeNormal, 1, true, style->theme.textSecondary);
    }
}

// 闪卡
static float Lerp(float a, float b, float t) { return a + (b - a) * t; }

int UIFlashCard(WordEntry* entry, Rectangle rect, CardFace* face,
                UIStyle* style, UIState* state, float* anim_time) {
    float target = (*face == CARD_FRONT) ? 0.0f : 1.0f;
    *anim_time = Lerp(*anim_time, target, GetFrameTime() * 5.0f);
    if(fabsf(*anim_time - target) < 0.01f) { *anim_time = target; }
    float scale = 1.0f - fabsf(*anim_time - 0.5f) * 0.2f;
    Rectangle dr = {rect.x+rect.width*(1-scale)/2, rect.y+rect.height*(1-scale)/2,
                    rect.width*scale, rect.height*scale};
    DrawRectangleRounded(dr, 0.1f, 8, style->theme.panelBg);
    DrawRectangleRoundedLines(dr, 0.1f, 8, style->theme.inputBorder);
    if(*anim_time < 0.5f) {
        float fs = style->fontSizeLarge * 1.2f;
        Vector2 ws = MeasureTextEx(style->font, entry->word, fs, 1);
        UIDrawText(entry->word, (Vector2){dr.x+(dr.width-ws.x)/2, dr.y+(dr.height-ws.y)/2},
            fs, 1, style->theme.textPrimary);
    }
    else {
        Rectangle defR = {dr.x+15, dr.y+10, dr.width-30, dr.height*0.6f-20};
        UIDrawTextRec(entry->definition, defR, style->fontSizeLarge*1.2f, 1, true, style->theme.textPrimary);
        if(entry->phonetic && *entry->phonetic) {
            Vector2 ps = MeasureTextEx(style->font, entry->phonetic, style->fontSizeNormal, 1);
            UIDrawText(entry->phonetic, (Vector2){dr.x+(dr.width-ps.x)/2, dr.y+dr.height*0.75f},
                style->fontSizeNormal, 1, style->theme.textSecondary);
        }
    }
    int action = 0;
    if(CheckCollisionPointRec(state->mousePos, dr) && state->mouseReleased) {
        *face = (*face == CARD_FRONT) ? CARD_BACK : CARD_FRONT;
        action = 3;
    }
    if(*anim_time > 0.9f && *face == CARD_BACK) {
        Rectangle br = {rect.x, rect.y+rect.height+40, 220, 40};
        if(UIButton(u8"认识", br, style, state, 100)) { action = 1; }
        br.x = rect.x + rect.width - 220;
        if(UIButton(u8"不认识", br, style, state, 101)) { action = 2; }
    }
    return action;
}

// 选择题
int UIMultipleChoice(const char* question, const char* options[], int option_count,
                     int correct, Rectangle rect, UIStyle* style, UIState* state) {
    UILayout lay = UIBeginLayout(rect, UI_DIR_VERTICAL, 8, 8);
    Rectangle qr = UILayoutNext(&lay, -1, style->fontSizeNormal*2);
    UIDrawTextRec(question, qr, style->fontSizeNormal, 1, true, style->theme.textPrimary);
    int sel = -1;
    for (int i = 0; i < option_count; i++) {
        Rectangle or = UILayoutNext(&lay, -1, 40);
        int id = UIGetID(options[i]);
        bool inside = CheckCollisionPointRec(state->mousePos, or);
        if(inside) {
            state->hotItem = id;
            if(state->mousePressed) { state->activeItem = id; }
        }
        Color bg = state->activeItem == id ? style->theme.primaryPressed :
                   state->hotItem == id ? style->theme.primaryHover : style->theme.panelBg;
        DrawRectangleRounded(or, 0.2f, 8, bg);
        DrawRectangleRoundedLines(or, 0.2f, 8, style->theme.inputBorder);
        char lb[32];
        snprintf(lb, sizeof(lb), "%c. %s", 'A'+i, options[i]);
        UIDrawText(lb, (Vector2){or.x+12, or.y+(or.height-style->fontSizeNormal)/2},
            style->fontSizeNormal, 1, style->theme.textPrimary);
        if(state->mouseReleased && state->hotItem == id && state->activeItem == id) { sel = i; }
    }
    return sel;
}

// 搜索栏
void UISearchBar(SearchBarState* sb, Rectangle rect, UIStyle* style, UIState* state) {
    UILayout lay = UIBeginLayout(rect, UI_DIR_HORIZONTAL, 8, 0);
    Rectangle ir = UILayoutNext(&lay, rect.width - 70, rect.height);
    Rectangle br = UILayoutNext(&lay, 60, rect.height);
    sb->searchTriggered = UITextBox(&sb->textState, ir, style, state, false)
                       || UIButton(u8"搜索", br, style, state, 200);
}

// 单词列表
int UIWordListView(WordEntry* words, int count, int* selected,
                   UIScrollView* scroll, UIStyle* style, UIState* state) {
    int clicked = -1;
    float item_h = 50.0f;
    scroll->contentSize = (Vector2){scroll->viewport.width, item_h * count};
    UIBeginScrollView(scroll, scroll->viewport, scroll->contentSize);
    for (int i = 0; i < count; i++) {
        Rectangle ir = {scroll->viewport.x, scroll->viewport.y+i*item_h-scroll->scrollOffset.y,
                        scroll->viewport.width, item_h};
        if(i == *selected) { DrawRectangleRec(ir, Fade(style->theme.primary, 0.3f)); }
        if(UIListItem(words[i].word, ir, style, state)) {
            clicked = i;
            *selected = i;
        }
    }
    UIEndScrollView(scroll, style, state);
    return clicked;
}
