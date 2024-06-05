#include "Texture.hpp"
#include <cstddef>
#include <cstdio>
#include <cstdint>
#include <malloc.h>
#include <grrlib.h>
#include "Renderer.hpp"
#include <cmath>
#include "Defs.hpp"
#include "../../global_mem.h"

std::size_t round_up(std::size_t x, std::uint32_t round_to = 4) {
    if (x % round_to != 0) {
        return ((x / round_to) + 1) * round_to;
    } else {
        return x;
    }
}

std::size_t get_padded_size(std::size_t width, std::size_t height, std::uint32_t blk_width, std::uint32_t blk_height, std::uint32_t bits_per_pixel) {
    int raw_size = round_up(width, blk_width) * round_up(height, blk_height);
    return raw_size * bits_per_pixel / 8;
}

constexpr std::size_t get_texture_len(std::size_t w, std::size_t h, std::uint32_t texture_format) {
    switch (texture_format)
    {
    case GX_TF_I4:
        return get_padded_size(w, h, 8, 8, 4);
    case GX_TF_I8:
        return get_padded_size(w, h, 8, 4, 8);
    case GX_TF_IA4:
        return get_padded_size(w, h, 8, 4, 8);
    case GX_TF_IA8:
        return get_padded_size(w, h, 4, 4, 16);
    case GX_TF_RGB565:
        return get_padded_size(w, h, 4, 4, 16);
    case GX_TF_RGB5A3:
        return get_padded_size(w, h, 4, 4, 16);
    case GX_TF_RGBA8:
        return get_padded_size(w, h, 4, 4, 32);
    case GX_TF_CI4:
        return get_padded_size(w, h, 8, 8, 4);
    case GX_TF_CI8:
        return get_padded_size(w, h, 8, 4, 8);
    case GX_TF_CI14:
        return get_padded_size(w, h, 4, 4, 16);
    case GX_TF_CMPR:
        return get_padded_size(w, h, 8, 8, 4);
    default:
        return 0;
    }
}

namespace ntr::gfx {
    Texture::Texture(const char *path) {
        FILE* tex_file = fopen(path, "rb");
        if (tex_file == nullptr) {
            printf("Failed opening texture file: %s.", path);
        }
        
        fread(reinterpret_cast<void*>(&hdr), sizeof(TEX0_Header), 1, tex_file);
        fseek(tex_file, 0x40, SEEK_SET);
        std::size_t texture_len = GX_GetTexBufferSize(hdr.width, hdr.height, hdr.texture_format, GX_FALSE, 0);
        // std::size_t texture_len = get_texture_len(hdr.width, hdr.height, hdr.texture_format);
        data = memalign(32, texture_len);
        INC_SIZE(texture_len);
        fread(data, texture_len, 1, tex_file);
        fclose(tex_file);

        GX_InitTexObj(&to, data, hdr.width, hdr.height, hdr.texture_format, GX_CLAMP, GX_CLAMP, GX_FALSE);
    }
    Texture::~Texture() {
        free(data);
    }
    void Texture::resource_bind() {
        GX_SetCopyFilter(rmode->aa, rmode->sample_pattern, GX_TRUE, rmode->vfilter);
        GX_LoadTexObj(&to, GX_TEXMAP0);
    }
    void Texture::resource_unbind() {

    }
    void Texture::draw(Renderer& renderer, vec2 position) {
        renderer.set_vertex_desc_2d_textured();
        resource_bind();

        float scaleX = 1.0;
        float scaleY = 1.0;
        float degrees = 0.0;
        float xpos = 0.0;
        float ypos = 0.0;

        Mtx model_matrix, model_view_matrix, view_matrix_2d;
        guVector axis{0, 0, 1};
        guMtxIdentity(model_matrix);

        const f32 width  = hdr.width * 0.5;
        const f32 height = hdr.height * 0.5;

        guMtxTransApply(model_matrix, model_matrix, position.x + width, position.y + height, 0);
        guMtxIdentity(view_matrix_2d);
        guMtxTransApply(view_matrix_2d, view_matrix_2d, 0.0f, 0.0f, -100.0f);
        guMtxConcat(view_matrix_2d, model_matrix, model_view_matrix);

        GX_LoadPosMtxImm(model_view_matrix, GX_PNMTX0);
        GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
            GX_Position3f32(-width, -height, 0);
            GX_Color1u32(0xffffffff);
            GX_TexCoord2f32(0, 0);

            GX_Position3f32(width, -height, 0);
            GX_Color1u32(0xffffffff);
            GX_TexCoord2f32(1, 0);

            GX_Position3f32(width, height, 0);
            GX_Color1u32(0xffffffff);
            GX_TexCoord2f32(1, 1);

            GX_Position3f32(-width, height, 0);
            GX_Color1u32(0xffffffff);
            GX_TexCoord2f32(0, 1);
        GX_End();
    }
    
    void Texture::draw_flipped(Renderer& renderer, vec2 position)
    {
        renderer.set_vertex_desc_2d_textured();
        resource_bind();

        float scaleX = 1.0;
        float scaleY = 1.0;
        float degrees = 0.0;
        float xpos = 0.0;
        float ypos = 0.0;

        Mtx model_matrix, model_view_matrix, view_matrix_2d;
        guVector axis{0, 0, 1};
        guMtxIdentity(model_matrix);

        const f32 width  = hdr.width * 0.5;
        const f32 height = hdr.height * 0.5;

        guMtxTransApply(model_matrix, model_matrix, position.x + width, position.y + height, 0);
        guMtxIdentity(view_matrix_2d);
        guMtxTransApply(view_matrix_2d, view_matrix_2d, 0.0f, 0.0f, -100.0f);
        guMtxConcat(view_matrix_2d, model_matrix, model_view_matrix);

        GX_LoadPosMtxImm(model_view_matrix, GX_PNMTX0);
        GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
            GX_Position3f32(-width, -height, 0);
            GX_Color1u32(0xffffffff);
            GX_TexCoord2f32(0, 1);

            GX_Position3f32(width, -height, 0);
            GX_Color1u32(0xffffffff);
            GX_TexCoord2f32(1, 1);

            GX_Position3f32(width, height, 0);
            GX_Color1u32(0xffffffff);
            GX_TexCoord2f32(1, 0);

            GX_Position3f32(-width, height, 0);
            GX_Color1u32(0xffffffff);
            GX_TexCoord2f32(0, 0);
        GX_End();
    }
};