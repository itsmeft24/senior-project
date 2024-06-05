#pragma once
#include <cstdint>

namespace ntr {
    class AudioFile {
    private:
        void* buffer;
        std::size_t size;
        std::uint32_t format_aesnd;
        float sample_rate_hz;
    public:
        AudioFile(const char* path);
        ~AudioFile();
        inline std::uint32_t format() const {
            return format_aesnd;
        }
        inline const std::uint8_t* data() const {
            return reinterpret_cast<std::uint8_t*>(buffer);
        }
        inline std::size_t len() const {
            return size;
        }
        inline float sample_rate() const {
            return sample_rate_hz;
        }
    };
};