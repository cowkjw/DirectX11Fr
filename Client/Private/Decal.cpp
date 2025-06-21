#include "Decal.h"
#include "GameInstance.h"
CDecal::CDecal(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject(pDevice, pContext)
	, m_iShaderPass(0)
	, m_fLifeTime(0.f)
	, m_fElapsedTime(0.f)
{
}

CDecal::CDecal(const CDecal& Prototype)
	: CGameObject(Prototype)
	, m_pTextureCom(Prototype.m_pTextureCom)
	, m_iShaderPass(Prototype.m_iShaderPass)
	, m_fLifeTime(Prototype.m_fLifeTime)
	, m_fElapsedTime(Prototype.m_fElapsedTime)
	, m_pShaderCom(Prototype.m_pShaderCom)
	, m_pVIBufferCom(Prototype.m_pVIBufferCom)
{
	//Safe_AddRef(m_pShaderCom);
	//Safe_AddRef(m_pVIBufferCom);
}

HRESULT CDecal::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CDecal::Initialize(void* pArg)
{
	CGameObject::GAMEOBJECT_DESC Desc{};
	Desc.fRotationPerSec = XMConvertToRadians(90.f);
	Desc.fSpeedPerSec = 0.f;
	Desc.strName = TEXT("Decal");
	if (FAILED(__super::Initialize(&Desc)))
		return E_FAIL;
	if (FAILED(Ready_Components()))
		return E_FAIL;
	return S_OK;
}

void CDecal::Update(_float fTimeDelta)
{
}

void CDecal::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::NONBLEND, this);
}

HRESULT CDecal::Render()
{
	if (FAILED(Bind_Shaders()))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Begin(m_iShaderPass)))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;
	return S_OK;
}

HRESULT CDecal::Ready_Components()
{
	m_pVIBufferCom = CVIBuffer_Rect::Create(m_pDevice, m_pContext);
	if (nullptr == m_pVIBufferCom)
		return E_FAIL;
	m_Components.emplace(L"Com_VIBuffer", m_pVIBufferCom);

    return S_OK;
}

HRESULT CDecal::Bind_Shaders()
{
	return S_OK;
}

CDecal* CDecal::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CDecal* pInstance = new CDecal(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created CDecal");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CDecal::Clone(void* pArg)
{
	CDecal* pInstance = new CDecal(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned CDecal");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CDecal::Free()
{
	__super::Free();
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pVIBufferCom);
	Safe_Release(m_pTextureCom);
}
