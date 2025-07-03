#pragma once
#include<d3d12.h>
#include<dxgi1_6.h>
#include<wrl.h>
#include<array>
#include<dxcapi.h>
#include<chrono>
#include<WinApp.h>
#include <Vector4.h>
#include <ViewportManager.h>
#include <FenceManager.h>
#include <FPSController.h>
#include<SwapChainManager.h>
#include "externals/DirectXTex/DirectXTex.h"
#pragma comment(lib,"dxcompiler.lib")
#include<DsvManager.h>

// Directx基盤
class DirectXCommon
{
public: // メンバ関数

	// デストラクタ
	~DirectXCommon();
	// 初期化
	void Initialize(WinApp* winApp);
	// 描画前処理
	void PreDraw(DsvManager* dsvManager);
	// 描画後処理
	void PostDrow();

	/// <summary>
	/// レンダーテクスチャのテクスチャリソースの生成
	/// </summary>
	void PreDrawRenderTexture(DsvManager* dsvManager);

	/// <summary>
	/// レンダーテクスチャの描画後処理
	/// </summary>
	void PostDrawRenderTexture();

	/// <summary>
	/// デスクリプタヒープを生成する
	/// </summary>
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);
	
private: // プライベートメンバ関数
	// デバイスの初期化
	void DebugInitialize();
	// コマンド関連の初期化
	void CommandInitialize();
	// 深度バッファの生成
	void CreateDepthStencilGenerate();
	// 各種でスクリプタヒープの生成
	void DescriptorHeapGenerate();
	// レンダーターゲットビューの初期化
	void RenderviewInitialize();

	// レンダーテクスチャの状態変異
	enum class RenderTextureState {
		RenderTarget,             // 現在、描画先（RTV）として使用中
		PixelShaderResource       // 現在、SRVとしてシェーダから読み込み可能
	};
	// リソースの状態を遷移
	void TransitionResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState);
private: // メンバ変数
	// ポインタ
	WinApp* winApp_ = nullptr;
    std::unique_ptr<ViewportManager> viewport_;
    std::unique_ptr<FenceManager> fence_;	
	std::unique_ptr<FPSController> fpscontroller_;	
	std::unique_ptr<SwapChainManager> swapchain_;
	Microsoft::WRL::ComPtr <ID3D12Device> device;	                                  // Devicex12デバイス
	Microsoft::WRL::ComPtr <IDXGIFactory7> dxgiFactory;		                          // DXGIファクトリ
	Microsoft::WRL::ComPtr <ID3D12CommandAllocator> commandAllocator = nullptr;	      // コマンドアロケータ
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;		              // コマンドリスト
	Microsoft::WRL::ComPtr <ID3D12CommandQueue> commandQueue;			              // コマンドキュー
	Microsoft::WRL::ComPtr<ID3D12Resource> depthbufferresource;			              // 深度バッファ

	// ディスクリプタ
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> rtvDescriptorHeap;	// RTV用のヒープでディスクリプタ
	// 各DescriptorSizeを取得する
	uint32_t descriptorsizeRTV;  // RTV用

	//RTVの設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	//ディスクリプタの先頭を取得する
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle;
	//RTVを2つ作るのでディスクリプタハンドルを2つ用意
	const uint32_t rtvHandlenum = 3;
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[3];
	// オフスクリーン用のレンダーテクスチャ	
	Microsoft::WRL::ComPtr<ID3D12Resource> renderTextureResource;              // カスタムRenderTarget用のリソース
	Vector4 kRenderTargetClearValue{};                                         // カスタムRenderTargetのリソースカラー
	RenderTextureState renderTextureState = RenderTextureState::RenderTarget;  // 初期状態はRenderTarget
	uint32_t srvIndexRenderTexture;                                            // レンダーテクスチャのSRVインデックス

	// DXCコンパイラの初期化
	Microsoft::WRL::ComPtr <IDxcUtils> dxcUtils = nullptr;
	Microsoft::WRL::ComPtr <IDxcCompiler3> dxcCompiler = nullptr;
	Microsoft::WRL::ComPtr <IDxcIncludeHandler> includeHandler = nullptr;
	// TransitionBarrierの設定
	D3D12_RESOURCE_BARRIER barrier{};
	// 描画先のRTVとDSVを設定する
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle;
public:
	// getter
	Microsoft::WRL::ComPtr <ID3D12Device> GetDevice() { return device; }
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> GetCommandList() const { return commandList.Get(); }
	// スワップチェーンリソースの数を取得
	Microsoft::WRL::ComPtr<ID3D12Resource>& GetrenderTextureResource() { return renderTextureResource; }
	SwapChainManager* GetSwapChain() { return swapchain_.get(); }
};