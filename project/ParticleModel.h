#pragma once
#include<Model.h>
#include<DirectXCommon.h>	
#include<PrimitiveGenerator.h>

// パーティクルモデル
class ParticleModel
{
public:
	// 初期化
	void Initialize(DirectXCommon* birectxcommon);
	// 描画処理
	void Draw();

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