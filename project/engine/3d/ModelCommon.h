#pragma once
#include"DirectXCommon.h"

// 3Dモデル共通部
class ModelCommon
{
public: // メンバ関数
	// 初期化
	void Initialize(DirectXCommon* dxCommon);

private:
	// ポインタ
	DirectXCommon* dxCommon_;
public:
	// getter
	DirectXCommon* GetDxCommon() const { return  dxCommon_; }
};