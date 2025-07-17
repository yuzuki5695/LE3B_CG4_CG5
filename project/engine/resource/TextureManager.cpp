#include "TextureManager.h"
#include <Logger.h>
#include <StringUtility.h>
#include <ResourceFactory.h>

using namespace Microsoft::WRL;
using namespace ResourceFactory;

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
	HRESULT hr;
	std::string fullPath;

	// 既に Resources/ から始まっているならそのまま使う（model読み込み時など）
	if (filePath.starts_with("Resources/")) {
		fullPath = filePath;
	} else {
		fullPath = "Resources/" + filePath ;
	}

	// 読み込み済みテクスチャの検索
	if (textureDatas.contains(fullPath)) {
		return; // 読み込み済みなら早期return
	}
	// テクスチャ枚数上限チェック
	assert(srvmanager_->TextureDataCheck());

	// テクスチャファイルを読み込でプログラムで扱えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW = StringUtility::ConvertString(fullPath);
	if (filePathW.ends_with(L".dds")) { // .ddsで終わっていたらddsとみなす。より安全な方法はいくらでもあるので余裕があれば対応すると良い
		hr = DirectX::LoadFromDDSFile(filePathW.c_str(), DirectX::DDS_FLAGS_NONE, nullptr, image);
	} else {
		hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	}
	assert(SUCCEEDED(hr));

	//ミップマップの作成
	DirectX::ScratchImage mipImages{};
	if (DirectX::IsCompressed(image.GetMetadata().format)) { // 圧縮するフォーマットかどうか調べる
		mipImages = std::move(image); // 圧縮フォーマットならそのまま使うのでmoveする
	} else {
		hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 4, mipImages);
		assert(SUCCEEDED(hr));
	}

	// 追加したテクスチャデータの参照を取得する
	TextureData& textureData = textureDatas[fullPath];
	textureData.metadata = mipImages.GetMetadata();
	textureData.resource = CreateTextureResource(dxCommon_->GetDevice(), textureData.metadata);

	// テクスチャデータの転送
	UploadTextureData(textureData.resource, mipImages);

	// テクスチャデータの要素数番号をSRVのインデックスとする
	textureData.srvIndex = srvmanager_->Allocate();
	textureData.srvHandleCPU = srvmanager_->GetCPUDescriptorHandle(textureData.srvIndex);
	textureData.srvHandleGPU = srvmanager_->GetGPUDescriptorHandle(textureData.srvIndex);

	// metaDataを基にSRVの設定
srvmanager_->CreateSRVforTexture2D(textureData.srvIndex, textureData.resource.Get(), textureData.metadata.format, (UINT)textureData.metadata.mipLevels, textureData.metadata.IsCubemap());
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

void TextureManager::UploadTextureData(ComPtr<ID3D12Resource>& texture, const DirectX::ScratchImage& mipImages)
{
    const DirectX::TexMetadata& metadata = mipImages.GetMetadata();

    if (metadata.IsCubemap() || metadata.arraySize > 1) {
        // キューブマップや配列テクスチャの場合
        size_t subresourceIndex = 0;
        for (size_t arraySlice = 0; arraySlice < metadata.arraySize; ++arraySlice) {
            for (size_t mipLevel = 0; mipLevel < metadata.mipLevels; ++mipLevel) {
                const DirectX::Image* img = mipImages.GetImage(mipLevel, arraySlice, 0);
                HRESULT hr = texture->WriteToSubresource(
                    UINT(subresourceIndex),
                    nullptr,
                    img->pixels,
                    UINT(img->rowPitch),
                    UINT(img->slicePitch)
                );
                assert(SUCCEEDED(hr));
                subresourceIndex++;
            }
        }
    }
    else {
        // 通常の2Dテクスチャ
        for (size_t mipLevel = 0; mipLevel < metadata.mipLevels; ++mipLevel)
        {
            const DirectX::Image* img = mipImages.GetImage(mipLevel, 0, 0);
            HRESULT hr = texture->WriteToSubresource(
                UINT(mipLevel),
                nullptr,
                img->pixels,
                UINT(img->rowPitch),
                UINT(img->slicePitch)
            );
            assert(SUCCEEDED(hr));
        }
    }
}