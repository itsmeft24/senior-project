#pragma once
#include "Defs.hpp"
namespace ntr::gfx {
	class Camera
	{
	private:
		Mtx vm;
		Mtx44 pm;
	public:
		vec3 position;
		quat rotation;
	public:
		Camera();
		Camera(const Camera&) = delete;
		Camera(Camera&&) = default;
		Camera& operator=(const Camera&) = delete;
		void update();
		Mtx44* projection_matrix();
		Mtx* view_matrix();
		vec2 world_to_screen(vec3 world);
	};
};