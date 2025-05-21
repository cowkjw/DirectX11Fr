#include "Picking.h"
#include "GameInstance.h"

CPicking::CPicking(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
	, m_vMouseRay{}
	, m_vMousePos{}
	, m_vLocalMouseRay{}
	, m_vLocalMousePos{}
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

HRESULT CPicking::Initialize(HWND hWnd, _uint iWinSizeX, _uint iWinSizeY)
{

	m_hWnd = hWnd;
	m_iWinSizeX = iWinSizeX;
	m_iWinSizeY = iWinSizeY;

	return S_OK;
}

void CPicking::Update(_float4x4 const& viewF, _float4x4 const& projF)
{
	POINT		ptMouse = CGameInstance::Get_Instance()->GetMousePos();
	XMMATRIX view = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&viewF));
	XMMATRIX proj = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&projF));

	// 역행렬 계산
	XMMATRIX invView = XMMatrixInverse(nullptr, view);
	XMMATRIX invProj = XMMatrixInverse(nullptr, proj);


	POINT pt = CGameInstance::Get_Instance()->GetMousePos();

	// NDC 좌표 계산 (ScreenToClient 불필요)
	_float nx = (2.0f * pt.x) / _float(m_iWinSizeX) - 1.0f;
	_float ny = -(2.0f * pt.y) / _float(m_iWinSizeY) + 1.0f;

	// 클립 공간 -> 뷰 공간
	XMVECTOR vClip = XMVectorSet(nx, ny, 0.0f, 1.0f);
	XMVECTOR vView = XMVector4Transform(vClip, invProj);
	vView = XMVectorScale(vView, 1.0f / XMVectorGetW(vView));

	// 뷰 공간 -> 월드 공간: 레이 원점, 방향
	XMStoreFloat3(&m_vMousePos,XMVector3TransformCoord(XMVectorZero(), invView));
	XMStoreFloat3(&m_vMouseRay ,XMVector3Normalize(XMVector3TransformNormal(vView, invView)));
	XMVector3TransformNormal(vView, invView);

}

_bool CPicking::Picking_InWorld(_float3& vPickedPos, const _float3& vPointA, const _float3& vPointB, const _float3& vPointC)
{
	_vector orig = XMLoadFloat3(&m_vMousePos);
	_vector dir = XMLoadFloat3(&m_vMouseRay);

	_vector v0 = XMLoadFloat3(&vPointA);
	_vector v1 = XMLoadFloat3(&vPointB);
	_vector v2 = XMLoadFloat3(&vPointC);

	// 2) Ray-Triangle 교차
	_float t = 0;
	_bool  hit = TriangleTests::Intersects(orig, dir, v0, v1, v2, t);

	if (hit)
	{
		// 3) 교차 지점 계산
		_vector pos = orig + dir * t;
		XMStoreFloat3(&vPickedPos, pos);
	}

	return (_bool)hit;

}

_bool CPicking::Picking_InLocal(_float3& vPickedPos, const _float3& vPointA, const _float3& vPointB, const _float3& vPointC)
{
	_vector orig = XMLoadFloat3(&m_vLocalMousePos);
	_vector dir = XMLoadFloat3(&m_vLocalMouseRay);

	_vector v0 = XMLoadFloat3(&vPointA);
	_vector v1 = XMLoadFloat3(&vPointB);
	_vector v2 = XMLoadFloat3(&vPointC);

	float t = 0;
	bool  hit = TriangleTests::Intersects(orig, dir, v0, v1, v2, t);

	if (hit)
	{
		_vector pos = orig + dir * t;
		XMStoreFloat3(&vPickedPos, pos);
	}

	return (_bool)hit;
}

void CPicking::Transform_ToLocalSpace(const _matrix& WorldMatrixInverse)
{
	// WorldMatrixInverse는 XMMATRIX 타입
	XMMATRIX invWorld = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&WorldMatrixInverse));

	// 월드 공간 레이 원점 → 로컬 공간
	_vector worldOrig = XMLoadFloat3(&m_vMousePos);
	_vector localOrig = XMVector3TransformCoord(worldOrig, invWorld);
	XMStoreFloat3(&m_vLocalMousePos, localOrig);

	// 월드 공간 레이 방향 → 로컬 공간 (정규화)
	_vector worldDir = XMLoadFloat3(&m_vMouseRay);
	_vector localDir = XMVector3TransformNormal(worldDir, invWorld);
	localDir = XMVector3Normalize(localDir);
	XMStoreFloat3(&m_vLocalMouseRay, localDir);
}



CPicking* CPicking::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, HWND hWnd, _uint iWinSizeX, _uint iWinSizeY)
{
	CPicking* pInstance = new CPicking(pDevice, pContext);
	if (FAILED(pInstance->Initialize(hWnd, iWinSizeX, iWinSizeY)))
	{
		MSG_BOX("Failed to Created : CPicking");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CPicking::Free()
{
	__super::Free();
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}
