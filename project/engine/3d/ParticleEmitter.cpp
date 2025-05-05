#include "ParticleEmitter.h"
#include<ParticleManager.h>
#ifdef USE_IMGUI
#include<ImGuiManager.h>
#endif // USE_IMGUI

ParticleEmitter::ParticleEmitter(const Vector3& position, const float lifetime, const float currentTime, const uint32_t count, const std::string& name, const Vector3& Velocity)
{
	position_ = position;//位置
	frequency = lifetime;//寿命
	frequencyTime = currentTime;//現在の寿命
	this->count = count;//count
	name_ = name;//名前
	velocity_ = Velocity; // 風の強さ
}

void ParticleEmitter::Update()
{
	// 時間を進める
	frequencyTime += 1.0f / 60.0f;

	// isAutoEmit_ が true のとき、自動発生処理
	if (isAutoEmit_) {
		// 発生間隔を超えたら Emit 実行
		if (frequencyTime >= frequency) {
			frequencyTime = 0.0f; // リセット
			Emit(); // パーティクル発生
		}
	}
}

void ParticleEmitter::Emit()
{
	//パーティクルを発生
	ParticleManager::GetInstance()->Emit(name_, position_, count, velocity_, frequency);
}

void ParticleEmitter::DebugUpdata() {
#ifdef USE_IMGUI
	ImGui::Begin("ParticleEmit");
	
	ImGui::Checkbox("Auto Emit", &isAutoEmit_);
	// 現在のパーティクル数と最大数を取得
	auto& group = ParticleManager::GetInstance()->GetGroup(name_);
	size_t currentCount = group.particles.size();
	uint32_t maxCount = ParticleManager::GetInstance()->GetMaxInstanceCount();
	uint32_t availableToEmit = static_cast<uint32_t>(std::min<size_t>(count, maxCount - currentCount));

	ImGui::Text("Available Emit Count: %u", availableToEmit);

	// Emit ボタン
	if (ImGui::Button("Emit Particles")) {
		if (availableToEmit > 0) {
			ParticleManager::GetInstance()->Emit("Circle", position_, availableToEmit, velocity_, frequency);
		}
	}

	ImGui::End();
#endif // USE_IMGUI
}