#include "UIButton.h"
#include "UIImage.h"
#include "GameInstance.h"

CUIButton::CUIButton(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CUIObject{ pDevice, pContext }
{
}


CUIButton::CUIButton(const CUIButton& Prototype)
	: CUIObject(Prototype)
{
}

HRESULT CUIButton::Initialize_Prototype()
{
    return E_NOTIMPL;
}

HRESULT CUIButton::Initialize(void* pArg)
{
	m_pButtonImage = CUIImage::Create(m_pDevice, m_pContext);
	if (nullptr == m_pButtonImage)
		return E_FAIL;
	if(FAILED(m_pButtonImage->Initialize(pArg)))
		return E_FAIL;
    return S_OK;
}

void CUIButton::Priority_Update(_float fTimeDelta)
{
}

void CUIButton::Update(_float fTimeDelta)
{
	if (m_bHovered)
	{
		if (m_pGameInstance->IsKeyDown(VK_LBUTTON))
		{
			m_bPressed = true;
			m_OnClick();
		}
	}
}

void CUIButton::Late_Update(_float fTimeDelta)
{

}

HRESULT CUIButton::Render()
{
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
    return nullptr;
}

CGameObject* CUIButton::Clone(void* pArg)
{
    return nullptr;
}

void CUIButton::Free()
{
	__super::Free();
	Safe_Release(m_pButtonImage);
}
