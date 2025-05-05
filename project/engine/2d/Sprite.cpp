#include "Sprite.h"
#include"SpriteCommon.h"
#include <cassert>
#include "MatrixVector.h"
#include "TextureManager.h"
#include "SrvManager.h"
#ifdef USE_IMGUI
#include<ImGuiManager.h>
#endif // USE_IMGUI

using namespace MatrixVector;

void Sprite::Initialize(SpriteCommon* spriteCommon) {
	// NULL検出
	assert(spriteCommon);
	// 引数で受け取ってメンバ変数に記録する
	this->spriteCommon_ = spriteCommon;
	// 頂点データの作成
	VertexDatacreation();
	// マテリアルの生成、初期化
	MaterialGenerate();
	// テクスチャサイズをイメージに合わせる
	//AdjustTextureSize();
	// WVP,World用のリソースの生成、初期化
	TransformationMatrixGenerate();
}

void Sprite::VertexDatacreation() {
	// 頂点リソースを作る
	vertexResoruce = spriteCommon_->GetDxCommon()->CreateBufferResource(sizeof(VertexData) * 4);
	// 頂点バッファビューを作成する
	// リソースの先頭のアドレスから使う
	vertexBufferView.BufferLocation = vertexResoruce->GetGPUVirtualAddress();
	// 使用するリソースのサイズは4つ分のサイズ
	vertexBufferView.SizeInBytes = sizeof(VertexData) * 4;
	// 1頂点当たりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);
	// 頂点リソースにデータを書き込む
	// 書き込むためのアドレスを取得
	vertexResoruce->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	// 頂点リソースを作る
	indexResource = spriteCommon_->GetDxCommon()->CreateBufferResource(sizeof(uint32_t) * 6);
	// 頂点バッファビューを作成する
	// リソースの先頭のアドレスから使う
	indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
	// 使用するリソースのサイズは6つ分のサイズ
	indexBufferView.SizeInBytes = sizeof(uint32_t) * 6;
	// インデックスはuint32_tとする
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;
}


void Sprite::MaterialGenerate() {
	// マテリアルリソースを作る
	materialResource = spriteCommon_->GetDxCommon()->CreateBufferResource(sizeof(Material));
	// Sprite用にデータを書き込むためのアドレスを取得
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	// 今回は白
	materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	// SpriteはLightingしないでfalseを設定する
	materialData->endbleLighting = false;
	// 単位行列を書き込んでおく
	materialData->uvTransform = MakeIdentity4x4();
}

void Sprite::TransformationMatrixGenerate() {
	// Sprite用のTransformationMatrix用のリソースを作る。
	transformationMatrixResource = spriteCommon_->GetDxCommon()->CreateBufferResource(sizeof(TransformationMatrix));
	// データを書き込むためのアドレスを取得
	transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));
	// 単位行列を書き込んでおく
	transformationMatrixData->World = MakeIdentity4x4();
	transformationMatrixData->WVP = MakeIdentity4x4();
}

void Sprite::Update() {
	
	transform.translate = { position_.x,position_.y,0.0f };
	transform.rotate = { 0.0f,0.0f,rotation_ };
	transform.scale = { size_.x,size_.y,1.0f };

	// アンカーポイント
	float left = 0.0f - anchorPoint.x;
	float right = 1.0f - anchorPoint.x;
	float top = 0.0f - anchorPoint.y;
	float bottom = 1.0f - anchorPoint.y;

	// フリップ
	if (isFlipX_) {
		left = -left;
		right = -right;
	}
	if (isFlipY_) {
		top = -top;
		bottom = -bottom;
	}

	// 頂点にデータを書き込む
	vertexData[0].position = { left,bottom,0.0f,1.0f };    // 左下
	vertexData[1].position = { left,top,0.0f,1.0f };    // 右上
	vertexData[2].position = { right,bottom,0.0f,1.0f };    // 右下
	vertexData[3].position = { right,top,0.0f,1.0f };    // 左上

	// テクスチャ範囲指定
	const DirectX::TexMetadata& metadata = TextureManager::GetInstance()->GetMetaData(textureFilePath_);
	// UV座標系に変換する
	float tex_left = textureLeftTop.x / metadata.width;
	float tex_right = (textureLeftTop.x + textureSize.x) / metadata.width;
	float tex_top = textureLeftTop.y / metadata.height;
	float tex_bottom = (textureLeftTop.y + textureSize.y) / metadata.height;

	// 頂点にデータを書き込む
	vertexData[0].texcoord = { tex_left,tex_bottom };    // 左下
	vertexData[1].texcoord = { tex_left,tex_top };    // 右上
	vertexData[2].texcoord = { tex_right,tex_bottom };    // 右下
	vertexData[3].texcoord = { tex_right,tex_top };    // 左上

	for (int i = 0; i < 4; i++) {
		vertexData[i].normal = { 0.0f,0.0f,-1.0f };
	}

	// インデックスリソースにデータを書き込む
	indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
	indexData[0] = 0; indexData[1] = 1; indexData[2] = 2;
	indexData[3] = 1; indexData[4] = 3; indexData[5] = 2;

	/*--------------------------------------------*/
	/*------worldViewProjectionMatrixを作る--------*/
	/*--------------------------------------------*/
	Matrix4x4 worldMatrixSprite = MakeAftineMatrix(transform.scale, transform.rotate, transform.translate);
	Matrix4x4 viewMatrixSprite = MakeIdentity4x4();
	Matrix4x4 projectionMatrixSprite = MakeOrthographicMatrix(0.0f, 0.0f, float(WinApp::kClientWidth), float(WinApp::kClientHeight), 0.0f, 100.0f);
	Matrix4x4 worldViewProjectionMatrixSprite = Multiply(worldMatrixSprite, Multiply(viewMatrixSprite, projectionMatrixSprite));
	transformationMatrixData->World = worldMatrixSprite;
	transformationMatrixData->WVP = worldViewProjectionMatrixSprite;

	/*----------------------------------------*/
	/*---------UVTransform用の行列を作る--------*/
	/*----------------------------------------*/
	Matrix4x4 uvTransformMatrix = MatrixVector::MakeScaleMatrix(uvTransform.scale);
	uvTransformMatrix = MatrixVector::Multiply(uvTransformMatrix, MatrixVector::MakeRotateZMatrix(uvTransform.rotate.z));
	uvTransformMatrix = MatrixVector::Multiply(uvTransformMatrix, MatrixVector::MakeTranslateMatrix(uvTransform.translate));
	materialData->uvTransform = uvTransformMatrix;
}

