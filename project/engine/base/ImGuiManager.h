#pragma once
#include<SrvManager.h>
#ifdef USE_IMGUI
#include<externals/imgui/imgui_impl_dx12.h>
#include<externals/imgui/imgui_impl_win32.h>
#endif // USE_IMGUI

class WinApp;
class DirectXCommon;

// ImGUIの管理
class ImGuiManager
{
private:
	static ImGuiManager* instance;

	ImGuiManager() = default;
	~ImGuiManager() = default;
	ImGuiManager(ImGuiManager&) = delete;
	ImGuiManager& operator=(ImGuiManager&) = delete;
public: // メンバ関数
	// シングルトンインスタンスの取得
	static ImGuiManager* GetInstance();
	///<summary>
	/// 初期化
	///</summary>
	// 初期化
	void Initialize(WinApp* winApp, DirectXCommon* DxCommon, SrvManager* srvManager);

	/// <summary>
	/// 終了
	/// </summary>
	void Finalize();

	/// <summary>
	/// ImGui受付開始
	/// </summary>
	void Begin();

	/// <summary>
	/// ImGui受付終了
	/// </summary>
	void End();

	/// <summary>
	/// 画面への描画
	/// </summary>
	void Draw();

private: // メンバ変数
	// ポインタ
	WinApp* winApp_;
	DirectXCommon* DxCommon_;
	SrvManager* srvManager_;
	// imgui番号
	uint32_t imguiindex;
	// ハンドル
	D3D12_CPU_DESCRIPTOR_HANDLE ImGuiHandleCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE ImGuiHandleGPU;
};