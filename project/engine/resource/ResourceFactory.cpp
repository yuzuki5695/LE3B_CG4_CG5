#include "ResourceFactory.h"
#include <dxgi1_6.h>

using namespace Microsoft::WRL;

namespace ResourceFactory {

    ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(const ComPtr <ID3D12Device>& device, int32_t width, int32_t heigth) {
        // 生成するResourceの設定
        D3D12_RESOURCE_DESC resourceDesc{};
        resourceDesc.Width = width; // Textureの幅
        resourceDesc.Height = heigth; // Textureの高さ
        resourceDesc.MipLevels = 1; // mipmapの数
        resourceDesc.DepthOrArraySize = 1; // 奥行　or 配列のTexture配列数
        resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // DepthStencilとして利用可能なフォーマット
        resourceDesc.SampleDesc.Count = 1; // サンプリングカウント 1固定
        resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; // 2次元
        resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL; // DepthStencilとして使う通知

        // 利用Heapの設定
        D3D12_HEAP_PROPERTIES heapProperies{};
        heapProperies.Type = D3D12_HEAP_TYPE_DEFAULT; // VRAW上に作る

        // 深度値のクリア設定
        D3D12_CLEAR_VALUE depthClearValue{};
        depthClearValue.DepthStencil.Depth = 1.0f; // 1.0f (最大値)でクリア
        depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // フォーマット。 Resourceと合わせる


        //3. Resourceを生成する
        ComPtr <ID3D12Resource> resource = nullptr;
        HRESULT hr = device->CreateCommittedResource(
            &heapProperies, //Heapの設定
            D3D12_HEAP_FLAG_NONE, //Heapの特殊な設定。特になし。
            &resourceDesc, //Resourceの設定
            D3D12_RESOURCE_STATE_DEPTH_WRITE, //深度値を書き込む状態にしておく
            &depthClearValue, //Clear最適値
            IID_PPV_ARGS(&resource)); //作成するResourceポインタへのポインタ
        assert(SUCCEEDED(hr));
        return resource;
    }

    //DirectX12のTextureResourceを作る
    ComPtr <ID3D12Resource> CreateTextureResource(const ComPtr <ID3D12Device>& device, const DirectX::TexMetadata& metadata)
    {
        //1. metadataを基にResourceの設定
        D3D12_RESOURCE_DESC resourceDesc{};
        resourceDesc.Width = UINT(metadata.width);									//Textureの幅
        resourceDesc.Height = UINT(metadata.height);								//Textureの高さ
        resourceDesc.MipLevels = UINT16(metadata.mipLevels);						//mipmapの数
        resourceDesc.DepthOrArraySize = UINT16(metadata.arraySize);					//奥行 or 配列Textureの配列行数
        resourceDesc.Format = metadata.format;										//TextureのFormat
        resourceDesc.SampleDesc.Count = 1;											//サンプリングカウント。1固定
        resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension);		//Textureの次元数。普段使っているのは二次元
        resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        //2. 利用するHeapの設定。非常に特殊な運用。02_04exで一般的なケース版がある
        D3D12_HEAP_PROPERTIES heapProperties{};
        heapProperties.Type = D3D12_HEAP_TYPE_CUSTOM;								//細かい設定を行う
        heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;		//WriteBackポリシーでCPUアクセス可能
        heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;					//プロセッサの近くに配膳

        //3. Resourceを生成する
        ComPtr <ID3D12Resource> resource = nullptr;
        HRESULT hr = device->CreateCommittedResource(
            &heapProperties,														//Heapの設定
            D3D12_HEAP_FLAG_NONE,													//Heapの特殊な設定。特になし。
            &resourceDesc,															///Resourceの設定
            D3D12_RESOURCE_STATE_GENERIC_READ,										//初回のResourceState。Textureは基本読むだけ
            nullptr,																//Clear最適値。使わないのでnullptr
            IID_PPV_ARGS(&resource));												//作成するResourceポインタへのポインタ
        assert(SUCCEEDED(hr));
        return resource;
    }


    ComPtr <ID3D12Resource> CreateRenderTextureResource(ComPtr <ID3D12Device> device, uint32_t width, uint32_t height, DXGI_FORMAT format, const Vector4& clearColor) {
        //1. metadataを基にResourceの設定
        D3D12_RESOURCE_DESC resourceDesc{};
        resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        resourceDesc.Alignment = 0;
        resourceDesc.Width = width;
        resourceDesc.Height = height;
        resourceDesc.DepthOrArraySize = 1;
        resourceDesc.MipLevels = 1;
        resourceDesc.Format = format;
        resourceDesc.SampleDesc.Count = 1;
        resourceDesc.SampleDesc.Quality = 0;
        resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

        //2. 利用するHeapの設定。非常に特殊な運用。02_04exで一般的なケース版がある
        D3D12_HEAP_PROPERTIES heapProperties{};
        heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;								// 当然VRAM上に作る

        D3D12_CLEAR_VALUE clearValue;
        clearValue.Format = format;
        clearValue.Color[0] = clearColor.x;
        clearValue.Color[1] = clearColor.y;
        clearValue.Color[2] = clearColor.z;
        clearValue.Color[3] = clearColor.w;

        //3. Resourceを生成する
        ComPtr <ID3D12Resource> resource = nullptr;
        HRESULT hr = device->CreateCommittedResource(
            &heapProperties,														//Heapの設定
            D3D12_HEAP_FLAG_NONE,													//Heapの特殊な設定。特になし。
            &resourceDesc,															///Resourceの設定
            D3D12_RESOURCE_STATE_RENDER_TARGET,										//これから描画することを前提としたTextureなのでRenderTargetとして使うことから始める
            &clearValue,															//Clear最適地,ClearRenderTargetをこの色でClearするようにする。最適化されれいるので高速である
            IID_PPV_ARGS(&resource));												//作成するResourceポインタへのポインタ
        assert(SUCCEEDED(hr));
        return resource;
    }

    // Resourceの関数化
    ComPtr <ID3D12Resource> CreateBufferResource(const ComPtr <ID3D12Device>& device,size_t sizeInBytes) {
        //IDXGIのファクトリーの生成
        ComPtr <IDXGIFactory7> dxgiFactory = nullptr;
        HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
        //頂点リソース用のヒープの設定
        D3D12_HEAP_PROPERTIES uploadHeapProperties{};
        uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
        //頂点リソースの設定
        D3D12_RESOURCE_DESC vertexResourceDesc{};
        //バッファリソース、テクスチャの場合はまた別の設定をする
        vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        vertexResourceDesc.Width = sizeInBytes;
        //バッファの場合はこれらは１にする
        vertexResourceDesc.Height = 1;
        vertexResourceDesc.DepthOrArraySize = 1;
        vertexResourceDesc.MipLevels = 1;
        vertexResourceDesc.SampleDesc.Count = 1;
        //バッファの場合はこれにする
        vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        //実際に頂点リソースを作る
        ComPtr <ID3D12Resource> Resource = nullptr;
        hr = device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
            &vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&Resource));
        assert(SUCCEEDED(hr));
        return Resource;
    };
}