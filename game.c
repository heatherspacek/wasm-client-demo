#include <stdint.h>

// Drawing
extern void js_clear(float r, float g, float b);
extern void js_draw_rect(float x, float y, float w, float h,
                         float r, float g, float b, float a);
extern void js_draw_sprite(float x, float y, float w, float h,
                           float sx, float sy, float sw, float sh);
extern void js_fill_text(const char *text, uint32_t len,
                         float x, float y,
                         float r, float g, float b,
                         float size);
extern void js_log(const char *text, uint32_t len);

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

////////////////////////////////////////////////////////////////////////
// ################################################################## //

static int FRAME_CNT = 0;
static int CURR_SCREEN = 0;

static _Bool DBG_TOGGLE = 0;

#define MAX_BTNS 15
#define MAX_SCREENS 6

#define SCREEN_TITLE 0
#define SCREEN_SETTINGS 1
#define SCREEN_DRAFT_WEAP 2
#define SCREEN_DRAFT_SPELLS 3
#define SCREEN_COMBAT 4

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

struct hoverbox
{
    int x;
    int y;
    int w;
    int h;
    _Bool state;
};

#define N_HOV 10
#define SQ_SIZE 40
struct hoverbox hoverboxes[N_HOV];

// bool point_in_rect(int p_x, int p_y, )
// {
// }

// bool mouse_collide()
// {
//     return point_in_rect()
// }

void cbk1()
{
    js_log("clicked?", 9);
    DBG_TOGGLE = !DBG_TOGGLE;
}

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
    struct screen title_screen;

    _init_ui_button(&title_screen, 20, 205, 105, 35, SV("Start Game"), cbk1);
    _init_ui_button(&title_screen, 20, 275, 105, 35, SV("Settings"), cbk1);
    _init_ui_button(&title_screen, 20, 345, 105, 35, SV("DEBUG"), cbk1);

    all_screens[SCREEN_TITLE] = title_screen;

    for (int i = 0; i < N_HOV; i++)
    {
        hoverboxes[i].x = i * SQ_SIZE;
        hoverboxes[i].y = 480 - SQ_SIZE;
        hoverboxes[i].w = SQ_SIZE;
        hoverboxes[i].h = SQ_SIZE;
        hoverboxes[i].state = 0;
    }
}

__attribute__((export_name("update"))) void update(double timestamp_ms)
{
    (void)timestamp_ms;

    for (int i = 0; i < N_HOV; i++)
    {
        int left = hoverboxes[i].x;
        int right = hoverboxes[i].x + hoverboxes[i].w;
        int top = hoverboxes[i].y;
        int bot = hoverboxes[i].y + hoverboxes[i].h;
        hoverboxes[i].state = inputs.mouse_x > (float)left &&
                              inputs.mouse_y > (float)top &&
                              inputs.mouse_x < (float)right &&
                              inputs.mouse_y < (float)bot;
    }

    // perform UI state updates for the current screen.
    struct screen this_scr = all_screens[CURR_SCREEN];
    // later, loop over all buttons...
    struct ui_button bxx = this_scr.buttons[0];
    int right = bxx.x + bxx.w;
    int bot = bxx.y + bxx.h;
    int hov = inputs.mouse_x > (float)bxx.x &&
              inputs.mouse_y > (float)bxx.y &&
              inputs.mouse_x < (float)right &&
              inputs.mouse_y < (float)bot;
    int click = hov && inputs.mouse_buttons;
    bxx.click_state = hov;
    if (click)
    {
        bxx.cbk();
    }

    FRAME_CNT++;
}

__attribute__((export_name("draw"))) void draw(void)
{
    js_clear(0.08f, 0.08f, 0.12f);

    js_fill_text("wasm client demo", 16,
                 160, 50, 0.7f, 0.8f, 0.6f, 32.0f);

    // mouse pointer
    js_draw_rect(inputs.mouse_x - 1.0f, inputs.mouse_y - 1.0f, 3.0f, 3.0f,
                 1.0f, 1.0f, 1.0f, 0.7f);

    for (int i = 0; i < N_HOV; i++)
    {
        js_draw_rect(hoverboxes[i].x, hoverboxes[i].y, hoverboxes[i].w, hoverboxes[i].h,
                     0.1f, 0.5f + hoverboxes[i].state * 0.4f, 1.0f, 1.0f);
    }

    const char *dbg = "u clicked >_>";
    if (DBG_TOGGLE)
    {
        js_fill_text(
            dbg, 13,
            300, 25,
            0.7f, 1.0f, 0.7f, 16.0f);
    }

    struct screen this_scr = all_screens[CURR_SCREEN];

    for (int i = 0; i < this_scr.n_buttons; i++)
    {
        struct ui_button bxx = this_scr.buttons[i];
        js_draw_rect(bxx.x, bxx.y, bxx.w, bxx.h,
                     0.2f, 0.4f + bxx.click_state * 0.4f, 0.8f, 0.6f);
        js_fill_text(bxx.label.data, bxx.label.count,
                     bxx.x + 10, bxx.y + 22, 1.0f, 1.0f, 1.0f, 12.0f);
    }
}
