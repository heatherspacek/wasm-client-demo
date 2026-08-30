#include "timers.h"
#include "text.h"

void set_and_start_timer(int init_seconds, void *callback, Timer *timer) {
    timer->ms_remaining = init_seconds * 1000;
    timer->active = 1;
    timer->callback = callback;
}

void fire_and_stop_timer(Timer *timer) {
    timer->ms_remaining = 0;
    timer->active = 0;
    timer->callback();
}

void draw_timer(int x, int y, Timer *timer) {

    _render_int(x, y, timer->ms_remaining);
}

void _tick_timer(int ms_elapsed, Timer *timer) {
    if (timer->active) {
        if (ms_elapsed >= timer->ms_remaining) {
            fire_and_stop_timer(timer);
        }
        else {
            timer->ms_remaining -= ms_elapsed;
        }
    }
}
