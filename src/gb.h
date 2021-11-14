#pragma once

#define SCREEN_WIDTH  160
#define SCREEN_HEIGHT 144

#define ROM_ENTRY     0x100
#define ROM_LOGO      0x104
#define ROM_LOGO_END  0x133
#define ROM_LOGO_SIZE ROM_LOGO_END - ROM_LOGO
#define ROM_TITLE     0x134
#define ROM_MANUF     0x13F
#define ROM_CGB       0x143
#define ROM_LICENSEE  0x144
#define ROM_SGB       0x146
#define ROM_CARTRIDGE 0x147
#define ROM_ROM_SIZE  0x148
#define ROM_RAM_SIZE  0x149
#define ROM_DEST_CODE 0x14A
#define ROM_OLD_LICEN 0x14B
#define ROM_ROM_VER   0x14C
#define ROM_HDR_CHKSM 0x14D
#define ROM_GBL_CHKSM 0x14E

#define IO_JOY  0xFF00
#define IO_DIV  0xFF04
#define IO_TIMA 0xFF05
#define IO_TMA  0xFF06
#define IO_TAC  0xFF07

#define IO_LCDC   0xFF40
#define IO_STAT   0xFF41
#define IO_SCY    0xFF42
#define IO_SCX    0xFF43
#define IO_LY     0xFF44
#define IO_LYC    0xFF45
#define IO_DMA    0xFF46
#define IO_BGP    0xFF47
#define IO_OBP0   0xFF48
#define IO_OBP1   0xFF49
#define IO_WY     0xFF4A
#define IO_WX     0xFF4B
#define IO_IF     0xFF0F
#define IO_IE     0xFFFF

#define INT_VBLANK  0
#define INT_LCDSTAT 1
#define INT_TIMER   2
#define INT_SERIAL  3
#define INT_JOYPAD  4

const u16 MEM_ROM00_START   = 0x0000;
const u16 MEM_ROM00_END     = 0x3FFF;
const u16 MEM_ROM00_SIZE    = MEM_ROM00_END - MEM_ROM00_START + 1;
const u16 MEM_ROMNN_START   = 0x4000;
const u16 MEM_ROMNN_END     = 0x7FFF;
const u16 MEM_ROMNN_SIZE    = MEM_ROMNN_END - MEM_ROMNN_START + 1;
const u16 MEM_VRAM_START  = 0x8000;
const u16 MEM_VRAM_END    = 0x9FFF;
const u16 MEM_VRAM_SIZE   = MEM_VRAM_END - MEM_VRAM_START + 1;
const u16 MEM_CARTRAM_START  = 0xA000;
const u16 MEM_CARTRAM_END    = 0xBFFF;
const u16 MEM_CARTRAM_SIZE   = MEM_CARTRAM_END - MEM_CARTRAM_START + 1;
const u16 MEM_WRAM0_START = 0xC000;
const u16 MEM_WRAM0_END   = 0xCFFF;
const u16 MEM_WRAM0_SIZE  = MEM_WRAM0_END - MEM_WRAM0_START + 1;
const u16 MEM_WRAM1_START = 0xD000;
const u16 MEM_WRAM1_END   = 0xDFFF;
const u16 MEM_WRAM1_SIZE  = MEM_WRAM1_END - MEM_WRAM1_START + 1;
const u16 MEM_MIRROR0_START = 0xE000;
const u16 MEM_MIRROR0_END   = 0xEFFF;
const u16 MEM_MIRROR0_SIZE  = MEM_MIRROR0_END - MEM_MIRROR0_START + 1;
const u16 MEM_MIRROR1_START = 0xEFFF;
const u16 MEM_MIRROR1_END   = 0xFDFF;
const u16 MEM_MIRROR1_SIZE  = MEM_MIRROR1_END - MEM_MIRROR1_START + 1;
const u16 MEM_OAM_START   = 0xFE00;
const u16 MEM_OAM_END     = 0xFE9F;
const u16 MEM_OAM_SIZE    = MEM_OAM_END - MEM_OAM_START + 1;
const u16 MEM_UNUSABLE_START   = 0xFEA0;
const u16 MEM_UNUSABLE_END     = 0xFEFF;
const u16 MEM_UNUSABLE_SIZE    = MEM_UNUSABLE_END - MEM_UNUSABLE_START + 1;
const u16 MEM_IO_START    = 0xFF00;
const u16 MEM_IO_END      = 0xFF7F;
const u16 MEM_IO_SIZE     = MEM_IO_END - MEM_IO_START + 1;
const u16 MEM_HRAM_START  = 0xFF80;
const u16 MEM_HRAM_END    = 0xFFFF;
const u16 MEM_HRAM_SIZE   = MEM_HRAM_END - MEM_HRAM_START + 1;
const u16 MEM_END         = 0xFFFF;
const u32 MEM_SIZE        = 0x10000;

