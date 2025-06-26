#include "CameraManager.h"
#include <Object3dCommon.h>
#include <ParticleCommon.h>
#include <MatrixVector.h>
#ifdef USE_IMGUI
#include <ImGuiManager.h>
#endif // USE_IMGUI
#include<json.hpp>

using json = nlohmann::json;

using namespace MatrixVector;

// 静的メンバ変数の定義
std::unique_ptr<CameraManager> CameraManager::instance = nullptr;

// シングルトンインスタンスの取得
CameraManager* CameraManager::GetInstance() {
    if (!instance) {
        instance = std::make_unique<CameraManager>();
    }
    return instance.get();
}

// 終了
void CameraManager::Finalize() {
    instance.reset();  // `delete` 不要
}

void CameraManager::Initialize() {
    // 初期状態はデフォルトカメラ
    currentMode_ = CameraMode::GamePlay;
	useFollowCamera_ = false;
    
    // すべてのカメラを作っておく
    defaultCamera_ = new Camera();
    defaultCamera_->SetTranslate({0, 3.0f, -30.0f});
    defaultCamera_->SetRotate({0, 0, 0});

    followCamera_ = new Camera();
    
	moveOffset_ = { 0.0f, 6.0f, -60.0f }; // カメラの移動オフセット初期化
    gamePlayCamera_ = new Camera();
    gamePlayCamera_->SetTranslate(moveOffset_);
    gamePlayCamera_->SetRotate({0, 0, 0});
    movefige = false;

    // jsonファイルからベジェ曲線の制御点を読み込む
	bezierPoints = LoadBezierFromJSON("Resources/bezier.json"); 
    const BezierPoint& start = bezierPoints[segmentIndex]; 
    const BezierPoint& end = bezierPoints[segmentIndex + 1]; 
    bezierPos_ = BezierInterpolate(
        moveOffset_ + start.controlPoint,
        moveOffset_ + start.handleRight,
        moveOffset_ + end.handleLeft,
        end.controlPoint,
        t
    );

}

void CameraManager::Update() {
    switch (currentMode_) {
    case CameraMode::Follow:
        if (followCamera_) {
            if (target_) {
                Vector3 targetPos = target_->GetTranslate();
                Vector3 offset(0.0f, 3.0f, -30.0f);
                Vector3 cameraPos = targetPos + offset;

                followCamera_->SetTranslate(cameraPos);

                Vector3 toTarget = targetPos - cameraPos;
                float angleY = std::atan2(toTarget.x, toTarget.z);
                followCamera_->SetRotate(Vector3(0.0f, angleY, 0.0f));
            }
            followCamera_->Update();
        }
        break;
    case CameraMode::GamePlay:
        if (gamePlayCamera_) {
            if (movefige) {
                // ベジェ曲線に沿った位置を計算
                UpdateObjectPosition();
            }
            // カメラの位置を更新 
            gamePlayCamera_->SetTranslate(bezierPos_);
            gamePlayCamera_->Update();
        }
        break;
    case CameraMode::Default:
    default:
        if (defaultCamera_) {
            defaultCamera_->Update();
        }
        break;
    }
}

void CameraManager::SetTarget(Object3d* target) {
    target_ = target;
}

void CameraManager::ToggleCameraMode(bool followMode) {
    useFollowCamera_ = followMode;
}

void CameraManager::DrawImGui() {
#ifdef USE_IMGUI
    ImGui::Begin("CameraManager");

    // ラジオボタンでモード選択
    int mode = static_cast<int>(currentMode_);
    ImGui::Text("Camera Mode");
    if (ImGui::RadioButton("Default", mode == 0)) {
        mode = 0;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Follow", mode == 1)) {
        mode = 1;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("GamePlay", mode == 2)) {
        mode = 2;
    }

    // モードが変わったら切り替える
    CameraMode newMode = static_cast<CameraMode>(mode);
    if (newMode != currentMode_) {
        currentMode_ = newMode;
        SetActiveCamera();
    }

    // 現在のカメラを取得して編集
    Camera* activeCamera = nullptr;
    const char* modeName = "";
    const char* label = "Translate";

    switch (currentMode_) {
    case CameraMode::Default:
        activeCamera = defaultCamera_;
        modeName = "Default";
        break;
    case CameraMode::Follow:
        activeCamera = followCamera_;
        modeName = "Follow";
        break;
    case CameraMode::GamePlay:
        activeCamera = gamePlayCamera_;
        modeName = "GamePlay";
        break;
    }        
    ImGui::Text("Current Mode: %s", modeName);


    if (activeCamera) {
        if (newMode == CameraMode::GamePlay) {
            ImGui::Checkbox("isBezier", &movefige);
            if (!movefige) {
                if (ImGui::DragFloat3(label, &bezierPos_.x, 0.01f)) {
                    activeCamera->SetTranslate(bezierPos_);
                }
            }
        } else {
            Vector3 pos = activeCamera->GetTranslate();
            if (ImGui::DragFloat3(label, &pos.x, 0.01f)) {
                activeCamera->SetTranslate(pos);
            }
        }
    }

    ImGui::End();
#endif // USE_IMGUI
}

