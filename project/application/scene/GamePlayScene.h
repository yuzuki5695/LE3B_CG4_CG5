#pragma once
#include<Model.h>
#include<SoundPlayer.h>
#include<BaseScene.h>
#include<ParticleEmitter.h>

class Object3d;
class Sprite;

// ゲームプレイシーン
class GamePlayScene : public BaseScene
{
public: // メンバ関数
    // 初期化
    void Initialize() override;
    // 終了
    void Finalize() override;
    // 毎フレーム更新
    void Update() override;
    // 描画
    void Draw() override;

private: // メンバ変数
    // オブジェクトデータ
    // 地面
    std::unique_ptr <Object3d> grass = nullptr;
    std::unique_ptr <Object3d> Object_ = nullptr;
	std::unique_ptr <Sprite> sprite_ = nullptr;
};