#include <stdio.h>
#include "utils.h"
#include "gb.h"

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

void StackPush(Stack *stack, u16 from, u16 to) {
    for(i32 i = stack->capacity; i > 0 ; i--) {
        stack->from[i] = stack->from[i-1];
        stack->to[i] = stack->to[i-1];
    }
    
    stack->from[0] = from;
    stack->to[0] = to;
}

typedef struct OAMSprite {
    u8 y;
    u8 x;
    u8 tile;
    u8 flags;
} OAMSprite;

typedef struct TileLine {
    u8 data_1;
    u8 data_2;
} TileLine;

internal void PrintHex(u8 *ptr, u32 nb) {
    u32 l = 0;
    
    for(u32 i = 0; i < nb; ++i) {
        SDL_Log("%02X ", *ptr++);
        l++;
        if(l >= 16 && i != nb - 1) {
            SDL_Log("\n");
            l = 0;
        }
    }
}

internal void PrintAscii(u8 *ptr, u32 nb) {
    u32 l = 0;
    
    for(u32 i = 0; i < nb; ++i) {
        SDL_Log(" %c ", *ptr++);
        l++;
        if(l >= 16 && i != nb - 1) {
            SDL_Log("\n");
            l = 0;
        }
    }
}

void DumpRomFormatted(u8 *start) {
    
    SDL_Log("Entry:\n");          PrintHex(start + ROM_ENTRY, 4); SDL_Log("\n");
    SDL_Log("Logo:\n");           PrintHex(start + ROM_LOGO, ROM_LOGO_SIZE); SDL_Log("\n");
    SDL_Log("Title:\n");          PrintHex(start + ROM_TITLE, 16); SDL_Log("\n"); 
    PrintAscii(start + ROM_TITLE, 16); SDL_Log("\n\n");
    
    SDL_Log("Manufacturer:   "); PrintHex(start + ROM_MANUF, 2); SDL_Log("\n");
    SDL_Log("CGB Flag:       "); PrintHex(start + ROM_CGB, 1); SDL_Log("\n");
    SDL_Log("Licensee:       "); PrintHex(start + ROM_LICENSEE, 2); SDL_Log("\n");
    SDL_Log("SGB Flag:       "); PrintHex(start + ROM_SGB, 1); SDL_Log("\n");
    SDL_Log("Cartridge type: "); PrintHex(start + ROM_CARTRIDGE, 1); SDL_Log("\n");
    SDL_Log("ROM Size:       "); PrintHex(start + ROM_ROM_SIZE, 1); SDL_Log("\n");
    SDL_Log("RAM Size:       "); PrintHex(start + ROM_RAM_SIZE, 1); SDL_Log("\n");
    SDL_Log("Destination:    "); PrintHex(start + ROM_DEST_CODE, 1); SDL_Log("\n");
    SDL_Log("Old Licensee:   "); PrintHex(start + ROM_OLD_LICEN, 1); SDL_Log("\n");
    SDL_Log("Rom Version:    "); PrintHex(start + ROM_ROM_VER, 1); SDL_Log("\n");
    SDL_Log("Header Checksum:"); PrintHex(start + ROM_HDR_CHKSM, 1); SDL_Log("\n");
    SDL_Log("Global Checksum:"); PrintHex(start + ROM_GBL_CHKSM, 2); SDL_Log("\n");
}

void gbBreakpoint(Gameboy *gb) {
    gb->step_through = true;
    gb->cycles_left = -1;
}

void gbWriteAt(Gameboy *gb, u16 address, u8 value, bool external) {
    if(external)
        gb->last_write = address;
   
    if(address >= MEM_ROM_START && address <= MEM_ROM_END) {
        RomWrite(&gb->rom, address, value);
        return;
    }
    if(address >= MEM_CARTRAM_START && address <= MEM_CARTRAM_END) {
        RomWrite(&gb->rom, address, value);
        return;
    }
    
    switch(address){
        case (IO_DMA) : {
            gb->DMA_cycles_left = 0x9F; // DMA transfer
        } break;
        case (IO_IF)   : value |= 0xE0; break;
        case (IO_TAC)  : value |= 0xF8; break;
        case (IO_DIV)  : if(external) gb->timer = 0; break;
        case (IO_NR52) : {
            bool toggle = !((value >> 7) & 1);
            SDL_PauseAudioDevice(gb->apu.audio_device, toggle);
            if(external) {
                value &= 0x80;
            }
        } break;

        case (IO_NR14) : {
            bool is_playing = (value & 0x80) != 0;
            if(is_playing) {
                EnveloppeInit(gb, &gb->apu.enveloppes[0], 0);
                u8 NR10 = gbReadAt(gb, IO_NR10, 0);
                u8 NR11 = gbReadAt(gb, IO_NR11, 0);
                gb->apu.channel1.pitch = gbReadAt(gb, IO_NR13, 0);
                gb->apu.channel1.pitch |= (value & 0x07) << 8;
                gb->apu.channel1.sweep_timer = (1.0f/128.0f);
                u8 time = (NR10 >> 4) & 0b111;
                gb->apu.channel1.sweep_period = time == 0 ? 8 : time;
                gb->apu.channel1.duty = (NR11 & 0xC0) >> 6;
            }
        } break;
    
        case (IO_NR24) : {
            bool is_playing = (value & 0x80) != 0;
            if(is_playing) {
                EnveloppeInit(gb, &gb->apu.enveloppes[1], 1);
                u8 NR21 = gbReadAt(gb, IO_NR21, 0);
                gb->apu.channel2.pitch = gbReadAt(gb, IO_NR23, 0);
                gb->apu.channel2.pitch |= (value & 0x07) << 8;
                gb->apu.channel2.duty = (NR21 & 0xC0) >> 6;
            }
        } break;
        case (IO_NR30): {
            gb->apu.is_playing[2] = (value >> 7) & 1;
        } break;
        case (IO_NR32): {
            gb->apu.enveloppes[2].volume = (value >> 5) & 3;
        } break;
        case (IO_NR34): {
            bool is_playing = (value & 0x80) != 0;
            if(is_playing) {
                EnveloppeInit(gb, &gb->apu.enveloppes[2], 2);
                u8 NR31 = gbReadAt(gb, IO_NR31, 0);
                gb->apu.channel3.pitch = gbReadAt(gb, IO_NR33, 0);
                gb->apu.channel3.pitch |= (value & 0x07) << 8;
            }
        } break;
        // Channel 4
        case (IO_NR42): {
            gb->apu.enveloppes[3].volume = (value >> 4) & 0xF;
        } break;
        case (IO_NR44) : {
            bool is_playing = (value & 0x80) != 0;
            if(is_playing) {
                EnveloppeInit(gb, &gb->apu.enveloppes[3], 3);
            }
        } break;
    }
    
    if(address >= IO_WAV && address <= IO_WAV_END) {
        u32 wave_id = (address - IO_WAV) * 2;
        gb->apu.wave[wave_id] = (value & 0xF0) >> 4;
        gb->apu.wave[wave_id + 1] = value & 0x0F;
    }

    if(address >= MEM_MIRROR0_START && address <= MEM_MIRROR1_END) {
        address -= MEM_MIRROR0_START;
    }
    
    gb->mem[address] = value;
    return;
}

