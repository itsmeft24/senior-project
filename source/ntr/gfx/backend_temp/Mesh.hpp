#pragma once
#include <cstddef>
#include <cstdint>
#include "Defs.hpp"
#include "Texture.hpp"
#include "Renderer.hpp"
#include "../../global_mem.h"

namespace ntr::gfx {
	struct Bone;
	class Mesh
	{
	public:
		struct BoneIndices {
			std::uint16_t indices[5];
		};
		struct BoneWeights {
			float weights[5];
		};
		std::size_t index_count;
		std::uint16_t* indices;
		std::size_t vertex_count;
        vec3* positions;
		vec3* normals;
		vec2* uv1;
		BoneIndices* bi;
		BoneWeights* w;

		Texture tex;
		Texture* ui_tex;
		std::string name;
		inline Mesh(std::size_t _index_count, std::uint16_t* _indices, std::size_t _vertex_count, vec3* _positions, vec3* _normals, vec2* _uv1, BoneIndices* _bi, BoneWeights* _w, char* _path, char* ui_path, std::string _name) : 
			index_count(_index_count),
			indices(_indices),
			vertex_count(_vertex_count),
            positions(_positions),
			normals(_normals),
			uv1(_uv1),
			bi(_bi),
			w(_w),
            tex(_path),
			name(_name)
		{
			if (ui_path != nullptr) {
				FILE* ui_tex_file = fopen(ui_path, "rb");
				if (ui_tex_file == nullptr) {
					ui_tex = nullptr;
				} else {
					fclose(ui_tex_file);
					ui_tex = new Texture(ui_path);
					INC_SIZE(sizeof(Texture));
				}
			} else {
				ui_tex = nullptr;
			}
		}
		~Mesh();
		Mesh(const Mesh&) = delete;
		Mesh(Mesh&&) = default;
		Mesh& operator=(const Mesh&) = delete;
		void draw(Renderer& renderer, Mtx44* skeleton, bool use_ui, bool is_blue);//Camera& camera,, Mat44& model_matrix);
	};
};