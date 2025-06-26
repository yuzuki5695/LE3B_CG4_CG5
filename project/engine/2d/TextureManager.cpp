#include "TextureManager.h"
#include "Logger.h"
#include "StringUtility.h"

// ImGuiで0番目に使用するため、1番から使用
uint32_t TextureManager::KSRVIndexTop = 1;

TextureManager* TextureManager::instance = nullptr;

TextureManager* TextureManager::GetInstance() {
	if (instance == nullptr) {
		instance = new TextureManager;
	}
	return instance;
}

void TextureManager::Finalize() {
	delete instance;
	instance = nullptr;
}

void TextureManager::Initialize(DirectXCommon* birectxcommon, SrvManager* srvmanager) {
	// NULL検出
	assert(birectxcommon);
	// メンバ変数に記録
	this->dxCommon_ = birectxcommon;
	this->srvmanager_ = srvmanager;
	// SRVの数と同数
	textureDatas.reserve(SrvManager::kMaxSRVCount);
}
void TextureManager::LoadTexture(const std::string& filePath) {
	// 読み込み済みテクスチャの検索
	if (textureDatas.contains(filePath)) {
		return; // 読み込み済みなら早期return
	}
	// テクスチャ枚数上限チェック
	assert(srvmanager_->TextureDataCheck());

	// テクスチャファイルを読み込でプログラムで扱えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW = StringUtility::ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));
	//ミップマップの作成
	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(hr));

	// 追加したテクスチャデータの参照を取得する
	TextureData& textureData = textureDatas[filePath];
	textureData.metadata = mipImages.GetMetadata();
	textureData.resource = dxCommon_->CreateTextureResource(dxCommon_->GetDevice(), textureData.metadata);

	// テクスチャデータの転送
	dxCommon_->UploadTextureData(textureData.resource, mipImages);

	// テクスチャデータの要素数番号をSRVのインデックスとする
	textureData.srvIndex = srvmanager_->Allocate();
	textureData.srvHandleCPU = srvmanager_->GetCPUDescriptorHandle(textureData.srvIndex);
	textureData.srvHandleGPU = srvmanager_->GetGPUDescriptorHandle(textureData.srvIndex);

	// metaDataを基にSRVの設定
	srvmanager_->CreateSRVforTexture2D(textureData.srvIndex, textureData.resource.Get(), textureData.metadata.format, (UINT)textureData.metadata.mipLevels);
}

const DirectX::TexMetadata& TextureManager::GetMetaData(const std::string& filepath)
{
	// 範囲外指定違反チェック
	assert(textureDatas.size() + KSRVIndexTop < SrvManager::kMaxSRVCount);
	TextureData& textureData = textureDatas[filepath];
	return textureData.metadata;
}

// SRVインデックスの取得
uint32_t TextureManager::GetSrvIndex(const std::string& filePath) {
	// 読み込み済みテクスチャの検索
	if (textureDatas.contains(filePath)) {
		return textureDatas[filePath].srvIndex;
	}
	assert(0);
	return 0;
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(const std::string& filepath)
{
	// 範囲外指定違反チェック
	assert(textureDatas.size() + KSRVIndexTop < SrvManager::kMaxSRVCount);
	return textureDatas.at(filepath).srvHandleGPU;
}