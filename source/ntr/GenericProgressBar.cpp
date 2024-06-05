#include <cstring>
#include <grrlib.h>
#include <stdio.h>
#include "GenericProgressBar.hpp"

/*
ntr::GenericProgressBar::GenericProgressBar(vec2 position, float max_length, float height, GXColor _color) {
    vertices[0] = position;
    vertices[1] = vec2{position.x + max_length, position.y};
    vertices[2] = vec2{position.x + max_length, position.y + height};
    vertices[3] = vec2{position.x, position.y + height};
    color = _color;
}
*/
ntr::GenericProgressBar::GenericProgressBar(vec2 *_vertices, GXColor _color) {
    std::memcpy(&vertices, _vertices, 4 * sizeof(vec2));
    color = _color;
    internal_offset = vec2{0,0};
}
/*
Top Left
Top Right
Bottom Right
Bottom Left
*/
void ntr::GenericProgressBar::resource_bind() {
    GX_ClearVtxDesc();
    GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    
    GX_SetNumChans(1);
    GX_SetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_VTX, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
	GX_SetChanMatColor(GX_COLOR0A0, color);
    GX_SetNumTexGens(1);
    GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
}

void ntr::GenericProgressBar::set_offset(vec2 offset) {
    internal_offset = offset;
}

void ntr::GenericProgressBar::draw(float percent)
{
    vec2 new_vertices[4] = {
        vertices[0] + internal_offset,
        (vertices[1] - vertices[0]) * percent + vertices[0] + internal_offset,
        (vertices[2] - vertices[3]) * percent + vertices[3] + internal_offset,
        vertices[3] + internal_offset,
    };
    Mtx model_view{};
    guMtxIdentity(model_view);
    guMtxTransApply(model_view, model_view, 0.0f, 0.0f, -100.0f);
    GX_Flush();
    GX_InvVtxCache();
    GX_LoadPosMtxImm(model_view, GX_PNMTX0);
    GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
        GX_Position3f32(new_vertices[0].x, new_vertices[0].y, 0);
        GX_Position3f32(new_vertices[1].x, new_vertices[1].y, 0);
        GX_Position3f32(new_vertices[2].x, new_vertices[2].y, 0);
        GX_Position3f32(new_vertices[3].x, new_vertices[3].y, 0);
    GX_End();
    GX_Flush();
    GX_InvVtxCache();
}
