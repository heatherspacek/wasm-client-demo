CC      = clang
INCLUDE = include
SRC     = $(wildcard src/*.c)
OUT     = public/game.wasm

PYTHON  = uv run python

FONT_SRC    = res/scratch1.png
FONT_HEADER = include/data/font_myscratch.h
FONT_SCRIPT = scripts/bake_font.py

SPRITE_SRC    = res/hands.png
SPRITE_HEADER = include/data/spritesheet.h
SPRITE_SCRIPT = scripts/bake_spritesheet.py

BAKE_HEADERS = $(FONT_HEADER) $(SPRITE_HEADER)

# -nostdlib          : no libc, no startup files -- we bring our own ABI
# --no-entry         : no _start / main required
# --export-dynamic   : keep all __attribute__((export_name)) exports
# -Wl,--allow-undefined : let the linker accept the js_* imports unresolved

CFLAGS = \
	--target=wasm32 \
	-nostdlib \
	-I$(INCLUDE) \
	-Wall \
	-Wl,--no-entry \
	-Wl,--export-dynamic \
	-Wl,--allow-undefined \
	-Wl,--import-memory \
# 	-O2 \

.PHONY: all clean

all: $(OUT)

$(OUT): $(SRC) $(BAKE_HEADERS)
	$(CC) -v $(CFLAGS) -o $@ $(SRC)
	@echo "Built $@ ($$(wc -c < $@) bytes)"

$(FONT_HEADER): $(FONT_SRC) $(FONT_SCRIPT)
	@mkdir -p $(dir $@)
	$(PYTHON) $(FONT_SCRIPT) $(FONT_SRC) $@

$(SPRITE_HEADER): $(SPRITE_SRC) $(SPRITE_SCRIPT)
	@mkdir -p $(dir $@)
	$(PYTHON) $(SPRITE_SCRIPT) $(SPRITE_SRC) $@

clean:
	rm -f $(OUT) $(BAKE_HEADERS)