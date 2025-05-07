#include "FreeCamera.h"
#include "GameInstance.h"
CFreeCamera::CFreeCamera(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCamera{ pDevice, pContext }
{
}

CFreeCamera::CFreeCamera(const CFreeCamera& Prototype)
	: CCamera(Prototype)
{
}

HRESULT CFreeCamera::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CFreeCamera::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	CAMERA_FREE_DESC* pDesc = static_cast<CAMERA_FREE_DESC*>(pArg);
	m_fMouseSensor = pDesc->fMouseSensor;

    return S_OK;
}

void CFreeCamera::Priority_Update(_float fTimeDelta)
{

}

void CFreeCamera::Update(_float fTimeDelta)
{
	if (m_pGameInstance->IsKeyDown('W'))
	{
		m_pTransformCom->Go_Straight(fTimeDelta);
	}
	if (m_pGameInstance->IsKeyDown('S'))
	{
		m_pTransformCom->Go_Backward(fTimeDelta);
	}
	if (m_pGameInstance->IsKeyDown('A'))
	{
		m_pTransformCom->Go_Left(fTimeDelta);
	}
	if (m_pGameInstance->IsKeyDown('D'))
	{
		m_pTransformCom->Go_Right(fTimeDelta);
	}

	POINT vMouseDelta = m_pGameInstance->GetMouseDelta();


	__super::Update_Camera();
}

void CFreeCamera::Late_Update(_float fTimeDelta)
{
}

CFreeCamera* CFreeCamera::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CFreeCamera* pInstance = new CFreeCamera(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CCamera_Free");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CFreeCamera::Clone(void* pArg)
{
	CFreeCamera* pInstance = new CFreeCamera(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CFreeCamera");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CFreeCamera::Free()
{
}
