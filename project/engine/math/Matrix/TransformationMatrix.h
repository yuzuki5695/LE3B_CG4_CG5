#pragma once
#include <Matrix4x4.h>

/// <summary>
/// 座標変換行列データ
/// </summary>
struct TransformationMatrix final {
	Matrix4x4 WVP;
	Matrix4x4 World;
	Matrix4x4 WorldInverseTranspose;
};