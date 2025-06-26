#include "ParticleEmitter.h"
#ifdef USE_IMGUI
#include<ImGuiManager.h>
#endif // USE_IMGUI
#include <ModelManager.h>

// テクスチャを定義
std::vector<std::string> ParticleEmitter::textureList_ = {
	"Resources/uvChecker.png",
	"Resources/monsterBall.png",
	"Resources/circle.png",
	"Resources/grass.png",
	"Resources/circle2.png",
	"Resources/gradationLine.png"
};

ParticleEmitter::ParticleEmitter(const std::string& name, const uint32_t count, const Transform& transform, const Vector4& color, const float lifetime, const float currentTime, const Velocity& Velocity, const RandomParameter& randomParameter) {
	name_ = name;//名前
	this->count = count;//count
	transform_ = transform;//位置
	this->color = color; // カラー
	frequency = lifetime;//寿命
	frequencyTime = currentTime;//現在の寿命
	velocity_ = Velocity; // 風の強さ
	randomParameter_ = randomParameter; // ランダムパラメータ
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
	ParticleManager::GetInstance()->Emit(name_, transform_, color, count, velocity_, frequency, randomParameter_);
}

void ParticleEmitter::DrawImGuiUI() {
#ifdef USE_IMGUI
	//if (ImGui::CollapsingHeader(name_.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) { // デフォルトで開いておく
	if (ImGui::CollapsingHeader(name_.c_str())) { // デフォルトで開いておく
		ImGui::Separator();
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "%s", name_.c_str()); // 黄色で強調

		ImGui::Checkbox(("Update##" + name_).c_str(), &isAutoEmit_);
		
		// --- ここに個数変更UIを追加 ---
		int countInt = static_cast<int>(count);
		if (ImGui::InputInt(("Emit Count##" + name_).c_str(), &countInt)) {
			if (countInt < 0) countInt = 0; // 負数防止
			count = static_cast<uint32_t>(countInt);
		}

		// --- テクスチャ選択 Combo ---
		if (ImGui::BeginCombo(("Texture##" + name_).c_str(), textureList_[textureIndex_].c_str())) {
			for (int i = 0; i < textureList_.size(); ++i) {
				bool selected = (textureIndex_ == i);
				if (ImGui::Selectable(textureList_[i].c_str(), selected)) {
					textureIndex_ = i;

					// テクスチャを設定（ここで ParticleManager に反映）
					ParticleManager::GetInstance()->SetParticleGroupTexture(name_, textureList_[textureIndex_]);
				}
				if (selected) ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		// Transformの編集
		if (ImGui::TreeNode(("Transform##" + name_).c_str())) {
			ImGui::DragFloat3(("Position##" + name_).c_str(), &transform_.translate.x, 0.01f);
			ImGui::DragFloat3(("Rotation##" + name_).c_str(), &transform_.rotate.x, 0.01f);
			ImGui::DragFloat3(("Scale##" + name_).c_str(), &transform_.scale.x, 0.01f);
			ImGui::TreePop();
		}

		auto& group = ParticleManager::GetInstance()->GetGroup(name_);
		size_t currentCount = group.particles.size();
		uint32_t maxCount = ParticleManager::GetInstance()->GetMaxInstanceCount();
		uint32_t availableToEmit = static_cast<uint32_t>(std::min<size_t>(count, maxCount - currentCount));

		// --- Emit ボタン ---
		if (ImGui::Button(("Emit Particles##" + name_).c_str())) {
			if (availableToEmit > 0) {
				// 発生
				ParticleManager::GetInstance()->Emit(name_, transform_, color, availableToEmit, velocity_, frequency, randomParameter_);
			}
		}
	}
#endif
}
