#include "Layer.h"
#include "GameObject.h"

CLayer::CLayer()
{
}

CComponent* CLayer::Get_Component(const _wstring& strComponentTag, _uint iIndex)
{
	auto	iter = m_GameObjects.begin();

	for (size_t i = 0; i < iIndex; i++)
		++iter;

	
	return (*iter)->Get_Component(strComponentTag);
}

HRESULT CLayer::Add_GameObject(CGameObject* pGameObject)
{
	if (nullptr == pGameObject)
		return E_FAIL;

	m_GameObjects.push_back(pGameObject);

	return S_OK;
}

void CLayer::Priority_Update(_float fTimeDelta)
{
	for (auto& pGameObject : m_GameObjects)
	{
		if (nullptr != pGameObject&&pGameObject->IsActive())
			pGameObject->Priority_Update(fTimeDelta);

	}
		
}

void CLayer::Update(_float fTimeDelta)
{
	for (auto& pGameObject : m_GameObjects)
	{
		if (nullptr != pGameObject && pGameObject->IsActive())
			pGameObject->Update(fTimeDelta);

	}
}

void CLayer::Late_Update(_float fTimeDelta)
{
	for (auto& pGameObject : m_GameObjects)
	{
		if (nullptr != pGameObject && pGameObject->IsActive())
			pGameObject->Late_Update(fTimeDelta);

	}
}

void CLayer::Remove_GameObject(CGameObject* pGameObject)
{
	if (nullptr == pGameObject)
		return;
	auto newEnd = remove_if(m_GameObjects.begin(), m_GameObjects.end(),
		[&](CGameObject* child) { return child == pGameObject; });
	if (newEnd != m_GameObjects.end())
	{
		m_GameObjects.erase(newEnd, m_GameObjects.end());
		for (auto& child : pGameObject->GetChildren())
		{
			if (child)
				child->SetParent(nullptr);
		}
		Safe_Release(pGameObject);
	}
}

void CLayer::Remove_GameObjectByName(const _wstring& strName)
{
	auto newEnd = remove_if(m_GameObjects.begin(), m_GameObjects.end(),
		[&](CGameObject* child) { return child->Get_Name() == strName; });
	if (newEnd != m_GameObjects.end())
	{
		m_GameObjects.erase(newEnd, m_GameObjects.end());
		for (auto& child : (*newEnd)->GetChildren())
		{
			if (child)
				child->SetParent(nullptr);
		}
		Safe_Release(*newEnd);
	}
}

CGameObject* CLayer::Find_GameObjectByName(const _wstring& strName)
{
	auto iter = find_if(m_GameObjects.begin(), m_GameObjects.end(),
		[&](CGameObject* pGameObject) { return pGameObject->Get_Name() == strName; });
	if (iter != m_GameObjects.end())
		return *iter;
	return nullptr;
}

CLayer* CLayer::Create()
{
	return new CLayer();
}

void CLayer::Free()
{
	__super::Free();

	for (auto& pGameObject : m_GameObjects)
		Safe_Release(pGameObject);
	m_GameObjects.clear();
}
