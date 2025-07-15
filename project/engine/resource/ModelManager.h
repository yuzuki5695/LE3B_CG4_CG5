#pragma once
#include <map>
#include "ModelCommon.h"
#include "Model.h"

// モデルマネージャ
class ModelManager
{
private:

	static ModelManager* instance;

	ModelManager() = default;
	~ModelManager() = default;
	ModelManager(ModelManager&) = delete;
	ModelManager& operator=(ModelManager&) = delete;
public:// メンバ関数
	// シングルトンインスタンスの取得
	static ModelManager* GetInstance();
	// 終了
	void Finalize();
	// 初期化
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// モデルファイルの読み込み
	/// </summary>	/// <param name="filePath"モデルファイルのパス></param>
	void LoadModel(const std::string& filePath);

	/// <summary>
	/// モデルファイルの検索
	/// </summary>	/// <param name="filePath"モデルファイルのパス></param>
	/// <returns>モデル</returns>
	Model* FindModel(const std::string& filePath);

private: // メンバ変数
	// モデルデータ
	std::map<std::string, std::unique_ptr<Model>> models;
	// ポインタ
	ModelCommon* modelCommon = nullptr;
public:
	// getter
	ModelCommon* GetModelCommon() const { return modelCommon; }
};