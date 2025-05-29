#include "PrimitiveGenerator.h"
#include <cmath>
#include <numbers>
#define _USE_MATH_DEFINES
#include <math.h>
#include <DirectXMath.h>
#include <MatrixVector.h>

using namespace MatrixVector;

namespace PrimitiveGenerator
{

    std::vector<VertexData> DrawRing(VertexData* vertexData, uint32_t KRingDivide, float KOuterRadius, float KInnerRadius) {
        const float radianPerDivide = 2.0f * std::numbers::pi_v<float> / float(KRingDivide);

        for (uint32_t i = 0; i < KRingDivide; ++i) {
            float angle = i * radianPerDivide;
            float nextAngle = (i + 1) * radianPerDivide;

            float sin = std::sin(angle);
            float cos = std::cos(angle);
            float sinNext = std::sin(nextAngle);
            float cosNext = std::cos(nextAngle);

            float u = float(i) / float(KRingDivide);
            float uNext = float(i + 1) / float(KRingDivide);

            uint32_t index = i * 6;

            // XY平面（Z = 0）にリングを構築
            vertexData[index + 0].position = { cos * KOuterRadius, sin * KOuterRadius, 0.0f, 1.0f };
            vertexData[index + 1].position = { cosNext * KOuterRadius, sinNext * KOuterRadius, 0.0f, 1.0f };
            vertexData[index + 2].position = { cos * KInnerRadius, sin * KInnerRadius, 0.0f, 1.0f };

            vertexData[index + 3].position = { cosNext * KOuterRadius, sinNext * KOuterRadius, 0.0f, 1.0f };
            vertexData[index + 4].position = { cosNext * KInnerRadius, sinNext * KInnerRadius, 0.0f, 1.0f };
            vertexData[index + 5].position = { cos * KInnerRadius, sin * KInnerRadius, 0.0f, 1.0f };

            // テクスチャ座標（仮の設定。必要なら調整）
            vertexData[index + 0].texcoord = { u, 0.0f };
            vertexData[index + 1].texcoord = { uNext, 0.0f };
            vertexData[index + 2].texcoord = { u, 1.0f };

            vertexData[index + 3].texcoord = { uNext, 0.0f };
            vertexData[index + 4].texcoord = { uNext, 1.0f };
            vertexData[index + 5].texcoord = { u, 1.0f };

            // 法線はZ+方向（XY平面の正面）
            for (int j = 0; j < 6; ++j) {
                vertexData[index + j].normal = { 0.0f, 0.0f, 1.0f };
            }
        }
        return std::vector<VertexData>(vertexData, vertexData + KRingDivide * 6);
    }

