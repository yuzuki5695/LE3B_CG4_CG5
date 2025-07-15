#pragma once
#include<Input.h>
#include<ResourceObject.h>
#include<TextureManager.h>
#include<SpriteCommon.h>
#include<Object3dCommon.h>
#include<ParticleCommon.h>
#include<ModelCommon.h>
#include<ModelManager.h>
#include<Camera.h>
#include<ImGuiManager.h>
#include<SoundPlayer.h>
#include<SceneManager.h>
#include<ParticleManager.h>
#include<RtvManager.h>
#include<DsvManager.h>

class WinApp;
class DirectXCommon;
class ModelCommon;
class SrvManager;
class AbstractSceneFactory;

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
	// imgui
	void DrawDebug();
protected:
	// シーンファクトリー
	std::unique_ptr <AbstractSceneFactory> sceneFactory_ = nullptr;
private: // メンバ変数
	// ポインタ
	std::unique_ptr <WinApp> winApp = nullptr;
	std::unique_ptr <DirectXCommon> dxCommon = nullptr;
	std::unique_ptr <ModelCommon> modelCommon = nullptr;
	std::unique_ptr <SrvManager> srvManager = nullptr;	
	std::unique_ptr <RtvManager> rtvManager = nullptr;
	std::unique_ptr <DsvManager> dsvManager = nullptr;
	// ゲーム終了フラグ
	bool endRequst_ = false;
public:
	// getter 
	WinApp* GetWinApp() const { return winApp.get(); }
	DirectXCommon* GetDirectXCommon() const { return dxCommon.get(); }
	SrvManager* GetSrvManager() const { return srvManager.get(); }
	// 終了フラグのチェック
	virtual bool IsEndRequst() { return  endRequst_; }
};