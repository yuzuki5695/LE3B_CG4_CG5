#pragma once

/// <summary>
/// 3次元ベクトル
/// </summary>
struct Vector3 final {
    float x, y, z;

    // 足し算
    Vector3 operator+(const Vector3& other) const {
        return { x + other.x, y + other.y, z + other.z };
    }

    // 引き算  
    Vector3 operator-(const Vector3& other) const {
        return { x - other.x, y - other.y, z - other.z };
    }

    // 代入付き足し算
    Vector3& operator+=(const Vector3& other) {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    // スカラー倍（左）
    friend Vector3 operator*(float scalar, const Vector3& vec) {
        return { scalar * vec.x, scalar * vec.y, scalar * vec.z };
    }

    // スカラー倍（右）
    Vector3 operator*(float scalar) const {
        return { x * scalar, y * scalar, z * scalar };
    }
};