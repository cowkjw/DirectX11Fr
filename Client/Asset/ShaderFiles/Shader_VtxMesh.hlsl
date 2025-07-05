#include "Engine_Shader_Defines.hlsli"

matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;


texture2D g_DiffuseTexture;
texture2D g_SpecularTexture;
texture2D g_NormalTexture;
texture2D g_EmissiveTexture;
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


// 툰 쉐이딩을 위한 smoothstep 함수
float ToonStep(float threshold, float softness, float value)
{
    return smoothstep(threshold - softness, threshold + softness, value);
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

    float mask = g_SpecularTexture.Sample(DefaultSampler, uv0).r;
    if (mask < 0.1)
        discard;

   
    float2 swirlUV = uv0;
    float2 swirlOff = (g_NormalTexture.Sample(DefaultSampler, swirlUV).rg - 0.5) * 0.1f;


    //float2 noiseUV = uv0;
    //float  noiseVal = g_EmissiveTexture.Sample(DefaultSampler, noiseUV).r;
    //float2 noiseOff = (float2(noiseVal, noiseVal) - 0.5) * 0.5f;

    float4 aura = g_EmissiveTexture.Sample(DefaultSampler, uv0);

    float2 uvFinal = uv0 + swirlOff;//+ noiseOff;

    float4 col = g_DiffuseTexture.Sample(DefaultSampler, uvFinal);
    col.rgb += aura.rgb * 0.2f;
    col.a = mask;

    Out.vColor = col;
    return Out;
}


PS_OUT_PRE PS_MAIN_Effect_Nob(PS_IN In)
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
    float t = saturate(g_fTime / g_fDuration);

    // 시간에 따라 y줄여가지고 사라지는 느낌으로
    float uvScaleY = lerp(1.0, 0.0, t);
    float2 uv0 = (In.vTexcoord + g_vUVOffset) * float2(g_vUVScale.x, uvScaleY);

    float mask = g_SpecularTexture.Sample(DefaultSampler, uv0).r;
    if (mask < 0.1) discard;


    float2 swirlUV = uv0;
    float2 swirlOff = (g_NormalTexture.Sample(DefaultSampler, swirlUV).rg - 0.5) * 0.1f;


    //float2 noiseUV = uv0;
    //float  noiseVal = g_EmissiveTexture.Sample(DefaultSampler, noiseUV).r;
    //float2 noiseOff = (float2(noiseVal, noiseVal) - 0.5) * 0.5f;

    float4 aura = g_EmissiveTexture.Sample(DefaultSampler, uv0);

    float2 uvFinal = uv0 + swirlOff;//+ noiseOff;

    float4 col = g_DiffuseTexture.Sample(DefaultSampler, uvFinal);
    col.rgb += aura.rgb * 0.1f;
    col.a = mask;
    float fade = saturate(1.0 - g_fTime / g_fDuration);
    col.a *= fade;
    Out.vColor = col;
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


}
