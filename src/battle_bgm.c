#include "battle_bgm.h"
#include "constants/songs.h"

static EWRAM_DATA u8 sBattleBgmChoices[BATTLE_BGM_TARGET_COUNT] =
{
    [BATTLE_BGM_TARGET_TRAINER] = BATTLE_BGM_CHOICE_DEFAULT,
    [BATTLE_BGM_TARGET_WILD]    = BATTLE_BGM_CHOICE_DEFAULT,
};

static const u8 *const sBattleBgmTargetNames[BATTLE_BGM_TARGET_COUNT] =
{
    [BATTLE_BGM_TARGET_TRAINER] = COMPOUND_STRING("Trainer"),
    [BATTLE_BGM_TARGET_WILD]    = COMPOUND_STRING("Wild"),
};

static const u8 *const sBattleBgmChoiceNames[BATTLE_BGM_CHOICE_COUNT] =
{
    [BATTLE_BGM_CHOICE_DEFAULT]           = COMPOUND_STRING("Default"),
    [BATTLE_BGM_CHOICE_HOENN_WILD]        = COMPOUND_STRING("Hoenn Wild"),
    [BATTLE_BGM_CHOICE_KANTO_WILD]        = COMPOUND_STRING("Kanto Wild"),
    [BATTLE_BGM_CHOICE_HOENN_TRAINER]     = COMPOUND_STRING("Hoenn Trainer"),
    [BATTLE_BGM_CHOICE_KANTO_TRAINER]     = COMPOUND_STRING("Kanto Trainer"),
    [BATTLE_BGM_CHOICE_HOENN_GYM]         = COMPOUND_STRING("Hoenn Gym"),
    [BATTLE_BGM_CHOICE_KANTO_GYM]         = COMPOUND_STRING("Kanto Gym"),
    [BATTLE_BGM_CHOICE_HOENN_CHAMPION]    = COMPOUND_STRING("Hoenn Champ"),
    [BATTLE_BGM_CHOICE_KANTO_CHAMPION]    = COMPOUND_STRING("Kanto Champ"),
    [BATTLE_BGM_CHOICE_ELITE_FOUR]        = COMPOUND_STRING("Elite Four"),
    [BATTLE_BGM_CHOICE_FRONTIER_BRAIN]    = COMPOUND_STRING("Frontier Brain"),
    [BATTLE_BGM_CHOICE_RIVAL]             = COMPOUND_STRING("Rival"),
    [BATTLE_BGM_CHOICE_AQUA_MAGMA]        = COMPOUND_STRING("Aqua/Magma"),
    [BATTLE_BGM_CHOICE_AQUA_MAGMA_LEADER] = COMPOUND_STRING("Aqua/Magma Lead"),
    [BATTLE_BGM_CHOICE_REGI]              = COMPOUND_STRING("Regi"),
    [BATTLE_BGM_CHOICE_GROUDON_KYOGRE]    = COMPOUND_STRING("Groudon/Kyogre"),
    [BATTLE_BGM_CHOICE_RAYQUAZA]          = COMPOUND_STRING("Rayquaza"),
    [BATTLE_BGM_CHOICE_MEW]               = COMPOUND_STRING("Mew"),
    [BATTLE_BGM_CHOICE_KANTO_LEGEND]      = COMPOUND_STRING("Kanto Legend"),
    [BATTLE_BGM_CHOICE_MEWTWO]            = COMPOUND_STRING("Mewtwo"),
    [BATTLE_BGM_CHOICE_DEOXYS]            = COMPOUND_STRING("Deoxys"),
    [BATTLE_BGM_CHOICE_LEGEND_BEAST]      = COMPOUND_STRING("Legend Beast"),
    [BATTLE_BGM_CHOICE_BW2_IRIS]          = COMPOUND_STRING("BW2 Iris"),
    [BATTLE_BGM_CHOICE_BW_LEGEND]         = COMPOUND_STRING("BW Legend"),
    [BATTLE_BGM_CHOICE_DP_WILD]           = COMPOUND_STRING("DPPt Wild"),
    [BATTLE_BGM_CHOICE_DP_TRAINER]        = COMPOUND_STRING("DPPt Trainer"),
    [BATTLE_BGM_CHOICE_DP_GYM]            = COMPOUND_STRING("DPPt Gym"),
    [BATTLE_BGM_CHOICE_DP_CHAMPION]       = COMPOUND_STRING("DPPt Champ"),
    [BATTLE_BGM_CHOICE_DP_LEGEND]         = COMPOUND_STRING("DPPt Legend"),
    [BATTLE_BGM_CHOICE_DP_ELITE_FOUR]     = COMPOUND_STRING("DPPt E4"),
    [BATTLE_BGM_CHOICE_DP_GALACTIC]       = COMPOUND_STRING("DPPt Galactic"),
    [BATTLE_BGM_CHOICE_DP_GALACTIC_COMMANDER] = COMPOUND_STRING("DPPt Cmdr"),
    [BATTLE_BGM_CHOICE_DP_CYRUS]          = COMPOUND_STRING("DPPt Cyrus"),
    [BATTLE_BGM_CHOICE_DP_RIVAL]          = COMPOUND_STRING("DPPt Rival"),
    [BATTLE_BGM_CHOICE_DP_UXIE_MESPRIT_AZELF] = COMPOUND_STRING("DPPt Lake Trio"),
    [BATTLE_BGM_CHOICE_DP_DIALGA_PALKIA]  = COMPOUND_STRING("DPPt Dialga"),
    [BATTLE_BGM_CHOICE_DP_ARCEUS]         = COMPOUND_STRING("DPPt Arceus"),
    [BATTLE_BGM_CHOICE_PL_GIRATINA]       = COMPOUND_STRING("Plt Giratina"),
    [BATTLE_BGM_CHOICE_PL_FRONTIER_BRAIN] = COMPOUND_STRING("Plt Frontier"),
    [BATTLE_BGM_CHOICE_PL_REGI]           = COMPOUND_STRING("Plt Regi"),
    [BATTLE_BGM_CHOICE_HGSS_WILD]         = COMPOUND_STRING("HGSS Wild"),
    [BATTLE_BGM_CHOICE_HGSS_TRAINER]      = COMPOUND_STRING("HGSS Trainer"),
    [BATTLE_BGM_CHOICE_HGSS_GYM]          = COMPOUND_STRING("HGSS Gym"),
    [BATTLE_BGM_CHOICE_HGSS_CHAMPION]     = COMPOUND_STRING("HGSS Champ"),
    [BATTLE_BGM_CHOICE_HGSS_LUGIA]        = COMPOUND_STRING("HGSS Lugia"),
    [BATTLE_BGM_CHOICE_HGSS_HO_OH]        = COMPOUND_STRING("HGSS Ho-Oh"),
    [BATTLE_BGM_CHOICE_HGSS_ROCKET]       = COMPOUND_STRING("HGSS Rocket"),
    [BATTLE_BGM_CHOICE_HGSS_RIVAL]        = COMPOUND_STRING("HGSS Rival"),
    [BATTLE_BGM_CHOICE_HGSS_SUICUNE]      = COMPOUND_STRING("HGSS Suicune"),
    [BATTLE_BGM_CHOICE_HGSS_ENTEI]        = COMPOUND_STRING("HGSS Entei"),
    [BATTLE_BGM_CHOICE_HGSS_RAIKOU]       = COMPOUND_STRING("HGSS Raikou"),
    [BATTLE_BGM_CHOICE_HGSS_WILD_KANTO]   = COMPOUND_STRING("HGSS Kanto Wild"),
    [BATTLE_BGM_CHOICE_HGSS_TRAINER_KANTO] = COMPOUND_STRING("HGSS Kanto Trn"),
    [BATTLE_BGM_CHOICE_HGSS_GYM_KANTO]    = COMPOUND_STRING("HGSS Kanto Gym"),
    [BATTLE_BGM_CHOICE_HGSS_FRONTIER_BRAIN] = COMPOUND_STRING("HGSS Frontier"),
    [BATTLE_BGM_CHOICE_HGSS_KYOGRE_GROUDON] = COMPOUND_STRING("HGSS Grou/Kyo"),
    [BATTLE_BGM_CHOICE_HGSS_ARCEUS]       = COMPOUND_STRING("HGSS Arceus"),
};

