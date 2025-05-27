#pragma once
#include<Vector3.h>
#include <string>
#include <vector>
#include <Transform.h>
#include<ParticleManager.h>

// パーティクル発生器
class ParticleEmitter
{
public:
	//ほとんどのメンバ変数をコンストラクタの引数として受け取り、メンバ変数に代入する
	ParticleEmitter(const std::string& name,const uint32_t count, const Transform& transform, const float lifetime, const float currentTime, const Vector3& Velocity, const RandomParameter& randomParameter);

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
	RandomParameter randomParameter_;


};