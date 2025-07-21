#include "Engine_Shader_Defines.hlsli"

/* 상수테이블 ConstantTable */
matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
matrix g_LightViewMatrix, g_LightProjMatrix;
matrix g_ViewMatrixInv, g_ProjMatrixInv;



Texture2D g_RenderTargetTexture;
Texture2D g_NormalTexture;
Texture2D g_DiffuseTexture;
Texture2D g_ShadeTexture;
Texture2D g_DepthTexture;
Texture2D g_DistortionTexture;
Texture2D g_DistortionObjTexture;
Texture2D g_DistortionFinalTexture;
Texture2D g_BlurEffectTexture;
Texture2D g_BloomBlurXTexture;
Texture2D g_BloomBlurYTexture;
Texture2D g_BloomEffectTexture;
Texture2D g_EffectTexture;
Texture2D g_BrightTexture;
Texture2D g_BloomTexture;
Texture2D g_FinalTexture;
Texture2D g_BlurXTexture;
Texture2D g_BlurYTexture;
Texture2D g_RimLightTexture;
Texture2D g_ShadowTexture;
Texture2D g_FogTexture;



float g_fCameraFar;



vector g_vLightDir;
vector g_vLightDiffuse;
float  g_fLightAmbient;
vector g_vLightSpecular;
vector g_vCamPosition;

float g_fLightRange;
vector g_vLightPos;

float  g_fMtrlAmbient = 0.4f;
vector g_vMtrlSpecular = 1.f;

float g_fShadowThreshold = 0.65f;      // 그림자 임계값
float g_fSaturationBoost = 1.6f;      // 채도 부스트
float g_fAmbientStrength = 0.15f;      // 앰비언트 강도

float4 g_vMtrlAmibient = float4(0.4f, 0.4f, 0.4f, 1.f);
float4 g_vShadowColor = float4(0.5f, 0.5f, 0.5f, 1.0f);


float g_fOutlineThreshold = 1.2f;      // 외곽선 감지 임계값
float g_fNormalEdgeStrength = 1.2f;    // 노말 엣지 강도
float g_fOutlineStrength = 0.8f;       // 외곽선 최종 강도
float4 g_vOutlineColor = float4(0.0f, 0.0f, 0.0f, 1.0f); // 외곽선 색상
int g_iWinSizeX = 1280;                // 화면 너비
int g_iWinSizeY = 720;                // 화면 높이
float g_fOutlineEnable = 1.0f;         // 외곽선 활성화 (0.0 = 비활성화, 1.0 = 활성화)


float4 g_FogColor;
float  g_FogStart, g_FogEnd;
bool g_bFogEnabled;

struct VS_IN
{
    float3 vPosition : POSITION;
    float2 vTexcoord : TEXCOORD0;
};

struct VS_OUT
{
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

struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
};

struct PS_OUT
{
    vector vBackBuffer : SV_TARGET0;
};

PS_OUT PS_MAIN_DEBUG(PS_IN In)
{
    PS_OUT Out;
    
    Out.vBackBuffer = g_RenderTargetTexture.Sample(DefaultSampler, In.vTexcoord);
    
    return Out;
}

struct PS_OUT_LIGHT
{
    vector vShade : SV_TARGET0;
    vector vSpecular : SV_TARGET1;
};

PS_OUT_LIGHT PS_MAIN_LIGHT_DIRECTIONAL(PS_IN In)
{
    PS_OUT_LIGHT Out;
    
    vector vNormalDesc = g_NormalTexture.Sample(DefaultSampler, In.vTexcoord);
    
    float4 vNormal = float4(vNormalDesc.xyz * 2.f - 1.f, 0.f);
    
    float fShade = max(dot(normalize(g_vLightDir) * -1.f, vNormal), 0.f) + (g_fLightAmbient * g_fMtrlAmbient);
    
    Out.vShade = g_vLightDiffuse * saturate(fShade);

	vector vDepth = g_DepthTexture.Sample(DefaultSampler, In.vTexcoord);
    float fViewZ = vDepth.y * g_fCameraFar;

    vector vWorldPos;
    vWorldPos.x = In.vTexcoord.x * 2.f - 1.f; // 투영공간으로 끌고 오기 위해서 정규화
    vWorldPos.y = In.vTexcoord.y * -2.f + 1.f;
    vWorldPos.z = vDepth.x; // 깊이 값을 x에 저장해뒀음
    vWorldPos.w = 1.f; // w는 자기 자신으로 나눴으니까 1

    // 투영에서 이제 월드로 가져오기
    vWorldPos = vWorldPos * fViewZ;
	vWorldPos = mul(vWorldPos, g_ProjMatrixInv);
	vWorldPos = mul(vWorldPos, g_ViewMatrixInv); 

    // 이건 스페큘러 계산할 때 사용
    vector vReflect = reflect(normalize(g_vLightDir), vNormal);
    vector vLook = vWorldPos - g_vCamPosition;

    Out.vSpecular = (g_vLightSpecular * g_vMtrlSpecular) * pow(max(dot(normalize(vLook) * -1.f, normalize(vReflect)), 0.f), 50.f);

    return Out;
}

