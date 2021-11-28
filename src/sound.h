#define M_PI2 (3.1415926 * 2.0)

#define IO_NR21 0xFF16 // DDLLLLLL	D=Wave pattern Duty L=Length 
#define IO_NR22 0xFF17 // VVVVDNNN	C1 Volume / Direction 0=down / envelope Number (fade speed)
#define IO_NR23 0xFF18 // LLLLLLLL	pitch L
#define IO_NR24 0xFF19 // IC---HHH  C1 Initial / Counter 1=stop / pitch 

#define IO_NR50 0xFF24 // -LLL-RRR  Channel volume
#define IO_NR51 0xFF25 // LLLLRRRR  Mixer
#define IO_NR52 0xFF26 // T---4321  T Sound toggle / 1-4 Status of channels 1-4

typedef struct Gameboy Gameboy;
void gbInitAudio (Gameboy *gb);
void gbAudioLoop(Gameboy *gb, f32 delta_time);
void OnChannel2InitSet(Gameboy *gb);
