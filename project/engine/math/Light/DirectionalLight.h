#pragma once
#include <Vector4.h>
#include <Vector3.h>

/// <summary>
/// 平行光源データ
/// </summary>
struct DirectionalLight final {
	Vector4 color; //!< ライトの色
	Vector3 direction; //!< ライトの向き
	float intensity; //!< 輝度
};