u8 gbReadAt(Gameboy *gb, u16 address, bool log) {
    if(log)
        gb->last_read = address;
    
     if(address >= MEM_ROM_START && address <= MEM_ROM_END) {
        return RomRead(&gb->rom, address);
    }
    if(address >= MEM_CARTRAM_START && address <= MEM_CARTRAM_END) {
        return RomRead(&gb->rom, address);
    }

    if(address >= MEM_MIRROR0_START && address <= MEM_MIRROR1_END) {
        address -= MEM_MIRROR0_START;
    }
    
    return gb->mem[address];
}

void gbInterrupt(Gameboy *gb, u8 id) {
    assert(id <= INT_JOYPAD);
    gb->halted = false;
    u8 IF = gbRead(gb, IO_IF);
    IF |= 1 << id;
    IF |= 0xE0;
    gbWrite(gb, IO_IF, IF);
    SDL_LogVerbose(0, "Triggering interrupt %d", id);
}

// ---------
// Lifecycle


void gbLoadRom(Gameboy *gb, const char *path) {
    RomLoad(&gb->rom, path);
    gb->rom_loaded = true;
}

void gbReset(Gameboy *gb) {
    gb->pc    = ROM_ENTRY;
    gb->sp    = 0xFFFE;
    gb->a     = 0;
    gb->b     = 0;
    gb->c     = 0;
    gb->d     = 0;
    gb->e     = 0;
    gb->f     = 0;
    gb->h     = 0;
    gb->l     = 0;
    gb->ime   = 0;
    gbWrite(gb, IO_LCDC, 0x91);
    gbWrite(gb, IO_STAT, 0x85);
    gbWrite(gb, IO_IF, 0xE1); 
    gbWrite(gb, IO_IE, 0x00); 
    gbWrite(gb, IO_TAC, 0x00); 
    gbWrite(gb, 0x2000, 0x01); // Select rom bank 1

    RomReset(&gb->rom);
}

void gbInit(Gameboy *gb) {
    gb->clock_speed = 4194304;
    gb->clock_mul = 1.0f;
    gb->cpu_clock = 1;
    gb->ppu_clock = 1;
    gb->cycles_left = 0;
    gb->running = true;
    gb->step_through = 0;
    gb->mem = calloc(1, MEM_SIZE);
    gb->keys_dpad = 0xFF;
    gb->keys_buttons = 0xFF;
    
    gb->call_stack.capacity = 8;
    gb->call_stack.from = calloc(sizeof(u32), gb->call_stack.capacity);
    gb->call_stack.to = calloc(sizeof(u32), gb->call_stack.capacity);
    
    FILE *file = fopen("bp.txt", "r+");
    i32 i = 0;
    if(file) {
        while(i < 16){
            i32 result = fscanf(file, "%4hX\n", &gb->breakpoints[i]);
            if(result == 0 || result == EOF)
                break;
            i++;
            
        }
        gb->breakpoint_count = i;
        fclose(file);
    } 
   
    gbInitAudio(gb, &gb->apu);
    gb->rom = (Rom){0};
    gbReset(gb);
}

