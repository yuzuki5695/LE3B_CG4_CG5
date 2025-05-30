#pragma once
#include<Sprite.h>
#include<Object3d.h>
#include<Model.h>
#include<SoundPlayer.h>
#include<BaseScene.h>
#include <ParticleEmitter.h>

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
    // camera 
    std::unique_ptr <Camera> camera = nullptr;
    //// カメラの現在の位置と回転を取得
    Vector3 Cameraposition;
    Vector3 Camerarotation;
    // Sprite
    std::unique_ptr<Sprite> sprite = nullptr;
    // Object3d
    std::unique_ptr <Object3d> object3d = nullptr;
    // 地面
    std::unique_ptr <Object3d> grass = nullptr;

    // 音声ファイル
    SoundData soundData;
    // 音声プレイフラグ
    uint32_t soundfige;

    // エミッター生成
    std::unique_ptr <ParticleEmitter> circle_;
    std::array<std::unique_ptr<ParticleEmitter>, 2> rings_;
    std::unique_ptr <ParticleEmitter> sphere_;
    std::unique_ptr <ParticleEmitter> rain_;
    std::unique_ptr <ParticleEmitter> sportclub_;
    
    RandomParameter defaultrandom_;
    RandomParameter random_;
    RandomParameter randomRing_;
    RandomParameter randomsphere_;
    RandomParameter randomrain_;
    RandomParameter randomsportclub_;
};