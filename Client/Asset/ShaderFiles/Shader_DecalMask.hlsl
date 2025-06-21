#include "Engine_Shader_Defines.hlsli"

cbuffer CB_Transform : register(b0)
{
	matrix g_WorldMatrix;
	matrix g_ViewMatrix;
	matrix g_ProjMatrix;
};
float   g_MaskThreshold = 0.1;  // 마스크 컷아웃 임계값
float   g_NoiseUVStrength = 0.2;  // 노이즈 기반 UV 왜곡 강도
float   g_NoiseAlphaStrength = 0.5;  // 노이즈 기반 알파 변조 강도


Texture2D g_Texture : register(t0); // 메인 스프라이트
Texture2D g_MaskTexture : register(t1); // 마스크용 그레이맵(RG 채널 사용)
Texture2D g_NoiseTexture : register(t2); // 노이즈 텍스처 
float4 g_vLightDir;
float4 g_vLightDiffuse;
float4 g_vLightAmbient;
float4 g_vLightSpecular;

float4 g_vCamPosition;
float g_Time;

static const float g_PulseSpeed = 6.0;  // 알파 펄스 속도 (라디안/sec)
static const float g_PulseStrength = 0.8;  // 펄스 강도 (0~1)
struct VS_IN { float3 pos : POSITION; float2 uv : TEXCOORD0; };
struct VS_OUT { float4 pos : SV_POSITION; float2 uv : TEXCOORD0; };

VS_OUT VS_MAIN(VS_IN In)
{
	VS_OUT Out;
	float4 worldPos = float4(In.pos, 1.f);
	Out.pos = mul(mul(worldPos, g_WorldMatrix), mul(g_ViewMatrix, g_ProjMatrix));
	Out.uv = In.uv;
	return Out;
}


float4 PS_MAIN(VS_OUT In) : SV_Target
{

	// 1) UV 회전 (원 중심 회전)
 float2 centerUV = In.uv - 0.5;                        //  영역
 float  angle = g_Time * 0.5;                       // 회전 속도 0.5rad/sec
 float  s = sin(angle);
 float  c = cos(angle);
 float2 uvRot = float2(
	 centerUV.x * c - centerUV.y * s,
	 centerUV.x * s + centerUV.y * c
 ) + 0.5;

 // 2) 원형 컷아웃
 float2 d = uvRot - 0.5;
 if (length(d) > 0.5) discard;

 // 3) 마스크 컷아웃
 float m = g_MaskTexture.Sample(DefaultSampler, uvRot).r;
 if (m < g_MaskThreshold) discard;

 // 4) 노이즈 & UV 왜곡
 float noise = g_NoiseTexture.Sample(DefaultSampler, uvRot).r;
 float2 uvD = uvRot + (noise - 0.5) * g_NoiseUVStrength;

 // 5) 디퓨즈 샘플링
 float4 baseCol = g_Texture.Sample(DefaultSampler, uvD);

 // 6) 노이즈 기반 알파 변조
 baseCol.a *= lerp(1.0, noise, g_NoiseAlphaStrength);

 // 7) 시간 기반 알파 펄스
 float pulse = (sin(g_Time * g_PulseSpeed) * 0.5 + 0.5) * g_PulseStrength
			   + (1 - g_PulseStrength);
 baseCol.a *= pulse;

 return baseCol;
}

float4 PS_MAIN_RECT(VS_OUT In) : SV_Target
{

	// 1) UV 회전 (원 중심 회전)
 float2 centerUV = In.uv - 0.5;                        //  영역
 float  angle = g_Time * 0.5;                       // 회전 속도 0.5rad/sec
 float  s = sin(angle);
 float  c = cos(angle);
 float2 uvRot = float2(
	 centerUV.x * c - centerUV.y * s,
	 centerUV.x * s + centerUV.y * c
 ) + 0.5;



 // 3) 마스크 컷아웃
 float m = g_MaskTexture.Sample(DefaultSampler, uvRot).r;
 if (m < g_MaskThreshold) discard;

 // 4) 노이즈 & UV 왜곡
 float noise = g_NoiseTexture.Sample(DefaultSampler, uvRot).r;
 float2 uvD = uvRot + (noise - 0.5) * g_NoiseUVStrength;

 // 5) 디퓨즈 샘플링
 float4 baseCol = g_Texture.Sample(DefaultSampler, uvD);

 // 6) 노이즈 기반 알파 변조
 baseCol.a *= lerp(1.0, noise, g_NoiseAlphaStrength);

 // 7) 시간 기반 알파 펄스
 float pulse = (sin(g_Time * g_PulseSpeed) * 0.5 + 0.5) * g_PulseStrength
			   + (1 - g_PulseStrength);
 baseCol.a *= pulse;

 return baseCol;
}


technique11 SpriteTech
{
	pass P0
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_AlphaBlend, float4(0,0,0,0), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		PixelShader = compile ps_5_0 PS_MAIN();
	}
	pass P1
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_AlphaBlend, float4(0,0,0,0), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		PixelShader = compile ps_5_0 PS_MAIN_RECT();
	}
}