void gbInput(Gameboy *gb, SDL_KeyboardEvent *e, bool is_up) {
    if(!is_up) {
        switch(e->keysym.scancode) {
            case(SDL_SCANCODE_SPACE) : case(SDL_SCANCODE_F9) : {
                gb->step_through = !gb->step_through; 
            } break;
            case(SDL_SCANCODE_F7) :
            case(SDL_SCANCODE_N)  : gb->cycles_left = gb->cpu_clock <= 0 ? 1 : gb->cpu_clock; break;
            case(SDL_SCANCODE_R)  : gbReset(gb); SDL_Log("Reset", global_logverbose); break;
            case(SDL_SCANCODE_V)  : { 
                global_logverbose = !global_logverbose; 
                SDL_LogSetPriority(LOG_OPCODE, global_logverbose ? SDL_LOG_PRIORITY_VERBOSE : SDL_LOG_PRIORITY_INFO);
                SDL_Log("Verbose = %d", global_logverbose); 
            } break;
            case(SDL_SCANCODE_1)     : { 
                SDL_Log("Triggering V-Blank Interrupt");
                gbInterrupt(gb, INT_VBLANK);
            } break;
            case(SDL_SCANCODE_KP_PLUS) : { 
                gb->clock_mul *= 2.0f;
            } break;
            case(SDL_SCANCODE_KP_MINUS) : { 
                gb->clock_mul /= 2.0f;
            } break;
            
            default: break; 
        }
    }
    
    switch(e->keysym.scancode) {
        case(SDL_SCANCODE_DOWN) : gb->keys_dpad = (gb->keys_dpad & (0b11110111)) | (is_up << 3);  break;
        case(SDL_SCANCODE_UP)   : gb->keys_dpad = (gb->keys_dpad & (0b11111011)) | (is_up << 2);  break;
        case(SDL_SCANCODE_LEFT) : gb->keys_dpad = (gb->keys_dpad & (0b11111101)) | (is_up << 1);  break;
        case(SDL_SCANCODE_RIGHT): gb->keys_dpad = (gb->keys_dpad & (0b11111110)) | (is_up << 0);  break;
        case(SDL_SCANCODE_X) : gb->keys_buttons = (gb->keys_buttons & (0b11110111)) | (is_up << 3);  break;
        case(SDL_SCANCODE_Z) : gb->keys_buttons = (gb->keys_buttons & (0b11111011)) | (is_up << 2);  break;
        case(SDL_SCANCODE_A) : gb->keys_buttons = (gb->keys_buttons & (0b11111101)) | (is_up << 1);  break;
        case(SDL_SCANCODE_S) : gb->keys_buttons = (gb->keys_buttons & (0b11111110)) | (is_up << 0);  break;
        default : break;
    }
    
    if(!is_up) {
        switch(e->keysym.scancode) {
            case(SDL_SCANCODE_DOWN) : 
            case(SDL_SCANCODE_UP)   : 
            case(SDL_SCANCODE_LEFT) : 
            case(SDL_SCANCODE_RIGHT): 
            case(SDL_SCANCODE_X) : 
            case(SDL_SCANCODE_Z) : 
            case(SDL_SCANCODE_A) : 
            case(SDL_SCANCODE_S) : gbInterrupt(gb, INT_JOYPAD);  break;
            default : break;
        }
    }
    
}

void gbRenderSpriteLine(Gameboy *gb, Palette p, TileLine line, u32 *x, u8 y, bool is_x_flipped, bool priority) {
    
    for(i32 i = 0; i < 8; i++) {
        if(*x >= SCREEN_WIDTH)
            return;
        u8 color_val;
        if(!is_x_flipped) {
            color_val = (line.data_2 >> (7-i) & 1);
            color_val <<= 1;
            color_val |=  (line.data_1 >> (7-i) & 1);
        } else {
            color_val = (line.data_2 >> (i) & 1);
            color_val <<= 1;
            color_val |=  (line.data_1 >> (i) & 1);
        }
        u8 paletted_color = p.array[color_val];
        if(color_val != 0) {
            if(!priority || gb->lcd_screen[y][*x] < 1)            
                gb->lcd_screen[y][*x] = paletted_color;
        }

        (*x)++;
    }
}
void gbRenderTileLine(Gameboy *gb, Palette p, TileLine line, u32 *x, u8 y, u8 x_offset) {
    
    for(i32 i = x_offset; i < 8; i++) {
        u8 color_val = (line.data_2 >> (7-i) & 1);
        color_val <<= 1;
        color_val |=  (line.data_1 >> (7-i) & 1);
        gb->lcd_screen[y][*x] = p.array[color_val];
        (*x)++;
    }
}

OAMSprite gbGetOAMSprite(Gameboy *gb, u16 tile_id) {
    OAMSprite result = {0};
    if(tile_id > 40) {
        SDL_Log("Attempting to access tile No %u. This is out of the bounds of the OAM", tile_id);
    }
    result.y = gbRead(gb, 0xFE00 + (tile_id * 4));
    result.x     = gbRead(gb, 0xFE00 + (tile_id * 4) + 1);
    result.tile  = gbRead(gb, 0xFE00 + (tile_id * 4) + 2);
    result.flags = gbRead(gb, 0xFE00 + (tile_id * 4) + 3);
    return result;
}

inline TileLine gbGetTileLine(Gameboy *gb, u8 tile_offset, bool mode, u8 line) {
    TileLine result = {0};
    if(mode == 1) {
        result.data_1 = gbReadAt(gb, 0x8000 + (tile_offset * 16) + (line * 2),0);
        result.data_2 = gbReadAt(gb, 0x8000 + (tile_offset * 16) + (line * 2) + 1,0);
    } else if(mode == 0) {
        i8 tile_addr = *((i8 *)(&tile_offset));
        //tile_addr *= 16;
        result.data_1 = gbReadAt(gb, 0x9000 + tile_addr * 16 + (line * 2),0);
        result.data_2 = gbReadAt(gb, 0x9000 + tile_addr * 16 + (line * 2) + 1,0);
    }
    return result;
}

void gbDrawBackgroundLine(Gameboy *gb, u8 LY, u8 SCX, u8 map_line, u8 tile_line, u16 base_address, bool tile_data_select) {
    
    Palette bg_pal = GetPaletteFromByte(gbReadAt(gb, IO_BGP, 0));

    for(u16 x = 0; x < SCREEN_WIDTH; x++) {
        u8 map_x = ((SCX + (u8)x) / 8) % 32;
        u16 address = (base_address + map_x) + (map_line * 32);
        u8 tile_id = gbReadAt(gb, address, 0);
        TileLine tl = gbGetTileLine(gb, tile_id, tile_data_select, tile_line);
        
        u8 tile_x = (x + SCX) % 8;

        u8 color_val = (tl.data_2 >> (7-tile_x) & 1);
        color_val <<= 1;
        color_val   |= (tl.data_1 >> (7-tile_x) & 1);
        gb->lcd_screen[LY][x] = bg_pal.array[color_val];
    }
}