Camera* CameraManager::GetActiveCamera() {
    switch (currentMode_) {
    case CameraMode::Follow:
        return followCamera_ ? followCamera_ : defaultCamera_;
    case CameraMode::GamePlay:
        return gamePlayCamera_ ? gamePlayCamera_ : defaultCamera_;
    case CameraMode::Default:
    default:
        return defaultCamera_;
    }
}

void CameraManager::SetCameraMode(CameraMode mode) {
    currentMode_ = mode;
    SetActiveCamera(); // カメラ共通リソースへ反映
}

void CameraManager::SetActiveCamera() {
    Camera* active = GetActiveCamera();
    if (active) {
        Object3dCommon::GetInstance()->SetDefaultCamera(active);
        ParticleCommon::GetInstance()->SetDefaultCamera(active);
    }
}

std::vector<BezierPoint> CameraManager::LoadBezierFromJSON(const std::string& filePath) {
    std::vector<BezierPoint> points;

    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("JSONファイルを開けませんでした");
    }

    json j;
    file >> j;

    // "Curve" の最初のスプラインを使う
    if (!j.contains("Curve") || !j["Curve"].is_array() || j["Curve"].empty()) {
        throw std::runtime_error("Curveデータが存在しません");
    }

    for (const auto& pointData : j["Curve"][0]) {
        BezierPoint pt;
        pt.handleLeft = {
            pointData["handle_left"][0],
            pointData["handle_left"][1],
            pointData["handle_left"][2]
        };
        pt.controlPoint = {
            pointData["control_point"][0],
            pointData["control_point"][1],
            pointData["control_point"][2]
        };
        pt.handleRight = {
            pointData["handle_right"][0],
            pointData["handle_right"][1],
            pointData["handle_right"][2]
        };
        points.push_back(pt);
    }

    return points;
}

Vector3 CameraManager::BezierInterpolate(const Vector3& p0, const Vector3& p1, const Vector3& p2, const Vector3& p3, float t) {
    float u = 1.0f - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;

    Vector3 result;
    result = uuu * p0; // (1 - t)^3 * P0
    result += 3 * uu * t * p1; // 3 * (1 - t)^2 * t * P1
    result += 3 * u * tt * p2; // 3 * (1 - t) * t^2 * P2
    result += ttt * p3; // t^3 * P3

    return result;
}

void CameraManager::UpdateObjectPosition() {
    if (segmentIndex + 1 < bezierPoints.size()) {
        const BezierPoint& start = bezierPoints[segmentIndex];
        const BezierPoint& end = bezierPoints[segmentIndex + 1];
         
        bezierPos_ = BezierInterpolate(
            moveOffset_ + start.controlPoint,
            moveOffset_ + start.handleRight,
            moveOffset_ + end.handleLeft,
            moveOffset_ + end.controlPoint,
            t
        );

        t += speed;
       
        // 一度だけ初期オフセットを加算
        if (!addedInitialOffset_) {
            addedInitialOffset_ = true;
        }

        if (t > 1.0f) {
            t = 0.0f;
            segmentIndex++;
        }
    } else {
        // 最後の点で停止
        moveOffset_ = bezierPoints.back().controlPoint;
    }

    if (segmentIndex == 1) {
        segmentIndex = 0; // ループする場合はリセット
        t = 0.0f; // tもリセット
        addedInitialOffset_ = false; // ループ時にリセットするならここも必要
    }
}