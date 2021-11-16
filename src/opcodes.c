typedef struct Opcode {
    char desc[16];
    u8 bytes;
} Opcode;

Opcode OPCODE_DESC_TABLE[0X100] = {
    {"NOP",1},
    {"LD BC,d16",3},
    {"LD (BC),A",1},
    {"INC BC",1},
    {"INC B",1},
    {"DEC B",1},
    {"LD B,d8",2},
    {"RLCA",1},
    {"LD (a16),SP",3},
    {"ADD HL,BC",1},
    {"LD A,(BC)",1},
    {"DEC BC",1},
    {"INC C",1},
    {"DEC C",1},
    {"LD C,d8",2},
    {"RRCA",1},
    {"STOP",1},
    {"LD DE,d16",3},
    {"LD (DE),A",1},
    {"INC DE",1},
    {"INC D",1},
    {"DEC D",1},
    {"LD D,d8",2},
    {"RLA",1},
    {"JR r8",2},
    {"ADD HL,DE",1},
    {"LD A,(DE)",1},
    {"DEC DE",1},
    {"INC E",1},
    {"DEC E",1},
    {"LD E,d8",2},
    {"RRA",1},
    {"JR NZ,r8",2},
    {"LD HL,d16",3},
    {"LD (HL+),A",1},
    {"INC HL",1},
    {"INC H",1},
    {"DEC H",1},
    {"LD H,d8",2},
    {"DAA",1},
    {"JR Z,r8",2},
    {"ADD HL,HL",1},
    {"LD A,(HL+)",1},
    {"DEC HL",1},
    {"INC L",1},
    {"DEC L",1},
    {"LD L,d8",2},
    {"CPL",1},
    {"JR NC,r8",2},
    {"LD SP,d16",3},
    {"LD (HL-),A",1},
    {"INC SP",1},
    {"INC (HL)",1},
    {"DEC (HL)",1},
    {"LD (HL),d8",2},
    {"SCF",1},
    {"JR C,r8",2},
    {"ADD HL,SP",1},
    {"LD A,(HL-)",1},
    {"DEC SP",1},
    {"INC A",1},
    {"DEC A",1},
    {"LD A,d8",2},
    {"CCF",1},
    {"LD B,B",1},
    {"LD B,C",1},
    {"LD B,D",1},
    {"LD B,E",1},
    {"LD B,H",1},
    {"LD B,L",1},
    {"LD B,(HL)",1},
    {"LD B,A",1},
    {"LD C,B",1},
    {"LD C,C",1},
    {"LD C,D",1},
    {"LD C,E",1},
    {"LD C,H",1},
    {"LD C,L",1},
    {"LD C,(HL)",1},
    {"LD C,A",1},
    {"LD D,B",1},
    {"LD D,C",1},
    {"LD D,D",1},
    {"LD D,E",1},
    {"LD D,H",1},
    {"LD D,L",1},
    {"LD D,(HL)",1},
    {"LD D,A",1},
    {"LD E,B",1},
    {"LD E,C",1},
    {"LD E,D",1},
    {"LD E,E",1},
    {"LD E,H",1},
    {"LD E,L",1},
    {"LD E,(HL)",1},
    {"LD E,A",1},
    {"LD H,B",1},
    {"LD H,C",1},
    {"LD H,D",1},
    {"LD H,E",1},
    {"LD H,H",1},
    {"LD H,L",1},
    {"LD H,(HL)",1},
    {"LD H,A",1},
    {"LD L,B",1},
    {"LD L,C",1},
    {"LD L,D",1},
    {"LD L,E",1},
    {"LD L,H",1},
    {"LD L,L",1},
    {"LD L,(HL)",1},
    {"LD L,A",1},
    {"LD (HL),B",1},
    {"LD (HL),C",1},
    {"LD (HL),D",1},
    {"LD (HL),E",1},
    {"LD (HL),H",1},
    {"LD (HL),L",1},
    {"HALT",1},
    {"LD (HL),A",1},
    {"LD A,B",1},
    {"LD A,C",1},
    {"LD A,D",1},
    {"LD A,E",1},
    {"LD A,H",1},
    {"LD A,L",1},
    {"LD A,(HL)",1},
    {"LD A,A",1},
    {"ADD A,B",1},
    {"ADD A,C",1},
    {"ADD A,D",1},
    {"ADD A,E",1},
    {"ADD A,H",1},
    {"ADD A,L",1},
    {"ADD A,(HL)",1},
    {"ADD A,A",1},
    {"ADC A,B",1},
    {"ADC A,C",1},
    {"ADC A,D",1},
    {"ADC A,E",1},
    {"ADC A,H",1},
    {"ADC A,L",1},
    {"ADC A,(HL)",1},
    {"ADC A,A",1},
    {"SUB B",1},
    {"SUB C",1},
    {"SUB D",1},
    {"SUB E",1},
    {"SUB H",1},
    {"SUB L",1},
    {"SUB (HL)",1},
    {"SUB A",1},
    {"SBC A,B",1},
    {"SBC A,C",1},
    {"SBC A,D",1},
    {"SBC A,E",1},
    {"SBC A,H",1},
    {"SBC A,L",1},
    {"SBC A,(HL)",1},
    {"SBC A,A",1},
    {"AND B",1},
    {"AND C",1},
    {"AND D",1},
    {"AND E",1},
    {"AND H",1},
    {"AND L",1},
    {"AND (HL)",1},
    {"AND A",1},
    {"XOR B",1},
    {"XOR C",1},
    {"XOR D",1},
    {"XOR E",1},
    {"XOR H",1},
    {"XOR L",1},
    {"XOR (HL)",1},
    {"XOR A",1},
    {"OR B",1},
    {"OR C",1},
    {"OR D",1},
    {"OR E",1},
    {"OR H",1},
    {"OR L",1},
    {"OR (HL)",1},
    {"OR A",1},
    {"CP B",1},
    {"CP C",1},
    {"CP D",1},
    {"CP E",1},
    {"CP H",1},
    {"CP L",1},
    {"CP (HL)",1},
    {"CP A",1},
    
    {"RET NZ", 1}, // C0
    {"POP BC", 1},
    {"JP NZ,a16",3},
    {"JP a16",3},
    {"CALL NZ,a16",3},
    {"PUSH BC",1},
    {"ADD A,d8",2},
    {"RST 00H",1},
    {"RET Z",1},
    {"RET",1},
    {"JP Z,a16",3},
    {"PREFIX CB",2},
    {"CALL Z,a16",3},
    {"CALL a16",3},
    {"ADC A,d8",2},
    {"RST 08H",1}, // CF
    
    {"RET NC",1}, //D0
    {"POP DE",1},
    {"JP NC,a16",3},
    {"",0},
    {"CALL NC,a16",3},
    {"PUSH DE",1},
    {"SUB d8",2},
    {"RST 10H",1},
    {"RET C",1}, // D8
    {"RETI",1},
    {"JP C,a16",3},
    {"",0},
    {"CALL C,a16",3},
    {"",0},
    {"SBC A,d8",2},
    {"RST 18H",1}, // DF
    
    {"LDH (FF00+a8),A",2}, // E0
    {"POP HL",1},
    {"LD (0xFF00+C),A",2},
    {"",0},
    {"",0},
    {"PUSH HL",1},
    {"AND d8",2},
    {"RST 20H",1},
    {"ADD SP,r8",2}, // E8
    {"JP (HL)",1},
    {"LD (a16),A",3},
    {"",0},
    {"",0},
    {"",0},
    {"XOR d8",2},
    {"RST 28H",1}, // EF
    
    {"LDH A,(FF00+a8)",2}, // F0
    {"POP AF",1},
    {"LD A,(0xFF00+C)",2},
    {"DI",1},
    {"",0},
    {"PUSH AF",1},
    {"OR d8",2},
    {"RST 30H",1},
    {"LD HL,SP+r8",2}, // F8
    {"LD SP,HL",1},
    {"LD A,(a16)",3},
    {"EI",1},
    {"",0},
    {"",0},
    {"CP d8",2},
    {"RST 38H",1}, // FF
};

