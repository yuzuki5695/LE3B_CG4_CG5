#pragma once
#include "SoundLoader.h"

// 音声再生
class SoundPlayer {
private:
    static std::unique_ptr<SoundPlayer> instance;

    SoundPlayer(SoundPlayer&) = delete;
    SoundPlayer& operator=(SoundPlayer&) = delete;
public: // メンバ関数
    SoundPlayer() = default;
    ~SoundPlayer() = default;

    // シングルトンインスタンスの取得
    static SoundPlayer* GetInstance();
    // 終了
    void Finalize();
    // 初期化
    void Initialize(SoundLoader* soundLoader);
    // 音声再生
    void SoundPlayWave(const SoundData& soundData, bool loop);
    // 音声データ解放
    void SoundUnload(SoundData* soundData);
private:
    SoundLoader* soundLoader_ = nullptr;
};