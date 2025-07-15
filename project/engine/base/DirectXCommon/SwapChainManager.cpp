#include "SwapChainManager.h"
#include <cassert>

using namespace Microsoft::WRL;

void SwapChainManager::Initialize(HWND hwnd, ComPtr<ID3D12CommandQueue>& commandQueue, ComPtr<IDXGIFactory7>& dxgiFactory, uint32_t width, uint32_t height) {
    ///---------------------------------------------------------------------///
    ///--------------SwapChain(スワップチェーン)を設定する----------------------///
    ///---------------------------------------------------------------------///
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
    swapChainDesc.Width = width;//画面の幅。ウィンドウのクライアント領域を同じものにしておく
    swapChainDesc.Height = height;//画面の高さ。ウィンドウのクライアント領域を同じものにしておく
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;//色の形式
    swapChainDesc.SampleDesc.Count = 1;//マルチサンプルしない
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;//描画のターゲットとして利用する
    swapChainDesc.BufferCount = 2;//ダブルバッファ
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;//モニタに移したら、中身居を破棄

    ///---------------------------------------------------------------------///
    ///--------------SwapChain(スワップチェーン)を生成する----------------------///
    ///---------------------------------------------------------------------///
    HRESULT hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue.Get(), hwnd, &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain_.GetAddressOf()));
    assert(SUCCEEDED(hr));
    
    // バックバッファの取得
    // SwapChainからResourceを引っ張ってくる
    for (int i = 0; i < 2; ++i) {
        hr = swapChain_->GetBuffer(i, IID_PPV_ARGS(&resources_[i])); 
        //上手く取得できなければ起動できない
        assert(SUCCEEDED(hr));
    }
}