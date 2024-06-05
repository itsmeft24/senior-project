#include "Model.hpp"
#include "../../Util.hpp"
#include <grrlib.h>
#include <cstring>
#include <ogc/lwp_watchdog.h>
#include <numeric>
#include <cmath>
#include "../../global_mem.h"

static vec3 x_axis_vec{1.0f, 0.0f, 0.0f};
static vec3 y_axis_vec{0.0f, 1.0f, 0.0f};
static vec3 z_axis_vec{0.0f, 0.0f, 1.0f};

namespace ntr::gfx {
    Model::Model() {
        root_pos = {0,0,0};
    }
    
    Model::~Model() {
        if (meshes != nullptr) {
            for (int i = 0; i < mesh_count; i++) {
                delete meshes[i];
            }
            delete[] meshes;
        }
        if (bones != nullptr) {
            delete[] bones;
        }
        if (requested_transforms != nullptr) {
            delete[] requested_transforms;
        }
        if (intermediates != nullptr) {
            delete[] requested_transforms;
        }
        if (calculated_bone_matrices != nullptr) {
            delete[] requested_transforms;
        }
    }

    bool Model::load(const char* path, const char* asset_folder) {
        FILE* model_file = fopen(path, "rb");
        if (model_file == nullptr) {
            printf("Failed opening model file: %s.", path);
        }
        fread(reinterpret_cast<void*>(&mesh_count), sizeof(std::uint16_t), 1, model_file);
        fread(reinterpret_cast<void*>(&bone_count), sizeof(std::uint16_t), 1, model_file);
        
        bones = new Bone[bone_count];
        fread(bones, sizeof(Bone) * bone_count, 1, model_file);
        INC_SIZE(bone_count * sizeof(Bone));

        void* mesh_names = malloc(16 * mesh_count);
        fread(mesh_names, 16 * mesh_count, 1, model_file);
        
        meshes = new Mesh*[mesh_count];
        for (int i = 0; i < mesh_count; i++) {
            char* mesh_name = reinterpret_cast<char*>(reinterpret_cast<std::uintptr_t>(mesh_names) + 16 * i);
            
            char texture_path[256];
            snprintf(texture_path, 256, "%s/tex/%s.tex0", asset_folder, mesh_name);

            char ui_texture_path[256];
            snprintf(ui_texture_path, 256, "%s/tex/%s_ui.tex0", asset_folder, mesh_name);

            std::uint32_t vertex_count, face_count = 0;
            fread(reinterpret_cast<void*>(&vertex_count), sizeof(std::uint32_t), 1, model_file);
            fseek(model_file, 4, SEEK_CUR);
            fread(reinterpret_cast<void*>(&face_count), sizeof(std::uint32_t), 1, model_file);
            printf("Mesh: %s has %d Vertices and %d Faces", mesh_name, vertex_count, face_count);
            
            vec3* positions = reinterpret_cast<vec3*>(malloc(sizeof(vec3) * vertex_count));
            vec2* uvs = reinterpret_cast<vec2*>(malloc(sizeof(vec2) * vertex_count));
            std::uint16_t* index_buffer = reinterpret_cast<std::uint16_t*>(malloc(sizeof(std::uint16_t) * face_count * 3));
            
            fread(reinterpret_cast<void*>(positions), sizeof(vec3) * vertex_count, 1, model_file);
            fread(reinterpret_cast<void*>(uvs), sizeof(vec2) * vertex_count, 1, model_file);
            fread(reinterpret_cast<void*>(index_buffer), sizeof(std::uint16_t) * face_count * 3, 1, model_file);
            
            Mesh::BoneIndices* bone_indices = reinterpret_cast<Mesh::BoneIndices*>(malloc(sizeof(Mesh::BoneIndices) * vertex_count));
            Mesh::BoneWeights* bone_weights = reinterpret_cast<Mesh::BoneWeights*>(malloc(sizeof(Mesh::BoneWeights) * vertex_count));
            fread(reinterpret_cast<void*>(bone_indices), sizeof(Mesh::BoneIndices) * vertex_count, 1, model_file);
            fread(reinterpret_cast<void*>(bone_weights), sizeof(Mesh::BoneWeights) * vertex_count, 1, model_file);
            
            INC_SIZE(sizeof(vec3) * vertex_count + sizeof(vec2) * vertex_count + sizeof(std::uint16_t) * face_count * 3+ sizeof(Mesh::BoneIndices) * vertex_count + sizeof(Mesh::BoneWeights) * vertex_count);
            INC_SIZE(sizeof(Mesh) + sizeof(Mesh*));
            meshes[i] = new Mesh(face_count, index_buffer, vertex_count, positions, nullptr, uvs, bone_indices, bone_weights, texture_path, ui_texture_path, mesh_name);
        }
        free(mesh_names);
        scale = {1,1,1};

        // Empty out all the requested transforms (transforms relative to the bone's rest pose)
        requested_transforms = new Mtx44[bone_count];
        INC_SIZE(sizeof(Mtx44) * bone_count);
        INC_SIZE(sizeof(Mtx44) * bone_count);
        INC_SIZE(sizeof(Mtx44) * bone_count);
        for (int i = 0; i < bone_count; i++) {
            guMtx44Identity(requested_transforms[i]);
        }
        intermediates = new Mtx44[bone_count];
        calculated_bone_matrices = new Mtx44[bone_count];
        return true;
    }