    std::vector<VertexData> DrawSphere(const uint32_t ksubdivision, VertexData* vertexdata) {
        // 球の頂点数を計算する
        //経度分割1つ分の角度 
        const float kLonEvery = (float)M_PI * 2.0f / float(ksubdivision);
        //緯度分割1つ分の角度 
        const float kLatEvery = (float)M_PI / float(ksubdivision);
        //経度の方向に分割
        for (uint32_t latIndex = 0; latIndex < ksubdivision; ++latIndex)
        {
            float lat = -(float)M_PI / 2.0f + kLatEvery * latIndex;	// θ
            //経度の方向に分割しながら線を描く
            for (uint32_t lonIndex = 0; lonIndex < ksubdivision; ++lonIndex)
            {
                float u = float(lonIndex) / float(ksubdivision);
                float v = 1.0f - float(latIndex) / float(ksubdivision);

                //頂点位置を計算する
                uint32_t start = (latIndex * ksubdivision + lonIndex) * 6;
                float lon = lonIndex * kLonEvery;	// Φ
                //頂点にデータを入力する。基準点 a
                vertexdata[start + 0].position = { cos(lat) * cos(lon) ,sin(lat) , cos(lat) * sin(lon) ,1.0f };
                vertexdata[start + 0].texcoord = { u,v };
                vertexdata[start + 0].normal.x = vertexdata[start + 0].position.x;
                vertexdata[start + 0].normal.y = vertexdata[start + 0].position.y;
                vertexdata[start + 0].normal.z = vertexdata[start + 0].position.z;

                //基準点 b
                vertexdata[start + 1].position = { cos(lat + kLatEvery) * cos(lon),sin(lat + kLatEvery),cos(lat + kLatEvery) * sin(lon) ,1.0f };
                vertexdata[start + 1].texcoord = { u ,v - 1.0f / float(ksubdivision) };
                vertexdata[start + 1].normal.x = vertexdata[start + 1].position.x;
                vertexdata[start + 1].normal.y = vertexdata[start + 1].position.y;
                vertexdata[start + 1].normal.z = vertexdata[start + 1].position.z;

                //基準点 c
                vertexdata[start + 2].position = { cos(lat) * cos(lon + kLonEvery),sin(lat), cos(lat) * sin(lon + kLonEvery) ,1.0f };
                vertexdata[start + 2].texcoord = { u + 1.0f / float(ksubdivision),v };
                vertexdata[start + 2].normal.x = vertexdata[start + 2].position.x;
                vertexdata[start + 2].normal.y = vertexdata[start + 2].position.y;
                vertexdata[start + 2].normal.z = vertexdata[start + 2].position.z;

                //基準点 d
                vertexdata[start + 3].position = { cos(lat + kLatEvery) * cos(lon + kLonEvery), sin(lat + kLatEvery) , cos(lat + kLatEvery) * sin(lon + kLonEvery) ,1.0f };
                vertexdata[start + 3].texcoord = { u + 1.0f / float(ksubdivision), v - 1.0f / float(ksubdivision) };
                vertexdata[start + 3].normal.x = vertexdata[start + 3].position.x;
                vertexdata[start + 3].normal.y = vertexdata[start + 3].position.y;
                vertexdata[start + 3].normal.z = vertexdata[start + 3].position.z;

                // 頂点4 (b, c, d)
                vertexdata[start + 4].position = { cos(lat) * cos(lon + kLonEvery),sin(lat),cos(lat) * sin(lon + kLonEvery),1.0f };
                vertexdata[start + 4].texcoord = { u + 1.0f / float(ksubdivision) ,v };
                vertexdata[start + 4].normal.x = vertexdata[start + 4].position.x;
                vertexdata[start + 4].normal.y = vertexdata[start + 4].position.y;
                vertexdata[start + 4].normal.z = vertexdata[start + 4].position.z;

                vertexdata[start + 5].position = { cos(lat + kLatEvery) * cos(lon),sin(lat + kLatEvery),cos(lat + kLatEvery) * sin(lon),1.0f };
                vertexdata[start + 5].texcoord = { u,v - 1.0f / float(ksubdivision) };
                vertexdata[start + 5].normal.x = vertexdata[start + 5].position.x;
                vertexdata[start + 5].normal.y = vertexdata[start + 5].position.y;
                vertexdata[start + 5].normal.z = vertexdata[start + 5].position.z;
            }
        }
        return std::vector<VertexData>(vertexdata, vertexdata + ksubdivision * 6);
    }

    std::vector<VertexData> PrimitiveGenerator::DrawCylinder(VertexData* vertexData, uint32_t kCylinderDivide, float kTopRadius, float kBottomRadius, float kHeight) {
        std::vector<VertexData> vertices;
        vertices.resize(kCylinderDivide * 6); // 1区画6頂点

        const float radianPerDivide = 2.0f * std::numbers::pi_v<float> / float(kCylinderDivide);

        for (uint32_t index = 0; index < kCylinderDivide; ++index) {
            float theta = index * radianPerDivide;
            float nextTheta = (index + 1) * radianPerDivide;

            float sin = std::sin(theta);
            float cos = std::cos(theta);
            float sinNext = std::sin(nextTheta);
            float cosNext = std::cos(nextTheta);

            float u = float(index) / float(kCylinderDivide);
            float uNext = float(index + 1) / float(kCylinderDivide);

            Vector4 top1 = { -sin * kTopRadius, kHeight, cos * kTopRadius, 1.0f };
            Vector4 top2 = { -sinNext * kTopRadius, kHeight, cosNext * kTopRadius, 1.0f };
            Vector4 bottom1 = { -sin * kBottomRadius, 0.0f, cos * kBottomRadius, 1.0f };
            Vector4 bottom2 = { -sinNext * kBottomRadius, 0.0f, cosNext * kBottomRadius, 1.0f };

            Vector3 normal1 = { -sin, 0.0f, cos };
            Vector3 normal2 = { -sinNext, 0.0f, cosNext };

            Vector2 uvTop = { u, 1.0f };
            Vector2 uvTopNext = { uNext, 1.0f };
            Vector2 uvBottom = { u, 0.0f };
            Vector2 uvBottomNext = { uNext, 0.0f };

            // 1区画6頂点（2三角形）
            vertices[index * 6 + 0] = { top1,     uvTop,       normal1 };
            vertices[index * 6 + 1] = { top2,     uvTopNext,   normal2 };
            vertices[index * 6 + 2] = { bottom1,  uvBottom,    normal1 };

            vertices[index * 6 + 3] = { bottom1,  uvBottom,    normal1 };
            vertices[index * 6 + 4] = { top2,     uvTopNext,   normal2 };
            vertices[index * 6 + 5] = { bottom2,  uvBottomNext,normal2 };
        }

        // vertexData にも書き込む（nullチェック付き）
        if (vertexData) {
            std::memcpy(vertexData, vertices.data(), sizeof(VertexData) * vertices.size());
        }

        return vertices;
    }


