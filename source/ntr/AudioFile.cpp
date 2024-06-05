#include "AudioFile.hpp"
#include "Util.hpp"
#include <cstdio>
#include <aesndlib.h>
#include "global_mem.h"

static inline std::uint16_t byteswap16(std::uint16_t x)
{
	return std::uint16_t(((x & 0x00FF) << 8) | ((x & 0xFF00) >> 8));
}

static inline std::uint32_t byteswap32(std::uint32_t x)
{
	return (x << 24) | ((x << 8) & 0x00ff0000) | ((x >> 8) & 0x0000ff00) | (x >> 24);
}

struct __attribute__((packed)) FMTChunkHeader {
    std::uint32_t magic;
    std::uint32_t len;
    std::uint16_t audio_format;
    std::uint16_t channel_count;
    std::uint32_t sampling_rate;
    std::uint32_t bytes_per_second;
    std::uint16_t bytes_per_sample;
    std::uint16_t bits_per_sample;
    void swap() {
        magic = byteswap32(magic);
        len = byteswap32(len);
        audio_format = byteswap16(audio_format);
        channel_count = byteswap16(channel_count);
        sampling_rate = byteswap32(sampling_rate);
        bytes_per_second = byteswap32(bytes_per_second);
        bytes_per_sample = byteswap16(bytes_per_sample);
        bits_per_sample = byteswap16(bits_per_sample);   
    }
};


struct __attribute__((packed)) DataChunkHeader {
    std::uint32_t magic;
    std::uint32_t len;
    void swap() {
        magic = byteswap32(magic);
        len = byteswap32(len);
    }
};

struct __attribute__((packed)) WAVHeader {
    std::uint32_t riff;
    std::uint32_t size_remaining;
    std::uint32_t wave;
    FMTChunkHeader fmt_chunk_hdr;
    void swap() {
        riff = byteswap32(riff);
        size_remaining = byteswap32(size_remaining);
        wave = byteswap32(wave);
        fmt_chunk_hdr.swap();
    }
};

namespace ntr {
    AudioFile::AudioFile(const char* path)
    {
        FILE* f = fopen(path, "rb");
        WAVHeader hdr{};
        fread(&hdr, sizeof(WAVHeader), 1, f);
        hdr.swap();

        fseek(f, ntr::util::clamp(static_cast<int>(hdr.fmt_chunk_hdr.len) - 16, 0, 1), SEEK_CUR);

        DataChunkHeader data_chunk_hdr{};
        fread(&data_chunk_hdr, sizeof(DataChunkHeader), 1, f);
        data_chunk_hdr.swap();

        if (hdr.fmt_chunk_hdr.audio_format != 1) {
            printf("Encountered a non-PCM audio format in wave file: %s\n", path);
        }
        
        if (hdr.fmt_chunk_hdr.channel_count == 1) {
            if (hdr.fmt_chunk_hdr.bits_per_sample == 16) {
                format_aesnd = VOICE_MONO16;
            } else if (hdr.fmt_chunk_hdr.bits_per_sample == 8) {
                format_aesnd = VOICE_MONO8_UNSIGNED;
            } else {
                printf("Audio format is neither 8-bit nor 16-bit in wave file: %s\n", path);
            }
        } else if (hdr.fmt_chunk_hdr.channel_count == 2) {
            if (hdr.fmt_chunk_hdr.bits_per_sample == 16) {
                format_aesnd = VOICE_STEREO16;
            } else if (hdr.fmt_chunk_hdr.bits_per_sample == 8) {
                format_aesnd = VOICE_STEREO8_UNSIGNED;
            } else {
                printf("Audio format is neither 8-bit nor 16-bit in wave file: %s\n", path);
            }
        }

        sample_rate_hz = float(hdr.fmt_chunk_hdr.sampling_rate);

        buffer = malloc(data_chunk_hdr.len);
        INC_SIZE(data_chunk_hdr.len);
        fread(buffer, data_chunk_hdr.len, 1, f);
        size = data_chunk_hdr.len;
        fclose(f);
        
        // me when i have to endian swap the wave file (shooter velocity moment)
        if (hdr.fmt_chunk_hdr.bits_per_sample == 16) {
            for (std::size_t i = 0; i < size / 2; i++) {
                reinterpret_cast<std::uint16_t*>(buffer)[i] = byteswap16(reinterpret_cast<std::uint16_t*>(buffer)[i]);
            }
        }
    }

    AudioFile::~AudioFile()
    {
        free(buffer);
    }
};