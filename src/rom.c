
bool RomCheckForCartridgeCompatibility(u8 cartridge_type) {
    for(u32 i = 0; i < sizeof(supported_cartridges); i++) {
        if(supported_cartridges[i] == cartridge_type)
            return true;
    }
    return false;
}

void RomGetSavePath(Rom *rom, char *save_path, u32 str_size) {
    strncpy(save_path, rom->path, 256);
    char *dot = strrchr(save_path, '.');
    if(dot) {
        strncpy(dot, ".sav", 4);
    }
}

void RomSaveRam(Rom *rom) {
    char save_path[256];
    RomGetSavePath(rom, save_path, 256);
    FILE *ram_file = fopen(save_path, "wb");
    if(ram_file) {
        fwrite(rom->ram_data, sizeof(u8), rom->ram_size, ram_file);
        fclose(ram_file);
        SDL_Log("Saved at at %s", save_path);
    }
}

void RomLoadRam(Rom *rom) {
    char save_path[256];
    RomGetSavePath(rom, save_path, 256);
    FILE *ram_file = fopen(save_path, "rb");
    if(ram_file) {
        fseek(ram_file, 0, SEEK_END);
        u32 size = ftell(ram_file);
        rewind(ram_file);
        if(size <= rom->ram_size) {
            fread(rom->ram_data, 1, rom->ram_size, ram_file);
        }
        fclose(ram_file);
    } else {
        SDL_Log("No save data found at %s", save_path);
    }
}

void RomLoad(Rom *rom, const char *path) {
    rom->path = path;
    FILE *rom_file = fopen(path, "rb");
    
    fseek(rom_file, 0, SEEK_END);
    rom->rom_size = ftell(rom_file);
    rewind(rom_file);
    
    rom->rom_data = calloc(rom->rom_size, sizeof(u8));
    
    fread(rom->rom_data, 1, rom->rom_size, rom_file);
    fclose(rom_file);
    SDL_Log("Rom size is %d", rom->rom_size);
    
    rom->type = RomRead(rom, 0x0147);
    if(!RomCheckForCartridgeCompatibility(rom->type)) {
        SDL_Log("Unsupported cartridge type 0x%02X. Some things might not work", rom->type);
    }

    u8 ram_size_value = RomRead(rom, 0x0149);
    switch(ram_size_value) {
        case 0: rom->ram_size = 0; break;
        case 1: rom->ram_size = 2048; break;
        case 2: rom->ram_size = 8192; break;
        case 3: rom->ram_size = 32768; break;
        case 4: rom->ram_size = 131072; break;
        case 5: rom->ram_size = 65536; break;
        default : assert(0); break;
    }
    if(rom->ram_size != 0)
        rom->ram_data = calloc(1, rom->ram_size);
    SDL_Log("Ram size is %d", rom->ram_size);
    
    // Load save if any
    RomLoadRam(rom);
    RomReset(rom);
}

void RomReset(Rom *rom) {
    rom->bank_mode = 0;
    rom->rom_bank = 1;
    rom->ram_bank = 0;
}

void RomWrite(Rom *rom, u16 address, u8 value) {
    // RAM 
    if(address >= ROM_RAM_START && address <= ROM_RAM_END) {
        if(rom->ram_size > 0) {
            if(rom->type != 0x05 && rom->type != 0x06) { // @todo MBC 2 has a weird type of ram
                u16 offset = rom->ram_bank * 0x2000;
                u16 relative_address = address - ROM_RAM_START;
                rom->ram_data[relative_address + offset] = value;
                return;
            } else {
                assert(0);
                return;
            }
        }
    }

    // We're writing in special registers, their behaviour vary depending on the mbc type.
    switch(rom->type) {
        case(0x0): { // ROM only
            // Do nothing you can't write here
            return;
        } break;
        case (0x1):
        case (0x2):
        case (0x3): { // MBC 1
            if(address >= 0x0000 && address <= 0x1FFF) { // RAM Enable
                if(value == 0) { // We're diabling Ram so save it to file
                    RomSaveRam(rom);
                    SDL_Log("Saving ram");
                }
                // @todo Not simulated 
                return;
            } else if (address >= 0x2000 && address <= 0x2FFF) { // ROM bank
                rom->rom_bank = value & 0b00011111;
                if(rom->rom_bank == 0 || rom->rom_bank == 0x20 || rom->rom_bank == 0x40 || rom->rom_bank == 0x60) {
                    rom->rom_bank++;
                }
                return;
            } else if (address >= 0x4000 && address <= 0x5FFF) { // RAM Bank or Upper ROM Bank
                if(rom->bank_mode == 0x1) {
                    rom->ram_bank = value & 0b00000011;
                } else {
                    rom->rom_bank = (rom->rom_bank & 0b00011111) | ((value & 0b00000011) << 5);
                }
            } else if (address >= 0x6000 && address <= 0x7FFF) { // Bank mode
                rom->bank_mode = value & 1;
            } else {
                assert(0);
                return; // Unreacheable
            }
        } break;
    }
}

u8 RomRead(Rom *rom, u16 address) {
    if (address <= ROM_0_END) {
        return rom->rom_data[address];
    }
    if(address >= ROM_BANK_START && address <= ROM_BANK_END) {
        u32 offset = rom->rom_bank * 0x4000;
        u32 base_address = address - ROM_BANK_START;
        return rom->rom_data[base_address + offset];
    }
    
    if(address >= ROM_RAM_START && address <= ROM_RAM_END) {
        if(rom->ram_size > 0) {
            u32 offset = rom->ram_bank * 0x2000;
            u32 relative_address = address - ROM_RAM_START;
            return rom->ram_data[relative_address + offset];
        }       
    }
    assert(0);
    return 0;
}
