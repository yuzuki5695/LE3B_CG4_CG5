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

void GamePlayScene::Finalize() {}

void GamePlayScene::Initialize() {

    // カメラの初期化
    camera = std::make_unique<Camera>();
    camera->SetRotate(Vector3(0.0f, 0.0f, 0.0f));
    camera->SetTranslate(Vector3(0.0f, 0.0f, -1000.0f));
    Object3dCommon::GetInstance()->SetDefaultCamera(camera.get());

    // カメラの現在の位置と回転を取得
    Cameraposition = camera->GetTranslate();
    Camerarotation = camera->GetRotate();

    // テクスチャを読み込む
    TextureManager::GetInstance()->LoadTexture("Resources/uvChecker.png");
    TextureManager::GetInstance()->LoadTexture("Resources/monsterBall.png");
    TextureManager::GetInstance()->LoadTexture("Resources/circle.png");
    TextureManager::GetInstance()->LoadTexture("Resources/grass.png");
    
    // .objファイルからモデルを読み込む
    ModelManager::GetInstance()->LoadModel("plane.obj");
    ModelManager::GetInstance()->LoadModel("axis.obj");
    ModelManager::GetInstance()->LoadModel("monsterBallUV.obj");
    ModelManager::GetInstance()->LoadModel("fence.obj");
    ModelManager::GetInstance()->LoadModel("terrain.obj");

    // 音声ファイルを追加
    soundData = SoundLoader::GetInstance()->SoundLoadWave("Resources/Alarm01.wav");

    // 音声プレイフラグ
    soundfige = 0;


    // スプライトの初期化
    sprite = Sprite::Create("Resources/uvChecker.png", Vector2{ 0.0f,0.0f }, 0.0f, Vector2{ 360.0f,360.0f });

    // オブジェクト作成
    object3d = Object3d::Create("monsterBallUV.obj", Transform({{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}}));
    grass = Object3d::Create("terrain.obj", Transform({ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} }));

    // パーティクル 
    // モデル生成
    ParticleManager::GetInstance()->SetParticleModel("Resources", "plane.obj");
    // テクスチャ生成
    ParticleManager::GetInstance()->CreateParticleGroup("Particles", "Resources/uvChecker.png");
    ParticleManager::GetInstance()->CreateParticleGroup("Circle", "Resources/circle.png");

    // 発生
    emitter = std::make_unique <ParticleEmitter>(
        Vector3{ 0.0f, -0.5f, 0.0f }, // 位置
        3.0f,                         // 発生周期 or 寿命（自由に定義可能）
        0.0f,                         // 経過時間（基本は0から開始）
        3,                            // 発生数
        "Particles",                  // パーティクルグループ名
        Vector3{ 0.001f, 0.0f, 0.0f }  // ← 風
    );

    emitter->Emit();
}

void GamePlayScene::Update() {

    if (Input::GetInstance()->Pushkey(DIK_SPACE) && soundfige == 0) {
        soundfige = 1;
    }

    if (soundfige == 1) {
        // 音声再生
        SoundPlayer::GetInstance()->SoundPlayWave(soundData, false);
        soundfige = 2;
    }

#pragma region  ImGuiの更新処理開始
    // デモウィンドウの表示
    //ImGui::ShowDemoWindow();   

    // スプライト
    //sprite->DebugUpdate();
    
    // object3d
    object3d->DebugUpdata("Object3d");
    grass->DebugUpdata("Grass");

    // Camera
    camera->DebugUpdate();

    ParticleManager::GetInstance()->DebugUpdata();

    emitter->DebugUpdata();

#pragma endregion ImGuiの更新処理終了 
    /*-------------------------------------------*/
    /*--------------Cameraの更新処理---------------*/
    /*------------------------------------------*/
    camera->Update();

#pragma region 全てのObject3d個々の更新処理

    // 更新処理
    object3d->Update();

    grass->Update();


#pragma endregion 全てのObject3d個々の更新処理

#pragma region 全てのSprite個々の更新処理

    
    // 更新処理
    sprite->Update();



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

    object3d->Draw();

    grass->Draw();

    // パーティクルの描画準備。パーティクルの描画に共通のグラフィックスコマンドを積む 
    ParticleCommon::GetInstance()->Commondrawing();

    ParticleManager::GetInstance()->Draw();


#pragma endregion 全てのObject3d個々の描画処理

#pragma region 全てのSprite個々の描画処理
    // Spriteの描画準備。Spriteの描画に共通のグラフィックスコマンドを積む
    SpriteCommon::GetInstance()->Commondrawing();
    
    sprite->Draw();

#pragma endregion 全てのSprite個々の描画処理
}