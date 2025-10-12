# Zpace Invaderz (Space Invaders Inspired Game)

Built in Jan. 2025 after about a month of C++ study.

## Context
Learning project to understand low-level game development and C++ fundamentals.
Used ECS library https://github.com/raphaelmayer/easys, shoutout to github.com/raphaelmayer for mentorship!

## What I Implemented
- All game systems (movement, shooting, collision, animation)
- SDL2 integration (rendering, input, audio)
- Game loop timing and state management
- Collision detection algorithms
- Audio system with SDL_mixer --> audio assets removed before publishing repo for licensing reasons.

## Credits

**Third-Party Libraries**:

This project uses the following libraries via CMake `FetchContent`:

- [SDL2](https://github.com/libsdl-org/SDL) (zlib license)
- [SDL2_mixer](https://github.com/libsdl-org/SDL_mixer) (zlib license)
- [SDL2_ttf](https://github.com/libsdl-org/SDL_ttf) (zlib license)
- [EasyS ECS](https://github.com/raphaelmayer/easys) (MIT license)

These libraries are fetched at configure time from their official sources.
All are used unmodified and in compliance with their respective licenses.

**Fonts:** 

Sixtyfour Convergence by Simon Cozens, Jens Kutílek
Licensed under SIL Open Font License 1.1
https://github.com/jenskutilek/homecomputer-fonts

