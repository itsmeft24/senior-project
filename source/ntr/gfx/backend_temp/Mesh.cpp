#include "Mesh.hpp"
#include "Model.hpp"
namespace ntr::gfx {
    Mesh::~Mesh() {
        if (indices != nullptr) free(indices);
        if (positions != nullptr) free(positions);
        if (normals != nullptr) free(normals);
		if (bi != nullptr) free(bi);
		if (w != nullptr) free(w);
        if (uv1 != nullptr) free(uv1);
		if (ui_tex != nullptr) delete ui_tex;
    }
    void Mesh::draw(Renderer& renderer, Mtx44* skeleton, bool use_ui, bool is_blue) {
		if (use_ui && ui_tex != nullptr) {
			ui_tex->resource_bind();
		}
		else {
			tex.resource_bind();
		}
		
		GX_Begin(GX_TRIANGLES, GX_VTXFMT0, index_count * 3);
		for (std::size_t face_index = 0; face_index < index_count; face_index++) {
			std::uint32_t vert_1 = indices[face_index * 3];
			std::uint32_t vert_2 = indices[face_index * 3 + 1];
			std::uint32_t vert_3 = indices[face_index * 3 + 2];
			
			auto& bone_indices_1 = bi[vert_1].indices;
			auto& bone_weights_1 = w[vert_1];
			vec3 position_1 {};
			///*
			for (std::size_t i = 0; i < 5; i++) {
				if (bone_weights_1.weights[i] == 0.0f) continue;
				short bone_index = bone_indices_1[i];
				auto& bone = skeleton[bone_index];
				vec3 addition{};
				util::mtx::multiply_vector(bone, &positions[vert_1], &addition);
				guVecScale(&addition, &addition, bone_weights_1.weights[i]);
				guVecAdd(&position_1, &addition, &position_1);
			}
			//*/
			
			auto& bone_indices_2 = bi[vert_2].indices;
			auto& bone_weights_2 = w[vert_2];
			vec3 position_2 {};
			///*
			for (std::size_t i = 0; i < 5; i++) {
				if (bone_weights_2.weights[i] == 0.0f) continue;
				short bone_index = bone_indices_2[i];
				auto& bone = skeleton[bone_index];
				vec3 addition{};
				util::mtx::multiply_vector(bone, &positions[vert_2], &addition);
				guVecScale(&addition, &addition, bone_weights_2.weights[i]);
				guVecAdd(&position_2, &addition, &position_2);
			}
			//*/
			
			auto& bone_indices_3 = bi[vert_3].indices;
			auto& bone_weights_3 = w[vert_3];
			vec3 position_3 = {}; 
			///*
			for (std::size_t i = 0; i < 5; i++) {
				if (bone_weights_3.weights[i] == 0.0f) continue;
				short bone_index = bone_indices_3[i];
				auto& bone = skeleton[bone_index];
				vec3 addition{};
				util::mtx::multiply_vector(bone, &positions[vert_3], &addition);
				guVecScale(&addition, &addition, bone_weights_3.weights[i]);
				guVecAdd(&position_3, &addition, &position_3);
			}
			//*/
			if (is_blue) {
				GX_Position3f32(position_1.x, position_1.y, position_1.z);
				GX_Color1u32(0x1140FFFF);
				GX_TexCoord2f32(uv1[vert_1].x, 1-uv1[vert_1].y);
				GX_Position3f32(position_2.x, position_2.y, position_2.z);
				GX_Color1u32(0x1140FFFF);
				GX_TexCoord2f32(uv1[vert_2].x, 1-uv1[vert_2].y);
				GX_Position3f32(position_3.x, position_3.y, position_3.z);
				GX_Color1u32(0x1140FFFF);
				GX_TexCoord2f32(uv1[vert_3].x, 1-uv1[vert_3].y);
			} else {
				GX_Position3f32(position_1.x, position_1.y, position_1.z);
				GX_Color1u32(0xFFFFFFFF);
				GX_TexCoord2f32(uv1[vert_1].x, 1-uv1[vert_1].y);
				GX_Position3f32(position_2.x, position_2.y, position_2.z);
				GX_Color1u32(0xFFFFFFFF);
				GX_TexCoord2f32(uv1[vert_2].x, 1-uv1[vert_2].y);
				GX_Position3f32(position_3.x, position_3.y, position_3.z);
				GX_Color1u32(0xFFFFFFFF);
				GX_TexCoord2f32(uv1[vert_3].x, 1-uv1[vert_3].y);
			}
		}
		GX_End();
		GX_Flush();
		if (use_ui && ui_tex != nullptr) {
			ui_tex->resource_unbind();
		}
		else {
			tex.resource_unbind();
		}
    }
};