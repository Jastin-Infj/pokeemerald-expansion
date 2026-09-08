-- XY-inspired status HUD. Run with real Aseprite, root and out_dir parameters.
-- The user reference is recorded in docs/features/xy_status_hud_constraints.md.
-- Generate into an existing directory, inspect boards, then install assets.
local root = assert(app.params.root)
local out = assert(app.params.out_dir)
assert(app.fs.isDirectory(out))
local font = dofile(root .. "/tools/xy_hud_font.lua")
local function open(name)
  return assert(app.open(root .. "/graphics/battle_interface/" .. name .. ".png"))
end
local function save(s, name)
  s:saveAs(out .. "/xy_" .. name .. ".png")
  s:saveAs(out .. "/xy_" .. name .. ".aseprite")
end
local function rect(im,x,y,w,h,c)
  for yy=y,y+h-1 do for xx=x,x+w-1 do im:drawPixel(xx,yy,c) end end
end
local function clearAsset(name)
  local s=open(name); s.cels[1].image:clear(0)
  s.layers[1].name="Transparent XY status field"
  save(s,name); s:close()
end
-- Text is drawn by the runtime. The large opaque Emerald panels disappear.
for _,name in ipairs({"healthbox_singles_player","healthbox_singles_opponent",
  "healthbox_doubles_player","healthbox_doubles_opponent","misc",
  "healthbox_doubles_frameend","healthbox_doubles_frameend_bar"}) do clearAsset(name) end

local s=open("ball_display")
local pal=Palette(16)
for n=0,15 do pal:setColor(n,s.palettes[1]:getColor(n)) end
-- Separate HP palette: party-ball graphics also use 7..9, so the runtime
-- must load this palette only for TAG_HEALTHBAR_PAL, not summary balls.
pal:setColor(7,Color{r=205,g=255,b=82})
pal:setColor(8,Color{r=148,g=213,b=49})
pal:setColor(9,Color{r=90,g=148,b=32})
s:setPalette(pal);save(s,"healthbar_palette"); s:close()

local function bar(im,x,filled,dark,light)
  rect(im,x,0,8,8,0)
  rect(im,x,1,8,1,1);rect(im,x,2,8,1,4)
  rect(im,x,3,8,3,5)
  rect(im,x,3,filled,1,light);rect(im,x,4,filled,2,dark)
  rect(im,x,6,8,1,1)
end
s=open("hpbar");s:setPalette(pal)
local im=s.cels[1].image;im:clear(0)
-- 16x8 HP capsule prefix. Letter forms are new pixel art, not rasterized text.
rect(im,9,1,15,6,1);rect(im,10,2,14,1,4)
local letters={"10101110","10101010","11101110","10101000","10101000"}
for y,row in ipairs(letters) do for x=1,8 do
  if row:sub(x,x)=="1" then im:drawPixel(12+x-1,y+1,y<3 and 7 or 8) end
end end
for n=0,8 do bar(im,(n+3)*8,n,11,10) end
s.layers[1].name="Lime HP label and 48px bar states";save(s,"hpbar");s:close()
s=open("hpbar_anim");s:setPalette(pal);im=s.cels[1].image
for n=0,8 do bar(im,n*8,n,13,12) end
bar(im,72,0,11,10)
for n=1,8 do bar(im,(n+9)*8,n,15,14) end
save(s,"hpbar_anim");s:close()

s=open("expbar");im=s.cels[1].image;im:clear(0)
for n=0,8 do
  rect(im,n*8,3,8,2,1)
  rect(im,n*8,3,n,1,11)
  rect(im,n*8,4,n,1,11)
end
s.layers[1].name="Thin blue experience strip";save(s,"expbar");s:close()

-- Status glyphs already use a battler-specific dynamic palette index.
-- Keep their exact glyphs and status order; remove the pale rectangular surround.
for battler,name in ipairs({"status","status2","status3","status4"}) do
  s=open(name);im=s.cels[1].image
  im:clear(0)
  for row,label in ipairs({"PSN","PAR","SLP","FRZ","BRN","FRB"}) do
    local y=(row-1)*8
    rect(im,1,y,14,8,1);rect(im,0,y+1,16,6,1)
    rect(im,1,y+1,14,6,11+battler)
    local glyph=Image(16,12,ColorMode.INDEXED)
    local kind=font.fit(label,14)
    font.draw(glyph,1,0,label,2,nil,kind)
    for yy=4,10 do for xx=1,14 do
      if glyph:getPixel(xx,yy)==2 then im:drawPixel(xx,y+yy-3,2) end
    end end
  end
  s.layers[1].name="Compact status capsule - runtime color index preserved"
  save(s,name);s:close()
