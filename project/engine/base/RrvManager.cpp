#include "RrvManager.h"
#include<ResourceFactory.h>
#include <DescriptorUtils.h>

using namespace ResourceFactory;
using namespace DescriptorUtils;

void RrvManager::Initialize(DirectXCommon* directXCommon) {
    assert(directXCommon); 	
   	// 引数で受け取ってメンバ変数に記録する 
    this->directXCommon = directXCommon;
    // RTV用のヒープでディスクリプタの数は2。RTVはshader内で触るものではないので、ShaderVisibleはfalse
    descriptorHeap = directXCommon->CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 3, false);
    // DescriptorSizeを取得する
    descriptorSize = directXCommon->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
 
    // カスタムRenderTarget用のリソース作成
    kRenderTargetClearValue = { 0.1f,0.25f,0.5f,1.0f };
    // [2]にrenderTexture を作る
    renderTextureResource = CreateRenderTextureResource(
        directXCommon->GetDevice(),
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
            target = directXCommon->GetSwapChain()->GetBuffer(i).Get(); // 0, 1 は swapChain
        } else {
            target = renderTextureResource.Get(); // 2 は renderTexture
        }

        directXCommon->GetDevice()->CreateRenderTargetView(target, &rtvDesc, rtvHandles[i]);
        rtvStartHandle.ptr += descriptorSize;
    }
    dsvHandle = directXCommon->GetDsvHandle();
}

void RtvManager::PreDrawRenderTexture() {
    if (renderTextureState != RenderTextureState::RenderTarget) {
        TransitionResource(
            renderTextureResource.Get(),
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
            D3D12_RESOURCE_STATE_RENDER_TARGET);
        renderTextureState = RenderTextureState::RenderTarget;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE rtHandle = rtvHandles[2];

    dsvHandle= directXCommon->GetDsvDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();
    
    directXCommon->GetCommandList()->OMSetRenderTargets(1, &rtHandle, false, &dsvHandle);

    float clearColor[] = { kRenderTargetClearValue.x, kRenderTargetClearValue.y, kRenderTargetClearValue.z, kRenderTargetClearValue.w };
     directXCommon->GetCommandList()->ClearRenderTargetView(rtHandle, clearColor, 0, nullptr);
}
