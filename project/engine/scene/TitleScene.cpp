#include "TitleScene.h"
#include<TextureManager.h>
#include<ModelManager.h>
#include<SpriteCommon.h>
#include<Object3dCommon.h>
#include <ParticleCommon.h>
#include <ParticleManager.h>
#include<Input.h>
#ifdef USE_IMGUI
#include<ImGuiManager.h>
#endif // USE_IMGUI
#include<SceneManager.h>

void TitleScene::Finalize() {
}

void TitleScene::Initialize() {

#pragma region 最初のシーンの初期化


#pragma endregion 最初のシーンの初期化

}

void TitleScene::Update() {
    // ENTERキーを押したら
    if (Input::GetInstance()->Triggrkey(DIK_RETURN)) {
        // シーン切り替え
        SceneManager::GetInstance()->ChangeScene("GAMEPLAY");
    }

#pragma region  ImGuiの更新処理開始



#pragma endregion ImGuiの更新処理終了

#pragma region 全てのObject3d個々の更新処理



#pragma endregion 全てのObject3d個々の更新処理

#pragma region 全てのSprite個々の更新処理



#pragma endregion 全てのSprite個々の更新処理
#ifdef USE_IMGUI
    // ImGuiの描画前準備
    ImGuiManager::GetInstance()->End();
#endif // USE_IMGUI
}

void TitleScene::Draw() {
#pragma region 全てのObject3d個々の描画処理
    // 3Dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
    Object3dCommon::GetInstance()->Commondrawing();


    // パーティクルの描画準備。パーティクルの描画に共通のグラフィックスコマンドを積む 
    ParticleCommon::GetInstance()->Commondrawing();


#pragma endregion 全てのObject3d個々の描画処理

#pragma region 全てのSprite個々の描画処理
    // Spriteの描画準備。Spriteの描画に共通のグラフィックスコマンドを積む
    SpriteCommon::GetInstance()->Commondrawing();



#pragma endregion 全てのSprite個々の描画処理
}