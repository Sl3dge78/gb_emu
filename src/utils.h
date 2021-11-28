#pragma once

#include <stdint.h>
#include <SDL/SDL_ttf.h>

#define assert(expression) __assert(expression)

#define BINARY_FMT(var) var >> 7 & 1, var >> 6 & 1, var >> 5 & 1, var >> 4 & 1, var >> 3 & 1, var >> 2 & 1, var >> 1 & 1, var >> 0 & 1

global bool BREAKPOINT = false;
global bool global_logverbose = true;
global TTF_Font *global_font;

typedef struct IO {
    
    bool mouse_down;
    u32 mouse_x, mouse_y;
    
} IO;

global IO io = {0};

typedef struct Gameboy Gameboy;
void __assert(bool value);
