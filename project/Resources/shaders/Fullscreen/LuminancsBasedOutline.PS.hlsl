#include "Fullscreen.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixeShaderOutput{
    float4 color : SV_TARGET0;
}; 

static const float2 kIndex3x3[3][3] ={
    { { -1.0f, -1.0f }, { 0.0f, -1.0f }, { 1.0f, -1.0f } },
    { { -1.0f, 0.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f } },
    { { -1.0f, 1.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f } },
};

static const float kKernel3x3[3][3] = {
    { 1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f},
    { 1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f},
    { 1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f},
};

static const float kPrewittHorizontalKernel[3][3] =
{
    { -1.0f / 6.0f, 0.0f, 1.0f / 6.0f },
    { -1.0f / 6.0f, 0.0f, 1.0f / 6.0f },
    { -1.0f / 6.0f, 0.0f, 1.0f / 6.0f },
};

static const float kPrewittVerticalKernel[3][3] =
{
    { -1.0f / 6.0f, -1.0f / 6.0f, -1.0f / 6.0f },
    { 0.0f, 0.0f, 0.0f },
    { 1.0f / 6.0f, 1.0f / 6.0f, 1.0f / 6.0f },
};

float Luminance(float3 v)
{
    return dot(v, float3(0.2125f, 0.7154f, 0.0721f));
}

PixeShaderOutput main(VertexShaderOutput input){
    uint32_t width, height;
    gTexture.GetDimensions(width, height);
    float2 uvStepsSize = float2(rcp(width), rcp(height));
    
    float2 differnece = float2(0.0f, 0.0f); // 縦横それぞれの畳み込みの結果を格納する

    for (int32_t x = 0; x < 3; ++x)
    {
        for (int32_t y = 0; y < 3; ++y)
        {
            float2 texcoord = input.texcoord + kIndex3x3[x][y] * uvStepsSize;
            float3 fechColor = gTexture.Sample(gSampler, texcoord).rgb;
            float luminance = Luminance(fechColor);
            differnece.x += luminance * kPrewittHorizontalKernel[x][y];
            differnece.y += luminance * kPrewittVerticalKernel[x][y];
        }
    }
    
    float weight = length(differnece);
    weight = saturate(weight * 6.0f);
    
    PixeShaderOutput output;
    output.color.rgb = (1.0f - weight) * gTexture.Sample(gSampler, input.texcoord).rgb;
    output.color.a = 1.0f;
   return output;
}