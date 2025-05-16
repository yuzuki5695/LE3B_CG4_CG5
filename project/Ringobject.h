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
#include<Object3dCommon.h>

class Ringobject
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

	// 座標変換行列データ
	struct TransformationMatrix {
		Matrix4x4 WVP;
		Matrix4x4 World;
		Matrix4x4 WorldInverseTranspose;
	};
	// 平行光源データ
	struct DirectionalLight {
		Vector4 color; //!< ライトの色
		Vector3 direction; //!< ライトの向き
		float intensity; //!< 輝度
	};
	struct CameraForGPU
	{
		Vector3 worldPosition;
	};

	struct PointLight
	{
		Vector4 color; //!< ライトの色
		Vector3 position; //!< ライトの位置
		float intensity; //!< 輝度
		float radius; //!< ライトの届く最大距離
		float decay; //!< 減衰率
		float padding[2];
	};

	struct SpotLight
	{
		Vector4 color; //!< ライトの色
		Vector3 position; //!< ライトの位置
		float intensity; //!< 輝度
		Vector3 direction; //!< スポットライトの向き
		float distance; //!< ライトの届く最大距離
		float decay; //!< 減衰率
		float cosAngle;  //!< スポットライトの余弦
		float cosFalloffStart;
		float padding[2];
	};
public: // メンバ関数
	// 初期化
	void Initialize(Object3dCommon* object3dCommon, std::string textureFilePath);
	// 更新処理
	void Update();
	// 描画処理
	void Draw();

	// Ringの作成
	void DrawRing(VertexData* vertexData, uint32_t KRingDivide, float KOuterRadius, float KInnerRadius);

	static std::unique_ptr<Ringobject> Create(std::string filePath, Transform transform);

	void DebugUpdata(const std::string& name);

private:
	// 頂点データ作成
	void VertexDatacreation();
	// リソース
	// マテリアル
	void MaterialGenerate();
	// リソース
	// トランスフォームマトリックス
	void TransformationMatrixGenerate();
	// 平行光源リソース
	void DirectionalLightGenerate();
	// カメラリソース
	void CameraForGPUGenerate();
	// 点光源リソース
	void PointlightSourceGenerate();
	// スポットライトリソース
	void SpotlightGenerate();
private:
	// ポインタ	
	Object3dCommon* object3dCommon = nullptr;
	Camera* camera = nullptr;
	// 使用するファイル
	std::string textureFilePath_;
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

	// バッファリソース
	Microsoft::WRL::ComPtr <ID3D12Resource> transformationMatrixResource;
	Microsoft::WRL::ComPtr <ID3D12Resource> directionalLightResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> cameraResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> pointLightResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> spotLightResource;
	// バッファリソース内のデータを指すポインタ
	TransformationMatrix* transformationMatrixData = nullptr;
	DirectionalLight* directionalLightDate = nullptr;
	CameraForGPU* cameraForGPUData = nullptr;
	PointLight* pointLightData = nullptr;
	SpotLight* spotLightData = nullptr;

	Transform transform_;

public:
	// getter

	Material* GetMaterialData() { return materialData; }

};