#pragma once

#define SDL_MAIN_HANDLED

#include <iostream>
#include <SDL.h>
#include <easys/easys.hpp>
#include <vector>
#include <random>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <functional>


//global scaling
#define PIXEL_SCALE 4*2 
#define GUMBEL_VALUE 10
//SDL classes
SDL_Window* window;
SDL_Renderer* renderer;

Easys::ECS ecs;