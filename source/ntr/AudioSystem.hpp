#pragma once
#include <aesndlib.h>
#include "AudioFile.hpp"

namespace ntr {
    class AudioSystem {
    private:
        AESNDPB* ui_sound_effect_voice;
        AESNDPB* sound_effect_voice;
        AESNDPB* music_voice;
    public:
        AudioSystem();
        ~AudioSystem();
        void play_ui_sound_effect(const AudioFile& se);
        void play_sound_effect(const AudioFile& se);
        void play_music(const AudioFile& mus, bool loop, int delay_ms);
        void set_ui_sound_effect_volume(float vol);
        void set_sound_effect_volume(float vol);
        void set_music_volume(float vol);
    };
};