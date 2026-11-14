#!/usr/bin/env bash
# Regenerates every image and sound of the game from scratch.
#
#   tools/generate-assets.sh [output-directory]      (default: assets)
#
# Nothing here is downloaded or traced from an existing picture: each file is
# drawn with ImageMagick primitives or synthesised with ffmpeg, so the whole
# set is original work under the licence of the repository. Random textures
# use fixed seeds: with a given version of ImageMagick, two runs give the same
# pictures.
#
# Requires: ImageMagick 7 (magick), ffmpeg, pngquant, python3.
set -euo pipefail

out="${1:-assets}"
img="$out/images"
snd="$out/sounds"
mkdir -p "$img" "$snd"
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT

# Palette (Open Color, MIT).
navy='#0b3954';  sea='#1c7ed6';  sky='#4dabf7';  foam='#e7f5ff'
steel='#495057'; grey='#868e96'; light='#ced4da'; white='#f8f9fa'
red='#e03131';   orange='#f76707'; yellow='#ffd43b'; black='#212529'

shrink() { magick "$1" -resize "$2" -strip "$3"; pngquant --quality 80-98 --strip --force --output "$3" "$3" 2>/dev/null || true; }

# ------------------------------------------------------------------ sprites --
# Drawn four times larger than needed, then reduced: cheap anti-aliasing.

# A warship seen from the side, on a tile of sea.
# $1: output file; the remaining arguments draw the flag.
ship() {
  local file="$1"
  shift
  magick -size 176x144 xc:none \
    -fill "$sky" -draw 'roundrectangle 0,0 175,143 14,14' \
    -fill "$sea" -draw 'roundrectangle 0,108 175,143 14,14' -draw 'rectangle 0,108 175,124' \
    -fill "$foam" -draw 'rectangle 0,106 175,110' \
    -fill "$steel" -draw 'polygon 14,80 164,80 148,112 30,112' \
    -fill "$grey" -draw 'rectangle 54,58 124,80' \
    -fill "$light" -draw 'rectangle 70,40 104,58' \
    -fill "$steel" -draw 'rectangle 132,70 158,76' -draw 'rectangle 126,66 136,80' \
    -fill "$navy" -draw 'rectangle 76,46 82,52' -draw 'rectangle 90,46 96,52' \
    -stroke "$steel" -strokewidth 4 -draw 'line 88,12 88,40' -stroke none \
    "$@" \
    "$tmp/ship.png"
  shrink "$tmp/ship.png" 44x36 "$file"
}
tricolour=(-fill '#1864ab' -draw 'rectangle 90,12 101,30'
           -fill "$white" -draw 'rectangle 101,12 112,30'
           -fill "$red" -draw 'rectangle 112,12 123,30')
jolly=(-fill "$black" -draw 'rectangle 90,12 124,32'
       -fill "$white" -draw 'circle 107,20 107,25' -draw 'rectangle 99,27 115,29')

ship "$img/ship.png" "${tricolour[@]}"

# The same ship crossed out, once per camp.
sunk() {
  local file="$1"
  shift
  ship "$tmp/unused.png" "$@"
  magick "$tmp/ship.png" -stroke "$red" -strokewidth 16 \
    -draw 'line 18,14 158,130' -draw 'line 158,14 18,130' "$tmp/sunk.png"
  shrink "$tmp/sunk.png" 44x36 "$file"
}
sunk "$img/ship-sunk-player.png" "${tricolour[@]}"
sunk "$img/ship-sunk-enemy.png" "${jolly[@]}"

# An explosion: three nested stars.
star() { # $1 centre, $2 outer radius, $3 inner radius, $4 points, $5 rotation (degrees)
  python3 - "$@" <<'PY'
import math, sys
c, outer, inner, n, rot = (float(v) for v in sys.argv[1:6])
pts = []
for i in range(int(n) * 2):
    r = outer if i % 2 == 0 else inner
    a = math.radians(rot) + math.pi * i / n
    pts.append(f"{c + r * math.sin(a):.1f},{c - r * math.cos(a):.1f}")
print(" ".join(pts))
PY
}
magick -size 176x176 xc:none \
  -fill "$red"    -draw "polygon $(star 88 86 46 9 0)" \
  -fill "$orange" -draw "polygon $(star 88 66 34 9 20)" \
  -fill "$yellow" -draw "polygon $(star 88 44 22 9 0)" \
  -fill "$white"  -draw 'circle 88,88 88,102' \
  "$tmp/explosion.png"
