
typedef struct
{
    int count;
    const char *data;
} String_View;
#define SV(cstr_lit) sv_from_parts(cstr_lit, sizeof(cstr_lit) - 1)

String_View sv_from_parts(const char *data, int count);
void _render_sv(int x, int y, String_View sv);