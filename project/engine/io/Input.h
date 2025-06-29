#pragma once
#include<Windows.h>
#include<wrl.h>
#define DIRECTINPUT_VERSION   0x0800 // DirectInputのバージョン指定
#include<dinput.h>
#include "WinApp.h"

// 入力
class Input
{
private:
	static Input* instance;

	Input() = default;
	~Input() = default;
	Input(Input&) = delete;
	Input& operator=(Input&) = delete;
public: // メンバ関数
	// シングルトンインスタンスの取得
	static Input* GetInstance();
	// 終了
	void Finalize();

	// namespace省略
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	// 初期化
	void Initialize(WinApp* winApp);
	// 更新
	void Update();

	/// <summary>
	/// キーの押下をチェック
	/// </summary>
	/// <param name="keyNumber"キー番号( DIK_0 等)</param>
	bool Pushkey(BYTE keyNumber);

	/// <summary>
	/// キーのトリガーをチェック
	/// </summary>
	/// <param name="keyNumber"キー番号( DIK_0 等)</param>
	/// <returns>トリガーか</returns>
	bool Triggrkey(BYTE keyNumber);

	/// <summary>
	/// マウスの座標を取得
	/// </summary>
    POINT GetMousePosition() const { return mousePosition_; }
    
	// ImGuiデバッグ描画
    void DrawImGui();
private: // メンバ変数	
	// ポインタ
	WinApp* winApp_ = nullptr;	
	//キーボードのデバイス
	ComPtr<IDirectInputDevice8> keyboard;
	// 全キーの状態
	BYTE key[256] = {};
	// 前回の全キーの状態
	BYTE keyPre[256] = {};
	// DirectInputのインスタンス生成
	ComPtr<IDirectInput8> dinput = nullptr;
	// マウスのデバイス
    ComPtr<IDirectInputDevice8> mouse;
    // マウスの状態（DirectInput 専用構造体）
    DIMOUSESTATE mouseState = {};
    // 画面上のマウス座標（独自で保持）
    POINT mousePosition_ = { 0, 0 };
	//　マウスを使うかどうか
	bool useMouseInput_ = false;
public: // getter setter
	bool GetMouseInput() { return useMouseInput_; }		
	void SetMouseInput(bool mouse) { useMouseInput_ = mouse; }
};