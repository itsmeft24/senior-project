#pragma once
#include <iostream>
#include <numbers>
#include <bit>
#include <utility>
#include <vector>
#include <filesystem>
#include <unordered_map>
#include <grrlib.h>
#include "Defs.hpp"
#include "../../Util.hpp"

namespace ntr::gfx {
	constexpr double ShortAngleAxisScale = 2048.0f / 3.14159265358979323846f;
	constexpr double ByteAxisScale = 127.5f / 3.14159265358979323846f;

	enum MotionType
	{
		CameraAnimation,
		StandardAnimation
	};
	enum PositionType
	{
		Linear,
		Bezier
	};
	enum RotationType
	{
		ByteAxis,
		ShortAxis,
		Quaternion,
		HalfQuaternion
	};
	struct ByteAngleAxis {
		char angle;
        char axis[3];
		inline float get_angle() {
			return float(angle / ByteAxisScale);
		}
		inline vec3 get_axis() {
			return vec3{
				float(double(axis[0]) / 127.0f),
				float(double(axis[1]) / 127.0f),
				float(double(axis[2]) / 127.0f)
			};
		}
	};
	struct ShortAngleAxis {
		short angle;
		std::int16_t axis[3];
		inline float get_angle() const {
			return float(angle / ShortAngleAxisScale);
		}
		inline vec3 get_axis() const {
			return vec3{
				float(double(axis[0]) / 32767.0f),
				float(double(axis[1]) / 32767.0f),
				float(double(axis[2]) / 32767.0f)
			};
		}
	};
	struct BezierPositionKey {
		float time;
		vec3 pos;
		vec3 tangent_in;
		vec3 tangent_out;
	};
	struct LinearPositionKey {
		float time;
		vec3 pos;
	};
	struct ByteRotationKey {
		float time;
		ByteAngleAxis rotation;
	};
	struct ShortRotationKey {
		float time;
		ShortAngleAxis rotation;
	};
	struct QuaternionRotationKey {
		float time;
        quat rotation;
	};
	/*
	struct HalfQuaternionRotationKey {
		float time;
		std::int16_t rotation[4];
		constexpr quat get_rotation() const {
			return quat{
				util::f16_to_f32(rotation[0]),
				util::f16_to_f32(rotation[1]),
				util::f16_to_f32(rotation[2]),
				util::f16_to_f32(rotation[3]),
			};
		}
	};
	*/
	struct CameraInfoKey {
		float time;
		float fov;
	};
	struct MotionInfo {
		float duration;
		int rotation_type;
		int position_type;
		unsigned int position_count;
		unsigned int rotation_count;
		unsigned int camera_info_count;
		unsigned int padding;
	};
	struct KeySetInfo {
		unsigned int key_count;
		unsigned int bone_index;
	};
	struct AnimationDataFileHeader { // i named this
		int version;
		int type;
		unsigned int frame_count;
		unsigned int bone_target_count;
	};

	class AnimationDataFile {
	public:
		struct Entry {
			LinearPositionKey* position_keys;
			std::size_t position_key_count;
			QuaternionRotationKey* rotation_keys;
			std::size_t rotation_key_count;
		};
		AnimationDataFileHeader header;
		std::unordered_map<std::string, Entry> bone_targets;
		MotionInfo motion_info;
	public:
		AnimationDataFile();
		~AnimationDataFile();
		AnimationDataFile(const AnimationDataFile&) = delete;
		AnimationDataFile(AnimationDataFile&&) = default;
		AnimationDataFile& operator=(const AnimationDataFile&) = delete;
		static std::pair<vec3, quat> sample(const AnimationDataFile::Entry& entry, double time);
		std::pair<vec3, quat> get_last_transform(const std::string& name) const;
		bool load(const char* path);
	};
};