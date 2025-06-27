#include "ShaderCompiler.h"
#include <Logger.h>
#include <StringUtility.h>
#include <format>
#include <cassert>

using namespace Microsoft::WRL;

// 静的メンバ変数の定義
std::unique_ptr<ShaderCompiler> ShaderCompiler::instance = nullptr;

// シングルトンインスタンスの取得
ShaderCompiler* ShaderCompiler::GetInstance() {
    if (!instance) {
        instance = std::make_unique<ShaderCompiler>();
    }
    return instance.get();
}

// 終了
void ShaderCompiler::Finalize() {
    instance.reset();  // `delete` 不要
}

void ShaderCompiler::Initialize() { 
    // DXCコンパイラの生成
    HRESULT hr;

    // dxCompilerを初期化
    hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
    assert(SUCCEEDED(hr));
    hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
    assert(SUCCEEDED(hr));
    //現時点でincludeはしないが、includeに対応するための設定を行っていく
    hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
    assert(SUCCEEDED(hr));
}

ComPtr <IDxcBlob> ShaderCompiler::CompileShader(const std::wstring& filePath,const wchar_t* profile) {
    //1.hlslファイルを読む
    //これからシェーダーをコンパイルする旨をログに出す
    Logger::Log(StringUtility::ConvertString(std::format(L"Begin CompileShader,path:{},profile:{}\n", filePath, profile)));
    Microsoft::WRL::ComPtr <IDxcBlobEncoding> shaderSource = nullptr;
    HRESULT hr = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);
    //読めなかったら止める
    assert(SUCCEEDED(hr));

    //読み込んだファイルの内容を設定する
    DxcBuffer shaderSourceBuffer;
    shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
    shaderSourceBuffer.Size = shaderSource->GetBufferSize();
    shaderSourceBuffer.Encoding = DXC_CP_UTF8;//UTF8のコードであることを通知
    
    //2.Compileする
    LPCWSTR arguments[] =
    {

         filePath.c_str(),
         L"-E",L"main",
         L"-T",profile,
         L"-Zi",L"-Qembed_debug",
         L"-Od",
         L"-Zpr",
    };
    //実際にshaderをコンパイルする
    ComPtr <IDxcResult> shaderResult = nullptr;
    hr = dxcCompiler->Compile(
        &shaderSourceBuffer,
        arguments,
        _countof(arguments),
        includeHandler.Get(),
        IID_PPV_ARGS(&shaderResult)
    );
    assert(SUCCEEDED(hr));

    //警告・エラーが出てたらログを出して止める
    ComPtr <IDxcBlobUtf8> shaderError = nullptr;
    shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
    if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
        Logger::Log(shaderError->GetStringPointer());
        assert(false);
    }
    //コンパイル結果から実行用のバイナリ部分を取得
    Microsoft::WRL::ComPtr <IDxcBlob> shaderBlob = nullptr;
    hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
    assert(SUCCEEDED(hr));
    //成功したログを出す
    Logger::Log(StringUtility::ConvertString(std::format(L"Compile Succeeded,path:{},profile:{}\n", filePath, profile)));
    //実行用のバイナリを返却
    return shaderBlob;
}