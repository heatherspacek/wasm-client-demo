#include "data/spritesheet.h"

#include "types.h"
#include "drawing.h"
#include "text.h"
#include "spelldata.h"

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

static int PREV_CLICK_STATE = 0;
static int CURR_GRABBED_I = -1;

static _Bool DBG_TOGGLE = 0;

#define MAX_BTNS 15
#define MAX_LABELS 15
#define MAX_STATICSPRITES 15
#define MAX_SPELLS 15
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
    int hover_state;
    int click_state; // 1=primed
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
    struct spellobject spellobjs[MAX_SPELLS];
    int n_spellobjs;
};
static struct screen all_screens[MAX_SCREENS];

void cbk_goto_settings()
{
    DBG_TOGGLE = 0;
    CURR_SCREEN = SCREEN_SETTINGS_ID;
}

void cbk_goto_title()
{
    CURR_SCREEN = SCREEN_TITLE_ID;
}

void cbk_goto_draft()
{
    DBG_TOGGLE = 0;
    CURR_SCREEN = SCREEN_DRAFT_SPELLS_ID;
}
void cbk_goto_game()
{
    DBG_TOGGLE = 0;
    CURR_SCREEN = SCREEN_COMBAT_ID;
}

void dummy_cbk() {}

void _init_ui_button(struct screen *target_scr,
                     int x, int y, int w, int h, String_View label, void *cbk)
{
    struct ui_button newbtn = {x, y, w, h, 0, 0, label, cbk};
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

void _add_spellobj_to_screen(struct screen *target_scr,
                             int x, int y, struct spellinfo info)
{
    struct spellobject newobj =
        {
            .x = x,
            .y = y,
            .grab_state = 0,
            .hover_state = 0,
            .spelldata = info};
    target_scr->spellobjs[target_scr->n_spellobjs] = newobj;
    target_scr->n_spellobjs++;
}

void cbk_spawn_spell()
{
    if (all_screens[CURR_SCREEN].n_spellobjs + 1 > MAX_SPELLS)
    {
        js_log_s("Can't spawn another spellobj!!", 30);
        return;
    }
    _add_spellobj_to_screen(&all_screens[CURR_SCREEN], 150 + 32 * all_screens[CURR_SCREEN].n_spellobjs, 150, spell01);
}

void cbk_dbg_toggle()
{
    DBG_TOGGLE = !DBG_TOGGLE;
}

int _mouse_in_rect(int x, int y, int w, int h)
{
    return inputs.mouse_x >= x &&
           inputs.mouse_y >= y &&
           inputs.mouse_x <= (x + w) &&
           inputs.mouse_y <= (y + h);
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

    _init_ui_button(&screen_title, 20, 205, 85, 24, SV("Start Game"), cbk_goto_draft);
    _init_ui_button(&screen_title, 20, 245, 85, 24, SV("Settings"), cbk_goto_settings);
    _init_ui_button(&screen_title, 20, 285, 85, 24, SV("DEBUG"), cbk_dbg_toggle);

    _init_ui_button(&screen_settings, 160, 100, 35, 35, SV("<"), dummy_cbk);
    _init_ui_button(&screen_settings, 195, 100, 35, 35, SV(">"), dummy_cbk);
    _init_ui_button(&screen_settings, 160, 150, 50, 35, SV("..."), dummy_cbk);
    _init_ui_button(&screen_settings, 120, 200, 105, 47, SV("Back to main menu"), cbk_goto_title);

    // ====================================

    _init_ui_button(&screen_draft_spells, 24, 440, 70, 24, SV("Quit match"), cbk_goto_title);
    _init_ui_button(&screen_draft_spells, 104, 440, 115, 24, SV("DEBUG-- ready up!"), cbk_goto_game);
    _init_ui_button(&screen_draft_spells, 95, 50, 450, 100, SV(""), dummy_cbk);
    _init_ui_button(&screen_draft_spells, 45, 175, 550, 100, SV(""), dummy_cbk);
    _init_ui_button(&screen_draft_spells, 95, SCR_H - 175, 450, 110, SV(""), dummy_cbk);
    _init_ui_label(&screen_draft_spells, 115, 55, SV("Opponent deck"));
    _init_ui_label(&screen_draft_spells, 55, 180, SV("AVAILABLE CARDS"));
    _init_ui_label(&screen_draft_spells, 115, 392, SV("Your deck"));

    _init_ui_button(&screen_draft_spells, 20, 90, 115, 24, SV("[DBG] spawn spell"), cbk_spawn_spell);

    // ====================================

    all_screens[SCREEN_TITLE_ID] = screen_title;
    all_screens[SCREEN_SETTINGS_ID] = screen_settings;
    all_screens[SCREEN_DRAFT_WEAP_ID] = screen_draft_weapon;
    all_screens[SCREEN_DRAFT_SPELLS_ID] = screen_draft_spells;
}

__attribute__((export_name("update"))) void update(double timestamp_ms)
{
    (void)timestamp_ms;

    // perform UI state updates for the current screen.
    struct screen *this_scr = &all_screens[CURR_SCREEN];
    int which_cursor = 0; // default

    // ============ hover handling.
    for (int i = 0; i < this_scr->n_buttons; i++)
    {
        struct ui_button *tb = &this_scr->buttons[i];
        tb->hover_state = _mouse_in_rect(tb->x, tb->y, tb->w, tb->h);
    }
    for (int i = 0; i < this_scr->n_spellobjs; i++)
    {
        struct spellobject *tso = &this_scr->spellobjs[i];
        tso->hover_state = _mouse_in_rect(tso->x, tso->y, 32, 32);
    }

    // ============ click handling.

    if (inputs.mouse_buttons && !PREV_CLICK_STATE)
    {
        // mouse DOWN event.
        // - grab grabby
        // - prime button
        int click_absorbed = 0;
        for (int i = 0; i < this_scr->n_spellobjs; i++)
        {
            struct spellobject ts = this_scr->spellobjs[i];
            if (_mouse_in_rect(ts.x, ts.y, 32, 32))
            {
                ts.grab_state = 1;
                CURR_GRABBED_I = i;
                click_absorbed = 1;
                break;
            }
        }
        if (!click_absorbed)
        {
            for (int i = 0; i < this_scr->n_buttons; i++)
            {
                struct ui_button tb = this_scr->buttons[i];
                if (_mouse_in_rect(tb.x, tb.y, tb.w, tb.h))
                {
                    tb.cbk();
                    break;
                }
            }
        }
    }
    else if (!inputs.mouse_buttons && PREV_CLICK_STATE)
    {
        // mouse UP event.
        // - release grabby
        // - click button if within rect + it was primed
        if (CURR_GRABBED_I >= 0)
        {
            this_scr->spellobjs[CURR_GRABBED_I].grab_state = 0;
            CURR_GRABBED_I = -1;
        }
    }

    // attach da grabby!
    if (CURR_GRABBED_I >= 0)
    {
        this_scr->spellobjs[CURR_GRABBED_I].x = inputs.mouse_x - 16;
        this_scr->spellobjs[CURR_GRABBED_I].y = inputs.mouse_y - 16;
        js_set_cursor_grabbing();
    }

    FRAME_CNT++;
    PREV_CLICK_STATE = (int)inputs.mouse_buttons;
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
        Pixel rectcolor = 0xFFAA3320 | (uint32_t)(0x40 * but.hover_state);
        _draw_rect(rectcolor, but.x, but.y, but.x + but.w, but.y + but.h);
        _render_sv(but.x + 5, but.y + (int)(0.5 * but.h) - 4, but.label);
    }
    for (int spr_i = 0; spr_i < this_scr.n_staticsprites; spr_i++)
    {
        _draw_sprite(this_scr.staticsprites[spr_i].sprite, this_scr.staticsprites[spr_i].x, this_scr.staticsprites[spr_i].y);
    }
    for (int spe_i = 0; spe_i < this_scr.n_spellobjs; spe_i++)
    {
        _draw_sprite(this_scr.spellobjs[spe_i].spelldata.sprite, this_scr.spellobjs[spe_i].x, this_scr.spellobjs[spe_i].y);
        if (this_scr.spellobjs[spe_i].hover_state)
        {
            // draw tooltip
            int cx = this_scr.spellobjs[spe_i].x + 16;
            int cy = this_scr.spellobjs[spe_i].y + 16;
            _draw_rect(0xFFDDAAFF, cx - 55, cy + 22, cx + 55, cy + 102);
            _fill_rect(0xFF220022, cx - 54, cy + 23, cx + 54, cy + 101);
            _render_sv(cx - 50, cy + 27, this_scr.spellobjs[spe_i].spelldata.name);
            _render_int(cx + 40, cy + 27, this_scr.spellobjs[spe_i].spelldata.cost);
            _render_sv(cx - 50, cy + 47, this_scr.spellobjs[spe_i].spelldata.description);
        }
    }

    if (DBG_TOGGLE)
    {
        int DEBUG_N_SPRITES = 16;
        for (int i = 0; i < DEBUG_N_SPRITES; i++)
        {
            int row = 64 + (i % 10) * 32;
            int col = 150 + (i / 10) * 64;

            _draw_sprite(all_sprites[i], col, row);
            _render_int(col - 16, row, i);
        }
    }

    _render_int(210, 0, inputs.mouse_x);
    _render_sv(240, 0, SV(","));
    _render_int(250, 0, inputs.mouse_y);

    // ============= M O U S E pointer =====
    _draw_rect(0xFFFFFFFF, inputs.mouse_x - 1, inputs.mouse_y - 1, inputs.mouse_x + 1, inputs.mouse_y + 1);
}
