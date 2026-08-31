#pragma once
#include "types.h"

typedef struct {
    void (*callback)();
    uint32_t ms_remaining;
    int active;
    int visible;
} Timer;

void set_and_start_timer(int init_seconds, void *callback, Timer *timer);

void fire_and_stop_timer(Timer *timer);

void draw_timer(int x, int y, Timer *timer);

void _tick_timer(int ms_elapsed, Timer *timer);
