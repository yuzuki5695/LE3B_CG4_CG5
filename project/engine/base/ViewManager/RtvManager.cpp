#include "RtvManager.h"
#include<ResourceFactory.h>
#include <DescriptorUtils.h>

using namespace ResourceFactory;
using namespace DescriptorUtils;

void RtvManager::Initialize(DirectXCommon* directXCommon) {
    assert(directXCommon);
    // 引数で受け取ってメンバ変数に記録する 
    this->dxCommon_ = directXCommon;
    // RTV用のヒープでディスクリプタの数は2。RTVはshader内で触るものではないので、ShaderVisibleはfalse
    descriptorHeap = dxCommon_->CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 3, false);
    // DescriptorSizeを取得する
    descriptorSize = dxCommon_->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    /*--------------------------------------------------------------------------------*/
    /*--------------------------レンダーターゲットビューの初期化---------------------------*/
    /*--------------------------------------------------------------------------------*/ 

    // カスタムRenderTarget用のリソース作成
    kRenderTargetClearValue = { 0.1f,0.25f,0.5f,1.0f };
    // [2]にrenderTexture を作る
    renderTextureResource = CreateRenderTextureResource(
        dxCommon_->GetDevice().Get(),
        WinApp::kClientWidth,
        WinApp::kClientHeight,
        DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
        kRenderTargetClearValue
    );

    /*-----------------------------------------------------------*/
    /*--------------------------RTVの設定--------------------------*/
    /*------------------------------------------------------------*/
    
    //RTVの設定
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;//出力結果をSRGB2変換して書き込む
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;//2Dテクスチャとして読み込む
    //ディスクリプタの先頭を取得する
    D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = GetCPUDescriptorHandle(descriptorHeap, descriptorSize, 0);

    for (uint32_t i = 0; i < rtvHandlenum; ++i) {
        rtvHandles[i] = rtvStartHandle;

        ID3D12Resource* target = nullptr;
        if (i < 2) {
            target = dxCommon_->GetSwapChain()->GetBuffer(i).Get(); // 0, 1 は swapChain
        } else {
            target = renderTextureResource.Get(); // 2 は renderTexture
        }

        dxCommon_->GetDevice()->CreateRenderTargetView(target, &rtvDesc, rtvHandles[i]);
        rtvStartHandle.ptr += descriptorSize;
    }
}

void RtvManager::PreDrawRenderTexture() {
        // バリア: SRV → RenderTarget
    if (renderTextureState != RenderTextureState::RenderTarget) {
        TransitionResource(
            renderTextureResource.Get(),
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
            D3D12_RESOURCE_STATE_RENDER_TARGET);
        renderTextureState = RenderTextureState::RenderTarget;
    }
}

void RtvManager::PostDrawRenderTexture() {
    // もし現在のテクスチャの状態が PixelShaderResource（シェーダーリソースビュー）でない場合、
    if (renderTextureState != RenderTextureState::PixelShaderResource) {
        TransitionResource(renderTextureResource.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        renderTextureState = RenderTextureState::PixelShaderResource;  // 現在の状態を記録しておく。
    }
}


void RtvManager::TransitionResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState) {
    // バリアの設定
    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;                      // 今回のバリアはTransition
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;                           // Noneにしておく 
    barrier.Transition.pResource = resource;                                    // バリアを張る対象のリソース。
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;   // 全サブリソースに対してバリアを張る
    barrier.Transition.StateBefore = beforeState;                               // 遷移前のResourceState
    barrier.Transition.StateAfter = afterState;                                 // TransitionBarrierを張る
    // コマンドリストにバリアを追加
    dxCommon_->GetCommandList()->ResourceBarrier(1, &barrier);
}