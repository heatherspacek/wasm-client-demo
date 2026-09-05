#include "sounds.h"
#define STB_HEXWAVE_NO_ALLOCATION
#define STB_HEXWAVE_IMPLEMENTATION

void play_sfx1(SfxBuf buffer)
{
    float mult = 1.0;
    for (int i=0; i<512; i += 8) {
        buffer[i] = 0.0;
        buffer[i+1] = 0.707 * mult;
        buffer[i+2] = 1.0 * mult;
        buffer[i+3] = 0.707 * mult;
        buffer[i+4] = 0.0;
        buffer[i+5] = -0.707 * mult;
        buffer[i+6] = -1.0 * mult;
        buffer[i+7] = -0.707 * mult;
        mult *= 0.94;
    }
    js_play_sfx_buffer();
}
