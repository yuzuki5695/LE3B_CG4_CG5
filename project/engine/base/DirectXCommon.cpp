#include "DirectXCommon.h"
#include<cassert>
#include<format>
#include <thread>
#include "Logger.h"
#include "StringUtility.h"
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

using namespace Microsoft::WRL;

DirectXCommon::~DirectXCommon() {
    // Win32APIの開放
    CloseHandle(fenceEvent);
}

void DirectXCommon::Initialize(WinApp* winApp){
    // FPS固定初期化
    InitializeFizFPS();

    // NULL検出
    assert(winApp);
    // メンバ変数に記録
    this->winApp_ = winApp;

	// デバイスの初期化
	DebugInitialize();
	// コマンド関連の初期化
	CommandInitialize();
	// スワップチェーンの生成
	SwapChainGenerate();
	// 深度バッファの生成
	CreateDepthStencilGenerate();
	// 各種でスクリプタヒープの生成
	DescriptorHeapGenerate();
	// レンダーターゲットビューの初期化
	RenderviewInitialize();
	// 深度ステルスビューの初期化
	DepthstealthviewInitialization();
	// フェンスの初期化
	FenceInitialize();
	// ビューポートの初期化
	viewportInitialize();
	// シザリング矩形
	scissorRectInitialize();
	// DXCコンパイラの生成
	DxCompilerGenerate();
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

void DirectXCommon::SwapChainGenerate() {

    HRESULT hr;

    ///---------------------------------------------------------------------///
    ///--------------SwapChain(スワップチェーン)を設定する----------------------///
    ///---------------------------------------------------------------------///
    swapChainDesc.Width = WinApp::kClientWidth;//画面の幅。ウィンドウのクライアント領域を同じものにしておく
    swapChainDesc.Height = WinApp::kClientHeight;//画面の高さ。ウィンドウのクライアント領域を同じものにしておく
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;//色の形式
    swapChainDesc.SampleDesc.Count = 1;//マルチサンプルしない
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;//描画のターゲットとして利用する
    swapChainDesc.BufferCount = 2;//ダブルバッファ
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;//モニタに移したら、中身居を破棄
    
    ///---------------------------------------------------------------------///
    ///--------------SwapChain(スワップチェーン)を生成する----------------------///
    ///---------------------------------------------------------------------///
    hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue.Get(), winApp_->Gethwnd(), &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain.GetAddressOf()));
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

void DirectXCommon::DescriptorHeapGenerate() {
    /*----------------------------------------------------------------------------*/
    /*---------------------------DescriptorHeap-----------------------------------*/
    /*----------------------------------------------------------------------------*/

    // RTV用のヒープでディスクリプタの数は2。RTVはshader内で触るものではないので、ShaderVisibleはfalse
    rtvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 3, false);
    // DSV用のヒープでディスクリプタの数は1。DSVはshader内で触るものではないので、ShaderVisibleはfalse
    dsvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);

    // DescriptorSizeを取得する
    descriptorsizeRTV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    descriptorsizeDSV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

void DirectXCommon::RenderviewInitialize() {

    HRESULT hr;

    //SwapChainからResourceを引っ張ってくる
    hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0]));
    //上手く取得できなければ起動できない
    assert(SUCCEEDED(hr));
    hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));
    assert(SUCCEEDED(hr));
    
    /*-----------------------------------------------------------*/
    /*--------------------------RTVの設定--------------------------*/
    /*------------------------------------------------------------*/
    
    //----------------カスタムの設定-----------//
    // RTVの設定
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;//出力結果をSRGB2変換して書き込む
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;//2Dテクスチャとして読み込む
    // ディスクリプタの先頭を取得する
    rtvStartHandle = GetCPUDescriptorHandle(rtvDescriptorHeap, descriptorsizeRTV, 0);

    // カスタムRenderTarget用のリソース作成（赤でクリアされる）
    const Vector4 kRenderTargetClearValue{ 1.0f, 0.0f, 0.0f, 1.0f }; // 赤

    ComPtr <ID3D12Resource>  renderTextureResource = CreateRenderTextureResource(
        device,
        WinApp::kClientWidth,
        WinApp::kClientHeight,
        DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
        kRenderTargetClearValue
    );

    // ハンドルの数だけ作成する
    for (uint32_t i = 0; i < rtvHandlenum; ++i) {
        // ハンドルを設定
        rtvHandles[i] = rtvStartHandle;
        if (i == 2) {
            // レンダーターゲットリソースの場合
            // swapChainResources[2] にカスタムRenderTextureリソースを代入
            swapChainResources[2] = renderTextureResource;
        }
        // RTVを作成 
        device->CreateRenderTargetView(swapChainResources[i].Get(), &rtvDesc, rtvHandles[i]);
        // 次のハンドルに進む
        rtvStartHandle.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    }
}