internal void gbPrefixCB(Gameboy *gb); 
internal void gbInstructionLD(Gameboy *gb, u8 op_code);

void gbExecute(Gameboy *gb) {
    
    u8 op_code = gbReadAt(gb, gb->pc, 0);
    switch(op_code) {
        case(0x00): { 
            gb->cpu_clock += 4;
            gb->pc++;
        } break;
        case (0x01) : { 
            gb->pc++;
            gb->c = gbReadAt(gb, gb->pc++, 0);
            gb->b = gbReadAt(gb, gb->pc++, 0);
            gb->cpu_clock += 12;
        } break;
        case (0x02): { 
            gb->cpu_clock += 4;
            gb->pc++;
            gbWrite(gb, gb->bc, gb->a);
        } break;
        case (0x03): {
            gb->cpu_clock += 4;
            gb->pc++;
            gb->c++;
            if(gb->c == 0x00)
                gb->b++;
        } break;
        case (0x04): {
            gb->cpu_clock += 4;
            gb->pc++;
            gb->b++;
            gbSetFlags(gb, gb->b == 0, 0, gb->b == 0, -1);
        } break;
        case (0x05): {
            gb->cpu_clock += 4;
            gb->pc++;
            gb->b--;
            gbSetFlags(gb, gb->b == 0, 1, gb->b == 0xFF, -1);
        } break;
        case (0x06): {
            gb->cpu_clock += 4;
            gb->pc++;
            gb->b = gbReadAt(gb, gb->pc++, 0);
        } break;
        case (0x07): {
            gb->cpu_clock += 4;
            gb->pc++;
            u8 bit = (gb->a & 0x80);
            gb->a <<= 1; 
            gb->a |= bit >> 7;
            gbSetFlags(gb, 0,0,0,bit >> 7);
        } break;
        case (0x08): {
            gb->cpu_clock += 20;
            gb->pc++;
            u16 addr = gbReadAt(gb, gb->pc++, 0);
            addr |= gbReadAt(gb, gb->pc++, 0) << 8;
            gbWrite(gb, addr, gb->sp & 0x00FF);
            gbWrite(gb, addr+1, (gb->sp & 0xFF00) >> 8);
        } break;
        case (0x09): {
            gb->cpu_clock += 8;
            gb->pc++;
            u32 val = gb->hl + gb->bc;
            gbSetFlags(gb, -1, 0, ((gb->hl & 0xFFF) + (gb->bc & 0xFFF)  > 0xFFF), val > 0xFFFF);
            gb->hl = val & 0xFFFF;
        } break;
        case (0x0A): { 
            gb->cpu_clock += 8;
            gb->pc++;
            gb->a = gbRead(gb, gb->bc);
        } break;
        case (0x0B): {
            gb->cpu_clock += 4;
            gb->pc++;
            gb->c--;
            if(gb->c == 0xFF)
                gb->b--;
        } break;
        case (0x0C): {
            gb->cpu_clock += 4;
            gb->pc++;
            gb->c++;
            gbSetFlags(gb, gb->c == 0, 0, gb->c == 0x00, -1);
        } break;
        case (0x0D): {
            gb->cpu_clock += 4;
            gb->pc++;
            gb->c--;
            gbSetFlags(gb, gb->c == 0, 1, gb->c == 0xFF, -1);
        } break;
        case (0x0E): { 
            gb->cpu_clock += 4;
            gb->pc++;
            gb->c = gbReadAt(gb, gb->pc++, 0);
        } break;
        case (0x0F): {
            gb->cpu_clock += 4;
            gb->pc++;
            u8 bit = (gb->a & 01);
            gb->a >>= 1;
            gb->a |= bit << 7;
            gbSetFlags(gb, 0, 0, 0, bit);
        } break;
        case(0x10): {
            gb->cpu_clock += 10;
            gb->pc++;
            gb->step_through = true;
            gb->halted = true;
            SDL_Log("Stop Instruction");
        } break;
        case (0x11) : { 
            gb->pc++;
            gb->e = gbReadAt(gb, gb->pc++, 0);
            gb->d = gbReadAt(gb, gb->pc++, 0);
            gb->cpu_clock += 12;
        } break;
        case (0x12): { 
            gb->cpu_clock += 4;
            gb->pc++;
            gbWrite(gb, gb->de, gb->a);
        } break;
        case (0x13): {
            gb->cpu_clock += 4;
            gb->pc++;
            gb->e++;
            if(gb->e == 0x00)
                gb->d++;
        } break;
        case (0x14): {
            gb->cpu_clock += 4;
            gb->pc++;
            gb->d++;
            gbSetFlags(gb, gb->d == 0, 0, gb->d == 0, -1);
        } break;
        case (0x15): {
            gb->cpu_clock += 4;
            gb->pc++;
            gb->d--;
            gbSetFlags(gb, gb->d == 0, 1, gb->d == 0xFF, -1);
        } break;
        case (0x16): {
            gb->cpu_clock += 4;
            gb->pc++;
            gb->d = gbReadAt(gb, gb->pc++, 0);
        } break;
        case (0x17): {
            gb->cpu_clock += 4;
            gb->pc++;
            u8 bit = (gb->a & 0x80);
            gb->a <<= 1; 
            gb->a |= bit >> 7;
            gbSetFlags(gb, 0,0,0, bit >> 7);
        } break;
        case (0x18): {
            gb->pc++;
            i8 offset = gbReadAt(gb, gb->pc++, 0);
            gb->pc += offset;
            gb->cpu_clock += 12;
        } break;
        case (0x19): {
            gb->cpu_clock += 8;
            gb->pc++;
            u32 val = gb->hl + gb->de;
            gbSetFlags(gb, -1, 0, ((gb->hl & 0xFFF) + (gb->de & 0xFFF)  > 0xFFF), val > 0xFFFF);
            gb->hl = val & 0xFFFF;
        } break;
        case (0x1A): { 
            gb->cpu_clock += 8;
            gb->pc++;
            gb->a = gbRead(gb, gb->de);
        } break;
        case (0x1B): {
            gb->cpu_clock += 4;
            gb->pc++;
            gb->e--;
            if(gb->e == 0xFF)
                gb->d--;
        } break;
        case (0x1C): {
            gb->cpu_clock += 4;
            gb->pc++;
            gb->e++;
            gbSetFlags(gb, gb->e == 0, 0, gb->e == 0x00, -1);
        } break;
        case (0x1D): {
            gb->cpu_clock += 4;
            gb->pc++;
            gb->e--;
            gbSetFlags(gb, gb->e == 0, 1, gb->e == 0xFF, -1);
        } break;
        case (0x1E): { 
            gb->cpu_clock += 4;
            gb->pc++;
            gb->e = gbReadAt(gb, gb->pc++, 0);
        } break;
        case (0x1F): {
            gb->cpu_clock += 4;
            gb->pc++;
            u8 bit = (gb->a & 0x1);
            gb->a >>= 1;
            gb->a |= ((gb->f & C_FLAG) != 0) << 7;
            gbSetFlags(gb, 0, 0, 0, bit);
        } break;
        case (0x20): {
            gb->pc++;
            i8 offset = gbReadAt(gb, gb->pc++, 0);
            if(!(gb->f & Z_FLAG)) {
                gb->pc += offset;
                gb->cpu_clock += 12;
            } else {
                gb->cpu_clock += 8;
            }
        } break;
        case (0x21): {
            gb->pc++;
            gb->l = gbReadAt(gb, gb->pc++, 0);
            gb->h = gbReadAt(gb, gb->pc++, 0);
            gb->cpu_clock += 4;
        } break;
        case (0x22): {
            gb->cpu_clock += 8;
            gb->pc++;
            gbWrite(gb, gb->hl, gb->a);
            gb->hl++;
        } break;
        case (0x23): {
            gb->cpu_clock += 4;
            gb->pc++;
            gb->l++;
            if(gb->l == 0x00)
                gb->h++;
        } break;
        case (0x24): {
            gb->cpu_clock += 4;
            gb->pc++;
            gb->h++;
            gbSetFlags(gb, gb->h == 0, 0, gb->h == 0, -1);
        } break;
        case (0x25): {
            gb->cpu_clock += 4;
            gb->pc++;
            gb->h--;
            gbSetFlags(gb, gb->h == 0, 1, gb->h == 0xFF, -1);
        } break;
        case (0x26): { 
            gb->cpu_clock += 4;
            gb->pc++;
            gb->h = gbReadAt(gb, gb->pc++, 0);
        } break;
        case (0x27): {
            gb->cpu_clock += 4;
            gb->pc++;
            u16 a = gb->a;
            u8 c = (gb->f & C_FLAG) != 0;
            if((gb->f & N_FLAG) == 0) {
                if(gb->f & H_FLAG || (a & 0x0F) > 0x09) {
                    a += 0x06;
                }
                if(c || (a & 0xFF0) > 0x90) {
                    a += 0x60;
                    c |= (a & 0xFF0) > 0x90;
                }
            } else {
                if(gb->f & H_FLAG) {
                    a -= 0x06;
                } 
                if(c) {
                    a = (a - 0x60) & 0xFF;
                }
            }
            u16 value = a;
            gb->a = value & 0xFF;
            gbSetFlags(gb, gb->a == 0, -1, 0, c);
        } break;
        case (0x28): {
            gb->pc++;
            i8 offset = gbReadAt(gb, gb->pc++, 0);
            if((gb->f & Z_FLAG)) {
                gb->pc += offset;
                gb->cpu_clock += 12;
            } else {
                gb->cpu_clock += 8;
            }
        } break;
        case (0x29): {
            gb->cpu_clock += 8;
            gb->pc++;
            u32 val = gb->hl + gb->hl;
            gbSetFlags(gb, -1, 0, ((gb->hl & 0xFFF) + (gb->hl & 0xFFF)  > 0xFFF), val > 0xFFFF);
            gb->hl = val & 0xFFFF;
        } break;
        case (0x2A): { 
            gb->cpu_clock += 8;
            gb->pc++;
            gb->a = gbRead(gb, gb->hl);
            gb->hl++;
        } break;
        case (0x2B): {
            gb->cpu_clock += 4;
            gb->pc++;
            gb->l--;
            if(gb->l == 0xFF)
                gb->h--;
        } break;
        case (0x2C): {
            gb->l++;
            gbSetFlags(gb, gb->l == 0, 0,  gb->l == 0, -1);
            gb->pc ++;
            gb->cpu_clock += 4;
        } break;
        case (0x2D): {
            gb->cpu_clock += 4;
            gb->pc++;
            gb->l--;
            gbSetFlags(gb, gb->l == 0, 1, gb->l == 0xFF, -1);
        } break;
        case (0x2E): { 
            gb->cpu_clock += 4;
            gb->pc++;
            gb->l = gbReadAt(gb, gb->pc++, 0);
        } break;
        case (0x2F): {
            gb->cpu_clock += 4;
            gb->pc++;
            gbXOR(gb, 0xFF);
            gbSetFlags(gb, -1, 1, 1, -1);
        } break;
        case (0x30): {
            gb->pc++;
            i8 offset = gbReadAt(gb, gb->pc++, 0);
            if(!(gb->f & C_FLAG)) {
                gb->pc += offset;
                gb->cpu_clock += 12;
            } else {
                gb->cpu_clock += 8;
            }
        } break;
        case(0x31): { 
            gb->pc++;
            u16 value = 0;
            value |= gbReadAt(gb, gb->pc++, 0);
            value |= gbReadAt(gb, gb->pc++, 0) << 8;
            gb->sp = value;
            gb->cpu_clock += 12;
        } break;
        case (0x32): { 
            gb->cpu_clock += 8;
            gb->pc++;
            gbWrite(gb, gb->hl, gb->a);
            gb->hl--;
        } break;
        case (0x33): {
            gb->cpu_clock += 4;
            gb->pc++;
            gb->sp++;
        } break;
        case (0x34): {
            gb->cpu_clock += 12;
            gb->pc++;
            
            u8 val = gbRead(gb, gb->hl);
            val++;
            gbWrite(gb, gb->hl, val);
            
            gbSetFlags(gb, val == 0, 0, val == 0, -1);
        } break;
        case (0x35): {
            gb->cpu_clock += 4;
            gb->pc++;
            u8 value = gbRead(gb, gb->hl);
            value--;
            gbWrite(gb, gb->hl, value);
            gbSetFlags(gb, value == 0, 1, value == 0xFF, -1);
        } break;
        case (0x36): { 
            gb->cpu_clock += 4;
            gb->pc++;
            gbWrite(gb, gb->hl, gbReadAt(gb, gb->pc++, 0));
        } break;
        case (0x37): {
            gb->cpu_clock += 4;
            gb->pc++;
            gbWrite(gb, IO_SCY, 1);
            gbSetFlags(gb, -1, 0, 0, 1);
        } break;
        case (0x38): {
            gb->pc++;
            i8 offset = gbReadAt(gb, gb->pc++, 0);
            if((gb->f & C_FLAG)) {
                gb->pc += offset;
                gb->cpu_clock += 12;
            } else {
                gb->cpu_clock += 8;
            }
        } break;
        case (0x39): {
            //gbBreakpoint(gb);
            gb->cpu_clock += 8;
            gb->pc++;
            u32 val = gb->hl + gb->sp;
            gbSetFlags(gb, -1, 0, ((gb->hl & 0xFFF) + (gb->sp & 0xFFF)  > 0xFFF), val > 0xFFFF);
            gb->hl = val & 0xFFFF;
        } break;
        case (0x3A): { 
            gb->cpu_clock += 8;
            gb->pc++;
            gb->a = gbRead(gb, gb->hl);
            gb->hl--;
        } break;
        case (0x3B): {
            gb->cpu_clock += 4;
            gb->pc++;
            gb->sp--;
        } break;
        case (0x3C): {
            gb->a++;
            gbSetFlags(gb, gb->a == 0, 0,  gb->a == 0, -1);
            gb->pc ++;
            gb->cpu_clock += 4;
        } break;
        case (0x3D): {
            gb->cpu_clock += 4;
            gb->pc++;
            gb->a--;
            gbSetFlags(gb, gb->a == 0, 1, gb->a == 0xFF, -1);
        } break;
        case (0x3E): { 
            gb->cpu_clock += 8;
            gb->pc++;
            gb->a = gbReadAt(gb, gb->pc++, 0);
        } break;
        case (0x3F): {
            gb->cpu_clock += 4;
            gb->pc++;
            u8 scy = gbRead(gb, IO_SCY);
            gbWrite(gb, IO_SCY, scy ^ 1);
            gbSetFlags(gb, -1, 0, 0, 1);
        } break;
        case(0x76): { 
            gb->halted = true;
            gb->pc++;
            gb->cpu_clock += 4;
        } break;
        case(0xC0): {
            gb->pc++;
            gb->cpu_clock += 8;
            if(!(gb->f & Z_FLAG)) {
                u16 pc = gbRead(gb, gb->sp++);
                pc |= gbRead(gb, gb->sp++) << 8;
                gb->pc = pc;
                gb->cpu_clock += 12;
            }
        } break;
        case(0xC1): {
            gb->pc++;
            gb->cpu_clock += 12;
            gb->c = gbRead(gb, gb->sp++);
            gb->b = gbRead(gb, gb->sp++);
        } break;
        case(0xC2): {
            u16 from = gb->pc;
            gb->pc++;
            u8 val1 = gbReadAt(gb, gb->pc++, 0);
            u8 val2 = gbReadAt(gb, gb->pc++, 0);
            u16 pc = val1 | val2 << 8;
            gb->cpu_clock += 12;
            if(!(gb->f & Z_FLAG)) {
                gb->pc = pc;
                gb->cpu_clock += 4;
                StackPush(&gb->call_stack, from, gb->pc);
            }
        } break;
        case(0xC3): {
            u16 pc = 0;
            pc |= gbReadAt(gb, gb->pc + 1, 0);
            pc |= gbReadAt(gb, gb->pc + 2, 0) << 8;
            u16 from = gb->pc;
            gb->pc = pc;
            gb->cpu_clock += 16;
            StackPush(&gb->call_stack, from, gb->pc);
        } break;
        case(0xC4): {
            u16 from = gb->pc;
            gb->pc++;
            u8 val1 = gbReadAt(gb, gb->pc++, 0);
            u8 val2 = gbReadAt(gb, gb->pc++, 0);
            gb->cpu_clock += 12;
            if(!(gb->f & Z_FLAG)) {
                u16 pc = val1 | val2 << 8;
                gbWrite(gb, --gb->sp, gb->pc >> 8 & 0xFF);
                gbWrite(gb, --gb->sp, gb->pc & 0xFF);
                gb->pc = pc;
                gb->cpu_clock += 4;
                StackPush(&gb->call_stack, from, gb->pc);
            }
        } break;
        case(0xC5): { 
            gb->pc++;
            gbWrite(gb, --gb->sp, gb->b);
            gbWrite(gb, --gb->sp, gb->c);
            gb->cpu_clock += 16;
        } break;
        case(0xC6): { 
            gb->pc++;
            gbADD(gb, gbReadAt(gb, gb->pc++, 0), 0);
            gb->cpu_clock += 8;
        } break;
        case(0xC7): {
            u16 from = gb->pc;
            gb->pc++;
            gbWrite(gb, --gb->sp, gb->pc >> 8 & 0xFF);
            gbWrite(gb, --gb->sp, gb->pc & 0xFF);
            gb->pc = 0x0000;
            StackPush(&gb->call_stack, from, 0x0000);
            gb->cpu_clock += 16;
        } break;
        case(0xC8): {
            u16 from = gb->pc;
            gb->pc++;
            gb->cpu_clock += 8;
            if((gb->f & Z_FLAG)) {
                u16 pc = gbRead(gb,gb->sp++);
                pc |= gbRead(gb,gb->sp++) << 8;
                gb->pc = pc;
                gb->cpu_clock += 12;
                StackPush(&gb->call_stack, from, gb->pc);
            }
        } break;
        case(0xC9): {
            u16 from = gb->pc;
            u16 pc = gbRead(gb,gb->sp++);
            pc |= gbRead(gb,gb->sp++) << 8;
            gb->pc = pc;
            gb->cpu_clock += 16;
            StackPush(&gb->call_stack, from, gb->pc);
        } break;
        case(0xCA): {
            u16 from = gb->pc;
            gb->pc++;
            u8 val1 = gbReadAt(gb, gb->pc++, 0);
            u8 val2 = gbReadAt(gb, gb->pc++, 0);
            
            u16 pc = val1 | val2 << 8;
            gb->cpu_clock += 12;
            if(gb->f & Z_FLAG) {
                gb->pc = pc;
                gb->cpu_clock += 4;
                StackPush(&gb->call_stack, from, gb->pc);
            }
        } break;
        case(0xCB): {
            gb->pc++;
            gbPrefixCB(gb);
        } break;
        case(0xCC): {
            u16 from = gb->pc;
            gb->pc++;
            u8 val1 = gbReadAt(gb, gb->pc++, 0);
            u8 val2 = gbReadAt(gb, gb->pc++, 0);
            gb->cpu_clock += 12;
            if(gb->f & Z_FLAG) {
                gbWrite(gb, --gb->sp, gb->pc >> 8 & 0xFF);
                gbWrite(gb, --gb->sp, gb->pc & 0xFF);
                u16 pc = val1 | val2 << 8;
                gb->pc = pc;
                gb->cpu_clock += 4;
                StackPush(&gb->call_stack, from, gb->pc);
            }
        } break;
        case(0xCD): {
            u16 from = gb->pc;
            gb->pc++;
            u8 val1 = gbReadAt(gb, gb->pc++, 0);
            u8 val2 = gbReadAt(gb, gb->pc++, 0);
            u16 pc = val1 | val2 << 8;
            gbWrite(gb, --gb->sp, gb->pc >> 8 & 0xFF);
            gbWrite(gb, --gb->sp, gb->pc & 0xFF);
            gb->pc = pc;
            gb->cpu_clock += 24;
            StackPush(&gb->call_stack, from, gb->pc);
        } break;
        case(0xCE): {
            gb->pc++;
            gbADD(gb, gbReadAt(gb, gb->pc++, 0), 1);
            gb->cpu_clock += 8;
        } break;
        case(0xCF): {
            u16 from = gb->pc;
            gb->pc++;
            gbWrite(gb, --gb->sp, gb->pc >> 8 & 0xFF);
            gbWrite(gb, --gb->sp, gb->pc & 0xFF);
            gb->pc = 0x0008;
            gb->cpu_clock += 16;
            StackPush(&gb->call_stack, from, gb->pc);
        } break;
        case(0xD0): {
            u16 from = gb->pc;
            gb->pc++;
            gb->cpu_clock += 8;
            if(!(gb->f & C_FLAG)) {
                u16 pc = gbRead(gb,gb->sp++);
                pc |= gbRead(gb,gb->sp++) << 8;
                gb->pc = pc;
                gb->cpu_clock += 12;
                StackPush(&gb->call_stack, from, gb->pc);
            }
        } break;
        case(0xD1): {
            gb->pc++;
            gb->cpu_clock += 12;
            gb->e = gbRead(gb,gb->sp++);
            gb->d = gbRead(gb,gb->sp++);
        } break;
        case(0xD2): {
            u16 from = gb->pc;
            gb->pc++;
            u8 val1 = gbReadAt(gb, gb->pc++, 0);
            u8 val2 = gbReadAt(gb, gb->pc++, 0);
            u16 pc = val1 | val2 << 8;
            gb->cpu_clock += 12;
            if(!(gb->f & C_FLAG)) {
                gb->pc = pc;
                gb->cpu_clock += 4;
                StackPush(&gb->call_stack, from, gb->pc);
            }
        } break;
        case(0xD4): {
            u16 from = gb->pc;
            gb->pc++;
            u8 val1 = gbReadAt(gb, gb->pc++, 0);
            u8 val2 = gbReadAt(gb, gb->pc++, 0);
            gb->cpu_clock += 12;
            if(!(gb->f & C_FLAG)) {
                gbWrite(gb, --gb->sp, gb->pc >> 8 & 0xFF);
                gbWrite(gb, --gb->sp, gb->pc & 0xFF);
                u16 pc = val1 | val2 << 8;
                gb->pc = pc;
                gb->cpu_clock += 4;
                StackPush(&gb->call_stack, from, gb->pc);
            }
        } break;
        case(0xD5): { 
            gb->pc++;
            gbWrite(gb, --gb->sp, gb->d);
            gbWrite(gb, --gb->sp, gb->e);
            gb->cpu_clock += 16;
        } break;
        case(0xD6): { 
            gb->pc++;
            gbSUB(gb, gbReadAt(gb, gb->pc++, 0), 0);
            gb->cpu_clock += 8;
        } break;
        case(0xD7): {
            u16 from = gb->pc;
            gb->pc++;
            gbWrite(gb, --gb->sp, gb->pc >> 8 & 0xFF);
            gbWrite(gb, --gb->sp, gb->pc & 0xFF);
            gb->pc = 0x0010;
            gb->cpu_clock += 16;
            StackPush(&gb->call_stack, from, gb->pc);
        } break;
        case(0xD8): {
            u16 from = gb->pc;
            gb->pc++;
            gb->cpu_clock += 8;
            if((gb->f & C_FLAG)) {
                u16 pc = gbRead(gb,gb->sp++);
                pc |= gbRead(gb,gb->sp++) << 8;
                gb->pc = pc;
                gb->cpu_clock += 12;
                StackPush(&gb->call_stack, from, gb->pc);
            }
        } break;
        case(0xD9): {
            u16 from = gb->pc;
            gb->pc++;
            gb->cpu_clock += 16;
            
            u16 pc = gbRead(gb,gb->sp++);
            pc |= gbRead(gb,gb->sp++) << 8;
            gb->pc = pc;
            StackPush(&gb->call_stack, from, gb->pc);
            gb->ime = 1;
        } break;
        case(0xDA): {
            u16 from = gb->pc;
            gb->pc++;
            u8 val1 = gbReadAt(gb, gb->pc++, 0);
            u8 val2 = gbReadAt(gb, gb->pc++, 0);
            u16 pc = val1 | val2 << 8;
            gb->cpu_clock += 12;
            if(gb->f & C_FLAG) {
                gb->pc = pc;
                gb->cpu_clock += 4;
                StackPush(&gb->call_stack, from, gb->pc);
            }
        } break;
        case(0xDC): {
            u16 from = gb->pc;
            gb->pc++;
            u8 val1 = gbReadAt(gb, gb->pc++, 0);
            u8 val2 = gbReadAt(gb, gb->pc++, 0);
            gb->cpu_clock += 12;
            if(gb->f & C_FLAG) {
                gbWrite(gb, --gb->sp, gb->pc >> 8 & 0xFF);
                gbWrite(gb, --gb->sp, gb->pc & 0xFF);
                u16 pc = val1 | val2 << 8;
                gb->pc = pc;
                gb->cpu_clock += 4;
                StackPush(&gb->call_stack, from, gb->pc);
            }
        } break;
        case(0xDE): {
            gb->pc++;
            gbSUB(gb, gbReadAt(gb, gb->pc++, 0), 1);
            gb->cpu_clock += 8;
        } break;
        case(0xDF): {
            u16 from = gb->pc;
            gb->pc++;
            gbWrite(gb, --gb->sp, gb->pc >> 8 & 0xFF);
            gbWrite(gb, --gb->sp, gb->pc & 0xFF);
            gb->pc = 0x0018;
            gb->cpu_clock += 16;
            StackPush(&gb->call_stack, from, gb->pc);
        } break;
        case(0xE0): { 
            gb->pc++;
            u8 offset = gbReadAt(gb, gb->pc++, 0);
            gbWrite(gb, 0xFF00 + offset, gb->a);
            gb->cpu_clock += 12;
        } break;
        case(0xE1): {
            gb->pc++;
            gb->cpu_clock += 12;
            gb->l = gbRead(gb,gb->sp++);
            gb->h = gbRead(gb,gb->sp++);
        } break;
        case(0xE2): {
            gbWrite(gb, gb->c + 0xFF00, gb->a);
            gb->pc++;
            gb->cpu_clock += 8;
        } break;
        case(0xE5): { 
            gb->pc++;
            gbWrite(gb, --gb->sp, gb->h);
            gbWrite(gb, --gb->sp, gb->l);
            gb->cpu_clock += 16;
        } break;
        case(0xE6): { 
            gb->pc++;
            gbAND(gb, gbReadAt(gb, gb->pc++, 0));
            gb->cpu_clock += 8;
        } break;
        case(0xE7): {
            u16 from = gb->pc;
            gb->pc++;
            gbWrite(gb, --gb->sp, gb->pc >> 8 & 0xFF);
            gbWrite(gb, --gb->sp, gb->pc & 0xFF);
            gb->pc = 0x0020;
            gb->cpu_clock += 16;
            StackPush(&gb->call_stack, from, gb->pc);
        } break;
        case(0xE8): {
            //gbBreakpoint(gb);
            gb->pc++;
            gb->cpu_clock += 16;
            
            i8 add = gbReadAt(gb, gb->pc++, 0);
            u32 result = gb->sp + add;
            
            gbSetFlags(gb, 0, 0, ((gb->sp & 0xF) + (add & 0xF)) > 0xF , ((gb->sp & 0xFF) + (add & 0xFF)) > 0xFF);
            gb->sp = result & 0xFFFF;
        } break;
        case(0xE9): {
            u16 from = gb->pc;
            gb->pc = gb->hl;
            gb->cpu_clock += 4;
            StackPush(&gb->call_stack, from, gb->pc);
        } break;
        case(0xEA): { 
            gb->pc++;
            u16 addr = 0;
            addr |= gbReadAt(gb, gb->pc++, 0);
            addr |= gbReadAt(gb, gb->pc++, 0) << 8;
            gbWrite(gb, addr, gb->a);
            gb->cpu_clock += 16;
        } break;
        case(0xEE): {
            gb->pc++;
            gbXOR(gb, gbReadAt(gb, gb->pc++, 0));
            gb->cpu_clock += 8;
        } break;
        case(0xEF): {
            u16 from = gb->pc;
            gb->pc++;
            gbWrite(gb, --gb->sp, gb->pc >> 8 & 0xFF);
            gbWrite(gb, --gb->sp, gb->pc & 0xFF);
            gb->pc = 0x0028;
            gb->cpu_clock += 16;
            StackPush(&gb->call_stack, from, gb->pc);
        } break;
        case(0xF0): { 
            gb->pc++;
            u8 offset = gbReadAt(gb, gb->pc++, 0);
            gb->a = gbRead(gb, 0xFF00 + offset);
            gb->cpu_clock += 12;
        } break;
        case(0xF1): {
            gb->pc++;
            gb->cpu_clock += 12;
            gb->f = gbRead(gb,gb->sp++);
            gb->a = gbRead(gb,gb->sp++);
            gb->f &= 0xF0;
        } break;
        case(0xF2): {
            u8 value = gbRead(gb, gb->c + 0xFF00);
            gb->a = value;
            gb->pc++;
            gb->cpu_clock += 8;
        } break;
        case(0xF3): { 
            gb->pc++;
            gb->ime = 0;
            gb->cpu_clock += 4;
        } break;
        case(0xF5): { 
            gb->pc++;
            gbWrite(gb, --gb->sp, gb->a);
            gbWrite(gb, --gb->sp, gb->f);
            gb->cpu_clock += 16;
        } break;
        case(0xF6): { 
            gb->pc++;
            gbOR(gb, gbReadAt(gb, gb->pc++, 0));
            gb->cpu_clock += 8;
        } break;
        case(0xF7): {
            u16 from = gb->pc;
            gb->pc++;
            gbWrite(gb, --gb->sp, gb->pc >> 8 & 0xFF);
            gbWrite(gb, --gb->sp, gb->pc & 0xFF);
            gb->pc = 0x0030;
            gb->cpu_clock += 16;
            StackPush(&gb->call_stack, from, gb->pc);
        } break;
        case(0xF8): {
            gb->pc++;
            gb->cpu_clock += 16;
            i8 add = gbReadAt(gb, gb->pc++, 0);
            u32 result = gb->sp + add;
            
            gbSetFlags(gb, 0, 0, ((gb->sp & 0xF) + (add & 0xF)) > 0xF , ((gb->sp & 0xFF) + (add & 0xFF)) > 0xFF);
            gb->hl = result & 0xFFFF;
        } break;
        case(0xF9): {
            gb->pc++;
            gb->cpu_clock += 8;
            gb->sp = gb->hl;
        } break;
        case(0xFA): { 
            gb->pc++;
            gb->cpu_clock += 16;
            u16 addr = gbReadAt(gb, gb->pc++, 0);
            addr |= gbReadAt(gb, gb->pc++, 0) << 8;
            gb->a = gbRead(gb, addr);
        } break;
        case(0xFB): { 
            gb->pc++;
            gb->ime = 1;
            gb->cpu_clock += 4;
        } break;
        case(0xFC): { 
            gb->pc++;
            gb->cpu_clock += 4;
            gb->ime = 1;
        } break;
        case(0xFE): { 
            gb->pc++;
            gb->cpu_clock += 4;
            gbCP(gb, gbReadAt(gb, gb->pc++, 0));
        } break;
        case(0xFF): {
            u16 from = gb->pc;
            gb->pc++;
            gbWrite(gb, --gb->sp, gb->pc >> 8 & 0xFF);
            gbWrite(gb, --gb->sp, gb->pc & 0xFF);
            gb->pc = 0x0038;
            gb->cpu_clock += 16;
            StackPush(&gb->call_stack, from, gb->pc);
        } break;
        default :
        if(op_code >= 0x40 && op_code <= 0x7F) { // LD
            gbInstructionLD(gb, op_code);
            gb->pc++;
            gb->cpu_clock += 4;
        } else if(op_code >= 0x80 && op_code <= 0xBF) { // MATHS
            u8 b2 = (op_code & 0xF);
            u8 operand = *gbGetRegisterFromID(gb, b2 % 8);
            u8 b1 = (op_code >> 4) & 0xF - 0x8;
            b1 *= 2;
            if (b2 > 7) {
                b1 += 1;
            }
            switch(b1) {
                case(0) : gbADD(gb, operand, 0); break;
                case(1) : gbADD(gb, operand, 1); break;
                case(2) : gbSUB(gb, operand, 0); break;
                case(3) : gbSUB(gb, operand, 1); break;
                case(4) : gbAND(gb, operand);    break;
                case(5) : gbXOR(gb, operand);    break;
                case(6) : gbOR (gb, operand);    break;
                case(7) : gbCP (gb, operand); break;
            }
            gb->cpu_clock += 4;
            gb->pc++;
            
        } else {
            SDL_Log("Unimplemented opcode : %02X", gbReadAt(gb, gb->pc, 1));
            gb->cpu_clock += 1;
            gb->step_through = true;
        }
        break;
    }
}

