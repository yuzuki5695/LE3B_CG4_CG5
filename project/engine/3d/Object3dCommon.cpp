#include "Object3dCommon.h"
#include <Logger.h>
#include <StringUtility.h>
#include <ShaderCompiler.h>
#include <ResourceFactory.h>
#include <numbers>
#include <MatrixVector.h>
#include<DsvManager.h>
#ifdef USE_IMGUI
#include<ImGuiManager.h>
#endif // USE_IMGUI

using namespace Microsoft::WRL;
using namespace MatrixVector;
using namespace ResourceFactory;

// 静的メンバ変数の定義
std::unique_ptr<Object3dCommon> Object3dCommon::instance = nullptr;

// シングルトンインスタンスの取得
Object3dCommon* Object3dCommon::GetInstance() {
    if (!instance) {
        instance = std::make_unique<Object3dCommon>();
    }
    return instance.get();
}

// 終了
void Object3dCommon::Finalize() {
    instance.reset();  // `delete` 不要
}

void Object3dCommon::Initialize(DirectXCommon* dxCommon,DsvManager* dsvManager) {
    assert(dxCommon);
    assert(dsvManager);
    // 引数を受け取ってメンバ変数に記録する
    dxCommon_ = dxCommon;     
    dsvManager_ = dsvManager;
    // グラフィックスパイプラインの生成
    GraphicsPipelineGenerate();

    // リソース
    // 平行光源の生成,初期化
    DirectionalLightGenerate();
    // 点光源リソースの生成、初期化
    PointlightSourceGenerate();
    // スポットライトリソースの生成、初期化
    SpotlightGenerate();
}

void Object3dCommon::Commondrawing() {
    // RootSignatureを設定。PSOに設定しているけど別途設定が必要
    dxCommon_->GetCommandList()->SetGraphicsRootSignature(rootSignature.Get());
    dxCommon_->GetCommandList()->SetPipelineState(graphicsPipelineState.Get());
    // 形状を設定。PSOに設定しているものとはまた別。同じものを設定する
    dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // 平行光源用のCBufferの場所を設定 
    dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
    // 点光源を設定 
    dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(5, pointLightResource->GetGPUVirtualAddress());
    // スポットライトを設定 
    dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(6, spotLightResource->GetGPUVirtualAddress());
}

void Object3dCommon::RootSignatureGenerate() {
    HRESULT hr;

    /*----------------------------------------------------------------------------------*/
    /*---------------------------------DescriptorRange作成-------------------------------*/
    /*----------------------------------------------------------------------------------*/
    D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
    descriptorRange[0].BaseShaderRegister = 0;
    descriptorRange[0].NumDescriptors = 1;
    descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    /*----------------------------------------------------------------------------------*/
    /*---------------------------------RootParameter作成---------------------------------*/
    /*----------------------------------------------------------------------------------*/
    D3D12_ROOT_PARAMETER rootParameters[7] = {};
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;// CBVを使う
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;// PixelShaderで使う
    rootParameters[0].Descriptor.ShaderRegister = 0;// レジスタ番号0を使う

    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;// CBVを使う
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;// VertexShaderでを使う
    rootParameters[1].Descriptor.ShaderRegister = 0;// レジスタ番号0を使う

    rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;//DescriptorTableを使う
    rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
    rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;//Tableの中身の配列を指定
    rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);//利用する数

    rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;// CBVを使う
    rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;// PixelShaderで使う
    rootParameters[3].Descriptor.ShaderRegister = 1;// レジスタ番号1を使う

    rootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;// CBVを使う
    rootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;// PixelShaderで使う
    rootParameters[4].Descriptor.ShaderRegister = 2;// レジスタ番号2を使う

    rootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;// CBVを使う
    rootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;// PixelShaderで使う
    rootParameters[5].Descriptor.ShaderRegister = 3;// レジスタ番号3を使う

    rootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;// CBVを使う
    rootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;// PixelShaderで使う
    rootParameters[6].Descriptor.ShaderRegister = 4;// レジスタ番号4を使う

    /*----------------------------------------------------------------------------------*/
    /*---------------------------------RootSignature作成---------------------------------*/
    /*----------------------------------------------------------------------------------*/
    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    descriptionRootSignature.pParameters = rootParameters;// ルートパラメータ配列へのポインタ
    descriptionRootSignature.NumParameters = _countof(rootParameters);// 配列の長さ

    /*----------------------------------------------------------------------------------*/
    /*-----------------------------------Samplerの設定-----------------------------------*/
    /*----------------------------------------------------------------------------------*/
    D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
    staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;//バイリニアフィルタ	
    staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//0～1の範囲外をリピート		
    staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;//比較しない		
    staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;//ありったけのMipmapを使う	
    staticSamplers[0].ShaderRegister = 0;//レジスタ番号０を使う
    staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
    descriptionRootSignature.pStaticSamplers = staticSamplers;
    descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

    /*----------------------------------------------------------------------------------*/
    /*-------------------------シリアライズしてバイナリにする--------------------------------*/
    /*----------------------------------------------------------------------------------*/
    ComPtr <ID3DBlob> signatureBlob = nullptr;
    ComPtr <ID3DBlob> errorBlob = nullptr;
    hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr)) {
        Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
        assert(false);
    }
    //バイナリを元に作成
    hr = dxCommon_->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
    assert(SUCCEEDED(hr));
}

