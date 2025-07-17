#pragma once
#include "Camera.h"
#include<TransformationMatrix.h>
#include <memory>
#include <wrl.h>
#include <d3d12.h>
#include<vector>
#include<Material.h>
#include<MaterialDate.h>

class SkyboxCommon;

//  3Dオブジェクト
class Skybox {
public: // メンバ関数
	struct CameraForGPU
	{
		Vector3 worldPosition;
	};
	struct VertexShaderInput {
		Vector4 position{};
		Vector3 texcoord{};
	};

	struct skyModelDate {
		std::vector<VertexShaderInput> vertices;
		MaterialDate material;
	};

public: // メンバ関数
	// 初期化
	void Initialize(SkyboxCommon* skyboxCommon);
	// 更新処理
	void Update();
	// 描画処理
	void Draw();
	std::vector<VertexShaderInput> CreateSkyboxCubeVertices();
	void SetTexture(const std::string& textureFilePath);
	
	static std::unique_ptr<Skybox> Create(const std::string& textureFilePath, Transform transform);

private:
	// リソース
	// 頂点データ作成
	void VertexDatacreation();
	// マテリアル
	void MaterialGenerate();
	// トランスフォームマトリックス
	void TransformationMatrixGenerate();
	// カメラリソース
	void CameraForGPUGenerate();
private:
	// ポインタ
	SkyboxCommon* skyboxCommon = nullptr;
	Camera* camera = nullptr;
	// モデルデータ
	skyModelDate modelDate;
	// バッファリソース
	Microsoft::WRL::ComPtr <ID3D12Resource> vertexResoruce;
	Microsoft::WRL::ComPtr <ID3D12Resource> materialResource;
	Microsoft::WRL::ComPtr <ID3D12Resource> transformationMatrixResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> cameraResource;
	// バッファリソース内のデータを指すポインタ
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	// バッファリソース内のデータを指すポインタ
	VertexShaderInput* vertexData = nullptr;
	Material* materialData = nullptr;
	TransformationMatrix* transformationMatrixData = nullptr;
	CameraForGPU* cameraForGPUData = nullptr;

	uint32_t vertexCount;
	Transform transform_;
};