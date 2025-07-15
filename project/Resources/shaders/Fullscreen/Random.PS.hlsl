#include "Fullscreen.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

float rand2dTo1d(float2 uv)
{
    return frac(sin(dot(uv, float2(12.9898, 78.233))) * 43758.5453);
}

cbuffer TimeBuffer : register(b0)
{
    float time;
}

struct PixeShaderOutput{
    float4 color : SV_TARGET0;
}; 

PixeShaderOutput main(VertexShaderOutput input)
{
    // 元画像を取得
    float4 baseColor = gTexture.Sample(gSampler, input.texcoord);

    // 時間で動くノイズ生成（砂嵐）
    float random = rand2dTo1d(input.texcoord * time);
    float noise = random > 0.5 ? 1.0 : 0.0;

    float3 noiseColor = float3(noise, noise, noise); // 白 or 黒のノイズ
    float3 result = baseColor.rgb * noiseColor; // ←★乗算合成★
    
    PixeShaderOutput output;
    output.color = float4(result, baseColor.a);
    return output;
}
