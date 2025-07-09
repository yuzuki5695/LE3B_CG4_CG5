#include "Fullscreen.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);
Texture2D<float> gMaskTexture : register(t1);

struct PixeShaderOutput{
    float4 color : SV_TARGET0;
}; 

PixeShaderOutput main(VertexShaderOutput input){
    PixeShaderOutput output;
    float mask = gMaskTexture.Sample(gSampler, input.texcoord);
    // maskの値が0.5(闘値)以下の場合はdiscardして抜く
    if (mask <= 0.5f)
    {
        discard;
    } 
    output.color = gTexture.Sample(gSampler, input.texcoord);
    return output;
}