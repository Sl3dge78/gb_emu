#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include <SDL/SDL.h>
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

#define assert(expression) __assert(expression, gb)


#define BINARY_FMT(var) var >> 7 & 1, var >> 6 & 1, var >> 5 & 1, var >> 4 & 1, var >> 3 & 1, var >> 2 & 1, var >> 1 & 1, var >> 0 & 1

global bool BREAKPOINT = false;
global bool global_logverbose = true;
global TTF_Font *global_font;

typedef struct IO {
    
    bool mouse_down;
    u32 mouse_x, mouse_y;
    
} IO;

global IO io = {0};

internal void RenderLine(SDL_Renderer *renderer, u32 x, u32 *y, const char *fmt, ...) {
    
    va_list ap;
    va_start(ap, fmt);
    char buf[1024] = {'\0'};
    vsnprintf(buf, 1024, fmt, ap);
    va_end(ap);
    
    SDL_Surface *surf = TTF_RenderText_Blended(global_font, buf, (SDL_Color){200, 200, 200, 255});
    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
    
    SDL_Rect dst = {x, *y, surf->w, surf->h};
    
    SDL_RenderCopy(renderer, tex, NULL, &dst);
    
    SDL_FreeSurface(surf);
    SDL_DestroyTexture(tex);
    *y += dst.h;
}

internal void RenderText(SDL_Renderer *renderer, u32 *x, u32 y, const char *fmt, ...) {
    
    va_list ap;
    va_start(ap, fmt);
    char buf[1024] = {'\0'};
    vsnprintf(buf, 1024, fmt, ap);
    va_end(ap);
    
    SDL_Color col;
    SDL_GetRenderDrawColor(renderer, &col.r, &col.g, &col.b, &col.a);
    SDL_Surface *surf = TTF_RenderText_Blended(global_font, buf, col);
    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
    
    SDL_Rect dst = {*x, y, surf->w, surf->h};
    
    SDL_RenderCopy(renderer, tex, NULL, &dst);
    
    SDL_FreeSurface(surf);
    SDL_DestroyTexture(tex);
    *x += dst.w;
}

#include "console.c"
#include "gb.h"
void __assert(bool value, Gameboy *gb);
#include "opcodes.c"
#include "gb.c"

void __assert(bool value, Gameboy *gb) {
    if (!value){ 
        SDL_Log("Assertion error at %04X", gb->pc); 
        BREAKPOINT = true; 
    }
}

i32 main(i32 argc, char *argv[]) {
    
    SDL_Init(SDL_INIT_EVERYTHING);
    TTF_Init();
    SDL_Window *window = SDL_CreateWindow("Gameboy", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    
    Console console = {0};
    SDL_LogSetOutputFunction(&SDLLogConsole, &console);
    SDL_LogSetPriority(LOG_OPCODE, global_logverbose ? SDL_LOG_PRIORITY_VERBOSE : SDL_LOG_PRIORITY_INFO);
    
    global_font = TTF_OpenFont("resources/font.ttf", 12);
    Gameboy gb = {0};
    gbInit(&gb);
    
    //gbTest(&gb);
    
    gbLoadRom(&gb, "resources/rom.gb");
    
    f32 delta_time = 0;
    u64 frame_start = SDL_GetPerformanceCounter();
    while(gb.running) {
        u64 time = SDL_GetPerformanceCounter();
        delta_time = (f32)(time - frame_start) / 10000000.f;
        frame_start = time;
        io.mouse_down = false;
        SDL_Event event;
        while (SDL_PollEvent(&event)) {  
            switch(event.type) {
                case SDL_QUIT : gb.running = false; break;
                case SDL_KEYDOWN : gbInput(&gb, &event.key, 0); break;
                case SDL_KEYUP   : gbInput(&gb, &event.key, 1); break;
                case SDL_MOUSEBUTTONDOWN : { 
                    if(event.button.button == SDL_BUTTON_LEFT) {
                        io.mouse_down = true;
                        io.mouse_x = event.button.x;
                        io.mouse_y = event.button.y;
                    }
                } break;
                case SDL_MOUSEBUTTONUP : { 
                    if(event.button.button == SDL_BUTTON_LEFT) {
                        io.mouse_x = event.button.x;
                        io.mouse_y = event.button.y;
                    }
                } break;
            }
        }
        
        if(BREAKPOINT) {
            gb.step_through = true;
            BREAKPOINT = false;
        }
        gbLoop(&gb, delta_time);
        i32 w, h;
        i32 zoom = 1;
        SDL_GetWindowSize(window, &w, &h);
        
        SDL_SetRenderDrawColor(renderer, 25, 25, 25, 255);
        SDL_RenderClear(renderer);
        
        gbDraw(&gb, zoom, renderer);
        gbDrawDebug(&gb, (SDL_Rect){0, 0, SCREEN_WIDTH * zoom, SCREEN_HEIGHT * zoom}, &console, renderer);
        
        // Console
        DrawConsole(renderer, &console, SCREEN_WIDTH * zoom, h);
        
        SDL_RenderPresent(renderer);
    }
    
    free(gb.mem);
    
    SDL_DestroyWindow(window);
    TTF_CloseFont(global_font);
    TTF_Quit();
    SDL_Quit();
    return 0;
}