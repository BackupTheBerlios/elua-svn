-- "Piano" in eLua, showing yet another use for the PWM module :)

local pwmid, tmrid

if not math then
  print "This example required floating point Lua (integer Lua not supported)" 
  return
end
  
if pd.board() == "EK-LM3S8962" or pd.board() == "EK-LM3S6965" then
  pwmid, tmrid = 1, 1
elseif pd.board() == "SAM7-EX256" then
  pwmid, tmrid = 0, 1
  tmr.setclock( 1, 1000000 )
else
  print( pd.board() .. " not supported with this example" )
  return
end

local oct, pause = 4, 0

-- Mapping between keys and note offsets
local offsets = { a = 0, w = 1, s = 2, d = 3, r = 4, f = 5, t = 6, g = 7, h = 8,
  u = 9, j = 10, i = 11, k = 12, o = 13, l = 14, [";"] = 15, ["["] = 16, ["'"] = 17,
  ["]"] = 18, ["\\"] = 19 }
  
-- Write the curent octave
function show_octave()
  term.putstrxy( 2, 4, "Oct: " .. tostring( oct ) .. "(+/-)" )
  term.gotoxy( 2, 19 )  
end

-- Write the current pause between notes
function show_pause()
  term.gotoxy( 2, 5 )
  term.clreol()
  term.putstrxy( 2, 5, "Pause between notes: " .. tostring( pause ) .. "ms (</>)" )
  term.gotoxy( 2, 19 )  
end

-- Show the main interface
function show_all()
  term.putstrxy( 2, 2, "eLua piano demo" )  
  show_octave()
  show_pause()
  term.putstrxy( 4,  7, " w   r  t   u  i  o   [  ]  " )
  term.putstrxy( 4,  8, " |   |  |   |  |  |   |  |  " )
  term.putstrxy( 4,  9, " A#  C# D#  F# G# A#  C# D# " )
  term.putstrxy( 4, 10, "A  BC  D  EF  G  A  BC  D  E" )
  term.putstrxy( 4, 11, "|  ||  |  ||  |  |  ||  |  |" )
  term.putstrxy( 4, 12, "a  sd  f  gh  j  k  l;  '  \\" )  
  term.putstrxy( 2, 14, "Use above keys to play notes." )
  term.putstrxy( 2, 15, "+/- to change octave." )
  term.putstrxy( 2, 16, "</> to change pause between notes." )
  term.putstrxy( 2, 17, "Space to stop playing." ) 
  term.putstrxy( 2, 18, "ESC to exit." ) 
  term.gotoxy( 2, 19 )
end

-- Conversion of note to frequency
function note_to_freq( index )
  return 55 * 2 ^ ( ( index + ( oct - 1 ) * 12 ) / 12 )
end

-- Entry point
term.clrscr()
pwm.setclock( pwmid, 1000000 )
show_all()
while true do
  local key = term.getch( term.WAIT )
  if key == term.KC_ESC then break end
  local res, strkey = pcall( string.char, key )
  if res then     
    if offsets[ strkey ] ~= nil then
      local freq = note_to_freq( offsets[ strkey ] )
      pwm.stop( pwmid )
      if pause > 0 then
        tmr.delay( tmrid, pause * 1000 )
      end
      pwm.setup( pwmid, freq, 50 )
      pwm.start( pwmid )
    elseif strkey == ">" then
      pause = pause < 1000 and pause + 10 or pause
      show_pause()
    elseif strkey == "<" then
      pause = pause > 0 and pause - 10 or pause
      show_pause() 
    elseif strkey == "+" then
      oct = oct < 7 and oct + 1 or oct
      show_octave()
    elseif strkey == "-" then
      oct = oct > 1 and oct - 1 or oct
      show_octave()
    elseif strkey == " " then
      pwm.stop( pwmid )
    end
  end  
end

pwm.stop( pwmid )
term.clrscr()
term.gotoxy( 1, 1 )
