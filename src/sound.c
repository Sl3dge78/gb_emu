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

void AudioChannel2(i16 *stream, i32 len, Gameboy *gb) {
    u8 NR21   = gbReadAt(gb, IO_NR21, 0);
    u8 length = (NR21 & 0x3F);
    u8 duty   = (NR21 & 0xC0) >> 6;
    
    
    u8 NR22 = gbReadAt(gb, IO_NR22, 0);
    u8 volume    = (NR22 & 0xF0) >> 4; 
    u8 direction = (NR22 & 0x08) >> 3;
    u8 env_num   = (NR22 & 0x07);

    u8 NR24   = gbReadAt(gb, IO_NR24, 0);
    u16 pitch = gbReadAt(gb, IO_NR23, 0);
    pitch    |= (NR24 & 0x07) << 8;
    pitch = 131072 / (2048 - pitch);
    bool initial  = (NR24 & 0x80) != 0;
    bool counter = (NR24 & 0x40) != 0;

    for(i32 i = 0; i < len; i++ ) {
        if(initial)
            stream[i] = SquareWave(&gb->audio_time, pitch, gb->sample_rate, GetSquareDuty(duty)) * (volume * 2000/16);
        else
            stream[i] = 0;
    }
}

void AudioCallback(void *data, u8 *_stream, i32 len) {
    i16 *stream = (i16 *)_stream;
    Gameboy *gb   = (Gameboy *)data;
    len /= sizeof(*stream);
    AudioChannel2(stream, len, gb);
}

void gbInitAudio (Gameboy *gb) {
    gb->sample_rate = 48000;
    gb->audio_time = 0.0f;

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
    gbWriteAt(gb, IO_NR52, 0xF1, 0);
}