void Object3dCommon::GraphicsPipelineGenerate() {
    // ルートシグネチャの生成
    RootSignatureGenerate();
    HRESULT hr;

    /*----------------------------------------------------------------------------------*/
    /*---------------------------------InputLayout設定-----------------------------------*/
    /*----------------------------------------------------------------------------------*/
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
    inputElementDescs[0].SemanticName = "POSITION";
    inputElementDescs[0].SemanticIndex = 0;
    inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

    inputElementDescs[1].SemanticName = "TEXCOORD";
    inputElementDescs[1].SemanticIndex = 0;
    inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
    inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

    inputElementDescs[2].SemanticName = "NORMAL";
    inputElementDescs[2].SemanticIndex = 0;
    inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

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

    //// 加算合成
    //blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;       // これから書き込む色。PixeShaderから出力する色 (ソースカラ―)
    //blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;           // これから書き込むα。PixeShaderから出力するα値 (ソースアルファ)
    //blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;            // すでに書き込まれている色 (デストカラー)

    //// 減算合成
    //blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;         // これから書き込む色。PixeShaderから出力する色 (ソースカラ―)
    //blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;    // これから書き込むα。PixeShaderから出力するα値 (ソースアルファ)
    //blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;              // すでに書き込まれている色 (デストカラー)

    //// 乗算合成
    //blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;           // これから書き込む色。PixeShaderから出力する色 (ソースカラ―)
    //blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;         // これから書き込むα。PixeShaderから出力するα値 (ソースアルファ)
    //blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_SRC_COLOR;    // すでに書き込まれている色 (デストカラー)

    //// スクリーン合成
    //blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_INV_DEST_COLOR;    // これから書き込む色。PixeShaderから出力する色 (ソースカラ―)
    //blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;             // これから書き込むα。PixeShaderから出力するα値 (ソースアルファ)
    //blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;              // すでに書き込まれている色 (デストカラー)

    //===== RasterizerStateの設定を行う ======//   
    D3D12_RASTERIZER_DESC rasterizerDesc{};
    //裏面(時計回り)を表示しない
    rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
    //三角形の中を塗りつぶす
    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

    /*----------------------------------------------------------------------------------*/
    /*--------------------------------ShaderをCompile-----------------------------------*/
    /*----------------------------------------------------------------------------------*/
    ComPtr <IDxcBlob> vertexShaderBlob = ShaderCompiler::GetInstance()->CompileShader(L"Resources/shaders/Object3d/Object3D.VS.hlsl", L"vs_6_0");
    assert(vertexShaderBlob != nullptr);
    ComPtr <IDxcBlob> pixelShaderBlob = ShaderCompiler::GetInstance()->CompileShader(L"Resources/shaders/Object3d/Object3D.PS.hlsl", L"ps_6_0");
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


void Object3dCommon::DirectionalLightGenerate() {
    // 平行光源用のリソースを作る
    directionalLightResource = CreateBufferResource(dxCommon_->GetDevice(), sizeof(DirectionalLight));
    // 平行光源用にデータを書き込むためのアドレスを取得
    directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightDate));
    // デフォルト値はとりあえず以下のようにして置く
    directionalLightDate->color = { 1.0f, 1.0f, 1.0f, 1.0f };
    directionalLightDate->direction = { 0.0f,-1.0f,0.0f };
    directionalLightDate->intensity = 1.0f;
}

