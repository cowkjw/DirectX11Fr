#include "Engine_Shader_Defines.hlsli"

matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;


texture2D g_DiffuseTexture;
texture2D g_SpecularTexture;
texture2D g_NormalTexture;
float2 g_fUVOffset;

float4 g_vLightDir;
float4 g_vLightDiffuse;
float4 g_vLightAmbient;
float4 g_vLightSpecular;

float4 g_vCamPosition;
float4 g_vColor = float4(1.f, 1.f, 1.f, 1.f); // 기본 색상 (흰색)

const float4 g_vMtrlAmibient = float4(0.4f, 0.4f, 0.4f, 1.f);
const float4 g_vMtrlSpecular = float4(1.f, 1.f, 1.f, 1.f);


// 툰 쉐이딩 전용 파라미터들
float g_fToonThreshold = 0.5f;        // 툰 쉐이딩 임계값
float g_fToonSoftness = 0.05f;        // 경계선 부드러움 정도
float4 g_vShadowColor = float4(0.5f, 0.5f, 0.65f, 1.0f);  // 그림자 색상 (약간 푸른빛)
float4 g_vHighlightColor = float4(1.05f, 1.05f, 1.0f, 1.0f); // 하이라이트 색상 (덜 밝게)
float g_fRimPower = 3.0f;             // 림 라이팅 강도
float4 g_vRimColor = float4(0.6f, 0.7f, 0.8f, 1.0f);     // 림 라이팅 색상 (덜 밝게)
float g_fOverallBrightness = 0.85f;   // 전체 밝기 조절


struct VS_IN
{
    float3 vPosition : POSITION;
    float3 vNormal : NORMAL;
    float3 vTangent : TANGENT;
    float2 vTexcoord : TEXCOORD0;
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

    matrix matWV, matWVP;

    /* mul : 모든 행렬의 곱하기를 수행한다. /w연산을 수행하지 않는다. */
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);

    Out.vPosition = mul(vector(In.vPosition, 1.f), matWVP);
    Out.vNormal = normalize(mul(vector(In.vNormal, 0.f), g_WorldMatrix));
    Out.vTexcoord = In.vTexcoord;
    Out.vWorldPos = mul(vector(In.vPosition, 1.f), g_WorldMatrix);
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

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out;

    vector vMtrlDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    if (vMtrlDiffuse.a < 0.3f)
        discard;

    float4 vShade = max(dot(normalize(g_vLightDir) * -1.f, In.vNormal), 0.f) +
        (g_vLightAmbient * g_vMtrlAmibient);
    float4 vLook = In.vWorldPos - g_vCamPosition;

    float4 vReflect = reflect(normalize(g_vLightDir), normalize(In.vNormal));


    float4 vSpecular = pow(max(dot(normalize(vLook) * -1.f, vReflect), 0.f), 50.f);


    Out.vColor = g_vLightDiffuse * vMtrlDiffuse * vShade + (g_vLightSpecular * g_vMtrlSpecular) * vSpecular;

    return Out;
}


// 툰 쉐이딩을 위한 smoothstep 함수
float ToonStep(float threshold, float softness, float value)
{
    return smoothstep(threshold - softness, threshold + softness, value);
}

PS_OUT PS_MAIN_TOON(PS_IN In)
{
    PS_OUT Out;

    // 디퓨즈 텍스처 샘플링
    vector vMtrlDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);

    // 알파 테스트 (필요시 주석 해제)
    if (vMtrlDiffuse.a < 0.1f)
        discard;

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


PS_OUT PS_MAIN_Effect_MASK_NORMAL_DIFF(PS_IN In)
{
    PS_OUT Out;


    float2 uv = In.vTexcoord + g_fUVOffset;

    // 디스토션 계산
    float2 distortion = (g_SpecularTexture.Sample(DefaultSampler, uv).rg - 0.5f) * 0.064f;
    float2 distortedUV = uv + distortion;

    // 마스크 처리 (노말텍스쳐를 마스크로 사용 중)
    float mask = g_NormalTexture.Sample(DefaultSampler, distortedUV).r;

    if (mask < 0.1f)
        discard;

    // 디퓨즈와 스페큘러는 왜곡된 UV로 샘플링
    float4 vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, distortedUV);
    float3 vSpecular = g_SpecularTexture.Sample(DefaultSampler, distortedUV).rgb;

    vDiffuse.rgb *= mask;

    Out.vColor.rgb = vDiffuse.rgb + vSpecular * 0.2f;
    Out.vColor.a = vDiffuse.a * mask;


    return Out;
}

PS_OUT PS_MAIN_Effect_MASK_NORMAL(PS_IN In)
{
    PS_OUT Out;

    float2 uv = In.vTexcoord + g_fUVOffset;

    // 디스토션 적용
    // 왜곡이 0~1 기준으로 처리해서 없으면 텍스쳐가 0.5 아니면 왼쪽으로 -5 아니면 오른쪽으로 5
    float2 distortion = (g_SpecularTexture.Sample(DefaultSampler, uv).rg - 0.5f) * 0.5f;
    float2 distortedUV = uv + distortion;

    // 기본 컬러
    float4 vDiffuse = g_vColor;

    // 마스크
    float mask = g_NormalTexture.Sample(DefaultSampler, uv).r;
    if (mask < 0.333f)
        discard;

    Out.vColor = vDiffuse;
    Out.vColor.a *= mask;

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

	pass Toon
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		PixelShader = compile ps_5_0 PS_MAIN_TOON();
	}

    pass MaskNormalDiffEffect
    {
		SetRasterizerState(RS_Cull_None);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		PixelShader = compile ps_5_0 PS_MAIN_Effect_MASK_NORMAL_DIFF();
    }

     pass MaskNormalEffect
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        PixelShader = compile ps_5_0 PS_MAIN_Effect_MASK_NORMAL();
    }

}
