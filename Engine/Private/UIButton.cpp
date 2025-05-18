#include "UIButton.h"
#include "UIImage.h"
#include "GameInstance.h"

CUIButton::CUIButton(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CUIObject{ pDevice, pContext }
{
	m_eUIType = UI_TYPE::BUTTON;
}


CUIButton::CUIButton(const CUIButton& Prototype)
	: CUIObject(Prototype)
{
}

HRESULT CUIButton::Initialize_Prototype()
{

	return S_OK;
}

HRESULT CUIButton::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_pButtonImage = CUIImage::Create(m_pDevice, m_pContext);
	if (nullptr == m_pButtonImage)
		return E_FAIL;
	if (FAILED(m_pButtonImage->Initialize(pArg))) // ½¦ÀÌ´õ¶û ÅØ½ºÃÄ Å° ÇÒ´ç 
		return E_FAIL;
	Safe_AddRef(m_pButtonImage);
	AddChild(m_pButtonImage);
	return S_OK;
}

void CUIButton::Priority_Update(_float fTimeDelta)
{
	CheckMouseOver();
	if (m_pButtonImage)
		m_pButtonImage->Priority_Update(fTimeDelta);
}

void CUIButton::Update(_float fTimeDelta)
{
	if (m_pButtonImage)
		m_pButtonImage->Update(fTimeDelta);
}

void CUIButton::Late_Update(_float fTimeDelta)
{
	CGameObject::Late_Update(fTimeDelta);
	if (m_pButtonImage)
		m_pButtonImage->Late_Update(fTimeDelta);


	if (m_bHovered)
	{
		if (m_pGameInstance->IsMousePressed(0))
		{
			m_bPressed = true;
			if (m_OnClick)
				m_OnClick();
		}
		else
		{
			m_bPressed = false;
		}
	}
}

HRESULT CUIButton::Render()
{
	if (m_pButtonImage)
		m_pButtonImage->Render();
	return S_OK;
}

void CUIButton::CheckMouseOver()
{
	POINT pt = m_pGameInstance->GetMousePos();

	_float left = m_fX - m_fSizeX * 0.5f;
	_float right = m_fX + m_fSizeX * 0.5f;
	_float top = m_fY - m_fSizeY * 0.5f;
	_float bottom = m_fY + m_fSizeY * 0.5f;

	m_bHovered = (pt.x >= left && pt.x <= right && pt.y >= top && pt.y <= bottom);
}

CUIButton* CUIButton::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUIButton* pInstance = new CUIButton(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CUIButton");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CUIButton::Clone(void* pArg)
{
	CUIButton* pInstance = new CUIButton(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created : CUIButton");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CUIButton::Free()
{
	__super::Free();
	Safe_Release(m_pButtonImage);
	Safe_Release(m_pButtonImage);
}
