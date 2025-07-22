#pragma once
#include<DirectXCommon.h>

// レンダーテクスチャの状態変異
enum class DSVState {
	DepthWrite,       // 現在、SRVとしてシェーダから読み込み可能	
	PixelShaderResource,       // 現在、SRVとしてシェーダから読み込み可能
};

class DsvManager {
public: // メンバ関数

	// 初期化
	void Initialize(DirectXCommon* directXCommon);

	/// <summary>
	/// レンダーテクスチャのテクスチャリソースの生成
	/// </summary>
	void PreDraw();

	/// <summary>
	/// レンダーテクスチャの描画後処理
	/// </summary>
	void PostDraw();
	
	// リソースの状態を遷移
	void TransitionResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState);


	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> GetDsvDescriptorHeap() const { return dsvDescriptorHeap; }

	D3D12_DEPTH_STENCIL_DESC GetDepthStencilDesc() const { return depthStencilDesc; }
	void SetDepthStencilDesc(D3D12_DEPTH_STENCIL_DESC StencilDesc) { depthStencilDesc = StencilDesc; }	
	Microsoft::WRL::ComPtr <ID3D12Resource> GetDepthStencilResource() const { return depthStencilResource; }
private: // メンバ変数		
	// ポインタ
	DirectXCommon* dxCommon_ = nullptr;
	// ディスクリプタ
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> dsvDescriptorHeap;	// DSV用のヒープでディスクリプタ
	// 各DescriptorSizeを取得する
	uint32_t descriptorsizeDSV;	 // DSV用
	// DepthStencilTextureをウインドウのサイズ
	Microsoft::WRL::ComPtr <ID3D12Resource> depthStencilResource;	
	// DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	// レンダーテクスチャの状態変異
	DSVState depthState = DSVState::DepthWrite; // 初期状態はRenderTarget
	Microsoft::WRL::ComPtr<ID3D12Resource> renderTextureResource;              // カスタムRenderTarget用のリソース
	Vector4 kRenderTargetClearValue{};                                         // カスタムRenderTargetのリソースカラー
	uint32_t srvIndexRenderTexture;                                            // レンダーテクスチャのSRVインデックス
};