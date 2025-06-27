#include "FenceManager.h"
#include <cassert>

using namespace Microsoft::WRL;

FenceManager::~FenceManager() {
    if (fenceEvent_) {
        // Win32APIの開放
        CloseHandle(fenceEvent_);        
        fenceEvent_ = nullptr;
    }
}

void FenceManager::Initialize(ComPtr <ID3D12Device>& device) {
	// フェンスの初期化	  
    HRESULT hr;
    //初期値0でFenceを作る
    hr = device->CreateFence(fenceVal_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
    assert(SUCCEEDED(hr));
    //FenceのSignalを待つためのイベントを作成する
    assert(fenceEvent_ != nullptr);
}


void FenceManager::SignalAndWait(ComPtr <ID3D12CommandQueue>& commandQueue) {    
    // Fenceの値の更新
    fenceVal_++;
    // GPUがここまでたどり着いたときに、Fenceの値に代入するようにSignalを送る
    commandQueue->Signal(fence_.Get(), fenceVal_);
    // Fenceの値が指定したSignal値にたどり着いているか確認する
    // GetCompletedValueの初期値はFence作成時に渡した初期値
    if (fence_->GetCompletedValue() < fenceVal_) {

        // 指定したSignalにたどりついていないので、たどり着くまで待つようにイベントを設定する
        fence_->SetEventOnCompletion(fenceVal_, fenceEvent_);
        //イベントを待つ
        WaitForSingleObject(fenceEvent_, INFINITE);
    }
}