void gbInstructionLD(Gameboy *gb, u8 op_code) {
    
    u8 b1 = (op_code >> 4) - 4;
    u8 b2 = op_code & 0xF;
    b1 *= 2;
    if (b2 > 7) {
        b1 += 1;
    }
    b2 = b2 % 8;
    
    u8 *r1 = gbGetRegisterFromID(gb, b1);
    u8 *r2 = gbGetRegisterFromID(gb, b2);
    *r1 = *r2;
}


void gbPrefixCB(Gameboy *gb) {
    u8 code = gbReadAt(gb, gb->pc++, 0);
    u8 *src = gbGetRegisterFromID(gb, (code & 0x0F) % 8);
    
    if((code & 0x0F) % 8 == 6) 
        gb->cpu_clock += 8;
    
    bool above_8 = (code & 0x0F) >= 8;
    u8 first = code >> 4 & 0x0F;
    
    i8 Z = 0;
    i8 N = 0;
    i8 H = 0;
    i8 C = 0;
    
    switch(first) {
        case(0) :{ // RLC -- RRC
            if(!above_8) {
                u8 bit = (*src & 0x80);
                *src <<= 1; 
                *src |= bit >> 7;
                C = bit >> 7;
            } else {
                u8 bit = (*src & 0x01);
                *src >>= 1;
                *src |= bit << 7;
                C = bit;
            }
            Z = *src == 0;
        } break;
        case(1) :{ // RL -- 
            if(!above_8) {
                u8 bit = (*src & 0x80);
                *src <<= 1; 
                C = bit >> 7;
                *src |= (gb->f & C_FLAG) == 1;
            } else { // RR
                u8 bit = (*src & 0x01);
                *src >>= 1;
                C = bit;
                *src |= ((gb->f & C_FLAG) != 0) << 7;
            }
            Z = *src == 0;
        } break;
        case(2) :{ // SLA 
            if(!above_8) {
                u8 bit = (*src & 0x80);
                *src <<= 1; 
                C = bit >> 7;
            } else { // SRA
                u8 bit = (*src & 0x80);
                C = *src & 0x01;
                *src >>= 1;
                *src |= bit << 7;
                
            }
            Z = *src == 0;
        } break;
        case(3) : {
            if(!above_8) { // swap
                u8 high = *src & 0xF0;
                u8 low = *src & 0x0F;
                *src = (high >> 4) | (low << 4);
            } else { // SRL
                u8 bit = (*src & 1);
                *src >>= 1; 
                C = bit;
            }
            Z = *src == 0;
        } break;
        default : 
        if(first >= 0x4 && first <= 0x7) { // BIT
            u8 bit = (first - 0x4) * 2 + above_8;
            Z = (((*src >> bit) & 1) == 0);
            N = 0;
            H = 1;
            C = -1;
        } else if(first >= 0x8 && first <= 0xB) { // RES
            u8 bit = (first - 0x8) * 2 + above_8;
            
            *src &= ~(1 << bit);
            
            Z = -1;
            N = -1;
            H = -1;
            C = -1;
        } else { // SET
            u8 bit = (first - 0xC) * 2 + above_8;
            
            *src |= 1 << bit;
            
            Z = -1;
            N = -1;
            H = -1;
            C = -1;
        }
        break;
    }
    gbSetFlags(gb, Z, N, H, C);
    gb->cpu_clock += 8;
}