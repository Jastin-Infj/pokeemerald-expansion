#ifndef GUARD_CONSTANTS_TMS_HMS_H
#define GUARD_CONSTANTS_TMS_HMS_H

#define FOREACH_TM(F) \
    F(TAKE_DOWN) \
    F(CHARM) \
    F(FAKE_TEARS) \
    F(AGILITY) \
    F(MUD_SLAP) \
    F(SCARY_FACE) \
    F(PROTECT) \
    F(FIRE_FANG) \
    F(THUNDER_FANG) \
    F(ICE_FANG) \
    F(WATER_PULSE) \
    F(LOW_KICK) \
    F(ACID_SPRAY) \
    F(ACROBATICS) \
    F(STRUGGLE_BUG) \
    F(PSYBEAM) \
    F(CONFUSE_RAY) \
    F(THIEF) \
    F(DISARMING_VOICE) \
    F(TRAILBLAZE) \
    F(POUNCE) \
    F(CHILLING_WATER) \
    F(CHARGE_BEAM) \
    F(FIRE_SPIN) \
    F(FACADE) \
    F(POISON_TAIL) \
    F(AERIAL_ACE) \
    F(BULLDOZE) \
    F(HEX) \
    F(SNARL) \
    F(METAL_CLAW) \
    F(SWIFT) \
    F(MAGICAL_LEAF) \
    F(ICY_WIND) \
    F(MUD_SHOT) \
    F(ROCK_TOMB) \
    F(DRAINING_KISS) \
    F(FLAME_CHARGE) \
    F(LOW_SWEEP) \
    F(AIR_CUTTER) \
    F(STORED_POWER) \
    F(NIGHT_SHADE) \
    F(FLING) \
    F(DRAGON_TAIL) \
    F(VENOSHOCK) \
    F(AVALANCHE) \
    F(ENDURE) \
    F(VOLT_SWITCH) \
    F(SUNNY_DAY) \
    F(RAIN_DANCE)

#define FOREACH_HM(F) \
    F(CUT) \
    F(FLY) \
    F(SURF) \
    F(STRENGTH) \
    F(FLASH) \
    F(ROCK_SMASH) \
    F(WATERFALL) \
    F(DIVE)

#define FOREACH_TMHM(F) \
    FOREACH_TM(F) \
    FOREACH_HM(F)

#endif