void gbBackground(Gameboy *gb, u8 LCDC, u8 LY) {
    u8 SCY  = gbReadAt(gb, IO_SCY ,0);
    u8 SCX  = gbReadAt(gb, IO_SCX ,0);
    u8 BGP  = gbReadAt(gb, IO_BGP ,0);

    bool tile_data_select = LCDC >> 4 & 1;
    bool bg_tile_map_select = LCDC >> 3 & 1;
    u16 tile_map_base_address = bg_tile_map_select ? 0x9C00 : 0x9800;

    u8 map_line  = ((LY + SCY) / 8) % 32;
    u8 tile_line = (LY + SCY) % 8;

    gbDrawBackgroundLine(gb, LY, SCX, map_line, tile_line, tile_map_base_address, tile_data_select);
}

void gbWindow(Gameboy *gb, u8 LCDC, u8 LY) {
    bool window_display = LCDC >> 5 & 1;
    if(window_display) {
        u8 WY   = gbReadAt(gb, IO_WY, 0);
        u8 WX   = gbReadAt(gb, IO_WX, 0) - 8;
            
        if(LY < WY)
            return;

        i8 map_line  = ((LY - WY) / 8);
        if (map_line > 32 || map_line < 0) 
            return;
        u8 tile_line = (LY - WY) % 8;
      
        bool tile_data_select = LCDC >> 4 & 1;
        bool window_tile_data_select = LCDC >> 6 & 1;
        u16 tile_map_base_address = window_tile_data_select ? 0x9C00 : 0x9800;
        
        gbDrawBackgroundLine(gb, LY, WX, map_line, tile_line, tile_map_base_address, tile_data_select);
    }
}

void gbOAM(Gameboy *gb, u8 LCDC, u8 LY) {
    u16 x = 0;
    u8 sprite_count = 0;
    bool sprite_size = (LCDC >> 2) & 1;
    u8 OBP0 = gbReadAt(gb, IO_OBP0 ,0);
    u8 OBP1 = gbReadAt(gb, IO_OBP1 ,0);
    for(u32 i = 0; i < 40; i++) {
        if(sprite_count >= 10) 
            break;
        OAMSprite sprite = gbGetOAMSprite(gb, i);
       if(sprite.y < 0x10)
            continue;
        bool is_palette_1 = sprite.flags >> 4 & 1;
        bool is_x_flipped = sprite.flags >> 5 & 1;
        bool is_y_flipped = sprite.flags >> 6 & 1;
        bool priority     = sprite.flags >> 7 & 1;
        Palette pal;
        if(is_palette_1) {
            pal = GetPaletteFromByte(OBP1);
        } else {
            pal = GetPaletteFromByte(OBP0);
        }
        
        sprite.y -= 0x10;
        if(sprite.y <= LY && sprite.y + 8 > LY) {
            // Draw tile
            u8 line = LY - sprite.y;
            if (is_y_flipped) {
                line = 8 - line;
            }
            TileLine tl = gbGetTileLine(gb, sprite.tile, 1, line);
            u32 sx = sprite.x - 8;
            gbRenderSpriteLine(gb, pal, tl, &sx, LY, is_x_flipped, priority);
            sprite_count++;
        } else if(sprite_size == 1 && sprite.y + 8 <= LY && sprite.y + 16 > LY) {
            u8 line = LY - sprite.y - 8;
            if (is_y_flipped) {
                line = 8 - line;
            }
            TileLine tl = gbGetTileLine(gb, sprite.tile+1, 1, line);
            u32 sx = sprite.x - 8;
            gbRenderSpriteLine(gb, pal, tl, &sx, LY, is_x_flipped, priority);
            sprite_count++;

        } else {
            continue;
        }
    }
}

void gbLCD(Gameboy *gb) {
    
    u8 LCDC = gbReadAt(gb, IO_LCDC ,0);
    
    if(!(LCDC >> 7 & 1)) { // LCD Display Enable
        gb->ppu_clock = 1;
        return;
    }
    
    u8 STAT = gbReadAt(gb, IO_STAT, 0);
    u8 LY   = gbReadAt(gb, IO_LY, 0);
    u8 LYC  = gbReadAt(gb, IO_LYC, 0);

    u8 mode = STAT & 0x3;
   
    switch(mode) {
        case 0: { // H blank, draw the line!
            gbBackground(gb, LCDC, LY);
            gbWindow(gb, LCDC, LY);
            gbOAM(gb, LCDC, LY);
            LY++;
            if(LY == 154) {
                LY = 0;
            }
            gbWriteAt(gb, 0xFF44, LY, 0);

            if(LY >= 144 && LY <= 153) { // We're in vblank
                mode = 1;
            } else { // Go the start of the sequence for the next line
                mode = 2;
            }
            gb->ppu_clock += 208;
        } break;
        case 1: { // VBlank
            LY++;
            if(LY == 154) {
                LY = 0;
            }
            gbWriteAt(gb, 0xFF44, LY, 0);

            if(LY >= 144 && LY <= 153) { // We're in vblank
                mode = 1;
            } else { // Go the start of the sequence for the next line
                mode = 2;
            }
            gb->ppu_clock += 456;
        } break;
        case 2: { // Scanning oam
            mode = 3;
            gb->ppu_clock += 80;
        } break;
        case 3: { // Reading oam
            mode = 0;
            gb->ppu_clock += 168;
        } break;
    }
    

    // Update STAT register
    STAT = (STAT & 0b11111011) | (LY == LYC) << 2; // Coincidence flag
    STAT = (STAT & 0b11111100) | mode; // Mode flag
    STAT |= 0x80;
    gbWriteAt(gb, IO_STAT, STAT, 0);
    
    // Interrupts
    if(LY == LYC && STAT >> 6 & 1) { // STAT Interrupt
        gbInterrupt(gb, INT_LCDSTAT);
    }
    
    if(STAT >> 5 & 1) { // OAM interrupt
        // TODO(Guigui): Trigger 
    }
    
    if(LY == 0x90 /*&& (STAT >> 4) & 1*/) { // V-Blank interrupt
        gbInterrupt(gb, INT_VBLANK);
    }
    
    if(STAT >> 3 & 1) { // H-Blank interrupt
        // TODO(Guigui): Trigger 
    }
}

