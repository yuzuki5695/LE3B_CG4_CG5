#include "SrvManager.h"

using namespace Microsoft::WRL;

const uint32_t 	SrvManager::kMaxSRVCount = 512;

void SrvManager::Initialize(DirectXCommon* directXCommon) {
	// 引数で受け取ってメンバ変数に記録する
	this->directXCommon = directXCommon;

	// SRV用のヒープでディスクリプタの数は128.RTVはshader内で触るものなので、ShaderVisibleはtrue
	descriptorHeap = directXCommon->CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, kMaxSRVCount, true);
	descriptorSize = directXCommon->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

uint32_t SrvManager::Allocate() {
	// NULL検出
	assert(useIndex < kMaxSRVCount);
	// return する番号を一旦記録しておく
	int index = useIndex;
	// 次回のために番号を1進める
	useIndex++;
	// 上で記録した番号をreturn
	return index;
};

D3D12_CPU_DESCRIPTOR_HANDLE SrvManager::GetCPUDescriptorHandle(uint32_t index)
{
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize * index);
	return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE SrvManager::GetGPUDescriptorHandle(uint32_t index)
{
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize * index);
	return handleGPU;
}

void SrvManager::CreateSRVforTexture2D(uint32_t srvIndex, ComPtr<ID3D12Resource> pResource, DXGI_FORMAT Format, UINT MipLevels) {
	// SRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;  //2Dテクスチャ
	srvDesc.Texture2D.MipLevels = UINT(MipLevels);
	// SRVの生成
	directXCommon->GetDevice()->CreateShaderResourceView(pResource.Get(), &srvDesc, GetCPUDescriptorHandle(srvIndex));
}

void SrvManager::CreateSRVforStructuredBuffer(uint32_t srvIndex, ComPtr<ID3D12Resource> pResource, UINT numElements, UINT structureByteStride) {
	// SRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;  // バッファ
	srvDesc.Buffer.NumElements = numElements;
	srvDesc.Buffer.StructureByteStride = UINT(structureByteStride);
	// SRVの生成
	directXCommon->GetDevice()->CreateShaderResourceView(pResource.Get(), &srvDesc, GetCPUDescriptorHandle(srvIndex));
}

void SrvManager::PreDraw() {
	// 描画用のDescriptorHeapの設定
	ID3D12DescriptorHeap* descriptorHeaps[] = { descriptorHeap.Get() };
	directXCommon->GetCommandList()->SetDescriptorHeaps(1, descriptorHeaps);
}

void SrvManager::SetGraphicsRootDescriptorTable(UINT RootParameterIndex, uint32_t srvIndex) {
	directXCommon->GetCommandList()->SetGraphicsRootDescriptorTable(RootParameterIndex, GetGPUDescriptorHandle(srvIndex));
}

bool SrvManager::TextureDataCheck() {
	if (useIndex < kMaxSRVCount) {
		return true;
	}
	// 確保しているテクスチャより大きい場合はエラー
	return false;
}