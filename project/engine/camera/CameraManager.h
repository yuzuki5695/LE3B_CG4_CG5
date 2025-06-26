#pragma once
#include <Object3d.h>
//#include<GamePlayCamera.h>

enum class CameraMode {
    Default,
    Follow,
    GamePlay,
};

struct BezierPoint {
    Vector3 handleLeft;
    Vector3 controlPoint;
    Vector3 handleRight;
};

class CameraManager {
private:
    static std::unique_ptr<CameraManager> instance;

    CameraManager(CameraManager&) = delete;
    CameraManager& operator=(CameraManager&) = delete;
public: // メンバ関数
    CameraManager() = default;
    ~CameraManager() = default;
    // シングルトンインスタンスの取得
    static CameraManager* GetInstance();
    // 終了
    void Finalize();

    // 初期化
    void Initialize();
    // 更新処理
    void Update();
    
    void ToggleCameraMode(bool followMode); // モード切替
    void DrawImGui(); // ImGui描画

	void SetActiveCamera(); // アクティブカメラを設定
    	
    std::vector<BezierPoint> LoadBezierFromJSON(const std::string& filePath);
    
    Vector3 BezierInterpolate(const Vector3& p0, const Vector3& p1, const Vector3& p2, const Vector3& p3, float t);


    void UpdateObjectPosition();

private:    
    // 現在のカメラモード
    CameraMode currentMode_;
    Object3d* target_ = nullptr; // 追従対象オブジェクト

    Camera* followCamera_;  // 追従用カメラ
    Camera* defaultCamera_; // 追従しないカメラ

    Camera* gamePlayCamera_ = nullptr; // ゲームプレイ用
	Vector3 moveOffset_; // カメラの移動オフセット

    Vector3 bezierPos_;

    std::vector<BezierPoint> bezierPoints; 
    float t = 0.0f;
	float speed = 0.001f; // 移動の速さ
	bool movefige; // ベジェ曲線に沿って移動するフラグ     
    int segmentIndex = 0; // 今は1つだけと仮定、複数ならループ管理
    // ヘッダーかクラス内に追加
    bool addedInitialOffset_ = false;

    bool useFollowCamera_ = false; // カメラモード切替用フラグ
public: // メンバ関数
    // 追従対象をセット（nullptrなら追従なし）
    void SetTarget(Object3d* target);
	Camera* GetFollowCamera() { return followCamera_; } // 追従カメラ取得
    Camera* GetActiveCamera();
	bool Getmovefige() { return movefige; } // ベジェ曲線に沿って移動するかどうか

    void SetCameraMode(CameraMode mode);

};