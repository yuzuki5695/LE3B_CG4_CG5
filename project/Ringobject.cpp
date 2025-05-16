#include "Ringobject.h"
#include "Object3d.h"
#include "MatrixVector.h"
#include "TextureManager.h"
#include <numbers>
#include <cassert>
#ifdef USE_IMGUI
#include<ImGuiManager.h>
#endif // USE_IMGUI

using namespace MatrixVector;

void Ringobject::Initialize(Object3dCommon* object3dCommon, std::string textureFilePath){
    // NULL検出
    assert(object3dCommon);
    // 引数で受け取ってメンバ変数に記録する
    this->object3dCommon = object3dCommon;
    this->camera = object3dCommon->GetDefaultCamera();
    this->textureFilePath_ = textureFilePath;
    // 頂点データの作成
    VertexDatacreation();
    // マテリアルの生成、初期化
    MaterialGenerate();
    // WVP,World用のリソースの生成、初期化
    TransformationMatrixGenerate();
    // 平行光源の生成,初期化
    DirectionalLightGenerate();
    // カメラリソースの生成、初期化
    CameraForGPUGenerate();
    // 点光源リソースの生成、初期化
    PointlightSourceGenerate();
    // スポットライトリソースの生成、初期化
    SpotlightGenerate();
}

std::unique_ptr<Ringobject> Ringobject::Create(std::string filePath, Transform transform) {
    std::unique_ptr<Ringobject> ringobject = std::make_unique<Ringobject>();
    // 初期化
    ringobject->Initialize(Object3dCommon::GetInstance(), filePath);
    // 座標をセット
    ringobject->transform_ = transform;

    return ringobject;
}

void Ringobject::Update() {
    // ワールド行列の作成
    Matrix4x4 worldMatrix = MakeAftineMatrix(transform_.scale, transform_.rotate, transform_.translate);
    // ワールド・ビュー・プロジェクション行列
    Matrix4x4 worldViewProjectionMatrix;
    if (camera) {
        const Matrix4x4& viewProjectionMatrix = camera->GetViewProjectionMatrix();
        worldViewProjectionMatrix = Multiply(worldMatrix, viewProjectionMatrix);
        // ✅ カメラのワールド座標をGPU用に渡す
        cameraForGPUData->worldPosition = camera->GetTranslate();
    } else {
        worldViewProjectionMatrix = worldMatrix;
        // カメラがない場合もデフォルト位置にしておく
        cameraForGPUData->worldPosition = { 0.0f, 0.0f, -1000.0f };
    }
    transformationMatrixData->WVP = worldViewProjectionMatrix;
    transformationMatrixData->World = worldMatrix;
    // WorldInverseTranspose行列を再計算
    transformationMatrixData->WorldInverseTranspose = InverseTranspose(worldMatrix);
}

void Ringobject::Draw() {
    // 座標変化行列CBufferの場所を設定
    object3dCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());
    // 平行光源用のCBufferの場所を設定 
    object3dCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
    // カメラの場所を設定 
    object3dCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(4, cameraResource->GetGPUVirtualAddress());
    // 点光源を設定 
    object3dCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(5, pointLightResource->GetGPUVirtualAddress());
    // スポットライトを設定 
    object3dCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(6, spotLightResource->GetGPUVirtualAddress());

    // VertexBufferViewの設定
    object3dCommon->GetDxCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
    // マテリアルCBufferの場所を設定
    object3dCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
    //SRVのDescriptortableの先頭を設定。２はrootParameter[2]である。
    //SRVを切り替えて画像を変えるS
    object3dCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(textureFilePath_));

    // 描画！
    object3dCommon->GetDxCommon()->GetCommandList()->DrawInstanced(vertexCount, 1, 0, 0);
}

void Ringobject::VertexDatacreation() {

    const uint32_t KRingDivide = 32;
    const float KOuterRadius = 1.0f;
    const float KInnerRadius = 0.2f;
    const float radianPerDivide = 2.0f * std::numbers::pi_v<float> / float(KRingDivide);
    vertexCount = KRingDivide * 6;
    // 関数化したResouceで作成
    vertexResoruce = object3dCommon->GetDxCommon()->CreateBufferResource(sizeof(VertexData) * vertexCount);
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

void Ringobject::MaterialGenerate() {
    // マテリアル用のリソース
    materialResource = object3dCommon->GetDxCommon()->CreateBufferResource(sizeof(Model::Material));
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


void Ringobject::TransformationMatrixGenerate() {
    // WVP,World用のリソースを作る。TransformationMatrixを用意する
    transformationMatrixResource = object3dCommon->GetDxCommon()->CreateBufferResource(sizeof(Object3d::TransformationMatrix));
    // データを書き込むためのアドレスを取得
    transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));
    // 単位行列を書き込んでおく
    transformationMatrixData->WVP = MakeIdentity4x4();
    transformationMatrixData->World = MakeIdentity4x4();
    // WorldInverseTransposeを計算してセット
    transformationMatrixData->WorldInverseTranspose = InverseTranspose(transformationMatrixData->World);
}

