#pragma once
#include<d3d12.h>
#include<wrl.h>

class ResourceObject
{
public:
	ResourceObject(Microsoft::WRL::ComPtr <ID3D12Resource> resource)
		:resource_(resource)
	{}
	// デストラクタはオブジェクトの寿命が尽きた時に呼ばれる
	~ResourceObject() {
		// ここで、Resourceを呼べばい良い
		if (resource_) {
			resource_->Release();
		}
	}

	Microsoft::WRL::ComPtr <ID3D12Resource> Get() { return resource_; }

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> resource_;
};