#include "text.h"
#include "drawing.h"
#include "data/font_myscratch.h"

String_View sv_from_parts(const char *data, int count)
{
    String_View sv;
    sv.count = count;
    sv.data = data;
    return sv;
}

void _render_sv(int x, int y, String_View sv)
{
    int x_offset = 0;
    for (int i = 0; i < sv.count; i++)
    {
        char thischar = sv.data[i];
        if (thischar == ' ')
        {
            x_offset += 5;
            continue;
        }
        for (int g_i = 0; g_i < N_CHARS; g_i++)
        {
            Glyph match_g = all_glyphs[g_i];
            if (match_g.character == thischar)
            {
                _blit_glyph_8wide(match_g.data_8, match_g.rows, x + x_offset, y);
                x_offset += match_g.spacing - 1;
                break;
            }
        }
    }
}