void Ringobject::DirectionalLightGenerate() {
    // 平行光源用のリソースを作る
    directionalLightResource = object3dCommon->GetDxCommon()->CreateBufferResource(sizeof(Object3d::DirectionalLight));
    // 平行光源用にデータを書き込むためのアドレスを取得
    directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightDate));
    // デフォルト値はとりあえず以下のようにして置く
    directionalLightDate->color = { 1.0f, 1.0f, 1.0f, 1.0f };
    directionalLightDate->direction = { 0.0f,-1.0f,0.0f };
    directionalLightDate->intensity = 1.0f;
}

void Ringobject::CameraForGPUGenerate() {
    // カメラ用リソースを作る
    cameraResource = object3dCommon->GetDxCommon()->CreateBufferResource(sizeof(Object3d::CameraForGPU));
    // 書き込むためのアドレスを取得
    cameraResource->Map(0, nullptr, reinterpret_cast<void**>(&cameraForGPUData));
    // 単位行列を書き込んでおく
    if (camera) {
        cameraForGPUData->worldPosition = camera->GetTranslate();
    } else {
        // カメラがない場合デフォルト位置にしておく
        cameraForGPUData->worldPosition = { 0.0f, 0.0f, -100.0f };
    }
}

void Ringobject::PointlightSourceGenerate() {
    // 点光源用リソースを作る
    pointLightResource = object3dCommon->GetDxCommon()->CreateBufferResource(sizeof(Object3d::PointLight));
    // 書き込むためのアドレスを取得
    pointLightResource->Map(0, nullptr, reinterpret_cast<void**>(&pointLightData));
    // デフォルト値
    pointLightData->color = { 1.0f,1.0f,1.0f,1.0f };
    pointLightData->position = { 0.0f,2.0f,0.0f };
    pointLightData->intensity = 0.0f;
    pointLightData->radius = 10.0f;
    pointLightData->decay = 1.0f;
}

void Ringobject::SpotlightGenerate() {
    // スポットライトリソースを作る
    spotLightResource = object3dCommon->GetDxCommon()->CreateBufferResource(sizeof(Object3d::SpotLight));
    // 書き込むためのアドレスを取得
    spotLightResource->Map(0, nullptr, reinterpret_cast<void**>(&spotLightData));
    // デフォルト値
    spotLightData->color = { 1.0f,1.0f,1.0f,1.0f };
    spotLightData->position = { 2.0f,1.25f,0.0f };
    spotLightData->distance = 7.0f;
    spotLightData->direction =
        Normalize({ -1.0f,-1.0f,0.0f });
    spotLightData->intensity = 4.0f;
    spotLightData->decay = 2.0f;
    spotLightData->cosFalloffStart = 1.0f;
    spotLightData->cosAngle =
        std::cos(std::numbers::pi_v<float> / 3.0f);
}


void Ringobject::DrawRing(VertexData* vertexData, uint32_t KRingDivide, float KOuterRadius, float KInnerRadius) {
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


void Ringobject::DebugUpdata(const std::string& name) {
#ifdef USE_IMGUI
    ImGui::Begin(name.c_str());
    // 座標セクション
    if (ImGui::CollapsingHeader("Transform")) {
        ImGui::DragFloat3("Translate", &transform_.translate.x, 0.01f);
        ImGui::DragFloat3("Rotate", &transform_.rotate.x, 0.01f);
        ImGui::DragFloat3("Scale", &transform_.scale.x, 0.01f);
    }
    // カラーセクション
    if (ImGui::CollapsingHeader("Material Color")) {
        ImGui::ColorEdit4("Color", reinterpret_cast<float*>(&materialData->color));
        ImGui::DragFloat("Shininess", &materialData->shinimess, 0.01f);
    }
    // ライトセクション
    if (ImGui::CollapsingHeader("Directional Light")) {
        ImGui::DragFloat3("Direction", &directionalLightDate->direction.x, 0.01f);
        ImGui::DragFloat("Intensity", &directionalLightDate->intensity, 0.01f);
        // ポイントライト
        if (ImGui::CollapsingHeader("Point Light")) {
            ImGui::DragFloat3("Position", &pointLightData->position.x, 0.01f);
            ImGui::DragFloat("Intensity", &pointLightData->intensity, 0.01f);
            ImGui::DragFloat("Radius", &pointLightData->radius, 0.01f);
            ImGui::DragFloat("Decay", &pointLightData->decay, 0.01f);
        }
        // スポットライト
        if (ImGui::CollapsingHeader("Spot Light")) {
            ImGui::DragFloat3("Position", &spotLightData->position.x, 0.01f);
            ImGui::DragFloat("Intensity", &spotLightData->intensity, 0.01f);
            ImGui::DragFloat3("Direction", &spotLightData->direction.x, 0.01f);
            ImGui::DragFloat("Decay", &spotLightData->decay, 0.01f);
            ImGui::DragFloat("CosAngle", &spotLightData->cosAngle, 0.01f);
            ImGui::DragFloat("CosFalloffStart", &spotLightData->cosFalloffStart, 0.01f);
        }
    }
    ImGui::End();
#endif // USE_IMGUI
}