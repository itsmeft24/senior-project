#include "Camera.hpp"
#include "../../Util.hpp"
#include <GRRLIB.h>
namespace ntr::gfx {
	Camera::Camera() {
		position = { 0, 0, 0 };
        rotation = {};
        guPerspective(pm, 43.21f * (45/58.54), (f32)rmode->fbWidth/rmode->efbHeight, 0.1, 2000);
	}
    
	Mtx44* Camera::projection_matrix() {
		return &pm;
	}
	
	Mtx* Camera::view_matrix() {
		return &vm;
	}

    vec2 Camera::world_to_screen(vec3 world) {
        vec4 temp{0,0,0,1};
        vec4 clip_space{0,0,0,1};
        guVecMultiply(vm, &world, reinterpret_cast<vec3*>(&temp));
        util::mtx::multiply_vector(pm, &temp, &clip_space);
        vec3 ndc_space{};
        if (clip_space.w == 0.0f) {
            clip_space.w = 1.0f;
        }
        guVecScale(reinterpret_cast<vec3*>(&clip_space), &ndc_space, 1.0f / clip_space.w);
        return vec2{
            (ndc_space.x + 1.0f) / 2.0f * rmode->fbWidth,
            (ndc_space.y + 1.0f) / 2.0f * rmode->efbHeight,
        };
    }

    void Camera::update() {
        quat quat_swapped = ntr::util::swizzle_quat(rotation);
        Mtx rot_mat{};
        c_guMtxQuat(rot_mat, &quat_swapped);

        vec3 default_eye = {0, 0, -1};
        vec3 default_look = {0, 0, 1};
        vec3 default_up = {0, 1, 0};
        vec3 look{};
        vec3 up{};
        vec3 eye{};
        
        guVecMultiply(rot_mat, &default_look, &look);
        guVecMultiply(rot_mat, &default_up, &up);
        guVecMultiply(rot_mat, &default_eye, &eye);

        eye.x += position.x;
        eye.y += position.y;
        eye.z += position.z;
    
        look.x += position.x;
        look.y += position.y;
        look.z += position.z;

        guLookAt(vm, &eye, &up, &look);
        GRRLIB_Camera3dSettings(eye.x, eye.y, eye.z, up.x, up.y, up.z, look.x, look.y, look.z);
    }
};