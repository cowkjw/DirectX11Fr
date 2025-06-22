#include "GameObject.h"
#include "GameInstance.h"


_uint CGameObject::s_uNextID = 0;

CGameObject::CGameObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
	, m_pGameInstance{ CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pDevice);
	m_uID = s_uNextID++;
}

CGameObject::CGameObject(const CGameObject& Prototype)
	: m_pDevice{ Prototype.m_pDevice }
	, m_pContext{ Prototype.m_pContext }
	, m_pGameInstance{ CGameInstance::Get_Instance() }
	, m_bIsCloned(true)
	, m_Components{ Prototype.m_Components }
{
	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pDevice);
	m_uID = s_uNextID++;

	for (auto& Pair : m_Components)
	{
		Safe_AddRef(Pair.second);
		Pair.second->SetOwner(this);
	}
}

CComponent* CGameObject::Get_Component(const _wstring& strComponentTag)
{
	auto	iter = m_Components.find(strComponentTag);
	if (iter == m_Components.end())
		return nullptr;

	return iter->second;
}

HRESULT CGameObject::Add_Component(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, const _wstring& strComponentTag, CComponent** ppOut, void* pArg)
{
	CComponent* pComponent = static_cast<CComponent*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::COMPONENT, iPrototypeLevelIndex, strPrototypeTag, pArg));
	if (nullptr == pComponent)
		return E_FAIL;

	m_Components.emplace(strComponentTag, pComponent);

	*ppOut = pComponent;

	Safe_AddRef(pComponent);
	pComponent->SetOwner(this);
	pComponent->SetPrototypeLevel(iPrototypeLevelIndex);
	pComponent->SetCreateLevel(m_iCreateLevel);
	pComponent->SetProtoTypeTag(strPrototypeTag);

	return S_OK;
}

HRESULT CGameObject::Add_Component(const _wstring& strComponentTag, CComponent* pComponent, CComponent** ppOut)
{
	if (nullptr == pComponent)
		return E_FAIL;
	m_Components.emplace(strComponentTag, pComponent);

	*ppOut = pComponent;
	Safe_AddRef(pComponent);
	pComponent->SetOwner(this);
	return S_OK;
}

HRESULT CGameObject::Initialize_Prototype()
{

	return S_OK;
}

HRESULT CGameObject::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return S_OK;

	GAMEOBJECT_DESC* pDesc = static_cast<GAMEOBJECT_DESC*>(pArg);
	m_strName = pDesc->strName;
	m_strTag = pDesc->strTag;

	m_pTransformCom = CTransform::Create(m_pDevice, m_pContext);
	if (nullptr == m_pTransformCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Initialize(pArg)))
		return E_FAIL;

	m_pTransformCom->SetOwner(this);
	m_Components.emplace(g_strTransformTag, m_pTransformCom);

	Safe_AddRef(m_pTransformCom);


	return S_OK;
}

void CGameObject::Priority_Update(_float fTimeDelta)
{

}

void CGameObject::Update(_float fTimeDelta)
{

}

void CGameObject::Late_Update(_float fTimeDelta)
{
	if (m_pTransformCom && m_pTransformCom->IsDirty())
	{
		for (auto& child : m_vecChildren)
		{
			if (child && child->IsActive())
			{
				child->GetTransform()->FollowParent(m_pTransformCom);
			}
		}
		m_pTransformCom->SetDirty(false);
	}
}

void CGameObject::AddChild(CGameObject* pGameObject)
{
	if (nullptr == pGameObject)
		return;
	m_vecChildren.push_back(pGameObject);
	pGameObject->SetParent(this);
	m_pTransformCom->Set_Parent(pGameObject);
}

HRESULT CGameObject::Render()
{

	return S_OK;
}

json CGameObject::Serialize()
{
	json j;
	j["ID"] = m_uID;
	j["parentId"] = m_pParent ? m_pParent->GetID() : 0;
	j["name"] = WStringToString(m_strName);
	j["tag"] = WStringToString(m_strTag);
	j["ProtoTypeTag"] = WStringToString(m_strPrototypeTag);
	j["isActive"] = m_bIsActive;
	j["ProtoLevel"] = m_iPrototypeLevel;
	j["CreateLevel"] = m_iCreateLevel;
	// Serialize components
	for (const auto& pair : m_Components)
	{
		if (pair.second)
		{
			json componentJson = pair.second->Serialize();
			j["components"][WStringToString(pair.first)] = componentJson;
		}
	}
	return j;
}

void CGameObject::Deserialize(const json& j)
{
	m_uID = j["ID"].get<_uint>();
	m_strName = StringToWString(j["name"].get<string>());
	m_strTag = StringToWString(j["tag"].get<string>());
	m_strPrototypeTag = StringToWString(j["ProtoTypeTag"].get<string>());
	m_iPrototypeLevel = j["ProtoLevel"].get<_uint>();
	m_iCreateLevel = j["CreateLevel"].get<_uint>();
	m_bIsActive = j["isActive"].get<_bool>();
	// Deserialize components
	//for (auto& pair : m_Components)
	//{
	//	auto iter = j["components"].find(WStringToString(pair.first));
	//	if (iter != j["components"].end())
	//	{
	//		pair.second->Deserialize(iter.value());
	//	}
	//}
}

void CGameObject::Free()
{
	__super::Free();

	Safe_Release(m_pGameInstance);
	Safe_Release(m_pTransformCom);


	for (auto& Pair : m_Components)
		Safe_Release(Pair.second);
	m_Components.clear();


	for (auto& pChild : m_vecChildren)
	{
		Safe_Release(pChild);
	}
	m_vecChildren.clear();


	Safe_Release(m_pContext);
	Safe_Release(m_pDevice);
}