void DirectXCommon::DepthstealthviewInitialization() {

    /*------------------------------------------------------------*/
    /*--------------------------DSVの設定--------------------------*/
    /*------------------------------------------------------------*/

    // DepthStencilTextureをウインドウのサイズで作成
    depthStencilResource = CreateDepthStencilTextureResource(device,WinApp::kClientWidth, WinApp::kClientHeight);

    // DSVの設定
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//Format。基本的にはResource合わせる
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D; //2dTexture 
    // DSVDescの先頭にDSVを作る
    device->CreateDepthStencilView(depthStencilResource.Get(), &dsvDesc, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

    // DepthStencilStateの設定
    // Depthの機能を有効化する
    depthStencilDesc.DepthEnable = true;
    // 書き込みする
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    // 書き込みしない
    //depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    // 比較関数はLessEqual。つまり、近ければ描画される
    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
}

void DirectXCommon::FenceInitialize() {

    HRESULT hr;

    //初期値0でFenceを作る
    hr = device->CreateFence(fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
    assert(SUCCEEDED(hr));
    //FenceのSignalを待つためのイベントを作成する
    assert(fenceEvent != nullptr);
}

void DirectXCommon::viewportInitialize() {
    //クライアント領域のサイズと一緒にして画面全体に表示
    viewport.Width = WinApp::kClientWidth;
    viewport.Height = WinApp::kClientHeight;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
}

void DirectXCommon::scissorRectInitialize() {
    // 基本的にビューポートと同じ矩形が構成されるようにする
    scissorRect.left = 0;
    scissorRect.right = WinApp::kClientWidth;
    scissorRect.top = 0;
    scissorRect.bottom = WinApp::kClientHeight;
}

void DirectXCommon::DxCompilerGenerate() {

    HRESULT hr;

    // dxCompilerを初期化
    hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
    assert(SUCCEEDED(hr));
    hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
    assert(SUCCEEDED(hr));
    //現時点でincludeはしないが、includeに対応するための設定を行っていく
    hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
    assert(SUCCEEDED(hr));
}

void DirectXCommon::PreDraw() {
    // ここから書き込むバックバッファのインデックスを取得
    UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();
    // 今回のバリアはTransition
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    // Noneにしておく
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    // バリアを張る対象のリソース。現在のバックバッファに対して行う
    barrier.Transition.pResource = swapChainResources[backBufferIndex].Get();
    // 遷移前のResourceState
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    // 遷移後のResourceState
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    // TransitionBarrierを張る
    commandList->ResourceBarrier(1, &barrier);
    // 描画先のRTVとDSVを設定する
    dsvHandle = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    commandList->RSSetViewports(1, &viewport);
    commandList->RSSetScissorRects(1, &scissorRect);

    // 描画先のRTVを指定する
    commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, &dsvHandle);
    // 指定した深度で画面全体をクリアする
    commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // 指定した色で画面全体をクリアする
    float clearColor[] = { 0.1f,0.25f,0.5f,1.0f };//青っぽい色。RGBAの順
    commandList->ClearRenderTargetView(rtvHandles[backBufferIndex], clearColor, 0, nullptr);
}


void DirectXCommon::PostDrow() {
    HRESULT hr;
    // バックバッファの番号を取得
    UINT bbIndex = swapChain->GetCurrentBackBufferIndex();
    // 画面に描く処理はすべて終わり、画面に移すので、状態を遷移
    // 今回はRenderTargetからPresentにする
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    // TransitionBarrierを張る
    commandList->ResourceBarrier(1, &barrier);
    // コマンドリストの内容を確定させる。全てのコマンドを積んでからCloseすること
    hr = commandList->Close();
    assert(SUCCEEDED(hr));
    // GPUにコマンドリストのリストの実行を行わせる
    ID3D12CommandList* commandLists[] = { commandList.Get() };
    commandQueue->ExecuteCommandLists(1, commandLists);
    // GPUとOSに画面の交換を行うように通知する
    swapChain->Present(1, 0);
    // Fenceの値の更新
    fenceVal++;
    // GPUがここまでたどり着いたときに、Fenceの値に代入するようにSignalを送る
    commandQueue->Signal(fence.Get(), fenceVal);
    // Fenceの値が指定したSignal値にたどり着いているか確認する
    // GetCompletedValueの初期値はFence作成時に渡した初期値
    if (fence->GetCompletedValue() < fenceVal)
    {
        // 指定したSignalにたどりついていないので、たどり着くまで待つようにイベントを設定する
        fence->SetEventOnCompletion(fenceVal, fenceEvent);
        //イベントを待つ
        WaitForSingleObject(fenceEvent, INFINITE);
    }
    
    //  FPS固定
    void UpdateFixFPS();

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

D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetCPUDescriptorHandle(Microsoft::WRL::ComPtr <ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorsize, uint32_t index) {

    D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
    handleCPU.ptr += (descriptorsize * index);
    return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetGPUDescriptorHandle(Microsoft::WRL::ComPtr <ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorsize, uint32_t index) {
    D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
    handleGPU.ptr += (descriptorsize * index);
    return handleGPU;
}

ComPtr<ID3D12Resource> DirectXCommon::CreateDepthStencilTextureResource(ComPtr <ID3D12Device>& device, int32_t width, int32_t heigth) {
    // 生成するResourceの設定
    D3D12_RESOURCE_DESC resourceDesc{};
    resourceDesc.Width = width; // Textureの幅
    resourceDesc.Height = heigth; // Textureの高さ
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
    ComPtr <ID3D12Resource> resource = nullptr;
    HRESULT hr = device->CreateCommittedResource(
        &heapProperies, //Heapの設定
        D3D12_HEAP_FLAG_NONE, //Heapの特殊な設定。特になし。
        &resourceDesc, //Resourceの設定
        D3D12_RESOURCE_STATE_DEPTH_WRITE, //深度値を書き込む状態にしておく
        &depthClearValue, //Clear最適値
        IID_PPV_ARGS(&resource)); //作成するResourceポインタへのポインタ
    assert(SUCCEEDED(hr));
    return resource;
}


// コンパイルシェーダー
ComPtr <IDxcBlob> DirectXCommon::CompileShader(const std::wstring& filePath,const wchar_t* profile) {
    //1.hlslファイルを読む
    //これからシェーダーをコンパイルする旨をログに出す
    Logger::Log(StringUtility::ConvertString(std::format(L"Begin CompileShader,path:{},profile:{}\n", filePath, profile)));
    Microsoft::WRL::ComPtr <IDxcBlobEncoding> shaderSource = nullptr;
    HRESULT hr = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);
    //読めなかったら止める
    assert(SUCCEEDED(hr));

    //読み込んだファイルの内容を設定する
    DxcBuffer shaderSourceBuffer;
    shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
    shaderSourceBuffer.Size = shaderSource->GetBufferSize();
    shaderSourceBuffer.Encoding = DXC_CP_UTF8;//UTF8のコードであることを通知

    //2.Compileする
    LPCWSTR arguments[] =
    {

         filePath.c_str(),
         L"-E",L"main",
         L"-T",profile,
         L"-Zi",L"-Qembed_debug",
         L"-Od",
         L"-Zpr",
    };
    //実際にshaderをコンパイルする
    ComPtr <IDxcResult> shaderResult = nullptr;
    hr = dxcCompiler->Compile(
        &shaderSourceBuffer,
        arguments,
        _countof(arguments),
        includeHandler.Get(),
        IID_PPV_ARGS(&shaderResult)
    );
    assert(SUCCEEDED(hr));

    //警告・エラーが出てたらログを出して止める
    ComPtr <IDxcBlobUtf8> shaderError = nullptr;
    shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
    if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
        Logger::Log(shaderError->GetStringPointer());
        assert(false);
    }
    //コンパイル結果から実行用のバイナリ部分を取得
    Microsoft::WRL::ComPtr <IDxcBlob> shaderBlob = nullptr;
    hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
    assert(SUCCEEDED(hr));
    //成功したログを出す
    Logger::Log(StringUtility::ConvertString(std::format(L"Compile Succeeded,path:{},profile:{}\n", filePath, profile)));
    //実行用のバイナリを返却
    return shaderBlob;
}

// Resourceの関数化
ComPtr <ID3D12Resource> DirectXCommon::CreateBufferResource(size_t sizeInBytes) {
    //IDXGIのファクトリーの生成
    ComPtr <IDXGIFactory7> dxgiFactory = nullptr;
    HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
    //頂点リソース用のヒープの設定
    D3D12_HEAP_PROPERTIES uploadHeapProperties{};
    uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
    //頂点リソースの設定
    D3D12_RESOURCE_DESC vertexResourceDesc{};
    //バッファリソース、テクスチャの場合はまた別の設定をする
    vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    vertexResourceDesc.Width = sizeInBytes;
    //バッファの場合はこれらは１にする
    vertexResourceDesc.Height = 1;
    vertexResourceDesc.DepthOrArraySize = 1;
    vertexResourceDesc.MipLevels = 1;
    vertexResourceDesc.SampleDesc.Count = 1;
    //バッファの場合はこれにする
    vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    //実際に頂点リソースを作る
    ComPtr <ID3D12Resource> Resource = nullptr;
    hr = device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
        &vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&Resource));
    assert(SUCCEEDED(hr));
    return Resource;
};

