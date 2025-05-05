#include "SceneManager.h"
#include <cassert>

// 静的メンバ変数の定義
std::unique_ptr<SceneManager> SceneManager::instance = nullptr;

// シングルトンインスタンスの取得
SceneManager* SceneManager::GetInstance() {
	if (!instance) {
		instance = std::make_unique<SceneManager>();
	}
	return instance.get();
}

// 終了
void SceneManager::Finalize() {
	// 最後のシーンの終了と解放
	if (scene_) {
		scene_->Finalize();
		scene_ = nullptr;
	}
	// シーンマネージャのインスタンスの解放
	instance.reset();
}

void SceneManager::Update() {
	// TODO:シーンの切り替え機構


	// 次のシーンの予約があるなら
	if (nextScene_) {
		// 旧シーンの終了
		if (scene_) {
			scene_->Finalize();
		}

		// シーン切り替え
		scene_ = std::move(nextScene_); // 所有権を移動
		nextScene_ = nullptr;
		// シーンマネージャをセット 
		scene_->SetSceneManeger(this);
		// 次のシーンを初期化する
		scene_->Initialize();
	}

	// 実行中シーンを更新する
	scene_->Update();
}

void SceneManager::Draw() {
	scene_->Draw();
}

void SceneManager::ChangeScene(const std::string& sceneName) {
	assert(sceneFactory_);
	assert(nextScene_ == nullptr);

	//次シーン生成
	nextScene_ = std::unique_ptr<BaseScene>(sceneFactory_->CreateScene(sceneName));
}