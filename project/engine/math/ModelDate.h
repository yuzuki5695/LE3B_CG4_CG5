#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <Vertex.h>
#include <MaterialDate.h>

/// <summary>
/// モデルデータ
/// </summary>
struct ModelDate final {
	std::vector<VertexData> vertices;
	MaterialDate material;
};