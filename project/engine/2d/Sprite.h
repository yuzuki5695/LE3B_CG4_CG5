#pragma once
#include"Transform.h"
#include"Vector2.h"
#include"Vector3.h"
#include"Vector4.h"
#include "Matrix4x4.h"
#include<wrl.h>
#include<d3d12.h>
#include<cstdint>
#include<fstream>
#include<Vertex.h>
#include<Material.h>

class SpriteCommon;

class Sprite
{
public: // メンバ関数	
	// 座標変換行列データ
	struct TransformationMatrix {
		Matrix4x4 WVP;
		Matrix4x4 World;
	};

public: // メンバ関数
	// 初期化
	void Initialize(SpriteCommon* spriteCommon);
	// 更新処理
	void Update();
	// 描画処理
	void Draw();

	// スプライト作成関数
	static std::unique_ptr<Sprite> Create(std::string textureFilePath, Vector2 position, float rotation, Vector2 size);

	// ImGuiの更新
	void DrawImGui();

private:
	// 頂点データ作成	
	void VertexDatacreation();
	// リソース
	// マテリアル
	void MaterialGenerate();
	// トランスフォームマトリックス
	void TransformationMatrixGenerate();
	// テクスチャサイズをイメージに合わせる
	void AdjustTextureSize();
private: // メンバ変数
	// ポインタ
	SpriteCommon* spriteCommon_ = nullptr;
	// 使用するファイル
	std::string textureFilePath_;
	// バッファリソース
	Microsoft::WRL::ComPtr <ID3D12Resource> vertexResoruce;
	Microsoft::WRL::ComPtr <ID3D12Resource> indexResource;
	Microsoft::WRL::ComPtr <ID3D12Resource> materialResource;
	Microsoft::WRL::ComPtr <ID3D12Resource> transformationMatrixResource;
	// バッファリソース内のデータを指すポインタ
	VertexData* vertexData = nullptr;
	uint32_t* indexData = nullptr;
	Material* materialData = nullptr;
	TransformationMatrix* transformationMatrixData = nullptr;
	// バッファリソースの使い道を補足するバッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;

	// トランスフォーム
	Transform transform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	Transform  uvTransform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	Vector2 position_{};
	float rotation_;
	Vector2 size_{};

	// テクスチャ番号
	uint32_t textureindex = 0;
	// アンカーポイント
	Vector2 anchorPoint = { 0.0f,0.0f };
	// 左右フリップ
	bool isFlipX_ = false;
	// 上下フリップ
	bool isFlipY_ = false;
	// テクスチャ左上座標
	Vector2 textureLeftTop = { 0.0f,0.0f };
	// テクスチャ切り出しサイズ
	Vector2 textureSize = { 640.0f,640.0f };
public:
	// getter
	const Vector2& GetPosition() const { return position_; }
	float GetRotation() const { return rotation_; }
	const Vector4& GetColor() const { return materialData->color; }
	const Vector2& GetSize()const { return size_; }
	const Vector2& GetAnchorPoint()const { return anchorPoint; }
	const bool& GetisFlipX()const { return isFlipX_; }
	const bool& GetisFlipY()const { return isFlipY_; }
	const Vector2& GetTextureLeftTop()const { return textureLeftTop; }
	const Vector2& GetTextureSize()const { return textureSize; }
	// setter
	void SetPosition(const Vector2& position) { this->position_ = position; }
	void SetRotation(float rotation) { this->rotation_ = rotation; }
	void SetColor(const Vector4& color) { materialData->color = color; }
	void SetSize(const Vector2& size) { this->size_ = size; }
	void SetTexture(const std::string& textureFilePath);
	void SetAnchorPoint(const Vector2& anchorPoint) { this->anchorPoint = anchorPoint; }
	void SetisFlipX(const bool& isFlipX) { this->isFlipX_ = isFlipX; }
	void SetisFlipY(const bool& isFlipY) { this->isFlipY_ = isFlipY; }
	void SetTextureLeftTop(const Vector2& textureLeftTop) { this->textureLeftTop = textureLeftTop; }
	void SetTextureSize(const Vector2& textureSize) { this->textureSize = textureSize; }
};