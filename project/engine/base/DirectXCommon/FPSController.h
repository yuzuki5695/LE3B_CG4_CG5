#pragma once
#include <chrono>

// FPS固定
class FPSController {
public: // メンバ関数
	/// <summary>
	/// FPS固定初期化(現在時刻を基準にする）
	/// </summary>
	void Initialize();
	//  FPS固定更新
	void Update();	
private: // メンバ変数
	// 記録時間(FPS固定)
	std::chrono::steady_clock::time_point reference_;
};