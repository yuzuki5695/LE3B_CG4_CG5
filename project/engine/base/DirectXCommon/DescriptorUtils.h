#pragma once
#include <d3d12.h>
#include <wrl.h>
#include<cstdint>

namespace DescriptorUtils {
	/// <summary>
	/// 指定番号のCPUディスクリプタハンドルを取得する
	/// </summary>
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(Microsoft::WRL::ComPtr <ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorsize, uint32_t index);

	/// <summary>
	/// 指定番号のGPUディスクリプタハンドルの取得をする
	/// </summary>
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorsize, uint32_t index);
};