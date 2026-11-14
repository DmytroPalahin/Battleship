# Assets

Everything in this directory is compiled into the executable as a
[GResource](https://docs.gtk.org/gio/struct.Resource.html) bundle
(see [`src/ui/CMakeLists.txt`](../src/ui/CMakeLists.txt)), so the game runs from any directory.

| File                               | Used for                                   | Origin               |
| ---------------------------------- | ------------------------------------------ | -------------------- |
| `style.css`                        | Appearance of every widget                 | This project (MIT)   |
| `images/background-menu.jpg`       | Menu page                                  | This project (MIT)   |
| `images/flag.png`, `miss.png`      | "No ship here" and "missed" markers        | This project (MIT)   |
| `images/background-game.jpg`       | Game page                                  | Third party          |
| `images/background-rules.jpg`      | Rules window                               | Third party          |
| `images/background-trailer.jpg`    | Trailer window                             | Third party          |
| `images/versus.png`                | Illustration between the boards            | Third party, collage |
| `images/ship*.png`, `rocket.png`   | Ship decks, keyboard cursor                | Third party          |
| `images/explosion.png`             | Hit marker                                 | Third party          |
| `sounds/hit.wav`, `sounds/miss.wav`| Sound effects (builds with GStreamer only) | Third party          |

## Licence notice

The MIT licence of this repository covers the source code and the files marked
"This project". It does **not** cover the third-party artwork and sounds.

Those files were gathered in 2023 for a non-commercial school project and their
individual sources were not recorded. They remain the property of their
respective owners and are kept here for educational, non-commercial use only.
The *Sup Galilée* name and logo belong to Institut Galilée, Université Sorbonne
Paris Nord.

If you own one of these files and want it credited or removed, please open an
issue. If you want to redistribute the game, replace them first: the code only
refers to them through the class names of `style.css` and the resource list of
`src/ui/battleship.gresource.xml.in`.
