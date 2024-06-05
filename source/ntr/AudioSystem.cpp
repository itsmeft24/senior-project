#include <aesndlib.h>
#include "AudioSystem.hpp"

namespace ntr {
    AudioSystem::AudioSystem() {
        AESND_Init();
        AESND_Pause(false);
        ui_sound_effect_voice = AESND_AllocateVoice(nullptr);
        sound_effect_voice = AESND_AllocateVoice(nullptr);
        music_voice =  AESND_AllocateVoice(nullptr);
    }

    AudioSystem::~AudioSystem() {
        AESND_Pause(true);

        AESND_FreeVoice(ui_sound_effect_voice);
        AESND_FreeVoice(sound_effect_voice);
        AESND_FreeVoice(music_voice);
    }

    void AudioSystem::play_ui_sound_effect(const AudioFile &se)
    {
        AESND_PlayVoice(ui_sound_effect_voice, se.format(), se.data(), se.len(), se.sample_rate(), 0, false);
    }

    void AudioSystem::play_sound_effect(const AudioFile& se)
    {
        AESND_PlayVoice(sound_effect_voice, se.format(), se.data(), se.len(), se.sample_rate(), 0, false);
    }

    void AudioSystem::play_music(const AudioFile& mus, bool loop, int delay_ms)
    {
        AESND_PlayVoice(music_voice, mus.format(), mus.data(), mus.len(), mus.sample_rate(), delay_ms, loop);
    }

    void AudioSystem::set_ui_sound_effect_volume(float vol)
    {
        AESND_SetVoiceVolume(ui_sound_effect_voice, (short)(vol * 255), (short)(vol * 255));
    }

    void AudioSystem::set_sound_effect_volume(float vol)
    {
        AESND_SetVoiceVolume(sound_effect_voice, (short)(vol * 255), (short)(vol * 255));
    }

    void AudioSystem::set_music_volume(float vol)
    {
        AESND_SetVoiceVolume(music_voice, (short)(vol * 255), (short)(vol * 255));
    }
};