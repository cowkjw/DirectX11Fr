#include "UIManager.h"

CUIManager::CUIManager()
{
}

void CUIManager::Update_UI(_float fTimeDelta)
{
	Priority_Update(fTimeDelta);
	Update(fTimeDelta);
	Late_Update(fTimeDelta);
	Render();
}

void CUIManager::Priority_Update(_float fTimeDelta)
{
	for (auto& Pair : m_mapCanvasUI)
	{
		CUICanvas* pCanvasUI = Pair.second;
		if (pCanvasUI && pCanvasUI->IsActive())
			pCanvasUI->Priority_Update(fTimeDelta);
	}
}

void CUIManager::Update(_float fTimeDelta)
{
	for (auto& Pair : m_mapCanvasUI)
	{
		CUICanvas* pCanvasUI = Pair.second;
		if (pCanvasUI && pCanvasUI->IsActive())
			pCanvasUI->Update(fTimeDelta);
	}
}

void CUIManager::Late_Update(_float fTimeDelta)
{
	for (auto& Pair : m_mapCanvasUI)
	{
		CUICanvas* pCanvasUI = Pair.second;
		if (pCanvasUI && pCanvasUI->IsActive())
			pCanvasUI->Late_Update(fTimeDelta);
	}
}

void CUIManager::Render()
{
	for (auto& Pair : m_mapCanvasUI)
	{
		CUICanvas* pCanvasUI = Pair.second;
		if (pCanvasUI && pCanvasUI->IsActive())
			pCanvasUI->Render();
	}
}

void CUIManager::AddCanvasUI(CUICanvas* pUI)
{
	if (nullptr == pUI)
		return;
	auto iter = m_mapCanvasUI.find(pUI->Get_Name());
	if (iter == m_mapCanvasUI.end())
	{
		m_mapCanvasUI.emplace(pUI->Get_Name(), pUI);
		Safe_AddRef(pUI);
	}
}

void CUIManager::RemoveCanvasUI(const _wstring& canvasName)
{
	auto iter = m_mapCanvasUI.find(canvasName);
	if (iter != m_mapCanvasUI.end())
	{
		auto& pCanvasUI = iter->second;
		Safe_Release(pCanvasUI);
		m_mapCanvasUI.erase(iter);
	}
}

void CUIManager::RemoveUI(const _wstring& canvasName, const _wstring& uiName)
{
	auto iter = m_mapCanvasUI.find(canvasName);
	if (iter != m_mapCanvasUI.end())
	{
		CUICanvas* pCanvasUI = iter->second;
		if (pCanvasUI)
		{
			pCanvasUI->RemoveChildUI(uiName);
		}
	}
}

void CUIManager::ClearCanvas()
{
	for (auto& Pair : m_mapCanvasUI)
	{
		Safe_Release(Pair.second);
	}
	m_mapCanvasUI.clear();
}

CUIObject* CUIManager::GetUI(const _wstring& canvasName, const _wstring& uiName)
{
	auto iter = m_mapCanvasUI.find(canvasName);
	if (iter != m_mapCanvasUI.end())
	{
		CUICanvas* pCanvasUI = iter->second;
		if (pCanvasUI)
		{
			return pCanvasUI->GetChildUI(uiName);
		}
	}
	return nullptr;
}

HRESULT CUIManager::Initialize()
{
	return S_OK;
}

CUIManager* CUIManager::Create()
{
	CUIManager* pInstance = new CUIManager();
	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("CUIManager Created Failed");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CUIManager::Free()
{
	__super::Free();
	for (auto& Pair : m_mapCanvasUI)
	{

		Safe_Release(Pair.second);
	}
	m_mapCanvasUI.clear();
}
