#!/usr/bin/env python3
"""Prepare a disposable ROM with a background/HUD redraw probe; never publish it.
Run from the repository root after make debug. Requires devkitARM on PATH.
After entering a full doubles battle, use --capture SESSION to record all 23
implemented environments. The matching ROM and ELF are mandatory.
"""
import argparse
import json
import subprocess
import time
from pathlib import Path

P = Path(".cache/ui-contrast")
NAMES = "grass long-grass sand underwater water pond mountain cave building plain frontier gym leader magma aqua sidney phoebe glacia drake champion groudon kyogre rayquaza".split()
CLI = "/home/jastin/dev/pokeemerald-expansion/tools/mgba_live/mgba_live_cli.sh"
parser = argparse.ArgumentParser()
parser.add_argument("--capture", metavar="SESSION")
args = parser.parse_args()
P.mkdir(parents=True, exist_ok=True)
if not args.capture:
    symbols = {}
    for line in subprocess.check_output(["arm-none-eabi-nm", "pokeemerald.elf"], text=True).splitlines():
        parts = line.split()
        if len(parts) == 3:
            symbols[parts[2]] = int(parts[0], 16)
    (P / "symbols.json").write_text(json.dumps(symbols))
    asm = [".syntax unified", ".cpu arm7tdmi", ".thumb", ".global fixture", ".thumb_func", "fixture:", "push {r4, lr}",
           f"ldr r0, ={symbols['gBattleEnvironment']}", "ldrb r0, [r0]", f"ldr r3, ={symbols['LoadBattleEnvironmentGfx'] | 1}", "bl call_r3"]
    for battler, offset in enumerate([0, 600, 100, 700]):
        asm += [f"ldr r0, ={symbols['gHealthboxSpriteIds'] + battler}", "ldrb r0, [r0]",
                f"ldr r1, ={symbols['gParties'] + offset}", "movs r2, #0",
                f"ldr r3, ={symbols['UpdateHealthboxAttribute'] | 1}", "bl call_r3"]
    asm += ["pop {r4, pc}", ".thumb_func", "call_r3:", "bx r3", ".ltorg"]
    (P / "fixture.s").write_text("\n".join(asm) + "\n")
    subprocess.run(["arm-none-eabi-as", "-mthumb", "-o", str(P / "fixture.o"), str(P / "fixture.s")], check=True)
    subprocess.run(["arm-none-eabi-ld", "-Ttext=0x09ff0000", "-e", "fixture", "-o", str(P / "fixture.elf"), str(P / "fixture.o")], check=True)
    subprocess.run(["arm-none-eabi-objcopy", "-O", "binary", str(P / "fixture.elf"), str(P / "fixture.bin")], check=True)
    rom = bytearray(Path("pokeemerald.gba").read_bytes())
    stub = (P / "fixture.bin").read_bytes()
    assert len(rom) == 0x2000000
    assert all(value == 255 for value in rom[0x1ff0000:0x1ff0000 + len(stub)])
    rom[0x1ff0000:0x1ff0000 + len(stub)] = stub
    (P / "contrast.gba").write_bytes(rom)
    print("Prepared", P / "contrast.gba", "with matching ELF symbols")
else:
    symbols = json.loads((P / "symbols.json").read_text())
    out = Path("docs/features/ui_style_switch/evidence/contrast")
    out.mkdir(parents=True, exist_ok=True)
    for environment, name in enumerate(NAMES):
        code = f'''local original=emu:read32({symbols['gMain'] + 4});
        emu:write8({symbols['gBattleEnvironment']},{environment});
        emu:write32({symbols['gMain'] + 4},0x09ff0001);
        local n=0; local id; id=callbacks:add("frame",function()
          n=n+1; if n==3 then emu:write32({symbols['gMain'] + 4},original); callbacks:remove(id) end
        end)'''
        subprocess.run([CLI, "run-lua", "--session", args.capture, "--code", code], stdout=subprocess.DEVNULL, check=True)
        time.sleep(0.15)
        subprocess.run([CLI, "screenshot", "--session", args.capture, "--out", str((out / f"{environment:02}-{name}.png").resolve())], stdout=subprocess.DEVNULL, check=True)
    print("Captured", len(NAMES), "backgrounds")
