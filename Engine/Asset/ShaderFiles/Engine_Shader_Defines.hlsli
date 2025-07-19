
sampler DefaultSampler = sampler_state
{
    filter = min_mag_mip_linear;
    AddressU = wrap;
    AddressV = wrap;

};

sampler PointSampler = sampler_state
{
    filter = min_mag_mip_point;
    AddressU = wrap;
    AddressV = wrap;
};

sampler MirrSmp = sampler_state
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = MIRROR; // U축을 미러 반복
    AddressV = CLAMP;
};

sampler LinearClampSampler = sampler_state
{
    filter = min_mag_mip_linear;
    AddressU = clamp;
    AddressV = clamp;
};

RasterizerState RS_Default
{
    FillMode = Solid;
    CullMode = back;
};

RasterizerState RS_Cull_Front
{
    FillMode = Solid;
    CullMode = front;
};

RasterizerState RS_Cull_None
{
    FillMode = Solid;
    CullMode = none;
};


DepthStencilState DSS_Default
{
    DepthEnable = true;
    DepthWriteMask = all;
    DepthFunc = less_equal;
};

DepthStencilState DSS_None
{
    DepthEnable = false;
    DepthWriteMask = zero;    
};

BlendState BS_Default
{
    BlendEnable[0] = false;

};

BlendState BS_AlphaBlend
{
    BlendEnable[0] = true;
    SrcBlend = Src_Alpha;
    DestBlend = Inv_Src_Alpha;
    BlendOp = Add;
};

BlendState BS_Additive
{
    // 첫 번째 렌더 타겟에만 적용
    BlendEnable[0] = true;
    // RGB, Alpha 모두 가산
    SrcBlend = One;
    DestBlend = One;
    BlendOp = Add;
    SrcBlendAlpha = One;
    DestBlendAlpha = One;
    BlendOpAlpha = Add;
    // RGBA 모두 출력
    RenderTargetWriteMask[0] = 0x0F;
};