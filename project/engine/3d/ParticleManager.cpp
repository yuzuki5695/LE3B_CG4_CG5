#include "ParticleManager.h"
#include <MatrixVector.h>
#include <cassert>
#include <ModelManager.h>
#include <TextureManager.h>
#include <numbers>
#include <externals/DirectXTex/d3dx12.h>
#include <Object3dCommon.h>
#ifdef USE_IMGUI
#include<ImGuiManager.h>
#endif // USE_IMGUI
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace MatrixVector;
using namespace Microsoft::WRL;

// 静的メンバ変数の定義
std::unique_ptr<ParticleManager> ParticleManager::instance = nullptr;

// シングルトンインスタンスの取得
ParticleManager* ParticleManager::GetInstance() {
    if (!instance) {
        instance = std::make_unique<ParticleManager>();
    }
    return instance.get();
}

// 終了
void ParticleManager::Finalize() {
    instance.reset();
}

void ParticleManager::Initialize(DirectXCommon* birectxcommon, SrvManager* srvmanager) {
    // NULL検出
    assert(birectxcommon);
    assert(srvmanager);
    // メンバ変数に記録
    this->dxCommon_ = birectxcommon;
    this->srvmanager_ = srvmanager;
    // 乱数エンジンを初期化
    std::random_device rd;// 乱数生成器
    randomEngine = std::mt19937(rd());
    // マテリアルの生成と初期化
    MaterialGenerate();
    //ビルボード行列作成
    backToFrontMatrix = MakeRotateYMatrix(std::numbers::pi_v<float>);
}

void ParticleManager::Update() {
    Matrix4x4 billboardMatrix;
    Matrix4x4 viewMatrix;
    Matrix4x4 projectionMatrix;
    // ビルボード行列: パーティクルがカメラに向くように変換
    if (camera_) {
        // カメラのワールド行列を取得し、ビルボード行列を計算
        billboardMatrix = Multiply(backToFrontMatrix, camera_->GetWorldMatrix());  // 修正: GetWorldMatrix
        // カメラからビュー行列とプロジェクション行列を取得
        viewMatrix = camera_->GetViewMatrix();
        projectionMatrix = camera_->GetProjectionMatrix();
    } else {
        // カメラがない場合の処理（必要であれば）
    }

    // パーティクルの位置をカメラの方向に合わせるために設定
    billboardMatrix.m[3][0] = 0.0f;
    billboardMatrix.m[3][1] = 0.0f;
    billboardMatrix.m[3][2] = 0.0f;

    // 各パーティクルグループの処理
    for (auto& [name, group] : particleGroups) {
        uint32_t counter = 0;
        for (auto particleIterator = group.particles.begin(); particleIterator != group.particles.end();) {
            // パーティクルの現在の時間を増加させる
            (*particleIterator).currentTime += 1.0f / 60.0f;  // 60fpsで時間をカウントアップ

            // パーティクルの寿命が尽きたら削除
            if ((*particleIterator).currentTime >= (*particleIterator).lifetime) {
                particleIterator = group.particles.erase(particleIterator);  // パーティクル削除
                continue;
            }

            // 透明度の更新（時間に基づいてフェード）
            float alpha = 1.0f - (*particleIterator).currentTime / (*particleIterator).lifetime;
            (*particleIterator).color.w = alpha;


            //particleIterator->transform.translate.x += particleIterator->Velocity.x;
            //particleIterator->transform.translate.y += particleIterator->Velocity.y;
            //particleIterator->transform.translate.z += particleIterator->Velocity.z;

            // world行列の計算
            Matrix4x4 scaleMatrix = MakeScaleMatrix((*particleIterator).transform.scale);
            // 回転行列を各軸ごとに作成して合成
            Matrix4x4 rotateXMatrix = MakeRotateXMatrix((*particleIterator).transform.rotate.x);
            Matrix4x4 rotateYMatrix = MakeRotateYMatrix((*particleIterator).transform.rotate.y);
            Matrix4x4 rotateZMatrix = MakeRotateZMatrix((*particleIterator).transform.rotate.z);
            // 回転順序: Z → X → Y（用途により調整）
            Matrix4x4 rotateMatrix = Multiply(Multiply(rotateZMatrix, rotateXMatrix), rotateYMatrix);
            Matrix4x4 translateMatrix = MakeTranslateMatrix((*particleIterator).transform.translate);
            // SRT順にビルボードを含めて合成
            Matrix4x4 worldMatrix = Multiply(Multiply(Multiply(scaleMatrix, rotateMatrix), billboardMatrix), translateMatrix);

            // worldViewProjection行列の計算
            Matrix4x4 viewProjectionMatrix = Multiply(viewMatrix, projectionMatrix);
            Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, viewProjectionMatrix);

            // インスタンスデータに更新した行列を設定
            if (counter < group.kNumInstance) {
                group.instanceData[counter].WVP = worldViewProjectionMatrix;
                group.instanceData[counter].World = worldMatrix;
                group.instanceData[counter].color = (*particleIterator).color;
                ++counter;
            }

            // 次のパーティクルに進む
            ++particleIterator;
        }

        // 描画で使用するインスタンス数を更新
        group.kNumInstance = counter; 
    }
}