static bool8 IsTrainerBattleBgm(u16 songId)
{
    switch (songId)
    {
    case MUS_VS_TRAINER:
    case MUS_VS_AQUA_MAGMA:
    case MUS_VS_AQUA_MAGMA_LEADER:
    case MUS_VS_RIVAL:
    case MUS_VS_GYM_LEADER:
    case MUS_VS_ELITE_FOUR:
    case MUS_VS_FRONTIER_BRAIN:
    case MUS_VS_CHAMPION:
    case MUS_RG_VS_TRAINER:
    case MUS_RG_VS_GYM_LEADER:
    case MUS_RG_VS_CHAMPION:
    case MUS_BW_VS_IRIS:
    case MUS_DP_VS_TRAINER:
    case MUS_DP_VS_GYM_LEADER:
    case MUS_DP_VS_CHAMPION:
    case MUS_DP_VS_ELITE_FOUR:
    case MUS_DP_VS_GALACTIC:
    case MUS_DP_VS_GALACTIC_COMMANDER:
    case MUS_DP_VS_GALACTIC_BOSS:
    case MUS_DP_VS_RIVAL:
    case MUS_PL_VS_FRONTIER_BRAIN:
    case MUS_HG_VS_TRAINER:
    case MUS_HG_VS_GYM_LEADER:
    case MUS_HG_VS_CHAMPION:
    case MUS_HG_VS_ROCKET:
    case MUS_HG_VS_RIVAL:
    case MUS_HG_VS_TRAINER_KANTO:
    case MUS_HG_VS_GYM_LEADER_KANTO:
    case MUS_HG_VS_FRONTIER_BRAIN:
        return TRUE;
    default:
        return FALSE;
    }
}

