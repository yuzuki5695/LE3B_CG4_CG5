#include "Controller.h"
#ifdef USE_IMGUI
#include<ImGuiManager.h>
#endif // USE_IMGUI
#include <string>
#include <algorithm>

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
        return; // ←ここで早期リターンしてOK
    }
}

bool Controller::IsPressed(WORD button) {
    return (currentState.Gamepad.wButtons & button) != 0;
}

bool Controller::IsTriggered(WORD button) {
    return !(previousState.Gamepad.wButtons & button) &&
        (currentState.Gamepad.wButtons & button);
}

void Controller::DrawImGui() {
#ifdef USE_IMGUI
    const XINPUT_GAMEPAD& pad = currentState.Gamepad;  // ゲームパッドの現在の状態を参照

    ImGui::Begin("Controller Debug");
    
    // トリガー（アナログ）
    float leftTrigger = pad.bLeftTrigger / 255.0f;
    float rightTrigger = pad.bRightTrigger / 255.0f;
    ImGui::Text("Left Trigger:  %.2f", leftTrigger);
    ImGui::ProgressBar(leftTrigger, ImVec2(300, 0));
    ImGui::Text("Right Trigger: %.2f", rightTrigger);
    ImGui::ProgressBar(rightTrigger, ImVec2(300, 0));

    // スティック（アナログ）
    float lx = NormalizeThumbAxis(pad.sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
    float ly = NormalizeThumbAxis(pad.sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
    float rx = NormalizeThumbAxis(pad.sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
    float ry = NormalizeThumbAxis(pad.sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);

    // スティック座標をテキストで表示
    ImGui::Separator();
    ImGui::Text("Left Stick:  (X = %.2f, Y = %.2f)", lx, ly);
    ImGui::Text("Right Stick: (X = %.2f, Y = %.2f)", rx, ry);

    // 十字キーとボタンの状態
    ImGui::Separator();
    ImGui::Text("Buttons Pressed:");
    
    // ボタンのビットフラグ
    struct ButtonInfo {
        WORD flag;
        const char* name;
    };

    static ButtonInfo buttons[] = {
        { XINPUT_GAMEPAD_A, "A" },
        { XINPUT_GAMEPAD_B, "B" },
        { XINPUT_GAMEPAD_X, "X" },
        { XINPUT_GAMEPAD_Y, "Y" },
        { XINPUT_GAMEPAD_DPAD_UP, "DPad Up" },
        { XINPUT_GAMEPAD_DPAD_DOWN, "DPad Down" },
        { XINPUT_GAMEPAD_DPAD_LEFT, "DPad Left" },
        { XINPUT_GAMEPAD_DPAD_RIGHT, "DPad Right" },
        { XINPUT_GAMEPAD_LEFT_SHOULDER, "L Shoulder" },
        { XINPUT_GAMEPAD_RIGHT_SHOULDER, "R Shoulder" },
        { XINPUT_GAMEPAD_START, "Start" },
        { XINPUT_GAMEPAD_BACK, "Back" },
        { XINPUT_GAMEPAD_LEFT_THUMB, "L Stick Press" },
        { XINPUT_GAMEPAD_RIGHT_THUMB, "R Stick Press" },
    };
    // 各ボタンについて、押されていれば表示
    for (const auto& b : buttons) {
        if (pad.wButtons & b.flag) {
            ImGui::SameLine(); // 横並びに表示
            ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "[%s]", b.name);
        }
    }
    ImGui::End();
#endif // USE_IMGUI
}

float Controller::NormalizeThumbAxis(SHORT value, SHORT deadzone) {
    if (abs(value) < deadzone) {
        return 0.0f;
    }
    float norm = (float)value / 32767.0f;
    return std::clamp(norm, -1.0f, 1.0f);
}