#include "Fullscreen.hlsli"
static const float PI = 3.14159265358979323846f;

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixeShaderOutput
{
    float4 color : SV_TARGET0;
};

float gauss(float x, float y, float sigma)
{
    float exponent = -(x * x + y * y) * rcp(2.0f * sigma * sigma);
    float denominator = 2.0f * PI * sigma * sigma;
    return exp(exponent) * rcp(denominator);
};

// ガウシアンカーネル3x3（sigma=1.0に基づいて正規化された例）
static const float kKernel3x3[3][3] =
{
    { 1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0 },
    { 2.0 / 16.0, 4.0 / 16.0, 2.0 / 16.0 },
    { 1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0 },
};

static const float2 kIndex3x3[3][3] =
{
    { { -1.0f, -1.0f }, { 0.0f, -1.0f }, { 1.0f, -1.0f } },
    { { -1.0f, 0.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f } },
    { { -1.0f, 1.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f } },
};

PixeShaderOutput main(VertexShaderOutput input)
{
    uint width, height;
    gTexture.GetDimensions(width, height);
    float2 uvStepsSize = float2(1.0 / width, 1.0 / height);
    
    // kenrnelを求める。weightは後で行う
    float3 blurColor = float3(0.0f, 0.0f, 0.0f);

    // 3x3 カーネルループ
    for (int x = 0; x < 3; ++x)
    {
        for (int y = 0; y < 3; ++y)
        {
            // 求めたkenrnelを使い、BoxFilterと同じく畳み込みを行う。kKernel3x3を定数にしていたところがkenrnel3xに変わるだけである  
            float2 offsetUV = input.texcoord + kIndex3x3[y][x] * uvStepsSize;
            float3 sampleColor = gTexture.Sample(gSampler, offsetUV).rgb;
            blurColor += sampleColor * kKernel3x3[y][x];
        }
    }
    // 畳み込み後の値を正規化する。本来gauss関数は全体を合計すると (積分) 1になるように設計されている。しかし、無限の範囲は足せないので、
    // kenrnel値の合計であるweightは1に満たない。なので、合計が1になるように逆数を掛けて全体を底上げして調整する
    PixeShaderOutput output;
    output.color = float4(blurColor, 1.0f);
    return output;
}


