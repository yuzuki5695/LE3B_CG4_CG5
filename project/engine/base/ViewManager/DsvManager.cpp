#include "DsvManager.h"
#include<ResourceFactory.h>
#include <DescriptorUtils.h>

using namespace ResourceFactory;
using namespace DescriptorUtils;

void DsvManager::Initialize(DirectXCommon* directXCommon) { 
    assert(directXCommon);
    // 引数で受け取ってメンバ変数に記録する 
    this->dxCommon_ = directXCommon;

    // DSV用のヒープでディスクリプタの数は1。DSVはshader内で触るものではないので、ShaderVisibleはfalse
    dsvDescriptorHeap = dxCommon_->CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);
    // DescriptorSizeを取得する
    descriptorsizeDSV = dxCommon_->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

    //------------------深度ステルスビューの初期化---------------------//
    /*------------------------------------------------------------*/
    /*--------------------------DSVの設定--------------------------*/
    /*------------------------------------------------------------*/
    // DepthStencilTextureをウインドウのサイズで作成
    depthStencilResource = CreateDepthStencilTextureResource(dxCommon_->GetDevice(), WinApp::kClientWidth, WinApp::kClientHeight);

    // DSVの設定
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//Format。基本的にはResource合わせる
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D; //2dTexture 
    // DSVDescの先頭にDSVを作る
    dxCommon_->GetDevice()->CreateDepthStencilView(depthStencilResource.Get(), &dsvDesc, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

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

void DsvManager::PreDraw() {
    if (depthState != DSVState::DepthWrite) {
        TransitionResource(
            depthStencilResource.Get(),
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        depthState = DSVState::DepthWrite;
    }
}
void DsvManager::PostDraw() {
    if (depthState != DSVState::PixelShaderResource) {
        TransitionResource(
            depthStencilResource.Get(),
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
            D3D12_RESOURCE_STATE_DEPTH_WRITE);
        depthState = DSVState::PixelShaderResource;
    }
}

void DsvManager::TransitionResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState) {
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