#include <stdint.h>
#define SCR_W 640
#define SCR_H 480
#define SCR_N (SCR_H * SCR_W)
#define PX(x, y) ((y) * SCR_W + (x))
#define CLIPX(x) ((x) < 0 ? 0 : ((x) >= SCR_W - 1 ? SCR_W - 1 : (x)))
#define CLIPY(y) ((y) < 0 ? 0 : ((y) >= SCR_H - 1 ? SCR_H - 1 : (y)))

typedef uint32_t Pixel;

extern Pixel scr_buf[SCR_W * SCR_H];

void _wipe_scr();
void _write_px(Pixel val, int x, int y);
void _draw_rect(Pixel col, int x1, int y1, int x2, int y2);
void _draw_sprite(const uint32_t *data, int x, int y);
void _blit_glyph_8wide(const uint8_t *bits, int rows, int x, int y);