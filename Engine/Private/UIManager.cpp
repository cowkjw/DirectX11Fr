#include "UIManager.h"
#include "UIImage.h"
#include "UIButton.h"
#include "UIProgressBar.h"

CUIManager::CUIManager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice },
	m_pContext{ pContext }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

HRESULT CUIManager::Initialize()
{
	return S_OK;
}

void CUIManager::Update_UI(_float fTimeDelta)
{
	Priority_Update(fTimeDelta);
	Update(fTimeDelta);
	Late_Update(fTimeDelta);
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


void CUIManager::AddCanvasUI(CUICanvas* pUI)
{
	if (nullptr == pUI)
		return;
	auto iter = m_mapCanvasUI.find(pUI->Get_Name());
	if (iter == m_mapCanvasUI.end())
	{
		m_mapCanvasUI.emplace(pUI->Get_Name(), pUI);
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

CGameObject* CUIManager::CreateUI(CUIObject::UIOBJECT_DESC* pDesc, UI_TYPE eUIType)
{
	if (nullptr == pDesc)
		return nullptr;

	CUIObject* pUI = nullptr;
	switch (eUIType)
	{
	case UI_TYPE::IMAGE:
		pUI = CUIImage::Create(m_pDevice, m_pContext);
		break;
	case UI_TYPE::BUTTON:
		pUI = CUIButton::Create(m_pDevice, m_pContext);
		break;
	case UI_TYPE::BAR:
		pUI = CUIProgressBar::Create(m_pDevice, m_pContext);
		break;
	case UI_TYPE::CANVAS:
		pUI = CUICanvas::Create(m_pDevice, m_pContext);
		break;
	}
	if (nullptr == pUI)
		return nullptr;
	if (FAILED(pUI->Initialize(pDesc)))
		return nullptr;

	if (eUIType == UI_TYPE::CANVAS)
		AddCanvasUI(static_cast<CUICanvas*>(pUI));
	return pUI;
}

CUIManager* CUIManager::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUIManager* pInstance = new CUIManager(pDevice, pContext);
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
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}
