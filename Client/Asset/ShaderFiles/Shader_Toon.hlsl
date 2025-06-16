#include "Engine_Shader_Defines.hlsli"

// 기본 변환 행렬들
matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
matrix g_BoneMatrices[512];

// 텍스처
texture2D g_DiffuseTexture;

// 라이팅 파라미터들
float4 g_vLightDir;
float4 g_vLightDiffuse;
float4 g_vLightAmbient;
float4 g_vCamPosition;

// 머티리얼 속성
float4 g_vMtrlAmibient = float4(0.4f, 0.4f, 0.4f, 1.f);

// 툰 쉐이딩 전용 파라미터들
float g_fToonThreshold = 0.5f;        // 툰 쉐이딩 임계값
float g_fToonSoftness = 0.05f;        // 경계선 부드러움 정도
float4 g_vShadowColor = float4(0.5f, 0.5f, 0.65f, 1.0f);  // 그림자 색상 (약간 푸른빛)
float4 g_vHighlightColor = float4(1.05f, 1.05f, 1.0f, 1.0f); // 하이라이트 색상 (덜 밝게)
float g_fRimPower = 6.0f;             // 림 라이팅 강도
float4 g_vRimColor = float4(0.6f, 0.7f, 0.8f, 1.0f);     // 림 라이팅 색상 (덜 밝게)
float g_fOverallBrightness = 0.65f;   // 전체 밝기 조절

struct VS_IN
{
    float3 vPosition : POSITION;
    float3 vNormal : NORMAL;
    float3 vTangent : TANGENT;
    float2 vTexcoord : TEXCOORD0;
    uint4  vBlendIndices : BLENDINDEX;
    float4 vBlendWeights : BLENDWEIGHT;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;
    float4 vNormal : NORMAL;
    float2 vTexcoord : TEXCOORD0;
    float4 vWorldPos : TEXCOORD1;
    float3 vViewDir : TEXCOORD2;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;

    // 본 변환 계산
    float fWeightW = 1.f - (In.vBlendWeights.x + In.vBlendWeights.y + In.vBlendWeights.z);
    matrix BoneMatrix = g_BoneMatrices[In.vBlendIndices.x] * In.vBlendWeights.x +
        g_BoneMatrices[In.vBlendIndices.y] * In.vBlendWeights.y +
        g_BoneMatrices[In.vBlendIndices.z] * In.vBlendWeights.z +
        g_BoneMatrices[In.vBlendIndices.w] * fWeightW;

    vector vPosition = mul(vector(In.vPosition, 1.f), BoneMatrix);
    vector vNormal = mul(vector(In.vNormal, 0.f), BoneMatrix);

    // 변환 행렬 계산
    matrix matWV, matWVP;
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);

    // 출력 계산
    Out.vPosition = mul(vPosition, matWVP);
    Out.vNormal = normalize(mul(vNormal, g_WorldMatrix));
    Out.vTexcoord = In.vTexcoord;
    Out.vWorldPos = mul(vPosition, g_WorldMatrix);

    // 뷰 방향 계산 (림 라이팅용)
    Out.vViewDir = normalize(g_vCamPosition.xyz - Out.vWorldPos.xyz);

    return Out;
}

struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float4 vNormal : NORMAL;
    float2 vTexcoord : TEXCOORD0;
    float4 vWorldPos : TEXCOORD1;
    float3 vViewDir : TEXCOORD2;
};

struct PS_OUT
{
    vector vColor : SV_TARGET0;
};

// 툰 쉐이딩을 위한 smoothstep 함수
float ToonStep(float threshold, float softness, float value)
{
    return smoothstep(threshold - softness, threshold + softness, value);
}

PS_OUT PS_MAIN_TOON(PS_IN In)
{
    PS_OUT Out;

    // 디퓨즈 텍스처 샘플링
    vector vMtrlDiffuse = g_DiffuseTexture.Sample(LinearClampSampler, In.vTexcoord);

    // 알파 테스트 (필요시 주석 해제)
    /*if (vMtrlDiffuse.a < 0.1f)
        discard;*/

        // 노말 정규화
    float3 normal = normalize(In.vNormal.xyz);
    float3 lightDir = normalize(-g_vLightDir.xyz);

    // 기본 라이팅 계산
    float NdotL = dot(normal, lightDir);

    // 툰 쉐이딩 적용
    float toonShade = ToonStep(g_fToonThreshold, g_fToonSoftness, NdotL);

    // 3단계 쉐이딩 (귀멸의 칼날 스타일) - 밝기 조정
    float shadeLevel;
    if (NdotL > 0.7f)
        shadeLevel = 0.9f;      // 밝은 영역 (덜 밝게)
    else if (NdotL > 0.3f)
        shadeLevel = 0.6f;      // 중간 영역
    else
        shadeLevel = 0.35f;     // 어두운 영역

    // 부드러운 전환을 위한 보간
    float smoothShade = lerp(shadeLevel * 0.8f, shadeLevel,
        smoothstep(0.0f, 0.1f, abs(NdotL - 0.7f)) *
        smoothstep(0.0f, 0.1f, abs(NdotL - 0.3f)));

    // 림 라이팅 계산 (캐릭터 외곽선 강조)
    float3 viewDir = normalize(In.vViewDir);
    float rimDot = 1.0f - dot(viewDir, normal);
    float rimIntensity = pow(rimDot, g_fRimPower);
    rimIntensity = smoothstep(0.6f, 1.0f, rimIntensity);

    // 최종 색상 계산
    float4 baseColor = vMtrlDiffuse * g_vLightDiffuse;

    // 그림자 영역에 색조 변화 적용
    float4 shadedColor = lerp(baseColor * g_vShadowColor, baseColor, smoothShade);

    // 밝은 영역에 하이라이트 적용 (덜 강하게)
    if (smoothShade > 0.8f)
    {
        shadedColor = lerp(shadedColor, shadedColor * g_vHighlightColor,
            (smoothShade - 0.8f) * 2.0f); // 강도 감소
    }

    // 앰비언트 라이팅 추가 (덜 밝게)
    float4 ambientColor = g_vLightAmbient * g_vMtrlAmibient * vMtrlDiffuse * 0.5f;

    // 림 라이팅 적용 (덜 강하게)
    float4 rimLighting = g_vRimColor * rimIntensity * 0.2f;

    // 최종 색상 합성 (전체 밝기 조절)
    Out.vColor = (shadedColor + ambientColor + rimLighting) * g_fOverallBrightness;
    Out.vColor.a = vMtrlDiffuse.a;

    return Out;
}