end
-- Frame-end blank must be transparent when status replaces the HP prefix.
clearAsset("misc_frameend")

local function rgba(r,g,b) return app.pixelColor.rgba(r,g,b,255) end
local white,dark,gray=rgba(255,255,222),rgba(24,24,32),rgba(82,90,98)
local lime,blue=rgba(180,238,65),rgba(65,205,255)
local function layer(s,name)
  local l=s:newLayer();l.name=name
  local i=Image(s.width,s.height,ColorMode.RGB)
  s:newCel(l,1,i);return s.cels[#s.cels].image
end
local function text(i,x,y,str,c,width)
  local kind=font.fit(str,width or 230-x)
  font.draw(i,x,y,str,c or white,dark,kind)
end
local function pokemon(i,name,back,x,y)
  local path=root.."/graphics/pokemon/"..name.."/"..(back and "back" or "anim_front")..".png"
  local mon=assert(app.open(path));local src=mon.cels[1].image
  local p=Palette{fromFile=root.."/graphics/pokemon/"..name.."/normal.pal"}
  for yy=0,63 do for xx=0,63 do
    local v=src:getPixel(xx,yy)
    if v~=0 and x+xx>=0 and x+xx<240 and y+yy>=0 and y+yy<112 then
      local c=p:getColor(v);i:drawPixel(x+xx,y+yy,rgba(c.red,c.green,c.blue))
    end
  end end
  mon:close()
end
local function panel(s,x,y,name,hp,maxhp,status,player,level)
  local i=layer(s,"Text "..name)
  text(i,x,y,name,white,55)
  text(i,x+59,y,"Lv"..(level or 100),white,30)
  local b=layer(s,"HP capsule "..name)
  rect(b,x+16,y+14,64,8,dark)
  rect(b,x+17,y+15,62,1,gray)
  text(b,x+18,y+10,"HP",lime,12)
  local count=math.floor(48*hp/maxhp)
  local c=hp/maxhp>0.5 and rgba(90,213,131) or hp/maxhp>0.2 and rgba(255,230,57) or rgba(255,90,57)
  rect(b,x+32,y+16,48,4,gray)
  if count>0 then rect(b,x+32,y+16,count,3,c) end
  if status then
    rect(b,x,y+15,16,7,rgba(123,65,148))
    text(b,x,y+11,status,white,16)
  end
  if player then
    text(i,x+40,y+21,hp.."/"..maxhp,white,47)
    rect(b,x+16,y+35,64,2,dark)
    if level~=100 then rect(b,x+16,y+35,37,1,blue) end
  end
end
local function board(name,double,state)
  local a=Sprite(240,160,ColorMode.RGB)
  a.layers[1].name="Battlefield context"
  local bg=a.cels[1].image
  rect(bg,0,0,240,64,rgba(90,106,123));rect(bg,0,64,240,48,rgba(156,172,131))
  local mons=layer(a,"Existing Pokemon sprites")
  pokemon(mons,"metang",false,150,8);pokemon(mons,"wailord",true,16,54)
  if double then
    pokemon(mons,"skarmory",false,104,6);pokemon(mons,"skarmory",true,62,60)
    panel(a,14,1,"METANG",99,120,"PSN",false,42)
    panel(a,2,26,"SKARMORY",40,120,"PAR",false,43)
    panel(a,132,61,"WAILORD",521,521,"SLP",false,100)
    panel(a,144,86,"ABCDEFGHIJKL",1,999,"BRN",false,100)
  else
    panel(a,14,12,"METANG",40,120,"PSN",false,42)
    local hp=({521,260,59,0})[state or 1]
    panel(a,134,73,"ABCDEFGHIJKL",hp,521,"SLP",true,state==1 and 71 or 100)
  end
  local commands=layer(a,"Existing command area - unchanged context")
  local shot=assert(app.open(app.params.context))
  local source=shot.cels[1].image
  for y=112,159 do for x=0,239 do commands:drawPixel(x,y,source:getPixel(x,y)) end end
  shot:close()
  save(a,name)
  app.activeSprite=a
  app.command.SpriteSize{ui=false,width=960,height=640,method="nearest-neighbor"}
  a:saveAs(out.."/xy_"..name.."_4x.png")
  a:close()
end
for n=1,4 do board("single_"..n,false,n) end
board("double",true)
print("[ok] XY runtime assets and layered design boards generated")
