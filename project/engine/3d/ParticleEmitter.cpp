#include "ParticleEmitter.h"
#include<ParticleManager.h>
#ifdef USE_IMGUI
#include<ImGuiManager.h>
#endif // USE_IMGUI
#include <ModelManager.h>

// 定義
std::vector<std::string> ParticleEmitter::textureList_ = {
	"Resources/uvChecker.png",
	"Resources/monsterBall.png",
	"Resources/circle.png",
	"Resources/grass.png",
	"Resources/circle2.png",
	"Resources/gradationLine.png"
};

std::vector<std::string> ParticleEmitter::modelList_ = {
	"plane.obj",
	"axis.obj",
	"monsterBallUV.obj",
	"fence.obj",
	"terrain.obj"
};

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
	// テクスチャリストの表示
	static int currentTextureIndex = 0;

	// テクスチャが存在するかチェック
	if (!textureList_.empty()) {
		// インデックスがリストサイズを超えていたらリセット
		if (currentTextureIndex >= textureList_.size()) {
			currentTextureIndex = 0;
		}

		// テクスチャ選択コンボボックス
		if (ImGui::BeginCombo("Texture Selector", textureList_[currentTextureIndex].c_str())) {
			for (int n = 0; n < (int)textureList_.size(); n++) {
				bool isSelected = (currentTextureIndex == n);
				if (ImGui::Selectable(textureList_[n].c_str(), isSelected)) {
					currentTextureIndex = n;
					// テクスチャ切り替え
					ParticleManager::GetInstance()->SetParticleGroupTexture(name_, textureList_[n]);
				}
				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
	} else {
		ImGui::Text("No textures available.");
	}
	// モデルリストの表示
	static int currentModelIndex = 0;
	std::vector<std::string> availableModels;
	// 使用可能なモデルだけ抽出
	for (const auto& modelName : modelList_) {
		if (ModelManager::GetInstance()->FindModel(modelName)) {
			availableModels.push_back(modelName);
		}
	}
	// モデル選択コンボボックス
	if (!availableModels.empty()) {
		if (ImGui::BeginCombo("Model Selector", availableModels[currentModelIndex].c_str())) {
			for (int i = 0; i < (int)availableModels.size(); ++i) {
				bool isSelected = (currentModelIndex == i);
				if (ImGui::Selectable(availableModels[i].c_str(), isSelected)) {
					currentModelIndex = i;
					// モデル切り替え処理
					ParticleManager::GetInstance()->SetParticleGroupModel(name_, availableModels[i]);
				}
				if (isSelected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
	} else {
		ImGui::Text("No Model");
	}

	ImGui::Checkbox("Auto Emit", &isAutoEmit_);
	// 現在のパーティクル数と最大数を取得
	auto& group = ParticleManager::GetInstance()->GetGroup(name_);
	size_t currentCount = group.particles.size();
	uint32_t maxCount = ParticleManager::GetInstance()->GetMaxInstanceCount();
	uint32_t availableToEmit = static_cast<uint32_t>(std::min<size_t>(count, maxCount - currentCount));
	
	
	// 現在のパーティクル数を表示
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