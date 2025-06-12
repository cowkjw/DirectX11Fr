#include "GameplayCanvas.h"
#include "BackGround.h"
#include "UIProgressBar.h"
#include <UIButton.h>

CGameplayCanvas::CGameplayCanvas(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CUICanvas(pDevice, pContext)
{
}


CGameplayCanvas::CGameplayCanvas(const CGameplayCanvas& Prototype)
	: CUICanvas(Prototype)
{
}

HRESULT CGameplayCanvas::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CGameplayCanvas::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_strName = L"GamePlayCanvas";
	if (FAILED(Ready_ChildUI()))
		return E_FAIL;

	return S_OK;
}

HRESULT CGameplayCanvas::Ready_ChildUI()
{
	CUIImage::UIOBJECT_DESC BackGroundDesc{};
	BackGroundDesc.fX = g_iWinSizeX * 0.5f;
	BackGroundDesc.fY = g_iWinSizeY * 0.5f;
	BackGroundDesc.fSizeX = g_iWinSizeX;
	BackGroundDesc.fSizeY = g_iWinSizeY;
	BackGroundDesc.strName = L"BackGround";
	BackGroundDesc.strShaderKey = L"Shader_VtxPosTex";
	BackGroundDesc.strTextureKey = L"TitleBack";
	BackGroundDesc.iLevel = 3;

	auto pBackGround = CUIImage::Create(m_pDevice, m_pContext);
	if (nullptr == pBackGround)
		return E_FAIL;
	pBackGround->Initialize(&BackGroundDesc);
	AddChildUI(pBackGround);

	return S_OK;
}

CGameplayCanvas* CGameplayCanvas::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CGameplayCanvas* pInstance = new CGameplayCanvas(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created CGameplayCanvas");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CGameplayCanvas::Clone(void* pArg)
{
	CGameplayCanvas* pInstance = new CGameplayCanvas(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created CGameplayCanvas");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CGameplayCanvas::Free()
{
	__super::Free();
	for (auto& pChild :  m_vecChildren)
	{
		Safe_Release(pChild);
	}
	m_vecChildren.clear();
}