PS_OUT_LIGHT PS_MAIN_LIGHT_POINT(PS_IN In)
{
    PS_OUT_LIGHT Out;

    vector vNormalDesc = g_NormalTexture.Sample(DefaultSampler, In.vTexcoord);

    float4 vNormal = float4(vNormalDesc.xyz * 2.f - 1.f, 0.f);


    vector vDepthDesc = g_DepthTexture.Sample(DefaultSampler, In.vTexcoord);
    float fViewZ = vDepthDesc.y * 500.f;

    vector vWorldPos;

    /* 로컬위치 * 월드 *뷰 * 투영 / w : 투영공간상의 위치. */
    vWorldPos.x = In.vTexcoord.x * 2.f - 1.f;
    vWorldPos.y = In.vTexcoord.y * -2.f + 1.f;
    vWorldPos.z = vDepthDesc.x;
    vWorldPos.w = 1.f;

    /* 로컬위치 * 월드 *뷰 * 투영  */
    vWorldPos = vWorldPos * fViewZ;

    vWorldPos = mul(vWorldPos, g_ProjMatrixInv);
    vWorldPos = mul(vWorldPos, g_ViewMatrixInv);

    vector vLightDir = vWorldPos - g_vLightPos;

    float fAtt = (g_fLightRange - length(vLightDir)) / g_fLightRange;

    float fShade = max(dot(normalize(vLightDir) * -1.f, vNormal), 0.f) + (g_fLightAmbient * g_fMtrlAmbient);

    Out.vShade = g_vLightDiffuse * saturate(fShade) * fAtt;

    vector vReflect = reflect(normalize(vLightDir), vNormal);
    vector vLook = vWorldPos - g_vCamPosition;

    Out.vSpecular = (g_vLightSpecular * g_vMtrlSpecular) * pow(max(dot(normalize(vLook) * -1.f, normalize(vReflect)), 0.f), 50.f) * fAtt;


    return Out;
}



PS_OUT PS_MAIN_DEFERRED(PS_IN In)
{
    PS_OUT Out;
    
    vector vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    //if (all(vDiffuse.rgb == 0.f))
    if(vDiffuse.a == 0.f)
        discard;
    
    vector vShade = g_ShadeTexture.Sample(DefaultSampler, In.vTexcoord);
    
    Out.vBackBuffer = vDiffuse * vShade;
    
    return Out;    
}

float4 AdjustSaturation(float4 color, float saturation)
{
    float gray = dot(color.rgb, float3(0.299, 0.587, 0.114));
    return float4(lerp(gray.xxx, color.rgb, saturation), color.a);
}

