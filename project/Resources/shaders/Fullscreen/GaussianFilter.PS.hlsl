#include "Fullscreen.hlsli"
static const float PI = 3.14159265358979323846f;

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixeShaderOutput{
    float4 color : SV_TARGET0;
};

static const float2 kIndex3x3[3][3] = {
    { { -1.0f, -1.0f }, { 0.0f, -1.0f }, { 1.0f, -1.0f } },
    { { -1.0f, 0.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f } },
    { { -1.0f, 1.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f } },
};

float gauss(float x, float y, float sigma) {
    float exponent = -(x * x + y * y) * rcp(2.0f * sigma * sigma);
    float denominator = 2.0f * PI * sigma * sigma;
    return exp(exponent) * rcp(denominator);
};

PixeShaderOutput main(VertexShaderOutput input){
    PixeShaderOutput output;
    uint width, height;
    gTexture.GetDimensions(width, height);
    float2 uvStepsSize = float2(1.0f / width, 1.0f / height);
    // kenrnelを求める。weightは後で行う
    float weight = 0.0f;
    float kenrnel3x3[3][3];
    for (int32_t x = 0; x < 3; ++x) {
        for (int32_t y = 0; y < 3; ++y) {
            float2 offset = kIndex3x3[x][y];
            kenrnel3x3[x][y] = gauss(offset.x, offset.y, 2.0f);
            weight += kenrnel3x3[x][y];
        }
    } 
    // 求めたkenrnelを使い、BoxFilterと同じく畳み込みを行う。kKernel3x3を定数にしていたところがkenrnel3xに変わるだけである
    float3 result = float3(0.0f, 0.0f, 0.0f);
    for (int x = 0; x < 3; ++x) {
        for (int y = 0; y < 3; ++y) {
            float2 offset = kIndex3x3[x][y] * uvStepsSize;
            float3 color = gTexture.Sample(gSampler, input.texcoord + offset).rgb;
            result += color * kenrnel3x3[x][y];

        }
    } 
    // 畳み込み後の値を正規化する。本来gauss関数は全体を合計すると (積分) 1になるように設計されている。しかし、無限の範囲は足せないので、
    // kenrnel値の合計であるweightは1に満たない。なので、合計が1になるように逆数を掛けて全体を底上げして調整する    
    output.color.rgb = result / weight; 
    output.color.a = 1.0f;
    return output;
}