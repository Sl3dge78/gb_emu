#include <string.h>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

#include "gb.h"

typedef struct ConsoleEntry {
    char command[128];
} ConsoleEntry;

#define CONSOLE_HISTORY_SIZE 8

typedef struct Console {
    ConsoleEntry history[CONSOLE_HISTORY_SIZE];
    u32 history_size;
} Console;

void ConsolePushHistory(Console *console, const char *text) {
    // shift everything one down
    for(u32 i = CONSOLE_HISTORY_SIZE - 1; i > 0; i--) {
        memcpy(&console->history[i], &console->history[i - 1], sizeof(ConsoleEntry));
    }
    
    memcpy(&console->history[0].command, text, 128);
    console->history_size++;
    if(console->history_size > CONSOLE_HISTORY_SIZE) {
        console->history_size = CONSOLE_HISTORY_SIZE;
    }
}

void SDLLogConsole(void *userdata, int category, SDL_LogPriority priority, const char *message) {
    if(priority < SDL_LOG_PRIORITY_INFO && !global_logverbose)
        return;
    Console *console = (Console *)userdata;
    ConsolePushHistory(console, message);
}

void DrawConsole(SDL_Renderer *renderer, Console *console, u32 x, u32 bottom) {
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    const i32 line_height = TTF_FontHeight(global_font);
    
    u32 top = bottom - 10 - (CONSOLE_HISTORY_SIZE + 1) * line_height; // History + offset + this text
    RenderLine(renderer, x + 10, &top, "-- Console --");
    
    for(i32 i = 0 ; i < console->history_size; ++i) {
        u32 line_y = (u32) bottom - 10 - (i * line_height) - line_height;
        RenderLine(renderer, x + 10, &line_y, console->history[i].command);
    }
    
    i32 w;
    TTF_SizeText(global_font, "X", &w, 0);
    
    const SDL_Rect rect = {x, top - line_height - 10, w * 64, bottom - top + line_height + line_height};
    SDL_RenderDrawRect(renderer, &rect);
}
