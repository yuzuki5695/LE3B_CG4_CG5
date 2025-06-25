#include "Copylmage.hlsli"

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);


struct PixeShaderOutput{
    float32_t4 color : SV_TARGET0;
}; 

PixeShaderOutput main(VertexShaderOutput input){
    PixeShaderOutput output;
    output.color = gTexture.Sample(gSampler,input.texcoord);

    return output;

}