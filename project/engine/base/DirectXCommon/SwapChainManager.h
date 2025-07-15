#pragma once
#include <wrl.h>
#include <dxgi1_6.h>
#include <d3d12.h>
#include <cstdint>
#include <array>

class SwapChainManager
{
public: // メンバ関数
	  void Initialize(
        HWND hwnd,
        Microsoft::WRL::ComPtr<ID3D12CommandQueue>& commandQueue,
        Microsoft::WRL::ComPtr<IDXGIFactory7>& dxgiFactory,
        uint32_t width,
        uint32_t height
    );
private: // メンバ変数
	// SwapChain(スワップチェーン)
	Microsoft::WRL::ComPtr <IDXGISwapChain4> swapChain_;	
	// スワップチェーンリソース
	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> resources_;
public: // getter
    Microsoft::WRL::ComPtr<IDXGISwapChain4> GetSwapChain() const { return swapChain_; }
   // リソース配列ごと取得したい場合（参照 or ポインタ）
    const std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2>& GetResources() const { return resources_; }
    Microsoft::WRL::ComPtr<ID3D12Resource> GetBuffer(uint32_t index) const { return resources_[index]; }
};