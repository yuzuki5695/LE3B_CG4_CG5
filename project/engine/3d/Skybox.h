#pragma once
#include "Camera.h"
#include<TransformationMatrix.h>
#include <memory>
#include <wrl.h>
#include <d3d12.h>
#include<vector>
#include<Vertex.h>
#include<Material.h>
#include<MaterialDate.h>
#include<ModelDate.h>

class SkyboxCommon;

//  3Dオブジェクト
class Skybox {
public: // メンバ関数
	struct CameraForGPU
	{
		Vector3 worldPosition;
	};

public: // メンバ関数
	// 初期化
	void Initialize(SkyboxCommon* skyboxCommon);
	// 更新処理
	void Update();
	// 描画処理
	void Draw();

	std::vector<VertexData> CreateSkyboxCubeVertices();

private:
	// リソース
//	// トランスフォームマトリックス
//	void TransformationMatrixGenerate();
//	// カメラリソース
//	void CameraForGPUGenerate();
//


private:
	// ポインタ
	SkyboxCommon* skyboxCommon = nullptr;
	Camera* camera = nullptr;
	// モデルデータ
	ModelDate modelDate;
	// バッファリソース
	Microsoft::WRL::ComPtr <ID3D12Resource> vertexResoruce;
	Microsoft::WRL::ComPtr <ID3D12Resource> materialResource;
	// バッファリソース内のデータを指すポインタ
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	// バッファリソース内のデータを指すポインタ
	VertexData* vertexData = nullptr;
	uint32_t vertexCount;
	Material* materialData = nullptr;


	Transform transform_;
};