#include "AnimationDataFile.hpp"
#include "../../global_mem.h"

template<typename T> T read_object(FILE* f) {
    T obj{};
    fread(&obj, sizeof(T), 1, f);
    return obj;
}

template<typename T> std::vector<T> read_array(FILE* f, std::size_t len) {
    std::vector<T> obj(len);
    fread(obj.data(), sizeof(T) * len, 1, f);
    return obj;
}

namespace ntr::gfx {
    AnimationDataFile::AnimationDataFile()
    {
    }

    AnimationDataFile::~AnimationDataFile()
    {
        for (auto& target: bone_targets) {
            delete[] target.second.position_keys;
            delete[] target.second.rotation_keys;
        }
    }

    std::pair<vec3, quat> AnimationDataFile::sample(const AnimationDataFile::Entry &bone_target, double time)
    {

        vec3 lerped_position{};
        quat lerped_rotation{};

        if (bone_target.position_key_count != 0) {
            std::size_t index_of_start_position = 0;
            for (std::size_t i = 0; i < bone_target.position_key_count; i++) {
                const auto& elem = bone_target.position_keys[i];
                if (elem.time > time) {
                    index_of_start_position = i - 1;
                    break;
                }
            }

            const auto& start_position = bone_target.position_keys[index_of_start_position];
            const auto& end_position = bone_target.position_keys[index_of_start_position + 1];

            auto time_since_start_position = (time - start_position.time);
            auto time_between_position_keys = (end_position.time - start_position.time);
            auto t_position = util::clamp(time_since_start_position / time_between_position_keys, 0.0, 1.0);

            lerped_position = util::lerp_vec(t_position, start_position.pos, end_position.pos);
        }

        if (bone_target.rotation_key_count != 0) {
            std::size_t index_of_start_rotation = 0;
            for (std::size_t i = 0; i < bone_target.rotation_key_count; i++) {
                const auto& elem = bone_target.rotation_keys[i];
                if (elem.time > time) {
                    index_of_start_rotation = i - 1;
                    break;
                }
            }

            const auto& start_rotation = bone_target.rotation_keys[index_of_start_rotation];
            const auto& end_rotation = bone_target.rotation_keys[index_of_start_rotation + 1];

            auto time_since_start_rotation = (time - start_rotation.time);
            auto time_between_rotation_keys = (end_rotation.time - start_rotation.time);
            auto t_rotation = util::clamp(time_since_start_rotation / time_between_rotation_keys, 0.0, 1.0);

            lerped_rotation = util::lerp_quat(static_cast<float>(t_rotation), start_rotation.rotation, end_rotation.rotation);
        }

        return { lerped_position, lerped_rotation };
    }

    std::pair<vec3, quat> AnimationDataFile::get_last_transform(const std::string& name) const
    {

        vec3 lerped_position{};
        quat lerped_rotation{};
        /*
        if (bone_targets.at(name).position_keys.size() != 0) {
            lerped_position = bone_targets.at(name).position_keys[bone_targets.at(name).position_keys.size() - 1].pos;
        }

        if (bone_targets.at(name).rotation_keys.size() != 0) {
            lerped_rotation = bone_targets.at(name).rotation_keys[bone_targets.at(name).rotation_keys.size() - 1].rotation;
        }
        */
        if (bone_targets.at(name).position_key_count != 0) {
            lerped_position = bone_targets.at(name).position_keys[bone_targets.at(name).position_key_count - 1].pos;
        }

        if (bone_targets.at(name).rotation_key_count != 0) {
            lerped_rotation = bone_targets.at(name).rotation_keys[bone_targets.at(name).rotation_key_count - 1].rotation;
        }
        return { lerped_position, lerped_rotation };
    }

    bool AnimationDataFile::load(const char* p)
    {
        FILE* f = fopen(p, "rb");
        header = read_object<AnimationDataFileHeader>(f);
        std::vector<std::string> string_table{};
        for (int x = 0; x < header.bone_target_count; x++) {
            unsigned int len = read_object<unsigned int>(f);
            char temp_buffer[260]{};
            fread(reinterpret_cast<void*>(temp_buffer), len, 1, f);
            string_table.push_back(temp_buffer);
            bone_targets.insert({ temp_buffer , {} });
        }
        motion_info = read_object<MotionInfo>(f);
        for (int x = 0; x < motion_info.position_count; x++) {
            KeySetInfo key_set_info = read_object<KeySetInfo>(f);
            auto& destination = bone_targets.at(string_table[key_set_info.bone_index]);
            /*
            if (motion_info.position_type == PositionType::Bezier) {
                auto keys = read_array<BezierPositionKey>(f, key_set_info.key_count);
                for (auto& key : keys) {
                    destination.position_keys.push_back(LinearPositionKey{key.time, key.pos});
                }
            }
            */
            if (motion_info.position_type == PositionType::Linear) {
                LinearPositionKey* pos_keys = new LinearPositionKey[key_set_info.key_count];
                fread(pos_keys, sizeof(LinearPositionKey) * key_set_info.key_count, 1, f);
                destination.position_keys = pos_keys;
                destination.position_key_count = key_set_info.key_count;
                INC_SIZE(key_set_info.key_count * sizeof(LinearPositionKey));
                /*
                auto keys = read_array<LinearPositionKey>(f, key_set_info.key_count);
                for (auto& key : keys) {
                    destination.position_keys.push_back(key);
                }
                */
            }
        }
        for (int x = 0; x < motion_info.rotation_count; x++) {
            KeySetInfo key_set_info = read_object<KeySetInfo>(f);
            auto& destination = bone_targets.at(string_table[key_set_info.bone_index]);
            if (motion_info.rotation_type == RotationType::Quaternion) {
                QuaternionRotationKey* rot_keys = new QuaternionRotationKey[key_set_info.key_count];
                fread(rot_keys, sizeof(QuaternionRotationKey) * key_set_info.key_count, 1, f);
                destination.rotation_keys = rot_keys;
                destination.rotation_key_count = key_set_info.key_count;
                INC_SIZE(key_set_info.key_count * sizeof(QuaternionRotationKey));
                /*
                auto keys = read_array<QuaternionRotationKey>(f, key_set_info.key_count);
                for (auto& key : keys) {
                    destination.rotation_keys.push_back(key);
                }
                */
            }
            /*
            else if (motion_info.rotation_type == RotationType::HalfQuaternion) {
                auto keys = read_array<HalfQuaternionRotationKey>(f, key_set_info.key_count);
                for (auto& key : keys) {
                    destination.rotation_keys.emplace_back(key.time, key.get_rotation());
                }
            }
            */
        }
        
        for (const auto& i : bone_targets) {
            INC_SIZE(i.first.capacity());
            // INC_SIZE(i.second.position_keys.capacity() * sizeof(LinearPositionKey));
            // INC_SIZE(i.second.rotation_keys.capacity() * sizeof(QuaternionRotationKey));
        }

        return true;
    }
};