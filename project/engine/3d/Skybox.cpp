#include "Skybox.h"
#include<SkyboxCommon.h>
#include <ResourceFactory.h>
#include <MatrixVector.h>

using namespace ResourceFactory;
using namespace MatrixVector;

void Skybox::Initialize(SkyboxCommon* skyboxCommon) {
    // NULL検出
    assert(skyboxCommon);
    // 引数で受け取ってメンバ変数に記録する
    this->skyboxCommon = skyboxCommon;
    this->camera = skyboxCommon->GetDefaultCamera();

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
    
    // マテリアル用のリソース
    materialResource = CreateBufferResource(skyboxCommon->GetDxCommon()->GetDevice(), sizeof(Material));
    // マテリアル用にデータを書き込むためのアドレスを取得
    materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
    // マテリアルデータの初期値を書き込む
    materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
    materialData->uvTransform = MakeIdentity4x4();











}

void Skybox::Update() {

    //// View行列の位置成分をゼロにして回転のみ反映
    //Matrix4 viewWithoutTranslation = camera->GetViewMatrix();
    //viewWithoutTranslation.SetTranslation({ 0.0f, 0.0f, 0.0f });  // ←重要

    //Matrix4 wvp = model->GetWorldMatrix() * viewWithoutTranslation * camera->GetProjectionMatrix();
    //skyboxTransformCBData->WVP = wvp;


}

void Skybox::Draw() {
    //skyboxCommon->GetDxCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &skyboxVertexBufferView);
    //skyboxCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, skyboxMaterialResource->GetGPUVirtualAddress());
    //skyboxCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU("rostock_laage_airport_4k.dds"));
    //skyboxCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(3, samplerDescriptorHandle); // samplerがrootParameter[3]にある前提
    //skyboxCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, skyboxTransformResource->GetGPUVirtualAddress());
    //skyboxCommon->GetDxCommon()->GetCommandList()->DrawInstanced(36, 1, 0, 0); // キューブの36頂点分（12三角形）
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