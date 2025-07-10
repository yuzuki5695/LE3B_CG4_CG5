#include "SkyboxCommon.h"
#include <Logger.h>
#include <StringUtility.h>
#include <ShaderCompiler.h>
#include<DsvManager.h>

using namespace Microsoft::WRL;

// 静的メンバ変数の定義
std::unique_ptr<SkyboxCommon> SkyboxCommon::instance = nullptr;

// シングルトンインスタンスの取得
SkyboxCommon* SkyboxCommon::GetInstance() {
    if (!instance) {
        instance = std::make_unique<SkyboxCommon>();
    }
    return instance.get();
}

// 終了
void SkyboxCommon::Finalize() {
    instance.reset();  // `delete` 不要
}

void SkyboxCommon::Initialize(DirectXCommon* dxCommon,DsvManager* dsvManager) {
    assert(dxCommon); 
    assert(dsvManager);
    // 引数を受け取ってメンバ変数に記録する
    dxCommon_ = dxCommon;
    dsvManager_ = dsvManager;
    // Skybox では深度書き込みを無効化        
    skyboxDepthStencilDesc.DepthEnable = true;
    skyboxDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    skyboxDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    // グラフィックスパイプラインの生成
    GraphicsPipelineGenerate();
}

void SkyboxCommon::Commondrawing() {
    // RootSignatureを設定。PSOに設定しているけど別途設定が必要
    dxCommon_->GetCommandList()->SetGraphicsRootSignature(rootSignature.Get());
    dxCommon_->GetCommandList()->SetPipelineState(graphicsPipelineState.Get());
    // 形状を設定。PSOに設定しているものとはまた別。同じものを設定する
    dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void SkyboxCommon::RootSignatureGenerate() {
    HRESULT hr;

    // SRV（キューブマップ）のディスクリプタレンジ設定
    D3D12_DESCRIPTOR_RANGE descriptorRange{};
    descriptorRange.BaseShaderRegister = 0;
    descriptorRange.NumDescriptors = 1;
    descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // ルートパラメータ（Material定数バッファ、Transform定数バッファ、SRV、必要に応じて追加）
    D3D12_ROOT_PARAMETER rootParameters[3] = {};

    // rootParameters[0]: gMaterial → register(b1)
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[0].Descriptor.ShaderRegister = 1; // ← b1 に修正！

    // rootParameters[1]: gTransformationMatrix → register(b0)（変更なし）
    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameters[1].Descriptor.ShaderRegister = 0; // b0

    // キューブマップ用 SRV（PixelShader用）
    rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;
    rootParameters[2].DescriptorTable.pDescriptorRanges = &descriptorRange;

    // サンプラー（バイリニアフィルタ）
    D3D12_STATIC_SAMPLER_DESC staticSampler{};
    staticSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    staticSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    staticSampler.MaxLOD = D3D12_FLOAT32_MAX;
    staticSampler.ShaderRegister = 0;
    staticSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    // ルートシグネチャ記述子
    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    rootSignatureDesc.NumParameters = _countof(rootParameters);
    rootSignatureDesc.pParameters = rootParameters;
    rootSignatureDesc.NumStaticSamplers = 1;
    rootSignatureDesc.pStaticSamplers = &staticSampler;

    // シリアライズと生成
    ComPtr<ID3DBlob> signatureBlob;
    ComPtr<ID3DBlob> errorBlob;
    hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr)) {
        Logger::Log(reinterpret_cast<const char*>(errorBlob->GetBufferPointer()));
        assert(false);
    }

    hr = dxCommon_->GetDevice()->CreateRootSignature(
        0,
        signatureBlob->GetBufferPointer(),
        signatureBlob->GetBufferSize(),
        IID_PPV_ARGS(&rootSignature));
    assert(SUCCEEDED(hr));
}

void SkyboxCommon::GraphicsPipelineGenerate() {
    // ルートシグネチャの生成
    RootSignatureGenerate();
    HRESULT hr;

    /*----------------------------------------------------------------------------------*/
    /*---------------------------------InputLayout設定-----------------------------------*/
    /*----------------------------------------------------------------------------------*/
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[2] = {};
    inputElementDescs[0].SemanticName = "POSITION";
    inputElementDescs[0].SemanticIndex = 0;
    inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

    inputElementDescs[1].SemanticName = "TEXCOORD";
    inputElementDescs[1].SemanticIndex = 0;
    inputElementDescs[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
    inputLayoutDesc.pInputElementDescs = inputElementDescs;
    inputLayoutDesc.NumElements = _countof(inputElementDescs);

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
    rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
    //三角形の中を塗りつぶす
    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

    /*----------------------------------------------------------------------------------*/
    /*--------------------------------ShaderをCompile-----------------------------------*/
    /*----------------------------------------------------------------------------------*/
    ComPtr <IDxcBlob> vertexShaderBlob = ShaderCompiler::GetInstance()->CompileShader(L"Resources/shaders/Skybox/Skybox.VS.hlsl", L"vs_6_0");
    assert(vertexShaderBlob != nullptr);
    ComPtr <IDxcBlob> pixelShaderBlob = ShaderCompiler::GetInstance()->CompileShader(L"Resources/shaders/Skybox/Skybox.PS.hlsl", L"ps_6_0");
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
    graphicsPipelineStateDesc.DepthStencilState = skyboxDepthStencilDesc;
    graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

    // 実際に生成
    hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState));
    assert(SUCCEEDED(hr));
}
