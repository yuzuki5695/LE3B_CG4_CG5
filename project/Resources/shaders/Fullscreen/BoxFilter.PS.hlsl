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

static const float2 kIndex5x5[5][5] = {
    { { -2.0f, -2.0f }, { -1.0f, -2.0f }, { 0.0f, -2.0f }, { 1.0f, -2.0f }, { 2.0f, -2.0f } },
    { { -2.0f, -1.0f }, { -1.0f, -1.0f }, { 0.0f, -1.0f }, { 1.0f, -1.0f }, { 2.0f, -1.0f } },
    { { -2.0f, 0.0f }, { -1.0f, 0.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 2.0f, 0.0f } },
    { { -2.0f, 1.0f }, { -1.0f, 1.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f }, { 2.0f, 1.0f } },
    { { -2.0f, 2.0f }, { -1.0f, 2.0f }, { 0.0f, 2.0f }, { 1.0f, 2.0f }, { 2.0f, 2.0f } },
};

static const float kKernel5x5[5][5] = {
    { 1.0f / 25.0f, 1.0f / 25.0f, 1.0f / 25.0f, 1.0f / 25.0f, 1.0f / 25.0f },
    { 1.0f / 25.0f, 1.0f / 25.0f, 1.0f / 25.0f, 1.0f / 25.0f, 1.0f / 25.0f },
    { 1.0f / 25.0f, 1.0f / 25.0f, 1.0f / 25.0f, 1.0f / 25.0f, 1.0f / 25.0f },
    { 1.0f / 25.0f, 1.0f / 25.0f, 1.0f / 25.0f, 1.0f / 25.0f, 1.0f / 25.0f },
    { 1.0f / 25.0f, 1.0f / 25.0f, 1.0f / 25.0f, 1.0f / 25.0f, 1.0f / 25.0f },
};

PixeShaderOutput main(VertexShaderOutput input){
    uint32_t width, height; // 1. uvStepsSizeの算出
    gTexture.GetDimensions(width, height);
    float2 uvStepsSize = float2(rcp(width), rcp(height));
    
    PixeShaderOutput output;
    output.color.rgb = float3(0.0f, 0.0f, 0.0f);
    output.color.a = 1.0f;
    
    //for (int32_t x = 0; x < 3; ++x) {// 2. 3x3ループ
    //    for (int32_t y = 0; y < 3; ++y) {
    //        float2 texcoord = input.texcoord + kIndex3x3[x][y] * uvStepsSize; // 3. 現在のtexcoordを算出
    //        float3 fechColor = gTexture.Sample(gSampler, texcoord).rgb; // 4. 色に1/9掛けて足す
    //        output.color.rgb += fechColor * kKernel3x3[x][y];
    //    }
    //}
    
    for (int x = 0; x < 5; ++x) {// 2. 5x5ループ
        for (int y = 0; y < 5; ++y) {
            float2 offset = kIndex5x5[x][y] * uvStepsSize; // 3. 現在のtexcoordを算出
            float3 color = gTexture.Sample(gSampler, input.texcoord + offset).rgb; // 4. 色に1/9掛けて足す
            output.color.rgb += color * kKernel5x5[x][y];
        }
    }
    
   return output;
}