#pragma once
#include<Material.h>
#include<MaterialDate.h>
#include<ModelDate.h>
#include <string>
#include <Vector3.h>
#include <map>

struct Quaternion {
	Vector3 rotate;  // 回転
};

//キーフレーム構造体
template <typename tValue>
struct Keyframe {
	float time;   //!< キーフレームの時刻(単位は秒)
	tValue value; //!< キーフレームの値
};

using keyframeVector3 = Keyframe<Vector3>;           // 座標
using keyframeQuaternion = Keyframe<Quaternion>;     // 回転

// アニメーションカーブ：キーフレームの配列
template <typename tValue>
struct AnimationCurve {
    std::vector<Keyframe<tValue>> keyframes; //!< アニメーション用のキーフレーム配列
};

// ノード単位のアニメーション：移動・回転・スケール
struct NodeAnimation {
	AnimationCurve<Vector3> translate;   //!< 座標アニメーション
	AnimationCurve<Quaternion> rotate;   //!< 回転アニメーション
	AnimationCurve<Vector3> scale;       //!< スケールアニメーション
};

struct Animation {
	float duration; // アニメーション全体の尺(単位は秒)
	// NodeAnimationの集合。Node名で引けるようにしておく
	std::map<std::string, NodeAnimation> nodeAnimations;
};

class AnimationPlayer {
public: // メンバ関数
	// 初期化
	void Initialize();
	// 描画処理
	void Draw();

	//// .mtlファイルの読み取り
	//static MaterialDate LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);
	//// .objファイルの読み取り
	//static ModelDate LoadObjFile(const std::string& directoryPath, const std::string& filename);


	Animation LoadAnimationFile(const std::string& directoryPath, const std::string& filename);

private:



};