#include "global.h"
#include "pokemon.h"
#include "event_data.h"
#include "item.h"

void Battle_EndTrainerBattle();
void RemoveAnyFaintedMons(bool8 keepItems);


void Battle_EndTrainerBattle()
{
  // FlagClear(FLAG_ROGUE_DYNAMAX_BATTLE);
  // FlagClear(FLAG_ROGUE_TERASTALLIZE_BATTLE);

  RemoveAnyFaintedMons(FALSE);
}

void RemoveAnyFaintedMons(bool8 keepItems)
{
  u8 read;
  bool8 hasValidSpecies;
  u8 write = 0;

  for(read = 0; read < PARTY_SIZE; ++read)
  {
    hasValidSpecies = GetMonData(&gPlayerParty[read] , MON_DATA_SPECIES) != SPECIES_NONE;
    
    // fainted
    if(hasValidSpecies && GetMonData(&gPlayerParty[read] , MON_DATA_HP , NULL) != 0)
    {

      if(write != read)
      {
        CopyMon(&gPlayerParty[write] , &gPlayerParty[read] , sizeof(struct Pokemon));
        ZeroMonData(&gPlayerParty[read]);
      }
      ++write;
    }
    else if(hasValidSpecies)
    {
      if(keepItems)
      {
        // held item to bug
        u16 helditem = GetMonData(&gPlayerParty[read] , MON_DATA_HELD_ITEM);
        if(helditem != ITEM_NONE)
          AddBagItem(helditem , 1);
      }
    }
  }
}