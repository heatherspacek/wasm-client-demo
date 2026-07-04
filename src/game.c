#include "data/font_myscratch.h"
#include "data/spritesheet.h"

#include "drawing.h"
#include "text.h"

extern void js_log_s(const char *text, uint32_t len);
extern void js_log_f(float value);
extern void js_log_i(int value);
extern void js_set_cursor_default();
extern void js_set_cursor_pointer();
extern void js_set_cursor_grab();
extern void js_set_cursor_grabbing();

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

static int FRAME_CNT = 0;
static int CURR_SCREEN = 0;

static _Bool DBG_TOGGLE = 0;

#define MAX_BTNS 15
#define MAX_LABELS 15
#define MAX_STATICSPRITES 15
#define MAX_SCREENS 6

#define SCREEN_TITLE_ID 0
#define SCREEN_SETTINGS_ID 1
#define SCREEN_DRAFT_WEAP_ID 2
#define SCREEN_DRAFT_SPELLS_ID 3
#define SCREEN_COMBAT_ID 4

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

struct ui_staticsprite
{
    int x;
    int y;
    const uint32_t *sprite;
};

struct ui_label
{
    int x;
    int y;
    String_View label;
};

struct screen
{
    struct ui_button buttons[MAX_BTNS];
    int n_buttons;
    struct ui_label labels[MAX_LABELS];
    int n_labels;
    struct ui_staticsprite staticsprites[MAX_STATICSPRITES];
    int n_staticsprites;
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

void cbk_goto_draft()
{
    CURR_SCREEN = SCREEN_DRAFT_SPELLS_ID;
}

void dummy_cbk() {}

void _init_ui_button(struct screen *target_scr,
                     int x, int y, int w, int h, String_View label, void *cbk)
{
    struct ui_button newbtn = {x, y, w, h, 0, label, cbk};
    target_scr->buttons[target_scr->n_buttons] = newbtn;
    target_scr->n_buttons++;
}

void _init_ui_label(struct screen *target_scr,
                    int x, int y, String_View lab)
{
    struct ui_label newlabel = {x, y, lab};
    target_scr->labels[target_scr->n_labels] = newlabel;
    target_scr->n_labels++;
}

void _init_ui_staticsprite(struct screen *target_scr,
                           int x, int y, const uint32_t *sprite)
{
    struct ui_staticsprite newsprite = {x, y, sprite};
    target_scr->staticsprites[target_scr->n_staticsprites] = newsprite;
    target_scr->n_staticsprites++;
}

__attribute__((export_name("init"))) void init()
{
    // TODO: loading bar, for when init gets huge!
    struct screen screen_title;
    struct screen screen_settings;
    struct screen screen_draft_weapon;
    struct screen screen_draft_spells;

    _init_ui_label(&screen_title, 16, 16, SV("HEATHER'S UNNAMED WIZARD GAME~"));
    _init_ui_label(&screen_title, 16, 48, SV("\" dot com. \""));

    _init_ui_staticsprite(&screen_title, 75, 35, sprite_2);

    _init_ui_button(&screen_title, 20, 205, 85, 24, SV("Start Game"), cbk_goto_draft);
    _init_ui_button(&screen_title, 20, 245, 85, 24, SV("Settings"), cbk_goto_settings);
    _init_ui_button(&screen_title, 20, 285, 85, 24, SV("DEBUG"), dummy_cbk);

    _init_ui_button(&screen_settings, 160, 100, 35, 35, SV("<"), dummy_cbk);
    _init_ui_button(&screen_settings, 195, 100, 35, 35, SV(">"), dummy_cbk);
    _init_ui_button(&screen_settings, 160, 150, 50, 35, SV("..."), dummy_cbk);
    _init_ui_button(&screen_settings, 120, 200, 105, 47, SV("Back to main menu"), cbk_goto_title);

    // ====================================

    _init_ui_button(&screen_draft_spells, 320, 420, 70, 24, SV("Quit match"), cbk_goto_title);
    // _init_ui_label(&screen_draft_spells, );
    // _init_ui_button()

    all_screens[SCREEN_TITLE_ID] = screen_title;
    all_screens[SCREEN_SETTINGS_ID] = screen_settings;
    all_screens[SCREEN_DRAFT_WEAP_ID] = screen_draft_weapon;
    all_screens[SCREEN_DRAFT_SPELLS_ID] = screen_draft_spells;
}

__attribute__((export_name("update"))) void update(double timestamp_ms)
{
    (void)timestamp_ms;

    // perform UI state updates for the current screen.
    struct screen this_scr = all_screens[CURR_SCREEN];
    js_set_cursor_default();
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
        if (hov)
        {
            js_set_cursor_pointer();
        }
    }

    FRAME_CNT++;
}

__attribute__((export_name("draw"))) void draw(void)
{
    _wipe_scr();

    struct screen this_scr = all_screens[CURR_SCREEN];
    for (int lab_i = 0; lab_i < this_scr.n_labels; lab_i++)
    {
        struct ui_label lab = this_scr.labels[lab_i];
        _render_sv(lab.x, lab.y, lab.label);
    }
    for (int but_i = 0; but_i < this_scr.n_buttons; but_i++)
    {
        struct ui_button but = this_scr.buttons[but_i];
        Pixel rectcolor = 0xFFAA3344 | (uint32_t)(0xFF * but.click_state);
        _draw_rect(rectcolor, but.x, but.y, but.x + but.w, but.y + but.h);
        _render_sv(but.x + 5, but.y + (int)(0.5 * but.h) - 4, but.label);
    }
    for (int spr_i = 0; spr_i < this_scr.n_staticsprites; spr_i++)
    {
        struct ui_staticsprite spr = this_scr.staticsprites[spr_i];
        _draw_sprite(spr.sprite, spr.x, spr.y);
    }

    // ============= M O U S E pointer =====
    _draw_rect(0xFFFFFFFF, inputs.mouse_x - 1, inputs.mouse_y - 1, inputs.mouse_x + 1, inputs.mouse_y + 1);

    // DEBUG: coordinates as digits
    _render_int(210, 0, inputs.mouse_x);
    _render_sv(240, 0, SV(","));
    _render_int(250, 0, inputs.mouse_y);
}