static bool8 IsWildBattleBgm(u16 songId)
{
    switch (songId)
    {
    case MUS_VS_WILD:
    case MUS_RG_VS_WILD:
    case MUS_C_VS_LEGEND_BEAST:
    case MUS_VS_RAYQUAZA:
    case MUS_VS_KYOGRE_GROUDON:
    case MUS_VS_REGI:
    case MUS_VS_MEW:
    case MUS_RG_VS_DEOXYS:
    case MUS_RG_VS_MEWTWO:
    case MUS_RG_VS_LEGEND:
    case MUS_BW_VS_LEGEND:
    case MUS_DP_VS_WILD:
    case MUS_DP_VS_LEGEND:
    case MUS_DP_VS_UXIE_MESPRIT_AZELF:
    case MUS_DP_VS_DIALGA_PALKIA:
    case MUS_DP_VS_ARCEUS:
    case MUS_PL_VS_GIRATINA:
    case MUS_PL_VS_REGI:
    case MUS_HG_VS_WILD:
    case MUS_HG_VS_LUGIA:
    case MUS_HG_VS_HO_OH:
    case MUS_HG_VS_SUICUNE:
    case MUS_HG_VS_ENTEI:
    case MUS_HG_VS_RAIKOU:
    case MUS_HG_VS_WILD_KANTO:
    case MUS_HG_VS_KYOGRE_GROUDON:
    case MUS_HG_VS_ARCEUS:
        return TRUE;
    default:
        return FALSE;
    }
}

