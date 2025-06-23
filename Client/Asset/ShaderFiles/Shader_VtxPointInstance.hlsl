//#include "Engine_Shader_Defines.hlsli"
//
//matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
//texture2D g_Texture;
//vector g_vCamPosition;
//
//struct VS_IN
//{
//    float3 vPosition : POSITION;   
//    float2 vPSize : PSIZE;
//    
//    row_major float4x4 TransformMatrix : WORLD;
//    
//    float2 vLifeTime : TEXCOORD4;    
//};
//
//struct VS_OUT
//{
//    /* SV_ : ShaderValue약자 */
//    /* 내가 해야할 연산은 다 했으니 이제 니(장치)가 알아서 추가적인 연산을 해라. */     
//    float4 vPosition : POSITION;
//    
//    float2 vLifeTime : TEXCOORD0;
//};
//
//VS_OUT VS_MAIN(VS_IN In)
//{
//    VS_OUT Out;
//    
//    matrix matWV, matWVP;
//    
//   
//    vector vPosition = mul(vector(In.vPosition, 1.f), In.TransformMatrix);
//    
//    
//    Out.vPosition = mul(vPosition, g_WorldMatrix);
//    
//    Out.vLifeTime = In.vLifeTime;
//    
//    return Out;
//}
//
///* 그리는 형태에 따라서 호출된다. */ 
//
//struct GS_IN
//{
//    float4 vPosition : POSITION;    
//    float2 vLifeTime : TEXCOORD0;
//};
//
//struct GS_OUT
//{
//    float4 vPosition : SV_POSITION;
//    float2 vTexcoord : TEXCOORD0;
//    float2 vLifeTime : TEXCOORD1;
//};
//
//
//void GS_MAIN(point GS_IN In[1], inout TriangleStream<GS_OUT> Triangles)
//{
//    GS_OUT Out[4];
//    
//    Triangles.Append(Out[0]);
//    Triangles.Append(Out[1]);
//    Triangles.Append(Out[2]);
//    Triangles.RestartStrip();
//    
//    Triangles.Append(Out[0]);
//    Triangles.Append(Out[2]);
//    Triangles.Append(Out[3]);
//    Triangles.RestartStrip();
//}
//
//
//struct PS_IN
//{
//    float4 vPosition : SV_POSITION;
//    float2 vTexcoord : TEXCOORD0;
//    float2 vLifeTime : TEXCOORD1;
//};
//
//struct PS_OUT
//{
//    vector vColor : SV_TARGET0;
//};
//
//PS_OUT PS_MAIN(PS_IN In)
//{
//    PS_OUT Out;    
//    
//    Out.vColor = g_Texture.Sample(DefaultSampler, In.vTexcoord);           
//    
//    if(Out.vColor.a < 0.3f)
//        discard;
//    
//    Out.vColor.a = saturate(In.vLifeTime.x - In.vLifeTime.y);
//    
//    float2 vRG = 1.f - Out.vColor.a + 0.3f;
//    
//    Out.vColor = float4(vRG.r, vRG.g, Out.vColor.b, Out.vColor.a);
//    
//    if (In.vLifeTime.y >= In.vLifeTime.x)
//        discard;
//    
//    return Out;
//}
//
//
//
//technique11 DefaultTechnique
//{
//  
//    pass Default
//    {
//        SetRasterizerState(RS_Default);
//        SetDepthStencilState(DSS_Default, 0);
//        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
//        
//
//        VertexShader = compile vs_5_0 VS_MAIN();    
//        GeometryShader = compile gs_5_0 GS_MAIN();
//        PixelShader = compile ps_5_0 PS_MAIN();      
//    }
// 
//}
