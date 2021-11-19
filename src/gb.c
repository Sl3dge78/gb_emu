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

void gbWriteAt(Gameboy *gb, u16 address, u8 value, bool log) {
    if(log)
        gb->last_write = address;
    
    // MBC Ram enable
    if(address >= 0x0000 && address <= 0x1FFF) {
        if(value >= 0x0A) {
            SDL_Log("Enabling MBC Ram");
        } else {
            SDL_Log("Disabling MBC Ram");
        }
        return;
    }
    // ROM bank number
    if(address >= 0x2000 && address <= 0x3FFF) {
        if(value == 0 || value == 0x20 || value == 0x40 || value == 0x60 ) {
            value++;
        }
        gb->rom_bank = value;
        return;
    }
    // RAM Bank number / Upper bits of rom bank number
    if(address >= 0x4000 && address <= 0x5FFF) {
        value &= 0x3;
        if(gb->ram_bank_mode) {
            gb->ram_bank = value;
        } else {
            gb->rom_bank = (gb->rom_bank & 0b10011111) | value;
        }
        return;
    }
    // Bank mode
    if(address >= 0x6000 && address <= 0x7FFF) {
        gb->ram_bank_mode = value;
        return;
    }
    
    // RAM
    if(address >= MEM_CARTRAM_START && address <= MEM_CARTRAM_END) {
        u16 offset = gb->ram_bank * 0x2000;
        u16 relative_address = address - MEM_CARTRAM_START;
        gb->cart_ram[relative_address + offset] = value;
        return;
    }
    
    switch(address){
        case (IO_DMA) : {
            gb->DMA_cycles_left = 0x9F; // DMA transfer
        } break;
        case (IO_STAT) : return; // Prevent writing
        case (IO_IF)   : value |= 0xE0; break;
        case (IO_TAC)  : value |= 0xF8; break;
        case (IO_DIV)  : gb->timer = 0; break;
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
    
    if(address <= MEM_ROM00_END) {
        return gb->rom[address];
    }
    if(address >= MEM_ROMNN_START && address <= MEM_ROMNN_END) {
        u16 offset = gb->rom_bank * 0x4000;
        u16 base_address = address - MEM_ROMNN_START;
        return gb->rom[base_address + offset];
    }
    
    if(address >= MEM_CARTRAM_START && address <= MEM_CARTRAM_END) {
        u16 offset = gb->ram_bank * 0x2000;
        u16 relative_address = address - MEM_CARTRAM_START;
        return gb->cart_ram[relative_address + offset];
    }
    
    if(address >= MEM_MIRROR0_START && address <= MEM_MIRROR1_END) {
        address -= MEM_MIRROR0_START;
    }
    
    return gb->mem[address];
}

u8 *gbGetPointerTo(Gameboy *gb, u16 address) {
    
    if(address <= MEM_ROM00_END) {
        return &gb->rom[address];
    }
    if(address >= MEM_ROMNN_START && address <= MEM_ROMNN_END) {
        u16 offset = gb->rom_bank * 0x4000;
        u16 base_address = address - MEM_ROMNN_START;
        return &gb->rom[base_address + offset];
    }
    
    if(address >= MEM_CARTRAM_START && address <= MEM_CARTRAM_END) {
        u16 offset = gb->ram_bank * 0x2000;
        u16 relative_address = address - MEM_CARTRAM_START;
        return &gb->cart_ram[relative_address + offset];
    }
    
    if(address >= MEM_MIRROR0_START && address <= MEM_MIRROR1_END) {
        address -= MEM_MIRROR0_START;
    }
    
    return &gb->mem[address];
}

void gbSetFlags(Gameboy *gb, i32 Z, i32 N, i32 H, i32 C) {
    
    if(Z != -1)
        gb->f = (gb->f & ~Z_FLAG) | (Z << 7 & Z_FLAG);
    if(N != -1)
        gb->f = (gb->f & ~N_FLAG) | (N << 6 & N_FLAG);
    if(H != -1)
        gb->f = (gb->f & ~H_FLAG) | (H << 5 & H_FLAG);
    if(C != -1)
        gb->f = (gb->f & ~C_FLAG) | (C << 4 & C_FLAG);
    
}

void gbADD(Gameboy *gb, u8 operand, u8 carry) {
    u16 half_a = gb->a & 0xF;
    half_a += operand & 0xF;
    u16 result = gb->a + operand;
    if((gb->f & C_FLAG) != 0 && carry) {
        result += carry;
        half_a += carry;
    }
    
    gbSetFlags(gb, (result & 0xFF) == 0, 0, half_a > 0xF, result > 0xFF);
    gb->a = result & 0xFF;
}

void gbSUB(Gameboy *gb, u8 operand, u8 carry) {
    u16 half_a = gb->a & 0xF;
    half_a -= operand & 0xF;
    u16 result = gb->a - operand;
    if((gb->f & C_FLAG) != 0 && carry) {
        result -= carry;
        half_a -= carry;
    }
    gbSetFlags(gb, (result & 0xFF) == 0, 1, half_a > 0xF, result > 0xFF);
    gb->a = result & 0xFF;
}

void gbAND(Gameboy *gb, u8 operand) {
    gb->a = operand & gb->a;
    gbSetFlags(gb, gb->a == 0, 0, 1, 0);
}

void gbXOR(Gameboy *gb, u8 operand) {
    gb->a = operand ^ gb->a;
    gbSetFlags(gb, gb->a == 0, 0, 0, 0);
}

void gbOR(Gameboy *gb, u8 operand) {
    gb->a = operand | gb->a;
    gbSetFlags(gb, gb->a == 0, 0, 0, 0);
}

void gbCP(Gameboy *gb, u8 operand) {
    i16 half_a = gb->a & 0xF;
    
    i16 result = gb->a - operand;
    half_a -= operand & 0xF;
    
    gbSetFlags(gb, result == 0, 1, half_a < 0, result < 0);
    
}

u8 *gbGetRegisterFromID(Gameboy *gb, u8 id) {
    switch(id) {
        case 0 : return &gb->b;
        case 1 : return &gb->c;
        case 2 : return &gb->d;
        case 3 : return &gb->e;
        case 4 : return &gb->h;
        case 5 : return &gb->l;
        case 6 : return gbGetPointerTo(gb, gb->hl);
        case 7 : return &gb->a;
        default: assert(0); return NULL;
    }
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
    
    FILE *rom = fopen(path, "rb");
    
    fseek(rom, 0, SEEK_END);
    u32 size = ftell(rom);
    rewind(rom);
    
    gb->rom = calloc(size, sizeof(u8));
    
    fread(gb->rom, 1, size, rom);
    fclose(rom);
    
    gb->cartridge_type = gbReadAt(gb, 0x0147, 0);
    if(gb->cartridge_type != 0x00 && gb->cartridge_type != 0x01) {
        SDL_Log("Unsupported cartridge type %02X. Something might not work", gb->cartridge_type);
    }
    gb->rom_size = gbReadAt(gb, 0x0148, 0);
    gb->ram_size = gbReadAt(gb, 0x0149, 0);
    
    gb->cart_ram = calloc(1, gb->ram_size);
    
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
}

void gbInit(Gameboy *gb) {
    gb->clock_speed = 4194304;
    gb->clock_mul = 1.0f;
    gb->cpu_clock = 1;
    gb->ppu_clock = 1;
    gb->cycles_left = 0;
    gb->running = true;
    gb->step_through = 1;
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
    
    gbReset(gb);
}

void gbInput(Gameboy *gb, SDL_KeyboardEvent *e, bool is_up) {
    if(!is_up) {
        switch(e->keysym.scancode) {
            case(SDL_SCANCODE_SPACE) : case(SDL_SCANCODE_F9) : {
                gb->step_through = !gb->step_through; 
                gb->cpu_clock = 0.0f; 
            } break;
            case(SDL_SCANCODE_F7) :
            case(SDL_SCANCODE_N)  : gb->cycles_left = gb->cpu_clock == 0 ? 1 : gb->cpu_clock; break;
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

void gbRenderTileLine(Gameboy *gb, TileLine line, u32 *x, u8 y, u8 x_offset) {
    
    for(i32 i = x_offset; i < 8; i++) {
        if(*x >= SCREEN_WIDTH)
            *x = 0;
        gb->lcd_screen[y][*x] = (line.data_2 >> (7-i) & 1);
        gb->lcd_screen[y][*x] <<= 1;
        gb->lcd_screen[y][*x] |=  (line.data_1 >> (7-i) & 1);
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

TileLine gbGetTileLine(Gameboy *gb, u8 tile_offset, bool mode, u8 line) {
    TileLine result = {0};
    if(mode == 1) {
        result.data_1 = gbReadAt(gb, 0x8000 + (tile_offset * 16) + (line * 2),0);
        result.data_2 = gbReadAt(gb, 0x8000 + (tile_offset * 16) + (line * 2) + 1,0);
    } else if(mode == 0) {
        i16 tile_addr = *((i16 *)(&tile_offset));
        tile_addr *= 16;
        result.data_1 = gbReadAt(gb, 0x9000 + tile_addr + (line * 2),0);
        result.data_2 = gbReadAt(gb, 0x9000 + tile_addr + (line * 2) + 1,0);
    }
    return result;
}

void gbLCD(Gameboy *gb) {
    
    u8 LCDC = gbReadAt(gb, IO_LCDC ,0);
    
    if(!(LCDC >> 7 & 1)) { // LCD Display Enable
        gb->ppu_clock = 1;
        return;
    }
    
    u8 STAT = gbReadAt(gb, IO_STAT ,0);
    u8 SCY  = gbReadAt(gb, IO_SCY ,0);
    u8 SCX  = gbReadAt(gb, IO_SCX ,0);
    u8 LY   = gbReadAt(gb, IO_LY ,0);
    u8 LYC  = gbReadAt(gb, IO_LYC ,0);
    u8 BGP  = gbReadAt(gb, IO_BGP ,0);
    u8 OBP0 = gbReadAt(gb, IO_OBP0 ,0);
    u8 OBP1 = gbReadAt(gb, IO_OBP1 ,0);
    u8 WY   = gbReadAt(gb, IO_WY ,0);
    u8 WX   = gbReadAt(gb, IO_WX ,0);
    
    if(LY < SCREEN_HEIGHT) {
        bool window_tile_map_select = LCDC >> 6 & 1;
        bool window_display = LCDC >> 5 & 1;
        bool tile_data_select = LCDC >> 4 & 1;
        bool bg_tile_map_select = LCDC >> 3 & 1;
        
        // Render scanline
        u32 x = 0;
        
        // BG
        u16 tile_map_base_address = bg_tile_map_select ? 0x9C00 : 0x9800;
        
        // Offset the start by SCX and SCY
        u16 tile_map_start = tile_map_base_address + SCX;
        tile_map_start += (LY + SCY) / 8 * 32;
        
        u8 x_offset = SCX % 8;
        
        while(x < SCREEN_WIDTH) {
            u8 line = (LY + SCY) % 8;
            u8 tile_id = gbReadAt(gb, tile_map_start,0);
            TileLine tl = gbGetTileLine(gb, tile_id, tile_data_select, line);
            u16 y = LY + SCY;
            if(y >= SCREEN_HEIGHT)
                break;
            
            gbRenderTileLine(gb, tl, &x, y, x_offset);
            tile_map_start++;
        }
        
        // OAM
        x = 0;
        u8 sprite_count = 0;
        for(u32 i = 0; i < 40; i++) {
            if(sprite_count >= 10) 
                break;
            OAMSprite sprite = gbGetOAMSprite(gb, i);
            
            if(sprite.y < 0x10)
                continue;
            
            sprite.y -= 0x10;
            if(sprite.y <= LY && sprite.y + 8 >= LY) {
                // Draw tile
                u8 line = LY - sprite.y;
                TileLine tl = gbGetTileLine(gb, sprite.tile, 1, line);
                u32 sx = sprite.x - 0x08;
                gbRenderTileLine(gb, tl, &sx, LY, 0);
                sprite_count++;
            } else {
                continue;
            }
        }
    }
    
    
    // End of Drawing
    LY++;
    if(LY == 154) {
        LY = 0;
    }
    gbWriteAt(gb, 0xFF44, LY, 0);
    
    // Update STAT register
    STAT = (STAT & ~(1 << 2)) | ((LY == LYC) << 2 & (1 << 2)); // Coincidence flag
    u8 mode = 0;
    if(LY >= 144 && LY <= 153) {
        mode = 1;
    } else {
        mode = 2;
    }
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
    
    gb->ppu_clock += 456;
}

void gbLoop(Gameboy *gb, f32 delta_time) {
    
    if(!gb->step_through) {
        // Cap the amount of instructions for each frame 
        gb->cycles_left = delta_time * gb->clock_speed * gb->clock_mul;
        if(gb->cycles_left > 69905) gb->cycles_left = 69905; // Clockspeed * 1 / 60
    }
    
    const u8 *keyboard = SDL_GetKeyboardState(0);
    
    while(gb->cycles_left >= 0) {
        if(gb->cpu_clock <= 0) {
            for(u32 i = 0; i < gb->breakpoint_count; i ++) {
                if(gb->pc == gb->breakpoints[i]) {
                    gb->step_through = true;
                    gb->cycles_left = 0;
                }
            }
            if(gb->step_through) {
                gb->cycles_left = 0;
            }

            if(!gb->halted) 
                gbExecute(gb);           
        } 
        while(gb->ppu_clock <= 0)
            gbLCD(gb);
        
        // DMA
        if(gb->DMA_cycles_left >= 0) {
            u16 addr_read = gbReadAt(gb, 0xFF46, 0) << 8;
            addr_read |= gb->DMA_cycles_left;
            u16 addr_write = 0xFE00 + gb->DMA_cycles_left;
            
            gbWriteAt(gb, addr_write, gbReadAt(gb, addr_read, 0), 0);
            gb->DMA_cycles_left--;
        }

        u8 TAC = gbReadAt(gb, IO_TAC,0 );
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
                    SDL_Log("Interrupt triggered %04X", gb->pc);
                }
            }
        }
        
        // Inputs
        u8 JOY = gbReadAt(gb, IO_JOY, 0);
        if(JOY & (1 << 5)) { // Button
            JOY = 0b11100000;
            JOY |= gb->keys_buttons & 0x0F;
        }
        if(JOY & (1 << 4)) { // D-Pad
            JOY = 0b11010000;
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
        RenderLine(renderer, x, &y, "ROM        %02X", gb->rom_bank);
        RenderLine(renderer, x, &y, "RAM        %02X", gb->ram_bank);
        
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
            
            RenderLine(renderer, x_section_start, &y, " ");
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
        RenderLine(renderer, x_section_start, &y_section, " ");
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
