-- Native-size feasibility boards, not runtime assets. Real Aseprite + game font.
local root=assert(app.params.root)
local out=assert(app.params.out_dir)
assert(app.fs.isDirectory(out))
local font=dofile(root.."/tools/xy_hud_font.lua")
local colors={0x103C42,0x102129,0xF7FFFF,0x18636B,0x39CEC5,0x394A5A,
  0x9CB5C5,0xD6E7EF,0xA57331,0xEFC56B,0x524273,0xADA5DE,
  0x943939,0xEF7373,0x428442,0xA5D66B}
local palette=Palette(16)
for n,c in ipairs(colors) do palette:setColor(n-1,Color{r=(c>>16)&255,g=(c>>8)&255,b=c&255}) end
local function rect(im,x,y,w,h,c)
  for yy=y,y+h-1 do for xx=x,x+w-1 do
    if xx>=0 and xx<im.width and yy>=0 and yy<im.height then im:drawPixel(xx,yy,c) end
  end end
end
-- Original compact symbols for runtime action buttons; not traced artwork.
local icons={
  fight={0x0080,0x0180,0x0380,0x0780,0x0788,0x0F98,0x1FB8,0x1FF8,
         0x3FF8,0x3FFC,0x3EFC,0x3C7C,0x1C78,0x0EF0,0x07E0,0x03C0},
  pokemon={0x0000,0x03C0,0x0FF0,0x1FF8,0x381C,0x700E,0x61C6,0xE3C7,
           0xE3C7,0x61C6,0x700E,0x381C,0x1FF8,0x0FF0,0x03C0,0x0000},
  bag={0x0000,0x03C0,0x0660,0x0660,0x1FF8,0x3FFC,0x300C,0x3FFC,
       0x3FFC,0x318C,0x318C,0x300C,0x3FFC,0x1FF8,0x0000,0x0000},
  run={0x0000,0x0600,0x0F00,0x0F18,0x063C,0x003C,0x0018,0x0700,
       0x0F80,0x0F80,0x070E,0x001F,0x001F,0x000E,0x0000,0x0000},
}
for name,rows in pairs(icons) do
  local icon=Sprite(16,16,ColorMode.INDEXED);icon:setPalette(palette)
  local pixels=icon.cels[1].image;pixels:clear(0)
  for y,row in ipairs(rows) do for x=0,15 do
    if row & (1<<(15-x)) ~= 0 then pixels:drawPixel(x,y-1,name=="fight" and 9 or 1) end
  end end
  icon:saveAs(out.."/sm_icon_"..name..".png")
  icon:saveAs(out.."/sm_icon_"..name..".aseprite");icon:close()
end
local function text(im,x,y,str,fg,width)
  font.draw(im,x,y,str,fg or 2,nil,font.fit(str,width or im.width-x))
end
local function canvas()
  local s=Sprite(240,48,ColorMode.INDEXED);s:setPalette(palette)
  local im=s.cels[1].image;im:clear(0)
  for y=0,47 do for x=0,239 do
    local dx,dy=x-120,y-24;local r=dx*dx+dy*dy
    if (r>225 and r<289) or (r>1764 and r<1936) or ((x+y)%40<2) then im:drawPixel(x,y,3) end
  end end
  return s,im
end
local function card(im,x,y,dark,light,selected)
  for row=1,22 do
    local inset=row<5 and 5-row or row>18 and row-18 or 0
    rect(im,x+2+inset,y+row,116-inset*2,1,selected and 4 or 1)
    if row>1 and row<22 then
      rect(im,x+4+inset,y+row,112-inset*2,1,row<11 and light or dark)
    end
  end
  -- Lower right slashes echo the source's diagonal sweep without covering text.
  for dy=0,5 do rect(im,x+106+dy,y+15+dy,2,1,light) end
  if selected then
    for dy=-3,3 do rect(im,x+1,y+11+dy,5-math.abs(dy),1,2) end
  end
end
local function save(s,name)
  s:saveAs(out.."/"..name..".png");s:saveAs(out.."/"..name..".aseprite");s:close()
end
local s,im=canvas()
local moves={{"Metal Claw","STEEL","32/35",5,7},{"Magnitude","GROUND","19/30",8,9},
  {"Astonish","GHOST","14/15",10,11},{"Bulldoze","GROUND","20/20",8,9}}
