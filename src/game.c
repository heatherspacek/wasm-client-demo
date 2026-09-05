#include "data/spritesheet.h"

#include "types.h"
#include "drawing.h"
#include "text.h"
#include "spelldata.h"
#include "gamestate.h"
#include "timers.h"
#include "network.h"

#define VERSION SV("v0.1.1")

extern void js_log_s(const char *text, uint32_t len);
extern void js_log_f(float value);
extern void js_log_i(int value);
extern void js_set_cursor_default();
extern void js_set_cursor_pointer();
extern void js_set_cursor_grab();
extern void js_set_cursor_grabbing();
extern void js_play_sfx_buffer();


typedef struct
{
    float mouse_x;
    float mouse_y;
    float mouse_buttons; // bitmask: bit 0 = left, bit 1 = middle, bit 2 = right
} InputState;

static InputState inputs = {0};

static Outbox outbox = {0};
static Inbox inbox = {0};

float sfx_buf[512] = {0};

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

__attribute__((export_name("sfx_buffer_ptr")))
uint32_t
sfx_buffer_ptr(void) { return (uint32_t)(uintptr_t)sfx_buf; }


static int FRAME_CNT = 0;
static int CURR_SCREEN = 0;

static int PREV_CLICK_STATE = 0;
static int CURR_GRABBED_I = -1;
static int CURR_HOVERED_BUTTON_I = -1;
static int CURR_HOVERED_SPELL_I = -1;

static _Bool DBG_TOGGLE = 0;

static double last_timestamp = 0.0;
Timer Timer1 = {0};
Timer Timer2 = {0};
Timer HeartBeatTimer = {0};

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

void dummy_cbk() {}
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
    set_and_start_timer(30, dummy_cbk, &Timer1);
}
void cbk_goto_game()
{
    DBG_TOGGLE = 0;
    CURR_SCREEN = SCREEN_COMBAT_ID;
}

void _init_ui_button(struct screen *target_scr,
                     int x, int y, int w, int h, String_View label, void *cbk)
{
    struct ui_button newbtn = {x, y, w, h, 0, 0, label, cbk};
    target_scr->buttons[target_scr->n_buttons] = newbtn;
    target_scr->n_buttons++;
}

int _init_ui_label(struct screen *target_scr,
                    int x, int y, String_View lab)
{
    struct ui_label newlabel = {x, y, lab};
    int label_i = target_scr->n_labels;
    target_scr->labels[label_i] = newlabel;
    target_scr->n_labels++;
    return label_i;
}

// likely not the best API for this. re-evaluate someday.
void _edit_ui_label(struct screen *target_scr, int label_i, String_View lab)
{
    struct ui_label newlabel = {target_scr->labels[label_i].x, target_scr->labels[label_i].y, lab};
    target_scr->labels[label_i] = newlabel;
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
    _add_spellobj_to_screen(&all_screens[CURR_SCREEN], 150 + 32 * all_screens[CURR_SCREEN].n_spellobjs, 150, spell00);
}

void cbk_hb() {
    js_log_s("sending request...", 18);
    _make_request(&outbox);
    set_and_start_timer(2, cbk_hb, &HeartBeatTimer);
}

void cbk_dbg_toggle()
{
    DBG_TOGGLE = !DBG_TOGGLE;
    // TODO: populate sound effect buffer
    for (int i=0; i<512; i += 8) {
        sfx_buf[i] = 0.0;
        sfx_buf[i+1] = 0.707;
        sfx_buf[i+2] = 1.0;
        sfx_buf[i+3] = 0.707;
        sfx_buf[i+4] = 0.0;
        sfx_buf[i+5] = -0.707;
        sfx_buf[i+6] = -1.0;
        sfx_buf[i+7] = -0.707;
    }
    js_play_sfx_buffer();
}

int _mouse_in_rect(int x, int y, int w, int h)
{
    return inputs.mouse_x >= x &&
           inputs.mouse_y >= y &&
           inputs.mouse_x <= (x + w) &&
           inputs.mouse_y <= (y + h);
}

GamePhase GP;

// reeeeeeally doubting this pattern.
int server_status_label_id;

