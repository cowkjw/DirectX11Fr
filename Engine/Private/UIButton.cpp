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
	, m_bHovered(Prototype.m_bHovered)
	
{
}

HRESULT CUIButton::Initialize_Prototype()
{
	m_bHovered = true;
	return S_OK;
}

HRESULT CUIButton::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	UIOBJECT_DESC* pUIObjectDesc = static_cast<UIOBJECT_DESC*>(pArg);

	// 이건 툴 상에서 만드는게 아니라서 넘기기 자식들 연결 시킬거임
	if (pUIObjectDesc->strTextureKey.empty())
		return S_OK;

	m_pButtonImage = CUIImage::Create(m_pDevice, m_pContext);
	if (nullptr == m_pButtonImage)
		return E_FAIL;
	if (FAILED(m_pButtonImage->Initialize(pArg))) // 쉐이더랑 텍스쳐 키 할당 
		return E_FAIL;
	UIButtonDesc* pUIButtonDesc = static_cast<UIButtonDesc*>(pArg);
	if (pUIButtonDesc->strButtonImageHoverKey.empty() == false)
	{
		m_pButtonImageHover = CUIImage::Create(m_pDevice, m_pContext);
		if (nullptr == m_pButtonImageHover)
			return E_FAIL;
		UIOBJECT_DESC desc = {};
		desc.fX = pUIButtonDesc->fX;
		desc.fY = pUIButtonDesc->fY;
		desc.fSizeX = pUIButtonDesc->fSizeX;
		desc.fSizeY = pUIButtonDesc->fSizeY;
		desc.strShaderKey = pUIButtonDesc->strShaderKey;
		desc.strTextureKey = pUIButtonDesc->strButtonImageHoverKey;
		if (FAILED(m_pButtonImageHover->Initialize(&desc))) // 쉐이더랑 텍스쳐 키 할당 
			return E_FAIL;
	}

	Safe_AddRef(m_pButtonImage);
	AddChild(m_pButtonImage);
	if (m_pButtonImageHover)
	{
		Safe_AddRef(m_pButtonImageHover);
		AddChild(m_pButtonImageHover);
	}

	return S_OK;
}

void CUIButton::Priority_Update(_float fTimeDelta)
{
	CheckMouseOver();
	if (m_pButtonImage)
		m_pButtonImage->Priority_Update(fTimeDelta);
	if (m_pButtonImageHover && m_bHovered)
	{
		m_pButtonImageHover->Priority_Update(fTimeDelta);
	}
}

void CUIButton::Update(_float fTimeDelta)
{
	if (m_pButtonImage)
		m_pButtonImage->Update(fTimeDelta);
	if (m_pButtonImageHover && m_bHovered)
	{
		m_pButtonImageHover->Update(fTimeDelta);
	}
}

void CUIButton::Late_Update(_float fTimeDelta)
{
	CGameObject::Late_Update(fTimeDelta);
	if (m_pButtonImage&&m_pButtonImage->IsActive())
		m_pButtonImage->Late_Update(fTimeDelta);

	if (m_pButtonImageHover && m_bHovered)
	{
		m_pButtonImageHover->Late_Update(fTimeDelta);
	}



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
	if (m_pButtonImageHover && m_bHovered)
		m_pButtonImageHover->Render();
	return S_OK;
}

json CUIButton::Serialize()
{
	json j = __super::Serialize();
	if (m_pButtonImage)
		j["ButtonImage"] = m_pButtonImage->Serialize();
	if (m_pButtonImageHover)
		j["HoverImage"] = m_pButtonImageHover->Serialize();
	return j;
}


void CUIButton::SetButtonImage(CUIImage* pButtonImage, CUIImage* pButtonImageHover)
{
	if (m_pButtonImage)
	{
		Safe_Release(m_pButtonImage);
		RemoveChild(m_pButtonImage);
	}
	m_pButtonImage = pButtonImage;
	if (m_pButtonImage)
	{
		Safe_AddRef(m_pButtonImage);
		AddChild(m_pButtonImage);
	}
	if (m_pButtonImageHover)
	{
		Safe_Release(m_pButtonImageHover);
		RemoveChild(m_pButtonImageHover);
	}
	m_pButtonImageHover = pButtonImageHover;
	if (m_pButtonImageHover)
	{
		Safe_AddRef(m_pButtonImageHover);
		AddChild(m_pButtonImageHover);
	}
}

void CUIButton::CheckMouseOver()
{
	POINT pt = m_pGameInstance->GetMousePos();

	_float left{}, right{}, top{}, bottom{};

	_vector pos = m_pTransformCom->Get_State(STATE::POSITION);
	_float fX = XMVectorGetX(pos);
	_float fY = XMVectorGetY(pos);
	_float screenX = m_fWidth * 0.5f + fX;
	_float screenY = m_fHeight * 0.5f - fY;

	_float3 vSize = m_pTransformCom->Get_Scaled();
	left = screenX - vSize.x * 0.5f;
	right = screenX + vSize.x * 0.5f;
	top = screenY - vSize.y * 0.5f;
	bottom = screenY + vSize.y * 0.5f;


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
	Safe_Release(m_pButtonImageHover);
	Safe_Release(m_pButtonImageHover);

}
