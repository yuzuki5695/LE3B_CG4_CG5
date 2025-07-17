#include "Framework.h"
#include<D3DResourceLeakChecker.h>
#include<DirectXCommon.h>
#include<ModelCommon.h>
#include<SrvManager.h>
#include<SceneFactory.h>
#include <ShaderCompiler.h>
#include<CopylmageCommon.h>
#include<Controller.h>
#include <psapi.h>
#include<SkyboxCommon.h>

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
    SkyboxCommon::GetInstance()->Finalize();
    ParticleCommon::GetInstance()->Finalize();
    SpriteCommon::GetInstance()->Finalize();
    Object3dCommon::GetInstance()->Finalize();
    // 入力解放
    Input::GetInstance()->Finalize();
    // レンダーテクスチャ共通部の解放
    CopylmageCommon::GetInstance()->Finalize();
    // パーティクルマネージャの終了
    ParticleManager::GetInstance()->Finalize();
    // テクスチャマネージャの終了
    TextureManager::GetInstance()->Finalize();
    // 3Dモデルマネージャの終了
    ModelManager::GetInstance()->Finalize();
    // ImGuiマネージャの解放
    ImGuiManager::GetInstance()->Finalize();   
    // DSVマネージャの開放
    dsvManager.reset();
    // RTVマネージャの開放
    rtvManager.reset();
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
	winApp->SetWindowTitle(L"CG4_CG5");
    winApp->Initialize();
    // DirectXの初期化
    dxCommon = std::make_unique <DirectXCommon>();
    dxCommon->Initialize(winApp.get()); 
    // シェーダーコンパイルの初期化
    ShaderCompiler::GetInstance()->Initialize();
    // 音声読み込み
    SoundLoader::GetInstance()->Initialize();
    // 音声再生
    SoundPlayer::GetInstance()->Initialize(SoundLoader::GetInstance());
    // SRVマネージャーの初期化
    srvManager = std::make_unique <SrvManager>();
    srvManager->Initialize(dxCommon.get());   
    // RTVマネージャーの初期化
    rtvManager = std::make_unique <RtvManager>();
    rtvManager->Initialize(dxCommon.get());      
    // DSVマネージャーの初期化
    dsvManager = std::make_unique <DsvManager>();
    dsvManager->Initialize(dxCommon.get());
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
    SpriteCommon::GetInstance()->Initialize(dxCommon.get(),dsvManager.get());

    // 3Dオブジェクト共通部の初期化
    Object3dCommon::GetInstance()->Initialize(dxCommon.get(),dsvManager.get());

    // パーティクル共通部の初期化
    ParticleCommon::GetInstance()->Initialize(dxCommon.get(),dsvManager.get());
 
	// レンダーテクスチャ共通部の初期化
    CopylmageCommon::GetInstance()->Initialize(dxCommon.get(), srvManager.get(),rtvManager.get(),dsvManager.get());
    
    // 箱の共通部の初期化
    SkyboxCommon::GetInstance()->Initialize(dxCommon.get(),dsvManager.get());

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
    // コントローラの更新
    Controller::GetInstance()->Update();
    // ImGuiの受付開始
    ImGuiManager::GetInstance()->Begin();
    // シーンマネージャの更新処理
    SceneManager::GetInstance()->Update(); 
    // デバッグ系ImGui表示
    DrawDebug();
    // ImGuiの描画前準備
    ImGuiManager::GetInstance()->End();
}

void Framework::Draw() {
    //  描画用のDescriptorHeapの設定
    srvManager->PreDraw();
    // レンダーテクスチャをレンダーターゲットにして描画開始準備
    rtvManager->PreDrawRenderTexture();
    dxCommon->PreDrawRenderTexture(rtvManager->GetRtvHandle(2),dsvManager->GetDsvDescriptorHeap(), rtvManager->GetkRenderTargetClearValue());
    // シーンマネージャの描画処理
    SceneManager::GetInstance()->Draw();
    // レンダーテクスチャをSRVとして使うための状態に遷移
    rtvManager->PostDrawRenderTexture();
    //  DirectXの描画準備。全ての描画に共通のグラフィックスコマンドを積む
    // ここから書き込むバックバッファのインデックスを取得
    UINT backBufferIndex = dxCommon->GetSwapChain()->GetSwapChain()->GetCurrentBackBufferIndex();
    dxCommon->PreDraw(rtvManager->GetRtvHandle(backBufferIndex),dsvManager->GetDsvDescriptorHeap());
	// ポストエフェクト描画（レンダーテクスチャ → 画面）
    CopylmageCommon::GetInstance()->Commondrawing(srvManager.get());
}

void Framework::DrawDebug() {
#ifdef USE_IMGUI
    ImGui::Begin("System Monitor");
    
    // FPSと1フレームの処理時間を表示（平均処理時間 = 1000 / FPS）
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    // FPSの推移を記録するためのバッファ（100フレーム分）
    static float values[100] = {};
    static int frame = 0;
    values[frame++ % 100] = ImGui::GetIO().Framerate;
    // FPSの履歴を折れ線グラフで表示
    ImGui::PlotLines("FPS History", values, 100, 0, nullptr, 0.0f, 100.0f);

    PROCESS_MEMORY_COUNTERS pmc;  
    // 現在のプロセス（このプロジェクト）のメモリ使用情報を取得
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        ImGui::Text("Memory Used: %.2f MB", pmc.WorkingSetSize / (1024.0f * 1024.0f));    // 使用中のメモリ（物理メモリ）をMB単位で表示
    }
    ImGui::End();
#endif
}