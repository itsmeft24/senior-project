#include <vector>
#include "gfx/backend_temp/AnimationDataFile.hpp"
#include "gfx/backend_temp/Model.hpp"

namespace ntr {
    class AnimationSet {
    private:
        struct Animation {
            ntr::gfx::AnimationDataFile data_file;
            bool is_looping;
            int hold_frame;
        };
        //int frame_in_anim;
        int selected_anim;
        float time_in_anim;
        std::vector<Animation> animations;
    public:
        AnimationSet();
        void add_animation(const char* path, bool is_looping, int hold_frame);
        void set_index(int index);
        int get_index() const;
        //int get_frame() const;
        float get_time() const;
        bool current_finished() const;
        void tick(float dt);
        void animate_model(ntr::gfx::Model& model) const;
    };
};