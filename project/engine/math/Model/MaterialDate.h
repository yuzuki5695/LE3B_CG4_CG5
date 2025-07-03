#pragma once
#include <string>
#include <cstdint>

/// <summary>
/// 2次元マテリアルデータ
/// </summary>
struct MaterialDate final  {
	std::string textureFilePath;
	uint32_t textureindex = 0;
};