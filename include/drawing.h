void _wipe_scr();
void _write_px(Pixel val, int x, int y);
void _draw_rect(Pixel col, int x1, int y1, int x2, int y2);
void _draw_sprite(const uint32_t *data, int x, int y);
void _blit_glyph_8wide(const uint8_t *bits, int rows, int x, int y);