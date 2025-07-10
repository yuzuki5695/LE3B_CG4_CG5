#include "Skybox.h"
#include<SkyboxCommon.h>
#include <ResourceFactory.h>
#include <MatrixVector.h>
#include <CameraManager.h>

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
    // VB & IB 設定
    skyboxCommon->GetDxCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
    // CBV設定（b0: Material、b1: Transform）
    skyboxCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
    // 座標変化行列CBufferの場所を設定
    skyboxCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());
    // SRV設定（t0: CubeMap）
   // skyboxCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(2, skyboxCommon->GetCubeMapGpuHandle());
    // 描画（Indexed）
    skyboxCommon->GetDxCommon()->GetCommandList()->DrawIndexedInstanced(36, 1, 0, 0, 0);
}

std::vector<VertexData> Skybox::CreateSkyboxCubeVertices()
{
    float halfSize = 1.0f;
    VertexData vertices[24];

    // 右面。描画インデックスは内側を向く
    vertices[0].position = { halfSize,  halfSize,  halfSize, 1.0f };
    vertices[1].position = { halfSize,  halfSize, -halfSize, 1.0f };
    vertices[2].position = { halfSize, -halfSize,  halfSize, 1.0f };
    vertices[3].position = { halfSize, -halfSize, -halfSize, 1.0f };

    // 左面 (-X)
    vertices[4].position  = { -halfSize,  halfSize, -halfSize, 1.0f };
    vertices[5].position  = { -halfSize,  halfSize,  halfSize, 1.0f };
    vertices[6].position  = { -halfSize, -halfSize, -halfSize, 1.0f };
    vertices[7].position  = { -halfSize, -halfSize,  halfSize, 1.0f };

    // 前面 (+Z)
    vertices[8].position = { -halfSize,  halfSize,  halfSize, 1.0f };
    vertices[9].position = {  halfSize,  halfSize,  halfSize, 1.0f };
    vertices[10].position = { -halfSize, -halfSize,  halfSize, 1.0f };
    vertices[11].position = {  halfSize, -halfSize,  halfSize, 1.0f };

    // 背面 (-Z)
    vertices[12].position = {  halfSize,  halfSize, -halfSize, 1.0f };
    vertices[13].position = { -halfSize,  halfSize, -halfSize, 1.0f };
    vertices[14].position = {  halfSize, -halfSize, -halfSize, 1.0f };
    vertices[15].position = { -halfSize, -halfSize, -halfSize, 1.0f };

    // 上面 (+Y)
    vertices[16].position = { -halfSize,  halfSize, -halfSize, 1.0f };
    vertices[17].position = {  halfSize,  halfSize, -halfSize, 1.0f };
    vertices[18].position = { -halfSize,  halfSize,  halfSize, 1.0f };
    vertices[19].position = {  halfSize,  halfSize,  halfSize, 1.0f };

    // 底面 (-Y)
    vertices[20].position = { -halfSize, -halfSize,  halfSize, 1.0f };
    vertices[21].position = {  halfSize, -halfSize,  halfSize, 1.0f };
    vertices[22].position = { -halfSize, -halfSize, -halfSize, 1.0f };
    vertices[23].position = {  halfSize, -halfSize, -halfSize, 1.0f };

    return std::vector<VertexData>(vertices, vertices + 24);
}

void Skybox::VertexDatacreation() {
    // 頂点データ生成（24頂点）
    modelDate.vertices = Skybox::CreateSkyboxCubeVertices();
    // 頂点数更新
    vertexCount = static_cast<uint32_t>(modelDate.vertices.size());
    // 頂点バッファ用リソース作成
    vertexResoruce = CreateBufferResource(skyboxCommon->GetDxCommon()->GetDevice(), sizeof(VertexData) * vertexCount);
    // 頂点バッファビューの設定
    vertexBufferView.BufferLocation = vertexResoruce->GetGPUVirtualAddress();
    vertexBufferView.SizeInBytes = sizeof(VertexData) * vertexCount;
    vertexBufferView.StrideInBytes = sizeof(VertexData);
    // GPUバッファに書き込み（Map/Unmap）
    VertexData* mappedData = nullptr;
    vertexResoruce->Map(0, nullptr, reinterpret_cast<void**>(&mappedData));
    memcpy(mappedData, modelDate.vertices.data(), sizeof(VertexData) * vertexCount);
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