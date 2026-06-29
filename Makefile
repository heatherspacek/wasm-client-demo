
CC      = clang
OUT     = public/game.wasm
INCLUDE = private
SRC     = src/*.c

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

$(OUT): $(SRC)
	$(CC) -v $(CFLAGS) -o $@ $<
	@echo "Built $@ ($$(wc -c < $@) bytes)"

clean:
	rm -f $(OUT)
