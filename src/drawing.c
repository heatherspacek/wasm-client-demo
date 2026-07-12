#include "types.h"
#include "drawing.h"

Pixel scr_buf[SCR_W * SCR_H] = {0};

void _wipe_scr()
{
    for (int pxi = 0; pxi < SCR_N; pxi++)
    {
        scr_buf[pxi] = 0;
    }
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
