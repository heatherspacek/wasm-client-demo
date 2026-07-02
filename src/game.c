
#include "globals.h"
#include "data/font_myscratch.h"
#include "data/spritesheet.h"

#include "drawing.h"

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

void _render_sv(int x, int y, String_View sv)
{
    int x_offset = 0;
    for (int i = 0; i < sv.count; i++)
    {
        char thischar = sv.data[i];
        if (thischar == ' ')
        {
            x_offset += 5;
            continue;
        }
        for (int g_i = 0; g_i < N_CHARS; g_i++)
        {
            Glyph match_g = all_glyphs[g_i];
            if (match_g.character == thischar)
            {
                _blit_glyph_8wide(match_g.data_8, match_g.rows, x + x_offset, y);
                x_offset += match_g.spacing - 1;
                break;
            }
        }
    }
}

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
    _render_sv(145, 120, SV("Sphinx of black quartz, judge my vow!"));
    _render_sv(145, 160, SV("THE QUICK BROWN FOX JUMPED OVER THE LAZY DOG"));

    _draw_rect(0xFFFFFFFF, inputs.mouse_x - 2, inputs.mouse_y - 2, inputs.mouse_x + 2, inputs.mouse_y + 2);
    _draw_sprite(sprite_0, inputs.mouse_x - 16, inputs.mouse_y);
    // if (inputs.mouse_buttons == (float)0)
    // {
    //     _draw_sprite(sprite_0, inputs.mouse_x - 16, inputs.mouse_y);
    // }
    // else if (inputs.mouse_buttons == (float)1)
    // {
    //     _draw_sprite(sprite_1, inputs.mouse_x - 16, inputs.mouse_y);
    // }

    struct screen this_scr = all_screens[CURR_SCREEN];
}
