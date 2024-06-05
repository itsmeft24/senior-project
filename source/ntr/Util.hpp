#pragma once
#include <bit>
#include <ogc/gu.h>
#include "gfx/backend_temp/Defs.hpp"

#ifdef GEKKO
extern "C" void mtx_multiply_44(const Mtx44 a, const Mtx44 b, Mtx44 ab);
#endif
#include <cmath>

namespace ntr::util {
    template<typename T> T lerp(T t, T a, T b){
        return (1 - t) * a + t * b;
    }

    inline vec3 lerp_vec(float t, vec3 a, vec3 b) {
        vec3 b_t{};
        guVecScale(&b, &b_t, t);
        vec3 a_one_minus_t{};
        guVecScale(&a, &a_one_minus_t, 1 - t);
        vec3 dst{};
        guVecAdd(&a_one_minus_t, &b_t, &dst);
        return dst;
    }
    
    inline quat lerp_quat(float t, quat a, quat b) {
        return {
            lerp(t, a.x, b.x),
            lerp(t, a.y, b.y),
            lerp(t, a.z, b.z),
            lerp(t, a.w, b.w),
        };
    }

    inline float dot_product(quat a, quat b) {
        return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    }

    inline quat slerp_quat(float t, quat a, quat b) {
        float dot = dot_product(a, b);
        
        if (dot < 0.0f) {
            quat _b = {-b.x, -b.y, -b.z, -b.w};
            return slerp_quat(t, a, _b);
        }
        
        if (dot > 0.9995f) {
            // lerp if the quats are close enough
            return lerp_quat(t, a, b);
        }
        
        float angle = std::acos(dot); // both should quats should have been normalied if not idk ill kms
        float sin_theta = std::sin(angle);
        float inv_sin_theta = 1.0f / sin_theta; // atp sin_theta shouldnt be close to zero if it is idk
        float coeff1 = std::sin((1.0f - t) * angle) * inv_sin_theta;
        float coeff2 = std::sin(t * angle) * inv_sin_theta;
        
        return {
            a.x * coeff1 + b.x * coeff2,
            a.y * coeff1 + b.y * coeff2,
            a.z * coeff1 + b.z * coeff2,
            a.w * coeff1 + b.w * coeff2
        };
    }

    inline quat swizzle_quat(quat quat) {
        return { -quat.z, quat.w, quat.x, quat.y };
    }

    template<typename T> T clamp(T x, T min, T max){
        if (x >= max) {
            return max;
        }
        if (x <= min) {
            return min;
        } 
        return x;
    }

    namespace mtx {
        inline void translation(Mtx44 mt, f32 xT, f32 yT, f32 zT) {
            mt[0][0] = 1.0f;  mt[0][1] = 0.0f;  mt[0][2] = 0.0f;  mt[0][3] = xT;
            mt[1][0] = 0.0f;  mt[1][1] = 1.0f;  mt[1][2] = 0.0f;  mt[1][3] = yT;
            mt[2][0] = 0.0f;  mt[2][1] = 0.0f;  mt[2][2] = 1.0f;  mt[2][3] = zT;
            mt[3][0] = 0.0f;  mt[3][1] = 0.0f;  mt[3][2] = 0.0f;  mt[3][3] = 1.0f;
        }

	    inline void rotation_quaternion(Mtx44 m, const guQuaternion* a) {
            guMtxRowCol(m, 0, 0) = 1.0f - 2.0f * a->y * a->y - 2.0f * a->z * a->z;
            guMtxRowCol(m, 1, 0) = 2.0f * a->x * a->y - 2.0f * a->z * a->w;
            guMtxRowCol(m, 2, 0) = 2.0f * a->x * a->z + 2.0f * a->y * a->w;
            guMtxRowCol(m, 3, 0) = 0.0f;

            guMtxRowCol(m, 0, 1) = 2.0f * a->x * a->y + 2.0f * a->z * a->w;
            guMtxRowCol(m, 1, 1) = 1.0f - 2.0f * a->x * a->x - 2.0f * a->z * a->z;
            guMtxRowCol(m, 2, 1) = 2.0f * a->z * a->y - 2.0f * a->x * a->w;
            guMtxRowCol(m, 3, 1) = 0.0f;

            guMtxRowCol(m, 0, 2) = 2.0f * a->x * a->z - 2.0f * a->y * a->w;
            guMtxRowCol(m, 1, 2) = 2.0f * a->z * a->y + 2.0f * a->x * a->w;
            guMtxRowCol(m, 2, 2) = 1.0f - 2.0f * a->x * a->x - 2.0f * a->y * a->y;
            guMtxRowCol(m, 3, 2) = 0.0f;

            guMtxRowCol(m, 0, 3) = 0.0f;
            guMtxRowCol(m, 1, 3) = 0.0f;
            guMtxRowCol(m, 2, 3) = 0.0f;
            guMtxRowCol(m, 3, 3) = 1.0f;
        }

