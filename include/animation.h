
#define MAX_ANIM_FRAMES 20
#define MAX_ANIM_ONSCREEN 50

typedef struct {
    int x;
    int y;
    int anim_len;
    int anim_ctr;
    void (*anim_cbk)(int, int, int);
} Animation;

typedef Animation AnimationArena[MAX_ANIM_ONSCREEN];

void tick_all_animations(AnimationArena aa);

int _make_anim_sparklegrow(int x, int y, AnimationArena aa);
void _anim_func_sparklegrow(int x, int y, int t);
