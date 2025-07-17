#include "CopylmageCommon.h"
#include <ShaderCompiler.h>
#include "Logger.h"
#include "StringUtility.h"
#include<SrvManager.h>
#include<RtvManager.h>
#include<DsvManager.h>

using namespace Microsoft::WRL;

// 静的メンバ変数の定義
std::unique_ptr<CopylmageCommon> CopylmageCommon::instance = nullptr;

// シングルトンインスタンスの取得
CopylmageCommon* CopylmageCommon::GetInstance() {
    if (!instance) {
        instance = std::make_unique<CopylmageCommon>();
    }
    return instance.get();
}

// 終了
void CopylmageCommon::Finalize() {
    instance.reset();  // `delete` 不要
}

void CopylmageCommon::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager, RtvManager* rtvManager, DsvManager* dsvManager) {
    assert(dxCommon);
    assert(srvManager);
    assert(rtvManager);
    assert(dsvManager);
    // 引数を受け取ってメンバ変数に記録する
    dxCommon_ = dxCommon;
    dsvManager_ = dsvManager;
    // グラフィックスパイプラインの生成
    GraphicsPipelineGenerate();
    // SRVマネージャーの取得
    srvIndex = srvManager->CreateSRVForRenderTexture(rtvManager->GetrenderTextureResource());

    // --- 時間用定数バッファ作成 ---
    D3D12_HEAP_PROPERTIES heapProps{};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

    D3D12_RESOURCE_DESC resourceDesc{};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Width = (sizeof(float) + 255) & ~255; // 256バイトアライン
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    dxCommon_->GetDevice()->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&timeConstBuffer_));

    // マッピング
    timeConstBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&mappedTime_));
}

void CopylmageCommon::Commondrawing(SrvManager* srvManager) {
     // ---- 時間を更新 ----
    mappedTime_->time += 1.0f / 60.0f; // 毎フレーム進む。

    // RootSignatureを設定。PSOに設定しているけど別途設定が必要
    dxCommon_->GetCommandList()->SetGraphicsRootSignature(rootSignature.Get());
    dxCommon_->GetCommandList()->SetPipelineState(graphicsPipelineState.Get());
    // 形状を設定。PSOに設定しているものとはまた別。同じものを設定する
    dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// SRVのディスクリプタテーブルをピクセルシェーダーへバインド
    srvManager->SetGraphicsRootDescriptorTable(0, srvIndex);
       // 定数バッファ（b0）をセット
    dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(1, timeConstBuffer_->GetGPUVirtualAddress());
    // 描画命令を出す
    dxCommon_->GetCommandList()->DrawInstanced(3, 1, 0, 0);
}

void CopylmageCommon::RootSignatureGenerate() {
    HRESULT hr;

    // ===== DescriptorTable(SRV) ===== //
    D3D12_DESCRIPTOR_RANGE descriptorRange{};
    descriptorRange.BaseShaderRegister = 0;
    descriptorRange.NumDescriptors = 1;
    descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // ===== Root Parameters ===== //
    D3D12_ROOT_PARAMETER rootParameters[2] = {};

    // SRV (コピー元テクスチャ)
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
    rootParameters[0].DescriptorTable.pDescriptorRanges = &descriptorRange;

    // CBV（トーンマップパラメータなど、必要に応じて）
    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[1].Descriptor.ShaderRegister = 0;

    // ===== Static Sampler ===== //
    D3D12_STATIC_SAMPLER_DESC staticSampler{};
    staticSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    staticSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    staticSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    staticSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    staticSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    staticSampler.ShaderRegister = 0;
    staticSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    staticSampler.MaxLOD = D3D12_FLOAT32_MAX;

    // ===== Root Signature Description ===== //
    D3D12_ROOT_SIGNATURE_DESC rootSigDesc{};
    rootSigDesc.NumParameters = _countof(rootParameters);
    rootSigDesc.pParameters = rootParameters;
    rootSigDesc.NumStaticSamplers = 1;
    rootSigDesc.pStaticSamplers = &staticSampler;
    rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    // ===== Serialize and Create Root Signature ===== //
    ComPtr<ID3DBlob> signatureBlob;
    ComPtr<ID3DBlob> errorBlob;
    hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr)) {
        if (errorBlob) {
            Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
        }
        assert(false);
    }

    hr = dxCommon_->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
    assert(SUCCEEDED(hr));
}


