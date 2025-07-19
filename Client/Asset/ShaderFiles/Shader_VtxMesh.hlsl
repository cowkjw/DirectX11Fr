#include "Engine_Shader_Defines.hlsli"

matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;


texture2D g_DiffuseTexture; // 1번
texture2D g_SpecularTexture; // 8번
texture2D g_NormalTexture; // 6번
texture2D g_EmissiveTexture; // 4번
texture2D g_NoiseTexture;
texture2D g_DissolveTexture;
texture2D g_DissolveEdgeTexture;
texture2D g_AlphaTexture;
texture2D g_SmokeTexture;
float2 g_vUVOffset;
float2 g_vUVScale;

float  g_fTime;       // 누적 시간
float  g_fDuration;   // 총 지속 시간
float g_fCameraFar;
float g_DistFade;
int g_iMeshId;

float4 g_vCamPosition;
float4 g_vColor = float4(1.f, 1.f, 1.f, 1.f); // 기본 색상 (흰색)

// 추가적으로 소용돌이용 텍스처들
Texture2D g_WPOTexture;
Texture2D g_Alpha2Texture;
Texture2D g_FresnelTexture;
Texture2D g_DistortionTexture;
Texture2D g_LineTexture;

cbuffer CB_Vortex1 : register(b2)
{
    float g_Texture_LineParam = 0.4;    // 00------Texture_LineParam
    float g_Texture_Color_Ceil = 0.3;    // 00--00--Texture_Color_Ceil
    float g_Texture_Color_Ceil_Thr = 1.3;    // 00--01--Texture_Color_Ceil Threshold
    float g_VertexColor = 0.0;    // 02------VertexColor
    float g_TexAlphaThreshold = 0.3;    // 03------TexAlphaThreshold
    float g_TexAlphaThreshold2 = 0.95;   // 03------TexAlphaThreshold2
    float g_Emissive = 1.0;    // 04------Emissive

    float g_Scale_U_1 = 1.0;    // 01------Scale_U_1
    float g_Scale_V_1 = 2.0;    // 01------Scale_V_1
    float g_Offset_U_1 = 0.0;    // 01------Offset_U_1
    float g_Offset_V_1 = 0.0;    // 01------Offset_V_1

    float g_Scale_U_Line = 8.0;    // 01------Scale_U_Line
    float g_Scale_V_Line = 2.0;    // 01------Scale_V_Line

    float g_Scale_U_2 = 1.02;   // 02------Scale_U_2
    float g_Scale_V_2 = 4.0;    // 02------Scale_V_2
    float g_Offset_U_2 = -0.025; // 02------Offset_U_2
    float g_Offset_V_2 = 0.0;    // 02------Offset_V_2

    float g_WPOMultiply = -3.5;   // SP--00--WPO Multiply
    float g_AlphaMask = 1.0;    // SP--01--Alpha Mask

    float g_NoiseScale_U = 1.0;    // SP--02--Scale_U
    float g_NoiseScale_V = 6.0;    // SP--02--Scale_V
    float g_NoiseSpeed_U = -4.0;   // SP--02--Speed_U
    float g_NoiseSpeed_V = -0.2;   // SP--02--Speed_V

    float g_RefractionDepthBias = 0.0;    // RefractionDepthBias
};

// Mesh1 (Vortex003) 전용 파라미터
cbuffer CB_Vortex3 : register(b3)
{
    float g3_TextureAlphaParam = 4.22799;  // 01------Texture_Alpha_Param
    float g3_TextureAlphaParam2 = 1.0;      // 01------Texture_Alpha_Param2
    float g3_VertexColor = 0.628571; // 02------VertexColor

    float g3_OffsetU = 0.0;      // 01------Offset_U
    float g3_OffsetV = -0.7;     // 01------Offset_V
    float g3_OffsetU2 = 0.075;    // 01------Offset_U2
    float g3_OffsetV2 = 0.0;      // 01------Offset_V2

    float g3_ScaleU = 16.0;     // 01------Scale_U
    float g3_ScaleV = 2.0;      // 01------Scale_V
    float g3_ScaleU2 = 0.75;     // 01------Scale_U2
    float g3_ScaleV2 = 0.5;      // 01------Scale_V2

    float g3_SpeedU = -2.0;     // 01------Speed_U

    float g3_DistortionMulti = 0.0;      // 07--01--DistortionMulti
    float g3_DistortScaleU = 1.0;      // 07--00--Scale_U_1
    float g3_DistortScaleV = 1.0;      // 07--00--Scale_V_1
    float g3_DistortSpeedU = 0.0;      // 07--00--Speed_U_1
    float g3_DistortSpeedV = 0.0;      // 07--00--Speed_V_1

    float g3_RefractionDepthBias = 0.0;      // RefractionDepthBias
};

