#include "Engine_Shader_Defines.hlsli"
matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

texture2D g_DiffuseTexture;

float4 g_vLightDir;
float4 g_vLightDiffuse;
float4 g_vLightAmbient;
float4 g_vLightSpecular;

float4 g_vCamPosition;

float4 g_vMtrlAmibient = float4(0.4f, 0.4f, 0.4f, 1.f);
float4 g_vMtrlSpecular = float4(1.f, 1.f, 1.f, 1.f);



struct VS_IN
{
    float3 vPosition : POSITION;
    float3 vNormal : NORMAL;
    float2 vTexcoord : TEXCOORD0;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;
    float4 vNormal : NORMAL;
    float2 vTexcoord : TEXCOORD0;
    float4 vWorldPos : TEXCOORD1;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;

    matrix matWV, matWVP;

    /* mul : 모든 행렬의 곱하기를 수행한다. /w연산을 수행하지 않는다. */
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);

    Out.vPosition = mul(vector(In.vPosition, 1.f), matWVP);
    Out.vNormal = normalize(mul(vector(In.vNormal, 0.f), g_WorldMatrix));
    Out.vTexcoord = In.vTexcoord;
    Out.vWorldPos = mul(vector(In.vPosition, 1.f), g_WorldMatrix);

    return Out;
}

struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float4 vNormal : NORMAL;
    float2 vTexcoord : TEXCOORD0;
    float4 vWorldPos : TEXCOORD1;
};

struct PS_OUT
{
    vector vDiffuse : SV_TARGET0;
    vector vNormal : SV_TARGET1;
};

PS_OUT PS_MAIN(PS_IN In)
{
    //PS_OUT Out;

    //vector      vMtrlDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord * 15.f);

    //float4 vShade = max(dot(normalize(g_vLightDir) * -1.f, In.vNormal), 0.f) +
    //    (g_vLightAmbient * g_vMtrlAmibient);

    //float4 vLook = In.vWorldPos - g_vCamPosition;

    //float4 vReflect = reflect(normalize(g_vLightDir), normalize(In.vNormal));


    //float4 vSpecular = pow(max(dot(normalize(vLook) * -1.f, vReflect), 0.f), 50.f);


    //Out.vColor = g_vLightDiffuse * vMtrlDiffuse * vShade;// +(g_vLightSpecular * g_vMtrlSpecular) * vSpecular;

    PS_OUT Out;
    // 디퓨즈 텍스처 샘플링
    vector vMtrlDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord*15.f);

    //// 노말과 라이트 방향
    //float3 normal = normalize(In.vNormal.xyz);
    //float3 lightDir = normalize(-g_vLightDir.xyz);

    //float NdotL = max(0.0f, dot(normal, lightDir));

    //float isLit = step(g_fShadowThreshold, NdotL);

    //// 기본 색상
    //float4 baseColor = vMtrlDiffuse;

    //// 2단계 색상 정의
    //float4 litColor = baseColor * g_vLightDiffuse;      // 밝은 부분
    //float4 shadowColor = baseColor * g_vShadowColor;  // 그림자 부분

    //// 하드 컷 적용
    //float4 toonColor = lerp(shadowColor, litColor, isLit);

    //// 최소한의 앰비언트만 추가
    //float4 ambientColor = g_vLightAmbient * g_vMtrlAmibient * vMtrlDiffuse * g_fAmbientStrength;

    //// 최종 색상
    //float4 finalColor = toonColor + ambientColor;

    //// 채도 부스트
    //finalColor = AdjustSaturation(finalColor, g_fSaturationBoost);

    //Out.vColor = saturate(finalColor);
    //Out.vColor.a = vMtrlDiffuse.a;
    //if (vMtrlDiffuse.a < 0.3f)
    //	discard;
   
	if (vMtrlDiffuse.a < 0.3f)
		discard;

    Out.vDiffuse = vMtrlDiffuse;

    /* -1.f -> 0.f, 1.f -> 1.f */
    Out.vNormal = vector(In.vNormal.xyz * 0.5f + 0.5f, 1.f);

    return Out;
}



technique11 DefaultTechnique
{

    pass Default
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        PixelShader = compile ps_5_0 PS_MAIN();
    }

}
