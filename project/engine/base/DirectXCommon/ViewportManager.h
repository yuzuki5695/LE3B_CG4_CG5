#pragma once
#include <d3d12.h>
#include <cstdint>

// ビューポート・シザー管理
class ViewportManager{
public: // メンバ関数
	/// <summary>
    /// 初期化
    /// </summary>
    void Initialize(uint32_t width, uint32_t height);
private: // メンバ変数
	// ビューポート
	D3D12_VIEWPORT viewport_{};
	// シザー短形
	D3D12_RECT scissorRect_{};	
public: // getter
	const D3D12_VIEWPORT& GetViewport() const { return viewport_; }
    const D3D12_RECT& GetScissorRect() const { return scissorRect_; }
};