//DirectX12のTextureResourceを作る
ComPtr <ID3D12Resource> DirectXCommon::CreateTextureResource(const ComPtr <ID3D12Device> &device, const DirectX::TexMetadata& metadata)
{
    //1. metadataを基にResourceの設定
    D3D12_RESOURCE_DESC resourceDesc{};
    resourceDesc.Width = UINT(metadata.width);									//Textureの幅
    resourceDesc.Height = UINT(metadata.height);								//Textureの高さ
    resourceDesc.MipLevels = UINT16(metadata.mipLevels);						//mipmapの数
    resourceDesc.DepthOrArraySize = UINT16(metadata.arraySize);					//奥行 or 配列Textureの配列行数
    resourceDesc.Format = metadata.format;										//TextureのFormat
    resourceDesc.SampleDesc.Count = 1;											//サンプリングカウント。1固定
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension);		//Textureの次元数。普段使っているのは二次元

    //2. 利用するHeapの設定。非常に特殊な運用。02_04exで一般的なケース版がある
    D3D12_HEAP_PROPERTIES heapProperties{};
    heapProperties.Type = D3D12_HEAP_TYPE_CUSTOM;								//細かい設定を行う
    heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;		//WriteBackポリシーでCPUアクセス可能
    heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;					//プロセッサの近くに配膳

    //3. Resourceを生成する
    ComPtr <ID3D12Resource> resource = nullptr;
    HRESULT hr = device->CreateCommittedResource(
        &heapProperties,														//Heapの設定
        D3D12_HEAP_FLAG_NONE,													//Heapの特殊な設定。特になし。
        &resourceDesc,															///Resourceの設定
        D3D12_RESOURCE_STATE_GENERIC_READ,										//初回のResourceState。Textureは基本読むだけ
        nullptr,																//Clear最適値。使わないのでnullptr
        IID_PPV_ARGS(&resource));												//作成するResourceポインタへのポインタ
    assert(SUCCEEDED(hr));
    return resource;
}

