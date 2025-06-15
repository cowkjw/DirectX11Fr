#include "Engine_Shader_Defines.hlsli"

/* 상수테이블 ConstantTable */
matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
texture2D g_Texture;

cbuffer CB_Cloud : register(b0)
{
    float2 g_uvOffset;
	float2 g_uvScale;      // UV 스케일 (default는 (1,1))
}

cbuffer CB_CloudColor : register(b1)
{
    float4 g_Color;        // 곱할 색 (default는 (1,1,1,1))
}

cbuffer CB_HP : register(b2)
{
    float g_HpRatio;    // 0~1 사이, C++에서 매 프레임 갱신
}




/* 정점의 기초적인 변환 (월드변환, 뷰, 투영변환) */
/* 정점의 구성 정보를 변형할 수 있다. */

struct VS_IN
{
    float3 vPosition : POSITION;
    float2 vTexcoord : TEXCOORD0;
};

struct VS_OUT
{
    /* SV_ : ShaderValue약자 */
    /* 내가 해야할 연산은 다 했으니 이제 니(장치)가 알아서 추가적인 연산을 해라. */
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;

    matrix matWV, matWVP;

    /* mul : 모든 행렬의 곱하기를 수행한다. /w연산을 수행하지 않는다. */
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);

    Out.vPosition = mul(vector(In.vPosition, 1.f), matWVP);
    Out.vTexcoord = In.vTexcoord;

    return Out;
}

//POSITION시멘틱이 붙은
//멤버변수에 대해서

/* W나누기 연산을 수행한다. */
/* 뷰포트로 변환한다. */
/* 모든 성분에 대해서 래스터라이즈. -> 픽셀을 생성한다. */


struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
};

struct PS_OUT
{
    vector vColor : SV_TARGET0;
};

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out;
    float2 uv;
    uv.x = In.vTexcoord.x * g_uvScale.x + g_uvOffset.x;
    uv.y = In.vTexcoord.y * g_uvScale.y + g_uvOffset.y;
    Out.vColor = g_Texture.Sample(DefaultSampler, uv) * g_Color;
    return Out;
}


PS_OUT PS_ClampMAIN(PS_IN In)
{
    PS_OUT Out;

   
    // 1) UV 스케일·오프셋 계산
    float2 uv = In.vTexcoord * g_uvScale + g_uvOffset;


    // 2) HP 비율보다 오른쪽이면 픽셀 버리기
    //    (버려지면 그 픽셀은 그려지지 않음)
    if (uv.x > g_HpRatio)
        clip(-1);

    // 3) 나머지 픽셀만 텍스처 샘플링
    Out.vColor = g_Texture.Sample(LinearClampSampler, uv) * g_Color;
	if (Out.vColor.a < 0.01f)
		clip(-1); // 알파값이 거의 0이면 픽셀 버리기
    return Out;
}



technique11 DefaultTechnique
{
    /* 패스를 생성하는 기준을 뭘로? */
    /* 같은 모델을 그릴때 각기 다른 렌더스테이츠를 먹여야하거나.
    완전히 다른 쉐이딩 기법을 적용해야하거나 */

    /* 각 쉐이더를 어떤 버젼으로 빌드할건지.  */
    /* 어떤 쉐이더를 사용할건지? */
    /* 진입점 함수 결정 */
    /* 렌더스테이츠에 대한 설정*/
    pass Default/* 명암 + 스펙큘러 + 그림자 + ssao + 림라이트 */
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);


        VertexShader = compile vs_5_0 VS_MAIN();
        PixelShader = compile ps_5_0 PS_MAIN();
    }

    pass Clamp
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_None, 0);
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		PixelShader = compile ps_5_0 PS_ClampMAIN();
	}
    //pass Disstortion/* 왜곡 1*/ 
    //{
    //    VertexShader = compile vs_5_0 VS_MAIN1();
    //    PixelShader = compile ps_5_0 PS_MAIN_Diss();
    //}
    //pass Blend/* 반투명 */
    //{
    //    VertexShader = compile vs_5_0 VS_MAIN();
    //    PixelShader = compile ps_5_0 PS_MAIN_Blend();
    //}
}