static u16 GetBattleBgmChoiceSong(u8 choice)
{
    switch (choice)
    {
    case BATTLE_BGM_CHOICE_HOENN_WILD:
        return MUS_VS_WILD;
    case BATTLE_BGM_CHOICE_KANTO_WILD:
        return MUS_RG_VS_WILD;
    case BATTLE_BGM_CHOICE_HOENN_TRAINER:
        return MUS_VS_TRAINER;
    case BATTLE_BGM_CHOICE_KANTO_TRAINER:
        return MUS_RG_VS_TRAINER;
    case BATTLE_BGM_CHOICE_HOENN_GYM:
        return MUS_VS_GYM_LEADER;
    case BATTLE_BGM_CHOICE_KANTO_GYM:
        return MUS_RG_VS_GYM_LEADER;
    case BATTLE_BGM_CHOICE_HOENN_CHAMPION:
        return MUS_VS_CHAMPION;
    case BATTLE_BGM_CHOICE_KANTO_CHAMPION:
        return MUS_RG_VS_CHAMPION;
    case BATTLE_BGM_CHOICE_ELITE_FOUR:
        return MUS_VS_ELITE_FOUR;
    case BATTLE_BGM_CHOICE_FRONTIER_BRAIN:
        return MUS_VS_FRONTIER_BRAIN;
    case BATTLE_BGM_CHOICE_RIVAL:
        return MUS_VS_RIVAL;
    case BATTLE_BGM_CHOICE_AQUA_MAGMA:
        return MUS_VS_AQUA_MAGMA;
    case BATTLE_BGM_CHOICE_AQUA_MAGMA_LEADER:
        return MUS_VS_AQUA_MAGMA_LEADER;
    case BATTLE_BGM_CHOICE_REGI:
        return MUS_VS_REGI;
    case BATTLE_BGM_CHOICE_GROUDON_KYOGRE:
        return MUS_VS_KYOGRE_GROUDON;
    case BATTLE_BGM_CHOICE_RAYQUAZA:
        return MUS_VS_RAYQUAZA;
    case BATTLE_BGM_CHOICE_MEW:
        return MUS_VS_MEW;
    case BATTLE_BGM_CHOICE_KANTO_LEGEND:
        return MUS_RG_VS_LEGEND;
    case BATTLE_BGM_CHOICE_MEWTWO:
        return MUS_RG_VS_MEWTWO;
    case BATTLE_BGM_CHOICE_DEOXYS:
        return MUS_RG_VS_DEOXYS;
    case BATTLE_BGM_CHOICE_LEGEND_BEAST:
        return MUS_C_VS_LEGEND_BEAST;
    case BATTLE_BGM_CHOICE_BW2_IRIS:
        return MUS_BW_VS_IRIS;
    case BATTLE_BGM_CHOICE_BW_LEGEND:
        return MUS_BW_VS_LEGEND;
    case BATTLE_BGM_CHOICE_DP_WILD:
        return MUS_DP_VS_WILD;
    case BATTLE_BGM_CHOICE_DP_TRAINER:
        return MUS_DP_VS_TRAINER;
    case BATTLE_BGM_CHOICE_DP_GYM:
        return MUS_DP_VS_GYM_LEADER;
    case BATTLE_BGM_CHOICE_DP_CHAMPION:
        return MUS_DP_VS_CHAMPION;
    case BATTLE_BGM_CHOICE_DP_LEGEND:
        return MUS_DP_VS_LEGEND;
    case BATTLE_BGM_CHOICE_DP_ELITE_FOUR:
        return MUS_DP_VS_ELITE_FOUR;
    case BATTLE_BGM_CHOICE_DP_GALACTIC:
        return MUS_DP_VS_GALACTIC;
    case BATTLE_BGM_CHOICE_DP_GALACTIC_COMMANDER:
        return MUS_DP_VS_GALACTIC_COMMANDER;
    case BATTLE_BGM_CHOICE_DP_CYRUS:
        return MUS_DP_VS_GALACTIC_BOSS;
    case BATTLE_BGM_CHOICE_DP_RIVAL:
        return MUS_DP_VS_RIVAL;
    case BATTLE_BGM_CHOICE_DP_UXIE_MESPRIT_AZELF:
        return MUS_DP_VS_UXIE_MESPRIT_AZELF;
    case BATTLE_BGM_CHOICE_DP_DIALGA_PALKIA:
        return MUS_DP_VS_DIALGA_PALKIA;
    case BATTLE_BGM_CHOICE_DP_ARCEUS:
        return MUS_DP_VS_ARCEUS;
    case BATTLE_BGM_CHOICE_PL_GIRATINA:
        return MUS_PL_VS_GIRATINA;
    case BATTLE_BGM_CHOICE_PL_FRONTIER_BRAIN:
        return MUS_PL_VS_FRONTIER_BRAIN;
    case BATTLE_BGM_CHOICE_PL_REGI:
        return MUS_PL_VS_REGI;
    case BATTLE_BGM_CHOICE_HGSS_WILD:
        return MUS_HG_VS_WILD;
    case BATTLE_BGM_CHOICE_HGSS_TRAINER:
        return MUS_HG_VS_TRAINER;
    case BATTLE_BGM_CHOICE_HGSS_GYM:
        return MUS_HG_VS_GYM_LEADER;
    case BATTLE_BGM_CHOICE_HGSS_CHAMPION:
        return MUS_HG_VS_CHAMPION;
    case BATTLE_BGM_CHOICE_HGSS_LUGIA:
        return MUS_HG_VS_LUGIA;
    case BATTLE_BGM_CHOICE_HGSS_HO_OH:
        return MUS_HG_VS_HO_OH;
    case BATTLE_BGM_CHOICE_HGSS_ROCKET:
        return MUS_HG_VS_ROCKET;
    case BATTLE_BGM_CHOICE_HGSS_RIVAL:
        return MUS_HG_VS_RIVAL;
    case BATTLE_BGM_CHOICE_HGSS_SUICUNE:
        return MUS_HG_VS_SUICUNE;
    case BATTLE_BGM_CHOICE_HGSS_ENTEI:
        return MUS_HG_VS_ENTEI;
    case BATTLE_BGM_CHOICE_HGSS_RAIKOU:
        return MUS_HG_VS_RAIKOU;
    case BATTLE_BGM_CHOICE_HGSS_WILD_KANTO:
        return MUS_HG_VS_WILD_KANTO;
    case BATTLE_BGM_CHOICE_HGSS_TRAINER_KANTO:
        return MUS_HG_VS_TRAINER_KANTO;
    case BATTLE_BGM_CHOICE_HGSS_GYM_KANTO:
        return MUS_HG_VS_GYM_LEADER_KANTO;
    case BATTLE_BGM_CHOICE_HGSS_FRONTIER_BRAIN:
        return MUS_HG_VS_FRONTIER_BRAIN;
    case BATTLE_BGM_CHOICE_HGSS_KYOGRE_GROUDON:
        return MUS_HG_VS_KYOGRE_GROUDON;
    case BATTLE_BGM_CHOICE_HGSS_ARCEUS:
        return MUS_HG_VS_ARCEUS;
    case BATTLE_BGM_CHOICE_DEFAULT:
    default:
        return 0;
    }
}

