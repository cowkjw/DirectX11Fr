#include "TitleCanvas.h"
#include "BackGround.h"
#include <UIButton.h>

CTitleCanvas::CTitleCanvas(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CUICanvas(pDevice, pContext)
{
}


CTitleCanvas::CTitleCanvas(const CTitleCanvas& Prototype)
	: CUICanvas(Prototype)
{
}

HRESULT CTitleCanvas::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CTitleCanvas::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_strName = L"TitleCanvas";
	if (FAILED(Ready_ChildUI()))
		return E_FAIL;

	return S_OK;
}

HRESULT CTitleCanvas::Ready_ChildUI()
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

	BackGroundDesc.fX = g_iWinSizeX * 0.5f;
	BackGroundDesc.fY = g_iWinSizeY * 0.5f;
	BackGroundDesc.fSizeX = g_iWinSizeX*0.5f;
	BackGroundDesc.fSizeY = g_iWinSizeY*0.5f;
	BackGroundDesc.strName = L"TitleEffect";
	BackGroundDesc.strShaderKey = L"Shader_VtxPosTex";
	BackGroundDesc.strTextureKey = L"TitleEffect";
	BackGroundDesc.iLevel = 3;

	pBackGround = CUIImage::Create(m_pDevice, m_pContext);
	if (nullptr == pBackGround)
		return E_FAIL;
	pBackGround->Initialize(&BackGroundDesc);
	AddChildUI(pBackGround);

	BackGroundDesc.fX = g_iWinSizeX * 0.8f;
	BackGroundDesc.fY = g_iWinSizeY * 0.3f;
	BackGroundDesc.fSizeX = g_iWinSizeX * 0.3f;
	BackGroundDesc.fSizeY = g_iWinSizeY * 0.3f;
	BackGroundDesc.strName = L"StartButton";
	BackGroundDesc.strShaderKey = L"Shader_VtxPosTex";
	BackGroundDesc.strTextureKey = L"TitleLogo";
	BackGroundDesc.iLevel = 3;

	auto pStartButton = CUIButton::Create(m_pDevice, m_pContext);
	if (nullptr == pStartButton)
		return E_FAIL;
	pStartButton->Initialize(&BackGroundDesc);
	AddChildUI(pStartButton);
	return S_OK;
}

CTitleCanvas* CTitleCanvas::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CTitleCanvas* pInstance = new CTitleCanvas(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created CTitleCanvas");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CTitleCanvas::Clone(void* pArg)
{
	CTitleCanvas* pInstance = new CTitleCanvas(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created CTitleCanvas");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CTitleCanvas::Free()
{
	__super::Free();
	for (auto& pChild :  m_vecChildren)
	{
		Safe_Release(pChild);
	}
	m_vecChildren.clear();
}
