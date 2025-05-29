#include "Tanjiro.h"
#include "GameInstance.h"
#include "Animation.h"
#include "StateIdle.h"
#include "InputBuffer.h"
#include "Weapon.h"	


CTanjiro::CTanjiro(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CBaseCharacter(pDevice, pContext)
{

}

CTanjiro::CTanjiro(const CTanjiro& Prototype)
	: CBaseCharacter(Prototype)
{
}
HRESULT CTanjiro::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CTanjiro::Initialize(void* pArg)
{
	GAMEOBJECT_DESC			Desc{};

	Desc.fRotationPerSec = XMConvertToRadians(90.f);
	Desc.fSpeedPerSec = 40.f;
	Desc.strName = TEXT("BaseCharacter");

	if (FAILED(__super::Initialize(&Desc)))
		return E_FAIL;

	m_pTransformCom->Scaling(_float3(0.1f, 0.1f, 0.1f));

	Ready_Animation();
    return S_OK;
}

void CTanjiro::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CTanjiro::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CTanjiro::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CTanjiro::Render()
{
	return __super::Render();
}

HRESULT CTanjiro::Ready_Components()
{
	if (FAILED(__super::Add_Component(TEXT("Com_Shader"), m_pGameInstance->GetShader(TEXT("Shader_VtxAnimMesh"), true), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;


	/* For.Com_Model */
	if (FAILED(__super::Add_Component(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_Fiona"),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;

	/* For.Com_AnimController*/
	if (FAILED(__super::Add_Component(ToIndex(LEVEL::STATIC), TEXT("Prototype_Component_Animator"),
		TEXT("Com_Animator"), reinterpret_cast<CComponent**>(&m_pAnimatroCom), m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

void CTanjiro::Ready_Animation()
{
}

CTanjiro* CTanjiro::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CTanjiro* pInstance = new CTanjiro(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CTanjiro");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CTanjiro::Clone(void* pArg)
{
	CTanjiro* pInstance = new CTanjiro(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created : CTanjiro");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CTanjiro::Free()
{
	__super::Free();
}
