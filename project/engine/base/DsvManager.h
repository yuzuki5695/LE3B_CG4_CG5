#pragma once
#include"DirectXCommon.h"

class DsvManager {
public: // メンバ関数

	// 初期化
	void Initialize(DirectXCommon* directXCommon);

private: // メンバ変数
	// ポインタ
	DirectXCommon* dxCommon_ = nullptr;
	// DSV用デスクリプタヒープ
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> descriptorHeap;		
	// DSV用のデスクリプタサイズ
	uint32_t descriptorSize;	
	// DepthStencilTextureをウインドウのサイズ
	Microsoft::WRL::ComPtr <ID3D12Resource> depthStencilResource;
	// DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
public: // メンバ関数	
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> GetDescriptorHeap() { return descriptorHeap; }
	D3D12_DEPTH_STENCIL_DESC GetdepthStencilDesc() { return depthStencilDesc; }
};