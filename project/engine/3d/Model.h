#pragma once
#include"ModelCommon.h"
#include"Transform.h"
#include"Vector2.h"
#include"Vector3.h"
#include"Vector4.h"
#include "Matrix4x4.h"
#include<d3d12.h>
#include<wrl.h>
#include<cstdint>
#include<fstream>
#include<string>
#include<vector>

// Model.h
enum class ModelType {
	Custom,  // .objファイルなどから読み込んだモデル
	Sphere   // 手動生成した球体
};

class Model
{
public:
	// 頂点データ
	struct VertexData
	{
		Vector4 position;
		Vector2 texcoord;
		Vector3 normal;
	};
	// マテリアルデータ
	struct Material {
		Vector4 color;
		int32_t endbleLighting;
		float padding[3];
		Matrix4x4 uvTransform;
		float shinimess;
	};
	struct MaterialDate {
		std::string textureFilePath;
		uint32_t textureindex = 0;
	};
	struct ModelDate {
		std::vector<VertexData> vertices;
		MaterialDate material;
	};
public: // メンバ関数
	// 初期化
	void Initialize(ModelCommon* modelCommon, const std::string& directorypath, const std::string& filename);
	// 描画処理
	void Draw();

	// .mtlファイルの読み取り
	static Model::MaterialDate LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);
	// .objファイルの読み取り
	static ModelDate LoadObjFile(const std::string& directoryPath, const std::string& filename);

	// テクスチャ変更
	void ChangeTexture(const std::string& newTexturePath);

	// 球の作成
	void DrawSphere(const uint32_t ksubdivision, VertexData* vertexdata);

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


	uint32_t kSubdivision; //球の分割数
	uint32_t vertexCount; //球の頂点数

public:
	// getter

	Material* GetMaterialData() { return materialData; }

};