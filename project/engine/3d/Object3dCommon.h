#pragma once
#include"DirectXCommon.h"
#include"Camera.h"
#include<DirectionalLight.h>
#include<SpotLight.h>
#include<PointLight.h>

class DsvManager;

// 3Dオブジェクト共通部
class Object3dCommon {
private:
	static std::unique_ptr<Object3dCommon> instance;

	Object3dCommon(Object3dCommon&) = delete;
	Object3dCommon& operator=(Object3dCommon&) = delete;
public: // メンバ関数
	Object3dCommon() = default;
	~Object3dCommon() = default;

	// シングルトンインスタンスの取得
	static Object3dCommon* GetInstance();
	// 終了
	void Finalize();
	// 初期化
	void Initialize(DirectXCommon* dxCommon,DsvManager* dsvManager);
	// 共通描画設定
	void Commondrawing();	
	// imgui
	void DrawImGui();
private:
	// ルートシグネチャの生成
	void RootSignatureGenerate();
	// グラフィックスパイプラインの生成
	void GraphicsPipelineGenerate();
	// 平行光源リソース
	void DirectionalLightGenerate();
	// 点光源リソース
	void PointlightSourceGenerate();
	// スポットライトリソース
	void SpotlightGenerate();

private:
	// ポインタ
	DirectXCommon* dxCommon_;
	Camera* defaultCamera = nullptr;
	DsvManager* dsvManager_;
	// RootSignature
	Microsoft::WRL::ComPtr <ID3D12RootSignature> rootSignature = nullptr;
	Microsoft::WRL::ComPtr <ID3D12PipelineState> graphicsPipelineState = nullptr;
	// バッファリソース
	Microsoft::WRL::ComPtr <ID3D12Resource> directionalLightResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> pointLightResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> spotLightResource;
	// バッファリソース内のデータを指すポインタ
	DirectionalLight* directionalLightDate = nullptr;
	PointLight* pointLightData = nullptr;
	SpotLight* spotLightData = nullptr;
public:
	// setter
	void SetDefaultCamera(Camera* camera) { this->defaultCamera = camera; }
	// getter
	DirectXCommon* GetDxCommon() const { return  dxCommon_; }
	Camera* GetDefaultCamera() const { return defaultCamera; }
};