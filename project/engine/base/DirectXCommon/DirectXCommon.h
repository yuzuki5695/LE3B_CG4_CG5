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

// Directx基盤
class DirectXCommon
{
public: // メンバ関数

	// デストラクタ
	~DirectXCommon();
	// 初期化
	void Initialize(WinApp* winApp);
	// 描画前処理
	void PreDraw(D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle,Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> DsvDescriptorHeap);
	// 描画後処理
	void PostDrow();

	/// <summary>
	/// レンダーテクスチャのテクスチャリソースの生成
	/// </summary>
	void PreDrawRenderTexture(D3D12_CPU_DESCRIPTOR_HANDLE RtvHandles, Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> DsvDescriptorHeap, const Vector4 color);

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
	// TransitionBarrierの設定
	D3D12_RESOURCE_BARRIER barrier{};
	// 描画先のRTVとDSVを設定する
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle;
public:
	// getter
	Microsoft::WRL::ComPtr <ID3D12Device> GetDevice() { return device; }
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> GetCommandList() const { return commandList.Get(); }
	// スワップチェーンリソースの数を取得
	SwapChainManager* GetSwapChain() { return swapchain_.get(); }
};