shrink "$tmp/explosion.png" 44x44 "$img/explosion.png"

# A missile pointing up and to the right: the keyboard cursor.
magick -size 176x176 xc:none \
  -fill "$red"   -draw 'polygon 88,10 108,48 68,48' \
  -fill "$white" -draw 'rectangle 68,48 108,128' \
  -fill "$light" -draw 'rectangle 96,48 108,128' \
  -fill "$steel" -draw 'rectangle 68,74 108,82' \
  -fill "$red"   -draw 'polygon 68,104 44,144 68,132' -draw 'polygon 108,104 132,144 108,132' \
  -fill "$orange" -draw 'polygon 74,128 102,128 88,164' \
  -background none -rotate 45 -gravity center -extent 176x176 \
  "$tmp/rocket.png"
shrink "$tmp/rocket.png" 44x44 "$img/rocket.png"

# A shot in the water and the "no ship here" pennant.
magick -size 176x176 xc:none -fill '#1f2933' -draw 'ellipse 88,96 34,13 0,360' \
  -fill '#52606d' -draw 'ellipse 88,92 26,7 0,360' "$tmp/miss.png"
shrink "$tmp/miss.png" 44x44 "$img/miss.png"
magick -size 176x176 xc:none -stroke '#3e4c59' -strokewidth 10 -draw 'line 62,40 62,144' \
  -stroke none -fill '#e12d39' -draw 'polygon 68,40 132,66 68,92' "$tmp/flag.png"
shrink "$tmp/flag.png" 44x44 "$img/flag.png"

# -------------------------------------------------------------- backgrounds --

# Draw commands for a square grid: $1 width, $2 height, $3 step.
grid() {
  local w="$1" h="$2" step="$3" x y cmds=""
  for ((x = step; x < w; x += step)); do cmds+="line $x,0 $x,$h "; done
  for ((y = step; y < h; y += step)); do cmds+="line 0,$y $w,$y "; done
  printf '%s' "$cmds"
}

# Draw commands for a compass rose: $1 x, $2 y, $3 radius.
compass() {
  python3 - "$@" <<'PY'
import math, sys
x, y, r = (float(v) for v in sys.argv[1:4])
out = [f"circle {x},{y} {x},{y - r}", f"circle {x},{y} {x},{y - r * 0.78}"]
for i in range(8):                      # eight points, the cardinal ones longer
    a = math.pi * i / 4
    tip = r * (0.98 if i % 2 == 0 else 0.6)
    left, right = a - 0.16, a + 0.16
    out.append("polygon "
               f"{x + tip * math.sin(a):.1f},{y - tip * math.cos(a):.1f} "
               f"{x + r * 0.2 * math.sin(left):.1f},{y - r * 0.2 * math.cos(left):.1f} "
               f"{x:.1f},{y:.1f} "
               f"{x + r * 0.2 * math.sin(right):.1f},{y - r * 0.2 * math.cos(right):.1f}")
print(" ".join(out))
PY
}

# Game page: a sea chart. Water texture, graticule, two compass roses.
magick -size 1280x850 -seed 7 plasma:fractal -colorspace Gray -blur 0x5 -normalize \
  -evaluate multiply 0.36 -evaluate add 32% "$tmp/noise.png"
magick -size 1280x850 gradient:'#4a9ad0-#1d5a88' "$tmp/noise.png" -compose Overlay -composite \
  -fill none -stroke 'rgba(255,255,255,0.09)' -strokewidth 1 -draw "$(grid 1280 850 85)" \
  -stroke 'rgba(255,255,255,0.16)' -strokewidth 2 -fill 'rgba(255,255,255,0.05)' \
  -draw "$(compass 190 660 150)" -draw "$(compass 1120 180 95)" \
  -strip -quality 86 "$img/background-game.jpg"

