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


            particleIterator->transform.translate.x += particleIterator->Velocity.x;


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
    // パーティクルグループごとに描画処理を行う
    for (const auto& [name, particleGroup] : particleGroups) {
        // インスタンス数が0の場合は描画しない
        if (particleGroup.kNumInstance == 0) {
            continue;
        }
        // モデルに必要なバッファをバインド（頂点バッファや定数バッファなど）
        particleGroup.model->Draw();
        // インスタンシングデータの SRV を設定（テクスチャファイルのパスを指定）
        srvmanager_->SetGraphicsRootDescriptorTable(1, particleGroup.srvindex);
        // SRVで画像を表示
        srvmanager_->SetGraphicsRootDescriptorTable(2, particleGroup.materialData.textureindex);
        // 描画（インスタンシング）を実行
        dxCommon_->GetCommandList()->DrawInstanced(static_cast<UINT>(particleGroup.model->GetVertexCount()), static_cast<UINT>(particleGroup.kNumInstance), 0, 0);
    }
}

void ParticleManager::CreateParticleGroup(const std::string& name, const std::string& textureFilepath, const std::string& filename, VertexType vertexType) {
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
        newGroup.model = std::make_unique<ParticleModel>();
        // 頂点タイプをセット
        newGroup.model->SetVertexType(vertexType);
        // モデルの初期化
        newGroup.model->Initialize(dxCommon_, filename);

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

void ParticleManager::Emit(const std::string& name, const Transform& transform, uint32_t count, const Vector3& velocity, float lifetime) {

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

        Particle newParticle;
        newParticle.transform.translate = { transform.translate.x + offset.x,transform.translate.y + offset.y ,transform.translate.z + offset.z };
        newParticle.transform.rotate = { transform.rotate.x,transform.rotate.y,transform.rotate.z + distRotate(randomEngine)};
        newParticle.transform.scale = { transform.scale.x,distScale(randomEngine),transform.scale.z};
        newParticle.color = { 1.0f,1.0f,1.0f,1.0f };
        newParticle.lifetime = lifetime;
        newParticle.currentTime = 0.0f;
        newParticle.Velocity = velocity;

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

void ParticleManager::SetParticleGroupTexture(const std::string& name, const std::string& textureFilepath) {
    auto it = particleGroups.find(name);
    if (it == particleGroups.end()) {
        throw std::runtime_error("Particle group not found: " + name);
    }

    // テクスチャがすでに読み込まれていなければ読み込む
    if (!TextureManager::GetInstance()->IsTextureLoaded(textureFilepath)) {
        TextureManager::GetInstance()->LoadTexture(textureFilepath);
    }

    it->second.materialData.textureFilePath = textureFilepath;
    it->second.materialData.textureindex = TextureManager::GetInstance()->GetSrvIndex(textureFilepath);
}

void ParticleManager::SetParticleGroupModel(const std::string& name, const std::string& modelFilepath) {
    auto it = particleGroups.find(name);
    if (it == particleGroups.end()) {
        throw std::runtime_error("Particle group not found: " + name);
    }

    // モデルがまだ読み込まれていない場合は読み込む
    if (!ModelManager::GetInstance()->FindModel(modelFilepath)) {
        ModelManager::GetInstance()->LoadModel(modelFilepath);
    }

    // モデル差し替え
    if (it->second.model) {
        // モデルが既に存在する場合は再初期化する
        it->second.model->Initialize(dxCommon_, modelFilepath);
    } else {
        // モデルがなければ新たに作成して初期化
        it->second.model = std::make_unique<ParticleModel>();
        it->second.model->SetVertexType(VertexType::Model); // 必要なら別途引数で指定
        it->second.model->Initialize(dxCommon_, modelFilepath);
    }
}