// Mesh2 (Vortex004) 전용 파라미터
cbuffer CB_Vortex4 : register(b4)
{
    float g4_TextureLineParam = 0.5;    // 00------Texture_LineParam
    float g4_TextureColorCeil = 0.5;    // 00--00--Texture_Color_Ceil
    float g4_TextureColorCeilThr = 18.0;   // 00--01--Texture_Color_Ceil Threshold

    float g4_TextureAlphaParam = 4.0;    // 01------Texture_Alpha_Param

    float g4_ScaleULine = 12.0;   // 01------Scale_U_Line
    float g4_ScaleVLine = 1.0;    // 01------Scale_V_Line
    float g4_OffsetULine = 0.0;    // 01------Offset_U_Line
    float g4_OffsetVLine = 0.0;    // 01------Offset_V_Line

    float g4_ScaleU = 18.0;   // 01------Scale_U
    float g4_ScaleV = 0.75;   // 01------Scale_V
    float g4_OffsetU = 0.0;    // 01------Offset_U
    float g4_OffsetV = 0.175;  // 01------Offset_V

    float g4_SpeedU = -10.0;  // 01------Speed_U
    float g4_SpeedV = 0.0;    // 01------Speed_V

    float g4_OffsetURGB = -0.02;  // 01------Offset_U_RGB
    float g4_OffsetVRGB = 0.175;  // 01------Offset_V_RGB
    float g4_ScaleURGB = 18.0;   // 01------Scale_U_RGB
    float g4_ScaleVRGB = 0.75;   // 01------Scale_V_RGB
    float g4_SpeedURGB = -10.0;  // 01------Speed_U_RGB
    float g4_SpeedVRGB = 0.0;    // 01------Speed_V_RGB

    float g4_FadeDistance = 0.0;    // 05------FadeDistance

    float g4_FresnelPower = 0.7;    // 06--03--FresnelPower

    float g4_DistortionMulti = 0.05;   // 07--01--DistortionMulti

    float g4_ScaleU1 = 1.0;    // 07--00--Scale_U_1
    float g4_ScaleV1 = 4.0;    // 07--00--Scale_V_1
    float g4_SpeedU1 = -0.5;   // 07--00--Speed_U_1
    float g4_SpeedV1 = 0.1;    // 07--00--Speed_V_1

    float g4_WPOMultiply = -2.0;   // SP--00--WPO Multiply

    float g4_NoiseScaleU = 1.0;    // SP--02--Scale_U
    float g4_NoiseScaleV = 5.0;    // SP--02--Scale_V
    float g4_NoiseSpeedU = -15.0;  // SP--02--Speed_U
    float g4_NoiseSpeedV = -0.2;   // SP--02--Speed_V
    float g4_TimeScale = 1.0;    // SP--02--TimeScale

    float g4_RefractionDepthBias = 0.0;    // RefractionDepthBias
};
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
    float4 vProjPos : TEXCOORD3;
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
	Out.vProjPos = Out.vPosition;
    return Out;
}

struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float4 vNormal : NORMAL;
    float2 vTexcoord : TEXCOORD0;
    float4 vWorldPos : TEXCOORD1;
    float3 vViewDir : TEXCOORD2;
	float4 vProjPos : TEXCOORD3;
};

struct PS_OUT
{
    vector vDiffuse : SV_TARGET0;
    vector vNormal : SV_TARGET1;
	vector vDepth : SV_TARGET2; // 깊이값을 저장할 타겟
};

struct PS_OUT_PRE
{
	vector vColor : SV_TARGET0;
};

struct PS_OUT_DISTORTION
{
    vector vColor : SV_TARGET0;
    vector vDistortion : SV_TARGET1; // 디스토션 효과를 위한 타겟
};

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out;

    vector  vMtrlDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    if (vMtrlDiffuse.a < 0.1f)
        discard;


    Out.vDiffuse = vMtrlDiffuse;
    /* -1.f -> 0.f, 1.f -> 1.f */
    Out.vNormal = vector(In.vNormal.xyz * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / g_fCameraFar, 0.f, 0.f);

    return Out;
}

PS_OUT PS_MAIN_INFI_ENV(PS_IN In)
{
    PS_OUT Out;

    vector  vMtrlDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    if (vMtrlDiffuse.a < 0.1f)
        discard;

    Out.vDiffuse = vMtrlDiffuse;
    float dist = length(In.vWorldPos.xyz - g_vCamPosition.xyz);
    float distFade = saturate(1.0 - dist / g_DistFade);
    Out.vDiffuse.a *= distFade;
    /* -1.f -> 0.f, 1.f -> 1.f */
    Out.vNormal = vector(In.vNormal.xyz * 0.5f + 0.5f, 0.5f);
    Out.vDepth  =  vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / g_fCameraFar, 0.f, 0.f);
   
    return Out;
}

PS_OUT PS_MAIN_TOON(PS_IN In)
{
    PS_OUT Out;

    vector  vMtrlDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    if (vMtrlDiffuse.a < 0.1f)
        discard;

    Out.vDiffuse = vMtrlDiffuse;
    /* -1.f -> 0.f, 1.f -> 1.f */
    Out.vNormal = vector(In.vNormal.xyz * 0.5f + 0.5f, 0.5f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / g_fCameraFar, 0.f, 0.f);
	return Out;
}

