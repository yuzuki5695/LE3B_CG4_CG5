#include "Sprite.hlsli"

struct Material
{
    float4 color;
    int endbleLighting;
    float32_t4x4 uvTransform;
};

struct DirectionalLight
{
    float4 color; //!< ライトの色
    float3 direction; //!< ライトの向き
    float intensity; //!< 輝度
};

ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);

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
    
    if (gMaterial.endbleLighting != 0)
    { // Linhthingする場合
        // half lambert
        float Ndont = dot(normalize(input.normal), -gDirectionalLight.direction);
        float cos = pow(Ndont * 0.5f + 0.5f, 2.0f);
         
        // 色にはLightingを行い,a値には行わないようにする
        output.color.rgb = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
        output.color.a = gMaterial.color.a * textureColor.a;
    }
    else
    { // Linhthingしない場合、前回までと同じ演算
        output.color = gMaterial.color * textureColor;
    }
    return output;
}