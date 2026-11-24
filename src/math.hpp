#pragma once

#include <cmath>

struct Vec3 {
    float x = 0, y = 0, z = 0;

    Vec3 operator+(const Vec3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    Vec3 operator*(float s) const { return {x * s, y * s, z * s}; }

    float dot(const Vec3& o) const { return x * o.x + y * o.y + z * o.z; }
    Vec3 cross(const Vec3& o) const {
        return {y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x};
    }
    float length() const { return std::sqrt(dot(*this)); }
    Vec3 normalized() const {
        float len = length();
        return len > 0 ? *this * (1.0f / len) : *this;
    }
};

struct Vec4 {
    float x = 0, y = 0, z = 0, w = 0;
};

// Row-major 4x4 matrix. Vectors are treated as columns: v' = M * v.
struct Mat4 {
    float m[4][4] = {};

    static Mat4 identity() {
        Mat4 r;
        for (int i = 0; i < 4; ++i) r.m[i][i] = 1;
        return r;
    }

    static Mat4 translate(float x, float y, float z) {
        Mat4 r = identity();
        r.m[0][3] = x;
        r.m[1][3] = y;
        r.m[2][3] = z;
        return r;
    }

    static Mat4 scale(float s) {
        Mat4 r = identity();
        r.m[0][0] = r.m[1][1] = r.m[2][2] = s;
        return r;
    }

    static Mat4 rotateX(float a) {
        Mat4 r = identity();
        float c = std::cos(a), s = std::sin(a);
        r.m[1][1] = c;  r.m[1][2] = -s;
        r.m[2][1] = s;  r.m[2][2] = c;
        return r;
    }

    static Mat4 rotateY(float a) {
        Mat4 r = identity();
        float c = std::cos(a), s = std::sin(a);
        r.m[0][0] = c;  r.m[0][2] = s;
        r.m[2][0] = -s; r.m[2][2] = c;
        return r;
    }

    static Mat4 rotateZ(float a) {
        Mat4 r = identity();
        float c = std::cos(a), s = std::sin(a);
        r.m[0][0] = c;  r.m[0][1] = -s;
        r.m[1][0] = s;  r.m[1][1] = c;
        return r;
    }

    // Right-handed perspective projection looking down -Z.
    // Maps the view frustum to clip space; NDC z ends up in [-1, 1].
    static Mat4 perspective(float fovY, float aspect, float near, float far) {
        Mat4 r;
        float f = 1.0f / std::tan(fovY / 2);
        r.m[0][0] = f / aspect;
        r.m[1][1] = f;
        r.m[2][2] = (far + near) / (near - far);
        r.m[2][3] = (2 * far * near) / (near - far);
        r.m[3][2] = -1;
        return r;
    }

    Mat4 operator*(const Mat4& o) const {
        Mat4 r;
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                for (int k = 0; k < 4; ++k)
                    r.m[i][j] += m[i][k] * o.m[k][j];
        return r;
    }

    Vec4 operator*(const Vec4& v) const {
        return {
            m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3] * v.w,
            m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3] * v.w,
            m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3] * v.w,
            m[3][0] * v.x + m[3][1] * v.y + m[3][2] * v.z + m[3][3] * v.w,
        };
    }
};