void gbLoop(Gameboy *gb, f32 delta_time) {

    if(!gb->step_through) {
        // Cap the amount of instructions for each frame 
        gb->cycles_left += delta_time * gb->clock_speed * gb->clock_mul;
        if(gb->cycles_left > 69905) gb->cycles_left = 69905; // 69905 = Clockspeed * 1 / 60
        
        // gbAudioLoop(gb, delta_time);
    }
    
    const u8 *keyboard = SDL_GetKeyboardState(0);
    
    while(gb->cycles_left >= 0) {
        // Cpu
        if(gb->cpu_clock <= 0) {
            if(!gb->halted) 
                gbExecute(gb);           
            for(u32 i = 0; i < gb->breakpoint_count; i ++) {
                if(gb->pc == gb->breakpoints[i]) {
                    gb->step_through = true;
                    gb->cycles_left = 0;
                }
            }
            
            if (gb->step_through) {
                gb->cycles_left = 0;
            }

        } 

        // Video
        while(gb->ppu_clock <= 0)
            gbLCD(gb);
       
        // Audio
            gbAudio(gb);

        // DMA
        if(gb->DMA_cycles_left >= 0) {
            u16 addr_read = gbReadAt(gb, 0xFF46, 0) << 8;
            addr_read |= gb->DMA_cycles_left;
            u16 addr_write = 0xFE00 + gb->DMA_cycles_left;
            
            gbWriteAt(gb, addr_write, gbReadAt(gb, addr_read, 0), 0);
            gb->DMA_cycles_left--;
        }

        u8 TAC = gbReadAt(gb, IO_TAC, 0);
        if(TAC >> 2 & 1) { // Timer enabled
            gb->timer++;
            u16 TIMA = gbReadAt(gb, IO_TIMA, 0);
            
            u8 speed = TAC & 0b00000011;
            switch(speed) {
                case 0: if(gb->timer >= 1024){ TIMA++; gb->timer = 0;} break;
                case 1: if(gb->timer >= 16)  { TIMA++; gb->timer = 0;} break;
                case 2: if(gb->timer >= 64)  { TIMA++; gb->timer = 0;} break;
                case 3: if(gb->timer >= 256) { TIMA++; gb->timer = 0;} break;
            }
            
            if(TIMA >= 0x100) { // overflow
                TIMA = gbReadAt(gb, IO_TMA, 0); // Reset it to TMA
                gbInterrupt(gb, INT_TIMER);
            }
            
            gbWriteAt(gb, IO_TIMA, TIMA, 0);
        }

        // Timer update
        gbWriteAt(gb, IO_DIV, gbReadAt(gb, IO_DIV, 0) + 1, 0);

        // Interrupts
        if(gb->ime) {
            u8 IF = gbReadAt(gb, IO_IF, 0);
            u8 IE = gbReadAt(gb, IO_IE, 0);
            
            for(u8 i = 0; i < 5; i++ ){
                if(IE & (1 << i) && IF & (1 << i)) {
                    IF = ~(1 << i) & IF;
                    gbWriteAt(gb, IO_IF, IF, 0);
                    gb->ime = 0;
                    gbWriteAt(gb, --gb->sp, gb->pc >> 8 & 0xFF, 0);
                    gbWriteAt(gb, --gb->sp, gb->pc & 0xFF, 0);
                    gb->pc = 0x40 + (i * 0x8);
                }
            }
        }
        
        // Inputs
        u8 JOY = gbReadAt(gb, IO_JOY, 0);
        if(JOY & (1 << 4)) { // Button
            JOY = 0b11010000;
            JOY |= gb->keys_buttons & 0x0F;
        }
        if(JOY & (1 << 5)) { // D-Pad
            JOY = 0b11100000;
            JOY |= gb->keys_dpad & 0x0F;
        }
        gbWriteAt(gb, IO_JOY, JOY, 0); 

        gb->cpu_clock--;
        gb->ppu_clock--;
        gb->cycles_left--;
    }
}

void gbDraw(Gameboy *gb, u32 zoom, SDL_Renderer *renderer) {
    SDL_Rect panel = {0, 0, SCREEN_WIDTH * zoom, SCREEN_HEIGHT * zoom };
    SDL_RenderDrawRect(renderer, &panel);
    
    for(u32 y = 0; y < SCREEN_HEIGHT; y++) {
        for(u32 x = 0; x < SCREEN_WIDTH; x++) {
            u8 pixel = gb->lcd_screen[y][x];
            
            Color color = palette[pixel];
            
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255); 
            SDL_RenderDrawPoint(renderer, x, y);
        }
    }
}

// --------
// Debug