// 외곽선
float DetectEdge(float2 uv, float2 texelSize)
{

    // 주변 픽셀들 검사하기 노말이랑 똑같이 x,y,z 다시 되돌려서 계산
    float3 normalUp = normalize(g_NormalTexture.Sample(DefaultSampler, uv + float2(0, texelSize.y)).xyz * 2.0 - 1.0);
    float3 normalDown = normalize(g_NormalTexture.Sample(DefaultSampler, uv - float2(0, texelSize.y)).xyz * 2.0 - 1.0);
    float3 normalLeft = normalize(g_NormalTexture.Sample(DefaultSampler, uv - float2(texelSize.x, 0)).xyz * 2.0 - 1.0);
    float3 normalRight = normalize(g_NormalTexture.Sample(DefaultSampler, uv + float2(texelSize.x, 0)).xyz * 2.0 - 1.0);


	// 노말 차이 계산

    // 유클리드 공식으로 크기를 구하면 되는데
    // 노말 벡터가 3차원 방향 정보를 가지고 있어서 차이말고 실제 거리로 구해야함
    // 내적하면 제곱합을 구할 수 있음

	// 자기 자신의 제곱을 구하는 것과 같음
    float normalDiffX = dot(normalRight - normalLeft, normalRight - normalLeft);
    float normalDiffY = dot(normalUp - normalDown, normalUp - normalDown);

    // 제곱의 합을 제곱근으로 구함
    float normalEdge = sqrt(normalDiffX + normalDiffY);

    // 엣지 강도 구하기 
    // 너무 작아서 값을 보정해줌
    float edge = normalEdge * g_fNormalEdgeStrength;

	// step은 (float edgeThreshold, float edgeValue) 형태로
	// edgeThreshold보다 edgeValue가 크면 1.0, 작으면 0.0을 반환
    // g_fOutlineThreshold은 어느정도부터 엣지로 판별할건지
    return step(g_fOutlineThreshold, edge);
}

PS_OUT PS_MAIN_DEFERRED_TOON_WRAP_OUTLINE(PS_IN In)
{
    PS_OUT Out;

 //   vector vShade = g_ShadeTexture.Sample(DefaultSampler, In.vTexcoord);
 //   vector vMtrlDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    float2 uv = In.vTexcoord;

    //float2 distortion = g_DistortionTexture.Sample(DefaultSampler, uv).rg;
    //distortion = (distortion * 2.0f - 1.0f) *5.f; // Strength 조절

    //uv += distortion;
	float4 vNormalSample = g_NormalTexture.Sample(DefaultSampler, uv);
    vector vDepthDesc = g_DepthTexture.Sample(DefaultSampler, In.vTexcoord);
	float fViewZ = vDepthDesc.y * g_fCameraFar;

    vector vPosition;

    vPosition.x = In.vTexcoord.x * 2.f - 1.f;
    vPosition.y = In.vTexcoord.y * -2.f + 1.f;
    vPosition.z = vDepthDesc.x;
    vPosition.w = 1.f;

    vPosition = vPosition * fViewZ;

    vPosition = mul(vPosition, g_ProjMatrixInv);
    vPosition = mul(vPosition, g_ViewMatrixInv);

    vPosition = mul(vPosition, g_LightViewMatrix);
    vPosition = mul(vPosition, g_LightProjMatrix);

    float2 vTexcoord;

    /* (-1, 1 ~ 1, -1) -> (0, 0 ~ 1, 1) */
    vTexcoord.x = vPosition.x / vPosition.w * 0.5f + 0.5f;
    vTexcoord.y = vPosition.y / vPosition.w * -0.5f + 0.5f;

    float4  vOldDepthDesc = g_ShadowTexture.Sample(DefaultSampler, vTexcoord);
    float fOldViewZ = vOldDepthDesc.y * g_fCameraFar;
    // 툰으로 그리기
    if (vNormalSample.w > 0.0f)
    {
        vector vShade = g_ShadeTexture.Sample(DefaultSampler, uv);
        vector vMtrlDiffuse = g_DiffuseTexture.Sample(DefaultSampler, uv);
        float4 vNormalSample = g_NormalTexture.Sample(DefaultSampler, uv);

        float3 normal = normalize(vNormalSample.xyz * 2.f - 1.f);
        float3 lightDir = normalize(g_vLightDir.xyz);
        float  NdotL = all(max(dot(normal, lightDir), 0.f));
        float  isLit = step(g_fShadowThreshold, NdotL);
        float4 baseColor = vMtrlDiffuse;
        float4 litColor = baseColor * g_vLightDiffuse;
        float4 shad = vMtrlDiffuse * g_vShadowColor;
        //float4 toonColor = lerp(shad, litColor, isLit);

        float levels = 2;                                 // 단계 수
        float d = floor(NdotL * levels) / (levels - 1);   // 0.0, 0.33, 0.66, 1.0 등
        float4 toonColor = lerp(shad, litColor, d);

        float4 amb = vMtrlDiffuse * (g_fLightAmbient) * g_fAmbientStrength;

        // 최종 및 채도 보정
        float4 finalColor = toonColor + amb;
        finalColor = AdjustSaturation(finalColor, g_fSaturationBoost);

       
        if (vNormalSample.w > 0.5f)
        {
            float2 texelSize = 1.0 / float2(g_iWinSizeX, g_iWinSizeY);

             float edge = DetectEdge(uv, texelSize);
             finalColor = lerp(saturate(finalColor), g_vOutlineColor, edge * g_fOutlineStrength);
             Out.vBackBuffer = finalColor;
            
        }
        else
        {
            finalColor = AdjustSaturation(finalColor, 0.5f);
			Out.vBackBuffer = saturate(finalColor);
        }
    }
    else
    {

        vector vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, uv);
        //if (all(vDiffuse.rgb == 0.f))
        if (vDiffuse.a == 0.f)
            discard;

        vector vShade = g_ShadeTexture.Sample(DefaultSampler, uv);

        Out.vBackBuffer = vDiffuse * vShade;

    }
 /*  
    if (fOldViewZ + 0.1f < vPosition.w)
    {
        Out.vBackBuffer = Out.vBackBuffer * 0.5f;
    }*/
    return Out;
}



