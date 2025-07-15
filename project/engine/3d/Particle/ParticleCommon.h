#pragma once
#include"DirectXCommon.h"
#include"Camera.h"

class DsvManager;

// 3Dオブジェクト共通部
class ParticleCommon
{
private:
	static std::unique_ptr<ParticleCommon> instance;

	ParticleCommon(ParticleCommon&) = delete;
	ParticleCommon& operator=(ParticleCommon&) = delete;
public: // メンバ関数
	ParticleCommon() = default;
	~ParticleCommon() = default;

	// シングルトンインスタンスの取得
	static ParticleCommon* GetInstance();
	// 終了
	void Finalize();
	// 初期化
	void Initialize(DirectXCommon* dxCommon, DsvManager* dsvManager);
	// 共通描画設定
	void Commondrawing();
private:
	// ルートシグネチャの生成
	void RootSignatureGenerate();
	// グラフィックスパイプラインの生成
	void GraphicsPipelineGenerate();
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