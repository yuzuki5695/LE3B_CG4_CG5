#pragma once
#include"DirectXCommon.h"

class SrvManager
{
public: // メンバ関数

	// 最大SRV数(最大テクスチャ枚数)
	static const uint32_t kMaxSRVCount;

	// 初期化
	void Initialize(DirectXCommon* directXCommon);
	// SRVマネージャーの確保
	uint32_t Allocate();
	// 描画前のDescriptorHeapの設定
	void PreDraw();

	/// <summary>
	/// 指定番号のCPUディスクリプタハンドルを取得する
	/// </summary>
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(uint32_t index);
	/// <summary>
	/// 指定番号のGPUディスクリプタハンドルの取得をする
	/// </summary>
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(uint32_t index);

	// SRV生成 (テクスチャ)
	void CreateSRVforTexture2D(uint32_t srvIndex, Microsoft::WRL::ComPtr <ID3D12Resource> pResource, DXGI_FORMAT Format, UINT MipLevels);
	// SRV生成 (Structured Buffer用)
	void CreateSRVforStructuredBuffer(uint32_t srvIndex, Microsoft::WRL::ComPtr <ID3D12Resource> pResource, UINT numElements, UINT structureByteStride);

	// 描画のセット
	void SetGraphicsRootDescriptorTable(UINT RootParameterIndex, uint32_t srvIndex);

	// テクスチャの確保が可能かチェック
	bool TextureDataCheck();

private: // メンバ変数
	// ポインタ
	DirectXCommon* directXCommon = nullptr;
	// SRV用デスクリプタヒープ
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> descriptorHeap;
	// SRV用のデスクリプタサイズ
	uint32_t descriptorSize;
	// 次に使用するSRVインデックス
	uint32_t useIndex = 0;
public:
	// getter
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> GetDescriptorHeap() { return descriptorHeap; }
};