    std::vector<VertexData> DrawStar(VertexData* vertexData, uint32_t kNumPoints, float kOuterRadius, float kInnerRadius) {
        const float radianPerDivide = 2.0f * std::numbers::pi_v<float> / float(kNumPoints * 2);

        for (uint32_t i = 0; i < kNumPoints; ++i) {
            float angleOuter0 = (2 * i) * radianPerDivide;       // 外側頂点 i
            float angleInner0 = (2 * i + 1) * radianPerDivide;   // 内側頂点 i
            float angleOuter1 = (2 * i + 2) * radianPerDivide;   // 外側頂点 i+1

            float cosOuter0 = std::cos(angleOuter0);
            float sinOuter0 = std::sin(angleOuter0);
            float cosInner0 = std::cos(angleInner0);
            float sinInner0 = std::sin(angleInner0);
            float cosOuter1 = std::cos(angleOuter1);
            float sinOuter1 = std::sin(angleOuter1);

            uint32_t index = i * 6;

            // 三角形1
            vertexData[index + 0].position = { cosOuter0 * kOuterRadius, sinOuter0 * kOuterRadius, 0.0f, 1.0f };
            vertexData[index + 1].position = { cosInner0 * kInnerRadius, sinInner0 * kInnerRadius, 0.0f, 1.0f };
            vertexData[index + 2].position = { cosOuter1 * kOuterRadius, sinOuter1 * kOuterRadius, 0.0f, 1.0f };

            // 三角形2
            vertexData[index + 3].position = { cosOuter1 * kOuterRadius, sinOuter1 * kOuterRadius, 0.0f, 1.0f };
            vertexData[index + 4].position = { cosInner0 * kInnerRadius, sinInner0 * kInnerRadius, 0.0f, 1.0f };
            vertexData[index + 5].position = { std::cos(angleOuter1 + radianPerDivide) * kInnerRadius, std::sin(angleOuter1 + radianPerDivide) * kInnerRadius, 0.0f, 1.0f };

            // テクスチャ座標（単純に0～1で割り当て例）
            float u0 = (cosOuter0 * 0.5f) + 0.5f;
            float v0 = (sinOuter0 * 0.5f) + 0.5f;
            float u1 = (cosInner0 * 0.5f) + 0.5f;
            float v1 = (sinInner0 * 0.5f) + 0.5f;
            float u2 = (cosOuter1 * 0.5f) + 0.5f;
            float v2 = (sinOuter1 * 0.5f) + 0.5f;
            float u3 = (std::cos(angleOuter1 + radianPerDivide) * 0.5f) + 0.5f;
            float v3 = (std::sin(angleOuter1 + radianPerDivide) * 0.5f) + 0.5f;

            vertexData[index + 0].texcoord = { u0, v0 };
            vertexData[index + 1].texcoord = { u1, v1 };
            vertexData[index + 2].texcoord = { u2, v2 };
            vertexData[index + 3].texcoord = { u2, v2 };
            vertexData[index + 4].texcoord = { u1, v1 };
            vertexData[index + 5].texcoord = { u3, v3 };

            // 法線はZ+方向（XY平面の正面）
            for (int j = 0; j < 6; ++j) {
                vertexData[index + j].normal = { 0.0f, 0.0f, 1.0f };
            }
        }

        return std::vector<VertexData>(vertexData, vertexData + kNumPoints * 6);
    }

