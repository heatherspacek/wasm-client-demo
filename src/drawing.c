#include "types.h"
#include "drawing.h"

Pixel scr_buf[SCR_W * SCR_H] = {0};

void _wipe_scr()
{
    /*for (int pxi = 0; pxi < SCR_N; pxi++)
    {
        scr_buf[pxi] = 0;
}*/

    __builtin_memset(scr_buf, 0, sizeof(scr_buf));
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
    x1 = CLIPX(x1);
    x2 = CLIPX(x2);
    y1 = CLIPY(y1);
    y2 = CLIPY(y2);
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
void _fill_rect(Pixel col, int x1, int y1, int x2, int y2)
{
    x1 = CLIPX(x1);
    x2 = CLIPX(x2);
    y1 = CLIPY(y1);
    y2 = CLIPY(y2);
    for (int xx = x1; xx <= x2; xx++)
    {
        for (int yy = y1; yy <= y2; yy++)
        {
            _write_px(col, xx, yy);
        }
    }
}

void setPixel(int x, int y)
{
    _write_px(0xFFFFFFFF, x, y);
}
void _draw_circle(int xm, int ym, int r)
{
    // Bresenham circle, with thanks to Alois Zingl:
    // http://members.chello.at/~easyfilter/bresenham.html

    int x = -r, y = 0, err = 2 - 2 * r; /* II. Quadrant */
    do
    {
        setPixel(xm - x, ym + y); /*   I. Quadrant */
        setPixel(xm - y, ym - x); /*  II. Quadrant */
        setPixel(xm + x, ym - y); /* III. Quadrant */
        setPixel(xm + y, ym + x); /*  IV. Quadrant */
        r = err;
        if (r <= y)
            err += ++y * 2 + 1; /* e_xy+e_y < 0 */
        if (r > x || err > y)
            err += ++x * 2 + 1; /* e_xy+e_x > 0 or no 2nd y-step */
    } while (x < 0);
}
void _draw_sparkle(int xm, int ym, int r)
{
    // inside out circle!
    int x = -r, y = 0, err = 2 - 2 * r; /* II. Quadrant */
    int offs = r;
    do
    {
        setPixel(xm - y + offs, ym + x + offs);
        setPixel(xm - y + offs, ym - x - offs);
        setPixel(xm + y - offs, ym - x - offs);
        setPixel(xm + y - offs, ym + x + offs);
        r = err;
        if (r <= y)
            err += ++y * 2 + 1; /* e_xy+e_y < 0 */
        if (r > x || err > y)
            err += ++x * 2 + 1; /* e_xy+e_x > 0 or no 2nd y-step */
    } while (x < 0);
}
void _draw_sprite(const uint32_t *data, int x, int y)
{
    for (int xx = 0; xx < 32; xx++)
    {
        for (int yy = 0; yy < 32; yy++)
        {
            int idx = (yy * 32) + xx;
            if (data[idx] == 0)
                continue;
            _write_px(data[idx], CLIPX(x + xx), CLIPY(y + yy));
        }
    }
}

uint8_t _clamped_add_u8(uint8_t base, uint8_t amt)
{
    if ((0xFF - base) <= amt)
    {
        return 0xFF;
    }
    return base + amt;
}

Pixel _lighten(Pixel color, uint8_t amt)
{
    uint8_t ch1 = _clamped_add_u8(color, amt);
    uint8_t ch2 = _clamped_add_u8(color >> 8, amt);
    uint8_t ch3 = _clamped_add_u8(color >> 16, amt);

    return 0xFF000000 + (ch1) + ((uint32_t)ch2 << 8)  + ((uint32_t)ch3 << 16);
}

uint8_t _clamped_subtract_u8(uint8_t base, uint8_t amt)
{
    if (base <= amt)
    {
        return 0x00;
    }
    return base - amt;
}

Pixel _darken(Pixel color, uint8_t amt)
{
    uint8_t ch1 = _clamped_subtract_u8(color, amt);
    uint8_t ch2 = _clamped_subtract_u8(color >> 8, amt);
    uint8_t ch3 = _clamped_subtract_u8(color >> 16, amt);

    return 0xFF000000 + (ch1) + ((uint32_t)ch2 << 8)  + ((uint32_t)ch3 << 16);
}

void _draw_vbar(Pixel base_color, int x, int y, int w, int h, float progress)
{
    Pixel highlight_color = _lighten(base_color, 0x30);
    Pixel lowlight_color = _darken(base_color, 0x30);
    Pixel outline_color = 0xFFFFFFFF;

    _draw_rect(outline_color, x, y, x + w, y + h);
    _draw_rect(base_color, x+1, y+1, x + w -1, y + h -1);
    _fill_rect(0xFF888888, x + 2, y + 2, x + w -2, y + h -2);

    int progress_px = progress * (h-2);
    int stop1 = 0.15 * w;
    int stop2 = 0.6 * w;
    _fill_rect(lowlight_color, x + 3, y + (h - progress_px), x + w -3, y + h -3);
    _fill_rect(base_color, x + 2, y + (h-progress_px) + 1, x + stop2 + 2, y + h - 2);
    _fill_rect(highlight_color, x + 2, y + (h-progress_px) + 1, x + stop1 + 2, y + h -2);
}