void ParticleManager::Draw() {
    // VertexBufferView を設定
    dxCommon_->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
    // マテリアル用の定数バッファを設定
    dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());

    // パーティクルグループごとに描画処理を行う
    for (const auto& [name, particleGroup] : particleGroups) {
        // インスタンス数が0の場合は描画しない
        if (particleGroup.kNumInstance == 0) {
            continue;
        }
        // インスタンシングデータの SRV を設定（テクスチャファイルのパスを指定）
        srvmanager_->SetGraphicsRootDescriptorTable(1, particleGroup.srvindex);
        // SRVで画像を表示
        srvmanager_->SetGraphicsRootDescriptorTable(2, particleGroup.materialData.textureindex);
        // 描画（インスタンシング）を実行
        dxCommon_->GetCommandList()->DrawInstanced(static_cast<UINT>(modelDate.vertices.size()), static_cast<UINT>(particleGroup.kNumInstance), 0, 0);
    }
}

void ParticleManager::SetParticleModel(const std::string& directorypath, const std::string& filename, VertexType type) {
    // モデルデータを取得
    modelDate = LoadObjFile(directorypath, filename);
    // 頂点データを作成
    VertexDatacreation(type);
    // .objの参照しているテクスチャ読み込み
    TextureManager::GetInstance()->LoadTexture(modelDate.material.textureFilePath);
    // 読み込んだテクスチャの番号を取得
    modelDate.material.textureindex = TextureManager::GetInstance()->GetSrvIndex(modelDate.material.textureFilePath);
}

void ParticleManager::VertexDatacreation(VertexType type) {
    switch (type) {
    case VertexType::OBJ: {
        size_t size = sizeof(VertexData) * modelDate.vertices.size();
        vertexResoruce = dxCommon_->CreateBufferResource(size);
        vertexBufferView.BufferLocation = vertexResoruce->GetGPUVirtualAddress();
        vertexBufferView.SizeInBytes = UINT(size);
        vertexBufferView.StrideInBytes = sizeof(VertexData);
        vertexResoruce->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
        std::memcpy(vertexData, modelDate.vertices.data(), size);
        break;
    }

    case VertexType::Sphere: {
        const uint32_t kSubdivision = 16;
        vertexCount = kSubdivision * kSubdivision * 6;
        size_t size = sizeof(VertexData) * vertexCount;
        vertexResoruce = dxCommon_->CreateBufferResource(size);
        vertexBufferView.BufferLocation = vertexResoruce->GetGPUVirtualAddress();
        vertexBufferView.SizeInBytes = size;
        vertexBufferView.StrideInBytes = sizeof(VertexData);
        vertexResoruce->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
        DrawSphere(kSubdivision, vertexData); // 自作関数
        break;
    }

    case VertexType::Ring: {
        const uint32_t KRingDivide = 32;
        const float KOuterRadius = 1.0f;
        const float KInnerRadius = 0.2f;
        vertexCount = KRingDivide * 6;
        size_t size = sizeof(VertexData) * vertexCount;
        vertexResoruce = dxCommon_->CreateBufferResource(size);
        vertexBufferView.BufferLocation = vertexResoruce->GetGPUVirtualAddress();
        vertexBufferView.SizeInBytes = size;
        vertexBufferView.StrideInBytes = sizeof(VertexData);
        vertexResoruce->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
        DrawRing(vertexData, KRingDivide, KOuterRadius, KInnerRadius); // 自作関数
        break;
    }

    default:
        assert(false && "不正な頂点タイプ");
        break;
    }
}


void ParticleManager::MaterialGenerate() {
    // マテリアル用のリソース
    materialResource = dxCommon_->CreateBufferResource(sizeof(Model::Material));
    // マテリアル用にデータを書き込むためのアドレスを取得
    materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
    // マテリアルデータの初期値を書き込む
    materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
    materialData->endbleLighting = true;
    materialData->uvTransform = MakeIdentity4x4();
}