void DebugDrawMemLines(Gameboy *gb, u16 addr, u16 nb, u32 *x, u32 *y, SDL_Renderer *renderer, u32 current_addr) {
    u32 x_start = *x;
    for(u32 i = 0; i < nb; i++) {
        *x = x_start;
        u32 instruction_bytes = 0;
        
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        
        RenderText(renderer, x, *y, " %04X | ", addr);
        for(u32 j = 0; j < 16; j++) {
            if(addr + j == current_addr) {
                SDL_SetRenderDrawColor(renderer, 100, 200, 100, 255);
                RenderText(renderer, x, *y, "%02X ", gbReadAt(gb, addr + j, 0));
                SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
                if(current_addr == gb->pc)
                    instruction_bytes = OPCODE_DESC_TABLE[gbReadAt(gb, gb->pc, 0)].bytes - 1;
            } else if (instruction_bytes > 0){
                SDL_SetRenderDrawColor(renderer, 200, 100, 100, 255);
                RenderText(renderer, x, *y, "%02X ", gbReadAt(gb, addr + j, 0));
                instruction_bytes--;
                SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
            } else {
                RenderText(renderer, x, *y, "%02X ", gbReadAt(gb, addr + j, 0));
            }
        }
        addr += 16;
        *y += TTF_FontHeight(global_font);
    }
}

void gbDrawDissassembly(Gameboy *gb, u32 *addr, u32 x, u32 *y, SDL_Renderer *renderer) {
    Opcode code = OPCODE_DESC_TABLE[gbReadAt(gb, *addr, 0)];
    u32 op_x = x;
    u32 orig_y = *y;
    u32 start_addr = *addr;
    if(start_addr == gb->pc)
        SDL_SetRenderDrawColor(renderer, 100, 200, 100, 255);
    for(u32 i = 0; i < gb->breakpoint_count; i ++) {
        if(start_addr == gb->breakpoints[i])
            SDL_SetRenderDrawColor(renderer, 200, 100, 100, 255);
    }
    
    RenderText(renderer, &x, *y, "%04X  ", *addr);
    
    if(code.bytes == 0) {
        RenderLine(renderer, x, y, "UNKNOWN");
        return;
    }
    
    for(u32 i = 0; i < code.bytes; i++){
        RenderText(renderer, &x, *y, "%02X ", gbReadAt(gb, start_addr + i, 0));
    }
    u32 desc_x = op_x + 150;
    RenderText(renderer, &desc_x, *y, "%s ", code.desc);
    u32 operand_x = op_x + 300;
    for(u32 i = 1; i < code.bytes; i++){
        RenderText(renderer, &operand_x, *y, "%02X", gbReadAt(gb, start_addr + i, 0));
    }
    *y += TTF_FontHeight(global_font);
    if(io.mouse_down) {
        if(io.mouse_x <= operand_x && io.mouse_x >= op_x && io.mouse_y <= *y && io.mouse_y > orig_y) {
            bool existed = false;
            for(u32 i = 0; i < gb->breakpoint_count; i ++) {
                if (gb->breakpoints[i] == *addr) {
                    memcpy(&gb->breakpoints[i], &gb->breakpoints[i+1], (15 - i) * sizeof(u16));
                    gb->breakpoint_count--;
                    existed = true;
                    break;
                }
            }
            if(!existed) {
                gb->breakpoints[gb->breakpoint_count++] = *addr;
                if(gb->breakpoint_count > 16)
                    gb->breakpoint_count = 0;
                
            }
            FILE *file = fopen("bp.txt", "w+");
            if(file) {
                for(u32 i = 0; i < gb->breakpoint_count; i++) {
                    fprintf(file, "%04X\n", gb->breakpoints[i]);
                    
                }
                fclose(file);
            } 
        }
    }
    
    (*addr) += code.bytes;
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
}