void Object3dCommon::PointlightSourceGenerate() {
    // 点光源用リソースを作る
    pointLightResource = CreateBufferResource(dxCommon_->GetDevice(), sizeof(PointLight));
    // 書き込むためのアドレスを取得
    pointLightResource->Map(0, nullptr, reinterpret_cast<void**>(&pointLightData));
    // デフォルト値
    pointLightData->color = { 1.0f,1.0f,1.0f,1.0f };
    pointLightData->position = { 0.0f,2.0f,0.0f };
    pointLightData->intensity = 0.0f;
    pointLightData->radius = 10.0f;
    pointLightData->decay = 1.0f;
}

void Object3dCommon::SpotlightGenerate() {
    // スポットライトリソースを作る
    spotLightResource = CreateBufferResource(dxCommon_->GetDevice(), sizeof(SpotLight));
    // 書き込むためのアドレスを取得
    spotLightResource->Map(0, nullptr, reinterpret_cast<void**>(&spotLightData));
    // デフォルト値
    spotLightData->color = { 1.0f,1.0f,1.0f,1.0f };
    spotLightData->position = { 2.0f,1.25f,0.0f };
    spotLightData->distance = 7.0f;
    spotLightData->direction =
        Normalize({ -1.0f,-1.0f,0.0f });
    spotLightData->intensity = 4.0f;
    spotLightData->decay = 2.0f;
    spotLightData->cosFalloffStart = 1.0f;
    spotLightData->cosAngle =
        std::cos(std::numbers::pi_v<float> / 3.0f);
}

void Object3dCommon::DrawImGui() {
#ifdef USE_IMGUI
    ImGui::Begin("lighting");
    // ライトセクション
    if (ImGui::CollapsingHeader("Directional Light")) {
        ImGui::DragFloat3("Direction", &directionalLightDate->direction.x, 0.01f);
        ImGui::DragFloat("Intensity", &directionalLightDate->intensity, 0.01f);
    }
    // ポイントライト
    if (ImGui::CollapsingHeader("Point Light")) {
        ImGui::DragFloat3("Point Light : Position", &pointLightData->position.x, 0.01f);
        ImGui::DragFloat("Point Light : Intensity", &pointLightData->intensity, 0.01f);
        ImGui::DragFloat("Point Light : Radius", &pointLightData->radius, 0.01f);
        ImGui::DragFloat("Point Light : Decay", &pointLightData->decay, 0.01f);
    }
    // スポットライト
    if (ImGui::CollapsingHeader("Spot Light")) {
        ImGui::DragFloat3("Spot Light : Position", &spotLightData->position.x, 0.01f);
        ImGui::DragFloat("Spot Light : Intensity", &spotLightData->intensity, 0.01f);
        ImGui::DragFloat3("Spot Light : Direction", &spotLightData->direction.x, 0.01f);
        ImGui::DragFloat("Spot Light : Decay", &spotLightData->decay, 0.01f);
        ImGui::DragFloat("Spot Light : CosAngle", &spotLightData->cosAngle, 0.01f);
        ImGui::DragFloat("Spot Light : CosFalloffStart", &spotLightData->cosFalloffStart, 0.01f);
    }

    ImGui::End();
#endif // USE_IMGUI
}