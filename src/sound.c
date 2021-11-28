f32 SquareWave(f32 *time, f64 note_freq, i32 sample_rate, f32 duty) {
    f32 result = 0;
    if(*time <= duty)
        result = -1.0f;
    else
        result = 1.0f;
    *time += note_freq * 1.0 / sample_rate;
    if(*time > 1.0f) 
        *time = 0.0f;
    return result;
}

f32 SawWave(f32 *time, f64 note_freq, i32 sample_rate) {
    f32 result = -fmod(*time, 1.0) + 0.5f;
    *time += note_freq * 1.0 / sample_rate;
    if(*time > 1.0f) 
        *time = 0.0f;
    return result;
}

f32 SineWave(f32 *time, f64 note_freq, i32 sample_rate) {
    f32 result = sin(*time);
    *time += note_freq * M_PI2 / sample_rate;
    if(*time >= M_PI2)
        *time = (-M_PI2);
    return result;
}

f32 GetSquareDuty(u8 byte) {
    switch(byte) {
        case 0 : return 0.125f;
        case 1 : return 0.250f;
        case 2 : return 0.500f;
        case 3 : return 0.750f;
    }

    return 0.0f;
}

void Channel2Audio(i16 *stream, i32 len, Gameboy *gb) {
    u8 NR21   = gbReadAt(gb, IO_NR21, 0);
    u8 duty   = (NR21 & 0xC0) >> 6;

    u8 NR24   = gbReadAt(gb, IO_NR24, 0);
    u16 pitch = gbReadAt(gb, IO_NR23, 0);
    pitch    |= (NR24 & 0x07) << 8;
    pitch = 131072 / (2048 - pitch);
    bool initial  = (NR24 & 0x80) != 0;
    bool counter = (NR24 & 0x40) != 0; 

    for(i32 i = 0; i < len; i++) {
        i16 raw = SquareWave(&gb->audio_time, pitch, gb->sample_rate, GetSquareDuty(duty));
        raw *= (gb->channel2_volume * gb->audio_gain/16);
        stream[i] += raw; 
    }
}

void Channel2Update(Gameboy *gb, f32 delta_time) {
    // Channel 2
    // Time
    u8 NR24   = gbReadAt(gb, IO_NR24, 0);
    bool counter = (NR24 & 0x40) != 0;

    if(counter) {
        gb->channel2_length -= delta_time;
        if(gb->channel2_length <= 0) {
            gb->channel2_length = 0;
            u8 NR52 = gb->mem[IO_NR52];
            NR52 = (NR52 & ~(0b10)); // Toggle byte 2 off
            gb->mem[IO_NR52] = NR52;
        }
    } else {
        gb->channel2_enveloppe = false;
    }
    // Enveloppe
   if (gb->channel2_enveloppe){ 
        gb->channel2_env_counter -= delta_time;
        
        if(gb->channel2_env_counter <= 0){
            u8 NR22 = gbReadAt(gb, IO_NR22, 0);
            u8 direction = (NR22 & 0x08) >> 3; 
            u8 env_num   = (NR22 & 0x07);      
            
            if(direction) {
                gb->channel2_volume++;
                if(gb->channel2_volume <= 0xF) {
                    gb->channel2_env_counter = (f32)env_num * (1.0f/64.0f);
                } else {
                    gb->channel2_enveloppe = false;
                    gb->channel2_volume = 0xF;
                }
            } else {
                gb->channel2_volume--;
                if(gb->channel2_volume >= 0) {
                    gb->channel2_env_counter = (f32)env_num * (1.0f/64.0f);
                } else {
                    gb->channel2_volume = 0;
                    gb->channel2_enveloppe = false;
                }
            }
        }
    }
}

void OnChannel2InitSet(Gameboy *gb) {
    gb->mem[IO_NR52] |= 0b10;
    
    u8 NR22 = gbReadAt(gb, IO_NR22, 0);
    u8 volume    = (NR22 & 0xF0) >> 4; 
    gb->channel2_volume = volume;
    gb->channel2_enveloppe = true;
}

void AudioCallback(void *data, u8 *_stream, i32 len) {
    i16 *stream = (i16 *)_stream;
    Gameboy *gb   = (Gameboy *)data;
    len /= sizeof(*stream);
    
    u8 NR52 = gbReadAt(gb, IO_NR52, 0);
    for(i32 i = 0; i < len; i++) {
        stream[i] = 0;
    }

    if((NR52 & 0x2) != 0)  
        Channel2Audio(stream, len, gb);

}

void gbAudioLoop(Gameboy *gb, f32 delta_time) {
    Channel2Update(gb, delta_time);    
}

void gbInitAudio (Gameboy *gb) {
    gb->sample_rate = 48000;
    gb->audio_time = 0.0f;
    gb->audio_gain = 2000;

    SDL_AudioSpec audio_spec = {0};
    audio_spec.freq = gb->sample_rate;
    audio_spec.format = AUDIO_S16SYS;
    audio_spec.channels = 1; // @TODO Switch to stereo
    audio_spec.samples = 1024;
    audio_spec.callback = AudioCallback;
    audio_spec.userdata = gb;
    gb->audio_device = SDL_OpenAudioDevice(NULL, 0, &audio_spec, NULL, 0);
   
    gbWriteAt(gb, IO_NR50, 0x77, 0);
    gbWriteAt(gb, IO_NR51, 0xF3, 0);
    gbWriteAt(gb, IO_NR52, 0x80, 0);
    
    u8 channel2_volume = 0;
    f32 channel2_length = 0;
    f32 channel2_env_counter = 0;
}
