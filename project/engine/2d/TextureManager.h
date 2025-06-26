#pragma once
#include <string>
#include "externals/DirectXTex/DirectXTex.h"
#include<wrl.h>
#include<d3d12.h>
#include "DirectXCommon.h"
#include "SrvManager.h"
#include <unordered_map>

// テクスチャマネージャー
class TextureManager
{
private:
	static TextureManager* instance;

	TextureManager() = default;
	~TextureManager() = default;
	TextureManager(TextureManager&) = delete;
	TextureManager& operator=(TextureManager&) = delete;
public: // メンバ関数
	// シングルトンインスタンスの取得
	static TextureManager* GetInstance();
	// 終了
	void Finalize();
	// 初期化
	void Initialize(DirectXCommon* birectxcommon, SrvManager* srvmanager);

	/// <summary>
	/// テクスチャファイルの読み込み
	/// </summary>	/// <param name="filePath"テクスチャファイルのパス></param>
	void LoadTexture(const std::string& filePath);

	// メタデータを取得
	const DirectX::TexMetadata& GetMetaData(const std::string& filepath);
	// SRVインデックスの取得
	uint32_t GetSrvIndex(const std::string& filePath);
	// CPUハンドルの取得
	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPU(const std::string& filepath);

	// テクスチャがすでにロードされているか確認
	bool IsTextureLoaded(const std::string& filepath) const {
		// textures マップにファイルパスが存在するか確認
		return textureDatas.find(filepath) != textureDatas.end();
	}
	
	/// <summary>
	/// テクスチャデータの輸送
	/// </summary>
	/// <param name="texture"></param>
	/// <param name="mipImages"></param>
	void UploadTextureData(Microsoft::WRL::ComPtr <ID3D12Resource>& texture, const DirectX::ScratchImage& mipImages);

private: // メンバ変数
	// テクスチャ1枚分のデータ
	struct TextureData {
		DirectX::TexMetadata metadata;
		Microsoft::WRL::ComPtr<ID3D12Resource> resource;
		uint32_t srvIndex;
		D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU;
		D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU;
	};
	// テクスチャデータ
	std::unordered_map<std::string, TextureData> textureDatas;
	// SRVインデックスの回版番号
	static uint32_t KSRVIndexTop;
	// インスタンスを取得
	DirectXCommon* dxCommon_;
	SrvManager* srvmanager_;
};