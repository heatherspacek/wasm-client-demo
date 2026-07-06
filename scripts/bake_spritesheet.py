from PIL import Image
import math

spritesheet_in = "res/spritesheet.png"
headerpath_out = "include/data/spritesheet.h"
spritesheet_grid = (32, 32)


def sub_img_to_hexvals(subimg: Image) -> list[str]:
    vals = []
    xg, yg = spritesheet_grid
    for yp in range(yg):
        for xp in range(xg):
            px = sub_img.getpixel((xp, yp))
            vals.append("0x{3:02X}{2:02X}{1:02X}{0:02X}".format(*px))
    return vals


def FMT(index, data):
    return f"""\
static const uint32_t sprite_{index}[{math.prod(spritesheet_grid)}] = {{
    {data}
}};
"""


def write_dataheader(registry: list[str]):
    PREAMBLE = """\
#pragma once
#include <stdint.h>
"""
    with open(headerpath_out, "w") as fp:
        fp.write(PREAMBLE)
        for i, entry in enumerate(registry):
            fp.write(FMT(index=i, data=",\n".join(entry)))
        fp.write(f"""\
static const uint32_t* all_sprites[] = {{
    {",\n".join([f"sprite_{i}" for i in range(len(registry))])}
}};
                 """)


if __name__ == "__main__":
    fontimg = Image.open(spritesheet_in)
    xg, yg = spritesheet_grid
    xi, yi = fontimg.size
    n_cols = math.floor(xi / xg)
    n_rows = math.floor(yi / yg)

    registry = []

    for row_i in range(n_rows):
        for col_i in range(n_cols):
            sub_img = fontimg.crop(
                (col_i * xg, row_i * yg, (col_i + 1) * xg, (row_i + 1) * xg)
            )
            registry.append(sub_img_to_hexvals(sub_img))
    write_dataheader(registry=registry)
