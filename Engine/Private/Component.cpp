#include "Component.h"
#include "GameObject.h"
#include "GameInstance.h"

CComponent::CComponent(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
	, m_isCloned{ false }
	, m_pGameInstance{ CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

CComponent::CComponent(const CComponent& Prototype)
	: m_pDevice{ Prototype.m_pDevice }
	, m_pContext{ Prototype.m_pContext }
	, m_isCloned{ true }
	,m_pOwner(Prototype.m_pOwner)
	, m_pGameInstance{ Prototype.m_pGameInstance }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

HRESULT CComponent::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CComponent::Initialize(void* pArg)
{
	return S_OK;
}

void CComponent::SetOwner(CGameObject* pOwner)
{
	if (pOwner)
	{
		m_pOwner = pOwner;
	}
}

json CComponent::Serialize()
{
	json j;
	j["ComponentTag"] = WStringToString(m_strPrototypeTag);
	j["ComponentLevel"] = m_iPrototypeLevel;
	j["ComponentCreateLevel"] = m_iCreateLevel;
	j["IsActive"] = m_bIsActive;
	return j;
}

void CComponent::Deserialize(const json& j)
{
	m_strPrototypeTag = StringToWString(j["ComponentTag"].get<string>());
	m_iPrototypeLevel = j["ComponentLevel"].get<_uint>();
	m_iCreateLevel = j["ComponentCreateLevel"].get<_uint>();
	m_bIsActive = j["IsActive"].get<_bool>();
}



void CComponent::Free()
{
	__super::Free();

	Safe_Release(m_pContext);
	Safe_Release(m_pDevice);
	Safe_Release(m_pGameInstance);
}
