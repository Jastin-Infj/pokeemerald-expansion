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
    F(LIGHT_SCREEN) \
    F(TACKLE) \
    F(RAIN_DANCE) \
    F(GIGA_DRAIN) \
    F(SAFEGUARD) \
    F(FRUSTRATION) \
    F(SOLAR_BEAM) \
    F(IRON_TAIL) \
    F(THUNDERBOLT) \
    F(THUNDER) \
    F(EARTHQUAKE) \
    F(RETURN) \
    F(DIG) \
    F(PSYCHIC) \
    F(SHADOW_BALL) \
    F(BRICK_BREAK) \
    F(DOUBLE_TEAM) \
    F(REFLECT) \
    F(SHOCK_WAVE) \
    F(FLAMETHROWER) \
    F(SLUDGE_BOMB) \
    F(SANDSTORM) \
    F(FIRE_BLAST) \
    F(ROCK_TOMB) \
    F(AERIAL_ACE) \
    F(TORMENT) \
    F(FACADE) \
    F(SECRET_POWER) \
    F(REST) \
    F(ATTRACT) \
    F(THIEF) \
    F(STEEL_WING) \
    F(SKILL_SWAP) \
    F(SNATCH) \
    F(OVERHEAT)

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
