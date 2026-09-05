#include "sounds.h"
#define STB_HEXWAVE_NO_ALLOCATION
#define STB_HEXWAVE_IMPLEMENTATION
#include "stb_hexwave.h"


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

void play_sfx2(SfxBuf buffer)
{
    const int W = 16;
    const int O = 8;
    float osc_buffer[16 * W * (O+1)];

    hexwave_init(16, 8, osc_buffer);
    HexWave osc = {0};
    hexwave_create(&osc, 1, 0.2, 0, 0);
    hexwave_generate_samples(buffer, 512, &osc, 0.1);
    hexwave_shutdown(osc_buffer);

    js_play_sfx_buffer();
}