void CopylmageCommon::GraphicsPipelineGenerate() {
    // ルートシグネチャの生成
    RootSignatureGenerate();
    HRESULT hr;

    /*----------------------------------------------------------------------------------*/
    /*---------------------------------InputLayout設定-----------------------------------*/
    /*----------------------------------------------------------------------------------*/

    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
    inputLayoutDesc.pInputElementDescs = nullptr;
    inputLayoutDesc.NumElements = 0;

    /*----------------------------------------------------------------------------------*/
    /*---------------------------------BlendStateの設定-----------------------------------*/
    /*----------------------------------------------------------------------------------*/
    D3D12_BLEND_DESC blendDesc{};
    //全ての色要素を書き込む
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;           // これから書き込む色。PixeShaderから出力する色 (ソースカラ―)
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;         // これから書き込むα。PixeShaderから出力するα値 (ソースアルファ)
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;         // すでに書き込まれている色 (デストカラー)

    //===== RasterizerStateの設定を行う ======//   
    D3D12_RASTERIZER_DESC rasterizerDesc{};
    //裏面(時計回り)を表示しない
    rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
    //三角形の中を塗りつぶす
    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

    /*----------------------------------------------------------------------------------*/
    /*--------------------------------ShaderをCompile-----------------------------------*/
    /*----------------------------------------------------------------------------------*/
    ComPtr <IDxcBlob> vertexShaderBlob = ShaderCompiler::GetInstance()->CompileShader(L"Resources/shaders/Fullscreen/Fullscreen.VS.hlsl", L"vs_6_0");
    assert(vertexShaderBlob != nullptr);
    type_ = PixelShaderType::Fullscreen; // ファイルパスを選択
    ComPtr <IDxcBlob> pixelShaderBlob = ShaderCompiler::GetInstance()->CompileShader(GetPixelShaderPath(type_), L"ps_6_0");
    assert(pixelShaderBlob != nullptr);

    /*-----------------------------------------------------------------------------------*/
    /*-------------------------------------PSO生成----------------------------------------*/
    /*-----------------------------------------------------------------------------------*/
    D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
    graphicsPipelineStateDesc.pRootSignature = rootSignature.Get();   // RootSignature
    graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;    // InputLayout
    graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),vertexShaderBlob->GetBufferSize() };  // VertexShader
    graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),pixelShaderBlob->GetBufferSize() };    // PixelShader
    graphicsPipelineStateDesc.BlendState = blendDesc;// BlendState
    graphicsPipelineStateDesc.RasterizerState = rasterizerDesc; //RasterizerState
    // 書き込むRTVの情報
    graphicsPipelineStateDesc.NumRenderTargets = 1;
    graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    //利用するトポロジ(形状)のタイプ
    graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    // どのように画面に色を打ち込むかの設定(気にしなくて良い)
    graphicsPipelineStateDesc.SampleDesc.Count = 1;
    graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
    // DepthStencilの設定
    graphicsPipelineStateDesc.DepthStencilState = dsvManager_->GetDepthStencilDesc();
    graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

    // 実際に生成
    hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState));
    assert(SUCCEEDED(hr));
}

std::wstring CopylmageCommon::GetPixelShaderPath(PixelShaderType type) {
    switch (type) {
    case PixelShaderType::Fullscreen:
        return L"Resources/shaders/Fullscreen/Fullscreen.PS.hlsl";
    case PixelShaderType::Vignette:
        return L"Resources/shaders/Fullscreen/Vignette.PS.hlsl";
    case PixelShaderType::BoxFilter:
        return L"Resources/shaders/Fullscreen/BoxFilter.PS.hlsl";
    case PixelShaderType::GaussianFilter:
        return L"Resources/shaders/Fullscreen/GaussianFilter.PS.hlsl";
    case PixelShaderType::LuminancsBasedOutline:
        return L"Resources/shaders/Fullscreen/LuminancsBasedOutline.PS.hlsl";
    case PixelShaderType::RadialBlur:
        return L"Resources/shaders/Fullscreen/RadialBlur.PS.hlsl";
    case PixelShaderType::Random:
        return L"Resources/shaders/Fullscreen/Random.PS.hlsl";
    default:
        return L"";
    }
}