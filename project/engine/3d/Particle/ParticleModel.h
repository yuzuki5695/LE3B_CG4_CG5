#pragma once
#include<Model.h>
#include<DirectXCommon.h>	
#include<PrimitiveGenerator.h>

enum class VertexType {
	Model,
	Ring,
	Sphere,
	Cylinder,
	Star,
	Spiral,
	Circle,
	Box,
	Cloud,
};

// パーティクルモデル
class ParticleModel
{
public:
	// 初期化
	void Initialize(DirectXCommon* birectxcommon, const std::string& filename);
	// 描画処理
	void Draw();

private:
	// .mtlファイルの読み取り
	static MaterialDate LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);
	// .objファイルの読み取り
	static ModelDate LoadObjFile(const std::string& directoryPath, const std::string& filename);
	// 頂点データ作成
	void CreateVertexBuffer();        // 共通処理にする
	void VertexDataModel();
	void VertexDataRing();
	void VertexDataSphere();
	void VertexDataStar();
	void VertexDataCylinder();
	void VertexDataSpiral();
	void VertexDataCircle();
	void VertexDataBox();
	void VertexDataCloud();
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

	VertexType vertexType_ = VertexType::Model;

	uint32_t vertexCount; // 頂点数
public:

	size_t GetVertexCount() const { return modelDate.vertices.size(); }


	void SetVertexType(VertexType type) { vertexType_ = type; }

};