__attribute__((export_name("init"))) void init()
{
    // TODO: loading bar, for when init gets huge!
    struct screen screen_title;
    struct screen screen_settings;
    struct screen screen_draft_weapon;
    struct screen screen_draft_spells;

    GP = INACTIVE;

    _init_ui_label(&screen_title, SCR_W_1_2 - 98, SCR_H_1_10, SV("HEATHER'S UNNAMED WIZARD GAME~"));

    _init_ui_label(&screen_title, 8, SCR_H_9_10 + 12, VERSION);

    server_status_label_id = _init_ui_label(&screen_title, SCR_W_4_10, SCR_H_9_10 + 12, SV("Connection to server: idk lol :D"));

    _init_ui_button(&screen_title, SCR_W_1_2 - 60, SCR_H_4_10, 120, BTN_H_SMALL, SV("Start Game"), cbk_goto_draft);
    _init_ui_button(&screen_title, SCR_W_1_2 - 60, SCR_H_5_10, 120, BTN_H_SMALL, SV("Settings"), cbk_goto_settings);
    _init_ui_button(&screen_title, SCR_W_1_2 - 60, SCR_H_6_10, 120, BTN_H_SMALL, SV("DEBUG"), cbk_dbg_toggle);

    _init_ui_button(&screen_settings, 160, 100, 35, 35, SV("<"), dummy_cbk);
    _init_ui_button(&screen_settings, 195, 100, 35, 35, SV(">"), dummy_cbk);
    _init_ui_button(&screen_settings, 160, 150, 50, 35, SV("..."), dummy_cbk);
    _init_ui_button(&screen_settings, 120, 200, 105, 47, SV("Back to main menu"), cbk_goto_title);

    // ====================================

    _init_ui_button(&screen_draft_spells, 24, 370, 70, BTN_H_SMALL, SV("Quit match"), cbk_goto_title);
    _init_ui_button(&screen_draft_spells, 104, 370, 115, BTN_H_SMALL, SV("DEBUG-- ready up!"), cbk_goto_game);
    _init_ui_button(&screen_draft_spells, 95, 50, 450, 100, SV(""), dummy_cbk);
    _init_ui_button(&screen_draft_spells, 45, 175, 550, 100, SV(""), dummy_cbk);
    _init_ui_button(&screen_draft_spells, 95, SCR_H - 175, 450, 110, SV(""), dummy_cbk);
    _init_ui_label(&screen_draft_spells, 115, 55, SV("Opponent deck"));
    _init_ui_label(&screen_draft_spells, 55, 180, SV("AVAILABLE CARDS"));
    _init_ui_label(&screen_draft_spells, 115, 352, SV("Your deck"));

    _init_ui_button(&screen_draft_spells, 20, 90, 115, BTN_H_SMALL, SV("[DBG] spawn spell"), cbk_spawn_spell);

    // ====================================

    all_screens[SCREEN_TITLE_ID] = screen_title;
    all_screens[SCREEN_SETTINGS_ID] = screen_settings;
    all_screens[SCREEN_DRAFT_WEAP_ID] = screen_draft_weapon;
    all_screens[SCREEN_DRAFT_SPELLS_ID] = screen_draft_spells;

    // ====================================

    set_and_start_timer(2, cbk_hb, &HeartBeatTimer);
}

__attribute__((export_name("update"))) void update(double timestamp_ms)
{
    int time_elapsed;
    if (last_timestamp == 0) {
        time_elapsed = 0;
    }
    else {
        time_elapsed = timestamp_ms - last_timestamp;

    }
    last_timestamp = timestamp_ms;

    _tick_timer(time_elapsed, &Timer1);
    _tick_timer(time_elapsed, &Timer2);
    _tick_timer(time_elapsed, &HeartBeatTimer);


    if (_poll_inbox(&inbox)) {
        uint8_t result = _dummy_receive(&inbox);
        _edit_ui_label(&all_screens[SCREEN_TITLE_ID], server_status_label_id, SV("CONNECTED (???ms)"));
        js_log_i(result);
    }


    // =========== // GAME LOGIC!
    // - check a state enum with basic rules
    // - go to a different gamestate_update fcn depending on the enum...
    //   - for example, during draft, decrement timer, check if player has made selection, talk to the server.
    //   - during combat, increment resources, check for health-depleted, talk to the server.
    void (*gfunc)() = determine_gamefunc(GP);
    gfunc();

    // perform UI state updates for the current screen.
    struct screen *this_scr = &all_screens[CURR_SCREEN];
    CURR_HOVERED_BUTTON_I = -1;
    CURR_HOVERED_SPELL_I = -1;

    // ============ hover handling.
    for (int i = 0; i < this_scr->n_buttons; i++)
    {
        struct ui_button *tb = &this_scr->buttons[i];
        tb->hover_state = _mouse_in_rect(tb->x, tb->y, tb->w, tb->h);
        if (tb->hover_state)
        {
            CURR_HOVERED_BUTTON_I = i;
            break;
        }
    }
    for (int i = 0; i < this_scr->n_spellobjs; i++)
    {
        struct spellobject *tso = &this_scr->spellobjs[i];
        tso->hover_state = _mouse_in_rect(tso->x, tso->y, 32, 32);
        if (tso->hover_state)
        {
            CURR_HOVERED_SPELL_I = i;
        }
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
    }

    // decide cursor...
    if (CURR_HOVERED_BUTTON_I >= 0)
    {
        js_set_cursor_pointer();
    }
    else if (CURR_HOVERED_SPELL_I >= 0)
    {
        js_set_cursor_grab();
    }
    else if (CURR_GRABBED_I >= 0)
    {
        js_set_cursor_grabbing();
    }
    else
    {
        js_set_cursor_default();
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
            _fill_rect(0xFF220022, cx - 54, cy + 23, cx + 54, cy + 101);
            _draw_rect(0xFFDDAAFF, cx - 55, cy + 22, cx + 55, cy + 42);
            _draw_rect(0xFFDDAAFF, cx - 55, cy + 43, cx + 55, cy + 102);
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

        _draw_vbar(0xFFAA0000, 250, 250, 16, 65, 0.4);
        _draw_vbar(0xFF00AA00, 290, 250, 24, 65, 0.8);
        _draw_vbar(0xFF0000AA, 330, 250, 32, 65, 0.95);
    }

    draw_timer(0, 0, &Timer1);

    _render_int(210, 0, inputs.mouse_x);
    _render_sv(240, 0, SV(","));
    _render_int(250, 0, inputs.mouse_y);

    // ============= M O U S E pointer =====
    _draw_rect(0xFFFFFFFF, inputs.mouse_x - 1, inputs.mouse_y - 1, inputs.mouse_x + 1, inputs.mouse_y + 1);
}
