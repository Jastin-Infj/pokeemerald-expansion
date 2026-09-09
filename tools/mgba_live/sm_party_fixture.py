#!/usr/bin/env python3
"""Generate a disposable six-slot party fixture from the matching debug ELF.

Load the generated Lua in mGBA before Party > Start Debug Battle. Never run it
against a player save or a different ROM/ELF. It changes only emulator RAM.
"""
import argparse
from pathlib import Path
import subprocess

parser = argparse.ArgumentParser()
parser.add_argument("--elf", required=True, type=Path)
parser.add_argument("--out", type=Path, default=Path(".cache/sm-menu/party-fixture.lua"))
parser.add_argument("--double", action="store_true")
args = parser.parse_args()
symbols = {}
for line in subprocess.check_output(["arm-none-eabi-nm", str(args.elf)], text=True).splitlines():
    parts = line.split()
    if len(parts) == 3:
        symbols[parts[2]] = int(parts[0], 16)
party, count, flags = (symbols[k] for k in ("gParties", "gPartiesCount", "gBattleTypeFlags"))
double = f"emu:write32({flags}, flags | 1)" if args.double else ""
code = f'''-- Display fixture for {args.elf.name}; all addresses resolved from that ELF.
local done = false
callbacks:add("frame", function()
  if done then return end
  local flags = emu:read32({flags})
  if flags ~= 4 and flags ~= 12 then return end
  local party = {party}
  for slot = 1, 5 do
    for n = 0, 99 do emu:write8(party + slot * 100 + n, emu:read8(party + n)) end
  end
  emu:write8({count}, 6)
  emu:write16(party + 100 + 0x56, 260)
  emu:write32(party + 100 + 0x50, 0x40)
  emu:write16(party + 200 + 0x56, 59)
  emu:write32(party + 200 + 0x50, 8)
  emu:write16(party + 300 + 0x56, 0)
  emu:write32(party + 300 + 0x50, 0)
  -- Twelve-character names span the header and encrypted growth substructure.
  -- Offset 18 is language/nature, NOT a nickname terminator.
  local mon = party + 400
  for n = 0, 9 do emu:write8(mon + 8 + n, 0xC7) end
  local personality = emu:read32(mon)
  local key = personality ~ emu:read32(mon + 4)
  local growth = emu:read8({symbols['sSubstructOffsets']} + personality % 24) * 3
  local words = {{}}
  for n = 0, 11 do words[n] = emu:read32(mon + 32 + n * 4) ~ key end
  words[growth + 1] = (words[growth + 1] & ~(255 << 21)) | (0xC7 << 21)
  words[growth + 2] = (words[growth + 2] & ~(255 << 22)) | (0xC7 << 22)
  local checksum = 0
  for n = 0, 11 do
    checksum = checksum + (words[n] & 65535) + ((words[n] >> 16) & 65535)
    emu:write32(mon + 32 + n * 4, words[n] ~ key)
  end
  emu:write16(mon + 28, checksum & 65535)
  {double}
  done = true
end)
'''
args.out.parent.mkdir(parents=True, exist_ok=True)
args.out.write_text(code)
print(args.out)
