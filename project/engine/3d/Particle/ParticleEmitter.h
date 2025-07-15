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
	ParticleEmitter(const std::string& name,const uint32_t count, const Transform& transform, const Vector4& color, const float lifetime, const float currentTime, const Velocity& Velocity, const RandomParameter& randomParameter);

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
	// カラー
	Vector4 color;
	// 数
	uint32_t count;
	// 風の強さ
	Velocity velocity_;
	// 寿命
	float frequency;
	// 現在の寿命
	float frequencyTime;
	// 自動発生するかどうかの
	bool isAutoEmit_ = true;
	// テクスチャ変更フラグ
	bool isTextureChange_ = false;
	// テクスチャリスト
	static std::vector<std::string> textureList_;
	// 選択中のモデルインデックス
	int textureIndex_ = 0;
	RandomParameter randomParameter_;

	//random_ = {
	//	//座標
	//	{0.0f,0.0f,0.0f},  // 最小
	//	{0.0f,0.0f,0.0f},  // 最大
	//	// 回転
	//	{0.0f,0.0f,-std::numbers::pi_v<float>},  // 最小
	//	{0.0f,0.0f, std::numbers::pi_v<float>},  // 最大
	//	// サイズ
	//	{0.0f,0.4f,0.0f}, // 最小
	//	{0.0f,1.5f,0.0f}, // 最大
	//	// カラー
	//	0.0f,  // 最小
	//	1.0f,  // 最大
	//	// 寿命
	//	0.0f, // 最小
	//	0.0f, // 最大
	//	// 速度
	//	{0.0f,0.0f,0.0f},// 最小
	//	{0.0f,0.0f,0.0f} // 最大
	//};

public:
	float GetfrequencyTime() {return frequencyTime; }

};