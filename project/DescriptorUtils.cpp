#include "DescriptorUtils.h"

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorUtils::GetCPUDescriptorHandle(Microsoft::WRL::ComPtr <ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorsize, uint32_t index) {

    D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
    handleCPU.ptr += (descriptorsize * index);
    return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE DescriptorUtils::GetGPUDescriptorHandle(Microsoft::WRL::ComPtr <ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorsize, uint32_t index) {
    D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
    handleGPU.ptr += (descriptorsize * index);
    return handleGPU;
}
