from PIL import Image

fontpath_in = "private/font_lookout_7.png"
headerpath_out = "private/font_lookout_auto.h"
spritesheet_grid = (16, 16)
character_limits = ...
charmap = [
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ",
    """abcdefghijklmnopqrstuvwxyz""",
    """0123456789.,;:?!"'+-=*%_()""",
    "[]{}~#&@©®™°^`|/\<>…€$£¢¿¡",
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


def pixels_from_rect(glyph: Image) -> list[list[int]]:
    data = []
    for row in range(16):
        rxx = []
        for col in range(16):
            rxx.append(int(glyph.getpixel([col, row]) == (255, 255, 255, 255)))
        data.append(rxx)
    return data


def compose_datastrings(): ...


def write_dataheader():
    PREAMBLE = "#include <stdint.h>\n"
    with open(headerpath_out, "w") as fp:
        fp.write(PREAMBLE)


if __name__ == "__main__":
    fontimg = Image.open(fontpath_in)

    xg, yg = spritesheet_grid  # 16, 16
    for row_i, row in enumerate(charmap):
        for letter_i, letter in enumerate(row):
            xcorn, ycorn = xg * letter_i, yg * row_i
            letter_img = fontimg.crop([xcorn, ycorn, xcorn + xg, ycorn + yg])

            ...

    breakpoint()

    write_dataheader()
