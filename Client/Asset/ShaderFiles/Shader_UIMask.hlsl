#include "Engine_Shader_Defines.hlsli"

cbuffer CB_Transform : register(b0)
{
    matrix g_WorldMatrix;
    matrix g_ViewMatrix;
    matrix g_ProjMatrix;
};

cbuffer CB_Sprite : register(b1)
{
    float4 g_Color;      // RGBA 곱 색
    float2 g_uvOffset;   // 기본 UV 오프셋
    float2 g_uvScale;    // 기본 UV 스케일
};

cbuffer CB_Mask : register(b2)
{
    float  maskThreshold;  // 0~1: 이 값보다 큰 마스크 영역만 렌더
    float2 maskUVOffset;   // 마스크 UV 오프셋
    float2 maskUVScale;    // 마스크 UV 스케일
    uint   useMask;        // 0 = 마스크 사용 안 함, 1 = 사용
    float2 _pad;           // 16byte align
};


Texture2D g_Texture : register(t0); // 메인 스프라이트
Texture2D g_MaskTexture : register(t1); // 마스크용 그레이맵(R 채널 사용)


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
    // 기본 UV
    float2 uv0 = In.uv * g_uvScale + g_uvOffset;

float4 baseCol = g_Texture.Sample(DefaultSampler, uv0) * g_Color;

// 마스크가 활성화된 경우
if (useMask == 1)
{
	// 기존 UV에 마스크 UV 오프셋과 스케일 적용
    float2 muv = uv0 * maskUVScale + maskUVOffset;

    float m = g_MaskTexture.Sample(LinearClampSampler, muv).r;

    // m < threshold 면 픽셀 버리기
    if (m < maskThreshold)
        discard;
}

return baseCol;
}

float4 PS_Ink(VS_OUT In) : SV_Target
{
    // 1) 스프라이트 UV
      float2 uv0 = In.uv * g_uvScale + g_uvOffset;

// 2) 텍스쳐 샘플 (LinearClampSampler 사용)
float4 tex = g_Texture.Sample(LinearClampSampler, uv0);

// 3) 그레이 값 (R 채널만 사용)
float m = tex.r;


const float threshold = 0.4f;
const float feather = 0.1f;

float alpha = saturate((threshold - m) / feather);


return float4(tex.rgb * g_Color.rgb, alpha * g_Color.a);
}


technique11 SpriteTech
{
    pass P0
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_AlphaBlend, float4(0,0,0,0), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        PixelShader = compile ps_5_0 PS_MAIN();
    }

        pass P1
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_AlphaBlend, float4(0,0,0,0), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        PixelShader = compile ps_5_0 PS_Ink();
    }
}