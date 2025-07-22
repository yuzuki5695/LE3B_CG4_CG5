#pragma once
#include"DirectXCommon.h"
#include <Matrix4x4.h>
#include <Camera.h>

class SrvManager;
class RtvManager;
class DsvManager;

class CopylmageCommon
{
private:
	static std::unique_ptr<CopylmageCommon> instance;

	CopylmageCommon(CopylmageCommon&) = delete;
	CopylmageCommon& operator=(CopylmageCommon&) = delete;
public: // メンバ関数
	CopylmageCommon() = default;
	~CopylmageCommon() = default;

	// シングルトンインスタンスの取得
	static CopylmageCommon* GetInstance();
	// 終了
	void Finalize();
	// 初期化
	void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager, RtvManager* rtvManager, DsvManager* dsvManager);
	// 共通描画設定
	void Commondrawing(SrvManager* srvManager);
private:
	// シェーダーの種類
	enum class PixelShaderType {
		Fullscreen,
		Vignette,
		BoxFilter,
		GaussianFilter,
		LuminanceBasedOutline,
		DepthBasedOutline,
	};
	// シェーダーのファイルパスを取得
	std::wstring GetPixelShaderPath(PixelShaderType type);

	// ルートシグネチャの生成
	void RootSignatureGenerate();
	// グラフィックスパイプラインの生成
	void GraphicsPipelineGenerate();
	// SRVマネージャーのインデックス
	uint32_t srvIndex;

	// CopylmageCommon.h
	struct Material {
		Matrix4x4 projectionInverse;
	};
private:
	// ポインタ
	DirectXCommon* dxCommon_;
	DsvManager* dsvManager_;		
	Camera* defaultCamera;
	// RootSignature
	Microsoft::WRL::ComPtr <ID3D12RootSignature> rootSignature = nullptr;
	Microsoft::WRL::ComPtr <ID3D12PipelineState> graphicsPipelineState = nullptr;
	// シェーダータイプ
	PixelShaderType type_;
	
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	Material* materialData;

public:
	// setter
	void SetDefaultCamera(Camera* camera) { this->defaultCamera = camera; }
	// gettre
	DirectXCommon* GetDxCommon() const { return  dxCommon_; }
};