// CFrustumCull_DX11.cpp
#include "FrustumCull.h"
#include "GameInstance.h"

CFrustumCull::CFrustumCull()
    : m_pGameInstance{ CGameInstance::Get_Instance() }
{
    Safe_AddRef(m_pGameInstance);
}

HRESULT CFrustumCull::Initialize_Prototype()
{
	// 1) 평면 초기화
	for (_int i = 0; i < 6; ++i)
		m_Planes[i] = XMPlaneNormalize(XMVectorZero());
	// 2) 뷰·투영 행렬 초기화
	m_ViewProj = XMMatrixIdentity();

    return S_OK;
}

void CFrustumCull::Update(_float4x4 const& viewF, _float4x4 const& projF)
{

    // 1) XMMATRIX 로 변환
    _matrix view = XMLoadFloat4x4(&viewF);
    _matrix proj = XMLoadFloat4x4(&projF);
    m_ViewProj = XMMatrixMultiply(view, proj);

    // 2) 행렬 요소에서 평면 뽑기 (왼쪽, 오른쪽, 위, 아래, 근, 원)
    _float4x4 vp; XMStoreFloat4x4(&vp, m_ViewProj);

    // left
    m_Planes[0] = XMPlaneNormalize(XMVectorSet(
        vp._14 + vp._11, vp._24 + vp._21, vp._34 + vp._31, vp._44 + vp._41));
    // right
    m_Planes[1] = XMPlaneNormalize(XMVectorSet(
        vp._14 - vp._11, vp._24 - vp._21, vp._34 - vp._31, vp._44 - vp._41));
    // top
    m_Planes[2] = XMPlaneNormalize(XMVectorSet(
        vp._14 - vp._12, vp._24 - vp._22, vp._34 - vp._32, vp._44 - vp._42));
    // bottom
    m_Planes[3] = XMPlaneNormalize(XMVectorSet(
        vp._14 + vp._12, vp._24 + vp._22, vp._34 + vp._32, vp._44 + vp._42));
    // near
    m_Planes[4] = XMPlaneNormalize(XMVectorSet(
        vp._13, vp._23, vp._33, vp._43));
    // far
    m_Planes[5] = XMPlaneNormalize(XMVectorSet(
        vp._14 - vp._13, vp._24 - vp._23, vp._34 - vp._33, vp._44 - vp._43));
}


_bool CFrustumCull::IsPointInFrustum(_float3 const& vPt)const
{
    _vector pt = XMLoadFloat3(&vPt);
    for (_int i = 0; i < 6; ++i)
    {
        // 평면 방정식: dot(plane, (pt,1))
        _float d = XMVectorGetX(XMPlaneDotCoord(m_Planes[i], pt));
        if (d < 0) return false;
    }
    return true;
}

_bool CFrustumCull::IsSphereInFrustum(_float3 const& vCenter, _float fRadius) const
{
    _vector center = XMLoadFloat3(&vCenter);
    for (_int i = 0; i < 6; ++i)
    {
        _float d = XMVectorGetX(XMPlaneDotCoord(m_Planes[i], center));
        if (d < -fRadius) return false;
    }
    return true;
}

_bool CFrustumCull::IsAABBInFrustum(_float3 const& vCenter, _float3 const& vHalfExtents) const
{
    _vector center = XMLoadFloat3(&vCenter);
    _vector half = XMLoadFloat3(&vHalfExtents);
    for (int i = 0; i < 6; ++i)
    {
        // AABB 반경 r = ext.x*|nx| + ext.y*|ny| + ext.z*|nz|
        _float4 p; XMStoreFloat4(&p, m_Planes[i]);
        float r = vHalfExtents.x * fabsf(p.x) + vHalfExtents.y * fabsf(p.y) + vHalfExtents.z * fabsf(p.z);
        float d = XMVectorGetX(XMPlaneDotCoord(m_Planes[i], center));
        if (d < -r) return false;
    }
    return true;
}

CFrustumCull* CFrustumCull::Create()
{
    CFrustumCull* pInstance = new CFrustumCull();
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created CFrustumCull");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CFrustumCull::Free()
{
	__super::Free();
	Safe_Release(m_pGameInstance);
}
