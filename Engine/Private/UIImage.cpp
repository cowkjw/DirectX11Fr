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
	if (m_bIsUVAnim)
	{
		// 1초당 m_UVSpeed 만큼 이동, 1.0 넘으면 wrap
		m_UVOffset[0] = fmodf(m_UVOffset[0] + m_UVSpeed[0] * fTimeDelta, 1.f);
		m_UVOffset[1] = fmodf(m_UVOffset[1] + m_UVSpeed[1] * fTimeDelta, 1.f);
		m_UVScale[0] = 1.f / m_fCols; // UV scale
		m_UVScale[1] = 1.f / m_fRows; // UV scale
	}
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

	if (m_bIsUVAnim|| m_bParentIsBar)
	{
		m_pShaderCom->Bind_RawValue("g_uvOffset", &m_UVOffset,sizeof(_float2));
		m_pShaderCom->Bind_RawValue("g_uvScale", &m_UVScale, sizeof(_float2));

		if (m_bParentIsBar)
		{
			m_pShaderCom->Bind_RawValue("g_HpRatio", &m_fRatio, sizeof(_float));
		}
	}

	m_pShaderCom->Bind_RawValue("g_Color", &m_vColor, sizeof(_float4));

	if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_Texture", m_iTextureIndex)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Begin(m_iShaderPass)))
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

	if (m_bIsUVAnim)
	{
		j["UVOffset"] = { m_UVOffset[0], m_UVOffset[1] };
		j["UVSpeed"] = { m_UVSpeed[0], m_UVSpeed[1] };
		j["UVScale"] = { m_UVScale[0], m_UVScale[1] };
	}
	return j;
}

void CUIImage::Deserialize(const json& j)
{
	CUIObject::Deserialize(j);
	if (j.contains("UVOffset"))
	{
		m_UVOffset[0] = j["UVOffset"][0].get<_float>();
		m_UVOffset[1] = j["UVOffset"][1].get<_float>();
		m_bIsUVAnim = true;
	}
	if (j.contains("UVSpeed"))
	{
		m_UVSpeed[0] = j["UVSpeed"][0].get<_float>();
		m_UVSpeed[1] = j["UVSpeed"][1].get<_float>();
	}
	if (j.contains("UVScale"))
	{
		m_UVScale[0] = j["UVScale"][0].get<_float>();
		m_UVScale[1] = j["UVScale"][1].get<_float>();
	}
}

void CUIImage::EnableUVAnim(_int cols, _int rows, _float frameSec)
{
	m_bIsUVAnim = true;
	m_fCols = static_cast<_float>(cols);
	m_fRows = static_cast<_float>(rows);

	// 한 프레임 크기
	m_UVScale[0] = 1.f / m_fCols;
	m_UVScale[1] = 1.f / m_fRows;

	// 한 프레임마다 UVOffset이 이동할 속도 = (프레임 크기) / (프레임 지속시간)
	m_UVSpeed[0] = m_UVScale[0] / frameSec;
	m_UVSpeed[1] = 0.f;  // 만약 세로 방향으로도 애니메이션이 필요하면 이쪽도 설정

	// 초기화
	m_UVOffset[0] = 0.f;
	m_UVOffset[1] = 0.f;
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
