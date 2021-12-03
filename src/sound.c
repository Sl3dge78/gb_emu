
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

void NoiseChannel(i16 *stream, i32 len, u16 noise_frequency, i8 volume, u32 sample_rate) {
    for(i32 i = 0; i < len; i++) {
        f32 test_noise_frequency = (f32)(rand() % USHRT_MAX) / (f32)USHRT_MAX;
        stream[i] += test_noise_frequency * volume; 
    }
}

void ToggleChannel(Gameboy *gb, u8 channel, bool value) {
    u8 NR52 = gb->mem[IO_NR52];
    u8 shifted = value << channel;
    NR52 = (NR52 & ~(1 << channel)) | shifted;
    gb->mem[IO_NR52] = NR52;
}

void EnveloppeInit(Gameboy *gb, AudioEnveloppe *env, u8 channel) {

    u16 NRX0 = env->NRX0;
    u8 NRX4 = gbReadAt(gb, NRX0 + 4, 0);
    u8 NRX2 = gbReadAt(gb, NRX0 + 2, 0);
    u8 length = gbReadAt(gb, NRX0 + 1, 0) & 0b11111;
    env->length = length;

    u8 volume = (NRX2 & 0xF0) >> 4; 
    env->volume = volume;
    u8 env_num = (NRX2 & 0x7);
    if(env_num != 0) { // Is enveloppe sweep on
        env->timer = env_num;
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

    if((NR52 & 0x1) != 0 && gb->channel1_enabled){
        u8 NR11   = gbReadAt(gb, IO_NR11, 0);
        u8 duty   = (NR11 & 0xC0) >> 6;
        u16 pitch = 131072 / (2048 - gb->chan1_tone);
        ToneChannel(stream, len, &gb->chan1_time, duty, pitch, gb->enveloppes[0].volume, gb->sample_rate);
    }

    if((NR52 & 0x2) != 0 && gb->channel2_enabled) {
        u8 NR21   = gbReadAt(gb, IO_NR21, 0);
        u8 duty   = (NR21 & 0xC0) >> 6;

        u8 NR24   = gbReadAt(gb, IO_NR24, 0);
        u16 pitch = gbReadAt(gb, IO_NR23, 0);
        pitch    |= (NR24 & 0x07) << 8;
        pitch = 131072 / (2048 - pitch);
        ToneChannel(stream, len, &gb->chan2_time, duty, pitch, gb->enveloppes[1].volume, gb->sample_rate);
    }
    if((NR52 & 0x8) != 0 && gb->channel4_enabled) {
        //NoiseChannel(stream, len, gb->noise_frequency, gb->enveloppes[3].volume, gb->sample_rate);
    }

    for(i32 i = 0; i < len; i++) {
        stream[i] *= gb->audio_gain/16;
    }
}

void LengthUpdate(Gameboy *gb) {
    for(i32 i = 0; i < 4; i ++) {
        if(gb->mem[IO_NR52] & (1 << i)) {
            AudioEnveloppe *env = &gb->enveloppes[i];
            u16 NRX0 = env->NRX0; 
            u8 NRX4  = gbReadAt(gb, NRX0 + 4, 0);
            bool counter = (NRX4 & 0x40) != 0;

            // Length
            if(counter) {
                env->length--;
                if(env->length <= 0) {
                    env->length = 0;
                    ToggleChannel(gb, i, 0);
                }
            }
        }
    }
}

void SweepUpdate(Gameboy *gb) {
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

void VolumeUpdate(Gameboy *gb) {
    for(i32 i = 0; i < 4; i ++) {
        if(gb->mem[IO_NR52] & (1 << i)) {
            AudioEnveloppe *env = &gb->enveloppes[i];
            u8 NRX2 = gbReadAt(gb, env->NRX0 + 2, 0);
            u8 env_num = (NRX2 & 0x07);      
            if (env_num) { 
                env->timer--;
                if(env->timer <= 0) {
                    env->timer = env_num;
                    u8 direction = (NRX2 & 0x08) >> 3; 
                    if(direction && env->volume < 0xF) {
                        env->volume++;
                    } else if(env->volume > 0) {
                        env->volume--;
                    }
                }
            }
        }
    }
}

void NoiseUpdate(Gameboy *gb) {
    if(gb->noise_timer <= 0) {


    }
}

void gbAudio(Gameboy *gb) {

    switch(gb->frame_sequencer) {
        case 0: LengthUpdate(gb); break;
        case 2: LengthUpdate(gb); SweepUpdate(gb); break;
        case 4: LengthUpdate(gb); break;
        case 6: LengthUpdate(gb); SweepUpdate(gb); break;
        case 7: VolumeUpdate(gb); break;
    }

    gb->frame_sequencer++;
    gb->frame_sequencer %= 8;
    gb->apu_clock += 8192;

    /*
    EnveloppeUpdate(gb, delta_time, &gb->channel1_enveloppe, 1);    

    if(gb->mem[IO_NR52] & 1) {

        // Sweep
            }
    
    if(gb->mem[IO_NR52] & 2) {
        EnveloppeUpdate(gb, delta_time, &gb->channel2_enveloppe, 2);    
    }
    
    // Channel 4
    if(gb->mem[IO_NR52] & 8) {
        //@Todo : maybe don't calcuate this each frame?
        
        gb->noise_timer -= delta_time;
        if(gb->noise_timer <= 0) {
            // Change the frequency
            
            u8 NR43 = gbReadAt(gb, IO_NR43, 0);
            u8 width_mode  = ((NR43 & 0b100) >> 3) & 1;

            u8 shift       = ((NR43 & 0b11110000) >> 4) & 0xF;
            u8 divisor     = (NR43 & 0b11) & 0b11;
            f32 real_divisor = divisor == 0 ? 0.5f : (f32)divisor;
            u16 noise_timer_frequency = (u16)(524288 / real_divisor) << shift;
            gb->noise_timer = 1.0f / noise_timer_frequency;
            
            u8 xor_result = ((gb->LFSR & 2) >> 1) ^ (gb->LFSR & 1);
            gb->LFSR >>= 1;
            gb->LFSR |= xor_result << 14;
            if(width_mode)
                gb->LSFR = (gb->LSFR & ~(1 << 6)) | (xor_result << 6);
        }

        EnveloppeUpdate(gb, delta_time, &gb->channel4_enveloppe, 4);    
    }
    */
}

void gbInitAudio (Gameboy *gb) {
    gb->sample_rate = 48000;
    gb->audio_gain = 2000;

    SDL_AudioSpec audio_spec = {0};
    audio_spec.freq = gb->sample_rate;
    audio_spec.format = AUDIO_S16SYS;
    audio_spec.channels = 1; // @todo Switch to stereo
    audio_spec.samples = 1024;
    audio_spec.callback = AudioCallback;
    audio_spec.userdata = gb;
    gb->audio_device = SDL_OpenAudioDevice(NULL, 0, &audio_spec, NULL, 0);
   
    gbWriteAt(gb, IO_NR50, 0x77, 0);
    gbWriteAt(gb, IO_NR51, 0xF3, 0);
    gbWriteAt(gb, IO_NR52, 0x80, 0);
   
    gb->enveloppes[0].NRX0 = 0xFF10;
    gb->enveloppes[1].NRX0 = 0xFF15;
    gb->enveloppes[3].NRX0 = 0xFF1F;
    
    gb->channel1_enabled = true;
    gb->channel2_enabled = true;
    gb->channel4_enabled = true;
    gb->LFSR = 0xFF;
}
