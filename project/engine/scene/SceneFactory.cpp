#include "SceneFactory.h"
#include<TitleScene.h>
#include<GamePlayScene.h>
#include<GameClearScene.h>

BaseScene* SceneFactory::CreateScene(const std::string& sceneName) {
	// 次のシーンの生成
	BaseScene* newScene = nullptr;

	if (sceneName == "TITLE") {
		newScene = new TitleScene();
	} else if (sceneName == "GAMEPLAY") {
		newScene = new GamePlayScene();
	}else if (sceneName == "GAMECLEAR") {
		newScene = new GameClearScene();
	}
	return newScene;
}