PS_OUT_PRE PS_MAIN_Effect_MASK_NORMAL_DIFF(PS_IN In)
{
    PS_OUT_PRE Out;


    float2 uv = In.vTexcoord + g_vUVOffset;

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

PS_OUT_PRE PS_MAIN_Effect_Fire(PS_IN In)
{
    PS_OUT_PRE Out;

    float2 uv = (In.vTexcoord + g_vUVOffset) * g_vUVScale;

    vector  Mask = g_SpecularTexture.Sample(DefaultSampler, uv);
    if (Mask.r < 0.1f)
        discard;
 //   float2 raw = g_NormalTexture.Sample(MirrSmp, uv).rg;
 //   float2 off = (raw - 0.5) * 0.5;
 //   float2 uvD = uv + off;

 //   vector  vMtrlDiffuse = g_DiffuseTexture.Sample(MirrSmp, uvD);
 //    마스크 처리

	//Out.vColor = vMtrlDiffuse;

    vector  vMtrlDiffuse = g_DiffuseTexture.Sample(DefaultSampler, uv);
    //if (vMtrlDiffuse.a < 0.1f)
    //    discard;

    Out.vColor = vMtrlDiffuse;
    return Out;
}

PS_OUT_PRE PS_MAIN_Effect_MASK_NORMAL(PS_IN In)
{
 //   PS_OUT_PRE Out;

 //   float2 uv = (In.vTexcoord + g_vUVOffset);
 //       float mask = g_NormalTexture.Sample(DefaultSampler, uv).r;
 //   if (mask < 0.1f)
 //       discard;
 //   // 디스토션 적용
 //   // 왜곡이 0~1 기준으로 처리해서 없으면 텍스쳐가 0.5 아니면 왼쪽으로 -5 아니면 오른쪽으로 5
 //   float2 distortion = (g_SpecularTexture.Sample(DefaultSampler, uv).rg - 0.5f) * 0.5f;
 //   float2 distortedUV = uv + distortion;
	//// distortedUV로 디퓨즈 텍스처 샘플링
	//float4 vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, distortedUV);


 //   // 기본 컬러
 //   vDiffuse*= g_vColor;

 //   // 마스크


 //   Out.vColor = vDiffuse;
 //   Out.vColor.a *= mask;

    PS_OUT_PRE Out;

    // 1) 기본 UV
    float2 uv0 = (In.vTexcoord + g_vUVOffset) * g_vUVScale;
    uv0.x *= 0.5f;

    float mask = g_DiffuseTexture.Sample(DefaultSampler, uv0).r;
    if (mask < 0.1) discard;
	//uv0.x *= 2.0f; // 원상복구
    float2 uvW = uv0;// +float2(0, -g_fTime * 0.5);
    float wave = g_EmissiveTexture.Sample(DefaultSampler, uvW).r;

    float2 uvS = uv0;// + float2(g_fTime * 0.2, 0);
    float2 swirl = (g_NormalTexture.Sample(DefaultSampler, uvS).rg - 0.5) * 0.2;

    // 노이즈
    float2 uvN = uv0;/// +float2(-g_fTime * 0.3, 0);
    float noise = (g_SpecularTexture.Sample(DefaultSampler, uvN).r - 0.5) * 0.5;

    float2 uvD = uv0 + swirl + float2(noise, noise);

    float4 col = g_vColor + wave;

    col.a = mask *wave;

    Out.vColor = col;
    return Out;

}

PS_OUT_PRE PS_MAIN_Effect_DistMask(PS_IN In)
{
    PS_OUT_PRE Out;

    //float2 uv = (In.vTexcoord + g_vUVOffset) * g_vUVScale;
    //vector  vMtrlSpec = g_SpecularTexture.Sample(DefaultSampler, uv);
    //if (vMtrlSpec.r < 0.1f)
    //    discard;
    //float2 distortion = (g_NormalTexture.Sample(DefaultSampler, uv).rg - 0.5f) * 0.5f;

    //float2 distortedUV = uv + distortion;
    //// uv로 일단 늘리기

    //vector  vMtrlDiffuse = g_DiffuseTexture.Sample(DefaultSampler, distortedUV);
    //// 마스크 처리


    //Out.vColor = vMtrlDiffuse;
    //return Out;

    float2 uv0 = In.vTexcoord + g_vUVOffset;

    //float mask = g_SpecularTexture.Sample(DefaultSampler, uv0).r;
    //if (mask < 0.1)
    //    discard;

   
    //float2 swirlUV = uv0;
    //float2 swirlOff = (g_NormalTexture.Sample(DefaultSampler, swirlUV).rg - 0.5) * 0.1f;

    //float4 aura = g_EmissiveTexture.Sample(DefaultSampler, uv0);

    //float2 uvFinal = uv0 + swirlOff;//+ noiseOff;

    //float4 col = g_DiffuseTexture.Sample(DefaultSampler, uvFinal);
    //col.rgb += aura.rgb * 0.2f;
    //col.a = mask;

    //Out.vColor = col;
    //return Out;

    //float2 uv0 = (In.vTexcoord + g_vUVOffset) * float2(g_vUVScale.x, uvScaleY);

    // 마스크
    float mask = g_DiffuseTexture.Sample(DefaultSampler, uv0).r;
    if (mask < 0.1)
        discard;

    // AuraSmoke  UV, 시간 기반 Offset
    float2 auraUV = uv0 + float2(-g_fTime * 0.2, -g_fTime * 0.2);

    // AuraSmoke 샘플
    float4 auraSmoke = g_EmissiveTexture.Sample(DefaultSampler, auraUV);

    // Diffuse 샘플
    float4 col = g_DiffuseTexture.Sample(DefaultSampler, auraUV);
    float4 specular = g_SpecularTexture.Sample(DefaultSampler, auraUV);

    // 밝기 추가 를 합산
    col.rgb += specular.rgb * 2.f;

    // 마스크와 페이드
    col.a = mask;


    Out.vColor = col;
    return Out;
}


PS_OUT_PRE PS_MAIN_Effect_Nob(PS_IN In)
{
//    PS_OUT_PRE Out;
//
//    //float2 uv = (In.vTexcoord + g_vUVOffset) * g_vUVScale;
//    //vector  vMtrlSpec = g_SpecularTexture.Sample(DefaultSampler, uv);
//    //if (vMtrlSpec.r < 0.1f)
//    //    discard;
//    //float2 distortion = (g_NormalTexture.Sample(DefaultSampler, uv).rg - 0.5f) * 0.5f;
//
//    //float2 distortedUV = uv + distortion;
//    //// uv로 일단 늘리기
//
//    //vector  vMtrlDiffuse = g_DiffuseTexture.Sample(DefaultSampler, distortedUV);
//    //// 마스크 처리
//
//
//    //Out.vColor = vMtrlDiffuse;
//    float t = saturate(g_fTime / g_fDuration);
//
//    // 시간에 따라 y줄여가지고 사라지는 느낌으로
//    float uvScaleY = lerp(1.0, 0.0, t);
//    float2 uv0 = (In.vTexcoord + g_vUVOffset) * float2(g_vUVScale.x, uvScaleY);
//
//    float mask = g_SpecularTexture.Sample(DefaultSampler, uv0).r;
//    if (mask < 0.1) discard;
//
//
//    float2 swirlUV = uv0;
//    float2 swirlOff = (g_NormalTexture.Sample(DefaultSampler, swirlUV).rg - 0.5) * 0.1f;
//
//
//    //float2 noiseUV = uv0;
//    //float  noiseVal = g_EmissiveTexture.Sample(DefaultSampler, noiseUV).r;
//    //float2 noiseOff = (float2(noiseVal, noiseVal) - 0.5) * 0.5f;
//
//    float4 aura = g_EmissiveTexture.Sample(DefaultSampler, uv0);
//
//    float2 uvFinal = uv0 + swirlOff;//+ noiseOff;
//
//    float4 col = g_DiffuseTexture.Sample(DefaultSampler, uvFinal);
//    col.rgb += aura.rgb * 0.1f;
//    col.a = mask;
//    float fade = saturate(1.0 - g_fTime / g_fDuration);
//    col.a *= fade;
//    Out.vColor = col;
//  float2 swirlOff = (g_NormalTexture.Sample(DefaultSampler, swirlUV).rg - 0.5) * 0.1f;
//
//// distortion 출력용
//float2 distortion = swirlOff;
    PS_OUT_PRE Out;

    float t = saturate(g_fTime / g_fDuration);
    float uvScaleY = lerp(1.0, 0.0, t);
    float2 uv0 = (In.vTexcoord + g_vUVOffset) * float2(g_vUVScale.x, uvScaleY);

    // 마스크
    float mask = g_DiffuseTexture.Sample(DefaultSampler, uv0).r;
    if (mask < 0.1)
        discard;

    // AuraSmoke  UV, 시간 기반 Offset
    float2 auraUV = uv0 + float2(-g_fTime * 0.2, -g_fTime * 0.2); 

    // AuraSmoke 샘플
    float4 auraSmoke = g_EmissiveTexture.Sample(DefaultSampler, auraUV);

    // Diffuse 샘플
    float4 col = g_DiffuseTexture.Sample(DefaultSampler, auraUV);
	float4 specular = g_SpecularTexture.Sample(DefaultSampler, auraUV);

    // 밝기 추가 를 합산
    col.rgb += specular.rgb*5.f ; 

    // 마스크와 페이드
    col.a = mask;
    float fade = saturate(1.0 - g_fTime / g_fDuration);
    col.a *= fade;

    Out.vColor = col;
	return Out;
}



PS_OUT_PRE PS_MAIN_Effect_MigCross(PS_IN In)
{
    PS_OUT_PRE Out;

    float2 uv0 = (In.vTexcoord + g_vUVOffset) * g_vUVScale;
    float mask = g_DiffuseTexture.Sample(DefaultSampler, uv0).r;

  
    if (mask < 0.33f)
        discard;
    float3 baseCol = g_vColor.rgb;
    Out.vColor = float4(baseCol, mask);
    //float fade = saturate(1.0 - g_fTime / g_fDuration);
  //  Out.vColor.a *= fade;
    return Out;
}

PS_OUT PS_MAIN_Effect_Mig(PS_IN In)
{
    //PS_OUT_PRE Out;
    PS_OUT Out;

    float2 uv0 = (In.vTexcoord + g_vUVOffset) * g_vUVScale;
    vector  vMtrlDiffuse = g_DiffuseTexture.Sample(DefaultSampler, uv0);
    if (vMtrlDiffuse.a < 0.1f)
        discard;

    Out.vDiffuse = vMtrlDiffuse;

    /* -1.f -> 0.f, 1.f -> 1.f */
    Out.vNormal = vector(In.vNormal.xyz * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / g_fCameraFar, 0.f, 0.f);
    //float fade = saturate(1.0 - g_fTime / g_fDuration);
    //Out.vDiffuse.a *= fade;

    return Out;

}

PS_OUT PS_MAIN_Effect_Dash(PS_IN In)
{
 //   PS_OUT Out;

 //   float2 uv0 = (In.vTexcoord + g_vUVOffset) * g_vUVScale;
 //   vector  vMtrlDiffuse = g_DiffuseTexture.Sample(DefaultSampler, uv0); // 라인
 //   if (vMtrlDiffuse.a < 0.1f)
 //       discard;
	//vector vMtrlSmoke = g_EmissiveTexture.Sample(DefaultSampler, uv0); // 연기
	//vector vMtrlGradiant = (g_NormalTexture.Sample(DefaultSampler, uv0).rg - 0.5) * 0.5f // 그라디언트
	//	+ 0.5f; // 0.0 ~ 1.0 범위로 조정
	//// 그라디언트 적용
	//vMtrlDiffuse.rgb *= vMtrlGradiant.r;
	//vMtrlDiffuse.a *= vMtrlGradiant.r;
	//// Out.vColor = vMtrlDiffuse;
	//Out.vDiffuse = vMtrlDiffuse * vMtrlGradiant.r

 //   /* -1.f -> 0.f, 1.f -> 1.f */
 //   Out.vNormal = vector(In.vNormal.xyz * 0.5f + 0.5f, 0.f);
 //   Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / g_fCameraFar, 0.f, 0.f);
 //   float fade = saturate(1.0 - g_fTime / g_fDuration);
 //   Out.vDiffuse.a *= fade;
 //   float3 baseCol = g_vColor.rgb;
	//Out.vDiffuse.rgb = lerp(Out.vDiffuse.rgb, baseCol, 0.5f); // 기본 색상과 섞기
 //   return Out;

    //PS_OUT Out;

    //// 1) UV 세팅
    //float2 uv0 = (In.vTexcoord + g_vUVOffset) * g_vUVScale;


    //float4 lineSample = g_DiffuseTexture.Sample(DefaultSampler, uv0);
    //if (lineSample.a < 0.1f)
    //    discard;


    //float smokeVal = g_NormalTexture.Sample(DefaultSampler, uv0).r;

    //float gradVal = g_SpecularTexture.Sample(DefaultSampler, uv0).r;

    //
    //float3 baseCol = lerp(lineSample.rgb, g_vColor.rgb, gradVal);

    //// 6) 노이즈·페이드 곱하기
    ////    smokeVal 로 RGB 떨림/흐림, alpha에도 곱해 꼬리 페이드
    //float  fade = saturate(1.0 - g_fTime / g_fDuration);
    //float3 finalRgb = baseCol * smokeVal;
    //float  finalA = lineSample.a * smokeVal * fade;

    //// 7) 결과
    //Out.vDiffuse = float4(finalRgb, finalA);
    //Out.vNormal = vector(In.vNormal.xyz * 0.5f + 0.5f, 0.f);
    //Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w,
    //    In.vProjPos.w / g_fCameraFar,
    //    0.f, 0.f);
    //return Out;

    PS_OUT Out;


 float2 uv0 = (In.vTexcoord + g_vUVOffset) * g_vUVScale;

    float mask = g_DiffuseTexture.Sample(DefaultSampler, uv0).r;
    if (mask < 0.1) discard;

    // 시간에 따른 페이드
    float fade = saturate(1.0 - g_fTime / g_fDuration);

    // 그라데이션
    float spec = g_SpecularTexture.Sample(DefaultSampler, uv0).r;

    // 기본 컬러 + 스페큘러 보강
    float3 baseColor = g_vColor.rgb;

    baseColor += spec * fade * 0.8;   

    // 최종 알파
    float alpha = mask * fade;

    Out.vDiffuse = float4(baseColor, alpha);
    Out.vNormal = vector(In.vNormal.xyz * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w,
        In.vProjPos.w / g_fCameraFar,
        0.f, 0.f);
    return Out;
}

PS_OUT_PRE PS_MAIN_Effect_Noise(PS_IN In)
{
    PS_OUT_PRE Out;

    float2 uv = (In.vTexcoord);

    // 노이즈 텍스처 샘플링
    float2 noiseUV = uv + float2(g_fTime * 0.1, -g_fTime * 0.15);
    float2 noise = (g_NoiseTexture.Sample(DefaultSampler, noiseUV).rg * 2.0 - 1.0) * 0.2;

    // UV에 노이즈를 더해 물결 효과
  
    float2 finalUV = uv + noise;

    vector vMtrlDiffuse = g_DiffuseTexture.Sample(DefaultSampler, finalUV);
    if (vMtrlDiffuse.a < 0.1f)
        discard;

	float2 dissolveUV = In.vTexcoord + g_vUVOffset;
	float dissolveMask = g_DissolveTexture.Sample(LinearClampSampler, dissolveUV).r;
	if (dissolveMask < 0.1f)
    {
		discard;
    }
    dissolveUV *= 10.f;
    vector vDissolveEdge = g_DissolveEdgeTexture.Sample(LinearClampSampler, dissolveUV);

    float edgeThreshold = 0.9f;
    float isEdge = step(dissolveMask, edgeThreshold);

    // 기본 디퓨즈 색상
    Out.vColor.rgb = vMtrlDiffuse.rgb;
    Out.vColor.a = vMtrlDiffuse.a;

    // 경계 영역에서 엣지 텍스처 적용
    if (isEdge > 0.3f)
    {
        // 엣지 텍스처의 흰색 영역만 사용
        Out.vColor.rgb = lerp(vMtrlDiffuse.rgb, float3(1.0f, 1.0f, 1.0f), vDissolveEdge.r);
        Out.vColor.a = vMtrlDiffuse.a;
    }

    return Out;
}

PS_OUT_PRE PS_MAIN_Diffuse(PS_IN In)
{
    PS_OUT_PRE Out;
    float2 uv = (In.vTexcoord  * g_vUVScale)+g_vUVOffset;
    float4 baseCol = g_DiffuseTexture.Sample(DefaultSampler, uv);
    if (baseCol.r < 0.1f && baseCol.g < 0.1f && baseCol.b < 0.1f)
        discard;

    Out.vColor = g_vColor;
    return Out;
}

PS_OUT_PRE PS_MAIN_DiffuseClamp(PS_IN In)
{
    PS_OUT_PRE Out;
    float2 uv = (In.vTexcoord * g_vUVScale) + g_vUVOffset;
    float4 baseCol = g_DiffuseTexture.Sample(LinearClampSampler, uv);
    if (baseCol.r < 0.1f && baseCol.g < 0.1f && baseCol.b < 0.1f)
        discard;

    Out.vColor = g_vColor;

	float t = saturate(g_fTime / g_fDuration);
	// 시간에 따라 알파 
	Out.vColor.a = lerp(1.0f, 0.0f, t);

    return Out;
}

PS_OUT_PRE PS_MAIN_Guard(PS_IN In)
{
    PS_OUT_PRE Out;
    float2 uv = (In.vTexcoord * g_vUVScale) + g_vUVOffset;
    float4 baseCol = g_DiffuseTexture.Sample(DefaultSampler, uv);
    if (g_iMeshId == 0) // 메인 메시니까 discard하고 마스크에 그냥 색상
    {
        if (baseCol.r < 0.1f && baseCol.g < 0.1f && baseCol.b < 0.1f)
            discard;

		Out.vColor = g_vColor;
    }
    else // 서브는 그라데이션으로 색상
    {
        float grad = baseCol.r;

        // 곱셈 계수 만들기 (검정=1, 흰=1.5)
        float bloomMul = lerp(1.0f, 1.5f, grad);

        Out.vColor.rgb = g_vColor.rgb * bloomMul;
        Out.vColor.a = baseCol.a * bloomMul;
    }


    return Out;
}

float2 g_vNoiseScale = float2(1.0f, 1.0f);      // Scale_U_1, Scale_V_1

float2 g_vNoiseSpeed = float2(-0.1f, 0.25f);    // Speed_U_1, Speed_V_1
float g_fDistortionMulti = 5.f;                  // DistortionMulti

PS_OUT_DISTORTION PS_MAIN_Effect_Distortion(PS_IN In)
{
    PS_OUT_DISTORTION Out;
	float2 uv = (In.vTexcoord + g_vUVOffset) * g_vUVScale;

	vector vMtrDiffuse = g_DiffuseTexture.Sample(DefaultSampler, uv);

	vector vMtrAlpha = g_AlphaTexture.Sample(DefaultSampler, uv);

    if (vMtrDiffuse.r < 0.1f )
        discard;
    Out.vColor = g_vColor;
	Out.vColor.a = vMtrAlpha.r; // 알파값은 알파 텍스쳐에서 가져옴

    float2 noiseUV = uv * g_vNoiseScale + g_fTime * g_vNoiseSpeed;
    float n = g_NoiseTexture.Sample(DefaultSampler, noiseUV).r;  // 0~1
    // X/Y 에 각각 다른 오프셋을 주고 싶으면 +float2(123.4, 567.8) 식으로 추가 오프셋
    float2 noiseUV2 = noiseUV ;
    float n2 = g_NoiseTexture.Sample(DefaultSampler, noiseUV2).r;

    // -0.5~+0.5 범위를 strength 만큼
    float2 dist = float2(n - 0.5, n2 - 0.5) * g_fDistortionMulti;
    // 알파에 따라 경계만 부드럽게
    dist *= vMtrAlpha.r;

    Out.vDistortion = float4(dist, 0, 0);

    return Out;
}

PS_OUT_DISTORTION PS_Vortex1(PS_IN In)
{
   PS_OUT_DISTORTION Out;

    float dissolveT = saturate((g_fTime / g_fDuration));

    //  UV 계산
    float2 uvBase  = In.vTexcoord * float2(g_Scale_U_1, g_Scale_V_1)
                   + float2(g_Offset_U_1, g_Offset_V_1);
    float2 uvNoise = In.vTexcoord * float2(g_NoiseScale_U, g_NoiseScale_V)
                   + float2(g_NoiseSpeed_U, g_NoiseSpeed_V) * g_fTime
                   + float2(g_Offset_U_2,  g_Offset_V_2);

    // 디졸브 마스크 
    float nMask = g_AlphaTexture.Sample(DefaultSampler, uvNoise).r;

	float t = saturate(g_fTime / g_fDuration);
	float alpha = lerp(1.0f, 0.0f, t);
    float2 wpo = g_WPOTexture.Sample(DefaultSampler, uvBase).rg - 0.5f;
    if (nMask > dissolveT)
    {
   
        float lum = g_DiffuseTexture.Sample(DefaultSampler, uvBase).r;
        float3 grad = float3(lum, lum, lum);
        float lineA = g_LineTexture.Sample(DefaultSampler, In.vTexcoord * float2(g_Scale_U_Line, g_Scale_V_Line)).r;
        float trailA = g_Alpha2Texture.Sample(DefaultSampler, uvNoise).r;

        Out.vColor.rgb = grad * trailA;
		Out.vColor.a = trailA * alpha; // 라인과 트레일 알파 곱하기
      
        Out.vDistortion = float4(wpo * g_WPOMultiply, 0, 0);
        return Out;
    }

    if (dissolveT < 1.0f)
    {
        Out.vColor = float4(0.5f, 0.5f, 0.5f, 0);
        
    }
    else
    {
       
        Out.vColor = float4(0.5f, 0.5f, 0.5f, 1.f);

    }
    Out.vDistortion = float4(wpo * g_WPOMultiply, 0, 0);

    return Out;
}

//// Vortex 두번째 메시
PS_OUT_PRE PS_Vortex3(PS_IN In)
{
    PS_OUT_PRE Out;

    //밝기
    float2 uvA = In.vTexcoord * float2(g3_ScaleU, g3_ScaleV)
        + float2(g3_OffsetU, g3_OffsetV);
    float lum = g_DiffuseTexture.Sample(DefaultSampler, uvA).r;
    if (lum < 0.1)            
        discard;
    float alpha0 = pow(lum, g3_TextureAlphaParam);

    // 마스크 두번째
    float2 uvB = In.vTexcoord * float2(g3_ScaleU2, g3_ScaleV2)
        + float2(g3_OffsetU2, g3_OffsetV2)
        + float2(g3_SpeedU, 0);
    float maskA = g_Alpha2Texture.Sample(DefaultSampler, uvB).r;
    if (maskA < 0.1)
        discard;

   // 컬러 & 알파
    Out.vColor.rgb = g_vColor.rgb * g3_VertexColor;
    Out.vColor.a = alpha0 * maskA;

   //// 디스토션
   // float2 uvD = In.vTexcoord * float2(g3_DistortScaleU, g3_DistortScaleV)
   //     + float2(g3_DistortSpeedU, g3_DistortSpeedV) * g_fTime;
   // float2 d = (g_DistortionTexture.Sample(DefaultSampler, uvD).rg - 0.5f)
   //     * g3_DistortionMulti;
   // Out.vDistortion = float4(d, 0, 0);
    float t = saturate(g_fTime / g_fDuration);
    float alpha = lerp(1.0f, 0.0f, t);
    Out.vColor.a *= alpha;
    return Out;
}

PS_OUT_PRE PS_Vortex4(PS_IN In)
{
    PS_OUT_PRE Out;

    //밝기
    float2 uvC = In.vTexcoord * float2(g4_ScaleU, g4_ScaleV)
        + float2(g4_OffsetU, g4_OffsetV);
    float lum = g_DiffuseTexture.Sample(DefaultSampler, uvC).r;
    float3 baseC = float3(lum, lum, lum);

    // Alpha
    float alpha0 = g_AlphaTexture.Sample(DefaultSampler, uvC).r;
    if (alpha0 < 0.1)
        discard;

    // Line mask
    float2 uvL = In.vTexcoord * float2(g4_ScaleULine, g4_ScaleVLine)
        + float2(g4_OffsetULine, g4_OffsetVLine);
    float lineMask = g_LineTexture.Sample(DefaultSampler, uvL).r;
    if (lineMask < g4_TextureLineParam)
        discard;

    //  Trail mask 
    float2 uvA2 = In.vTexcoord * float2(g4_ScaleURGB, g4_ScaleVRGB)
        + float2(g4_OffsetURGB, g4_OffsetVRGB)
        + float2(g4_SpeedURGB, g4_SpeedVRGB);
    float alpha2 = g_Alpha2Texture.Sample(DefaultSampler, uvA2).r;
    if (alpha2 < 0.1)
        discard;

    // 컬러 & 알파
    Out.vColor.rgb = baseC;
    Out.vColor.a = alpha0 * alpha2;
    Out.vColor.rgb = clamp(Out.vColor.rgb, 0, g4_TextureColorCeil);
    float thr = step(g4_TextureColorCeilThr, Out.vColor.r);
    Out.vColor.rgb = lerp(Out.vColor.rgb, baseC, thr);

    // 프레넬 강조
    float3 N = normalize(In.vNormal.xyz);
    float3 V = normalize(g_vCamPosition.xyz - In.vWorldPos.xyz);
    float fres = pow(1 - saturate(dot(N, V)), g4_FresnelPower);
    Out.vColor.rgb += fres;

    //// 픽셀 디스토션 (노이즈 + WPO)
    //float2 noiseUV = In.vTexcoord * float2(g4_NoiseScaleU, g4_NoiseScaleV)
    //    + float2(g4_NoiseSpeedU, g4_NoiseSpeedV) * (g_fTime * g4_TimeScale);
    //float2 distNoise = (g_DistortionTexture.Sample(DefaultSampler, noiseUV).rg - 0.5f)
    //    * g4_DistortionMulti;
    //float2 distWPO = (g_WPOTexture.Sample(DefaultSampler, uvC).rg - 0.5f)
    //    * g4_WPOMultiply;
    //Out.vDistortion = float4(distNoise + distWPO, 0, 0);
    float t = saturate(g_fTime / g_fDuration);
    float alpha = lerp(1.0f, 0.0f, t);
	Out.vColor.a *= alpha;
    return Out;
}


PS_OUT PS_TwistWind(PS_IN In)
{
    PS_OUT Out;

    float2 uv = In.vTexcoord * g_vUVScale
        + g_vUVOffset;
    uv.y += 0.25 * g_fTime;

    float mask = g_DiffuseTexture.Sample(DefaultSampler, uv).r;

   // 소프트 엣지로 경계 주기
    float lo = 0.7 - 0.6;
    float hi = 0.7 + 0.6;
    float alpha = saturate((mask - lo) / (hi - lo));

    //  거리 페이드 점점 사라짐
    float dist = length(In.vWorldPos.xyz - g_vCamPosition.xyz);
    float distFade = saturate(1.0 - dist / g_DistFade);
    alpha *= distFade;

    //  프레넬 강조 
    float3 N = normalize(In.vNormal.xyz);
    float3 V = normalize(g_vCamPosition.xyz - In.vWorldPos.xyz);
    float fres = pow(1 - saturate(dot(N, V)), 0.95);


    float3 baseCol = g_vColor.rgb * fres;
    Out.vDiffuse = float4(baseCol, alpha);

   
    Out.vNormal = float4(N * 0.5 + 0.5, 0);
    Out.vDepth  = float4(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / g_fCameraFar, 0, 0);
    return Out;
}

PS_OUT_DISTORTION PS_MAIN_MaskDist(PS_IN In)
{
    PS_OUT_DISTORTION Out;
	float2 uv = In.vTexcoord * g_vUVScale + g_vUVOffset;

    float mask = g_DiffuseTexture.Sample(DefaultSampler, uv).r;
    if (mask <= 0.1f)
        discard;

    float2 d = g_DistortionTexture.Sample(DefaultSampler, uv*2.f).rg - 0.5f;


    // 4) 최종 컬러
    Out.vColor = g_vColor;
    Out.vDistortion = vector(d, 0.0f, 0.0f); // 왜곡 벡터를 출력
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

    pass FireEffect
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        PixelShader = compile ps_5_0 PS_MAIN_Effect_Fire();
    }

    pass KienFireEffect
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        PixelShader = compile ps_5_0 PS_MAIN_Effect_DistMask();
    }

    pass NobFireEffect
    {

		SetRasterizerState(RS_Cull_None);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		PixelShader = compile ps_5_0 PS_MAIN_Effect_Nob();
    }


    pass MigCrossEffect
    {

        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        PixelShader = compile ps_5_0 PS_MAIN_Effect_MigCross();
    }

    pass MigEffect
    {

        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        PixelShader = compile ps_5_0 PS_MAIN_Effect_Mig();
    }

    pass DashEffect
    {

        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        PixelShader = compile ps_5_0 PS_MAIN_Effect_Dash();
    }

    pass NoiseEffect
    {
		SetRasterizerState(RS_Cull_None);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		PixelShader = compile ps_5_0 PS_MAIN_Effect_Noise();
    }
    
    pass DiffuseMask
    {
		SetRasterizerState(RS_Cull_None);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		PixelShader = compile ps_5_0 PS_MAIN_Diffuse();
    }

    pass DiffuseMaskClamp
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        PixelShader = compile ps_5_0 PS_MAIN_DiffuseClamp();
    }

    pass Guard
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        PixelShader = compile ps_5_0 PS_MAIN_Guard();
    }

    pass Distortion  //14
    {
		SetRasterizerState(RS_Cull_None);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		PixelShader = compile ps_5_0 PS_MAIN_Effect_Distortion();
    }


        pass Vortex1
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default,0);
        SetBlendState(BS_AlphaBlend, float4(0,0,0,0),0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        PixelShader = compile ps_5_0 PS_Vortex1();
    }

        pass Vortex3
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default,0);
        SetBlendState(BS_AlphaBlend, float4(0,0,0,0),0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        PixelShader = compile ps_5_0 PS_Vortex3();
    }

        pass Vortex4
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default,0);
        SetBlendState(BS_AlphaBlend, float4(0,0,0,0),0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        PixelShader = compile ps_5_0 PS_Vortex4();
    }

	pass TwistWind // 18
	{
		SetRasterizerState(RS_Cull_None);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		PixelShader = compile ps_5_0 PS_TwistWind();
	}

    pass SwingWind // 19
    {
		SetRasterizerState(RS_Cull_None);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		PixelShader = compile ps_5_0 PS_MAIN_MaskDist();
    }

    pass INFI_ENV // 20
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        PixelShader = compile ps_5_0 PS_MAIN_INFI_ENV();
    }
}