    void DrawSpiral(uint32_t kSpiralDiv, float spiralRadius, float spiralHeight, uint32_t spiralTurn, VertexData* vertexData) {
        for (uint32_t i = 0; i <= kSpiralDiv; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(kSpiralDiv);
            float angle = t * spiralTurn * 2.0f * std::numbers::pi_v<float>;
            float height = t * spiralHeight;

            float x = std::cos(angle) * spiralRadius;
            float y = height;
            float z = std::sin(angle) * spiralRadius;

            vertexData[i].position = { x, y, z, 1.0f };
            vertexData[i].texcoord = { t, 0.0f }; // 仮で流す
            vertexData[i].normal = { 0.0f, 1.0f, 0.0f }; // 仮の法線
        }
    }
    std::vector<VertexData> DrawCircle(VertexData* vertexData, uint32_t segmentCount, float radius) {
        std::vector<VertexData> vertices;
        vertices.reserve(segmentCount + 1); // 最後に始点と結ぶために +1

        for (uint32_t i = 0; i <= segmentCount; ++i) {
            float angle = static_cast<float>(i) / segmentCount * 2.0f * float(M_PI);
            float x = std::cos(angle) * radius;
            float z = std::sin(angle) * radius;

            VertexData v{};
            v.position = { x, 0.0f, z, 1.0f };  // w=1.0 の位置ベクトル
            v.texcoord = { static_cast<float>(i) / segmentCount, 0.0f };  // 任意で設定
            v.normal = { 0.0f, 1.0f, 0.0f };  // 上方向を向いている想定

            vertices.push_back(v);
        }

        // GPU用のバッファにコピー
        std::memcpy(vertexData, vertices.data(), sizeof(VertexData) * vertices.size());

        return vertices;
    }

    std::vector<VertexData> DrawBox(VertexData* vertexData) {
        std::vector<VertexData> vertices(36);

        const float size = 0.5f;

        Vector3 p[8] = {
            {-size, -size, -size},
            { size, -size, -size},
            { size,  size, -size},
            {-size,  size, -size},
            {-size, -size,  size},
            { size, -size,  size},
            { size,  size,  size},
            {-size,  size,  size},
        };

        uint32_t indices[] = {
            4, 5, 6, 4, 6, 7,
            1, 0, 3, 1, 3, 2,
            0, 4, 7, 0, 7, 3,
            5, 1, 2, 5, 2, 6,
            3, 7, 6, 3, 6, 2,
            0, 1, 5, 0, 5, 4,
        };

        Vector3 faceNormals[] = {
            {0, 0, 1},
            {0, 0, -1},
            {-1, 0, 0},
            {1, 0, 0},
            {0, 1, 0},
            {0, -1, 0},
        };

        for (int face = 0; face < 6; ++face) {
            for (int i = 0; i < 6; ++i) {
                int vertexIndex = face * 6 + i;
                Vector3 pos = p[indices[vertexIndex]];

                vertices[vertexIndex].position.x = pos.x;
                vertices[vertexIndex].position.y = pos.y;
                vertices[vertexIndex].position.z = pos.z;
                vertices[vertexIndex].position.w = 1.0f;

                vertices[vertexIndex].normal = faceNormals[face];

                // ここでUVを割り当てたい場合は適宜調整
                vertices[vertexIndex].texcoord = { 0.0f, 0.0f };
            }
        }

        if (vertexData) {
            std::memcpy(vertexData, vertices.data(), sizeof(VertexData) * vertices.size());
        }

        return vertices;
    }

    std::vector<VertexData> GenerateCircle(float cx, float cy, float cz, float radius, int segments) {
        std::vector<VertexData> vertices;
        for (int i = 0; i < segments; ++i) {
            float theta0 = 2.0f * 3.1415926f * i / segments;
            float theta1 = 2.0f * 3.1415926f * (i + 1) / segments;

            Vector4 center = { cx, cy, cz, 1.0f };
            Vector4 p0 = { cx + cosf(theta0) * radius, cy + sinf(theta0) * radius, cz, 1.0f };
            Vector4 p1 = { cx + cosf(theta1) * radius, cy + sinf(theta1) * radius, cz, 1.0f };

            VertexData v0{ center, {0.5f, 0.5f}, {0.0f, 0.0f, 1.0f} };
            VertexData v1{ p0, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f} };
            VertexData v2{ p1, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f} };

            vertices.push_back(v0);
            vertices.push_back(v1);
            vertices.push_back(v2);
        }
        return vertices;
    }


}