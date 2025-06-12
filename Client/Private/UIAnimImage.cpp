#include "UIAnimImage.h"
#include "GameInstance.h"

CUIAnimImage::CUIAnimImage(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CUIImage(pDevice, pContext)
	, m_fTimeElapsed(0.f)
	, m_iCurrentFrame(0)
	, m_iNumFrames(0)
	, m_fFrameDuration(0.1f) // 기본 프레임 지속 시간
{
}
CUIAnimImage::CUIAnimImage(const CUIAnimImage& rhs)
	: CUIImage(rhs)
	, m_fTimeElapsed(rhs.m_fTimeElapsed)
	, m_iCurrentFrame(rhs.m_iCurrentFrame)
	, m_iNumFrames(rhs.m_iNumFrames)
	, m_fFrameDuration(rhs.m_fFrameDuration)
{
}
HRESULT CUIAnimImage::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CUIAnimImage::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;
    return S_OK;
}

void CUIAnimImage::Update(_float fTimeDelta)
{
}

void CUIAnimImage::Late_Update(_float fTimeDelta)
{
}

HRESULT CUIAnimImage::Render()
{
    return E_NOTIMPL;
}

void CUIAnimImage::OnEnable()
{
}

void CUIAnimImage::OnDisable()
{
}

CUIAnimImage* CUIAnimImage::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    return nullptr;
}

CGameObject* CUIAnimImage::Clone(void* pArg)
{
    return nullptr;
}

void CUIAnimImage::Free()
{
}
