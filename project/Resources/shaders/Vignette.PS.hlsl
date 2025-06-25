#include "Fullscreen.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixeShaderOutput{
    float4 color : SV_TARGET0;
}; 

PixeShaderOutput main(VertexShaderOutput input){
    PixeShaderOutput output;
    output.color = gTexture.Sample(gSampler,input.texcoord);
    // 周囲を0に,中心になるほど明るくなるように計算で調整
    float2 correct = input.texcoord * (1.0f - input.texcoord.yx);
    // correctだけで計算すると中心の最大値が0.0625で暗すぎるのでSCaleで調整。この例では16倍にして1にしている 
    float vignette = correct.x * correct.y * 16.0f;
    // とりあえず0.8乗でそれっぽくしてみた
    vignette = saturate(pow(vignette, 0.8f));
    // 係数として計算
    output.color.rgb *= vignette;
    return output;
}