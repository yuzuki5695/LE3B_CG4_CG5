#include "Fullscreen.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixeShaderOutput{
    float4 color : SV_TARGET0;
}; 

PixeShaderOutput main(VertexShaderOutput input){
    PixeShaderOutput output;
    output.color = gTexture.Sample(gSampler,input.texcoord);
    // Grayscale(グレースケール化)
    float value = dot(output.color.rgb, float3(0.2125f, 0.7154f, 0.0721f));
    //output.color.rgb = float3(value, value, value); 
    // セピア長
    //output.color.rgb = value * float3(1.0f, 74.0f / 107.0f, 43.0f / 107.0f); 
    return output;
}