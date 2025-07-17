#pragma once
#include"DirectXCommon.h"
#include"Camera.h"

class DsvManager;

// 箱の共通部
class SkyboxCommon {
private:
	static std::unique_ptr<SkyboxCommon> instance;

	SkyboxCommon(SkyboxCommon&) = delete;
	SkyboxCommon& operator=(SkyboxCommon&) = delete;
public: // メンバ関数
	SkyboxCommon() = default;
	~SkyboxCommon() = default;

	// シングルトンインスタンスの取得
	static SkyboxCommon* GetInstance();
	// 終了
	void Finalize();
	// 初期化
	void Initialize(DirectXCommon* dxCommon,DsvManager* dsvManager);
	// 共通描画設定
	void Commondrawing();

private:
	// ルートシグネチャの生成
	void RootSignatureGenerate();
	// グラフィックスパイプラインの生成
	void GraphicsPipelineGenerate();
	// DsvManager から元の設定を取得
	D3D12_DEPTH_STENCIL_DESC skyboxDepthStencilDesc;
private:
	// ポインタ
	DirectXCommon* dxCommon_;
	DsvManager* dsvManager_;
	Camera* defaultCamera = nullptr;
	// RootSignature
	Microsoft::WRL::ComPtr <ID3D12RootSignature> rootSignature = nullptr;
	Microsoft::WRL::ComPtr <ID3D12PipelineState> graphicsPipelineState = nullptr;
public:
	// setter
	void SetDefaultCamera(Camera* camera) { this->defaultCamera = camera; }
	// getter
	DirectXCommon* GetDxCommon() const { return  dxCommon_; }
	Camera* GetDefaultCamera() const { return defaultCamera; }
};