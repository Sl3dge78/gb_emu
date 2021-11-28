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

void ToneChannel(i16 *stream, i32 len, f32 *time, u8 duty, u16 pitch, i8 volume, u32 sample_rate) {
   
    for(i32 i = 0; i < len; i++) {
        //i16 raw = SquareWave(time, pitch, sample_rate, GetSquareDuty(duty));
        f32 result = 0;
        if(*time <= GetSquareDuty(duty))
            result = -1.0f;
        else
            result = 1.0f;
        *time += pitch * 1.0 / sample_rate;
        if(*time > 1.0f) 
            *time = 0.0f;
        result *= volume;
        stream[i] += result; 
    }
}

u16 GetNRX0(u8 channel) {
    switch (channel) {
    case 1 : return 0xFF10;
    case 2 : return 0xFF15;
    case 3 : return 0xFF1A;
    case 4 : return 0xFF20;
    }
    assert(0);
    return 0;
}

void EnveloppeUpdate(Gameboy *gb, f32 delta_time, AudioEnveloppe *chan, u8 num) {
    // Because calculating the proper memory address is complicated,
    // we just did a switch to fetch the address of the 0th register of each sound channel
    // We call this regiser NRX0. X referring to the current audio channel.

    u16 NRX0 = chan->NRX0; 

    // Time
    u8 NRX4   = gbReadAt(gb, NRX0 + 4, 0);
    bool counter = (NRX4 & 0x40) != 0;

    if(counter) {
        chan->length -= delta_time;
        if(chan->length <= 0) {
            chan->length = 0;
            u8 NR52 = gb->mem[IO_NR52];
            NR52 = (NR52 & ~(1 << (num - 1) )); // Toggle byte X off
            gb->mem[IO_NR52] = NR52;
        }
    } else {
        chan->env_active = false;
    }
    // Enveloppe
   if (chan->env_active){ 
        chan->timer -= delta_time;
        
        if(chan->timer <= 0){
            u8 NRX2 = gbReadAt(gb, NRX0 + 2, 0);
            u8 direction = (NRX2 & 0x08) >> 3; 
            u8 env_num   = (NRX2 & 0x07);      
            
            if(direction) {
                chan->volume++;
                if(chan->volume <= 0xF) {
                    chan->timer = (f32)env_num * (1.0f/64.0f);
                } else {
                    chan->env_active = false;
                    chan->volume = 0xF;
                }
            } else {
                chan->volume--;
                if(chan->volume >= 0) {
                    chan->timer = (f32)env_num * (1.0f/64.0f);
                } else {
                    chan->volume = 0;
                    chan->env_active = false;
                }
            }
        }
    }
}

void OnChannelInitSet(Gameboy *gb, AudioEnveloppe *env) {
    
    u16 NRX0 = env->NRX0;
    u8 NRX2 = gbReadAt(gb, NRX0 + 2, 0);
    u8 volume    = (NRX2 & 0xF0) >> 4; 
    env->volume = volume;
    env->env_active = true;
}

void AudioCallback(void *data, u8 *_stream, i32 len) {
    i16 *stream = (i16 *)_stream;
    Gameboy *gb   = (Gameboy *)data;
    len /= sizeof(*stream);
    
    u8 NR52 = gbReadAt(gb, IO_NR52, 0);
    for(i32 i = 0; i < len; i++) {
        stream[i] = 0;
    }

    if((NR52 & 0x1) != 0){
        u8 NR11   = gbReadAt(gb, IO_NR11, 0);
        u8 duty   = (NR11 & 0xC0) >> 6;

        u8 NR14   = gbReadAt(gb, IO_NR14, 0);
        u16 pitch = gbReadAt(gb, IO_NR13, 0);
        pitch    |= (NR14 & 0x07) << 8;
        pitch = 131072 / (2048 - pitch);
        ToneChannel(stream, len, &gb->chan1_time, duty, pitch, gb->channel1_enveloppe.volume, gb->sample_rate);
    }

    if((NR52 & 0x2) != 0) {
        u8 NR21   = gbReadAt(gb, IO_NR21, 0);
        u8 duty   = (NR21 & 0xC0) >> 6;

        u8 NR24   = gbReadAt(gb, IO_NR24, 0);
        u16 pitch = gbReadAt(gb, IO_NR23, 0);
        pitch    |= (NR24 & 0x07) << 8;
        pitch = 131072 / (2048 - pitch);
        ToneChannel(stream, len, &gb->chan2_time, duty, pitch, gb->channel2_enveloppe.volume, gb->sample_rate);
    }
    for(i32 i = 0; i < len; i++) {
        stream[i] *= gb->audio_gain/16;
    }
}

void gbAudioLoop(Gameboy *gb, f32 delta_time) {
    EnveloppeUpdate(gb, delta_time, &gb->channel1_enveloppe, 1);    
    EnveloppeUpdate(gb, delta_time, &gb->channel2_enveloppe, 2);    
}

void gbInitAudio (Gameboy *gb) {
    gb->sample_rate = 48000;
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
   
    gb->channel1_enveloppe = (AudioEnveloppe){0};
    gb->channel1_enveloppe.NRX0 = 0xFF10;

    gb->channel2_enveloppe = (AudioEnveloppe){0};
    gb->channel2_enveloppe.NRX0 = 0xFF15;
}
