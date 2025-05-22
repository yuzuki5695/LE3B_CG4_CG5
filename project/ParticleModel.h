#pragma once
#include<Model.h>
#include<DirectXCommon.h>	

// パーティクルモデル
class ParticleModel
{
public:
	// 頂点データ
	struct VertexData
	{
		Vector4 position;
		Vector2 texcoord;
		Vector3 normal;
	};
	// マテリアル
	struct Material {
		Vector4 color;
		int32_t endbleLighting;
		float padding[3];
		Matrix4x4 uvTransform;
	};
	// マテリアルデータ
	struct MaterialDate {
		std::string textureFilePath;
		uint32_t textureindex = 0;
	};
	// モデルデータ
	struct ModelDate {
		std::vector<VertexData> vertices;
		MaterialDate material;
	};
public:
	// 初期化
	void Initialize(DirectXCommon* birectxcommon);
	// 描画処理
	void Draw();

	void DrawRing(VertexData* vertexData, uint32_t KRingDivide, float KOuterRadius, float KInnerRadius);


private:
	// .mtlファイルの読み取り
	static MaterialDate LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);
	// .objファイルの読み取り
	static ModelDate LoadObjFile(const std::string& directoryPath, const std::string& filename);
	// 頂点データ作成
	void VertexDatacreation();
	// マテリアル
	void MaterialGenerate();
private:
	// ポインタ
	DirectXCommon* dxCommon_;
	// Objファイルのデータ
	ModelDate modelDate;
	// バッファリソース
	Microsoft::WRL::ComPtr <ID3D12Resource> vertexResoruce;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;;
	// バッファリソース内のデータを指すポインタ
	VertexData* vertexData = nullptr;
	Material* materialData = nullptr;
	// バッファリソースの使い道を補足するバッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;



	uint32_t vertexCount; //球の頂点数
public:

	size_t GetVertexCount() const { return modelDate.vertices.size(); }





};