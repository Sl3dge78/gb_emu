#define M_PI2 (3.1415926 * 2.0)

#define IO_NR10 0xFF10 // -TTTDNNN 	T=Time,D=direction,N=Numberof shifts
#define IO_NR11 0xFF11 // DDLLLLLL	D=Wave pattern Duty L=Length 
#define IO_NR12 0xFF12 // VVVVDNNN	C1 Volume / Direction 0=down / envelope Number (fade speed)
#define IO_NR13 0xFF13 // LLLLLLLL	pitch L
#define IO_NR14 0xFF14 // IC---HHH  C1 Initial / Counter 1=stop / pitch 

#define IO_NR21 0xFF16 // DDLLLLLL	D=Wave pattern Duty L=Length 
#define IO_NR22 0xFF17 // VVVVDNNN	C1 Volume / Direction 0=down / envelope Number (fade speed)
#define IO_NR23 0xFF18 // LLLLLLLL	pitch L
#define IO_NR24 0xFF19 // IC---HHH  C1 Initial / Counter 1=stop / pitch 

#define IO_NR41 0xFF20 // ---LLLLL  L=Length
#define IO_NR42 0xFF21 // VVVVDNNN 	Volume / Direction 0=down / envelope Number (fade speed)
#define IO_NR43 0xFF22 // SSSSCDDD 	Shift clock frequency (pitch) / Counter Step 0=15bit 1=7bit / Dividing ratio (roughness)
#define IO_NR44 0xFF23 // IC------ 	C1 Initial / Counter 1=stop

#define IO_NR50 0xFF24 // -LLL-RRR  Channel volume
#define IO_NR51 0xFF25 // LLLLRRRR  Mixer
#define IO_NR52 0xFF26 // T---4321  T Sound toggle / 1-4 Status of channels 1-4

typedef struct Gameboy Gameboy;

typedef struct AudioEnveloppe {
    u16 NRX0;
    i8 volume;
    f32 length;
    u8 timer;
} AudioEnveloppe;

typedef struct Channel1 {
    bool is_playing;
    u8 duty; 
    f32 time;
    u16 pitch;
    f32 sweep_timer;
    u8 sweep_period;
} Channel1;

typedef struct Channel2 {
    bool is_playing;
    u8 duty; 
    f32 time;
    u16 pitch; 
} Channel2;

typedef struct Channel4 {
    bool is_playing;
    u16 LFSR;
    u16 timer;
} Channel4;

typedef struct APU {
    u32 sample_rate;
    SDL_AudioDeviceID audio_device;
    u32 audio_gain;

    i32 apu_clock;
    i32 frame_sequencer;
    AudioEnveloppe enveloppes[4];

    Channel1 channel1;
    Channel2 channel2;
    Channel4 channel4;
} APU;

void gbInitAudio (Gameboy *gb, APU *apu);
void gbAudio(Gameboy *gb);
void EnveloppeInit(Gameboy *gb, AudioEnveloppe *env, u8 channel);
void NoiseUpdate(Gameboy *gb, Channel4 *chan4);
