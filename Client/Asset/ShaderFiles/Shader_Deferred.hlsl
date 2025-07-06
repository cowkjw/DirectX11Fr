#include "Engine_Shader_Defines.hlsli"

/* 상수테이블 ConstantTable */
matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
matrix g_ViewMatrixInv, g_ProjMatrixInv;
texture2D g_RenderTargetTexture;
texture2D g_NormalTexture;
texture2D g_DiffuseTexture;
texture2D g_ShadeTexture;
texture2D g_DepthTexture;

float g_fCameraFar;



vector g_vLightDir;
vector g_vLightDiffuse;
float  g_fLightAmbient;
vector g_vLightSpecular;
vector g_vCamPosition;

float  g_fMtrlAmbient = 0.4f;

float g_fShadowThreshold = 0.65f;      // 그림자 임계값
float g_fSaturationBoost = 1.6f;      // 채도 부스트
float g_fAmbientStrength = 0.15f;      // 앰비언트 강도

float4 g_vMtrlAmibient = float4(0.4f, 0.4f, 0.4f, 1.f);
float4 g_vShadowColor = float4(0.5f, 0.5f, 0.5f, 1.0f);


float g_fOutlineThreshold = 1.2f;      // 외곽선 감지 임계값
float g_fNormalEdgeStrength = 1.2f;    // 노말 엣지 강도
float g_fDepthEdgeStrength = 1.2f;     // 깊이 엣지 강도
float g_fOutlineStrength = 0.6f;       // 외곽선 최종 강도
float4 g_vOutlineColor = float4(0.0f, 0.0f, 0.0f, 1.0f); // 외곽선 색상
int g_iWinSizeX = 1280;                // 화면 너비
int g_iWinSizeY = 720;                // 화면 높이
float g_fOutlineEnable = 1.0f;         // 외곽선 활성화 (0.0 = 비활성화, 1.0 = 활성화)

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
    //vector vReflect = reflect(normalize(g_vLightDir), vNormal);
    //vector vLook = vWorldPos - g_vCamPosition;

    //Out.vSpecular = (g_vLightSpecular * g_vMtrlSpecular) * pow(max(dot(normalize(vLook) * -1.f, normalize(vReflect)), 0.f), 50.f);

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
    // 주변 4개의 픽셀로만 8개 하니까 너무 좀 이상해 보임
    float3 normalUp = normalize(g_NormalTexture.Sample(DefaultSampler, uv + float2(0, texelSize.y)).xyz * 2.0 - 1.0);
    float3 normalDown = normalize(g_NormalTexture.Sample(DefaultSampler, uv - float2(0, texelSize.y)).xyz * 2.0 - 1.0);
    float3 normalLeft = normalize(g_NormalTexture.Sample(DefaultSampler, uv - float2(texelSize.x, 0)).xyz * 2.0 - 1.0);
    float3 normalRight = normalize(g_NormalTexture.Sample(DefaultSampler, uv + float2(texelSize.x, 0)).xyz * 2.0 - 1.0);


	// 노말 차이 계산

    // 내적하면 제곱합을 구할 수 있음

    float normalDiffX = dot(normalRight - normalLeft, normalRight - normalLeft);
    float normalDiffY = dot(normalUp - normalDown, normalUp - normalDown);

    // 제곱의 합을 제곱근으로 구함
    float normalEdge = sqrt(normalDiffX + normalDiffY);

    // 엣지 강도 구하기 깊이 노말 더 큰걸로 
    float edge = normalEdge * g_fNormalEdgeStrength;
    return step(g_fOutlineThreshold, edge);
}

PS_OUT PS_MAIN_DEFERRED_TOON_WRAP(PS_IN In)
{
    PS_OUT Out;

    vector vShade = g_ShadeTexture.Sample(DefaultSampler, In.vTexcoord);

    vector vMtrlDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    //if (vMtrlDiffuse.a == 0.f)
    //    discard;


    float3 normal = normalize(g_NormalTexture.Sample(DefaultSampler, In.vTexcoord).xyz * 2.f - 1.f);
    float3 lightDir = normalize(-g_vLightDir.xyz);
    float  NdotL = max(dot(normal, lightDir), 0.f);
    float  isLit = step(g_fShadowThreshold, NdotL);
	float4 baseColor = vMtrlDiffuse;
	float4 litColor = baseColor * g_vLightDiffuse;      // 그림자 아닌 곳에 그릴 색상
    float4 shad = vMtrlDiffuse * g_vShadowColor; // 그림자
    float4 toonColor = lerp(shad, litColor, isLit); 

    // 앰비언트
    float4 amb = vMtrlDiffuse * (g_fLightAmbient);// *g_fMtrlAmbient);

    float4 finalColor = toonColor +amb;
    finalColor = AdjustSaturation(finalColor, g_fSaturationBoost);
	Out.vBackBuffer = saturate(finalColor);
  
    return Out;
}

