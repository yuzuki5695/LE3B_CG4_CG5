#include "Input.h"
#ifdef USE_IMGUI
#include<ImGuiManager.h>
#endif // USE_IMGUI
#include<cassert>
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

Input* Input::instance = nullptr;

Input* Input::GetInstance() {
	if (instance == nullptr) {
		instance = new Input;
	}
	return instance;
}

void Input::Finalize() {
	delete instance;
	instance = nullptr;
}

void Input::Initialize(WinApp* winApp) {

	// 借りてきたwinAppのインスタンスを記録
	this->winApp_ = winApp;

	HRESULT result;
	// DirectInputのインスタンス生成
	result = DirectInput8Create(winApp->GetHInstance(), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&dinput, nullptr);
	assert(SUCCEEDED(result));

	// キーボ－ドデバイス生成
	result = dinput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
	assert(SUCCEEDED(result));
	// 入力データの形式のセット
	result = keyboard->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(result));
	// 排他制御レベルのセット
	result = keyboard->SetCooperativeLevel(winApp->Gethwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));

	// マウスデバイス生成
	result = dinput->CreateDevice(GUID_SysMouse, &mouse, nullptr);
	assert(SUCCEEDED(result));
	// データフォーマットのセット
	result = mouse->SetDataFormat(&c_dfDIMouse);
	assert(SUCCEEDED(result));
	// 排他制御レベルのセット
	result = mouse->SetCooperativeLevel(winApp->Gethwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	assert(SUCCEEDED(result));
}

void Input::Update() {
	// 前回のキー入力を保存
	memcpy(keyPre, key, sizeof(keyPre));
	// キーボード情報の取得開始
	keyboard->Acquire();
	// 全キ-の入力状態を取得する
	keyboard->GetDeviceState(sizeof(key), key);

	// マウスを
	if (useMouseInput_) {
		// マウス取得
		mouse->Acquire();
		mouse->GetDeviceState(sizeof(mouseState), &mouseState);
		// マウス座標を加算更新（移動量）
		mousePosition_.x += mouseState.lX;
		mousePosition_.y += mouseState.lY;
		// 画面外に出ないよう制限（ウィンドウ内クリップ）
		mousePosition_.x = max(0, min(mousePosition_.x, (int)WinApp::kClientWidth));
		mousePosition_.y = max(0, min(mousePosition_.y, (int)WinApp::kClientHeight));
	}
}

bool Input::Pushkey(BYTE keyNumber)
{
	// 指定キーを押していればtrueを返す
	if (key[keyNumber]) {
		return true;
	}
	// そうでなければfalseを返す
	return false;
}

bool Input::Triggrkey(BYTE keyNumber)
{
	// 前回は押していない,今回は押しているのであればtrueを返す
	if (!keyPre[keyNumber] && key[keyNumber]) {
		return true;
	}
	// そうでなければfalseを返す
	return false;
}

void Input::DrawImGui() {
#ifdef USE_IMGUI
	ImGui::Begin("Input Debug");

	// --- マウス入力のON/OFF切り替え ---
	ImGui::Checkbox("Enable Mouse Input", &useMouseInput_);

	ImGui::Text("Mouse Position: (%d, %d)", mousePosition_.x, mousePosition_.y);  // 現在のマウス座標を表示
	ImGui::Text("Mouse Delta:    (%ld, %ld)", mouseState.lX, mouseState.lY);      // 今フレームでのマウスの移動量を表示（相対値）

	ImGui::Separator();
	ImGui::Text("Mouse Buttons:"); // マウスボタンの状態を表示
	for (int i = 0; i < 3; ++i) {
		if (mouseState.rgbButtons[i] & 0x80) {
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(1, 1, 0, 1), "[Button %d]", i); // ボタン名を黄色で表示
		}
	}
	ImGui::End();
#endif
}