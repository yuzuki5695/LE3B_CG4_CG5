#include "Fullscreen.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixeShaderOutput{
    float4 color : SV_TARGET0;
}; 

PixeShaderOutput main(VertexShaderOutput input){
    const float2 kCenter = float2(0.5f, 0.5f); // 中心点。ここを基準に放射状にブラーをがかかる
    const int kNumSamples = 10; // サンプリング数。多いほど滑らかだが重い
    const float kBlurwidth = 0.01f; // ぼかしの幅。大きいほど大きい
    // 中心から現在のuvに対しての方向を計算
    // 普段方向といえば、単位ベクトルだが、ここではあえて、正規化せず、遠いほどより遠くをサンプリングする
    float2 direction = input.texcoord - kCenter;
    float3 outputColor = float3(0.0f, 0.0f, 0.0f);
    for (int sampleindex = 0; sampleindex < kNumSamples; ++sampleindex)
    {
        // 現在のuvからさきほど計算した方向にサンプリング点を進めながらサンプリングしていく
        float2 texcoord = input.texcoord + direction * kBlurwidth * float(sampleindex);
        texcoord = clamp(texcoord, float2(0.0f, 0.0f), float2(1.0f, 1.0f));
        outputColor.rgb += gTexture.Sample(gSampler, texcoord).rgb;

    } 
    // 平均化する
    outputColor.rgb *= rcp(kNumSamples);
    
    PixeShaderOutput output;
    output.color.rgb = outputColor;
    output.color.a = 1.0f;
    return output;
}