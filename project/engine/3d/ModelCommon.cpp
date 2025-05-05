#include "ModelCommon.h"

void ModelCommon::Initialize(DirectXCommon* dxCommon) {
    // NULL検出
    assert(dxCommon);
    // 引数を受け取ってメンバ変数に記録する
    dxCommon_ = dxCommon;
}