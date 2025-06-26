#pragma once
#include<d3d12.h>
#include<dxgi1_6.h>
#include<wrl.h>
#include<array>
#include<dxcapi.h>
#include<chrono>
#include"WinApp.h"
#include "externals/DirectXTex/DirectXTex.h"
#include <Vector4.h>
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
	void PreDraw();
	// 描画後処理
	void PostDrow();

	/// <summary>
	/// デスクリプタヒープを生成する
	/// </summary>
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);

	// リソース
	Microsoft::WRL::ComPtr <ID3D12Resource> CreateDepthStencilTextureResource(Microsoft::WRL::ComPtr <ID3D12Device>& device, int32_t width, int32_t heigth);

	// コンパイルシェーダー
	Microsoft::WRL::ComPtr <IDxcBlob> CompileShader(const std::wstring& filePath, const wchar_t* profile);

	/// <summary>
	/// バッファリソースの生成
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(size_t sizeInBytes);

	/// <summary>
	/// テクスチャリソースの生成
	/// </summary>
	Microsoft::WRL::ComPtr <ID3D12Resource> CreateTextureResource(const Microsoft::WRL::ComPtr <ID3D12Device>& device, const DirectX::TexMetadata& metadata);

	/// <summary>
	/// レンダーテクスチャの生成
	/// </summar
	Microsoft::WRL::ComPtr <ID3D12Resource> CreateRenderTextureResource(Microsoft::WRL::ComPtr <ID3D12Device> device, uint32_t width, uint32_t height, DXGI_FORMAT format, const Vector4& clearColor);

	/// <summary>
	/// テクスチャデータの輸送
	/// </summary>
	/// <param name="texture"></param>
	/// <param name="mipImages"></param>
	void UploadTextureData(Microsoft::WRL::ComPtr <ID3D12Resource>& texture, const DirectX::ScratchImage& mipImages);

private: // プライベートメンバ関数
	// デバイスの初期化
	void DebugInitialize();
	// コマンド関連の初期化
	void CommandInitialize();
	// スワップチェーンの生成
	void SwapChainGenerate();
	// 深度バッファの生成
	void CreateDepthStencilGenerate();
	// 各種でスクリプタヒープの生成
	void DescriptorHeapGenerate();
	// レンダーターゲットビューの初期化
	void RenderviewInitialize();
	// 深度ステルスビューの初期化
	void DepthstealthviewInitialization();
	// フェンスの初期化
	void FenceInitialize();
	// ビューポートの初期化
	void viewportInitialize();
	// シザリング矩形
	void scissorRectInitialize();
	// DXCコンパイラの生成
	void DxCompilerGenerate();
	
	/// <summary>
	/// 指定番号のCPUディスクリプタハンドルを取得する
	/// </summary>
	static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(Microsoft::WRL::ComPtr <ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorsize, uint32_t index);

	/// <summary>
	/// 指定番号のGPUディスクリプタハンドルの取得をする
	/// </summary>
	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorsize, uint32_t index);
	
	// FPS固定初期化
	void InitializeFizFPS();
	//  FPS固定更新
	void UpdateFixFPS();

private: // メンバ変数
	// ポインタ
	WinApp* winApp_ = nullptr;
	// Devicex12デバイス
	Microsoft::WRL::ComPtr <ID3D12Device> device;
	// DXGIファクトリ
	Microsoft::WRL::ComPtr <IDXGIFactory7> dxgiFactory;
	// コマンドアロケータ
	Microsoft::WRL::ComPtr <ID3D12CommandAllocator> commandAllocator = nullptr;
	// コマンドリスト
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
	// コマンドキュー
	Microsoft::WRL::ComPtr <ID3D12CommandQueue> commandQueue;
	// SwapChain(スワップチェーン)
	Microsoft::WRL::ComPtr <IDXGISwapChain4> swapChain;
	// スワップチェーン生成
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	// 深度バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> depthbufferresource;
	// RTV用のヒープでディスクリプタ
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> rtvDescriptorHeap;
	// DSV用のヒープでディスクリプタ
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> dsvDescriptorHeap;
	// 各DescriptorSizeを取得する
	uint32_t descriptorsizeRTV;
	uint32_t descriptorsizeDSV;
	// スワップチェーンリソース
	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 3> swapChainResources;
	//RTVの設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	//ディスクリプタの先頭を取得する
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle;
	//RTVを2つ作るのでディスクリプタハンドルを2つ用意
	const uint32_t rtvHandlenum = 3;
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[3];
	// DepthStencilTextureをウインドウのサイズ
	Microsoft::WRL::ComPtr <ID3D12Resource> depthStencilResource;
	// フェンスの生成
	Microsoft::WRL::ComPtr <ID3D12Fence> fence = nullptr;
	// 初期値0でFenceを作る
	UINT64 fenceVal = 0;
	//FenceのSignalを待つためのイベントを作成する
	HANDLE fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	// ビューポート
	D3D12_VIEWPORT viewport{};
	// シザー短形
	D3D12_RECT scissorRect{};
	// DXCコンパイラの初期化
	Microsoft::WRL::ComPtr <IDxcUtils> dxcUtils = nullptr;
	Microsoft::WRL::ComPtr <IDxcCompiler3> dxcCompiler = nullptr;
	Microsoft::WRL::ComPtr <IDxcIncludeHandler> includeHandler = nullptr;
	// TransitionBarrierの設定
	D3D12_RESOURCE_BARRIER barrier{};
	// 描画先のRTVとDSVを設定する
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle;
	// DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	// 記録時間(FPS固定)
	std::chrono::steady_clock::time_point reference_;
public:
	// getter
	Microsoft::WRL::ComPtr <ID3D12Device> GetDevice() const { return device.Get(); }
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> GetCommandList() const { return commandList.Get(); }
	D3D12_DEPTH_STENCIL_DESC GetdepthStencilDesc() { return depthStencilDesc; }
	HANDLE GetfenceEvent() const { return fenceEvent; }
	// スワップチェーンリソースの数を取得
	size_t  GetSwapChainResourcesNum() const { return  swapChainResources.size(); }
};