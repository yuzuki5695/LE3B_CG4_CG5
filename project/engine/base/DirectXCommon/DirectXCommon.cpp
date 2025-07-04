#include "DirectXCommon.h"
#include <cassert>
#include <format>
#include <thread>
#include <Logger.h>
#include <StringUtility.h>
#include <DescriptorUtils.h>
#include <ResourceFactory.h>
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

using namespace Microsoft::WRL;
using namespace DescriptorUtils;
using namespace ResourceFactory;

DirectXCommon::~DirectXCommon() { viewport_.reset(); }

void DirectXCommon::Initialize(WinApp* winApp){ 
    // NULL検出
    assert(winApp);
    // メンバ変数に記録
    this->winApp_ = winApp;    
    // ポインタ生成 
    viewport_ = std::make_unique<ViewportManager>();  
    fence_ = std::make_unique<FenceManager>();
    fpscontroller_ = std::make_unique<FPSController>();
    swapchain_ = std::make_unique<SwapChainManager>();
    // 初期化関数
    fpscontroller_->Initialize();                                                     // FPS固定の初期化
	DebugInitialize();	                                                              // デバイスの初期化
	CommandInitialize();	                                                          // コマンド関連の初期化
    swapchain_->Initialize(winApp_->Gethwnd(), commandQueue, dxgiFactory, // スワップチェーンの生成
    WinApp::kClientWidth, WinApp::kClientHeight);
	CreateDepthStencilGenerate();	                                                  // 深度バッファの生成
    fence_->Initialize(device);                                                       // フェンスの初期化
    viewport_->Initialize(WinApp::kClientWidth,WinApp::kClientHeight);                // ビューポート・ シザリング矩形の初期化
}

void DirectXCommon::DebugInitialize() {

    HRESULT hr;

    ///---------------------------------------------------------------------///
    ///-----------------------デバックレイヤーをオン----------------------------///
    ///---------------------------------------------------------------------///
#ifdef _DEBUG
    ComPtr <ID3D12Debug1> debugController = nullptr;

    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
        //デバッグレイヤーを有効化
        debugController->EnableDebugLayer();
        //さらにGPU側でもチェックを行えるようにする
        debugController->SetEnableGPUBasedValidation(TRUE);
    }
#endif // _DEBUG

    ///---------------------------------------------------------------------///
    ///----------------------IDXGIのファクトリー生成---------------------------///
    ///---------------------------------------------------------------------///

    // HRESULTはWindows系のエラーコードであり、
    // 関数が成功したかどうかをSUCCEEDEDマクロで判定できる
    hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
    //初期化の根本的な部分でエラーが出た場合はプログラムが間違っているか、どうにもできない場合が
    //多いのでassertにする
    assert(SUCCEEDED(hr));

    ///---------------------------------------------------------------------///
    ///--------------------------アダプターの列挙------------------------------///
    ///---------------------------------------------------------------------///
    //仕様するアダプター用の変数。最初にnullptrを入れておく
    ComPtr <IDXGIAdapter4> useAdapter = nullptr;
    //良い順にアダプターを頼む
    for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i,
        DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter)) !=
        DXGI_ERROR_NOT_FOUND; ++i) {
        //アダプターの情報を取得する
        DXGI_ADAPTER_DESC3 adapterDesc{};
        hr = useAdapter->GetDesc3(&adapterDesc);
        assert(SUCCEEDED(hr));//取得できないのは一大事
        //ソフトウェアアダプターでなければ採用
        if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
            //採用したアダプタの情報をログに出力。wstringの方なので注意
            Logger::Log(StringUtility::ConvertString(std::format(L"Use Adapater:{}\n", adapterDesc.Description)));
            break;
        }
        useAdapter = nullptr;//ソフトウェアアダプタの場合は見なかったことにする
    }
    //適切なアダプタが見つからないので起動できない
    assert(useAdapter != nullptr);

    ///---------------------------------------------------------------------///
    ///---------------------------デバイスの生成------------------------------///
    ///---------------------------------------------------------------------///
    
    //機能レベルとログ出力用の文字列
    D3D_FEATURE_LEVEL featureLevels[] = {
      D3D_FEATURE_LEVEL_12_2,D3D_FEATURE_LEVEL_12_1,D3D_FEATURE_LEVEL_12_0
    };
    const char* featureLevelStrings[] = { "12.2","12.1","12.0" };
    //高い順に生成できるか試す
    for (size_t i = 0; i < _countof(featureLevels); ++i) {
        //採用したアダプターでデバイスを生成
        hr = D3D12CreateDevice(useAdapter.Get(), featureLevels[i], IID_PPV_ARGS(&device));
        //指定した機能レベルでデバイスが生成できたか確認
        if (SUCCEEDED(hr)) {
            //生成できたのでログ出力を行ってループを抜ける
            Logger::Log(std::format("FeatureLevel : {}\n", featureLevelStrings[i]));
            break;
        }
    }
    //デバイスの生成がうまくいかなかったので起動できない
    assert(device != nullptr);
    Logger::Log("Complete create D3D12Device!!!\n");//初期化完了のログを出す

    ///---------------------------------------------------------------------///
    ///-------------------------エラー時にブレ―ク-----------------------------///
    ///--------------------------------------------------------------------///
