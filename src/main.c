#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

#include "utils.h"
#include "gb.h"
#include "console.c"
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
    SDL_Window *window = SDL_CreateWindow("Gameboy", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
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
    bool draw_debug = false;
    while(gb.running) {
        u64 time = SDL_GetPerformanceCounter();
        delta_time = (f32)(time - frame_start) / 10000000.f;
        frame_start = time;
        io.mouse_down = false;
        SDL_Event event;
        while (SDL_PollEvent(&event)) {  
          switch(event.type) {
            case SDL_QUIT : gb.running = false; break;
            case SDL_KEYDOWN : {
               switch(event.key.keysym.scancode) {
               case (SDL_SCANCODE_F5) : {
                    draw_debug = !draw_debug;
                    if(draw_debug)
                        SDL_SetWindowSize(window, 1280, 720);
                    else 
                        SDL_SetWindowSize(window, SCREEN_WIDTH, SCREEN_HEIGHT);
                    
               } break;
               default : gbInput(&gb, &event.key, 0); break;
               }
             } break;
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

        if(draw_debug) {
            gbDrawDebug(&gb, (SDL_Rect){0, 0, SCREEN_WIDTH * zoom, SCREEN_HEIGHT * zoom}, &console, renderer);
            DrawConsole(renderer, &console, SCREEN_WIDTH * zoom, h);
        }

        SDL_RenderPresent(renderer);
    }
    
    free(gb.mem);
    
    SDL_DestroyWindow(window);
    TTF_CloseFont(global_font);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
