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

// Placed at a fixed address the JS side is told about via input_state_ptr().
static InputState g_input = {0};

// Exported so the shim can discover the address once at startup.
__attribute__((export_name("input_state_ptr")))
uint32_t
input_state_ptr(void)
{
    return (uint32_t)(uintptr_t)&g_input;
}

static int frame = 0;
static int hover = 0;

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

__attribute__((export_name("init"))) void init()
{
    for (int i = 0; i < N_HOV; i++)
    {
        hoverboxes[i].x = i * SQ_SIZE;
        hoverboxes[i].y = 150;
        hoverboxes[i].w = SQ_SIZE;
        hoverboxes[i].h = SQ_SIZE;
        hoverboxes[i].state = 0;
    }
}

__attribute__((export_name("update"))) void update(double timestamp_ms)
{
    (void)timestamp_ms;

    hover = g_input.mouse_x > 20.0f &&
            g_input.mouse_y > 20.0f &&
            g_input.mouse_x < 220.0f &&
            g_input.mouse_y < 70.0f;

    for (int i = 0; i < N_HOV; i++)
    {
        int left = hoverboxes[i].x;
        int right = hoverboxes[i].x + hoverboxes[i].w;
        int top = hoverboxes[i].y;
        int bot = hoverboxes[i].y + hoverboxes[i].h;
        hoverboxes[i].state = g_input.mouse_x > (float)left &&
                              g_input.mouse_y > (float)top &&
                              g_input.mouse_x < (float)right &&
                              g_input.mouse_y < (float)bot;
    }

    frame++;
}

__attribute__((export_name("draw"))) void draw(void)
{
    js_clear(0.08f, 0.08f, 0.12f);

    // mouse pointer
    js_draw_rect(g_input.mouse_x - 1.0f, g_input.mouse_y - 1.0f, 3.0f, 3.0f,
                 1.0f, 1.0f, 1.0f, 0.7f);

    for (int i = 0; i < N_HOV; i++)
    {
        js_draw_rect(hoverboxes[i].x, hoverboxes[i].y, hoverboxes[i].w, hoverboxes[i].h,
                     0.1f, 0.5f + hoverboxes[i].state * 0.4f, 1.0f, 1.0f);
    }

    js_draw_rect(20.0f, 20.0f, 200.0f, 50.0f,
                 0.2f, 0.4f + hover * 0.4f, 0.8f, 0.6f);
    const char *label = "welcome to da demo :]";
    js_fill_text(label, 21,
                 20.0f, 60.0f,
                 0.9f, 0.9f, 0.9f,
                 16.0f);
}
