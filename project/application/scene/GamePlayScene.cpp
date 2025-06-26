#include "GamePlayScene.h"
#include<TextureManager.h>
#include<ModelManager.h>
#include<SpriteCommon.h>
#include<Object3dCommon.h>
#include<Input.h>
#ifdef USE_IMGUI
#include<ImGuiManager.h>
#endif // USE_IMGUI
#include<SceneManager.h>
#include <ParticleCommon.h>
#include <ParticleManager.h>
#include <numbers>
#include <CameraManager.h>


void GamePlayScene::Finalize() {}

void GamePlayScene::Initialize() {

    // カメラの初期化
    CameraManager::GetInstance()->Initialize();

    // テクスチャを読み込む
    TextureManager::GetInstance()->LoadTexture("Resources/uvChecker.png");
    TextureManager::GetInstance()->LoadTexture("Resources/monsterBall.png");

    // .objファイルからモデルを読み込む
    ModelManager::GetInstance()->LoadModel("plane.obj");
    ModelManager::GetInstance()->LoadModel("terrain.obj");

    // 音声ファイルを追加
    soundData = SoundLoader::GetInstance()->SoundLoadWave("Resources/Alarm01.wav");

    // 音声プレイフラグ
    soundfige = 0;

    grass = Object3d::Create("terrain.obj", Transform({ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} }));

}

void GamePlayScene::Update() {
#pragma region  ImGuiの更新処理開始
#ifdef USE_IMGUI
    // object3d

    // Camera
    CameraManager::GetInstance()->DrawImGui();

#endif // USE_IMGUI
#pragma endregion ImGuiの更新処理終了 
    /*-------------------------------------------*/
    /*--------------Cameraの更新処理---------------*/
    /*------------------------------------------*/
    CameraManager::GetInstance()->Update();

#pragma region 全てのObject3d個々の更新処理

    // 更新処理
    grass->Update();

    ParticleManager::GetInstance()->Update();
#pragma endregion 全てのObject3d個々の更新処理

#pragma region 全てのSprite個々の更新処理


#pragma endregion 全てのSprite個々の更新処理
#ifdef USE_IMGUI
    // ImGuiの描画前準備
    ImGuiManager::GetInstance()->End();
#endif // USE_IMGUI
}

void GamePlayScene::Draw() {

#pragma region 全てのObject3d個々の描画処理
    // 3Dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
    Object3dCommon::GetInstance()->Commondrawing();

    grass->Draw();

    // パーティクルの描画準備。パーティクルの描画に共通のグラフィックスコマンドを積む 
    ParticleCommon::GetInstance()->Commondrawing();
    ParticleManager::GetInstance()->Draw();
#pragma endregion 全てのObject3d個々の描画処理

#pragma region 全てのSprite個々の描画処理
    // Spriteの描画準備。Spriteの描画に共通のグラフィックスコマンドを積む
    SpriteCommon::GetInstance()->Commondrawing();

#pragma endregion 全てのSprite個々の描画処理
}