PS_OUT PS_MAIN_DEFERRED_TOON_WRAP_OUTLINE(PS_IN In)
{
    PS_OUT Out;

 //   vector vShade = g_ShadeTexture.Sample(DefaultSampler, In.vTexcoord);
 //   vector vMtrlDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
	float4 vNormalSample = g_NormalTexture.Sample(DefaultSampler, In.vTexcoord);

 //   float3 normal = normalize(vNormalSample.xyz * 2.f - 1.f);
 //   float3 lightDir = normalize(-g_vLightDir.xyz);
 //   float  NdotL = all(max(dot(normal, lightDir), 0.f));
 //   float  isLit = step(g_fShadowThreshold, NdotL);
 //   float4 baseColor = vMtrlDiffuse;
 //   float4 litColor = baseColor * g_vLightDiffuse;
 //   float4 shad = vMtrlDiffuse * g_vShadowColor;
 //   float4 toonColor = lerp(shad, litColor, isLit);

 //   float4 amb = vMtrlDiffuse * (g_fLightAmbient * g_fMtrlAmbient) * g_fAmbientStrength;

 //   // 최종 및 채도 보정
 //   float4 finalColor = toonColor + amb;
 //   finalColor = AdjustSaturation(finalColor, g_fSaturationBoost);

    // 툰으로 그리기
    if (vNormalSample.w > 0.0f)
    {
        vector vShade = g_ShadeTexture.Sample(DefaultSampler, In.vTexcoord);
        vector vMtrlDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
        float4 vNormalSample = g_NormalTexture.Sample(DefaultSampler, In.vTexcoord);

        float3 normal = normalize(vNormalSample.xyz * 2.f - 1.f);
        float3 lightDir = normalize(-g_vLightDir.xyz);
        float  NdotL = all(max(dot(normal, lightDir), 0.f));
        float  isLit = step(g_fShadowThreshold, NdotL);
        float4 baseColor = vMtrlDiffuse;
        float4 litColor = baseColor * g_vLightDiffuse;
        float4 shad = vMtrlDiffuse * g_vShadowColor;
        //float4 toonColor = lerp(shad, litColor, isLit);

        float levels = 3;                                 // 단계 수
        float d = floor(NdotL * levels) / (levels - 1);   // 0.0, 0.33, 0.66, 1.0 등
        float4 toonColor = lerp(shad, litColor, d);

        float4 amb = vMtrlDiffuse * (g_fLightAmbient) * g_fAmbientStrength;

        // 최종 및 채도 보정
        float4 finalColor = toonColor + amb;
        finalColor = AdjustSaturation(finalColor, g_fSaturationBoost);

       
        if (vNormalSample.w > 0.5f)
        {
            float2 texelSize = 1.0 / float2(g_iWinSizeX, g_iWinSizeY);

             float edge = DetectEdge(In.vTexcoord, texelSize);
             finalColor = lerp(saturate(finalColor), g_vOutlineColor, edge * g_fOutlineStrength);
             Out.vBackBuffer = finalColor;
        }
        else
        {
            finalColor = AdjustSaturation(finalColor, 0.5f);
			Out.vBackBuffer = saturate(finalColor);
        }
        return Out;
    }
    else
    {

        vector vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
        //if (all(vDiffuse.rgb == 0.f))
        if (vDiffuse.a == 0.f)
            discard;

        vector vShade = g_ShadeTexture.Sample(DefaultSampler, In.vTexcoord);

        Out.vBackBuffer = vDiffuse * vShade;

    }

  //  Out.vBackBuffer = finalColor;
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
 
        PixelShader = compile ps_5_0 PS_MAIN_LIGHT_DIRECTIONAL();
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

    //pass Deferred_ToonClamp
    //{
    //    SetRasterizerState(RS_Default);
    //    SetDepthStencilState(DSS_None, 0);
    //    SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);


    //    VertexShader = compile vs_5_0 VS_MAIN();
    //    PixelShader = compile ps_5_0 PS_MAIN_DEFERRED_TOON_WRAP();
    //}
  
}
