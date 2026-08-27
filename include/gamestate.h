
#include "types.h"
typedef enum {
    INACTIVE = 0,
    QUEUED,
    DRAFT_W_1,
    DRAFT_W_2,
    DRAFT_1,
    DRAFT_2,
    DRAFT_3,
    DRAFT_4,
    DRAFT_5,
    DRAFT_6,
    DRAFT_7,
    DRAFT_8,
    DRAFT_9,
    DRAFT_10,
    DRAFT_11,
    DRAFT_12,
    COMBAT,
    SIDEBOARD,
} GamePhase ;

void *determine_gamefunc(GamePhase gp);
