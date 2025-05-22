#include "PrimitiveGenerator.h"
#include <cmath>
#include <numbers>
#define _USE_MATH_DEFINES
#include <math.h>

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



}