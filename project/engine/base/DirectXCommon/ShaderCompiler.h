#pragma once
#include<DirectXCommon.h>

// シェーダーコンパイル
class ShaderCompiler
{
private:
	static std::unique_ptr<ShaderCompiler> instance;

	ShaderCompiler(ShaderCompiler&) = delete;
	ShaderCompiler& operator=(ShaderCompiler&) = delete;
public: // メンバ関数
	ShaderCompiler() = default;
	~ShaderCompiler() = default;

	// シングルトンインスタンスの取得
	static ShaderCompiler* GetInstance();
	// 終了
	void Finalize();
	// 初期化
	void Initialize();

	// コンパイルシェーダー
	Microsoft::WRL::ComPtr <IDxcBlob> CompileShader(const std::wstring& filePath, const wchar_t* profile);
private: // メンバ変数
	// DXCコンパイラの初期化
	Microsoft::WRL::ComPtr <IDxcUtils> dxcUtils = nullptr;
	Microsoft::WRL::ComPtr <IDxcCompiler3> dxcCompiler = nullptr;
	Microsoft::WRL::ComPtr <IDxcIncludeHandler> includeHandler = nullptr;
};