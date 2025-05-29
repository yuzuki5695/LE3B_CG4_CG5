#pragma once
#include<Vertex.h>
#include<Material.h>
#include<MaterialDate.h>
#include<ModelDate.h>

namespace PrimitiveGenerator
{
 
	// リングの頂点データを生成する関数
	std::vector<VertexData> DrawRing(VertexData* vertexData, uint32_t KRingDivide, float KOuterRadius, float KInnerRadius);

	// 球の頂点データを生成する関数
	std::vector<VertexData> DrawSphere(const uint32_t ksubdivision, VertexData* vertexdata);

    // 円柱の頂点データを生成する関数
    std::vector<VertexData> DrawCylinder(VertexData* vertexData, uint32_t kCylinderDivide, float kTopRadius, float kBottomRadius, float kHeight);

	// 星形の頂点データを生成する関数
	std::vector<VertexData> DrawStar(VertexData* vertexData, uint32_t kNumPoints, float kOuterRadius, float kInnerRadius);

	// スパイラル状の頂点データを生成する関数
	void DrawSpiral(uint32_t kSpiralDiv, float spiralRadius, float spiralHeight, uint32_t spiralTurn, VertexData* vertexData);
	
	// サークルの頂点データを作成する関数
	std::vector<VertexData> DrawCircle(VertexData* vertexData, uint32_t segmentCount, float radius);
	
	// 正方形の頂点データを作成する関数
	std::vector<VertexData> DrawBox(VertexData* vertexData);

	std::vector<VertexData> GenerateCircle(float cx, float cy, float cz, float radius, int segments);


};
