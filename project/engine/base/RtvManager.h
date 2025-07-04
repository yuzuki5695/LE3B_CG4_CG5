#pragma once
#include<DirectXCommon.h>

// レンダーテクスチャの状態変異
enum class RenderTextureState {
	RenderTarget,             // 現在、描画先（RTV）として使用中
	PixelShaderResource       // 現在、SRVとしてシェーダから読み込み可能
};

class RtvManager {
public: // メンバ関数

	// 初期化
	void Initialize(DirectXCommon* directXCommon);
	
	/// <summary>
	/// レンダーテクスチャのテクスチャリソースの生成
	/// </summary>
	void PreDrawRenderTexture();

	/// <summary>
	/// レンダーテクスチャの描画後処理
	/// </summary>
	void PostDrawRenderTexture();

	// RTVのハンドル取得（インデックス指定）
	D3D12_CPU_DESCRIPTOR_HANDLE GetRtvHandle(uint32_t index) const { return rtvHandles[index]; }
	
	// リソースの状態を遷移
	void TransitionResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState);

private: // メンバ変数
	// ポインタ
	DirectXCommon* dxCommon_ = nullptr;
	// RTV用デスクリプタヒープ
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> descriptorHeap;
	// RTV用のデスクリプタサイズ
	uint32_t descriptorSize;
	//RTVの設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	//ディスクリプタの先頭を取得する
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle;
	//RTVを2つ作るのでディスクリプタハンドルを2つ用意
	const uint32_t rtvHandlenum = 3;
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[3];
	// オフスクリーン用のレンダーテクスチャ		
	// レンダーテクスチャの状態変異
	RenderTextureState renderTextureState = RenderTextureState::RenderTarget; // 初期状態はRenderTarget
	Microsoft::WRL::ComPtr<ID3D12Resource> renderTextureResource;              // カスタムRenderTarget用のリソース
	Vector4 kRenderTargetClearValue{};                                         // カスタムRenderTargetのリソースカラー
	uint32_t srvIndexRenderTexture;                                            // レンダーテクスチャのSRVインデックス
public:
    // ディスクリプタヒープ取得
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap>  GetHeap() const { return descriptorHeap; }	
	Microsoft::WRL::ComPtr <ID3D12Resource> GetrenderTextureResource() const { return renderTextureResource; }
	const Vector4 GetkRenderTargetClearValue() const { return kRenderTargetClearValue; }
};