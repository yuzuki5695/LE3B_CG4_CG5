#include "Skybox.h"
#include<SkyboxCommon.h>
#include <ResourceFactory.h>
#include <MatrixVector.h>
#include <CameraManager.h>
#include <TextureManager.h>
#define _USE_MATH_DEFINES
#include <math.h>

using namespace ResourceFactory;
using namespace MatrixVector;

void Skybox::Initialize(SkyboxCommon* skyboxCommon) {
    // NULL検出
    assert(skyboxCommon);
    // 引数で受け取ってメンバ変数に記録する
    this->skyboxCommon = skyboxCommon;
    this->camera = skyboxCommon->GetDefaultCamera();
    // 頂点データの作成
    VertexDatacreation();
    // マテリアルの生成、初期化
    MaterialGenerate();
    // WVP,World用のリソースの生成、初期化
    TransformationMatrixGenerate();
    // カメラリソースの生成、初期化
    CameraForGPUGenerate();
}

void Skybox::Update() {   
    // ワールド行列の作成
    Matrix4x4 worldMatrix = MakeAftineMatrix(transform_.scale, transform_.rotate, transform_.translate);
    // ワールド・ビュー・プロジェクション行列
    Matrix4x4 worldViewProjectionMatrix;
    // カメラを CameraManager 経由で取得
    Camera* activeCamera = CameraManager::GetInstance()->GetActiveCamera();
    if (activeCamera) {
        const Matrix4x4& viewProjectionMatrix = activeCamera->GetViewProjectionMatrix();
        worldViewProjectionMatrix = Multiply(worldMatrix, viewProjectionMatrix);

        // カメラのワールド座標をGPU用に渡す
        cameraForGPUData->worldPosition = activeCamera->GetTranslate();
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

void Skybox::Draw() {
    // 頂点バッファの設定
    skyboxCommon->GetDxCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
    // ルートパラメータ[0] = b1 : マテリアル用CBV
    skyboxCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
    // ルートパラメータ[1] = b0 : 変換行列用CBV
    skyboxCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());
    // ルートパラメータ[2] = t0 : キューブマップSRV
    skyboxCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(modelDate.material.textureFilePath));
    // 描画呼び出しを変更！
    skyboxCommon->GetDxCommon()->GetCommandList()->DrawInstanced(vertexCount, 1, 0, 0);
}

void Skybox::SetTexture(const std::string& textureFilePath) {
    // モデルデータのテクスチャファイルにファイル名を取得
    modelDate.material.textureFilePath = "Resources/" + textureFilePath;
    // 読み込んだテクスチャの番号を取得
    modelDate.material.textureindex = TextureManager::GetInstance()->GetSrvIndex(modelDate.material.textureFilePath);
}

std::unique_ptr<Skybox> Skybox::Create(const std::string& textureFilePath, Transform transform) {
    std::unique_ptr<Skybox> object = std::make_unique<Skybox>();
    // 初期化
    object->Initialize(SkyboxCommon::GetInstance());
    // モデルをセットする
    object->SetTexture(textureFilePath);
    // 座標をセット
    object->transform_ = transform;
    return object;
}

void Skybox::VertexDatacreation() {   
    // 頂点データ生成
    modelDate.vertices = CreateSkyboxCubeVertices();
    // 頂点数更新
    vertexCount = static_cast<uint32_t>(modelDate.vertices.size());
    // 頂点バッファ用リソース作成
    vertexResoruce = CreateBufferResource(skyboxCommon->GetDxCommon()->GetDevice(), sizeof(VertexShaderInput) * vertexCount);
    // 頂点バッファビューの設定
    vertexBufferView.BufferLocation = vertexResoruce->GetGPUVirtualAddress();
    vertexBufferView.SizeInBytes = sizeof(VertexShaderInput) * vertexCount;
    vertexBufferView.StrideInBytes = sizeof(VertexShaderInput);
    // GPUバッファに書き込み（Map/Unmap）
    vertexResoruce->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
    memcpy(vertexData, modelDate.vertices.data(), sizeof(VertexShaderInput) * vertexCount);
    vertexResoruce->Unmap(0, nullptr);
}

void Skybox::MaterialGenerate() {
    // マテリアル用のリソース
    materialResource = CreateBufferResource(skyboxCommon->GetDxCommon()->GetDevice(), sizeof(Material));
    // マテリアル用にデータを書き込むためのアドレスを取得
    materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
    // マテリアルデータの初期値を書き込む
    materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
    materialData->uvTransform = MakeIdentity4x4();
}

