#pragma once
#include<Sprite.h>
#include<Object3d.h>
#include<Model.h>
#include<SoundPlayer.h>
#include<BaseScene.h>
#include<ParticleEmitter.h>

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

    // 音声ファイル
    SoundData soundData;
    // 音声プレイフラグ
    uint32_t soundfige;

};