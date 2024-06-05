#include "AnimationSet.hpp"

namespace ntr {
    AnimationSet::AnimationSet() {
        time_in_anim = 0;
        selected_anim = 0;
    }
    void AnimationSet::add_animation(const char* path, bool is_looping, int hold_frame) {
        ntr::gfx::AnimationDataFile data_file{};
        data_file.load(path);
        if (hold_frame == -1) {
            hold_frame = data_file.header.frame_count - 2;
        }
        animations.emplace_back(Animation { std::move(data_file), is_looping, hold_frame });
    }
    void AnimationSet::set_index(int index) {
        if (index < animations.size() && index != selected_anim) {
            selected_anim = index;
            time_in_anim = 0;
        }
    }
    int AnimationSet::get_index() const {
        return selected_anim;
    }
    /*
    int AnimationSet::get_frame() const {
        return frame_in_anim;
    }
    */
    float AnimationSet::get_time() const {
        return time_in_anim;
    }
    bool AnimationSet::current_finished() const {
        const auto& animation = animations[selected_anim];
        //if (!animation.is_looping) {
            return time_in_anim >= (animation.hold_frame / 30.0f);
        //}
        //return false;
    }
    void AnimationSet::tick(float dt) {
        time_in_anim += dt;
        const auto& animation = animations[selected_anim];
        if (animation.is_looping) {
			if (time_in_anim >= animation.data_file.motion_info.duration) {
				time_in_anim = 0;
			}
        }
        else {
            if (time_in_anim >= animation.hold_frame / 30.0f) {
				time_in_anim = animation.hold_frame / 30.0f;
			}
        }
    }
    void AnimationSet::animate_model(ntr::gfx::Model& model) const {
        const auto& animation = animations[selected_anim];
        // time_in_animfloat fraction = ((float)frame_in_anim / (float)animation.data_file.header.frame_count);
		model.animate(animation.data_file, time_in_anim);
    }
};