# Rules window: squared notebook paper, where the game was first played.
magick -size 1280x720 xc:'#fbfcfe' \
  -fill none -stroke 'rgba(77,171,247,0.30)' -strokewidth 1 -draw "$(grid 1280 720 32)" \
  -stroke 'rgba(224,49,49,0.35)' -strokewidth 2 -draw 'line 96,0 96,720' \
  -stroke 'rgba(28,126,214,0.16)' -strokewidth 3 -fill 'rgba(28,126,214,0.05)' \
  -draw 'polygon 360,520 920,520 860,620 420,620' \
  -draw 'rectangle 520,440 780,520' -draw 'rectangle 590,370 710,440' \
  -draw 'line 650,250 650,370' -draw 'polygon 656,256 740,286 656,316' \
  -strip -quality 88 "$img/background-rules.jpg"

# Trailer window: a pale sky over a swell, light enough for black text.
waves() { # polygon closing the bottom of the picture under a sine wave
  python3 - "$@" <<'PY'
import math, sys
base, amp, length, phase = (float(v) for v in sys.argv[1:5])
pts = [f"{x},{base + amp * math.sin(x / length + phase):.1f}" for x in range(0, 1281, 16)]
print("polygon 0,720 " + " ".join(pts) + " 1280,720")
PY
}
magick -size 1280x720 -seed 11 plasma:fractal -colorspace Gray -blur 0x14 -normalize \
  -evaluate multiply 0.22 -evaluate add 39% "$tmp/clouds.png"
magick -size 1280x720 gradient:'#eaf2f8-#c5d8e6' "$tmp/clouds.png" -compose Overlay -composite \
  -stroke none \
  -fill 'rgba(116,163,196,0.30)' -draw "$(waves 500 16 70 0.0)" \
  -fill 'rgba(92,146,184,0.30)'  -draw "$(waves 560 20 85 1.3)" \
  -fill 'rgba(72,128,170,0.30)'  -draw "$(waves 630 24 100 2.6)" \
  -strip -quality 88 "$img/background-trailer.jpg"

# Menu page: the horizon.
magick -size 1600x420 gradient:'#4a8fd6-#dcefff' "$tmp/sky.png"
magick -size 1600x420 xc:none -fill 'rgba(255,255,255,0.85)' \
  -draw 'ellipse 1150,430 520,60 0,360' -blur 0x50 "$tmp/glow.png"
magick -size 160x480 -seed 2023 xc:gray50 +noise Gaussian -colorspace Gray \
  -resize '1600x480!' -blur 0x1 -normalize -evaluate multiply 0.30 -evaluate add 35% \
  "$tmp/ripples.png"
magick -size 1600x480 gradient:'#3f8fb5-#0a3550' "$tmp/ripples.png" \
  -compose Overlay -composite "$tmp/water.png"
magick "$tmp/sky.png" "$tmp/glow.png" -composite "$tmp/water.png" -append \
  -strip -quality 88 "$img/background-menu.jpg"

# ------------------------------------------------------------- illustration --
# The two fleets face to face: the navy above, the pirates below.

rounded() { # clips $1 to a rounded rectangle, in place
  magick "$1" \( +clone -alpha transparent -fill white \
    -draw 'roundrectangle 0,0 499,299 26,26' \) -compose DstIn -composite "$1"
}

