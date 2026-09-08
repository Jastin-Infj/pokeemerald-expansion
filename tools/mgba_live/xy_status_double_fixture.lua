-- Disposable fixture for the matching debug ELF; never writes a player save.
local done=false
callbacks:add("frame",function()
  if done then return end
  local flags=emu:read32(0x020000dc)
  if flags==4 or flags==12 then
    local party=0x02031c04
    for n=0,99 do emu:write8(party+100+n,emu:read8(party+n)) end
    emu:write8(0x02031bf4,2)
    -- Two player Pokemon, with status badges distinct from the enemy pair.
    emu:write32(party+0x50,0x10)
    emu:write32(party+100+0x50,0x40)
    emu:write32(party+600+0x50,0x08)
    emu:write32(party+700+0x50,0x01)
    for n=0,9 do emu:write8(party+100+8+n,0xC7) end -- 10 M glyphs
    emu:write8(party+100+18,0xFF)
    emu:write32(0x020000dc,flags|1)
    done=true
  end
end)
