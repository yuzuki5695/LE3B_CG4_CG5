#include "Model.h"
#include "Object3d.h"
#include "MatrixVector.h"
#include "TextureManager.h"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <numbers>

using namespace MatrixVector;

void Model::Initialize(ModelCommon* modelCommon, const std::string& directorypath, const std::string& filename) {
    // NULL検出
    assert(modelCommon);
    // 引数で受け取ってメンバ変数に記録する
    this->modelCommon = modelCommon;

    // モデル読み込み
    modelDate = LoadObjFile(directorypath, filename);

    // 頂点データの作成
    VertexDatacreation();
    // マテリアルの生成、初期化
    MaterialGenerate();

    // .objの参照しているテクスチャ読み込み
    TextureManager::GetInstance()->LoadTexture(modelDate.material.textureFilePath);
    // 読み込んだテクスチャの番号を取得
    modelDate.material.textureindex = TextureManager::GetInstance()->GetSrvIndex(modelDate.material.textureFilePath);
}

void Model::Draw() {
    // VertexBufferViewの設定
    modelCommon->GetDxCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
    // マテリアルCBufferの場所を設定
    modelCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
    //SRVのDescriptortableの先頭を設定。２はrootParameter[2]である。
    //SRVを切り替えて画像を変えるS
    modelCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(modelDate.material.textureFilePath));

    // 描画！(今回は球)
   // modelCommon->GetDxCommon()->GetCommandList()->DrawInstanced(UINT(modelDate.vertices.size()), 1, 0, 0);


    // 描画！
    modelCommon->GetDxCommon()->GetCommandList()->DrawInstanced(vertexCount, 1, 0, 0);
}

void Model::VertexDatacreation() {

    //// 関数化したResouceで作成
    //vertexResoruce = modelCommon->GetDxCommon()->CreateBufferResource(sizeof(Model::VertexData) * modelDate.vertices.size());

    ////頂点バッファビューを作成する
    //// リソースの先頭のアドレスから使う
    //vertexBufferView.BufferLocation = vertexResoruce->GetGPUVirtualAddress();
    //// 使用するリソースのサイズはの頂点のサイズ
    //vertexBufferView.SizeInBytes = UINT(sizeof(Model::VertexData) * modelDate.vertices.size());
    //// 1頂点当たりのサイズ
    //vertexBufferView.StrideInBytes = sizeof(Model::VertexData);

    //// 頂点リソースにデータを書き込むためのアドレスを取得
    //vertexResoruce->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
    //// 頂点データをリソースにコピー
    //std::memcpy(vertexData, modelDate.vertices.data(), sizeof(Model::VertexData) * modelDate.vertices.size());


    //kSubdivision = 16;

    //vertexCount = kSubdivision * kSubdivision * 6; //球の頂点数

    //// 関数化したResouceで作成
    //vertexResoruce = modelCommon->GetDxCommon()->CreateBufferResource(sizeof(VertexData) * vertexCount);

    ////頂点バッファビューを作成する
    //// リソースの先頭のアドレスから使う
    //vertexBufferView.BufferLocation = vertexResoruce->GetGPUVirtualAddress();
    //// 使用するリソースのサイズはの頂点のサイズ
    //vertexBufferView.SizeInBytes = sizeof(VertexData) * vertexCount;
    //// 1頂点当たりのサイズ
    //vertexBufferView.StrideInBytes = sizeof(VertexData);

    //// 頂点リソースにデータを書き込むためのアドレスを取得
    //vertexResoruce->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

    //// 球の頂点にデータを入力
    //DrawSphere(kSubdivision, vertexData);


    const uint32_t KRingDivide = 32;
    const float KOuterRadius = 1.0f;
    const float KInnerRadius = 0.2f;
    const float radianPerDivide = 2.0f * std::numbers::pi_v<float> / float(KRingDivide);
    vertexCount = KRingDivide * 6;
    // 関数化したResouceで作成
    vertexResoruce = modelCommon->GetDxCommon()->CreateBufferResource(sizeof(VertexData) * vertexCount);
    //頂点バッファビューを作成する
    // リソースの先頭のアドレスから使う
    vertexBufferView.BufferLocation = vertexResoruce->GetGPUVirtualAddress();
    // 使用するリソースのサイズはの頂点のサイズ
    vertexBufferView.SizeInBytes = sizeof(VertexData) * vertexCount;
    // 1頂点当たりのサイズ
    vertexBufferView.StrideInBytes = sizeof(VertexData);

    // 頂点リソースにデータを書き込むためのアドレスを取得
    vertexResoruce->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
    // Ringの頂点データを入力
    DrawRing(vertexData, KRingDivide, KOuterRadius, KInnerRadius);

}

void Model::MaterialGenerate() {
    // マテリアル用のリソース
    materialResource = modelCommon->GetDxCommon()->CreateBufferResource(sizeof(Model::Material));
    // マテリアル用にデータを書き込むためのアドレスを取得
    materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
    // マテリアルデータの初期値を書き込む
    materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
    // SpriteはLightingしないでfalseを設定する
    materialData->endbleLighting = true;
    // 単位行列を書き込んでおく
    materialData->uvTransform = MakeIdentity4x4();
    // 光沢度を書き込む
    materialData->shinimess = 70;
}


Model::MaterialDate Model::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename) {
    // 1. 中で必要となる変数の宣言
    Model::MaterialDate materialDate; // 構築するMaterialDate
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

Model::ModelDate Model::LoadObjFile(const std::string& directoryPath, const std::string& filename) {
    // 1. 中で必要となる変数の宣言
    Model::ModelDate modelDate; // 構築するModelDate
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
            Model::VertexData triangle[3];
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

void Model::ChangeTexture(const std::string& newTexturePath) {
    // 新しいテクスチャファイルを読み込む
    modelDate.material.textureFilePath = newTexturePath; // 新しいテクスチャファイルのパスを設定
    TextureManager::GetInstance()->LoadTexture(newTexturePath); // 新しいテクスチャをロード
    modelDate.material.textureindex = TextureManager::GetInstance()->GetSrvIndex(newTexturePath); // 新しいテクスチャインデックスを取得
}

void Model::DrawSphere(const uint32_t ksubdivision, VertexData* vertexdata) {
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
}

void Model::DrawRing(VertexData* vertexData, uint32_t KRingDivide, float KOuterRadius, float KInnerRadius) {
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
}
