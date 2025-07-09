#pragma once
#include"DirectXCommon.h"

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
		DirectX::XMMATRIX projectionInverse;
	};
	Material materialData_;
private:
	// ポインタ
	DirectXCommon* dxCommon_;
	DsvManager* dsvManager_;
	// RootSignature
	Microsoft::WRL::ComPtr <ID3D12RootSignature> rootSignature = nullptr;
	Microsoft::WRL::ComPtr <ID3D12PipelineState> graphicsPipelineState = nullptr;
	// シェーダータイプ
	PixelShaderType type_;

	uint32_t depthSrvIndex_ = 0;
	uint32_t samplerIndexPoint_ = 0;  // PointSampler用（必要に応じて）
	uint32_t cbvIndex_ = 0;

public:
	// gettre
	DirectXCommon* GetDxCommon() const { return  dxCommon_; }
};