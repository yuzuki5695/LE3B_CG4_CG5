#pragma once
#include<DirectXCommon.h>

class DsvManager {
public: // メンバ関数

	// 初期化
	void Initialize(DirectXCommon* directXCommon);


	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> GetDsvDescriptorHeap() const { return dsvDescriptorHeap; }

	D3D12_DEPTH_STENCIL_DESC GetDepthStencilDesc() const { return depthStencilDesc; }
	void SetDepthStencilDesc(D3D12_DEPTH_STENCIL_DESC StencilDesc) { depthStencilDesc = StencilDesc; }

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
};