#include <stdint.h>

#define SCR_W 640
#define SCR_H 480
#define SCR_N (SCR_H * SCR_W)
#define PX(x, y) ((y) * SCR_W + (x))
#define CLIPX(x) ((x) < 0 ? 0 : ((x) >= SCR_W - 1 ? SCR_W - 1 : (x)))
#define CLIPY(y) ((y) < 0 ? 0 : ((y) >= SCR_H - 1 ? SCR_H - 1 : (y)))

extern void js_log_s(const char *text, uint32_t len);
extern void js_log_f(float value);
extern void js_log_i(int value);

typedef uint32_t Pixel;

extern Pixel scr_buf[SCR_W * SCR_H];
