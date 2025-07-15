#pragma once
#include<d3d12.h>
#include<dxgi1_6.h>
#include<wrl.h>

// フェンスマネージャ
class FenceManager
{
public: // メンバ関数
	~FenceManager();
    /// <summary>
    /// 初期化
    /// </summary>
    void Initialize(Microsoft::WRL::ComPtr <ID3D12Device>& device);
	// 描画後のFenceの値    
	void SignalAndWait(Microsoft::WRL::ComPtr <ID3D12CommandQueue>& commandQueue);
private: // メンバ変数
	// フェンスの生成
	Microsoft::WRL::ComPtr <ID3D12Fence> fence_ = nullptr;
	// 初期値0でFenceを作る
	UINT64 fenceVal_ = 0;
	//FenceのSignalを待つためのイベントを作成する
	HANDLE fenceEvent_ = CreateEvent(NULL, FALSE, FALSE, NULL);	
public: // getter
	const Microsoft::WRL::ComPtr <ID3D12Fence>& GetFence() const { return fence_; }
    const HANDLE& GetfenceEvent() const { return fenceEvent_; }
};