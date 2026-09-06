#include "animation.h"
#include "drawing.h"


static int _anim_inactive(Animation *a) {
    return a->anim_ctr >= a->anim_len;
}

static int _first_open_anim_slot(AnimationArena aa)
{
    for (int a_i = 0; a_i < MAX_ANIM_ONSCREEN; a_i++)
    {
        if (_anim_inactive(&aa[a_i])) {
            // anim done playing, safe to overwrite <3
            return a_i;
        }
    }
    return -1;
}

void tick_all_animations(AnimationArena aa)
{
    for (int a_i = 0; a_i < MAX_ANIM_ONSCREEN; a_i++)
    {
        if (!_anim_inactive(&aa[a_i])) {
            Animation *anim_ptr = &aa[a_i];
            anim_ptr->anim_cbk(anim_ptr->x ,anim_ptr->y, anim_ptr->anim_ctr);
            anim_ptr->anim_ctr++;
        }
    }
}

int _make_anim_sparklegrow(int x, int y, AnimationArena aa)
{
    Animation anim = {x, y, 18, 0, _anim_func_sparklegrow};
    int slot = _first_open_anim_slot(aa);
    aa[slot] = anim;
    return slot;
}

void _anim_func_sparklegrow(int x, int y, int t)
{
    _draw_sparkle(x,     y-t,     t+3);
    if (t<12)
    {
        _draw_sparkle(x+t,   y-2*t,   (int)(t+2)*0.8);
    }
    if (t<9)
    {
        _draw_sparkle(x-2*t, y-3*t,    (int)(t+1)*0.5);
    }
}
