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

void NoiseChannel(i16 *stream, i32 len) {
    
    

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

void ToggleChannel(Gameboy *gb, u8 channel, bool value) {
    u8 NR52 = gb->mem[IO_NR52];
    u8 shifted = value << channel;
    NR52 = (NR52 & ~(1 << channel)) | shifted;
    gb->mem[IO_NR52] = NR52;
}

void EnveloppeUpdate(Gameboy *gb, f32 delta_time, AudioEnveloppe *chan, u8 num) {
    // Because calculating the proper memory address is complicated,
    // we just did a switch to fetch the address of the 0th register of each sound channel
    // We call this regiser NRX0. X referring to the current audio channel.

    u16 NRX0 = chan->NRX0; 

    u8 NRX4   = gbReadAt(gb, NRX0 + 4, 0);
    bool counter = (NRX4 & 0x40) != 0;

    // Length
    if(counter) {
        chan->length -= delta_time;
        if(chan->length <= 0) {
            chan->length = 0;
            ToggleChannel(gb, num - 1, 0);
            return;
        }
    }
    
    // Enveloppe sweep
    u8 NRX2 = gbReadAt(gb, NRX0 + 2, 0);
    u8 env_num = (NRX2 & 0x07);      
    if (env_num) { 
        chan->timer -= delta_time;
        
        if(chan->timer <= 0){
            u8 direction = (NRX2 & 0x08) >> 3; 
            
            if(direction && chan->volume < 0xF) {
                chan->volume++;
                chan->timer = (f32)env_num * (1.0f/64.0f);
            } else if(chan->volume > 0) {
                chan->volume--;
                chan->timer = (f32)env_num * (1.0f/64.0f);
            }
        }
    }
}

void EnveloppeInit(Gameboy *gb, AudioEnveloppe *env, u8 channel) {

    u16 NRX0 = env->NRX0;
    u8 NRX4 = gbReadAt(gb, NRX0 + 4, 0);
    u8 NRX2 = gbReadAt(gb, NRX0 + 2, 0);
    u8 length = gbReadAt(gb, NRX0 + 1, 0) & 0b11111;
    env->length = (64 - length) * (1.0f/256.0f);

    u8 volume = (NRX2 & 0xF0) >> 4; 
    env->volume = volume;
    u8 env_num = (NRX2 & 0x7);
    if(env_num != 0) { // Is enveloppe sweep on
        env->timer = (f32)env_num * (1.0f/64.0f);
    }

    ToggleChannel(gb, channel, volume > 0);
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
        u16 pitch = 131072 / (2048 - gb->chan1_tone);
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
    if((NR52 & 0x8) != 0) {
        u8 NR43 = gbReadAt(gb, IO_NR43, 0);
        u8 clock_freq   = (NR43 >> 4) & 0xF;
        u8 counter_step = (NR43 >> 3) & 0x1;
        u8 ratio        = (NR43 & 0x7);
       
        //NoiseChannel(stream, len);
    }

    for(i32 i = 0; i < len; i++) {
        stream[i] *= gb->audio_gain/16;
    }
}

void gbAudioLoop(Gameboy *gb, f32 delta_time) {
    EnveloppeUpdate(gb, delta_time, &gb->channel1_enveloppe, 1);    
    if(gb->mem[IO_NR52] & 1) {

        // Sweep
        if(gb->sweep_timer >= 0) {

            gb->sweep_timer -= delta_time;
            if(gb->sweep_timer <= 0) {                
                gb->sweep_timer = 1.0f/128.0f;

                u8 NR10 = gbReadAt(gb, IO_NR10, 0);
                u8 time     = (NR10 >> 4) & 0b111;
                u8 decrease = (NR10 >> 3) & 1;
                u8 nb       = (NR10) & 0b11;
                i8 mul = decrease ? -1 : 1;
                
                gb->sweep_period--;

                if(gb->sweep_period == 0) {
                    gb->sweep_period = time == 0 ? 8 : time;

                    // Calculate new frequency
                    u16 new_frequency = gb->chan1_tone >> nb;
                    if(decrease) {
                        new_frequency = gb->chan1_tone - new_frequency;
                    } else {
                        new_frequency = gb->chan1_tone + new_frequency;
                    }
                    if(new_frequency > 0x7FF) {
                        //ToggleChannel(gb, 0, 0);
                    } else {
                        gb->chan1_tone = new_frequency;
                    }
                }
            }
        }
    }
    
    if(gb->mem[IO_NR52] & 2) {
        EnveloppeUpdate(gb, delta_time, &gb->channel2_enveloppe, 2);    
    }
    EnveloppeUpdate(gb, delta_time, &gb->channel4_enveloppe, 4);    
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
    
    gb->channel4_enveloppe = (AudioEnveloppe){0};
    gb->channel4_enveloppe.NRX0 = 0xFF19;
    
    srand(time(NULL));
}
