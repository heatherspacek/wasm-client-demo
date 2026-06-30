#include <stdint.h>
#include "font_myscratch.h" // private! font licensing :)

#define SCR_W 640
#define SCR_H 480
#define SCR_N (SCR_H * SCR_W)
#define PX(x, y) ((y) * SCR_W + (x))
#define CLIPX(x) ((x) < (0) ? (0) : ((x) >= (SCR_W) ? (SCR_W) : (x)))

extern void js_log_s(const char *text, uint32_t len);
extern void js_log_f(float value);
extern void js_log_i(int value);

typedef uint32_t Pixel;

typedef struct
{
    float mouse_x;
    float mouse_y;
    float mouse_buttons; // bitmask: bit 0 = left, bit 1 = middle, bit 2 = right
} InputState;

typedef struct
{
    uint8_t pending;
    uint8_t message_type;
    uint8_t callback_id;
    uint16_t body_len;
    uint8_t message[1024];
} Outbox;

typedef struct
{
    uint8_t ready;
    uint8_t _pad;
    uint32_t len;
    uint8_t data[1024];
} Inbox;

static Outbox outbox = {0};
static Inbox inbox = {0};
static InputState inputs = {0};
static Pixel scr_buf[SCR_W * SCR_H] = {0};

__attribute__((export_name("outbox_ptr")))
uint32_t
outbox_ptr(void) { return (uint32_t)(uintptr_t)&outbox; }

__attribute__((export_name("inbox_ptr")))
uint32_t
inbox_ptr(void) { return (uint32_t)(uintptr_t)&inbox; }

__attribute__((export_name("input_state_ptr")))
uint32_t
input_state_ptr(void)
{
    return (uint32_t)(uintptr_t)&inputs;
}
__attribute__((export_name("screenbuffer_ptr")))
uint32_t
screenbuffer_ptr(void) { return (uint32_t)(uintptr_t)&scr_buf; }

////////////////////////////////////////////////////////////////////////
// ################################################################## //

///////
// drawings
///////
void _wipe_scr()
{
    for (int pxi = 0; pxi < SCR_N; pxi++)
    {
        scr_buf[pxi] = 0;
    }
}
void _write_px(Pixel val, int x, int y)
{
    scr_buf[PX(x, y)] = val;
}
void _blit_glyph_8wide(const uint8_t *bits, int rows, int x, int y)
{
    for (int row_i = 0; row_i < rows; row_i++)
    {
        uint8_t this_row = bits[row_i];
        for (int col_i = 0; col_i < 8; col_i++)
            if (this_row & (0x80 >> col_i))
            {
                _write_px(0xFFFFFFFF, x + col_i, y + row_i);
            }
    }
}
void _draw_rect(Pixel col, int x1, int y1, int x2, int y2)
{
    if (x1 < 0)
        x1 = 0;
    if (x2 < 0)
        x2 = 0;
    if (y1 < 0)
        y1 = 0;
    if (y2 < 0)
        y2 = 0;
    if (x1 >= SCR_W)
        x1 = SCR_W - 1;
    if (x2 >= SCR_W)
        x2 = SCR_W - 1;
    if (y1 >= SCR_H)
        y1 = SCR_H - 1;
    if (y2 >= SCR_H)
        y2 = SCR_H - 1;
    for (int xx = x1; xx <= x2; xx++)
    {
        _write_px(col, xx, y1);
        _write_px(col, xx, y2);
    }
    for (int yy = y1; yy <= y2; yy++)
    {
        _write_px(col, x1, yy);
        _write_px(col, x2, yy);
    }
}

static int FRAME_CNT = 0;
static int CURR_SCREEN = 0;

static _Bool DBG_TOGGLE = 0;

#define MAX_BTNS 15
#define MAX_SCREENS 6

#define SCREEN_TITLE_ID 0
#define SCREEN_SETTINGS_ID 1
#define SCREEN_DRAFT_WEAP_ID 2
#define SCREEN_DRAFT_SPELLS_ID 3
#define SCREEN_COMBAT_ID 4

typedef struct
{
    int count;
    const char *data;
} String_View;
#define SV(cstr_lit) sv_from_parts(cstr_lit, sizeof(cstr_lit) - 1)
String_View sv_from_parts(const char *data, int count)
{
    String_View sv;
    sv.count = count;
    sv.data = data;
    return sv;
}