void Sprite::Draw() {
	// wvp用のCBufferの場所を設定
	spriteCommon_->GetDxCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
	spriteCommon_->GetDxCommon()->GetCommandList()->IASetIndexBuffer(&indexBufferView);//IBVを設定
	// Spriteの描画。変更が必要なものだけ変更する
	spriteCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	// TransformationMatrixBufferの場所を設定
	spriteCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());
	//// Spriteの描画は常にuvCheckerにする
	//spriteCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);
	// SRVのDescriptortableの先頭を設定
	spriteCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(textureFilePath_));
	// 描画! (DrawCall/ドローコール) 6個のインデックスを使用し1つのインスタンスを描画、その他は当面０で良い
	spriteCommon_->GetDxCommon()->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

void Sprite::SetTexture(const std::string& textureFilePath) {
	// テクスチャインデックスを取得
	uint32_t newTextureIndex = TextureManager::GetInstance()->GetSrvIndex(textureFilePath);
	// テクスチャインデックスを更新
	this->textureindex = newTextureIndex;
}

void Sprite::AdjustTextureSize() {
	// テクスチャメタデータを取得
	const DirectX::TexMetadata& metadata = TextureManager::GetInstance()->GetMetaData(textureFilePath_);
	textureSize.x = static_cast<float>(metadata.width);
	textureSize.y = static_cast<float>(metadata.height);
	// 画像サイズをテクスチャサイズに合わせる
	size_ = textureSize;
}

std::unique_ptr<Sprite> Sprite::Create(std::string textureFilePath, Vector2 position, float rotation, Vector2 size) {
	std::unique_ptr<Sprite> sprite = std::make_unique<Sprite>();
	sprite->Initialize(SpriteCommon::GetInstance());
	// 引数で受け取ってメンバ変数に記録する
	sprite->textureFilePath_ = textureFilePath;
	// 単位行列を書き込んでおく
	sprite->textureindex = TextureManager::GetInstance()->GetSrvIndex(sprite->textureFilePath_);
	sprite->position_ = position;
	sprite->rotation_ = rotation;
	sprite->size_ = size;
	// Transformの初期化
	sprite->transform.translate = { sprite->position_.x,sprite->position_.y,0.0f };
	sprite->transform.rotate = { 0.0f,0.0f,sprite->rotation_ };
	sprite->transform.scale = { sprite->size_.x,sprite->size_.y,1.0f };
	return sprite;
}

void Sprite::DebugUpdata() {
#ifdef USE_IMGUI
	// ウィンドウサイズを指定
	ImGui::Begin("Sprite");
	// 座標
	ImGui::SliderFloat2("position", &position_.x, 0.0f, 1280.0f, "%.01f");
	// 回転
	ImGui::SliderFloat("rotation", &rotation_, -6.2f, 6.2f, "%.01f");
	// サイズ
	ImGui::SliderFloat2("size", &size_.x, 0.0f, 1000.0f, "%.01f");
	// カラー
	ImGui::ColorEdit4("color", reinterpret_cast<float*>(&materialData->color));
	// UV座標の変更
	ImGui::DragFloat2("UVTranslate", &uvTransform.translate.x, 0.01f, -10.0f, 10.0f);
	// UVの大きさの変更
	ImGui::DragFloat2("UVScale", &uvTransform.scale.x, 0.01f, -10.0f, 10.0f);
	// UVの回転の変更
	ImGui::SliderAngle("UVRotate", &uvTransform.rotate.z);
	ImGui::End();
#endif // USE_IMGUI
}