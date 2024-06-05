#include "Renderer.hpp"
#include <cstdlib>
#include <cstring>
#include <malloc.h>
#include <grrlib.h>

namespace ntr::gfx {
constexpr auto DEFAULT_FIFO_SIZE = 256 * 1024;
    Renderer::Renderer()
    {
        window_closed = false;
        GRRLIB_Init();
        GRRLIB_Settings.antialias = true;
        GX_SetCopyClear((GXColor){ 0x20, 0x20, 0x20, 0xFF }, GX_MAX_Z24);
    }

    Renderer::~Renderer()
    {
        GRRLIB_Exit();
    }

    bool Renderer::is_window_closed() { 
        return window_closed;
    }

    void Renderer::set_window_closed() {
        window_closed = true;
    }

    // 43.21f * (45/58.54)
    void Renderer::start_3d() {
        // const bool normalmode = false;
        const bool texturemode = true;

        Mtx44 projection_matrix{};
        guPerspective(projection_matrix, 43.21f * (45/58.54), (f32)rmode->fbWidth/rmode->efbHeight, 0.1, 3000);
        GX_LoadProjectionMtx(projection_matrix, GX_PERSPECTIVE);
        GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
        GX_SetCullMode(GX_CULL_NONE);
    }

    void Renderer::start_2d() {
        Mtx view{};
        Mtx44 orthographic_projection_matrix{};

        GX_SetZMode(GX_FALSE, GX_LEQUAL, GX_TRUE);

        GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);

        guOrtho(orthographic_projection_matrix, 0, rmode->efbHeight, 0, rmode->fbWidth, 0, 1000.0f);
        GX_LoadProjectionMtx(orthographic_projection_matrix, GX_ORTHOGRAPHIC);

        guMtxIdentity(view);
        guMtxTransApply(view, view, 0, 0, -100.0f);
        GX_LoadPosMtxImm(view, GX_PNMTX0);
    }

    void Renderer::set_vertex_desc_textured() {
        GX_ClearVtxDesc();
        GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
        GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
        GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);

        GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
        GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
        GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
        
        GX_SetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);

        GX_SetNumTevStages(1);
        GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
        GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
    }

    void Renderer::set_vertex_desc_colored() {
        GX_ClearVtxDesc();
        GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
        GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
        GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);

        GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
        GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
        GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

        GX_SetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);

        GX_SetNumTevStages(1);
        GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLOR0);
        GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    }

    void Renderer::set_vertex_desc_2d_textured() {
        GX_ClearVtxDesc();
        GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
        GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
        GX_SetVtxDesc(GX_VA_TEX0, GX_NONE);
        GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
        GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
        GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

        GX_SetNumTexGens(1);  // One texture exists
        GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
        GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
        GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);

        GX_SetNumTevStages(1);

        GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);

        // GX_SetNumChans(1);
        GX_SetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);
        GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);

        GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
        GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    }
    
    void Renderer::end_frame() { 
        GRRLIB_Render();
    }
};