ParticleManager::MaterialDate ParticleManager::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename) {
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

ParticleManager::ModelDate ParticleManager::LoadObjFile(const std::string& directoryPath, const std::string& filename) {
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

void ParticleManager::CreateParticleGroup(const std::string& name, const std::string& textureFilepath) {
    // すでにテクスチャがロードされているか確認
    if (!TextureManager::GetInstance()->IsTextureLoaded(textureFilepath)) {
        // マテリアルのテクスチャファイルをロード
        TextureManager::GetInstance()->LoadTexture(textureFilepath);
    }

    // パーティクルグループ名が既に存在するかチェック
    auto it = particleGroups.find(name);
    if (it != particleGroups.end()) {
        // 名前が一致するグループが存在する場合、そのグループのテクスチャが一致するか確認
        if (it->second.materialData.textureFilePath == textureFilepath) {
            // テクスチャが一致する場合、既存のグループを再利用
            return;
        } else {
            // テクスチャが異なる場合、既存のグループを更新
            it->second.materialData.textureFilePath = textureFilepath;
            it->second.materialData.textureindex = TextureManager::GetInstance()->GetSrvIndex(textureFilepath);
            // 必要に応じてリソースの再割り当てなどを行うことができます
        }
    } else {
        // 新しいパーティクルグループを作成
        ParticleGroup& newGroup = particleGroups[name];

        // 新しいパーティクルグループにテクスチャパスとインデックスを設定
        newGroup.materialData.textureFilePath = textureFilepath;
        newGroup.materialData.textureindex = TextureManager::GetInstance()->GetSrvIndex(textureFilepath);
        newGroup.kNumInstance = 0;

        // インスタンス用のリソースバッファを作成
        newGroup.Resource = dxCommon_->CreateBufferResource(sizeof(InstanceData) * MaxInstanceCount);
        newGroup.instanceData = nullptr;
        newGroup.Resource->Map(0, nullptr, reinterpret_cast<void**>(&newGroup.instanceData));

        // インスタンスデータを初期化
        for (uint32_t index = 0; index < MaxInstanceCount; ++index) {
            newGroup.instanceData[index].WVP = MakeIdentity4x4();
            newGroup.instanceData[index].World = MakeIdentity4x4();
            newGroup.instanceData[index].color = { 1.0f, 1.0f, 1.0f, 0.0f }; // 透明
        }
        // 書き込み後にリソースをアンマップ
        newGroup.Resource->Unmap(0, nullptr);
        // インスタンスバッファ用のSRVを割り当て、インデックスを記録
        newGroup.srvindex = srvmanager_->Allocate();
        // 構造体バッファ用のSRVを作成
        srvmanager_->CreateSRVforStructuredBuffer(newGroup.srvindex, newGroup.Resource.Get(), MaxInstanceCount, sizeof(InstanceData));
    }
}

void ParticleManager::Emit(const std::string& name, const Vector3& position, uint32_t count, const Vector3& velocity, float lifetime) {

    auto it = particleGroups.find(name);
    if (it == particleGroups.end()) {
        throw std::runtime_error("Particle group not found: " + name);
    }

    ParticleGroup& group = it->second;
    camera_ = Object3dCommon::GetInstance()->GetDefaultCamera();

    size_t currentParticleCount = group.particles.size();
    if (currentParticleCount + count > MaxInstanceCount) {
        count = static_cast<uint32_t>(MaxInstanceCount - currentParticleCount);
    }

    if (count == 0) return;

    // ランダムオフセット
    std::uniform_real_distribution<float> dist(0.0f, 0.0f);
    std::uniform_real_distribution<float> colorDist(0.0f, 1.0f);
    std::uniform_real_distribution<float> distRotate(-std::numbers::pi_v<float>, std::numbers::pi_v<float>);
    std::uniform_real_distribution<float> distScale(0.4f, 1.5f);

    for (uint32_t i = 0; i < count; ++i) {
        Vector3 offset(dist(randomEngine), dist(randomEngine), dist(randomEngine));
        Vector3 rotate = Vector3(0.0f,3.0f, 0.0f);
        Vector3 scale = Vector3(1.0f, 1.0f, 1.0f);

        Particle newParticle;
        newParticle.transform.translate = { position.x + offset.x,position.y + offset.y ,position.z + offset.z };
        newParticle.transform.rotate = rotate;
        newParticle.transform.scale = scale;
        //newParticle.color = { colorDist(randomEngine),  colorDist(randomEngine),  colorDist(randomEngine),1.0f };
        newParticle.color = { 1.0f,1.0f,1.0f,1.0f };
        newParticle.lifetime = lifetime;
        newParticle.currentTime = 0.0f;
        newParticle.Velocity = velocity;  // 渡されたベロシティを使う

        // 作成したパーティクルをパーティクルリストに追加
        group.particles.push_back(newParticle);
    }
    // 描画で使用するインスタンス数を更新
    group.kNumInstance = static_cast<uint32_t>(group.particles.size());
}

void ParticleManager::DebugUpdata() {
#ifdef USE_IMGUI
    // ウィンドウサイズを指定
    ImGui::Begin("Particle");
    // 全パーティクルグループの合計数を表示
    int totalCount = 0;
    for (const auto& [name, group] : particleGroups) {
        totalCount += static_cast<int>(group.particles.size());
    }
    ImGui::SliderInt("MaxInstanceCount", (int*)&MaxInstanceCount, 0, 1000);
    ImGui::Text("Max Particles: %u", MaxInstanceCount);
    ImGui::Text("Current Total Particles: %d", totalCount);

    // 各グループごとの詳細も表示（任意）
    for (const auto& [name, group] : particleGroups) {
        ImGui::Separator();
        ImGui::Text("Group: %s", name.c_str());
        ImGui::Text("  Particles: %zu", group.particles.size());
        ImGui::Text("  Instances: %u", group.kNumInstance);
    }
    ImGui::End();
#endif // USE_IMGUI
}

void ParticleManager::DrawSphere(const uint32_t ksubdivision, VertexData* vertexdata) {
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

void ParticleManager::DrawRing(VertexData* vertexData, uint32_t KRingDivide, float KOuterRadius, float KInnerRadius) {
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
