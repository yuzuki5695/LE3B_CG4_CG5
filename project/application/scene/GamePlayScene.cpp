#include "GamePlayScene.h"
#include<SceneManager.h>
#include<TextureManager.h>
#include<ModelManager.h>
#include<SpriteCommon.h>
#include<Object3dCommon.h>
#include <CameraManager.h>
#include <ParticleCommon.h>
#include<Input.h>
#include<Controller.h>
#ifdef USE_IMGUI
#include<ImGuiManager.h>
#endif // USE_IMGUI
#include<SkyboxCommon.h>

void GamePlayScene::Finalize() {}

void GamePlayScene::Initialize() {
    // カメラマネージャの初期化
    CameraManager::GetInstance()->Initialize();

    // テクスチャを読み込む
    TextureManager::GetInstance()->LoadTexture("uvChecker.png");
    TextureManager::GetInstance()->LoadTexture("monsterBall.png");

    // .objファイルからモデルを読み込む
    ModelManager::GetInstance()->LoadModel("plane.obj");
    ModelManager::GetInstance()->LoadModel("terrain.obj"); 
    ModelManager::GetInstance()->LoadModel("monsterBallUV.obj");
    
    // 音声ファイルを追加
    SoundData soundData = SoundLoader::GetInstance()->SoundLoadWave("Alarm01.wav");

    // スプライトの作成
    sprite_ = Sprite::Create("uvChecker.png", Vector2{ 0.0f,0.0f }, 0.0f, Vector2{ 180.0f,180.0f });
    
    // オブジェクトの作成
    Object_ =  Object3d::Create("monsterBallUV.obj", Transform({ {1.0f, 1.0f, 1.0f}, {0.0f, -1.6f, 0.0f}, {0.0f, 1.0f, 0.0f} }));
    grass = Object3d::Create("terrain.obj", Transform({ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} })); 
    // ターゲットカメラの追従対象を設定
    CameraManager::GetInstance()->SetTarget(Object_.get());
   
    // 箱の生成
    TextureManager::GetInstance()->LoadTexture("rostock_laage_airport_4k.dds");    
    skybox_ = Skybox::Create("rostock_laage_airport_4k.dds", Transform({ 1000.0f,1000.0f,1000.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f }));
}

void GamePlayScene::Update() {
    /*-------------------------------------------*/
    /*--------------Cameraの更新処理---------------*/
    /*------------------------------------------*/
    CameraManager::GetInstance()->Update();
 
    // ENTERキーを押したら
    if (Input::GetInstance()->Triggrkey(DIK_RETURN)) {
        // シーン切り替え
        //SceneManager::GetInstance()->ChangeScene("GAMECLEAR");
    }

#pragma region 全てのObject3d個々の更新処理
        
    skybox_->Update();

    // 更新処理 
    Object_->Update();
    grass->Update();

    ParticleManager::GetInstance()->Update();
#pragma endregion 全てのObject3d個々の更新処理

#pragma region 全てのSprite個々の更新処理

    sprite_->Update();

#pragma endregion 全てのSprite個々の更新処理
    
#pragma region  ImGuiの更新処理開始
#ifdef USE_IMGUI
    Object3dCommon::GetInstance()->DrawImGui();

    // object3d
    Object_->DrawImGui("Object");
    // Camera
    CameraManager::GetInstance()->DrawImGui();

    //sprite_->DrawImGui();

#endif // USE_IMGUI
#pragma endregion ImGuiの更新処理終了 
}

void GamePlayScene::Draw() {
#pragma region 全てのObject3d個々の描画処理
    // 箱オブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
    SkyboxCommon::GetInstance()->Commondrawing();

    skybox_->Draw();

    // 3Dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
    Object3dCommon::GetInstance()->Commondrawing();
    
    // 描画処理
    Object_->Draw();
    grass->Draw();

    // パーティクルの描画準備。パーティクルの描画に共通のグラフィックスコマンドを積む 
    ParticleCommon::GetInstance()->Commondrawing();
    ParticleManager::GetInstance()->Draw();
#pragma endregion 全てのObject3d個々の描画処理

#pragma region 全てのSprite個々の描画処理
    // Spriteの描画準備。Spriteの描画に共通のグラフィックスコマンドを積む
    SpriteCommon::GetInstance()->Commondrawing();

    sprite_->Draw();

#pragma endregion 全てのSprite個々の描画処理
}