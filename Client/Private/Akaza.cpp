#include "Akaza.h"
#include "GameInstance.h"
#include "Animation.h"
#include "StateIdle.h"
#include "InputBuffer.h"
#include "Weapon.h"	


CAkaza::CAkaza(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CBaseCharacter(pDevice, pContext)
{

}

CAkaza::CAkaza(const CAkaza& Prototype)
	: CBaseCharacter(Prototype)
{
}
HRESULT CAkaza::Initialize_Prototype()
{

	return S_OK;
}

HRESULT CAkaza::Initialize(void* pArg)
{
	GAMEOBJECT_DESC			Desc{};

	Desc.fRotationPerSec = XMConvertToRadians(90.f);
	Desc.fSpeedPerSec = 40.f;
	Desc.strName = TEXT("Akaza");

	if (FAILED(CGameObject::Initialize(&Desc)))
		return E_FAIL;

	m_pTransformCom->Scaling(_float3(0.1f, 0.1f, 0.1f));

	if (FAILED(Ready_Components()))
		return E_FAIL;

	Ready_Animation();


	if (FAILED(__super::Add_Component(ToIndex(LEVEL::STATIC), TEXT("Prototype_Component_CapsuleCollider"),
		TEXT("Com_Collider"), reinterpret_cast<CComponent**>(&m_pColliderCom))))
		return E_FAIL;
	m_pColliderCom->SetListener(this);
	return S_OK;
}

void CAkaza::Priority_Update(_float fTimeDelta)
{
//	__super::Priority_Update(fTimeDelta);
}

void CAkaza::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CAkaza::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CAkaza::Render()
{
	__super::Render();
	return S_OK;
}

HRESULT CAkaza::Ready_Components()
{
	if (FAILED(__super::Add_Component(TEXT("Com_Shader"), m_pGameInstance->GetShader(TEXT("Shader_VtxAnimMesh"), true), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;


	/* For.Com_Model */
	if (FAILED(__super::Add_Component(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_Akaza"),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;

	/* For.Com_AnimController*/
	if (FAILED(__super::Add_Component(ToIndex(LEVEL::STATIC), TEXT("Prototype_Component_Animator"),
		TEXT("Com_Animator"), reinterpret_cast<CComponent**>(&m_pAnimatroCom), m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

void CAkaza::Ready_Animation()
{
}

CAkaza* CAkaza::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CAkaza* pInstance = new CAkaza(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CAkaza");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CAkaza::Clone(void* pArg)
{
	CAkaza* pInstance = new CAkaza(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created : CAkaza");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CAkaza::Free()
{
	__super::Free();
}