//TextureResourceにデータを移送する
void DirectXCommon::UploadTextureData(ComPtr <ID3D12Resource> &texture, const DirectX::ScratchImage& mipImages)
{
    //Meta情報を取得
    const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
    //全MipMapについて
    for (size_t mipLevel = 0; mipLevel < metadata.mipLevels; ++mipLevel)
    {
        //MipMapLevelを指定して各Imageを取得
        const DirectX::Image* img = mipImages.GetImage(mipLevel, 0, 0);
        //Textureに転送
        HRESULT hr = texture->WriteToSubresource(
            UINT(mipLevel),
            nullptr,				//全領域へコピー
            img->pixels,			//元データアドレス
            UINT(img->rowPitch),	//1ラインサイズ
            UINT(img->slicePitch)	//1枚サイズ
        );
        assert(SUCCEEDED(hr));
    }
}

void DirectXCommon::InitializeFizFPS() {
    // 現在時間を記録する
    reference_ = std::chrono::steady_clock::now();
}


void DirectXCommon::UpdateFixFPS() {

    // 1/60秒ぴったりの時間
    const std::chrono::microseconds kMinTime(uint64_t(1000000.0f / 60.0f));
    // 1/60秒よりわずかに短い時間
    const std::chrono::microseconds kMinCheckTime(uint64_t(1000000.0f / 65.0f));

    // 現在時間を取得する
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    // 前回記録からの経過時間を取得する
    std::chrono::microseconds elapsed =
        std::chrono::duration_cast<std::chrono::microseconds>(now - reference_);

    // 1/60秒 (よりわずかに短い時間)　経っていない場合
    if (elapsed < kMinCheckTime) {
        // 1/60秒経過するまで微小なスリープを繰り返す
        while (std::chrono::steady_clock::now() - reference_ < kMinTime) {
            // 1マイクロ秒スリープ
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        }
    }
    // 現在時間を記録する
    reference_ = std::chrono::steady_clock::now();
}

