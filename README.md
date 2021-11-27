Just another GameBoy emulator made for fun and learning. The code base is pretty messy, and there's still a lof of work to do to make it fully functionnal. But it can currently play Tetris' start screen properly (that bit where the game plays on its own). It also features a full memory map view of the key places, breakpoints, and dissassembly.
Run fairly smoothly on my machine. No optimization done at all. 

# Use 
- Press F1 to open a rom

### Input
- A : A
- B : S
- Start : X
- Select : W
- Up : Up Arrow
- Down : Down Arrow
- Left : Left Arrow
- Right : Right Arrow
- Space : Pause/Resume execution

### Other
- Press F5 to toggle debug mode
- N/F7 : Run to next intruction
- Space/F9 : Toggle execution
- Click on a line in the disassembly to set a breakpoint or edit bp.txt next to the .exe and add your addresses

# Roadmap
### Blargg's Tests Progress
 - [X] CPU Instructions 
     - [X] 01-special
     - [X] 02-interrupts
     - [X] 03-op sp,hl
     - [X] 04-op r,imm
     - [X] 05-op rp
     - [X] 06-ld r,r
     - [X] 07-jr,jp,call,ret,rst
     - [X] 08-misc instrs
     - [X] 09-op r,r
     - [X] 10-bit ops
     - [X] 11-op a,(hl)
 - [ ] Instruction Timing
 - [ ] Memory Timing
 - [ ] DMG Soung

### MoonEye Tests 
 - [ ] Acceptance
    - [ ] Timer
        - [X] div_write
        - [ ] rapid_toggle
        - [X] tim00
        - [ ] tim00_div_trigger
        - [X] tim01
        - [ ] tim01_div_trigger
        - [X] tim10
        - [ ] tim10_div_trigger
        - [X] tim11
        - [ ] tim11_div_trigger
        - [ ] tima_reload
        - [ ] time_write_reloading
        - [ ] tma_write_reloading

### Features
- [ ] Audio
- [ ] More MBC Types
- [ ] GBC modes

# Screenshots
![1](https://github.com/Sl3dge78/gb_emu/blob/main/screenshots/1.png?raw=true)  
![1](https://github.com/Sl3dge78/gb_emu/blob/main/screenshots/2.png?raw=true)  
