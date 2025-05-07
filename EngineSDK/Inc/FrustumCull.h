#pragma once
#include "Base.h"

BEGIN_NAMESPACE(Engine)
class CFrustumCull final : public CBase
{
private:
    CFrustumCull();
	virtual ~CFrustumCull() = default;
public:
    // 카메라로부터 뷰, 투영 행렬을 받습니다.
    void    Update(_float4x4 const& viewF, _float4x4 const& projF);
    HRESULT Initialize_Prototype();

    _bool    IsPointInFrustum(_float3 const& vPt) const;
    _bool    IsSphereInFrustum(_float3 const& vCenter, _float fRadius) const;
    _bool    IsAABBInFrustum(_float3 const& vCenter, _float3 const& vHalfExtents) const;
private:
    _vector m_Planes[6];

    // 뷰·투영 곱
    _matrix m_ViewProj;
	class CGameInstance* m_pGameInstance = { nullptr };
public:
	static CFrustumCull* Create();  
    virtual void Free() override;
};
END_NAMESPACE