#include <stdint.h>
#include "text.h"
#include "data/spritesheet.h"

enum Spelltype
{
    VOID = 0,
    MIND = 1,
    SLIME = 2,
    METAL = 3,
    AIR = 4,
    ZAP = 5,
    FIRE = 6,
    ICE = 7
};

struct spellinfo
{
    String_View name;
    int cost;
    enum Spelltype type;
    String_View description;
    const uint32_t *sprite;
    // TODO: spell behaviour!!!!
};

struct spellobject
{
    int x;
    int y;
    int grab_state;
    int hover_state;
    struct spellinfo spelldata;
};

const struct spellinfo spell00 = {
    .name = (String_View){9, "Ice Lance"},
    .cost = 25,
    .type = ICE,
    .description = SVC("Launch a penetrating \nbolt of ice."),
    .sprite = sprite_0};

const struct spellinfo spell01 = {
    .name = SVC("Acid Spray"),
    .cost = 15,
    .type = SLIME,
    .description = SVC("Fire a glob of \ndissolving ooze."),
    .sprite = sprite_1};

const struct spellinfo allspells[] = {
    spell00,
    spell01,
};