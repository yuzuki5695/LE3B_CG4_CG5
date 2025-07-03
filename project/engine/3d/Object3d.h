#pragma once
#include"Model.h"
#include "Camera.h"

class Object3dCommon;
class TransformationMatrix;
class PointLight;
class SpotLight;
class DirectionalLight;

//  3Dオブジェクト
class Object3d
{
public: // メンバ関数
	struct CameraForGPU
	{
		Vector3 worldPosition;
	};

public: // メンバ関数
	// 初期化
	void Initialize(Object3dCommon* object3dCommon);
	// 更新処理
	void Update();
	// 描画処理
	void Draw();

	// 3Dobject作成関数
	static std::unique_ptr<Object3d> Create(std::string filePath, Transform transform);


	void DebugUpdata(const std::string& name);

private:
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
	Model* model = nullptr;
	Camera* camera = nullptr;
	// バッファリソース
	Microsoft::WRL::ComPtr <ID3D12Resource> transformationMatrixResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> cameraResource;
	// バッファリソース内のデータを指すポインタ
	TransformationMatrix* transformationMatrixData = nullptr;
	DirectionalLight* directionalLightDate = nullptr;
	CameraForGPU* cameraForGPUData = nullptr;
	PointLight* pointLightData = nullptr;
	SpotLight* spotLightData = nullptr;


	Microsoft::WRL::ComPtr <ID3D12Resource> directionalLightResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> pointLightResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> spotLightResource;		


	Transform transform_;
public:
	// getter
	const Transform& GetTransform() const { return transform_; }
	const Vector3& GetScale() const { return transform_.scale; }
	const Vector3& GetRotate() const { return transform_.rotate; }
	const Vector3& GetTranslate() const { return transform_.translate; }

	// setter
	void SetModel(const std::string& filePath);
	void SetScale(const Vector3& scale) { this->transform_.scale = scale; }
	void SetRotate(const Vector3& rotate) { this->transform_.rotate = rotate; }
	void SetTranslate(const Vector3& translate) { this->transform_.translate = translate; }
	void SetCamera(Camera* camera) { this->camera = camera; }
};