u8 GetBattleBgmChoice(u8 target)
{
    if (target >= BATTLE_BGM_TARGET_COUNT)
        target = BATTLE_BGM_TARGET_TRAINER;

    return sBattleBgmChoices[target];
}

void SetBattleBgmChoice(u8 target, u8 choice)
{
    if (target >= BATTLE_BGM_TARGET_COUNT)
        return;

    if (choice < BATTLE_BGM_CHOICE_COUNT)
        sBattleBgmChoices[target] = choice;
    else
        sBattleBgmChoices[target] = BATTLE_BGM_CHOICE_DEFAULT;
}

const u8 *GetBattleBgmTargetName(u8 target)
{
    if (target >= BATTLE_BGM_TARGET_COUNT)
        target = BATTLE_BGM_TARGET_TRAINER;

    return sBattleBgmTargetNames[target];
}

const u8 *GetBattleBgmChoiceName(u8 choice)
{
    if (choice >= BATTLE_BGM_CHOICE_COUNT)
        choice = BATTLE_BGM_CHOICE_DEFAULT;

    return sBattleBgmChoiceNames[choice];
}

u16 GetBattleBgmChoicePreviewSong(u8 target, u8 choice)
{
    u16 songId = GetBattleBgmChoiceSong(choice);

    if (songId != 0)
        return songId;

    switch (target)
    {
    case BATTLE_BGM_TARGET_WILD:
        return MUS_VS_WILD;
    case BATTLE_BGM_TARGET_TRAINER:
    default:
        return MUS_VS_TRAINER;
    }
}

u16 ApplyBattleBgmSelection(u16 songId)
{
    u16 override;

    if (IsWildBattleBgm(songId))
        override = GetBattleBgmChoiceSong(sBattleBgmChoices[BATTLE_BGM_TARGET_WILD]);
    else if (IsTrainerBattleBgm(songId))
        override = GetBattleBgmChoiceSong(sBattleBgmChoices[BATTLE_BGM_TARGET_TRAINER]);
    else
        override = 0;

    return override != 0 ? override : songId;
}