void Skybox::TransformationMatrixGenerate() {
    // WVP,World用のリソースを作る。TransformationMatrixを用意する
    transformationMatrixResource = CreateBufferResource(skyboxCommon->GetDxCommon()->GetDevice(), sizeof(TransformationMatrix));
    // データを書き込むためのアドレスを取得
    transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));
    // 単位行列を書き込んでおく
    transformationMatrixData->WVP = MakeIdentity4x4();
    transformationMatrixData->World = MakeIdentity4x4();
    // WorldInverseTransposeを計算してセット
    transformationMatrixData->WorldInverseTranspose = InverseTranspose(transformationMatrixData->World);
}

void Skybox::CameraForGPUGenerate() {
    // カメラ用リソースを作る
    cameraResource = CreateBufferResource(skyboxCommon->GetDxCommon()->GetDevice(), sizeof(Skybox::CameraForGPU));
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

std::vector<Skybox::VertexShaderInput> Skybox::CreateSkyboxCubeVertices()
{
    float halfSize = 1.0f;

    // 6面 × 2三角形 × 3頂点 = 36頂点
    Skybox::VertexShaderInput vertices[36] = {};

    // 右面 (+X)
    vertices[0].position = { halfSize,  halfSize,  halfSize, 1.0f };
    vertices[1].position = { halfSize, -halfSize,  halfSize, 1.0f };
    vertices[2].position = { halfSize,  halfSize, -halfSize, 1.0f };
    vertices[3].position = { halfSize,  halfSize, -halfSize, 1.0f };
    vertices[4].position = { halfSize, -halfSize,  halfSize, 1.0f };
    vertices[5].position = { halfSize, -halfSize, -halfSize, 1.0f };

    // 左面 (-X)
    vertices[6].position = { -halfSize,  halfSize, -halfSize, 1.0f };
    vertices[7].position = { -halfSize, -halfSize, -halfSize, 1.0f };
    vertices[8].position = { -halfSize,  halfSize,  halfSize, 1.0f };
    vertices[9].position = { -halfSize,  halfSize,  halfSize, 1.0f };
    vertices[10].position = { -halfSize, -halfSize, -halfSize, 1.0f };
    vertices[11].position = { -halfSize, -halfSize,  halfSize, 1.0f };

    // 前面 (+Z)
    vertices[12].position = { -halfSize,  halfSize,  halfSize, 1.0f };
    vertices[13].position = { -halfSize, -halfSize,  halfSize, 1.0f };
    vertices[14].position = { halfSize,  halfSize,  halfSize, 1.0f };
    vertices[15].position = { halfSize,  halfSize,  halfSize, 1.0f };
    vertices[16].position = { -halfSize, -halfSize,  halfSize, 1.0f };
    vertices[17].position = { halfSize, -halfSize,  halfSize, 1.0f };

    // 背面 (-Z)
    vertices[18].position = { halfSize,  halfSize, -halfSize, 1.0f };
    vertices[19].position = { halfSize, -halfSize, -halfSize, 1.0f };
    vertices[20].position = { -halfSize,  halfSize, -halfSize, 1.0f };
    vertices[21].position = { -halfSize,  halfSize, -halfSize, 1.0f };
    vertices[22].position = { halfSize, -halfSize, -halfSize, 1.0f };
    vertices[23].position = { -halfSize, -halfSize, -halfSize, 1.0f };

    // 上面 (+Y)
    vertices[24].position = { -halfSize,  halfSize, -halfSize, 1.0f };
    vertices[25].position = { halfSize,  halfSize, -halfSize, 1.0f };
    vertices[26].position = { -halfSize,  halfSize,  halfSize, 1.0f };
    vertices[27].position = { -halfSize,  halfSize,  halfSize, 1.0f };
    vertices[28].position = { halfSize,  halfSize, -halfSize, 1.0f };
    vertices[29].position = { halfSize,  halfSize,  halfSize, 1.0f };

    // 底面 (-Y)
    vertices[30].position = { -halfSize, -halfSize,  halfSize, 1.0f };
    vertices[31].position = { halfSize, -halfSize,  halfSize, 1.0f };
    vertices[32].position = { -halfSize, -halfSize, -halfSize, 1.0f };
    vertices[33].position = { -halfSize, -halfSize, -halfSize, 1.0f };
    vertices[34].position = { halfSize, -halfSize,  halfSize, 1.0f };
    vertices[35].position = { halfSize, -halfSize, -halfSize, 1.0f };

    // 正規化が必要
    for (int i = 0; i < 36; ++i) {
        Vector3 dir = Vector3(vertices[i].position.x, vertices[i].position.y, vertices[i].position.z);
        dir = Normalize(dir);
        vertices[i].texcoord = dir;
    }

    return std::vector<VertexShaderInput>(vertices, vertices + 36);
}
