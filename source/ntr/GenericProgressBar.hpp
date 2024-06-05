#pragma once
#include "./gfx/backend_temp/Defs.hpp"
#include "./gfx/backend_temp/Texture.hpp"
namespace ntr {
    class GenericProgressBar {
    private:
        GXColor color;
        vec2 vertices[4];
        vec2 internal_offset;
    public:
        // GenericProgressBar(vec2 position, float max_length, float height, GXColor _color);
        GenericProgressBar(vec2* _vertices, GXColor _color);
        void resource_bind();
        void set_offset(vec2 offset);
        void draw(float percent);
    };
};