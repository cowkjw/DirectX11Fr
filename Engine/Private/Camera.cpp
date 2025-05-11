#include "Camera.h"
#include "GameInstance.h"

CCamera::CCamera(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CCamera::CCamera(const CCamera& Prototype)
	: CGameObject(Prototype)
{
}

HRESULT CCamera::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CCamera::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	CAMERA_DESC* pDesc = static_cast<CAMERA_DESC*>(pArg);
	if (nullptr == pDesc)
		return E_FAIL;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&pDesc->vEye), 1.f));
	m_pTransformCom->LookAt(XMVectorSetW(XMLoadFloat3(&pDesc->vAt), 1.f));

	m_fFov = pDesc->fFov;
	m_fNear = pDesc->fNear;
	m_fFar = pDesc->fFar;

	_uint					iNumViewports = { 1 };
	D3D11_VIEWPORT			ViewportDesc{};

	m_pContext->RSGetViewports(&iNumViewports, &ViewportDesc);

	m_fAspect = static_cast<_float>(ViewportDesc.Width) / ViewportDesc.Height;


	return S_OK;
}

void CCamera::Priority_Update(_float fTimeDelta)
{
}

void CCamera::Update(_float fTimeDelta)
{
}

void CCamera::Late_Update(_float fTimeDelta)
{
}

HRESULT CCamera::Update_Camera()
{
	m_pGameInstance->Set_Transform(TRANSFORM::VIEW ,m_pTransformCom->Get_WorldMatrix_Inverse());
	m_pGameInstance->Set_Transform(TRANSFORM::PROJECTION, XMMatrixPerspectiveFovLH(m_fFov, m_fAspect, m_fNear, m_fFar));

	return S_OK;
}

void CCamera::Free()
{
	__super::Free();
}