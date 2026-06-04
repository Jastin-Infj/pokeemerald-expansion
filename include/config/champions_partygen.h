#ifndef GUARD_CONFIG_CHAMPIONS_PARTYGEN_H
#define GUARD_CONFIG_CHAMPIONS_PARTYGEN_H

/*
 * Champions Party Generator settings.
 *
 * This header is intentionally standalone because src/data/trainers.party is
 * preprocessed with -traditional-cpp before trainerproc. Keep macro values
 * numeric and avoid trailing // comments on macro lines.
 */

#define B_CHAMPIONS_PARTYGEN_TRAINERS           1
#define B_CHAMPIONS_PARTYGEN_LEVEL              50

#define B_CHAMPIONS_PARTYGEN_EXP_NORMAL         0
#define B_CHAMPIONS_PARTYGEN_EXP_NONE           1
#define B_CHAMPIONS_PARTYGEN_EXP_MODE           B_CHAMPIONS_PARTYGEN_EXP_NONE

#define B_CHAMPIONS_PARTYGEN_EV_NORMAL          0
#define B_CHAMPIONS_PARTYGEN_EV_NONE            1
#define B_CHAMPIONS_PARTYGEN_EV_MODE            B_CHAMPIONS_PARTYGEN_EV_NONE

#define B_CHAMPIONS_PARTYGEN_BADGE_BOOSTS       0
#define B_CHAMPIONS_PARTYGEN_OBEDIENCE_CHECKS   0

#endif /* GUARD_CONFIG_CHAMPIONS_PARTYGEN_H */
