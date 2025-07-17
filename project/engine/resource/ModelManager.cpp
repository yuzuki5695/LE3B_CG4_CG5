#include "ModelManager.h"

ModelManager* ModelManager::instance = nullptr;

ModelManager* ModelManager::GetInstance() {
	if (instance == nullptr) {
		instance = new ModelManager;
	}
	return instance;
}

void ModelManager::Finalize() {
	delete instance;
	instance = nullptr;
}

void ModelManager::Initialize(DirectXCommon* dxCommon) {
	// NULL検出
	assert(dxCommon);
	// 3Dモデル共通部の初期化
	// メンバ変数に記録
	modelCommon = new ModelCommon;
	modelCommon->Initialize(dxCommon);
}

void ModelManager::LoadModel(const std::string& filePath) {
	//  読み込み済みモデルを検索
	if (models.contains(filePath)) {
		// 読み込み済みなら早期return
		return;
	}

	// Resources/ を前に追加
	std::string fullPath = "Resources/" + filePath;

	// ディレクトリとファイル名を分割
	size_t lastSlash = filePath.find_last_of("/\\");
	std::string directory;
	std::string filename;

	if (lastSlash != std::string::npos) {
		directory = "Resources/" + filePath.substr(0, lastSlash);
		filename = filePath.substr(lastSlash + 1);
	} else {
		directory = "Resources";
		filename = filePath;
	}
	
	// モデルの生成とファイル読み込み,初期化
	std::unique_ptr<Model> model = std::make_unique<Model>();
	model->Initialize(modelCommon, directory, filename);
	// モデルをmapコンテナに格納する
	models.insert(std::make_pair(filePath, std::move(model)));
}

Model* ModelManager::FindModel(const std::string& filePath) {
	//  読み込み済みモデルを検索
	if (models.contains(filePath)) {
		// 読み込みモデルを戻り値としてreturn
		return  models.at(filePath).get();
	}
	// ファイル名一致なし -> 例外を投げてエラーにする
    throw std::runtime_error("Model not found: " + filePath);
};