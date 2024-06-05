#pragma once

#include <iostream>
#include <unordered_map>
#include <gccore.h>
#include <cstdint>

namespace ntr { 
	namespace gfx {
		class Renderer {
		private:
			bool window_closed;
		public:
			Renderer();
			~Renderer();
			Renderer(const Renderer&) = delete;
			Renderer& operator=(const Renderer&) = delete;

			bool is_window_closed();
			void set_window_closed();
			void start_3d();
			void start_2d();
			void set_vertex_desc_textured();
			void set_vertex_desc_colored();
			void set_vertex_desc_2d_textured();
			void end_frame();
		};
	}
};