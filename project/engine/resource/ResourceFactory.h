#pragma once
#include<d3d12.h>
#include<wrl.h>
#include <cstdint>
#include <Vector4.h>
#include <externals/DirectXTex/DirectXTex.h>
#pragma comment(lib,"dxcompiler.lib")

namespace  ResourceFactory {
	/// <summary>
	/// バッファリソースの生成
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(const Microsoft::WRL::ComPtr <ID3D12Device>& device, size_t sizeInBytes);

	/// <summary>
	/// 深度ステンシルテクスチャリソースの生成
	/// </summary>
	Microsoft::WRL::ComPtr <ID3D12Resource> CreateDepthStencilTextureResource(const Microsoft::WRL::ComPtr <ID3D12Device>& device, int32_t width, int32_t heigth);


	/// <summary>
	/// テクスチャリソースの生成
	/// </summary>
	Microsoft::WRL::ComPtr <ID3D12Resource> CreateTextureResource(const Microsoft::WRL::ComPtr <ID3D12Device>& device, const DirectX::TexMetadata& metadata);

	/// <summary>
	/// レンダーテクスチャの生成
	/// </summar
	Microsoft::WRL::ComPtr <ID3D12Resource> CreateRenderTextureResource(Microsoft::WRL::ComPtr <ID3D12Device> device, uint32_t width, uint32_t height, DXGI_FORMAT format, const Vector4& clearColor);
};