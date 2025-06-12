#include "UIProgressBar.h"
#include "Shader.h"
#include "GameInstance.h"
#include "UIImage.h"

CUIProgressBar::CUIProgressBar(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CUIObject{ pDevice, pContext }
{
	m_eUIType = UI_TYPE::BAR;
}

CUIProgressBar::CUIProgressBar(const CUIProgressBar& Prototype)
	: CUIObject(Prototype)
{

}

HRESULT CUIProgressBar::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUIProgressBar::Initialize(void* pArg)
{
	if (FAILED(CUIObject::Initialize(pArg)))
		return E_FAIL;
	BAR_DESC* pBarDesc = static_cast<BAR_DESC*>(pArg);
	if (pBarDesc->strTextureKey.empty())
		return S_OK;

	m_pBGImage = CUIImage::Create(m_pDevice, m_pContext);
	if (nullptr == m_pBGImage)
		return E_FAIL;
	if (FAILED(m_pBGImage->Initialize(pArg)))
		return E_FAIL;

	UIOBJECT_DESC desc{};
	if (pBarDesc->strFillTextureKey.empty() == false)
	{
		m_pFillImage = CUIImage::Create(m_pDevice, m_pContext);
		if (nullptr == m_pFillImage)
			return E_FAIL;
		UIOBJECT_DESC fillDesc = {};
		fillDesc.fX = pBarDesc->fX;
		fillDesc.fY = pBarDesc->fY;
		fillDesc.fSizeX = pBarDesc->fSizeX;
		fillDesc.fSizeY = pBarDesc->fSizeY;
		fillDesc.strShaderKey = pBarDesc->strBarShaderKey;
		fillDesc.strTextureKey = pBarDesc->strFillTextureKey;
		if (FAILED(m_pFillImage->Initialize(&fillDesc)))
			return E_FAIL;
	}

	if (pBarDesc->strDamageTextureKey.empty() == false)
	{
		m_pDamageImage = CUIImage::Create(m_pDevice, m_pContext);
		if (nullptr == m_pDamageImage)
			return E_FAIL;
		UIOBJECT_DESC damageDesc = {};
		damageDesc.fX = pBarDesc->fX;
		damageDesc.fY = pBarDesc->fY;
		damageDesc.fSizeX = pBarDesc->fSizeX;
		damageDesc.fSizeY = pBarDesc->fSizeY;
		damageDesc.strShaderKey = pBarDesc->strBarShaderKey;
		damageDesc.strTextureKey = pBarDesc->strDamageTextureKey;
		if (FAILED(m_pDamageImage->Initialize(&damageDesc)))
			return E_FAIL;
	}

	Safe_AddRef(m_pBGImage);
	Safe_AddRef(m_pFillImage);
	Safe_AddRef(m_pDamageImage);
	AddChild(m_pBGImage);
	AddChild(m_pFillImage);
	AddChild(m_pDamageImage);

	return S_OK;
}

void CUIProgressBar::Priority_Update(_float fTimeDelta)
{
	if (m_pBGImage)
		m_pBGImage->Priority_Update(fTimeDelta);
	if (m_pFillImage)
		m_pFillImage->Priority_Update(fTimeDelta);
	if (m_pDamageImage)
		m_pDamageImage->Priority_Update(fTimeDelta);

}

void CUIProgressBar::Update(_float fTimeDelta)
{
	if (m_pBGImage)
		m_pBGImage->Update(fTimeDelta);
	if (m_pFillImage)
		m_pFillImage->Update(fTimeDelta);
	if (m_pDamageImage)
		m_pDamageImage->Update(fTimeDelta);

	if (m_pGameInstance->IsKeyPressed(VK_TAB))
	{
		ApplyDamage(0.25f * m_fMaxValue); // 직접 값 변경 대신 ApplyDamage 사용
	}
	_float curRatio = m_fCurValue / m_fMaxValue;
	if (m_fPrevValue > curRatio)
	{
		m_fDelayTimer += fTimeDelta;
		if (m_fDelayTimer >= m_fDelayTime)
		{
			// 피해바 위치를 천천히 curRatio 쪽으로 당김
			m_fPrevValue = max(curRatio,
				m_fPrevValue - m_fRecoverSpeed * fTimeDelta);
			m_fDelayTimer = 0.f; // 딜레이 타이머 초기화
		}
	}
}

void CUIProgressBar::Late_Update(_float fTimeDelta)
{
	if (m_pBGImage)
		m_pBGImage->Late_Update(fTimeDelta);

	_float curRatio = m_fCurValue / m_fMaxValue; // 현재 HP 비율 (0~1)
	_float prevRatio = m_fPrevValue;
	//if (m_bReverse)
	//{
	//	curRatio = 1.f - curRatio;
	//	prevRatio = 1.f - prevRatio;
	//}

	// FillImage: 손실된 HP 부분을 지연 후 표시 (빨간색 부분)
	if (m_pFillImage)
	{
		m_pFillImage->SetRatio(prevRatio); // 이전 HP 비율을 셰이더에 전달
		m_pFillImage->Late_Update(fTimeDelta);
	}

	if (m_pDamageImage)
	{
		m_pDamageImage->SetRatio(curRatio); // 현재 HP 비율을 셰이더에 전달
		m_pDamageImage->Late_Update(fTimeDelta);
	}
}


HRESULT CUIProgressBar::Render()
{
	return S_OK;
}

void CUIProgressBar::SetBarImage(CUIImage* pBackgroundImage, CUIImage* pFillImage, CUIImage* pDamageImage)
{
	if (m_pBGImage)
	{
		Safe_Release(m_pBGImage);
		RemoveChild(m_pBGImage);
	}
	m_pBGImage = pBackgroundImage;
	if (m_pBGImage)
	{
		Safe_AddRef(m_pBGImage);
		AddChild(m_pBGImage);
	}
	if (m_pFillImage)
	{
		Safe_Release(m_pFillImage);
		RemoveChild(m_pFillImage);
	}
	m_pFillImage = pFillImage;
	if (m_pFillImage)
	{
		Safe_AddRef(pFillImage);
		AddChild(pFillImage);
	}
	if (m_pDamageImage)
	{
		Safe_Release(m_pDamageImage);
		RemoveChild(m_pDamageImage);
	}
	m_pDamageImage = pDamageImage;
	if (m_pDamageImage)
	{
		Safe_AddRef(m_pDamageImage);
		AddChild(m_pDamageImage);
	}

	if (m_pFillImage)
	{
		m_pFillImage->SetIsBarChild(true);
		m_pFillImage->SetShaderPass(1);
	}
	if (m_pDamageImage)
	{
		m_pDamageImage->SetIsBarChild(true);
		m_pDamageImage->SetShaderPass(1);
	}

}

json CUIProgressBar::Serialize()
{
	json j = __super::Serialize();
	if (m_pBGImage)
		j["BGImage"] = m_pBGImage->Serialize();
	if (m_pFillImage)
		j["FillImage"] = m_pFillImage->Serialize();
	if (m_pDamageImage)
		j["DamageImage"] = m_pDamageImage->Serialize();
	return j;
}

void CUIProgressBar::ApplyDamage(_float amount)
{
	_float oldRatio = m_fCurValue / m_fMaxValue;

	// 현재값 갱신
	m_fCurValue = max(0.f, m_fCurValue - amount);

	//// 피해바 시작 위치 & 타이머 리셋
	m_fPrevValue = oldRatio;
	m_fDelayTimer = 0.f;
}

CUIProgressBar* CUIProgressBar::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUIProgressBar* pInstance = new CUIProgressBar(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CUIProgressBar");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CUIProgressBar::Clone(void* pArg)
{
	CUIProgressBar* pInstance = new CUIProgressBar(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CUIProgressBar");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CUIProgressBar::Free()
{
	__super::Free();
	Safe_Release(m_pBGImage);
	Safe_Release(m_pFillImage);
	Safe_Release(m_pDamageImage);
}
