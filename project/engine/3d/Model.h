#pragma once
#include"ModelCommon.h"
#include"Transform.h"
#include"Vector2.h"
#include"Vector4.h"
#include "Matrix4x4.h"
#include<d3d12.h>
#include<wrl.h>
#include<cstdint>
#include<fstream>
#include<string>
#include<vector>
#include<Vertex.h>
#include<Material.h>
#include<MaterialDate.h>
#include<ModelDate.h>

class Model
{
public: // メンバ関数
	// 初期化
	void Initialize(ModelCommon* modelCommon, const std::string& directorypath, const std::string& filename);
	// 描画処理
	void Draw();

	// .mtlファイルの読み取り
	static MaterialDate LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);
	// .objファイルの読み取り
	static ModelDate LoadObjFile(const std::string& directoryPath, const std::string& filename);
private:
	// 頂点データ作成
	void VertexDatacreation();
	// リソース
	// マテリアル
	void MaterialGenerate();
private:
	// ポインタ
	ModelCommon* modelCommon = nullptr;
	// Objファイルのデータ
	ModelDate modelDate;
	// バッファリソース
	Microsoft::WRL::ComPtr <ID3D12Resource> vertexResoruce;
	Microsoft::WRL::ComPtr <ID3D12Resource> materialResource;
	// バッファリソースの使い道を補足するバッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	// バッファリソース内のデータを指すポインタ
	VertexData* vertexData = nullptr;
	Material* materialData = nullptr;
public:
	// getter

	Material* GetMaterialData() { return materialData; }

};