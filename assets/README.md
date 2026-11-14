# Assets

Everything in this directory is compiled into the executable as a
[GResource](https://docs.gtk.org/gio/struct.Resource.html) bundle
(see [`src/ui/CMakeLists.txt`](../src/ui/CMakeLists.txt)), so the game runs from any directory.

| File                              | Used for                                   |
| --------------------------------- | ------------------------------------------ |
| `style.css`                       | Appearance of every widget                 |
| `images/background-menu.jpg`      | Menu page: the horizon                     |
| `images/background-game.jpg`      | Game page: a sea chart                     |
| `images/background-rules.jpg`     | Rules window: squared notebook paper       |
| `images/background-trailer.jpg`   | Trailer window: a pale sky over a swell    |
| `images/versus.png`               | Illustration between the two boards        |
| `images/ship.png`                 | An intact deck                             |
| `images/ship-sunk-player.png`     | A deck of a sunk ship of the player        |
| `images/ship-sunk-enemy.png`      | A deck of a sunk pirate ship               |
| `images/explosion.png`            | A hit                                      |
| `images/miss.png`, `flag.png`     | A shot in the water; "no ship can be here" |
| `images/rocket.png`               | Keyboard cursor                            |
| `sounds/hit.wav`, `sounds/miss.wav` | Sound effects (builds with GStreamer only) |

## Where they come from

Every image and sound is **original work of this project**, under the same
[MIT licence](../LICENSE) as the code. None is downloaded, traced or sampled from an existing
picture or recording: the images are drawn with ImageMagick primitives and the sounds are
synthesised with ffmpeg, by one script.

```bash
make assets          # runs tools/generate-assets.sh
```

Random textures use fixed seeds, so with a given version of ImageMagick the script reproduces
the same files. Change a colour or a shape in the script, run it, rebuild.

## History

Until version 2.0.0 the game used artwork and sounds gathered in 2023 for the original school
project, whose sources had not been recorded. Version 2.1.0 replaced all of them. They remain
visible in the Git history, in the files attached to the `v1.0.0` and `v2.0.0` releases and in
the screenshots of the 2023 version kept under `docs/screenshots/v1/`.
