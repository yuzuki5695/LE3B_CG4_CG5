#include "Object3d.hlsli"

struct Material
{
    float4 color;
    int endbleLighting;
    float32_t4x4 uvTransform;
    float shininess;
};

struct DirectionalLight
{
    float4 color; //!< ライトの色
    float3 direction; //!< ライトの向き
    float intensity; //!< 輝度
};

struct Camera
{
    float3 worldPosition;
};

struct PointLight
{
    float4 color; //!< ライトの色
    float3 position; //!< ライトの位置
    float intensity; //!< 輝度
    float radius; //!< ライトの届く最大距離
    float decay; //!< 減衰率
};

struct SpotLight
{
    float4 color; //!< ライトの色
    float3 position; //!< ライトの位置
    float intensity; //!< 輝度
    float3 direction; //!< スポットライトの向き
    float radius; //!< ライトの届く最大距離
    float decay; //!< 減衰率
    float cosAngle; //!< スポットライトの余弦
    float cosFalloffStart; //!<Falloff開始
};

ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);
ConstantBuffer<Camera> gCamera : register(b2);
ConstantBuffer<PointLight> gPointLight : register(b3);
ConstantBuffer<SpotLight> gSpotLight : register(b4);

struct PixeShaderOutput
{
    float4 color : SV_TARGET0;
};

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

PixeShaderOutput main(VertexShaderOutput input)
{
    //TextureをSampling
    float4 TransformesUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float4 textureColor = gTexture.Sample(gSampler, TransformesUV.xy);
    
    PixeShaderOutput output;
     
    // textureのα値が0.5以下のときにpixelを棄却
    if (textureColor.a <= 0.5)
    {
        discard;
    }

    // textureのα値が0のときにpixelを棄却
    if (textureColor.a == 0.0)
    {
        discard;
    }
    
    // output.colorのα値が0のときにpixelを棄却
    if (output.color.a == 0.0)
    {
        discard;
    }
    
    if (gMaterial.endbleLighting != 0)
    { // Linhthingする場合
        // half lambert
        float Ndont = dot(normalize(input.normal), normalize(-gDirectionalLight.direction));
        float cos = pow(Ndont * 0.5f + 0.5f, 2.0f);
        float3 toEve = normalize(gCamera.worldPosition - input.worldPosition);
        
        ///-----------------------------------------------------------------------------------///
        ///------------------------------ディレクショナルライト-----------------------------------///
        ///-----------------------------------------------------------------------------------///
        // 拡散反射（ディレクショナルライト）
        float3 diffuse =
        gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
        // 鏡面反射（ディレクショナルライト）
        float3 halfVector = normalize(-gDirectionalLight.direction + toEve);
        float NDotH = dot(normalize(input.normal), halfVector);
        float specularPow = pow(saturate(NDotH), gMaterial.shininess); // 反射強度
        float3 specular =
        gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPow * float3(1.0f, 1.0f, 1.0f);

        
        ///-----------------------------------------------------------------------------------///
        ///----------------------------------ポイントライト-------------------------------------///
        ///-----------------------------------------------------------------------------------///
        
        float3 pointLightDirection = normalize(input.worldPosition - gPointLight.position);
        // 再計算
        Ndont = dot(normalize(input.normal), -pointLightDirection);
        cos = pow(Ndont * 0.5f + 0.5f, 2.0f);
        
        // 拡散反射（ポイントライト）
        float distance = length(gPointLight.position - input.worldPosition); // ポイントライトへの距離
        float factor = pow(saturate(-distance / gPointLight.radius + 1.0), gPointLight.decay); // 指数によるコントロール
        float3 pointLightDiffuse =
        gMaterial.color.rgb * textureColor.rgb * gPointLight.color.rgb * cos * gPointLight.intensity * factor;
        // 鏡面反射（ポイントライト）
        float3 pointLightHalfVector = normalize(-pointLightDirection + toEve);
        float pointLightNDotH = dot(normalize(input.normal), pointLightHalfVector);
        float pointLightSpecularPow = pow(saturate(pointLightNDotH), gMaterial.shininess);
        float3 pointLightSpecular =
        gPointLight.color.rgb * gPointLight.intensity * pointLightSpecularPow * float3(1.0f, 1.0f, 1.0f) * factor;
         
        
        ///-----------------------------------------------------------------------------------///
        ///----------------------------------スポットライト-------------------------------------///
        ///-----------------------------------------------------------------------------------///
        
        // 再計算
        Ndont = dot(normalize(input.normal), -gSpotLight.direction);
        cos = pow(Ndont * 0.5f + 0.5f, 2.0f);
        
        float3 spotLightDirectionOnSurface = normalize(input.worldPosition - gSpotLight.position);
        float cosAngle = dot(spotLightDirectionOnSurface, gSpotLight.direction);
        float falloffFactor = saturate((cosAngle - gSpotLight.cosAngle) / (gSpotLight.cosFalloffStart - gSpotLight.cosAngle));
        float attenuationFactor = gSpotLight.intensity * falloffFactor;
        
        // 拡散反射(スポットライト)
        float spotLightdistance = length(gSpotLight.position - input.worldPosition); // ポイントライトへの距離
        float spotLightfactor = pow(saturate(-spotLightdistance / gSpotLight.radius + 1.0), gSpotLight.decay); // 指数によるコントロール
        float3 spotLightDiffuse =
        gMaterial.color.rgb * textureColor.rgb * gSpotLight.color.rgb * attenuationFactor * gSpotLight.intensity * spotLightfactor;
        
        // 鏡面反射（スポットライト）
        float3 spotLightHalfVector = normalize(-spotLightDiffuse + toEve);
        float spotLightNDotH = dot(normalize(input.normal), spotLightHalfVector);
        float spotLightSpecularPow = pow(saturate(spotLightNDotH), gMaterial.shininess);
        float3 spotLightSpecular =
        gPointLight.color.rgb * gPointLight.intensity * spotLightSpecularPow * float3(1.0f, 1.0f, 1.0f) * spotLightfactor;
        
        // ライト
        output.color.rgb = diffuse + specular + pointLightDiffuse + pointLightSpecular + spotLightDiffuse + spotLightSpecular;
        output.color.a = gMaterial.color.a * textureColor.a;
    }
    else
    { // Linhthingしない場合、前回までと同じ演算
        output.color = gMaterial.color * textureColor;
    }
    return output;
}