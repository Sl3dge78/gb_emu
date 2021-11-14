
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


void gbWriteAt(Gameboy *gb, const u16 address, const u8 value, bool log) {
    if(log)
        gb->last_write = address;
    
    switch(address){
        case (IO_DMA) : {
            gb->DMA_cycles_left = 0x9F; // DMA transfer
        } break;
        case (IO_STAT) : return; // Prevent writing
    }
    gb->mem[address] = value;
    
    /*
    if(address == 0xC001)
        gbBreakpoint(gb);
    */
    
    return;
}

u8 gbReadAt(Gameboy *gb, const u16 address, bool debug) {
    
    if(!debug)
        gb->last_read = address;
    
    return gb->mem[address];
}

u8 *gbGetPointerTo(Gameboy *gb, const u16 address) {
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
    i16 result = gb->a + operand + (carry && ((gb->f & C_FLAG) != 0));
    half_a += operand & 0xF;
    
    gbSetFlags(gb, (result & 0xFF) == 0, 0, half_a > 0xF, result > 0xFF);
    
    gb->a = result & 0xFF;
}

void gbSUB(Gameboy *gb, u8 operand, u8 carry) {
    i16 half_a = gb->a & 0xF;
    
    i16 result = gb->a - operand + (carry && ((gb->f & C_FLAG) != 0));
    half_a -= operand & 0xF;
    
    gbSetFlags(gb, result == 0, 1, half_a < 0, result < 0);
    
    if(result < 0)
        gb->a = 256 + result;
    else
        gb->a = result;
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
    u8 IF = gbRead(gb, IO_IF);
    IF |= 1 << id;
    IF |= 0xE0;
    gbWrite(gb, IO_IF, IF);
}

// ---------
// Lifecycle


void gbLoadRom(Gameboy *gb, const char *path) {
    
    FILE *rom = fopen(path, "rb");
    
    fseek(rom, 0, SEEK_END);
    u32 size = ftell(rom);
    
    rewind(rom);
    i32 a = MEM_ROM_SIZE;
    assert(size - 1 <= MEM_ROM_SIZE);
    fread(gb->mem, 1, MEM_ROM_SIZE, rom);
    fclose(rom);
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
    gbWrite(gb, IO_LCDC, 0x91); // LCDC 
    gbWrite(gb, IO_STAT, 0x85); // STAT
    gbWrite(gb, IO_IF, 0xE1); // IF
    gbWrite(gb, IO_IE, 0x00); // IE
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

void gbInput(Gameboy *gb, SDL_KeyboardEvent *e) {
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
        default : return;
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
        result.data_1 = gbRead(gb, 0x8000 + (tile_offset * 16) + (line * 2));
        result.data_2 = gbRead(gb, 0x8000 + (tile_offset * 16) + (line * 2) + 1);
    } else if(mode == 0) {
        i16 tile_addr = *((i16 *)(&tile_offset));
        tile_addr *= 16;
        result.data_1 = gbRead(gb, 0x9000 + tile_addr + (line * 2));
        result.data_2 = gbRead(gb, 0x9000 + tile_addr + (line * 2) + 1);
    }
    return result;
}