for i,m in ipairs(moves) do
  local x=((i-1)%2)*120;local y=math.floor((i-1)/2)*24
  card(im,x,y,m[4],m[5],i==2)
  text(im,x+10,y-1,m[1],1,100)
  rect(im,x+9,y+12,39,9,1)
  text(im,x+11,y+9,m[2],2,35)
  text(im,x+53,y+9,"PP "..m[3],2,54)
end
save(s,"moves")
s,im=canvas()
local function button(x,y,w,h,dark,light,label,selected)
  for row=1,h-2 do
    local inset=row<4 and 4-row or row>h-5 and row-(h-5) or 0
    rect(im,x+inset,y+row,w-inset*2,1,selected and 2 or 4)
    if row>2 and row<h-3 then rect(im,x+2+inset,y+row,w-4-inset*2,1,row<h/2 and light or dark) end
  end
  text(im,x+math.floor((w-font.width(label))/2),y+math.floor(h/2)-6,label,1,w-8)
end
button(0,0,72,24,14,15,"POKEMON",false)
button(0,24,72,24,8,9,"BAG",false)
button(178,0,62,48,12,13,"FIGHT",true)
button(80,29,90,19,5,6,"RUN",false)
for _,center in ipairs({{103,18},{147,9}}) do
  for dy=-7,7 do for dx=-12,12 do
    local r=dx*dx+dy*dy*3
    if r>85 and r<145 then rect(im,center[1]+dx,center[2]+dy,1,1,4) end
  end end
end
for dy=-3,3 do rect(im,174,24+dy,5-math.abs(dy),1,2) end
save(s,"actions")
s,im=canvas()
for row=3,44 do
  local inset=row<8 and 8-row or row>39 and row-39 or 0
  rect(im,4+inset,row,232-inset*2,1,4)
  if row>4 and row<43 then rect(im,6+inset,row,228-inset*2,1,1) end
end
text(im,14,8,"Buffie used",2,212)
text(im,14,23,"Mirror Coat!",2,212)
for d=0,5 do rect(im,216+d,35-d,2,6,4) end
save(s,"message")

-- Party selection is a separate 240x160 menu, not six cards crushed into 48px.
s=Sprite(240,160,ColorMode.INDEXED);s:setPalette(palette);im=s.cels[1].image
im:clear(14)
local party={{"Diglett",18,25,36},{"Magnemite",19,26,38},{"Dartrix",20,65,65},
  {"Machop",17,28,52},{"Grimer",18,0,46},{"Growlithe",18,0,45}}
for i,m in ipairs(party) do
  local x=((i-1)%2)*120;local y=math.floor((i-1)/2)*44
  local edge=m[3]==0 and 12 or 14;local tint=m[3]==0 and 13 or 15
  for row=2,41 do
    local inset=row<8 and 8-row or row>35 and row-35 or 0
    rect(im,x+2+inset,y+row,116-inset*2,1,i==4 and 4 or 1)
    if row>3 and row<40 then rect(im,x+4+inset,y+row,112-inset*2,1,row<23 and 2 or tint) end
  end
  rect(im,x+8,y+8,3,25,edge)
  text(im,x+16,y+4,m[3]==0 and "FAINTED" or "Lv"..m[2],edge,96)
  text(im,x+16,y+15,m[1],1,96)
  rect(im,x+17,y+31,46,6,1)
  rect(im,x+18,y+32,math.floor(44*m[3]/m[4]),4,14)
  text(im,x+69,y+27,m[3].."/"..m[4],1,45)
end
rect(im,4,136,232,20,1);text(im,12,139,"Choose a Pokemon.  B: Back",2,216)
save(s,"party")

s=Sprite(240,80,ColorMode.INDEXED);s:setPalette(palette);im=s.cels[1].image
im:clear(1)
for y=2,77 do
  local inset=y<7 and 7-y or y>72 and y-72 or 0
  rect(im,2+inset,y,236-inset*2,1,6)
  if y>3 and y<76 then rect(im,4+inset,y,232-inset*2,1,y<20 and 6 or 5) end
end
text(im,12,5,"Metal Claw",1,144);text(im,170,5,"PHYSICAL",1,60)
text(im,12,22,"STEEL   PP 32/35",2,125)
text(im,12,35,"Power 50    Accuracy 95",2,216)
text(im,12,49,"Rakes the target with steel claws.",2,216)
text(im,12,61,"May also raise Attack.",2,216)
save(s,"details")
