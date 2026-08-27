#include "types.h"
#define SCR_W 480
#define SCR_H 320
#define SCR_N (SCR_H * SCR_W)

#define BTN_H_SMALL 24
#define SCR_W_1_2 240
#define SCR_W_1_3 160
#define SCR_W_2_3 320
#define SCR_W_1_4 120
#define SCR_W_2_4 SCR_W_1_2
#define SCR_W_3_4 360
#define SCR_W_1_5 96
#define SCR_W_2_5 192
#define SCR_W_3_5 288
#define SCR_W_4_5 384
#define SCR_W_1_6 80
#define SCR_W_2_6 SCR_W_1_3
#define SCR_W_3_6 SCR_W_1_2
#define SCR_W_4_6 SCR_W_2_3
#define SCR_W_5_6 400
#define SCR_W_1_8 60
#define SCR_W_2_8 SCR_W_1_4
#define SCR_W_3_8 180
#define SCR_W_4_8 SCR_W_1_2
#define SCR_W_5_8 300
#define SCR_W_6_8 SCR_W_3_4
#define SCR_W_7_8 420
#define SCR_W_1_10 48
#define SCR_W_2_10 SCR_W_1_5
#define SCR_W_3_10 144
#define SCR_W_4_10 SCR_W_2_5
#define SCR_W_5_10 SCR_W_1_2
#define SCR_W_6_10 SCR_W_3_5
#define SCR_W_7_10 336
#define SCR_W_8_10 SCR_W_4_5
#define SCR_W_9_10 432

#define SCR_H_1_2 160
#define SCR_H_1_4 80
#define SCR_H_3_4 240
#define SCR_H_1_5 64
#define SCR_H_2_5 128
#define SCR_H_3_5 192
#define SCR_H_4_5 256
#define SCR_H_1_8 40
#define SCR_H_2_8 SCR_H_1_4
#define SCR_H_3_8 120
#define SCR_H_4_8 SCR_H_1_2
#define SCR_H_5_8 200
#define SCR_H_6_8 SCR_H_3_4
#define SCR_H_7_8 280
#define SCR_H_1_10 32
#define SCR_H_2_10 SCR_H_1_5
#define SCR_H_3_10 96
#define SCR_H_4_10 SCR_H_2_5
#define SCR_H_5_10 SCR_H_1_2
#define SCR_H_6_10 SCR_H_3_5
#define SCR_H_7_10 224
#define SCR_H_8_10 SCR_H_4_5
#define SCR_H_9_10 288


#define PX(x, y) ((y) * SCR_W + (x))
#define CLIPX(x) ((x) < 0 ? 0 : ((x) >= SCR_W - 1 ? SCR_W - 1 : (x)))
#define CLIPY(y) ((y) < 0 ? 0 : ((y) >= SCR_H - 1 ? SCR_H - 1 : (y)))

typedef uint32_t Pixel;

extern Pixel scr_buf[SCR_W * SCR_H];

void _wipe_scr();
void _write_px(Pixel val, int x, int y);
void _draw_rect(Pixel col, int x1, int y1, int x2, int y2);
void _fill_rect(Pixel col, int x1, int y1, int x2, int y2);
void _draw_circle(int xm, int ym, int r);
void _draw_sparkle(int xm, int ym, int r);
void _draw_sprite(const uint32_t *data, int x, int y);
void _blit_glyph_8wide(const uint8_t *bits, int rows, int x, int y);

void _draw_vbar(Pixel base_color, int x, int y, int w, int h, float progress);