#ifdef _DEBUG
    ComPtr<ID3D12InfoQueue> infoQueue = nullptr;
    if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
        //ヤバイエラー時に止まる
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
        //エラー時に止まる
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
        //警告時に止まる
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
        //抑制するメッセージのID
        D3D12_MESSAGE_ID denyIds[] = {
            //Windows11でのDXGIデバッグレイヤーとDX12デバッグレイヤーの相互作用バグによるエラーメッセージ
            //https://stackoverflow.com/questions/69805245/directx-12-application-is-crashing-in-windows-11
            D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
        };
        //抑制するレベル
        D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
        D3D12_INFO_QUEUE_FILTER filter{};
        filter.DenyList.NumIDs = _countof(denyIds);
        filter.DenyList.pIDList = denyIds;
        filter.DenyList.NumSeverities = _countof(severities);
        filter.DenyList.pSeverityList = severities;
        //指定したメッセージの表示を抑制
        infoQueue->PushStorageFilter(&filter);
    }
#endif
}

void DirectXCommon::CommandInitialize() {

    HRESULT hr;
    ///---------------------------------------------------------------------///
    ///------------------------コマンドキューを生成する-------------------------///
    ///---------------------------------------------------------------------///
    D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
    hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
    //コマンドキューの生成がうまくいかなかったので起動できない
    assert(SUCCEEDED(hr));

    ///---------------------------------------------------------------------///
    ///---------------------コマンドアロケータを生成する-------------------------///
    ///---------------------------------------------------------------------///
    //コマンドアロケーターを生成する
    hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
    //コマンドアロケータの生成がうまくいかなかったので起動できない
    assert(SUCCEEDED(hr));

    ///---------------------------------------------------------------------///
    ///------------------------コマンドリストを生成する-------------------------///
    ///---------------------------------------------------------------------///
    //コマンドリストを生成する
    hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));
    //コマンドリストの生成がうまくいかなかったので起動できない
    assert(SUCCEEDED(hr));
}

void DirectXCommon::CreateDepthStencilGenerate() {

    // 生成するResourceの設定
    D3D12_RESOURCE_DESC resourceDesc{};
    resourceDesc.Width = WinApp::kClientWidth; // Textureの幅
    resourceDesc.Height = WinApp::kClientHeight; // Textureの高さ
    resourceDesc.MipLevels = 1; // mipmapの数
    resourceDesc.DepthOrArraySize = 1; // 奥行　or 配列のTexture配列数
    resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // DepthStencilとして利用可能なフォーマット
    resourceDesc.SampleDesc.Count = 1; // サンプリングカウント 1固定
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; // 2次元
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL; // DepthStencilとして使う通知

    // 利用Heapの設定
    D3D12_HEAP_PROPERTIES heapProperies{};
    heapProperies.Type = D3D12_HEAP_TYPE_DEFAULT; // VRAW上に作る

    // 深度値のクリア設定
    D3D12_CLEAR_VALUE depthClearValue{};
    depthClearValue.DepthStencil.Depth = 1.0f; // 1.0f (最大値)でクリア
    depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // フォーマット。 Resourceと合わせる

    //3. Resourceを生成する
    HRESULT hr = device->CreateCommittedResource(
        &heapProperies, //Heapの設定
        D3D12_HEAP_FLAG_NONE, //Heapの特殊な設定。特になし。
        &resourceDesc, //Resourceの設定
        D3D12_RESOURCE_STATE_DEPTH_WRITE, //深度値を書き込む状態にしておく
        &depthClearValue, //Clear最適値
        IID_PPV_ARGS(&depthbufferresource)); //作成するResourceポインタへのポインタ
    assert(SUCCEEDED(hr));
}