float g_fWeights[13] =
{
    0.0561, 0.1353, 0.278, 0.4868, 0.7261, 0.9231, 1.f, 0.9231, 0.7261, 0.4868, 0.278, 0.1353, 0.0561
};



struct PS_OUT_BLUR
{
    vector vColor : SV_TARGET0;
};

PS_OUT_BLUR PS_MAIN_BLURX(PS_IN In)
{
    PS_OUT_BLUR Out;

    float2 vTexcoord;

    vector vColor;
    float sum = 0;
    for (int i = -6; i < 7; ++i)
    {
        vTexcoord.x = In.vTexcoord.x + i / 1280.f;
        vTexcoord.y = In.vTexcoord.y;
		sum += g_fWeights[i + 6];
        Out.vColor += g_fWeights[i + 6] * g_BlurEffectTexture.Sample(LinearClampSampler, vTexcoord);
    }

    Out.vColor /= sum;

    return Out;
}

PS_OUT PS_MAIN_BLURY(PS_IN In)
{
    PS_OUT Out;

    float2 vTexcoord;

    vector vColor;
    float sum = 0;
    for (int i = -6; i < 7; ++i)
    {
        vTexcoord.x = In.vTexcoord.x;
        vTexcoord.y = In.vTexcoord.y + i / 720.f;
        sum += g_fWeights[i + 6];
        Out.vBackBuffer += g_fWeights[i + 6] * g_BlurXTexture.Sample(LinearClampSampler, vTexcoord);
    }

    Out.vBackBuffer /= sum;

    return Out;
}


PS_OUT PS_MAIN_BRIGHT_EFFECT(PS_IN In)
{
    PS_OUT Out;
    float4 c = g_BloomEffectTexture.Sample(LinearClampSampler, In.vTexcoord);
    // Luminance 계산
    float lum = dot(c.rgb, float3(0.299, 0.587, 0.114));

    // Threshold(임계치)와 Knee(부드럼 영역 폭) 정의
    float threshold = 0.8f;
    float knee = 0.2f;   // 0.0 ~ 1.0  작을 수록 값자기 클수록 좀 부드럽게

   //Soft threshold: knee 구간 안에서는 0 → 1로 선형 보간
    float soft = saturate((lum - threshold + knee) / knee);
    // 임계치 아래에서는 0, knee 구간 안에서는 soft, 그 이상은 1
    float extract = saturate((lum - threshold) / knee + 0.5f) * soft;

    // 추출 값으로 원본 컬러 스케일
    Out.vBackBuffer = c * extract;

    return Out;
}

PS_OUT PS_MAIN_BLOOM(PS_IN In)
{
    PS_OUT Out;

	vector vBlur = g_BloomBlurYTexture.Sample(DefaultSampler, In.vTexcoord);
	vector vBright = g_BrightTexture.Sample(DefaultSampler, In.vTexcoord);


	Out.vBackBuffer = vBlur*3.f  + vBright*0.5f;
    return Out;
}

