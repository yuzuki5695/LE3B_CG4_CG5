#pragma once
#include <Vector4.h>
#include <Vector3.h>

/// <summary>
/// ポイントライト
/// </summary>
struct SpotLight final {
	Vector4 color; //!< ライトの色
	Vector3 position; //!< ライトの位置
	float intensity; //!< 輝度
	Vector3 direction; //!< スポットライトの向き
	float distance; //!< ライトの届く最大距離
	float decay; //!< 減衰率
	float cosAngle;  //!< スポットライトの余弦
	float cosFalloffStart;
	float padding[2];
};