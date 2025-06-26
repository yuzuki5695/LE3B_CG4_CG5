#pragma once
#include <Vector4.h>
#include <Matrix4x4.h>
#include <cstdint>

/// <summary>
/// マテリアル
/// </summary>
struct Material final {
	Vector4 color;
	int32_t endbleLighting;
	float padding[3];
	Matrix4x4 uvTransform;
	float shinimess;
};