PS_OUT PS_MAIN_WrapTOON(PS_IN In)
{
    PS_OUT Out;

    // 디퓨즈 텍스처 샘플링
    vector vMtrlDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);

    // 알파 테스트 (필요시 주석 해제)
    /*if (vMtrlDiffuse.a < 0.1f)
        discard;*/

        // 노말 정규화
    float3 normal = normalize(In.vNormal.xyz);
    float3 lightDir = normalize(-g_vLightDir.xyz);

    // 기본 라이팅 계산
    float NdotL = dot(normal, lightDir);

    // 툰 쉐이딩 적용
    float toonShade = ToonStep(g_fToonThreshold, g_fToonSoftness, NdotL);

    // 3단계 쉐이딩 밝기 조정
    float shadeLevel;
    if (NdotL > 0.7f)
        shadeLevel = 0.9f;      // 밝은 영역 (덜 밝게)
    else if (NdotL > 0.3f)
        shadeLevel = 0.6f;      // 중간 영역
    else
        shadeLevel = 0.35f;     // 어두운 영역

    // 부드러운 전환을 위한 보간
    float smoothShade = lerp(shadeLevel * 0.8f, shadeLevel,
        smoothstep(0.0f, 0.1f, abs(NdotL - 0.7f)) *
        smoothstep(0.0f, 0.1f, abs(NdotL - 0.3f)));

    // 림 라이팅 계산 (캐릭터 외곽선 강조)
    float3 viewDir = normalize(In.vViewDir);
    float rimDot = 1.0f - dot(viewDir, normal);
    float rimIntensity = pow(rimDot, g_fRimPower);
    rimIntensity = smoothstep(0.6f, 1.0f, rimIntensity);

    // 최종 색상 계산
    float4 baseColor = vMtrlDiffuse * g_vLightDiffuse;

    // 그림자 영역에 색조 변화 적용
    float4 shadedColor = lerp(baseColor * g_vShadowColor, baseColor, smoothShade);

    // 밝은 영역에 하이라이트 적용 (덜 강하게)
    if (smoothShade > 0.8f)
    {
        shadedColor = lerp(shadedColor, shadedColor * g_vHighlightColor,
            (smoothShade - 0.8f) * 2.0f); // 강도 감소
    }

    // 앰비언트 라이팅 추가 (덜 밝게)
    float4 ambientColor = g_vLightAmbient * g_vMtrlAmibient * vMtrlDiffuse * 0.5f;

    // 림 라이팅 적용 (덜 강하게)
    float4 rimLighting = g_vRimColor * rimIntensity * 0.2f;

    // 최종 색상 합성 (전체 밝기 조절)
    Out.vColor = (shadedColor + ambientColor + rimLighting) * g_fOverallBrightness;
    Out.vColor.a = vMtrlDiffuse.a;

    return Out;
}


PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out;
    vector vMtrlDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);

    float4 vShade = max(dot(normalize(g_vLightDir) * -1.f, In.vNormal), 0.f) +
        (g_vLightAmbient * g_vMtrlAmibient);

    Out.vColor = g_vLightDiffuse * vMtrlDiffuse * vShade;

    return Out;
}

PS_OUT PS_MAIN_CLAMP(PS_IN In)
{
    PS_OUT Out;
    vector vMtrlDiffuse = g_DiffuseTexture.Sample(LinearClampSampler, In.vTexcoord);

    float4 vShade = max(dot(normalize(g_vLightDir) * -1.f, In.vNormal), 0.f) +
        (g_vLightAmbient * g_vMtrlAmibient);

    Out.vColor = g_vLightDiffuse * vMtrlDiffuse * vShade;

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

    pass ClampDefault
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        PixelShader = compile ps_5_0 PS_MAIN_CLAMP();
    }

    // 새로운 툰 쉐이딩 패스
    pass ToonShading
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        PixelShader = compile ps_5_0 PS_MAIN_TOON();
    }
    
    pass ToonShadingWrap
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        PixelShader = compile ps_5_0 PS_MAIN_WrapTOON();
    }
}