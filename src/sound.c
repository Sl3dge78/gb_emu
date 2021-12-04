
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

void NoiseChannel(i16 *stream, i32 len, i16 value, i8 volume, u32 sample_rate) {
    for(i32 i = 0; i < len; i++) {
        f32 random = (f32)(rand() % 3) / 2.0f;
        stream[i] += random * value * volume;
    }
}

void ToggleChannel(APU *apu, u8 channel, bool value) {
    switch(channel) {
        case 0 : apu->channel1.is_playing = value; break;
        case 1 : apu->channel2.is_playing = value; break;
        //case 2 : apu->channel3.is_playing = false; break; @todo channel3
        case 3 : apu->channel4.is_playing = value; break;
    }
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
    ToggleChannel(&gb->apu, channel, volume > 0);
}

void AudioCallback(void *data, u8 *_stream, i32 len) {
    i16 *stream = (i16 *)_stream;
    APU *apu   = (APU *)data;
    len /= sizeof(*stream);
    
    for(i32 i = 0; i < len; i++) {
        stream[i] = 0;
    }

    if(apu->channel1.is_playing){
        u16 pitch = 131072 / (2048 - apu->channel1.pitch);
        //ToneChannel(stream, len, &apu->channel1.time, apu->channel1.duty, pitch, apu->enveloppes[0].volume, apu->sample_rate);
    }

    if(apu->channel2.is_playing) {
        u16 pitch = 131072 / (2048 - apu->channel2.pitch);
        //ToneChannel(stream, len, &apu->channel2.time, apu->channel2.duty, pitch, apu->enveloppes[1].volume, apu->sample_rate);
    }

    if(apu->channel4.is_playing) {
        i16 value = apu->channel4.LFSR & 1 ? -1 : 1;
        NoiseChannel(stream, len, value, apu->enveloppes[3].volume, apu->sample_rate);
    }

    for(i32 i = 0; i < len; i++) {
        stream[i] *= apu->audio_gain/16;
    }
}

void LengthUpdate(Gameboy *gb) {
    for(i32 i = 0; i < 4; i ++) {
        if(gb->mem[IO_NR52] & (1 << i)) {
            AudioEnveloppe *env = &gb->apu.enveloppes[i];
            u16 NRX0 = env->NRX0; 
            u8 NRX4  = gbReadAt(gb, NRX0 + 4, 0);
            bool counter = (NRX4 & 0x40) != 0;

            // Length
            if(counter) {
                env->length--;
                if(env->length <= 0) {
                    env->length = 0;
                    ToggleChannel(&gb->apu, i, 0);
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

    gb->apu.channel1.sweep_period--;

    if(gb->apu.channel1.sweep_period == 0) {
        gb->apu.channel1.sweep_period = time == 0 ? 8 : time;

        // Calculate new frequency
        u16 new_frequency = gb->apu.channel1.pitch >> nb;
        if(decrease) {
            new_frequency = gb->apu.channel1.pitch - new_frequency;
        } else {
            new_frequency = gb->apu.channel1.pitch + new_frequency;
        }
        if(new_frequency > 0x7FF) {
            //ToggleChannel(gb, 0, 0);
        } else {
            gb->apu.channel1.pitch = new_frequency;
        }
    }
}

void VolumeUpdate(Gameboy *gb) {
    for(i32 i = 0; i < 4; i ++) {
        if(gb->mem[IO_NR52] & (1 << i)) {
            AudioEnveloppe *env = &gb->apu.enveloppes[i];
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

void NoiseUpdate(Gameboy *gb, Channel4 *chan4) {
    chan4->timer--;
    if(chan4->timer <= 0) {
        // Calculate noise timer
        static const u8 divisors_table[8] = {8, 16, 32, 48, 64, 80, 96, 112};
        u8 NR43 = gbReadAt(gb, IO_NR43, 0);
        u8 shift       = ((NR43 & 0b11110000) >> 4) & 0xF;
        u8 divisor     = (NR43 & 0b11) & 0b11;
        u8 width_mode  = ((NR43 & 0b100) >> 3) & 1;
        divisor = divisors_table[divisor];
        chan4->timer = divisor << shift;
        
        u8 xor_result = ((chan4->LFSR & 2) >> 1) ^ (chan4->LFSR & 1);
        chan4->LFSR >>= 1;
        chan4->LFSR |= xor_result << 14;
        if(width_mode)
            chan4->LFSR = (chan4->LFSR & ~(1 << 6)) | (xor_result << 6);
    }
}

void gbAudio(Gameboy *gb) {
    APU *apu = &gb->apu;
    switch(gb->apu.frame_sequencer) {
        case 0: LengthUpdate(gb); break;
        case 2: LengthUpdate(gb); SweepUpdate(gb); break;
        case 4: LengthUpdate(gb); break;
        case 6: LengthUpdate(gb); SweepUpdate(gb); break;
        case 7: VolumeUpdate(gb); break;
    }
        
    u8 NR52 = apu->channel4.is_playing << 3 /*| apu->channel3.is_playing << 2*/ | apu->channel2.is_playing << 1 | apu->channel2.is_playing;
    gb->mem[IO_NR52] = (gb->mem[IO_NR52] & 0xF0) | NR52;

    gb->apu.frame_sequencer++;
    gb->apu.frame_sequencer %= 8;
    gb->apu.apu_clock += 8192;
}

void gbInitAudio (Gameboy *gb, APU *apu) {
    apu->sample_rate = 44000;
    apu->audio_gain = 2000;

    SDL_AudioSpec audio_spec = {0};
    audio_spec.freq = apu->sample_rate;
    audio_spec.format = AUDIO_S16SYS;
    audio_spec.channels = 1; // @todo Switch to stereo
    audio_spec.samples = 1024;
    audio_spec.callback = AudioCallback;
    audio_spec.userdata = apu;
    apu->audio_device = SDL_OpenAudioDevice(NULL, 0, &audio_spec, NULL, 0);
   
    gbWriteAt(gb, IO_NR50, 0x77, 0);
    gbWriteAt(gb, IO_NR51, 0xF3, 0);
    gbWriteAt(gb, IO_NR52, 0x80, 0);
   
    apu->enveloppes[0].NRX0 = 0xFF10;
    apu->enveloppes[1].NRX0 = 0xFF15;
    apu->enveloppes[3].NRX0 = 0xFF1F;
    
    apu->channel4.LFSR = 0xFFFF;
    srand(time(NULL));
}
