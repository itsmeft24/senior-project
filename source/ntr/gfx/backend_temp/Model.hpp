#pragma once
#include "Mesh.hpp"
#include "Camera.hpp"
#include "AnimationDataFile.hpp"
#include <grrlib.h>

namespace ntr::gfx {
	struct __attribute__((packed)) Bone {
		short id;
		short parent_id;
		char name[64];
		Mtx44 local_matrix;
		Mtx44 inv_model_matrix;
	};
	enum DrawType {
		Simple,
		UITexture,
		BlueBorder
	};
	class Model
	{
	protected:
		Bone* bones;
		std::uint16_t bone_count;
		Mesh** meshes;
		std::uint16_t mesh_count;
	public:
		vec3 position;
        vec3 rotation;
        vec3 scale;
	private:
		Mtx44* requested_transforms;
		vec3 root_pos;
	private:
		Mtx44* intermediates;
		Mtx44* calculated_bone_matrices;
		void calculate_bone_matrices();
	public:
		Model();
		~Model();
		Model(const Model&) = delete;
		Model& operator=(const Model&) = delete;
		inline void set_position(const vec3& pos) {
			position = pos;
		}
        inline void set_rotation(const vec3& rot) {
			rotation = rot;
		}
        inline void set_scale(const vec3& scl) {
			scale = scl;
		}
        bool load(const char* path, const char* asset_folder);
		bool draw(Camera& camera, Renderer& renderer, DrawType draw_type);//ntr::gfx::Camera& camera);
		void animate(const ntr::gfx::AnimationDataFile& anim, float time);
	};
};