void DirectXCommon::PreDrawRenderTexture(D3D12_CPU_DESCRIPTOR_HANDLE RtvHandles, Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> DsvDescriptorHeap, const Vector4 color) {
    // 描画先 = rtvHandles[2]（中間テクスチャ）
    D3D12_CPU_DESCRIPTOR_HANDLE rtHandle = RtvHandles;

    dsvHandle = DsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    commandList->OMSetRenderTargets(1, &rtHandle, false, &dsvHandle);

    // カスタムRenderTarget用で設定した値を使って画面全体をクリアする
    float clearColor[] = { color.x, color.y, color.z, color.w };
    commandList->ClearRenderTargetView(rtHandle, clearColor, 0, nullptr);
    commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
    commandList->RSSetViewports(1, &viewport_->GetViewport());
    commandList->RSSetScissorRects(1, &viewport_->GetScissorRect());
}

void DirectXCommon::PreDraw(D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle,Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> DsvDescriptorHeap) {
    // ここから書き込むバックバッファのインデックスを取得
    UINT backBufferIndex = swapchain_->GetSwapChain()->GetCurrentBackBufferIndex();
    // 今回のバリアはTransition
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    // Noneにしておく
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    // バリアを張る対象のリソース。現在のバックバッファに対して行う
    barrier.Transition.pResource = swapchain_->GetBuffer(backBufferIndex).Get();
    // 遷移前のResourceState
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    // 遷移後のResourceState
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    // TransitionBarrierを張る
    commandList->ResourceBarrier(1, &barrier);
    // 描画先のRTVとDSVを設定する
    dsvHandle = DsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    // 描画先のRTVを指定する
    commandList->OMSetRenderTargets(1, &rtvHandle, false, &dsvHandle);
    // 指定した色で画面全体をクリアする
    float clearColor[] = { 0.1f,0.25f,0.5f,1.0f };//青っぽい色。RGBAの順
    commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    commandList->RSSetViewports(1, &viewport_->GetViewport());
    commandList->RSSetScissorRects(1, &viewport_->GetScissorRect());
    // 指定した深度で画面全体をクリアする
    commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void DirectXCommon::PostDrow() {
    HRESULT hr;
    // バックバッファの番号を取得
    UINT bbIndex = swapchain_->GetSwapChain()->GetCurrentBackBufferIndex();
    // 画面に描く処理はすべて終わり、画面に移すので、状態を遷移
    // 今回はRenderTargetからPresentにする
    // PostDrow 内で再設定が必要（PreDrawとは別フレームなので）：
    barrier.Transition.pResource = swapchain_->GetBuffer(bbIndex).Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    // TransitionBarrierを張る
    commandList->ResourceBarrier(1, &barrier);
    // コマンドリストの内容を確定させる。全てのコマンドを積んでからCloseすること
    hr = commandList->Close();
    assert(SUCCEEDED(hr));
    if (FAILED(hr)) {
        OutputDebugStringA("commandList->Close() failed!\n");
        // hr を調べる
    }
    // GPUにコマンドリストのリストの実行を行わせる
    ID3D12CommandList* commandLists[] = { commandList.Get() };
    commandQueue->ExecuteCommandLists(1, commandLists);
    // GPUとOSに画面の交換を行うように通知する
    swapchain_->GetSwapChain()->Present(1, 0);
 
    // 描画後のFence
    fence_->SignalAndWait(commandQueue);
 
    //  FPS固定
    fpscontroller_->Update();

    // 次のフレーム用のコマンドリストを準備
    hr = commandAllocator->Reset();
    assert(SUCCEEDED(hr));
    hr = commandList->Reset(commandAllocator.Get(), nullptr);
    assert(SUCCEEDED(hr));
}

ComPtr <ID3D12DescriptorHeap> DirectXCommon::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible){
    //ディスクリプタヒープの生成
    ComPtr <ID3D12DescriptorHeap> descriptorHeap = nullptr;
    D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
    descriptorHeapDesc.Type = heapType;
    descriptorHeapDesc.NumDescriptors = numDescriptors;
    descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
    //ディスクリプタヒープが作れなかったので起動できない
    assert(SUCCEEDED(hr));
    return descriptorHeap;
}