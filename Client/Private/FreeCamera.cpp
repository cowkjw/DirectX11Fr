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
	CCamera::CAMERA_DESC			Desc{};

	Desc.vEye = _float3(0.f, 20.f, -15.f);
	Desc.vAt = _float3(0.f, 0.f, 0.f);
	Desc.fFov = XMConvertToRadians(60.0f);
	Desc.fNear = 0.1f;
	Desc.fFar = 500.f;
	Desc.fRotationPerSec = XMConvertToRadians(180.0f);
	Desc.fSpeedPerSec = 10.0f;
	m_strName = TEXT("Camera");
	m_fMouseSensor = 0.1f;
	if (FAILED(__super::Initialize(&Desc)))
		return E_FAIL;

	//CAMERA_FREE_DESC* pDesc = static_cast<CAMERA_FREE_DESC*>(pArg);
	//m_fMouseSensor = pDesc->fMouseSensor;

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

	if (m_pGameInstance->IsMouseDown(1))
	{
		POINT vMouseDelta = m_pGameInstance->GetMouseDelta();
		_long			MouseMove = {};

		if (vMouseDelta.x != 0)
		{
			_float fYawAngle = vMouseDelta.x * fTimeDelta * m_fMouseSensor;
			// Up 축 (0,1,0) 기준으로 Yaw
			m_pTransformCom->Turn(
				XMVectorSet(0.f, 1.f, 0.f, 0.f),
				fYawAngle
			);
		}

		if (vMouseDelta.y != 0)
		{
			_float fPitchAngle = vMouseDelta.y * fTimeDelta * m_fMouseSensor;
			// Right 축 기준으로 Pitch
			m_pTransformCom->Turn(
				m_pTransformCom->Get_State(STATE::RIGHT),
				fPitchAngle
			);
		}
	}
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