void gbDrawDebug(Gameboy *gb, SDL_Rect rect, Console *console, SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    const u32 min_x = rect.w + 10;
    
    {   // Registers
        u32 y = rect.h + 10;
        u32 x = 10;
        RenderLine(renderer, x, &y, "af %04X", gb->af);
        RenderLine(renderer, x, &y, "bc %04X", gb->bc);
        RenderLine(renderer, x, &y, "de %04X", gb->de);
        RenderLine(renderer, x, &y, "hl %04X", gb->hl);
        RenderLine(renderer, x, &y, "sp %04X", gb->sp);
        RenderLine(renderer, x, &y, "pc %04X", gb->pc);
        RenderLine(renderer, x, &y, "--------");
        RenderLine(renderer, x, &y, "Z %u", (gb->f >> 7) & 1);
        RenderLine(renderer, x, &y, "N %u", (gb->f >> 6) & 1);
        RenderLine(renderer, x, &y, "H %u", (gb->f >> 5) & 1);
        RenderLine(renderer, x, &y, "C %u", (gb->f >> 4) & 1);
        RenderLine(renderer, x, &y, " ");
        RenderLine(renderer, x, &y, "----------------");
        u8 LCDC = gbReadAt(gb, 0xFF40, 0);
        u8 STAT = gbReadAt(gb, 0xFF41, 0);
        RenderLine(renderer, x, &y, "LCDC(FF40) %u%u%u%u%u%u%u%u %02X", BINARY_FMT(LCDC), LCDC);
        RenderLine(renderer, x, &y, "STAT(FF41) %u%u%u%u%u%u%u%u %02X", BINARY_FMT(STAT), STAT);
        RenderLine(renderer, x, &y, "SCY (FF42) %02X",    gbReadAt(gb, IO_SCY, 0));
        RenderLine(renderer, x, &y, "SCX (FF43) %02X",    gbReadAt(gb, IO_SCX, 0));
        RenderLine(renderer, x, &y, "LY  (FF44) %02X %u", gbReadAt(gb, IO_LY, 0), gbReadAt(gb, 0XFF44, 0));
        RenderLine(renderer, x, &y, "DMA (FF46) %02X",    gbReadAt(gb, IO_DMA, 0));
        RenderLine(renderer, x, &y, "DMA Timer  %02X", gb->DMA_cycles_left);
        RenderLine(renderer, x, &y, "----------------");
        RenderLine(renderer, x, &y, "LAST R     %04X", gb->last_read);
        RenderLine(renderer, x, &y, "LAST W     %04X", gb->last_write);
        RenderLine(renderer, x, &y, "TIMA(FF05) %02X", gbReadAt(gb, IO_TIMA, 0));
        RenderLine(renderer, x, &y, "TMA (FF06) %02X", gbReadAt(gb, IO_TMA, 0));
        RenderLine(renderer, x, &y, "TAC (FF07) %02X", gbReadAt(gb, IO_TAC, 0));
        RenderLine(renderer, x, &y, "BUT        %u%u%u%u%u%u%u%u", BINARY_FMT(gb->keys_buttons));
        RenderLine(renderer, x, &y, "PAD        %u%u%u%u%u%u%u%u", BINARY_FMT(gb->keys_dpad));
        RenderLine(renderer, x, &y, "ROM        %02X", gb->rom.rom_bank);
        RenderLine(renderer, x, &y, "RAM        %02X", gb->rom.ram_bank);
        RenderLine(renderer, x, &y, "----------------");
        RenderLine(renderer, x, &y, "NR52(FF26) %u%u%u%u%u%u%u%u", BINARY_FMT(gbReadAt(gb, IO_NR52, 0)));
        
    }
    {
        u32 y = rect.h + 10;
        u32 x = 75;
        
        u8 IF = gbReadAt(gb, IO_IF, 0);
        u8 IE = gbReadAt(gb, IO_IE, 0);
        RenderLine(renderer, x, &y, "ime   %d", gb->ime);
        RenderLine(renderer, x, &y, "IF %u%u%u%u%u%u %02X", IF >> 5 & 1,IF >> 4 & 1, IF >> 3 & 1, IF >> 2 & 1, IF >> 1 & 1, IF >> 0 & 1, IF);
        RenderLine(renderer, x, &y, "IE %u%u%u%u%u%u %02X", IE >> 5 & 1, IE >> 4 & 1, IE >> 3 & 1, IE >> 2 & 1, IE >> 1 & 1, IE >> 0 & 1, IE);
        RenderLine(renderer, x, &y, "(hl)  %04X", gbReadAt(gb, gb->hl, 0));
        RenderLine(renderer, x, &y, "(sp)  %04X", gbReadAt(gb, gb->sp, 0));
        
        RenderLine(renderer, x, &y, " ");
        RenderLine(renderer, x, &y, "--------");
        RenderLine(renderer, x, &y, "CPU CLK  %d", gb->cpu_clock);
        RenderLine(renderer, x, &y, "PPU CLK  %d", gb->ppu_clock);
        RenderLine(renderer, x, &y, "GBL CLK  %d", gb->cycles_left);
        RenderLine(renderer, x, &y, "SPD   %.3fx", gb->clock_mul);
        RenderLine(renderer, x, &y, "Timer    %d", gb->timer);
        RenderLine(renderer, x, &y, "STEP     %d", gb->step_through);
    }
    
    {
        u32 x_section_start = min_x;
        u32 x_section = min_x;
        u32 y_section_start = 10;
        u32 y_section = 10;
        
        { // Disassembly
            RenderLine(renderer, x_section, &y_section, "DISASSEMBLY");
            u32 pc = gb->pc;
            for(u32 i = 0; i < 19; i ++) {
                gbDrawDissassembly(gb, &pc, x_section, &y_section, renderer);
            }
            RenderLine(renderer, x_section, &y_section, " ");
        }
        
        {   // LAST READ
            const u32 lines = 8;
            i32 start_addr = gb->last_read - (gb->last_read % 16) - (lines / 2) * 16;
            while(start_addr < 0) {
                start_addr += 16;
            }
            u32 x = x_section_start;
            RenderLine(renderer, x, &y_section, "LAST READ");
            DebugDrawMemLines(gb, start_addr, lines, &x_section, &y_section, renderer, gb->last_read);
        }
        {   // LAST WRITE
            const u32 lines = 8;
            i32 start_addr = gb->last_write - (gb->last_write % 16) - (lines / 2) * 16;
            while(start_addr < 0) {
                start_addr += 16;
            }
            u32 x = x_section_start;
            RenderLine(renderer, x, &y_section, "LAST WRITE");
            DebugDrawMemLines(gb, start_addr, lines, &x, &y_section, renderer, gb->last_write);
        }
        
        
        // New column
        x_section += 10;
        x_section_start = x_section;
        y_section_start = 10;
        y_section = y_section_start;
        {   // ROM
            const u32 lines = 8;
            i32 start_addr = gb->pc - (gb->pc % 16) - (lines / 2) * 16;
            while(start_addr < 0) {
                start_addr += 16;
            }
            RenderLine(renderer, x_section, &y_section, "ROM");
            DebugDrawMemLines(gb, start_addr, lines, &x_section, &y_section, renderer, gb->pc);
        }
        {   // OAM
            RenderLine(renderer, x_section_start, &y_section, "OAM");
            u32 x = x_section_start;
            DebugDrawMemLines(gb, MEM_OAM_START, MEM_OAM_SIZE / 16 + 1, &x, &y_section, renderer, gb->hl);
        }
        {   // IO
            RenderLine(renderer, x_section_start, &y_section, "IO");
            u32 x = x_section_start;
            DebugDrawMemLines(gb, MEM_IO_START, 0x80 / 16, &x, &y_section, renderer, gb->last_write);
        }
        {
            // HRAM
            RenderLine(renderer, x_section_start, &y_section, "HRAM");
            u32 x = x_section_start;
            DebugDrawMemLines(gb, MEM_HRAM_START, 0x80 / 16, &x, &y_section, renderer, gb->last_write);
        }
        
        // New column
        x_section += 10;
        x_section_start = x_section;
        y_section_start = 10;
        y_section = y_section_start;
        
        {
            RenderLine(renderer, x_section_start, &y_section, "TILES");
            u8 tile_addr = 0x00;
            u32 x = x_section_start;
            u32 y = y_section;
            for(u32 j = 0; j < 0x10; j ++){ // Tiles vert
                x = x_section_start;
                for(u32 i = 0; i < 0x10; i++) { // Tiles hor
                    for(u32 t_y = 0; t_y < 8; t_y++) {
                        TileLine tl = gbGetTileLine(gb, tile_addr, 1, t_y);
                        for(i32 t_x = 0; t_x < 8; t_x++) {
                            u8 pixel = (tl.data_2 >> (7-t_x) & 1);
                            pixel <<= 1;
                            pixel |= (tl.data_1 >> (7-t_x) & 1);
                            Color color = palette[pixel];
                            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255); 
                            SDL_RenderDrawPoint(renderer, x + t_x, y + t_y);
                            
                        }
                    }
                    x += 8;
                    tile_addr+=1;
                }
                y+= 8;
            }
            tile_addr = 0x00;
            x = x_section_start;
            for(u32 j = 0; j < 0x8; j ++){ // Tiles vert
                x = x_section_start;
                for(u32 i = 0; i < 0x10; i++) { // Tiles hor
                    for(u32 t_y = 0; t_y < 8; t_y++) {
                        TileLine tl = gbGetTileLine(gb, tile_addr, 0, t_y);
                        for(i32 t_x = 0; t_x < 8; t_x++) {
                            u8 pixel = (tl.data_2 >> (7-t_x) & 1);
                            pixel <<= 1;
                            pixel |= (tl.data_1 >> (7-t_x) & 1);
                            Color color = palette[pixel];
                            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255); 
                            SDL_RenderDrawPoint(renderer, x + t_x, y + t_y);
                            
                        }
                    }
                    x += 8;
                    tile_addr+=1;
                }
                y+= 8;
            }
            y_section = y;
        }
        {
            RenderLine(renderer, x_section_start, &y_section, "MAP 1");
            u8 LCDC = gbReadAt(gb, 0xFF40, 0);
            bool tile_data_select = LCDC >> 4 & 1;
            u16 base_addr = 0x9800;
            for(u32 y = 0; y < 32; y++) {
                for(u32 x = 0; x < 32; x++) {
                    u16 addr = base_addr + x + y * 32;
                    u8 tile_id = gbReadAt(gb, addr, 0);
                    for(u16 line = 0; line < 8; line++) {
                        TileLine tl = gbGetTileLine(gb, tile_id, tile_data_select, line);
                        for(i32 row = 0; row < 8; row++) {
                            u8 pixel = (tl.data_2 >> (7 - row) & 1);
                            pixel <<= 1;
                            pixel |= (tl.data_1 >> (7 - row) & 1);
                            Color color = palette[pixel];
                            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255); 
                            SDL_RenderDrawPoint(renderer, x_section + x * 8 + row, y_section + y * 8 + line);
                        }
                    }
                }
            }
            y_section += 32 * 8;
        }
        
        {
            RenderLine(renderer, x_section_start, &y_section, "MAP 2");
            u8 LCDC = gbReadAt(gb, 0xFF40, 0);
            bool tile_data_select = LCDC >> 4 & 1;
            u16 base_addr = 0x9C00;
            for(u32 y = 0; y < 32; y++) {
                for(u32 x = 0; x < 32; x++) {
                    u16 addr = base_addr + x + y * 32;
                    u8 tile_id = gbReadAt(gb, addr, 0);
                    for(u16 line = 0; line < 8; line++) {
                        TileLine tl = gbGetTileLine(gb, tile_id, tile_data_select, line);
                        for(i32 row = 0; row < 8; row++) {
                            u8 pixel = (tl.data_2 >> (7 - row) & 1);
                            pixel <<= 1;
                            pixel |= (tl.data_1 >> (7 - row) & 1);
                            Color color = palette[pixel];
                            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255); 
                            SDL_RenderDrawPoint(renderer, x_section + x * 8 + row, y_section + y * 8 + line);
                        }
                    }
                }
            }
        }


        // New column
        x_section += 140;
        x_section_start = x_section;
        y_section_start = 10;
        y_section = y_section_start;
        {   // Stack
            RenderLine(renderer, x_section_start, &y_section, "STACK");
            
            u16 addr = gb->sp + 8;
            
            for(u32 i = 0; i < 15; i++) {
                u32 x = x_section_start;
                if(addr - i == gb->sp) {
                    SDL_SetRenderDrawColor(renderer, 100, 200, 100, 255);
                } else {
                    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
                }
                RenderText(renderer, &x, y_section, "%04X ", addr - i);
                RenderText(renderer, &x, y_section, "%02X", gbReadAt(gb, addr - i, 0));
                y_section += TTF_FontHeight(global_font);
                x_section = x;
            }
        }
        // New column
        x_section += 10;
        x_section_start = x_section;
        y_section_start = 10;
        y_section = y_section_start;
        {   // Call Stack
            RenderLine(renderer, x_section_start, &y_section, "JUMP LIST");
            for(u32 i = 0; i < gb->call_stack.capacity; i++) {
                RenderLine(renderer, x_section, &y_section, "%04X -> %04X", gb->call_stack.from[i], gb->call_stack.to[i]);
            }
        }
    }
}
