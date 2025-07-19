#include "Engine_Shader_Defines.hlsli"

matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
texture2D g_Texture : register(t0);
texture2D g_MaskTexture : register(t1);
texture2D g_DistortionTexture;
texture2D g_NoiseTexture;

vector g_vCamPosition;

cbuffer CB_UV : register(b0)
{
    float2 g_uvOffset;
    float2 g_uvScale;      // UV 스케일 (default는 (1,1))
}

struct VS_IN
{
    float3 vPosition : POSITION;

    row_major float4x4 TransformMatrix : WORLD;


    float2 vLifeTime : TEXCOORD0;
    float3 vStartColor : TEXCOORD1;
    float3 vEndColor : TEXCOORD2;
    float  fAlphaVar : TEXCOORD3;
};

struct VS_OUT
{
    /* SV_ : ShaderValue약자 */
    /* 내가 해야할 연산은 다 했으니 이제 니(장치)가 알아서 추가적인 연산을 해라. */
    float4 vPosition : POSITION;

    float2 vPSize : PSIZE;

    float2 vLifeTime : TEXCOORD0;

    float3 vStartColor : TEXCOORD1;
    float3 vEndColor : TEXCOORD2;
    float  fAlphaVar : TEXCOORD3;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;

    matrix matWV, matWVP;

    vector vPosition = mul(vector(In.vPosition, 1.f), In.TransformMatrix);

    Out.vPosition = mul(vPosition, g_WorldMatrix);

    Out.vPSize = float2(length(In.TransformMatrix._11_12_13), length(In.TransformMatrix._21_22_23));

    Out.vLifeTime = In.vLifeTime;
	Out.vStartColor = In.vStartColor;
	Out.vEndColor = In.vEndColor;
	Out.fAlphaVar = In.fAlphaVar;

    return Out;
}

/* 그리는 형태에 따라서 호출된다. */

struct GS_IN
{
    float4 vPosition : POSITION;

    float2 vPSize : PSIZE;

    float2 vLifeTime : TEXCOORD0;


    float3 vStartColor : TEXCOORD1;
    float3 vEndColor : TEXCOORD2;
    float  fAlphaVar : TEXCOORD3;
};

struct GS_OUT
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
    float2 vLifeTime : TEXCOORD1;
    float3 vStartColor : TEXCOORD2;
    float3 vEndColor : TEXCOORD3;
    float  fAlphaVar : TEXCOORD4;
};

[maxvertexcount(6)]
void GS_MAIN(point GS_IN In[1], inout TriangleStream<GS_OUT> Triangles)
{
    GS_OUT Out[4];

    float3 vLook = g_vCamPosition.xyz - In[0].vPosition.xyz;
    float3 vRight = normalize(cross(float3(0.f, 1.f, 0.f), vLook)) * In[0].vPSize.x * 0.5f;
    float3 vUp = normalize(cross(vLook, vRight)) * In[0].vPSize.y * 0.5f;

    matrix matVP = mul(g_ViewMatrix, g_ProjMatrix);

    Out[0].vPosition = mul(float4(In[0].vPosition.xyz + vRight + vUp, 1.f), matVP);
    Out[0].vTexcoord = float2(0.f, 0.f);
    Out[0].vLifeTime = In[0].vLifeTime;


    Out[1].vPosition = mul(float4(In[0].vPosition.xyz - vRight + vUp, 1.f), matVP);
    Out[1].vTexcoord = float2(1.f, 0.f);
    Out[1].vLifeTime = In[0].vLifeTime;

    Out[2].vPosition = mul(float4(In[0].vPosition.xyz - vRight - vUp, 1.f), matVP);
    Out[2].vTexcoord = float2(1.f, 1.f);
    Out[2].vLifeTime = In[0].vLifeTime;

    Out[3].vPosition = mul(float4(In[0].vPosition.xyz + vRight - vUp, 1.f), matVP);
    Out[3].vTexcoord = float2(0.f, 1.f);
    Out[3].vLifeTime = In[0].vLifeTime;
    
    for (int i = 0; i < 4; i++)
    {
        Out[i].vStartColor = In[0].vStartColor;
        Out[i].vEndColor = In[0].vEndColor;
        Out[i].fAlphaVar = In[0].fAlphaVar;
    }

    Triangles.Append(Out[0]);
    Triangles.Append(Out[1]);
    Triangles.Append(Out[2]);
    Triangles.RestartStrip();

    Triangles.Append(Out[0]);
    Triangles.Append(Out[2]);
    Triangles.Append(Out[3]);
    Triangles.RestartStrip();
}


struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
    float2 vLifeTime : TEXCOORD1;
	float3 vStartColor : TEXCOORD2;
	float3 vEndColor : TEXCOORD3;
	float  fAlphaVar : TEXCOORD4;
};

struct PS_OUT
{
    vector vColor : SV_TARGET0;
};

struct PS_OUT_DISTORTION
{
	vector vColor : SV_TARGET0;
    vector vDistortion : SV_TARGET01;
};

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out;

    Out.vColor = g_Texture.Sample(DefaultSampler, In.vTexcoord);

    // 알파 컷
    if (Out.vColor.a < 0.3f)
        discard;

    // 시간 비율 계산
    float t = saturate(In.vLifeTime.y / In.vLifeTime.x);

    // 색상 보간
    float3 interpColor = lerp(In.vStartColor, In.vEndColor, t);
    Out.vColor.rgb *= interpColor;

    // 알파 보간 + alpha variation 적용
    float alpha = (1.f - t) * lerp(1.0f, In.fAlphaVar, t);
    Out.vColor.a *= alpha;

    // 최종 생명 종료 조건
    if (In.vLifeTime.y >= In.vLifeTime.x)
        discard;
    return Out;
}

PS_OUT PS_MAIN_MASK(PS_IN In)
{
    PS_OUT Out;

    Out.vColor = g_Texture.Sample(DefaultSampler, In.vTexcoord);

    // r로 마스크 처리
    if (Out.vColor.r < 0.3f)
        discard;

    // 시간 비율 계산
    float t = saturate(In.vLifeTime.y / In.vLifeTime.x);

    // 색상 보간
    float3 interpColor = lerp(In.vStartColor, In.vEndColor, t);
	Out.vColor.r = interpColor.r;
    Out.vColor.gb += interpColor.gb;

    // 알파 보간 + alpha variation 적용
    float alpha = (1.f - t) * lerp(1.0f, In.fAlphaVar, t);
    Out.vColor.a *= alpha;

    // 최종 생명 종료 조건
    if (In.vLifeTime.y >= In.vLifeTime.x)
        discard;

    return Out;
}


PS_OUT PS_MAIN_MASK2(PS_IN In)
{
	PS_OUT Out;
    float4 tex = g_Texture.Sample(DefaultSampler, In.vTexcoord);
    if (tex.r < 0.1f)
        discard;

    // 비율 (0center, 1outer)
	// 알파를 거리 비율로 사용
    float dist = distance(In.vTexcoord, float2(0.5f,0.5f)) / In.fAlphaVar;
    dist = saturate(dist);

    // 중심은 StartColor, 외곽은 EndColor
    float3 radialColor = lerp(In.vStartColor, In.vEndColor, dist);

 //   float lifeT = saturate(In.vLifeTime.y / In.vLifeTime.x);
 //   float3 lifeColor = lerp(radialColor, In.vEndColor, lifeT);
 //   float3 finalRGB = tex.a * lifeColor;
 //   float  finalA = tex.a;
    float3 finalRGB = tex.a * radialColor;
    float  finalA = tex.a;
    Out.vColor = float4(finalRGB, finalA);
    //Out.vColor = float4(finalRGB, finalA);

	return Out;
}

