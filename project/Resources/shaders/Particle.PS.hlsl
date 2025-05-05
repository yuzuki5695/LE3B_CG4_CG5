#include "Particle.hlsli"

struct Material
{
    float4 color;
    int endbleLighting;
    float4x4 uvTransform;
};

struct DirectionalLight
{
    float4 color; //!< ライトの色
    float3 direction; //!< ライトの向き
    float intensity; //!< 輝度
};

ConstantBuffer<Material> gMaterial : register(b0);

struct PixeShaderOutput
{
    float4 color : SV_TARGET0;
    
};

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

PixeShaderOutput main(VertexShaderOutput input)
{
        
    PixeShaderOutput output;
    
    //TextureをSampling
    float4 TransformesUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float4 textureColor = gTexture.Sample(gSampler, TransformesUV.xy);

    output.color = gMaterial.color * textureColor * input.color;
    if (output.color.a == 0.0)
    {
        discard;
    }
    
    return output;
}