magick -size 500x300 gradient:'#74c0fc-#e7f5ff' \
  -fill '#fff3bf' -draw 'circle 410,70 410,104' \
  -fill "$sea" -draw 'rectangle 0,226 499,299' -fill "$foam" -draw 'rectangle 0,222 499,228' \
  -fill "$steel" -draw 'polygon 50,170 450,170 410,236 96,236' \
  -fill "$grey" -draw 'rectangle 160,118 340,170' \
  -fill "$light" -draw 'rectangle 205,78 295,118' \
  -fill "$navy" -draw 'rectangle 218,90 234,104' -draw 'rectangle 242,90 258,104' \
                -draw 'rectangle 266,90 282,104' \
  -fill "$steel" -draw 'rectangle 356,148 426,160' -draw 'rectangle 344,138 372,170' \
                 -draw 'rectangle 82,150 130,160' -draw 'rectangle 120,140 146,170' \
  -stroke "$steel" -strokewidth 6 -draw 'line 250,22 250,78' -stroke none \
  -fill '#1864ab' -draw 'rectangle 254,22 276,56' -fill "$white" -draw 'rectangle 276,22 298,56' \
  -fill "$red" -draw 'rectangle 298,22 320,56' \
  "$tmp/navy.png"
rounded "$tmp/navy.png"

magick -size 500x300 gradient:'#495057-#adb5bd' \
  -fill '#f1f3f5' -draw 'circle 92,70 92,100' -fill '#495057' -draw 'circle 104,62 104,88' \
  -fill "$navy" -draw 'rectangle 0,226 499,299' -fill '#5c7cfa' -draw 'rectangle 0,222 499,227' \
  -fill '#5c3d1e' -draw 'polygon 60,176 440,176 404,236 104,236' \
  -fill '#7a5230' -draw 'rectangle 60,170 440,180' \
  -stroke '#3b2a1a' -strokewidth 7 -draw 'line 178,44 178,176' -draw 'line 312,24 312,176' \
  -stroke none -fill "$black" \
  -draw 'polygon 122,60 234,60 246,150 110,150' -draw 'polygon 244,40 380,40 394,158 230,158' \
  -draw 'polygon 316,22 372,34 316,46' \
  -fill "$white" -draw 'circle 312,88 312,108' -draw 'rectangle 300,104 324,116' \
  -fill "$black" -draw 'circle 304,86 304,91' -draw 'circle 320,86 320,91' \
  -stroke "$white" -strokewidth 6 -draw 'line 280,118 344,146' -draw 'line 344,118 280,146' \
  "$tmp/pirates.png"
rounded "$tmp/pirates.png"

magick -size 560x720 xc:none -fill '#f1f3f5' -draw 'roundrectangle 0,0 559,719 34,34' \
  "$tmp/navy.png" -geometry +30+34 -composite \
  "$tmp/pirates.png" -geometry +30+386 -composite \
  -fill "$black" -draw 'polygon 318,296 232,372 276,372 238,428 336,346 290,346' \
  -fill "$yellow" -draw 'polygon 310,308 248,364 286,364 256,410 320,354 280,354' \
  "$tmp/versus.png"
shrink "$tmp/versus.png" 560x720 "$img/versus.png"

# ------------------------------------------------------------------- sounds --
# A hit: a low thump under a burst of filtered noise.
ffmpeg -loglevel error -y \
  -f lavfi -i 'anoisesrc=color=brown:duration=0.9:amplitude=0.9:seed=1' \
  -f lavfi -i 'sine=frequency=62:duration=0.9' \
  -filter_complex "[0]lowpass=f=1100,afade=t=out:st=0.03:d=0.85:curve=exp[noise];\
[1]afade=t=out:st=0:d=0.55:curve=exp,volume=1.4[thump];\
[noise][thump]amix=inputs=2:normalize=0,alimiter=limit=0.9" \
  -ar 44100 -ac 1 -c:a pcm_s16le "$snd/hit.wav"

# A miss: a short splash.
ffmpeg -loglevel error -y \
  -f lavfi -i 'anoisesrc=color=white:duration=0.6:amplitude=0.6:seed=2' \
  -filter_complex "[0]highpass=f=900,lowpass=f=5200,afade=t=in:st=0:d=0.015,\
afade=t=out:st=0.04:d=0.55:curve=exp,alimiter=limit=0.8" \
  -ar 44100 -ac 1 -c:a pcm_s16le "$snd/miss.wav"

echo "assets written to $out"