PS_OUT_BLUR PS_MAIN_BLOOM_BLURX(PS_IN In)
{
    PS_OUT_BLUR Out;

    float2 vTexcoord;

    vector vColor;
    float sum = 0;
    for (int i = -6; i < 7; ++i)
    {
        vTexcoord.x = In.vTexcoord.x + i / 1280.f;
        vTexcoord.y = In.vTexcoord.y;
        sum += g_fWeights[i + 6];
        Out.vColor += g_fWeights[i + 6] * g_BrightTexture.Sample(LinearClampSampler, vTexcoord);
    }

    Out.vColor /= sum;

    return Out;
}

PS_OUT PS_MAIN_BLOOM_BLURY(PS_IN In)
{
    PS_OUT Out;

    float2 vTexcoord;

    vector vColor;
    float sum = 0;
    for (int i = -6; i < 7; ++i)
    {
        vTexcoord.x = In.vTexcoord.x;
        vTexcoord.y = In.vTexcoord.y + i / 720.f;
        sum += g_fWeights[i + 6];
        Out.vBackBuffer += g_fWeights[i + 6] * g_BloomBlurXTexture.Sample(LinearClampSampler, vTexcoord);
    }

    Out.vBackBuffer /= sum;

    return Out;
}


PS_OUT PS_MAIN_FinalRender(PS_IN In)
{
    PS_OUT Out;

    float2 d = g_DistortionTexture.Sample(DefaultSampler, In.vTexcoord).rg;
    float2 offset = (d * 2.0f - 1.0f) * 0.5f;
    float2 uvDist = In.vTexcoord + offset;

    // 모든 샘플링에 디스토션 적용 (필요에 따라)
    vector vFinal = g_FinalTexture.Sample(DefaultSampler, uvDist);
    vector vRawEffect = g_EffectTexture.Sample(DefaultSampler, uvDist);
    vector vBloom = g_BloomTexture.Sample(DefaultSampler, In.vTexcoord);
    vector vBlurY = g_BlurYTexture.Sample(DefaultSampler, In.vTexcoord);
    vector vRimLight = g_RimLightTexture.Sample(DefaultSampler, In.vTexcoord);

    float4 objCol = g_DistortionObjTexture.Sample(DefaultSampler, uvDist);

    float4 finalColor = vFinal + vRawEffect + vBloom + vBlurY * 2.5f + vRimLight + objCol;

    // 안개 처리
    if (g_bFogEnabled)
    {
        float depth = g_DepthTexture.Sample(DefaultSampler, In.vTexcoord).y * g_fCameraFar;
        float fogFactor = saturate((depth - g_FogStart) / (g_FogEnd - g_FogStart));
        float finalFogFactor = fogFactor * g_FogColor.a;
        finalColor.rgb = lerp(finalColor.rgb, g_FogColor.rgb, finalFogFactor);
    }

    Out.vBackBuffer = finalColor;
    return Out;
}

technique11 DefaultTechnique
{
    pass Debug 
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        

        VertexShader = compile vs_5_0 VS_MAIN();
        PixelShader = compile ps_5_0 PS_MAIN_DEBUG();
    }

    pass Light_Directional
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        

        VertexShader = compile vs_5_0 VS_MAIN();
        PixelShader = compile ps_5_0 PS_MAIN_LIGHT_DIRECTIONAL();
    }

    pass Light_Point
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        

        VertexShader = compile vs_5_0 VS_MAIN();
        PixelShader = compile ps_5_0 PS_MAIN_LIGHT_POINT();
    }

    pass Deferred
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        

        VertexShader = compile vs_5_0 VS_MAIN();
        PixelShader = compile ps_5_0 PS_MAIN_DEFERRED();
    }

    pass Deferred_ToonWrap
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);


        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_DEFERRED_TOON_WRAP_OUTLINE();
    }

    pass BlurX
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_BLURX();
    }

    pass BlurY
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_BLURY();
    }

    pass FinalRender
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_FinalRender();
    }

    pass BrightPass
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0,0,0,0), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        PixelShader = compile ps_5_0 PS_MAIN_BRIGHT_EFFECT();
    }


    pass Bloom
    {   SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_BLOOM();
    }


    pass BloomBlurX
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_BLOOM_BLURX();
    }

    pass BloomBlurY
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_BLOOM_BLURY();
    }
}
