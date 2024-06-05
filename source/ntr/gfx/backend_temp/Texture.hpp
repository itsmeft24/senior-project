#pragma once
#include <ogc/gx.h>
#include "Renderer.hpp"
#include "Defs.hpp"
namespace ntr::gfx {
    class Texture {
    public:
        struct TEX0_Header {
            char magic[4];
            std::uint32_t size;
            std::uint32_t version_number;
            std::uint32_t brres_offset;
            std::uint32_t section_offset;
            std::uint32_t string_offset;
            std::uint32_t has_palette;
            std::uint16_t width;
            std::uint16_t height;
            std::uint32_t texture_format;
            std::uint32_t number_images;
        };
        GXTexObj to;
        TEX0_Header hdr{};
        void* data;
    public:
        Texture(const char* path);
        ~Texture();
        Texture(const Texture&) = delete;
        Texture(Texture&&) = default;
        Texture& operator=(const Texture&) = delete;
        void resource_bind();
        void resource_unbind();
        void draw(Renderer&, vec2 position);
        void draw_flipped(Renderer&, vec2 position);
    };
} // namespace ntr::gfx
