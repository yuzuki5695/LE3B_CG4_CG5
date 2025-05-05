#include "SoundPlayer.h"
#include <cassert>

// 静的メンバ変数の定義
std::unique_ptr<SoundPlayer> SoundPlayer::instance = nullptr;

// シングルトンインスタンスの取得
SoundPlayer* SoundPlayer::GetInstance() {
    if (!instance) {
        instance = std::make_unique<SoundPlayer>();
    }
    return instance.get();
}

// 終了
void SoundPlayer::Finalize() {
    instance.reset();
}

void SoundPlayer::Initialize(SoundLoader* soundLoader) {
    // NULL検出
    assert(soundLoader);
    // メンバ変数に記録
    this->soundLoader_ = soundLoader;
}

void SoundPlayer::SoundPlayWave(const SoundData& soundData, bool loop) {
    HRESULT result;

    // 波形フォーマットを元にSourceVoiceの生成
    IXAudio2SourceVoice* pSourceVoice = nullptr;
    result = soundLoader_->GetIXAudio2()->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
    assert(SUCCEEDED(result));

    // 再生する波形データの設定
    XAUDIO2_BUFFER buf{};
    buf.pAudioData = soundData.pBuffer;
    buf.AudioBytes = soundData.bufferSize;

    // 再生をループするかどうか
    if (loop) {
        // 無限ループ
        buf.LoopCount = XAUDIO2_LOOP_INFINITE;
    } else {
        // 1度だけ再生
        buf.Flags = XAUDIO2_END_OF_STREAM;
    }

    // 波形データの再生
    result = pSourceVoice->SubmitSourceBuffer(&buf);
    result = pSourceVoice->Start();
}

void SoundPlayer::SoundUnload(SoundData* soundData)
{
    // バッファのメモリを解放
    delete[] soundData->pBuffer;
    soundData->pBuffer = 0;
    soundData->bufferSize = 0;
    soundData->wfex = {};
}