#pragma once
#include<Input.h>
#include<DirectXCommon.h>
#include<ResourceObject.h>
#include<D3DResourceLeakChecker.h>
#include<TextureManager.h>
#include<SpriteCommon.h>
#include<Object3dCommon.h>
#include<ParticleCommon.h>
#include<ModelCommon.h>
#include<ModelManager.h>
#include<Camera.h>
#include<SrvManager.h>
#include<ImGuiManager.h>
#include<SoundPlayer.h>
#include<SceneManager.h>
#include<SceneFactory.h>
#include<ParticleManager.h>

// ゲーム全体
class Framework
{
public:// メンバ関数
	// デストラクタ
	virtual ~Framework() = default;

	// 初期化
	virtual void Initialize();
	// 終了
	virtual void Finalize();
	// 毎フレーム更新
	virtual void Update();
	// 描画
	virtual void Draw() = 0;

	// 実行
	void Run();

protected:
	// シーンファクトリー
	std::unique_ptr <AbstractSceneFactory> sceneFactory_ = nullptr;
private: // メンバ変数
	// ポインタ
	std::unique_ptr <WinApp> winApp = nullptr;
	std::unique_ptr <DirectXCommon> dxCommon = nullptr;
	std::unique_ptr <ModelCommon> modelCommon = nullptr;
	std::unique_ptr <SrvManager> srvManager = nullptr;

	// ゲーム終了フラグ
	bool endRequst_ = false;
public:
	// getter 
	WinApp* GetWinApp() const { return winApp.get(); }
	DirectXCommon* GetDirectXCommon() const { return dxCommon.get(); }
	SrvManager* GetSrvManager() const { return srvManager.get(); }
	//SceneManeger* GetSceneManeger() const { return sceneManeger_; }
	// 終了フラグのチェック
	virtual bool IsEndRequst() { return  endRequst_; }
};