#include "UIImage.h"
#include "UIImage.h"
#include "GameInstance.h"
#include "Texture.h"
#include "Shader.h"
#include "VIBuffer_Rect.h"
CUIImage::CUIImage(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CUIObject{ pDevice, pContext }
{
	m_eUIType = UI_TYPE::IMAGE;
}

CUIImage::CUIImage(const CUIImage& Prototype)
	: CUIObject(Prototype)
{
}


HRESULT CUIImage::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CUIImage::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	UIOBJECT_DESC* pDesc = static_cast<UIOBJECT_DESC*>(pArg);
	m_strTextureKey = pDesc->strTextureKey;
	m_strShaderKey = pDesc->strShaderKey;

	if (m_strShaderKey.empty()|| m_strTextureKey.empty())
		return E_FAIL;
		

	if (FAILED(Ready_Components()))
		return E_FAIL;

	return S_OK;
}

void CUIImage::Priority_Update(_float fTimeDelta)
{
}

void CUIImage::Update(_float fTimeDelta)
{
}

void CUIImage::Late_Update(_float fTimeDelta)
{
	CGameObject::Late_Update(fTimeDelta);
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::UI, this);
}

HRESULT CUIImage::Render()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_Texture", m_iTextureIndex)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Begin(0)))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;

}

json CUIImage::Serialize()
{
	json j = CUIObject::Serialize();
	j["TextureKey"] = WStringToString(m_strTextureKey);
	j["ShaderKey"] = WStringToString(m_strShaderKey);
	j["NumTextures"] = m_iNumTextures;
	j["TextureIndex"] = m_iTextureIndex;
	return j;
}


HRESULT CUIImage::Ready_Components()
{
	if (FAILED(CGameObject::Add_Component(TEXT("Com_Texture"), m_pGameInstance->GetTexture(m_strTextureKey, true), reinterpret_cast<CComponent**>(&m_pTextureCom))))
		return E_FAIL;

	m_iNumTextures = m_pTextureCom->Get_NumTextures();

	if (FAILED(CGameObject::Add_Component(TEXT("Com_Shader"), m_pGameInstance->GetShader(m_strShaderKey, true), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;
	return S_OK;
}

CUIImage* CUIImage::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUIImage* pInstance = new CUIImage(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created CUIImage");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CUIImage::Clone(void* pArg)
{
	CUIImage* pInstance = new CUIImage(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created CUIImage");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CUIImage::Free()
{
	__super::Free();
	if (!m_bIsCloned)
	{
		Safe_Release(m_pShaderCom);
		Safe_Release(m_pTextureCom);
		Safe_Release(m_pVIBufferCom);
	}
}
