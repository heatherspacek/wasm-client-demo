#pragma once
typedef struct
{
    int count;
    const char *data;
} String_View;
#define SV(cstr_lit) sv_from_parts(cstr_lit, sizeof(cstr_lit) - 1)
// used for designated initializers... or something.
#define SVC(cstr_lit) \
    (String_View) { sizeof(cstr_lit) - 1, cstr_lit }

String_View sv_from_parts(const char *data, int count);
void _render_sv(int x, int y, String_View sv);
char _digit_to_char(int digit);
void _render_int(int x, int y, int input_int);