PS_OUT PS_MAIN_MASK_UV(PS_IN In)
{
    if (In.vLifeTime.y >= In.vLifeTime.x)
        discard;
    PS_OUT Out;
    float2 uv;
    uv.x = In.vTexcoord.x * g_uvScale.x + g_uvOffset.x;
    uv.y = In.vTexcoord.y * g_uvScale.y + g_uvOffset.y;
    float4 tex = g_Texture.Sample(DefaultSampler, uv);
    if (tex.r < 0.1f)
        discard;

    // 비율 (0center, 1outer)
    // 알파를 거리 비율로 사용
    float dist = distance(uv, float2(0.5f, 0.5f)) / In.fAlphaVar;
    dist = saturate(dist);

    // 중심은 StartColor, 외곽은 EndColor
    float3 radialColor = lerp(In.vStartColor, In.vEndColor, dist);

    float3 finalRGB = tex.a * radialColor;
    float  finalA = tex.a;
    Out.vColor = float4(finalRGB, finalA);
    //Out.vColor = float4(finalRGB, finalA);

    return Out;
}

PS_OUT PS_MAIN_MASK4(PS_IN In)
{
    PS_OUT Out;

    Out.vColor = g_Texture.Sample(DefaultSampler, In.vTexcoord);

    // r로 마스크 처리
    if (Out.vColor.r < 0.3f)
        discard;

    // 시간 비율 계산
    float t = saturate(In.vLifeTime.y / In.vLifeTime.x);

    // 색상 보간
    float3 interpColor = lerp(In.vStartColor, In.vEndColor, t);
    Out.vColor.r = interpColor.r;
    Out.vColor.gb += interpColor.gb;

    // 알파 보간 + alpha variation 적용
    float alpha = (1.f - t) * lerp(1.0f, In.fAlphaVar, t);
    Out.vColor.a *= alpha;

    // 최종 생명 종료 조건
    if (In.vLifeTime.y >= In.vLifeTime.x)
        discard;

    return Out;
}
float2 g_DistortSpeed = { 0.3, 0.2 };
float2 g_NoiseSpeed = { 0.5, 0.5 };

PS_OUT PS_DistDiffNoiseMask(PS_IN In)
{
    PS_OUT Out;
	float2 uv = (In.vTexcoord*g_uvScale) + g_uvOffset;

	vector mask = g_MaskTexture.Sample(DefaultSampler, uv);
	if (mask.r < 0.3f)
		discard;
    float2 distUV = In.vTexcoord * 4.0f
        + In.vLifeTime.y * g_DistortSpeed;
    float2 distortion = g_DistortionTexture
        .Sample(DefaultSampler, distUV)
        .rg * 2.0f - 1.0f;

	uv.x += distortion.r * 0.2f; // distortion을 이용해 UV를 변형
	uv.y += distortion.g * 0.2f; // distortion을 이용해 UV를 변형

    float2 noiseUV = In.vTexcoord * 2.0f
        + In.vLifeTime.y * g_NoiseSpeed;
    float2 noise = g_NoiseTexture
        .Sample(DefaultSampler, noiseUV)
        .rg * 2.0f - 1.0f;
	uv += noise * 0.1f; 

	Out.vColor = g_Texture.Sample(DefaultSampler, uv);
    // 시간 비율 계산
    float t = saturate(In.vLifeTime.y / In.vLifeTime.x);
    float alpha = (1.f - t) * lerp(1.0f, In.fAlphaVar, t);
   Out.vColor.rgb += alpha;

    // 최종 생명 종료 조건
    if (In.vLifeTime.y >= In.vLifeTime.x)
        discard;

    return Out;

}