        inline void multiply(const Mtx44 a, const Mtx44 b, Mtx44 ab) {
            #ifdef GEKKO
            mtx_multiply_44(a,b,ab);
            #else
            
            Mtx44 tmp;
            Mtx44P m;

            if (ab == b || ab == a)
                m = tmp;
            else
                m = ab;

            m[0][0] = a[0][0] * b[0][0] + a[0][1] * b[1][0] + a[0][2] * b[2][0] + a[0][3] * b[3][0];
            m[0][1] = a[0][0] * b[0][1] + a[0][1] * b[1][1] + a[0][2] * b[2][1] + a[0][3] * b[3][1];
            m[0][2] = a[0][0] * b[0][2] + a[0][1] * b[1][2] + a[0][2] * b[2][2] + a[0][3] * b[3][2];
            m[0][3] = a[0][0] * b[0][3] + a[0][1] * b[1][3] + a[0][2] * b[2][3] + a[0][3] * b[3][3];

            m[1][0] = a[1][0] * b[0][0] + a[1][1] * b[1][0] + a[1][2] * b[2][0] + a[1][3] * b[3][0];
            m[1][1] = a[1][0] * b[0][1] + a[1][1] * b[1][1] + a[1][2] * b[2][1] + a[1][3] * b[3][1];
            m[1][2] = a[1][0] * b[0][2] + a[1][1] * b[1][2] + a[1][2] * b[2][2] + a[1][3] * b[3][2];
            m[1][3] = a[1][0] * b[0][3] + a[1][1] * b[1][3] + a[1][2] * b[2][3] + a[1][3] * b[3][3];

            m[2][0] = a[2][0] * b[0][0] + a[2][1] * b[1][0] + a[2][2] * b[2][0] + a[2][3] * b[3][0];
            m[2][1] = a[2][0] * b[0][1] + a[2][1] * b[1][1] + a[2][2] * b[2][1] + a[2][3] * b[3][1];
            m[2][2] = a[2][0] * b[0][2] + a[2][1] * b[1][2] + a[2][2] * b[2][2] + a[2][3] * b[3][2];
            m[2][3] = a[2][0] * b[0][3] + a[2][1] * b[1][3] + a[2][2] * b[2][3] + a[2][3] * b[3][3];

            m[3][0] = a[3][0] * b[0][0] + a[3][1] * b[1][0] + a[3][2] * b[2][0] + a[3][3] * b[3][0];
            m[3][1] = a[3][0] * b[0][1] + a[3][1] * b[1][1] + a[3][2] * b[2][1] + a[3][3] * b[3][1];
            m[3][2] = a[3][0] * b[0][2] + a[3][1] * b[1][2] + a[3][2] * b[2][2] + a[3][3] * b[3][2];
            m[3][3] = a[3][0] * b[0][3] + a[3][1] * b[1][3] + a[3][2] * b[2][3] + a[3][3] * b[3][3];

            if (m == tmp)
                guMtx44Copy(tmp, ab);
            
            #endif
        }

        inline void multiply_vector(const Mtx44 mt, const guVector* src, guVector* dst){
            guVector tmp;

            tmp.x = mt[0][0] * src->x + mt[0][1] * src->y + mt[0][2] * src->z + mt[0][3];
            tmp.y = mt[1][0] * src->x + mt[1][1] * src->y + mt[1][2] * src->z + mt[1][3];
            tmp.z = mt[2][0] * src->x + mt[2][1] * src->y + mt[2][2] * src->z + mt[2][3];

            dst->x = tmp.x;
            dst->y = tmp.y;
            dst->z = tmp.z;
        }

        inline void multiply_vector(const Mtx44 mt, const vec4* src, vec4* dst) {
            vec4 tmp;

            tmp.x = mt[0][0] * src->x + mt[0][1] * src->y + mt[0][2] * src->z + mt[0][3] * src->w;
            tmp.y = mt[1][0] * src->x + mt[1][1] * src->y + mt[1][2] * src->z + mt[1][3] * src->w;
            tmp.z = mt[2][0] * src->x + mt[2][1] * src->y + mt[2][2] * src->z + mt[2][3] * src->w;
            tmp.w = mt[3][0] * src->x + mt[3][1] * src->y + mt[3][2] * src->z + mt[3][3] * src->w;

            dst->x = tmp.x;
            dst->y = tmp.y;
            dst->z = tmp.z;
            dst->w = tmp.w;
        }
    };
};