void gbLCD(Gameboy *gb) {
    
    u8 LCDC = gbRead(gb, IO_LCDC);
    
    if(!(LCDC >> 7 & 1)) { // LCD Display Enable
        gb->ppu_clock = 1;
        return;
    }
    
    u8 STAT = gbRead(gb, IO_STAT);
    u8 SCY  = gbRead(gb, IO_SCY);
    u8 SCX  = gbRead(gb, IO_SCX);
    u8 LY   = gbRead(gb, IO_LY);
    u8 LYC  = gbRead(gb, IO_LYC);
    u8 BGP  = gbRead(gb, IO_BGP);
    u8 OBP0 = gbRead(gb, IO_OBP0);
    u8 OBP1 = gbRead(gb, IO_OBP1);
    u8 WY   = gbRead(gb, IO_WY);
    u8 WX   = gbRead(gb, IO_WX);
    
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
            u8 tile_id = gbRead(gb, tile_map_start);
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
        //memset(gb->lcd_screen, 0, SCREEN_WIDTH * SCREEN_HEIGHT);
    }
    gbWrite(gb, 0xFF44, LY);
    
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
    gbWrite(gb, IO_STAT, STAT);
    
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
        gb->cycles_left = delta_time * gb->clock_speed * gb->clock_mul;
    }
    
    while(gb->cycles_left >= 0) {
        if(gb->cpu_clock <= 0) {
            gbExecute(gb);
            for(u32 i = 0; i < gb->breakpoint_count; i ++) {
                if(gb->pc == gb->breakpoints[i]) {
                    gb->step_through = true;
                    gb->cycles_left = 0;
                }
            }
            if(gb->step_through) {
                gb->cycles_left = 0;
            }
        } 
        while(gb->ppu_clock <= 0)
            gbLCD(gb);
        
        // Timer update
        gbWriteAt(gb, IO_DIV, gbRead(gb, IO_DIV) + 1, 0);
        
        u8 TAC = gbRead(gb, IO_TAC);
        if(TAC >> 2 & 1) { // Timer enabled
            gb->timer++;
            u8 TIMA = gbRead(gb, IO_TIMA);
            
            u8 speed = TAC & 0x3;
            switch(speed) {
                case 0: if(gb->timer >= 1024) TIMA++; gb->timer = 0; break;
                case 1: if(gb->timer >= 16)   TIMA++; gb->timer = 0; break;
                case 2: if(gb->timer >= 64)   TIMA++; gb->timer = 0; break;
                case 3: if(gb->timer >= 256)  TIMA++; gb->timer = 0; break;
            }
            
            if(TIMA == 0x00) { // overflow
                TIMA = gbRead(gb, IO_TMA); // Reset it to TMA
                gbInterrupt(gb, INT_TIMER);
            }
            
            gbWrite(gb, IO_TIMA, TIMA);
            
        }
        
        // DMA
        if(gb->DMA_cycles_left >= 0) {
            u16 addr_read = gbRead(gb, 0xFF46) << 8;
            addr_read |= gb->DMA_cycles_left;
            u16 addr_write = 0xFE00 + gb->DMA_cycles_left;
            
            gbWrite(gb, addr_write, gbRead(gb, addr_read));
            gb->DMA_cycles_left--;
        }
        
        // Interrupts
        if(gb->ime) {
            u8 IF = gbRead(gb, IO_IF);
            u8 IE = gbRead(gb, IO_IE);
            
            for(u8 i = 0; i < 5; i++ ){
                if(IE & (1 << i) && IF & (1 << i)) {
                    IF = ~(1 << i) & IF;
                    gbWrite(gb, IO_IF, IF);
                    gb->ime = 0;
                    gbWrite(gb, --gb->sp, gb->pc >> 8 & 0xFF);
                    gbWrite(gb, --gb->sp, gb->pc & 0xFF);
                    gb->pc = 0x40 + i * 0x8;
                    SDL_LogVerbose(0, "Interrupt triggered %04X", gb->pc);
                }
            }
        }
        
        // Inputs
        u8 JOY = gbRead(gb, IO_JOY);
        
        const u8 *keyboard = SDL_GetKeyboardState(0);
        if(JOY & 1 << 5) { // Button
            JOY = 0b11100000;
            JOY |= !(keyboard[SDL_SCANCODE_A]) << 3; // B
            JOY |= !(keyboard[SDL_SCANCODE_S]) << 2; // A
            JOY |= !(keyboard[SDL_SCANCODE_Z]) << 1; // Select
            JOY |= !(keyboard[SDL_SCANCODE_X]) << 0; // Start
        }
        if(JOY & 1 << 4) { // D-Pad
            JOY = 0b11010000;
            JOY |= !(keyboard[SDL_SCANCODE_DOWN]) << 3;
            JOY |= !(keyboard[SDL_SCANCODE_UP]) << 2;
            JOY |= !(keyboard[SDL_SCANCODE_LEFT]) << 1;
            JOY |= !(keyboard[SDL_SCANCODE_RIGHT]) << 0;
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
                RenderText(renderer, x, *y, "%02X ", gbReadAt(gb, addr + j, 1));
                SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
                if(current_addr == gb->pc)
                    instruction_bytes = OPCODE_DESC_TABLE[gbReadAt(gb, gb->pc, 1)].bytes - 1;
            } else if (instruction_bytes > 0){
                SDL_SetRenderDrawColor(renderer, 200, 100, 100, 255);
                RenderText(renderer, x, *y, "%02X ", gbReadAt(gb, addr + j, 1));
                instruction_bytes--;
                SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
            } else {
                RenderText(renderer, x, *y, "%02X ", gbReadAt(gb, addr + j, 1));
            }
        }
        addr += 16;
        *y += TTF_FontHeight(global_font);
    }
}