    void Model::calculate_bone_matrices() {
        for (int i = 0; i < bone_count; i++)
		{
			util::mtx::multiply(bones[i].local_matrix, requested_transforms[i], intermediates[i]);
		}
        // "intermediates" now contains the local transforms for each bone
        // we'll use calculated_bone_matrices to store the model transforms for each bone so we dont allocate yet ANOTHER array
		guMtx44Copy(intermediates[0], calculated_bone_matrices[0]);
		for (int i = 1; i < bone_count; i++)
		{
			short parent = bones[i].parent_id;
			util::mtx::multiply(calculated_bone_matrices[parent], intermediates[i], calculated_bone_matrices[i]);
		}
		for (int i = 0; i < bone_count; i++)
		{
			util::mtx::multiply(calculated_bone_matrices[i], bones[i].inv_model_matrix, calculated_bone_matrices[i]);
		}
    }

    void Model::animate(const AnimationDataFile& anim, float time) {
        auto before = gettime();
        double time_to_apply_transforms = 0.0;
		for (std::size_t i = 0; i < bone_count; i++) {
            auto iterator = anim.bone_targets.find(bones[i].name);
            if (iterator != anim.bone_targets.end()) {
                
                Mtx44 trans_mat{};
                Mtx44 rot_mat{};
                auto [trans, rot] = AnimationDataFile::sample(iterator->second, time);

                auto before_transform = gettime();
                util::mtx::translation(trans_mat, trans.x, trans.y, trans.z);
                guQuaternion quat_swapped{ rot.x, rot.y, rot.z, -rot.w };
                util::mtx::rotation_quaternion(rot_mat, &quat_swapped);
                util::mtx::multiply(trans_mat, rot_mat, requested_transforms[i]);
                auto after_transform = gettime();
                time_to_apply_transforms += double(diff_nsec(before_transform, after_transform));
            }
		}
        auto after = gettime();
        //printf("Took: %f s to animate model. %f was spent mathing.\n", float(double(diff_nsec(before, after)) / 1000000000.0), float(time_to_apply_transforms/ 1000000000.0));
    }
    
    bool Model::draw(Camera& camera, Renderer& renderer, DrawType draw_type) {
        /**/
        calculate_bone_matrices();

        if (draw_type == DrawType::BlueBorder) renderer.set_vertex_desc_colored();
        else renderer.set_vertex_desc_textured();
        
        Mtx* view_matrix = camera.view_matrix();
        Mtx model_matrix{};
        Mtx m{};
        Mtx model_view_matrix{}, model_view_matrix_inverse{};

        guMtxIdentity(model_matrix);

        if((scale.x != 1.0f) || (scale.y != 1.0f) || (scale.z != 1.0f)) {
            guMtxIdentity(m);
            guMtxScaleApply(m, m, scale.x, scale.y, scale.z);

            guMtxConcat(m, model_matrix, model_matrix);
        }

        if((rotation.x != 0.0f) || (rotation.y != 0.0f) || (rotation.z != 0.0f)) {
            Mtx rx, ry, rz;
            guMtxIdentity(m);
            guMtxRotAxisDeg(rx, &x_axis_vec, rotation.x);
            guMtxRotAxisDeg(ry, &y_axis_vec, rotation.y);
            guMtxRotAxisDeg(rz, &z_axis_vec, rotation.z);
            guMtxConcat(rz, ry, m);
            guMtxConcat(m, rx, m);
            
            guMtxConcat(m, model_matrix, model_matrix);
        }

        if((position.x != 0.0f) || (position.y != 0.0f) || (position.z != 0.0f)) {
            guMtxIdentity(m);
            guMtxTransApply(m, m, position.x, position.y, position.z);

            guMtxConcat(m, model_matrix, model_matrix);
        }

        guMtxConcat(*view_matrix, model_matrix, model_view_matrix);
        GX_LoadPosMtxImm(model_view_matrix, GX_PNMTX0);

        guMtxInverse(model_view_matrix, model_view_matrix_inverse);
        guMtxTranspose(model_view_matrix_inverse, model_view_matrix);
        GX_LoadNrmMtxImm(model_view_matrix, GX_PNMTX0);

        for (std::size_t i = 0; i < mesh_count; i++) {
            // auto sbefore = gettime();
            meshes[i]->draw(renderer, calculated_bone_matrices, draw_type == DrawType::UITexture, draw_type == BlueBorder);
            //auto safter = gettime();
            // printf("Took: %f s to calculate draw %s.\n", float(double(diff_nsec(sbefore, safter)) / 1000000000.0), meshes[i]->name.c_str());
        }
        return true;
    }
};