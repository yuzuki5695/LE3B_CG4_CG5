#include "Framework.h"

void Framework::Run() {
    // ゲームの初期化
    Initialize();

    while (true) // ゲームループ
    {
        // 毎フレーム更新
        Update();
        // 終了リクエストが来たらループを抜ける
        if (IsEndRequst()) {
            break;
        }
        // 描画
        Draw();
    }
    // ゲームの終了
    Finalize();
}

void Framework::Finalize() {
    // シーンファクトリの解放 
    sceneFactory_.reset();
    // シーンマネージャの解放
    SceneManager::GetInstance()->Finalize();
    // 基盤システムの解放
    ParticleCommon::GetInstance()->Finalize();
    SpriteCommon::GetInstance()->Finalize();
    Object3dCommon::GetInstance()->Finalize();
    // 入力解放
    Input::GetInstance()->Finalize();
    // パーティクルマネージャの終了
    ParticleManager::GetInstance()->Finalize();
    // テクスチャマネージャの終了
    TextureManager::GetInstance()->Finalize();
    // 3Dモデルマネージャの終了
    ModelManager::GetInstance()->Finalize();
    // ImGuiマネージャの解放
    ImGuiManager::GetInstance()->Finalize();
    // SRVマネージャの開放
    srvManager.reset();
    // DirectXの解放
    dxCommon.reset();
    // WindowsAPIの解放
    winApp.reset();
    //リソースリークチェック
    D3DResourceLeakChecker leakCheck;
}

void Framework::Initialize() {
    OutputDebugStringA("Hello,Directx!\n");
    // ウィンドウ作成
    // WindowsAPIの初期化
    winApp = std::make_unique <WinApp>();    
    // ウィンドウのタイトルを変更
	winApp->SetWindowTitle(L"LE3B_MyGame");
    winApp->Initialize();
    // DirectXの初期化
    dxCommon = std::make_unique <DirectXCommon>();
    dxCommon->Initialize(winApp.get());
    // 音声読み込み
    SoundLoader::GetInstance()->Initialize();
    // 音声再生
    SoundPlayer::GetInstance()->Initialize(SoundLoader::GetInstance());
    // SRVマネージャーの初期化
    srvManager = std::make_unique <SrvManager>();
    srvManager->Initialize(dxCommon.get());
    // ImGuiマネージャの初期化
    ImGuiManager::GetInstance()->Initialize(winApp.get(), dxCommon.get(), srvManager.get());
    // テクスチャマネージャの初期化
    TextureManager::GetInstance()->Initialize(dxCommon.get(), srvManager.get());
    // 3Dモデルマネージャの初期化
    ModelManager::GetInstance()->Initialize(dxCommon.get());
    // パーティクルマネージャの初期化
    ParticleManager::GetInstance()->Initialize(dxCommon.get(), srvManager.get());

#pragma region 基盤システムの初期化

    // 入力の初期化
    Input::GetInstance()->Initialize(winApp.get());

    // スプライト共通部の初期化
    SpriteCommon::GetInstance()->Initialize(dxCommon.get());

    // 3Dオブジェクト共通部の初期化
    Object3dCommon::GetInstance()->Initialize(dxCommon.get());

    // パーティクル共通部の初期化
    ParticleCommon::GetInstance()->Initialize(dxCommon.get());

#pragma endregion 基盤システムの初期化
}

void Framework::Update() {
    // Windowのメッセージ処理
    if (winApp->ProcessMessage()) {
        // ゲームループを抜ける
        endRequst_ = true;
    }
    // 入力の更新
    Input::GetInstance()->Update();
    // ImGuiの受付開始
    ImGuiManager::GetInstance()->Begin();
    // シーンマネージャの更新処理
    SceneManager::GetInstance()->Update();
}

void Framework::Draw() {
    //  描画用のDescriptorHeapの設定
    srvManager->PreDraw();
    //  DirectXの描画準備。全ての描画に共通のグラフィックスコマンドを積む
    dxCommon->PreDraw();
}