PS_OUT PS_DistNoiseMask(PS_IN In)
{
    PS_OUT Out;
    float2 uv = (In.vTexcoord * g_uvScale) + g_uvOffset;

    vector mask = g_MaskTexture.Sample(DefaultSampler, uv);
    if (mask.r < 0.3f)
        discard;
    float2 distUV = In.vTexcoord * 4.0f
        + In.vLifeTime.y * g_DistortSpeed;
    float2 distortion = g_DistortionTexture
        .Sample(DefaultSampler, distUV)
        .rg * 2.0f - 1.0f;

    uv.x += distortion.r * 0.2f; // distortion을 이용해 UV를 변형
    uv.y += distortion.g * 0.2f; // distortion을 이용해 UV를 변형

    float2 noiseUV = In.vTexcoord * 2.0f
        + In.vLifeTime.y * g_NoiseSpeed;
    float2 noise = g_NoiseTexture
        .Sample(DefaultSampler, noiseUV)
        .rg * 2.0f - 1.0f;
    uv += noise * 0.1f;

	mask = g_MaskTexture.Sample(DefaultSampler, uv);
    Out.vColor = mask;
    float t = saturate(In.vLifeTime.y / In.vLifeTime.x);

    // 색상 보간
    float3 interpColor = lerp(In.vStartColor, In.vEndColor, t);
    Out.vColor.r = interpColor.r;
    Out.vColor.gb += interpColor.gb;

    // 알파 보간 + alpha variation 적용
    float alpha = (1.f - t) * lerp(1.0f, In.fAlphaVar, t);
    Out.vColor.a *= alpha;

    // 최종 생명 종료 조건
    if (In.vLifeTime.y >= In.vLifeTime.x)
        discard;

    return Out;

}

//Nej 파티클
PS_OUT PS_DistMask(PS_IN In)
{
    PS_OUT Out;
    float2 uv = In.vTexcoord;
    float2 uv1 = uv * float2(0.5, 0.5);
    float2 uv2 = uv * float2(2, 2);

    float2 d1 = g_DistortionTexture.Sample(DefaultSampler, uv1).rg - 0.5f;
    float2 d2 = g_DistortionTexture.Sample(DefaultSampler, uv2).rg - 0.5f;

    float2 dist = (d1 + d2) * 0.8;
    float mask = g_MaskTexture.Sample(DefaultSampler, dist).r;
	if (mask < 0.7f)
		discard;
    Out.vColor = (In.vStartColor.xyz, mask);

	float t = saturate(In.vLifeTime.y / In.vLifeTime.x);
    // 알파 보간
    Out.vColor.a*= t;

    //Out.vDistortion = float4(dist, 0, 0);

    return Out;
}





technique11 DefaultTechnique
{

    pass Default
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);


        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = compile gs_5_0 GS_MAIN();
        PixelShader = compile ps_5_0 PS_MAIN();
    }

    pass Mask
    {
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_None, 0);
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = compile gs_5_0 GS_MAIN();
		PixelShader = compile ps_5_0 PS_MAIN_MASK();
    }
	pass Mask2
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = compile gs_5_0 GS_MAIN();
        PixelShader = compile ps_5_0 PS_MAIN_MASK2();
    }

    pass MaskUV
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = compile gs_5_0 GS_MAIN();
        PixelShader = compile ps_5_0 PS_MAIN_MASK_UV();
    }

    pass MaskWave
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = compile gs_5_0 GS_MAIN();
        PixelShader = compile ps_5_0 PS_MAIN_MASK4();
    }

	pass DistortionNoiseDiffuse
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_None, 0);
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = compile gs_5_0 GS_MAIN();
		PixelShader = compile ps_5_0 PS_DistDiffNoiseMask();
	}

    pass DistortionNoise
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = compile gs_5_0 GS_MAIN();
        PixelShader = compile ps_5_0 PS_DistNoiseMask();
    }

	pass DistMask //7
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_None, 0);
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = compile gs_5_0 GS_MAIN();
		PixelShader = compile ps_5_0 PS_DistMask();
	}

}
