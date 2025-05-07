#include "UICanvas.h"

CUICanvas::CUICanvas(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CUIObject(pDevice, pContext)
	, m_vecChildUIObjects()
{
}

CUICanvas::CUICanvas(const CUICanvas& Prototype)
	: CUIObject(Prototype)
	, m_vecChildUIObjects(Prototype.m_vecChildUIObjects)
{
}


HRESULT CUICanvas::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUICanvas::Initialize(void* pArg)
{
	return S_OK;
}

// 나중에 캔버스만 오브젝트 매니저에 넣을 예정
void CUICanvas::Priority_Update(_float fTimeDelta)
{
	for (auto& pChild : m_vecChildUIObjects)
	{
		if (pChild && pChild->IsActive())
			pChild->Priority_Update(fTimeDelta);
	}
}

void CUICanvas::Update(_float fTimeDelta)
{
	for (auto& pChild : m_vecChildUIObjects)
	{
		if(pChild&&pChild->IsActive())
			pChild->Update(fTimeDelta);
	}
	SortChildUI();
}

void CUICanvas::Late_Update(_float fTimeDelta)
{
	for (auto& pChild : m_vecChildUIObjects)
	{
		if (pChild && pChild->IsActive())
			pChild->Late_Update(fTimeDelta);
	}
}

HRESULT CUICanvas::Render()
{
	for (auto& pChild : m_vecChildUIObjects)
	{
		if (pChild && pChild->IsActive())
			pChild->Render();
	}
	return S_OK;
}

void CUICanvas::AddChildUI(CUIObject* pChildUI)
{
	if (nullptr == pChildUI)
		return;
	Safe_AddRef(pChildUI);
	m_vecChildUIObjects.push_back(pChildUI);
	pChildUI->SetParent(this);
	pChildUI->SetSortingOrder(m_iSortingOrder + 1);
	SortChildUI();
}

void CUICanvas::RemoveChildUI(CUIObject* pChildUI)
{
	if (nullptr == pChildUI)
		return;
	auto iter = find(m_vecChildUIObjects.begin(), m_vecChildUIObjects.end(), pChildUI);
	if (iter != m_vecChildUIObjects.end())
	{
		Safe_Release(pChildUI);
		m_vecChildUIObjects.erase(iter);
		SortChildUI();
	}
}

void CUICanvas::SortChildUI()
{
	sort(m_vecChildUIObjects.begin(), m_vecChildUIObjects.end(),
		[](CUIObject* pA, CUIObject* pB) { return pA->GetSortingOrder() < pB->GetSortingOrder(); });
}

CUICanvas* CUICanvas::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUICanvas* pInstance = new CUICanvas(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created CUICanvas");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CUIObject* CUICanvas::Clone(void* pArg)
{
	CUICanvas* pInstance = new CUICanvas(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created CUICanvas");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CUICanvas::Free()
{
	__super::Free();
	for (auto& pChild : m_vecChildUIObjects)
	{
		Safe_Release(pChild);
	}
	m_vecChildUIObjects.clear();
}
