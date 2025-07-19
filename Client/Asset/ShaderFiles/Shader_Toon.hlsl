#include "Engine_Shader_Defines.hlsli"

// 기본 변환 행렬들
matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
matrix g_BoneMatrices[512];

// 텍스처
texture2D g_DiffuseTexture;

//// 라이팅 파라미터들
//float4 g_vLightDir;
//float4 g_vLightDiffuse;
//float4 g_vLightAmbient;
//float4 g_vLightSpecular;

float g_fCameraFar;
float4 g_vCamPosition;

// 머티리얼 속성
//float4 g_vMtrlAmibient = float4(0.4f, 0.4f, 0.4f, 1.f);
float4 g_vShadowColor = float4(0.4f, 0.4f, 0.4f, 1.0f);  


float g_fShadowThreshold = 0.65f;      // 그림자 임계값
float g_fSaturationBoost = 1.6f;      // 채도 부스트
float g_fAmbientStrength = 0.6f;      // 앰비언트 강도

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
	float4 vProjPos : TEXCOORD3; 
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

	Out.vPosition = mul(vPosition, matWVP);
	Out.vNormal = normalize(mul(vNormal, g_WorldMatrix));
	Out.vTexcoord = In.vTexcoord;
	Out.vWorldPos = mul(vPosition, g_WorldMatrix);

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

//struct PS_OUT
//{
//	vector vColor : SV_TARGET0;
//};
struct PS_OUT
{
	vector vDiffuse : SV_TARGET0;
	vector vNormal : SV_TARGET1;
	vector vDepth : SV_TARGET2; // 깊이값을 저장할 타겟
};

float4 AdjustSaturation(float4 color, float saturation)
{
	float gray = dot(color.rgb, float3(0.299, 0.587, 0.114));
	return float4(lerp(gray.xxx, color.rgb, saturation), color.a);
}

PS_OUT PS_MAIN_TOON(PS_IN In)
{
	PS_OUT Out;

	vector  vMtrlDiffuse = g_DiffuseTexture.Sample(LinearClampSampler, In.vTexcoord);
	if (vMtrlDiffuse.a < 0.1f)
		discard;

	Out.vDiffuse = vMtrlDiffuse;

	/* -1.f -> 0.f, 1.f -> 1.f */
	Out.vNormal = vector(In.vNormal.xyz * 0.5f + 0.5f, 1.f);
	// 깊이값 계산
	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / g_fCameraFar, 0.f, 0.f);


	return Out;
}


PS_OUT PS_MAIN_WrapTOON(PS_IN In)
{

	PS_OUT Out;
	// 디퓨즈 텍스처 샘플링
	vector vMtrlDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);

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
	vMtrlDiffuse.a = 1.f;

	Out.vDiffuse = vMtrlDiffuse;

	/* -1.f -> 0.f, 1.f -> 1.f */
	Out.vNormal = vector(In.vNormal.xyz * 0.5f + 0.5f, 1.f);
	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / g_fCameraFar, 0.f, 0.f);
	return Out;

}


PS_OUT PS_MAIN(PS_IN In)
{
	//PS_OUT Out;
	//vector vMtrlDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);

	//float4 vShade = max(dot(normalize(g_vLightDir) * -1.f, In.vNormal), 0.f) +
	//	(g_vLightAmbient * g_vMtrlAmibient);

	//Out.vColor = g_vLightDiffuse * vMtrlDiffuse * vShade;

	PS_OUT Out;

	vector  vMtrlDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
	if (vMtrlDiffuse.a < 0.1f)
		discard;

	Out.vDiffuse = vMtrlDiffuse;

	/* -1.f -> 0.f, 1.f -> 1.f */
	Out.vNormal = vector(In.vNormal.xyz * 0.5f + 0.5f, 1.f);
	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / g_fCameraFar, 0.f, 0.f);
	return Out;
}

PS_OUT PS_MAIN_CLAMP(PS_IN In)
{
	PS_OUT Out;
	vector vMtrlDiffuse = g_DiffuseTexture.Sample(LinearClampSampler, In.vTexcoord);
	if (vMtrlDiffuse.a < 0.3f)
		discard;

	Out.vDiffuse = vMtrlDiffuse;

	/* -1.f -> 0.f, 1.f -> 1.f */
	Out.vNormal = vector(In.vNormal.xyz * 0.5f + 0.5f, 1.f);
	// 깊이값 계산
	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / g_fCameraFar, 0.f, 0.f);

	return Out;
}

struct VS_OUT_SHADOW
{
	float4 vPosition : SV_POSITION;
	float4 vProjPos : TEXCOORD0;
};


VS_OUT_SHADOW VS_MAIN_SHADOW(VS_IN In)
{
	VS_OUT_SHADOW Out;

	float fWeightW = 1.f - (In.vBlendWeights.x + In.vBlendWeights.y + In.vBlendWeights.z);

	matrix BoneMatrix = g_BoneMatrices[In.vBlendIndices.x] * In.vBlendWeights.x +
		g_BoneMatrices[In.vBlendIndices.y] * In.vBlendWeights.y +
		g_BoneMatrices[In.vBlendIndices.z] * In.vBlendWeights.z +
		g_BoneMatrices[In.vBlendIndices.w] * fWeightW;

	vector vPosition = mul(vector(In.vPosition, 1.f), BoneMatrix);

	matrix matWV, matWVP;

	/* mul : 모든 행렬의 곱하기를 수행한다. /w연산을 수행하지 않는다. */
	matWV = mul(g_WorldMatrix, g_ViewMatrix);
	matWVP = mul(matWV, g_ProjMatrix);

	Out.vPosition = mul(vPosition, matWVP);
	Out.vProjPos = Out.vPosition;

	return Out;
}

struct PS_IN_SHADOW
{
	float4 vPosition : SV_POSITION;
	float4 vProjPos : TEXCOORD0;
};

struct PS_OUT_SHADOW
{
	vector vShadow : SV_TARGET0;
};

PS_OUT_SHADOW PS_MAIN_SHADOW(PS_IN_SHADOW In)
{
	PS_OUT_SHADOW Out;

	Out.vShadow = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w /g_fCameraFar, 0.f, 0.f);

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

	pass Shadow
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN_SHADOW();
		PixelShader = compile ps_5_0 PS_MAIN_SHADOW();
	}
}