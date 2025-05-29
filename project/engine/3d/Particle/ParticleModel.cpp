#include "ParticleModel.h"
#include <MatrixVector.h>
#include <cassert>
#include <ModelManager.h>
#include <TextureManager.h>
#include <numbers>
#include <externals/DirectXTex/d3dx12.h>
#include <Object3dCommon.h>

using namespace MatrixVector;
using namespace Microsoft::WRL;
using namespace PrimitiveGenerator;

void ParticleModel::Initialize(DirectXCommon* birectxcommon, const std::string& filename) {
    // NULL検出
    assert(birectxcommon);
    // メンバ変数に記録
    this->dxCommon_ = birectxcommon;
    // マテリアルの生成と初期化
    MaterialGenerate();
	// 頂点データの作成
	if (vertexType_ == VertexType::Model) { // モデルの頂点データを作成
        modelDate = LoadObjFile("Resources", filename);
        VertexDataModel();  // 頂点データコピー
        //.objの参照しているテクスチャ読み込み
        TextureManager::GetInstance()->LoadTexture(modelDate.material.textureFilePath);
        // 読み込んだテクスチャの番号を取得
        modelDate.material.textureindex = TextureManager::GetInstance()->GetSrvIndex(modelDate.material.textureFilePath);
	} else if (vertexType_ == VertexType::Ring) { // リングの頂点データを作成
        VertexDataRing();
	} else if (vertexType_ == VertexType::Sphere) { // 球の頂点データを作成
        VertexDataSphere();
	} else if (vertexType_ == VertexType::Cylinder) { // 円柱の頂点データを作成
        VertexDataCylinder();
	} else if (vertexType_ == VertexType::Star) { // 星の頂点データを作成
        VertexDataStar();
    } else if (vertexType_ == VertexType::Spiral) { // スパイラル状の頂点データを作成
        VertexDataSpiral();
    } else if (vertexType_ == VertexType::Circle) { // サークルの頂点データを作成
        VertexDataCircle();
    } else if (vertexType_ == VertexType::Box) { // 正方形の頂点データを作成
        VertexDataBox();
    } else if (vertexType_ == VertexType::Cloud) { // 正方形の頂点データを作成
        VertexDataCloud();
    }
}

void ParticleModel::Draw() {
    // VertexBufferView を設定
    dxCommon_->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
    // マテリアル用の定数バッファを設定
    dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
}

void ParticleModel::CreateVertexBuffer() {
    // 関数化したResouceで作成
    vertexResoruce = dxCommon_->CreateBufferResource(sizeof(VertexData) * modelDate.vertices.size());
    //頂点バッファビューを作成する
    // リソースの先頭のアドレスから使う
    vertexBufferView.BufferLocation = vertexResoruce->GetGPUVirtualAddress();
    // 使用するリソースのサイズはの頂点のサイズ
    vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelDate.vertices.size());
    // 1頂点当たりのサイズ
    vertexBufferView.StrideInBytes = sizeof(VertexData);
    // 頂点リソースにデータを書き込むためのアドレスを取得
    vertexResoruce->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
}

void ParticleModel::VertexDataModel() {
    // 共通の頂点バッファビュー処理
    CreateVertexBuffer();
    std::memcpy(vertexData, modelDate.vertices.data(), sizeof(VertexData) * modelDate.vertices.size());
}

void ParticleModel::VertexDataRing() {
    const uint32_t kRingDivide = 32;
    const float kOuterRadius = 1.0f;
    const float kInnerRadius = 0.2f;
    vertexCount = kRingDivide * 6;
    // 頂点数を計算
    modelDate.vertices.resize(vertexCount);
    // 共通の頂点バッファビュー処理
    CreateVertexBuffer();
    // 頂点データ生成
    modelDate.vertices = DrawRing(vertexData, kRingDivide, kOuterRadius, kInnerRadius);
}

