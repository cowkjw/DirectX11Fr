#include "Engine_Shader_Defines.hlsli"

matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;


texture2D g_DiffuseTexture; // 1번
texture2D g_SpecularTexture; // 8번
texture2D g_NormalTexture; // 6번
texture2D g_EmissiveTexture; // 4번
texture2D g_NoiseTexture;
texture2D g_DissolveTexture;
texture2D g_DissolveEdgeTexture;
float2 g_vUVOffset;
float2 g_vUVScale;

float  g_fTime;       // 누적 시간
float  g_fDuration;   // 총 지속 시간
float g_fCameraFar;

float4 g_vCamPosition;
float4 g_vColor = float4(1.f, 1.f, 1.f, 1.f); // 기본 색상 (흰색)


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

    vector  vMtrlDiffuse = g_DiffuseTexture.Sample(LinearClampSampler, In.vTexcoord);
    if (vMtrlDiffuse.a < 0.1f)
        discard;

    Out.vDiffuse = vMtrlDiffuse;

    /* -1.f -> 0.f, 1.f -> 1.f */
    Out.vNormal = vector(In.vNormal.xyz * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / g_fCameraFar, 0.f, 0.f);

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
    float fade = saturate(1.0 - g_fTime / g_fDuration);
    Out.vColor.a *= fade;
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
    float fade = saturate(1.0 - g_fTime / g_fDuration);
    Out.vDiffuse.a *= fade;
    if (Out.vDiffuse.b < 0.5f)
    {
        Out.vDiffuse.a *= 0.5f; // 파란색이 낮으면 알파도 낮추기
        Out.vDiffuse.rgb = saturate(Out.vDiffuse.rgb * 1.5f); // 알파 낮췄으니까 좀 더 밝게 보이게 하기
    }
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
    if (Out.vColor.b > 0.5f)
    {
        Out.vColor.a *= 0.8f;
        Out.vColor.rgb = saturate(Out.vColor.rgb * 1.5f); // 알파 낮췄으니까 좀 더 밝게 보이게 하기
    }
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

}
