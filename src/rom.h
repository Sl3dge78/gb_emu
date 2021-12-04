#pragma once 

const u16 ROM_0_START    = 0x0000;
const u16 ROM_0_END      = 0x3FFF;
const u16 ROM_BANK_START = 0x4000;
const u16 ROM_BANK_END   = 0x7FFF;
const u16 ROM_RAM_START  = 0xA000;
const u16 ROM_RAM_END    = 0xBFFF;

const u8 supported_cartridges[] = {
    0x00,
    0x01,
    0x02,
    0x03,
};


typedef struct Rom {
    char *path;
    u8 type;

    u8 *rom_data;
    u32 rom_size;

    u8 *ram_data;
    u32 ram_size;

    u8 rom_bank;
    u8 ram_bank;
    bool bank_mode;

} Rom;

bool RomCheckForCartridgeCompatibility(u8 cartridge_type);
void RomLoad(Rom *rom, const char* path);
void RomReset(Rom *rom);
void RomUnload(Rom *rom);
u8   RomRead(Rom *rom, u16 address);
void RomWrite(Rom *rom,u16 address, u8 value);
void RomDestroy(Rom *rom);
