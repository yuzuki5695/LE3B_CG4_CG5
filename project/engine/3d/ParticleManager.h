#pragma once
#include<DirectXCommon.h>
#include<SrvManager.h>
#include<random>
#include<Vector2.h>
#include<Vector3.h>
#include<Vector4.h>
#include<Matrix4x4.h>
#include <Transform.h>
#include<Camera.h>
#include<Model.h>

enum class VertexType {
	OBJ,
	Sphere,
	Ring
};

// 3Dオブジェクト共通部
class ParticleManager
{
public:
	// 頂点データ
	struct VertexData
	{
		Vector4 position;
		Vector2 texcoord;
		Vector3 normal;
	};
	// マテリアル
	struct Material {
		Vector4 color;
		int32_t endbleLighting;
		float padding[3];
		Matrix4x4 uvTransform;
	};
	// マテリアルデータ
	struct MaterialDate {
		std::string textureFilePath;
		uint32_t textureindex = 0;
	};
	// モデルデータ
	struct ModelDate {
		std::vector<VertexData> vertices;
		MaterialDate material;
	};
	// パーティクル
	struct Particle {
		Transform transform;
		Vector3 Velocity;
		float lifetime;
		float currentTime;
		Vector4 color;
	};
	// インスタンスデータ
	struct InstanceData
	{
		Matrix4x4 WVP;
		Matrix4x4 World;
		Vector4 color;
	};
	// パーティクルグループ
	struct ParticleGroup {
		MaterialDate materialData;                             // マテリアルデータ(テクスチャファイルパスとテクスチャ用SRVインデックス)
		std::list<Particle> particles;                         // パーティクルのリスト
		uint32_t srvindex;                                     // インスタンシング用SRVインデックス
		Microsoft::WRL::ComPtr <ID3D12Resource> Resource;      // インスタンシングリソース
		uint32_t kNumInstance;                                 // インスタンス数
		InstanceData* instanceData = nullptr;                  // インスタンシングデータを書き込むためのポインタ
		// 発生間隔管理用のメンバー
		float spawnTime = 0.0f;  // 現在の発生までの経過時間
		float spawnFrequency = 2.0f;  // 2.0秒ごとに発生
	};
private:
	static std::unique_ptr<ParticleManager> instance;

	ParticleManager(ParticleManager&) = delete;
	ParticleManager& operator=(ParticleManager&) = delete;
public: // メンバ関数
	ParticleManager() = default;
	~ParticleManager() = default;

	// シングルトンインスタンスの取得
	static ParticleManager* GetInstance();
	// 終了
	void Finalize();
	// 初期化
	void Initialize(DirectXCommon* birectxcommon, SrvManager* srvmanager);
	// 更新処理
	void Update();
	// 描画処理
	void Draw();

	//　パーティクルモデル生成
	void SetParticleModel(const std::string& directorypath, const std::string& filename, VertexType type);

	// パーティクルグループの作成
	void CreateParticleGroup(const std::string& name, const std::string& textureFilepath);
	
	// 発生
	void Emit(const std::string& name, const Vector3& position, uint32_t count, const Vector3& velocity, float lifetime);

	void DebugUpdata();

	// 球の作成
	void DrawSphere(const uint32_t ksubdivision, VertexData* vertexdata);

	// 
	void DrawRing(VertexData* vertexData, uint32_t KRingDivide, float KOuterRadius, float KInnerRadius);


private:
	// .mtlファイルの読み取り
	static ParticleManager::MaterialDate LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);
	// .objファイルの読み取り
	static ParticleManager::ModelDate LoadObjFile(const std::string& directoryPath, const std::string& filename);
	// 頂点データ作成
	void VertexDatacreation(VertexType type);
	// マテリアル
	void MaterialGenerate();
private: // メンバ変数
	// ポインタ
	DirectXCommon* dxCommon_;
	SrvManager* srvmanager_;
	Camera* camera_;
	// ランダムエンジン
	std::mt19937 randomEngine;
	// Objファイルのデータ
	ModelDate modelDate;
	// バッファリソース
	Microsoft::WRL::ComPtr <ID3D12Resource> vertexResoruce;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;;
	// バッファリソース内のデータを指すポインタ
	VertexData* vertexData = nullptr;
	Material* materialData = nullptr;
	// バッファリソースの使い道を補足するバッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	//最大インスタンス
	uint32_t MaxInstanceCount = 200;
	//ビルボード行列
	Matrix4x4 backToFrontMatrix;
	// パーティクルグループコンテナ
	std::unordered_map<std::string, ParticleGroup> particleGroups;

	uint32_t vertexCount; //球の頂点数

public:
	// getter
	ParticleGroup& GetGroup(const std::string& name) {
		assert(particleGroups.count(name));
		return particleGroups[name];
	}
	uint32_t GetMaxInstanceCount() const { return MaxInstanceCount; }

	// setter
	void SetCamera(Camera* camera) { this->camera_ = camera; }
};