struct ui_button
{
    int x;
    int y;
    int w;
    int h;
    int click_state;
    String_View label;
    void (*cbk)();
};

struct screen
{
    struct ui_button buttons[MAX_BTNS];
    int n_buttons;
};
static struct screen all_screens[MAX_SCREENS];

void cbk_goto_settings()
{
    CURR_SCREEN = SCREEN_SETTINGS_ID;
}

void cbk_goto_title()
{
    CURR_SCREEN = SCREEN_TITLE_ID;
}

void dummy_cbk() {}

void _init_ui_button(struct screen *target_scr,
                     int x, int y, int w, int h, String_View label, void *cbk)
{
    struct ui_button newbtn = {x, y, w, h, 0, label, cbk};
    target_scr->buttons[target_scr->n_buttons] = newbtn;
    target_scr->n_buttons++;
}

void _render_sv(int x, int y, String_View sv, int doublesize)
{
    int x_offset = 0;
    for (int i = 0; i < sv.count; i++)
    {
        char thischar = sv.data[i];
        if (thischar == ' ')
        {
            x_offset += 5 * (doublesize + 1);
            continue;
        }
        for (int g_i = 0; g_i < N_CHARS; g_i++)
        {
            Glyph match_g = all_glyphs[g_i];
            if (match_g.character == thischar)
            {
                if (doublesize)
                {
                    // js_draw_glyph_16wide(match_g.data_16, 2 * match_g.rows, x + x_offset, y);
                    x_offset += 2 * (match_g.spacing - 1);
                }
                else
                {
                    _blit_glyph_8wide(match_g.data_8, match_g.rows, x + x_offset, y);
                    x_offset += match_g.spacing - 1;
                }
                break;
            }
        }
    }
}

__attribute__((export_name("init"))) void init()
{
    // TODO: loading bar, for when init gets huge!
    struct screen screen_title;
    struct screen screen_settings;
    struct screen screen_draft_weapon;

    _init_ui_button(&screen_title, 20, 205, 105, 35, SV("Start Game"), dummy_cbk);
    _init_ui_button(&screen_title, 20, 275, 105, 35, SV("Settings"), cbk_goto_settings);
    _init_ui_button(&screen_title, 20, 345, 105, 35, SV("DEBUG"), dummy_cbk);

    _init_ui_button(&screen_settings, 160, 100, 35, 35, SV("<"), dummy_cbk);
    _init_ui_button(&screen_settings, 195, 100, 35, 35, SV(">"), dummy_cbk);
    _init_ui_button(&screen_settings, 160, 150, 50, 35, SV("..."), dummy_cbk);
    _init_ui_button(&screen_settings, 120, 200, 105, 35, SV("Back to main menu"), cbk_goto_title);

    all_screens[SCREEN_TITLE_ID] = screen_title;
    all_screens[SCREEN_SETTINGS_ID] = screen_settings;
    all_screens[SCREEN_DRAFT_WEAP_ID] = screen_draft_weapon;
}

__attribute__((export_name("update"))) void update(double timestamp_ms)
{
    (void)timestamp_ms;

    // perform UI state updates for the current screen.
    struct screen this_scr = all_screens[CURR_SCREEN];

    for (int i = 0; i < this_scr.n_buttons; i++)
    {
        struct ui_button bxx = this_scr.buttons[i];
        int right = bxx.x + bxx.w;
        int bot = bxx.y + bxx.h;
        int hov = inputs.mouse_x > (float)bxx.x &&
                  inputs.mouse_y > (float)bxx.y &&
                  inputs.mouse_x < (float)right &&
                  inputs.mouse_y < (float)bot;
        // write back!
        all_screens[CURR_SCREEN].buttons[i].click_state = hov;
        if (hov && inputs.mouse_buttons)
        {
            bxx.cbk();
        }
    }
    FRAME_CNT++;
}

__attribute__((export_name("draw"))) void draw(void)
{
    _wipe_scr();
    _render_sv(145, 120, SV("Sphinx of black quartz, judge my vow!"), 0);
    _render_sv(145, 160, SV("THE QUICK BROWN FOX JUMPED OVER THE LAZY DOG"), 0);

    _draw_rect(0xFFBBCCFF, inputs.mouse_x - 3, inputs.mouse_y - 3, inputs.mouse_x + 3, inputs.mouse_y + 3);
    // struct screen this_scr = all_screens[CURR_SCREEN];
}
