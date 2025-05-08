#include "UIProgressBar.h"

CUIProgressBar::CUIProgressBar(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CUIObject{ pDevice, pContext }
{
}

CUIProgressBar::CUIProgressBar(const CUIProgressBar& Prototype)
	: CUIObject(Prototype)
{
}

HRESULT CUIProgressBar::Initialize_Prototype()
{
    return E_NOTIMPL;
}

HRESULT CUIProgressBar::Initialize(void* pArg)
{
    return E_NOTIMPL;
}

void CUIProgressBar::Priority_Update(_float fTimeDelta)
{
}

void CUIProgressBar::Update(_float fTimeDelta)
{
}

void CUIProgressBar::Late_Update(_float fTimeDelta)
{
}

HRESULT CUIProgressBar::Render()
{
    return E_NOTIMPL;
}

CUIProgressBar* CUIProgressBar::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    return nullptr;
}

CGameObject* CUIProgressBar::Clone(void* pArg)
{
    return nullptr;
}

void CUIProgressBar::Free()
{
}
