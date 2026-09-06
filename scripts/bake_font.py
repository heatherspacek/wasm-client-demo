from PIL import Image

fontpath_in = "res/private/lookout_7.png"
headerpath_out = "include/private_data/font_lookout.h"
spritesheet_grid = (16, 16)
char_limits = (1, 0, 9, 16)
charmap = [
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ",
    """abcdefghijklmnopqrstuvwxyz""",
    """0123456789.,;:?!"'+-=*%_()""",
    r"[]{}~#&@©®™°^`|/\<>…€$£¢¿¡",
]
space_spacing = 5
spacings = {
    11: "MNW",
    10: "BDEFHKLOPQRUVXY@$ÐÈÉÊËĘŁÒÓÔÖÕŐØẞÙÚÛÜŰÝŸВДЕЁЖКЛНОПРУХЧЩ",
    9: "ASTmw©®™€£ÀÁÄÃÅĄŚŞÞ",
    8: "CGJZ%&ÂÇĆĞŹŻæœßЄСЭЮЯдлпчщъ",
    7: "Idekprstvxyz0123456789?_~#^…¿“”„ÌÍÎÏİðèéêëęőøśşþűýÿźż",
    6: "abcfghnoqu+*°`¢«»àáâäãåąçćğñńòóôöõùúûüабвєзийносьэя",
    5: """ij\"-=/\\<>‹›ìíîïıłії""",
    4: "l.,;:!()[]{}|‘’‚•",
    3: "'¡·",
}
# spacings_scratch = {
#     11: "",
#     10: "MDWFKLOPQRUVXY@$ÐÈÉÊËĘŁÒÓÔÖÕŐØẞÙÚÛÜŰÝŸВДЕЁЖКЛНОПРУХЧЩ",
#     9: "ANBETHmw©®™€£ÀÁÄÃÅĄŚŞÞ",
#     8: "CSGJZzv%&ÂÇĆĞŹŻæœßЄСЭЮЯдлпчщъ",
#     7: "aIbgdeopuqsntxy0123456789?_~#^…¿“”„ÌÍÎÏİðèéêëęőøśşþűýÿźż",
#     6: "cfihkr+*°`¢«»àáâäãåąçćğñńòóôöõùúûüабвєзийносьэя",
#     5: """jl\"-=/\\<>‹›ìíîïıłії""",
#     4: ".,;:!()[]{}|‘’‚•",
#     3: "'¡·",
# }
# spacings = spacings_scratch


def c_escaped(str_in: str):
    if str_in == "'":
        return "\\'"
    if str_in == "\\":
        return "\\\\"
    return str_in


def DEF_8(index, n_rows, data):
    return f"""\
static const uint8_t glyph_{index}_8[{n_rows}] = {{
    {data}
}};
"""


def DEF_16(index, n_rows, data):
    return f"""\
static const uint16_t glyph_{index}_16[{n_rows}] = {{
    {data}
}};
"""


def asciify(str_in: str):
    return "".join([ch if ch.isascii() else " " for ch in str_in])


def get_spacing(test_char: str) -> int:
    if test_char == " ":
        return space_spacing
    for k, v in spacings.items():
        if test_char in v:
            return k
    raise ValueError("get_spacing: character not found")


def rect_to_pixelslist(glyph: Image) -> list[list[int]]:
    data = []
    for row in range((char_limits[3] - char_limits[1])):
        rxx = []
        for col in range((char_limits[2] - char_limits[0])):
            rxx.append(int(glyph.getpixel([col, row]) == (255, 255, 255, 255)))
        data.append(rxx)
    return data


def pixelslist_to_literal(pixlist: list[list[int]]) -> str:
    "This will be illegible to me in five minutes but I'm on a hot streak."
    return ",\n".join(["0b" + "".join([str(c) for c in row]) for row in pixlist])


def pixelslist_to_lit16(pixlist: list[list[int]]) -> str:
    list1 = ["0b" + "".join([str(c) + str(c) for c in row]) for row in pixlist]
    list2 = []
    for elem in list1:
        list2.extend([elem, elem])
    return ",\n".join(list2)


def write_dataheader(registry: dict[str, list[list[int]]]):
    PREAMBLE = f"""\
#include "types.h"
#define N_CHARS {len(registry.keys())}
typedef struct
{{
    const char character;
    const uint8_t spacing;
    const uint8_t rows;
    const uint8_t *data_8;
    const uint16_t *data_16;
}} Glyph;
"""

    with open(headerpath_out, "w") as fp:
        fp.write(PREAMBLE)
        for i, (k, v) in enumerate(registry.items()):
            fp.write(DEF_8(index=i, n_rows=len(v), data=pixelslist_to_literal(v)))
            fp.write(DEF_16(index=i, n_rows=2 * len(v), data=pixelslist_to_lit16(v)))
        # Then, write the mapping table:
        fp.write("static const Glyph all_glyphs[N_CHARS] = {\n")
        for i, (k, v) in enumerate(registry.items()):
            fp.write(
                f"{{'{c_escaped(k)}', {get_spacing(k)}, {len(v)}, glyph_{i}_8, glyph_{i}_16}},\n"
            )
        fp.write("};\n")


if __name__ == "__main__":
    fontimg = Image.open(fontpath_in)
    xg, yg = spritesheet_grid  # 16, 16

    registry = {}

    for row_i, row in enumerate(charmap):
        for letter_i, letter in enumerate(asciify(row)):
            if letter == " ":
                # non-ascii character has been removed. just skip
                continue

            xcorn, ycorn = xg * letter_i, yg * row_i
            letter_img = fontimg.crop([xcorn, ycorn, xcorn + xg, ycorn + yg])
            # clumsy rn: crop to left half of the grid. buh
            letter_sub = letter_img.crop(char_limits)
            registry[letter] = rect_to_pixelslist(letter_sub)

    write_dataheader(registry=registry)
