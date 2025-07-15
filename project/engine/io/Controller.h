#pragma once
#define WIN32_LEAN_AND_MEAN // 不要なヘッダを除外
#include <Windows.h>
#include <XInput.h>
#pragma comment(lib, "xinput.lib") // XInputリンク

// コントローラー
class Controller {
private:    
    // コンストラクタとデストラクタ
    Controller() = default;
    ~Controller() = default;
public:
    // インスタンス取得（シングルトン）
    static Controller* GetInstance();

    /// <summary>
    /// 入力状態を更新
    /// </summary>
    void Update();
     
    /// <summary>
    /// 指定ボタンが押されているかどうか（押されてる間 true）
    /// </summary>
    bool IsPressed(WORD button);
    /// <summary>
    /// 指定ボタンが新しく押されたかどうか（押された瞬間 true）
    /// </summary>
    bool IsTriggered(WORD button);

    // imgui
    void DrawImGui();
    // コントローラーのデットゾーン処理
    float NormalizeThumbAxis(SHORT value, SHORT deadzone);
private: // メンバ変数
    XINPUT_STATE currentState{};     // 現在のゲームパッドの状態
    XINPUT_STATE previousState{};    // 前フレームのゲームパッドの状態
};