# Dungeon of Altamira

A Dungeon Crawler game.

WARNING: This is still very WIP...

Tested in Linux and Windows, but probably will work on other \*NIX systems
supported by SDL2 (Might need some tweaks on the source code to work on macOS).
Also needs a graphics card with OpenGL 3.3 or GL ES 3.0 support.

## Building

Install the SDL2 libraries from your distro repository, if you're on Windows
download and extract the `SDL2-devel-VERSION-mingw.zip` file from the latest 2.*
release here: <https://github.com/libsdl-org/SDL/releases>, and add the
`SDL2-2.*/YOUR_ARCHITECTURE-w64-mingw32/bin` folder to your path.

You'll also need:

* GNU Make
* GCC or Clang (Other compilers might work if you tweak the flags on the
`Makefile`)

If you're on Windows you can get both with
[w64devkit](https://github.com/skeeto/w64devkit).

To actually build the game, run:

    make

If `sdl2-config` isn't in your path, run:

    make SDL_CONFIG=/PATH/TO/sdl2-config

To force a build with OpenGL ES:

    make GL_ES=1

To use GLVND instead of glad (only on \*NIX):

    make GLVND=1

To run the game, execute `out/dungeon` in the same folder where `assets` is.
