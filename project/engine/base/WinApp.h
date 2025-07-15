#pragma once
#include<Windows.h>
#include<cstdint>
#include <string>

// WindowsAPI
class WinApp
{
public: // 静的メンバ関数
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

public: // メンバ関数
	// デストラクタ
	~WinApp() { Finalize(); }
	// 初期化
	void Initialize();
	// 終了
	void Finalize();
	// メッセージ処理
	bool ProcessMessage();
public: // 定数
	//クライアント領域のサイズ
	static const int32_t kClientWidth = 1280;
	static const int32_t kClientHeight = 720;
private: // メンバ変数
	// ウィンドウハンドル
	HWND hwnd = nullptr;
	// ウィンドウクラスの設定
	WNDCLASS wc{};
	std::wstring windowTitle_ = L"LE3B"; // ← デフォルト値
public:
	// getter
	HWND Gethwnd() const { return hwnd; }
	HINSTANCE GetHInstance() const { return wc.hInstance; }
	void SetWindowTitle(const std::wstring& title);
};