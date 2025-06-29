#include "Controller.h"

Controller* Controller::GetInstance() {
    // インスタンス取得
    static Controller instance;
    return &instance;
}

void Controller::Update() {
    // 前回の状態を保存
    previousState = currentState;

    // 現在の状態を取得
    ZeroMemory(&currentState, sizeof(XINPUT_STATE));
    DWORD result = XInputGetState(0, &currentState);  // 0番目のコントローラーの状態を取得

    // コントローラーが接続されていない場合、状態をゼロクリア
    if (result != ERROR_SUCCESS) {
        ZeroMemory(&currentState, sizeof(XINPUT_STATE));
    }
}

bool Controller::IsPressed(WORD button) {
    return (currentState.Gamepad.wButtons & button) != 0;
}

bool Controller::IsTriggered(WORD button) {
    return !(previousState.Gamepad.wButtons & button) &&
           (currentState.Gamepad.wButtons & button);
}