ComPtr <ID3D12Resource> DirectXCommon::CreateRenderTextureResource(Microsoft::WRL::ComPtr <ID3D12Device> device, uint32_t width, uint32_t height, DXGI_FORMAT format, const Vector4& clearColor) {
    //1. metadataを基にResourceの設定
    D3D12_RESOURCE_DESC resourceDesc{};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resourceDesc.Alignment = 0;
    resourceDesc.Width = width;
    resourceDesc.Height = height;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.Format = format;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    //2. 利用するHeapの設定。非常に特殊な運用。02_04exで一般的なケース版がある
    D3D12_HEAP_PROPERTIES heapProperties{};
    heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;								// 当然VRAM上に作る

    D3D12_CLEAR_VALUE clearValue;
    clearValue.Format = format;
    clearValue.Color[0] = clearColor.x;
    clearValue.Color[1] = clearColor.y;
    clearValue.Color[2] = clearColor.z;
    clearValue.Color[3] = clearColor.w;

    //3. Resourceを生成する
    ComPtr <ID3D12Resource> resource = nullptr;
    HRESULT hr = device->CreateCommittedResource(
        &heapProperties,														//Heapの設定
        D3D12_HEAP_FLAG_NONE,													//Heapの特殊な設定。特になし。
        &resourceDesc,															///Resourceの設定
        D3D12_RESOURCE_STATE_RENDER_TARGET,										//これから描画することを前提としたTextureなのでRenderTargetとして使うことから始める
        &clearValue,															//Clear最適地,ClearRenderTargetをこの色でClearするようにする。最適化されれいるので高速である
        IID_PPV_ARGS(&resource));												//作成するResourceポインタへのポインタ
    assert(SUCCEEDED(hr));
    return resource;
}