void gbDrawDissassembly(Gameboy *gb, u32 *addr, u32 x, u32 *y, SDL_Renderer *renderer) {
    Opcode code = OPCODE_DESC_TABLE[gbReadAt(gb, *addr, 1)];
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
        RenderText(renderer, &x, *y, "%02X ", gbReadAt(gb, start_addr + i, 1));
    }
    u32 desc_x = op_x + 150;
    RenderText(renderer, &desc_x, *y, "%s ", code.desc);
    u32 operand_x = op_x + 300;
    for(u32 i = 1; i < code.bytes; i++){
        RenderText(renderer, &operand_x, *y, "%02X", gbReadAt(gb, start_addr + i, 1));
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
        u8 LCDC = gbRead(gb, 0xFF40);
        u8 STAT = gbRead(gb, 0xFF41);
        RenderLine(renderer, x, &y, "LCDC(FF40) %u%u%u%u%u%u%u%u %02X", LCDC >> 7 & 1, LCDC >> 6 & 1, LCDC >> 5 & 1, LCDC >> 4 & 1, LCDC >> 3 & 1, LCDC >> 2 & 1, LCDC >> 1 & 1, LCDC >> 0 & 1, LCDC);
        RenderLine(renderer, x, &y, "STAT(FF41) %u%u%u%u%u%u%u%u %02X", STAT >> 7 & 1, STAT >> 6 & 1, STAT >> 5 & 1, STAT >> 4 & 1, STAT >> 3 & 1, STAT >> 2 & 1, STAT >> 1 & 1, STAT >> 0 & 1, STAT);
        RenderLine(renderer, x, &y, "SC Y(FF42) %02X", gbReadAt(gb, 0XFF42, 1));
        RenderLine(renderer, x, &y, "SCX (FF43) %02X", gbReadAt(gb, 0XFF43, 1));
        RenderLine(renderer, x, &y, "LY  (FF44) %02X %u", gbReadAt(gb, 0XFF44, 1), gbReadAt(gb, 0XFF44, 1));
        RenderLine(renderer, x, &y, "DMA (FF46) %02X", gbReadAt(gb, 0xFF46, 1));
        RenderLine(renderer, x, &y, "DMA Timer  %02X", gb->DMA_cycles_left);
        RenderLine(renderer, x, &y, "----------------");
        RenderLine(renderer, x, &y, "LAST R     %04X", gb->last_read);
        RenderLine(renderer, x, &y, "LAST W     %04X", gb->last_write);
        RenderLine(renderer, x, &y, "TAC        %u%u%u", gbReadAt(gb, 0xFF06, 1));
        RenderLine(renderer, x, &y, "TIMA       %04X", gbReadAt(gb, 0xFF05, 1));
        
        
    }
    {
        u32 y = rect.h + 10;
        u32 x = 75;
        
        u8 IF = gbRead(gb, IO_IF);
        u8 IE = gbRead(gb, IO_IE);
        RenderLine(renderer, x, &y, "ime   %d", gb->ime);
        RenderLine(renderer, x, &y, "IF %u%u%u%u%u%u %02X", IF >> 5 & 1,IF >> 4 & 1, IF >> 3 & 1, IF >> 2 & 1, IF >> 1 & 1, IF >> 0 & 1, IF);
        RenderLine(renderer, x, &y, "IE %u%u%u%u%u%u %02X", IE >> 5 & 1, IE >> 4 & 1, IE >> 3 & 1, IE >> 2 & 1, IE >> 1 & 1, IE >> 0 & 1, IE);
        RenderLine(renderer, x, &y, "(hl)  %04X", gbReadAt(gb, gb->hl, 1));
        RenderLine(renderer, x, &y, "(sp)  %04X", gbReadAt(gb, gb->sp, 1));
        
        RenderLine(renderer, x, &y, " ");
        RenderLine(renderer, x, &y, "--------");
        RenderLine(renderer, x, &y, "CPU CLK  %d", gb->cpu_clock);
        RenderLine(renderer, x, &y, "PPU CLK  %d", gb->ppu_clock);
        RenderLine(renderer, x, &y, "GBL CLK  %d", gb->cycles_left);
        RenderLine(renderer, x, &y, "SPD   %.3fx", gb->clock_mul);
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
            for(u32 i = 0; i < 9; i ++) {
                gbDrawDissassembly(gb, &pc, x_section, &y_section, renderer);
            }
            RenderLine(renderer, x_section, &y_section, " ");
        }
        
        {   // ROM
            const u32 lines = 8;
            i32 start_addr = gb->pc - (gb->pc % 16) - (lines / 2) * 16;
            while(start_addr < 0) {
                start_addr += 16;
            }
            RenderLine(renderer, x_section, &y_section, "ROM");
            DebugDrawMemLines(gb, start_addr, lines, &x_section, &y_section, renderer, gb->pc);
        }
        
        {   // LAST READ
            const u32 lines = 8;
            i32 start_addr = gb->last_read - (gb->last_read % 16) - (lines / 2) * 16;
            while(start_addr < 0) {
                start_addr += 16;
            }
            u32 x = x_section_start;
            RenderLine(renderer, x, &y_section, "LAST READ");
            DebugDrawMemLines(gb, start_addr, lines, &x, &y_section, renderer, gb->last_read);
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
        {   // OAM
            RenderLine(renderer, x_section_start, &y_section, "OAM");
            DebugDrawMemLines(gb, MEM_OAM_START, MEM_OAM_SIZE / 16 + 1, &x_section, &y_section, renderer, gb->hl);
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
        {   // HL
            const u32 lines = 8;
            i32 start_addr = gb->hl - (gb->hl % 16) - (lines / 2) * 16;
            while(start_addr < 0) {
                start_addr += 16;
            }
            u32 x = x_section_start;
            RenderLine(renderer, x, &y_section, "HL");
            DebugDrawMemLines(gb, start_addr, lines, &x, &y_section, renderer, gb->hl);
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
            u8 LCDC = gbRead(gb, 0xFF40);
            bool tile_data_select = LCDC >> 4 & 1;
            u16 base_addr = 0x9800;
            for(u32 y = 0; y < 32; y++) {
                for(u32 x = 0; x < 32; x++) {
                    u16 addr = base_addr + x + y * 32;
                    u8 tile_id = gbReadAt(gb, addr, 1);
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
                RenderText(renderer, &x, y_section, "%02X", gbReadAt(gb, addr - i, 1));
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