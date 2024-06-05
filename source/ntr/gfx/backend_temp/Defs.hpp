#pragma once
#include <ogc/gu.h>
using vec3 = guVector;
using quat = guQuaternion;
struct vec2 {
    float x;
    float y;
};
struct vec4 {
    float x;
    float y;
    float z;
    float w;
    inline operator vec3() {
        return vec3{x,y,z};
    }
};
static_assert(sizeof(vec4) == 4 * 4);
static_assert(sizeof(vec3) == 3 * 4);
static_assert(sizeof(vec2) == 2 * 4);
inline vec2 operator-(const vec2& a, const vec2& b) {
    return vec2{a.x - b.x, a.y - b.y};
}

inline vec2 operator+(const vec2& a, const vec2& b) {
    return vec2{a.x + b.x, a.y + b.y};
}

inline vec2 operator*(const vec2& a, float s) {
    return vec2{a.x * s, a.y * s};
}