void ParticleModel::VertexDataSphere() {
    const uint32_t kSubdivision = 16;
    vertexCount = kSubdivision * kSubdivision * 6;
    // 頂点数を計算
    modelDate.vertices.resize(vertexCount);
    // 共通の頂点バッファビュー処理
    CreateVertexBuffer();
    // 頂点データ生成
    modelDate.vertices = DrawSphere(kSubdivision, vertexData);
}

void ParticleModel::VertexDataCylinder() {
    const uint32_t kCylinderDivide = 32;
    const float kTopRadius = 1.0f;
    const float kBottomRadius = 1.0f;
    const float kHeight = 3.0f;
    const float radianPerDivide = 2.0f * std::numbers::pi_v<float> / float(kCylinderDivide);
    vertexCount = kCylinderDivide * 6;
    // 頂点数を計算
    modelDate.vertices.resize(vertexCount);
    // 共通の頂点バッファビュー処理
    CreateVertexBuffer();
    // 頂点データ生成
    modelDate.vertices = DrawCylinder(vertexData, kCylinderDivide, kTopRadius, kBottomRadius, kHeight);
}

void ParticleModel::VertexDataStar() {
    const uint32_t kNumPoints = 5;  // 星の先端数
    const float kOuterRadius = 1.0f;
    const float kInnerRadius = 0.5f;
    vertexCount = kNumPoints * 6;
    // 頂点数を計算
    modelDate.vertices.resize(vertexCount);
    // 共通の頂点バッファビュー処理
    CreateVertexBuffer();
    // 頂点データ生成
    modelDate.vertices = DrawStar(vertexData, kNumPoints, kOuterRadius, kInnerRadius);
}

void ParticleModel::VertexDataSpiral() {
    uint32_t kSpiralDiv = 100;
    vertexCount = kSpiralDiv + 1;
    // 頂点数を計算
    modelDate.vertices.resize(vertexCount);
    // 共通の頂点バッファビュー処理
    CreateVertexBuffer();
    // 頂点データ生成
    DrawSpiral(kSpiralDiv, 5.0f, 10.0f, 3, vertexData);
}

void ParticleModel::MaterialGenerate() {
    // マテリアル用のリソース
    materialResource = dxCommon_->CreateBufferResource(sizeof(Material));
    // マテリアル用にデータを書き込むためのアドレスを取得
    materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
    // マテリアルデータの初期値を書き込む
    materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
    materialData->endbleLighting = true;
    materialData->uvTransform = MakeIdentity4x4();
}

void ParticleModel::VertexDataCircle() {
    const uint32_t kSegmentCount = 64;
    const float kRadius = 1.0f;
    // 頂点数は線で円を構成するので segmentCount+1（ループ閉じ）
    vertexCount = (kSegmentCount + 1);
    modelDate.vertices.resize(vertexCount);
    // 頂点バッファ作成
    CreateVertexBuffer();
    modelDate.vertices = DrawCircle(vertexData, kSegmentCount, kRadius);
}

MaterialDate ParticleModel::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename) {
    // 1. 中で必要となる変数の宣言
    MaterialDate materialDate; // 構築するMaterialDate
    std::string line; // ファイルから読んだ1行を格納するもの
    std::ifstream file(directoryPath + "/" + filename); // 2.ファイルを開く
    assert(file.is_open()); // とりあえず開けなかったら止める
    // 3. 実際にファイルを読み、MaterialDateを構築していく
    while (std::getline(file, line)) {
        std::string identifier;
        std::istringstream s(line);
        s >> identifier;

        // identifierの応じた処理
        if (identifier == "map_Kd") {
            std::string textureFilename;
            s >> textureFilename;
            // 連結してファイルパスにする
            materialDate.textureFilePath = directoryPath + "/" + textureFilename;
        }
    }
    return materialDate;
}

