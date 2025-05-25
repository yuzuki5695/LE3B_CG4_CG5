#pragma once
#include<Vector3.h>
#include <string>
#include <vector>
#include <Transform.h>

// パーティクル発生器
class ParticleEmitter
{
public:
	//ほとんどのメンバ変数をコンストラクタの引数として受け取り、メンバ変数に代入する
	ParticleEmitter(const std::string& name,const uint32_t count, const Transform& transform, const float lifetime, const float currentTime, const Vector3& Velocity);

	// 更新処理
	void Update();
	// パーティクル発生
	void Emit();
	// imgui
	void DrawImGuiUI();

private:
	// 名前
	std::string name_;
	// 座標
	Transform transform_;
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
	// テクスチャ変更フラグ
	bool isTextureChange_ = false;
	// テクスチャリスト
	static std::vector<std::string> textureList_;
	// 選択中のモデルインデックス
	int textureIndex_ = 0;

	//// 発生
	//emitter = std::make_unique <ParticleEmitter>(
	//	"Particles",                  // パーティクルグループ名
	//	1,                            // 発生数
	//	Transform{ { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 2.0f, 0.0f } }, // 位置
	//	3.0f,                         // 発生周期 or 寿命（自由に定義可能）
	//	0.0f,                         // 経過時間（基本は0から開始）
	//	Vector3{ 0.0f, 0.0f, 0.0f }  // ← 風
	//);
};