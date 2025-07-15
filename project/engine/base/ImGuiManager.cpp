#include "ImGuiManager.h"
#include<WinApp.h>
#include<DirectXCommon.h>

using namespace Microsoft::WRL;

ImGuiManager* ImGuiManager::instance = nullptr;

ImGuiManager* ImGuiManager::GetInstance() {
	if (instance == nullptr) {
		instance = new ImGuiManager;
	}
	return instance;
}

void ImGuiManager::Finalize() {
#ifdef USE_IMGUI
	// ImGuiの終了処理。後始末
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif // USE_IMGUI
	delete instance;
	instance = nullptr;
}

void ImGuiManager::Initialize([[maybe_unused]] WinApp* winApp, [[maybe_unused]] DirectXCommon* DxCommon, [[maybe_unused]] SrvManager* srvManager) {
#ifdef USE_IMGUI
	// 引数で受け取ってメンバ変数に記録する
	this->winApp_ = winApp;
	this->DxCommon_ = DxCommon;
	this->srvManager_ = srvManager;

	// ImGuiのコンテキストを生成
	ImGui::CreateContext();
	// ImGuiのスタイルを設定
	ImGui::StyleColorsDark();

	// インデックスを確保
	imguiindex = srvManager_->Allocate();
	ImGuiHandleCPU = srvManager_->GetCPUDescriptorHandle(imguiindex);
	ImGuiHandleGPU = srvManager_->GetGPUDescriptorHandle(imguiindex);

	// Win32用初期化
	ImGui_ImplWin32_Init(winApp_->Gethwnd());
	// DirectX12用初期化
	ImGui_ImplDX12_Init(
		DxCommon_->GetDevice().Get(),
		static_cast<int>(DxCommon_->GetSwapChain()->GetResources().size()),
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
		srvManager_->GetDescriptorHeap().Get(),
		ImGuiHandleCPU,
		ImGuiHandleGPU
	);
#endif // USE_IMGUI
}

void ImGuiManager::Begin() {
#ifdef USE_IMGUI
	// ImGuiフレーム開始
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
#endif // USE_IMGUI
}

void ImGuiManager::End() {
#ifdef USE_IMGUI
	// 描画前準備
	ImGui::Render();
#endif // USE_IMGUI
}

void ImGuiManager::Draw() {
#ifdef USE_IMGUI
	ComPtr<ID3D12GraphicsCommandList> commandList = DxCommon_->GetCommandList();
	// デスクリプタヒープの配列をセットするコマンド
	ComPtr<ID3D12DescriptorHeap> ppheaps[] = { srvManager_->GetDescriptorHeap().Get() };
	commandList->SetDescriptorHeaps(_countof(ppheaps), ppheaps->GetAddressOf());
	// 描画コマンドを発行
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList.Get());
#endif // USE_IMGUI
}