void ParticleModel::VertexDataCloud() {
    const int kCloudParts = 10; // 雲の構成要素（円）の数
    const float kRadius = 1.0f; // 各円の半径
    const float kSpread = 1.5f; // 雲の広がり

    std::vector<VertexData> cloudVertices;

    for (int i = 0; i < kCloudParts; ++i) {
        // ランダムな位置に円を配置
        float offsetX = ((rand() % 100) / 100.0f - 0.5f) * kSpread * 2.0f;
        float offsetY = ((rand() % 100) / 100.0f - 0.5f) * kSpread * 2.0f;

        // 各円の頂点を生成
        std::vector<VertexData> part = GenerateCircle(offsetX, offsetY, 0.0f, kRadius, 32);
        cloudVertices.insert(cloudVertices.end(), part.begin(), part.end());
    }

    vertexCount = static_cast<int>(cloudVertices.size());
    modelDate.vertices = cloudVertices;

    CreateVertexBuffer();
    std::memcpy(vertexData, modelDate.vertices.data(), sizeof(VertexData) * vertexCount);
}


void ParticleModel::VertexDataBox() {
    vertexCount = 36; // 立方体は常に36頂点
    modelDate.vertices.resize(vertexCount);
    CreateVertexBuffer();
    modelDate.vertices = DrawBox(vertexData);
}

ModelDate ParticleModel::LoadObjFile(const std::string& directoryPath, const std::string& filename) {
    // 1. 中で必要となる変数の宣言
    ModelDate modelDate; // 構築するModelDate
    std::vector<Vector4> positions; // 位置
    std::vector<Vector3> normals; // 法線
    std::vector<Vector2> texcoords; // テクスチャ座標
    std::string line; // ファイルから読んだ1桁を格納するもの
    // 2.  ファイルを開く
    std::ifstream file(directoryPath + "/" + filename); // ファイルを開く
    assert(file.is_open()); // とりあえず開けなかったら止める

    // 3. 実際にファイルを読み、ModelDateを構築していく
    while (std::getline(file, line)) {
        std::string identifier;
        std::istringstream s(line);
        s >> identifier;// 先頭の識別子を読む

        // identifierの応じた処理
        if (identifier == "v") {
            Vector4 position;
            s >> position.x >> position.y >> position.z;
            position.x *= -1.0f;// 位置のx成分を反転
            position.w = 1.0f;
            positions.push_back(position);
        } else if (identifier == "vt") {
            Vector2 texcoord;
            s >> texcoord.x >> texcoord.y;
            texcoord.y = 1.0f - texcoord.y;
            texcoords.push_back(texcoord);
        } else if (identifier == "vn") {
            Vector3 normal;
            s >> normal.x >> normal.y >> normal.z;
            normal.x *= -1.0f;// 法線のx成分を反転
            normals.push_back(normal);
        } else if (identifier == "f") {
            VertexData triangle[3];
            // 面は三角形限定。その他は未対応
            for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
                std::string vertexDefinition;
                s >> vertexDefinition;
                // 頂点の要素へのIndexは、[位置/UV/法線]で格納されているので、分解してIndexを取得する
                std::istringstream v(vertexDefinition);
                uint32_t elementIndices[3];
                for (uint32_t element = 0; element < 3; ++element) {
                    std::string index;
                    std::getline(v, index, '/');// /区切りでインデックスを読んでいく
                    elementIndices[element] = std::stoi(index);
                }
                // 要素のIndexから、実際の要素の値を取得して、頂点を構築する
                Vector4 position = positions[elementIndices[0] - 1];
                Vector2 texcoord = texcoords[elementIndices[1] - 1];
                Vector3 normal = normals[elementIndices[2] - 1];
                //VertexData vertex = { position,texcoord,normal };
                //modelDate.vertices.push_back(vertex);
                triangle[faceVertex] = { position,texcoord,normal };
            }
            // 頂点を逆順で登録することで、回り順を逆にする
            modelDate.vertices.push_back(triangle[2]);
            modelDate.vertices.push_back(triangle[1]);
            modelDate.vertices.push_back(triangle[0]);
        } else if (identifier == "mtllib") {
            // materialTemplateLibrarvファイルの名前を取得する
            std::string materialFilename;
            s >> materialFilename;
            // 基本的にobjファイルと同一階層にmtlは存在させるので、ディレクトリ名とファイル名を渡す
            modelDate.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
        }
    }
    // 4. ModelDateを返す
    return modelDate;
}