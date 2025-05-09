#pragma once
#include<Vector3.h>
#include <string>

// パーティクル発生器
class ParticleEmitter
{
public:
	//ほとんどのメンバ変数をコンストラクタの引数として受け取り、メンバ変数に代入する
	ParticleEmitter(const Vector3& position, const float lifetime, const float currentTime, const uint32_t count, const std::string& name, const Vector3& Velocity);

	// 更新処理
	void Update();
	// パーティクル発生
	void Emit();
	// imgui
	void DebugUpdata(const std::string& name);

private:
	// 名前
	std::string name_;
	// 座標
	Vector3 position_;
	// 数
	uint32_t count;
	// 風の強さ
	Vector3 velocity_;
	// 寿命
	float frequency;
	// 現在の寿命
	float frequencyTime;
	// 自動発生するかどうかの
	bool isAutoEmit_ = false;
};