typedef struct Color {
    u8 r;
    u8 g;
    u8 b;
} Color;

const Color palette[4] = {
    {0xE0, 0xF8, 0xD0},
    {0x88, 0xC0, 0x70},
    {0x34, 0x68, 0x56},
    {0x08, 0x18, 0x20},
};

#define LOG_OPCODE SDL_LOG_CATEGORY_CUSTOM

u8 Z_FLAG = 1 << 7;
u8 N_FLAG = 1 << 6;
u8 H_FLAG = 1 << 5; 
u8 C_FLAG = 1 << 4;

typedef struct Stack {
    u16 *from;
    u16 *to;
    u32 capacity;
} Stack;

void StackPush(Stack *stack, u16 from, u16 to);

typedef struct Gameboy {
    // Memory
    u8 *rom;
    u8 *mem;
    u8 ime;
    
    // Registers
    union {
        struct {
            u8 f;
            u8 a;
        };
        u16 af;
    };
    union {
        struct {
            u8 c;
            u8 b;
        };
        u16 bc;
    };
    union {
        struct {
            u8 e;
            u8 d;
        };
        u16 de;
    };
    union {
        struct {
            u8 l;
            u8 h;
        };
        u16 hl;
    };
    
    u16 sp;
    u16 pc;
    
    u8 cartridge_type;
    u8 rom_size;
    u8 ram_size;
    u8 rom_bank;
    u8 ram_bank;
    u8 ram_bank_mode;
    
    u8 *cart_ram;
    
    u16 timer;
    
    u8 keys_dpad;
    u8 keys_buttons;
    
    // Our stuff
    i32 cpu_clock;
    i32 ppu_clock;
    i32 cycles_left;
    i16 DMA_cycles_left;
    bool running;
    bool step_through;
    u64 clock_speed;
    bool halted;
    
    u16 breakpoints[16];
    u16 breakpoint_count;
    
    u8 lcd_screen [SCREEN_HEIGHT][SCREEN_WIDTH];
    
    u16 last_write;
    u16 last_read;
    f32 clock_mul;
    
    Stack call_stack;
} Gameboy;

#define gbWrite(gb, addr, value) gbWriteAt(gb, addr, value, 1)
void gbWriteAt(Gameboy *gb, const u16 address, const u8 value, bool log);

u8 gbReadAt(Gameboy *gb, const u16 address, bool debug);
#define gbRead(gb, add) gbReadAt(gb, add, 1)

u8 *gbGetPointerTo(Gameboy *gb, const u16 address);
u8 *gbGetRegisterFromID(Gameboy *gb, u8 id);

void gbSetFlags(Gameboy *gb, i32 Z, i32 N, i32 H, i32 C);
void gbADD(Gameboy *gb, u8 operand, u8 carry);
void gbSUB(Gameboy *gb, u8 operand, u8 carry);
void gbAND(Gameboy *gb, u8 operand);
void gbXOR(Gameboy *gb, u8 operand);
void gbOR(Gameboy *gb, u8 operand);
void gbCP(Gameboy *gb, u8 operand);

void gbBreakpoint(Gameboy *gb);