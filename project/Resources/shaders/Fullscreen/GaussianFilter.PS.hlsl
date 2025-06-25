#include "Fullscreen.hlsli"
static const float PI = 3.14159265358979323846f;

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixeShaderOutput{
    float4 color : SV_TARGET0;
};

float gauss(float x, float y, float sigma) {
    float exponent = -(x * x + y * y) * rcp(2.0f * sigma * sigma);
    float denominator = 2.0f * PI * sigma * sigma;
    return exp(exponent) * rcp(denominator);
};

PixeShaderOutput main(VertexShaderOutput input){
    uint width, height;
    gTexture.GetDimensions(width, height);
    float2 uvStepsSize = float2(1.0 / width, 1.0 / height); 
    
    // kenrnelを求める。weightは後で行う
    float sigma = 2.0f; // ぼかし強度（値を上げると広範囲にぼける）
    float3 blurColor = float3(0.0, 0.0, 0.0);
    float weightSum = 0.0f;
    
    // 3x3 カーネルループ
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float2 offset = float2(x, y);
            float2 sampleUV = input.texcoord + offset * uvStepsSize;

            float weight = gauss(offset.x, offset.y, sigma);
            float3 sampleColor = gTexture.Sample(gSampler, sampleUV).rgb; 
            // 求めたkenrnelを使い、BoxFilterと同じく畳み込みを行う。kKernel3x3を定数にしていたところがkenrnel3xに変わるだけである 
            blurColor += sampleColor * weight;
            weightSum += weight;
        }
    }

    // 畳み込み後の値を正規化する。本来gauss関数は全体を合計すると (積分) 1になるように設計されている。しかし、無限の範囲は足せないので、
    // kenrnel値の合計であるweightは1に満たない。なので、合計が1になるように逆数を掛けて全体を底上げして調整する
    
    PixeShaderOutput output;
    output.color.rgb = blurColor / weightSum; // 正規化
    output.color.a = 1.0f;
    return output;
}


