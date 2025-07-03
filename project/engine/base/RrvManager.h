#pragma once
#include <DirectXCommon.h>

// レンダーテクスチャの状態変異
enum class RenderTextureState {
	RenderTarget,             // 現在、描画先（RTV）として使用中
	PixelShaderResource       // 現在、SRVとしてシェーダから読み込み可能
};

class RrvManager
{
public: // メンバ関数	
	// 初期化
	void Initialize(DirectXCommon* directXCommon);
	
private: // メンバ変数
	// ポインタ
	DirectXCommon* directXCommon = nullptr;
	// RTV用デスクリプタヒープ
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> descriptorHeap;
	// レンダーテクスチャの状態変異
	RenderTextureState renderTextureState = RenderTextureState::RenderTarget; // 初期状態はRenderTarget
	// RTV用のデスクリプタサイズ
	uint32_t descriptorSize;
	//RTVの設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	// ディスクリプタの先頭を取得する
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle;
	// RTVを2つ作るのでディスクリプタハンドルを2つ用意
	const uint32_t rtvHandlenum = 3;
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[3];
	// レンダーテクスチャ－
	Microsoft::WRL::ComPtr<ID3D12Resource> renderTextureResource;              // カスタムRenderTarget用のリソース
	Vector4 kRenderTargetClearValue{};                                         // カスタムRenderTargetのリソースカラー
};