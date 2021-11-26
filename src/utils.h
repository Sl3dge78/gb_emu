#pragma once

#include <stdint.h>
#include <SDL/SDL_ttf.h>

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint32_t u64;

typedef float f32;
typedef u8 bool;
#define true 1
#define false 0

#define internal static
#define global static

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
