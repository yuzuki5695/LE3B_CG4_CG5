#include "Input.h"
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

}

void Input::Update() {
	// 前回のキー入力を保存
	memcpy(keyPre, key, sizeof(keyPre));
	// キーボード情報の取得開始
	keyboard->Acquire();
	// 全キ-の入力状態を取得する
	keyboard->GetDeviceState(sizeof(key), key);
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