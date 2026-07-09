#include "text.h"
#include "drawing.h"
#include "private_data/font_lookout_GEN.h"

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
    int y_offset = 0;
    for (int i = 0; i < sv.count; i++)
    {
        char thischar = sv.data[i];
        if (thischar == ' ')
        {
            x_offset += 5;
            continue;
        }
        else if (thischar == '\n')
        {
            y_offset += 14;
            x_offset = 0;
            continue;
        }
        for (int g_i = 0; g_i < N_CHARS; g_i++)
        {
            Glyph match_g = all_glyphs[g_i];
            if (match_g.character == thischar)
            {
                _blit_glyph_8wide(match_g.data_8, match_g.rows, x + x_offset, y + y_offset);
                x_offset += match_g.spacing - 1;
                break;
            }
        }
    }
}

char _digit_to_char(int digit)
{
    return (char)digit + 48;
}

void _render_int(int x, int y, int input_int)
{
    // maxes out at 6 for now
    int x_offset = 0;
    int n_digits = 0;
    int digits[6] = {0};
    for (int i = 6; i > 0; i--)
    {
        digits[i - 1] = input_int % 10;
        input_int /= 10;
        n_digits += 1;
        if (input_int == 0)
            break;
    }

    for (int i = 0; i < n_digits; i++)
    {
        char thischar = _digit_to_char(digits[(6 - n_digits) + i]);

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
