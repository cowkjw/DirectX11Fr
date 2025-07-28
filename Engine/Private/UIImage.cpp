#include "GameInstance.h"
#include "VIBuffer_Rect.h"
#include "UIImage.h"
#include "Texture.h"
#include "Shader.h"

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

	if (m_bIsFrameBased)
	{
		// 시간 누적
		m_fCurrentTime += fTimeDelta;

		// 프레임 전환 체크
		if (m_fCurrentTime >= m_fFrameTime)
		{
			m_fCurrentTime = 0.f;
			m_iCurrentFrame = (m_iCurrentFrame + 1) % m_iTotalFrames;

			// 현재 프레임의 행/열 계산
			_int row = m_iCurrentFrame / static_cast<_int>(m_fCols);
			_int col = m_iCurrentFrame % static_cast<_int>(m_fCols);

			// UV 오프셋 계산
			m_UVOffset[0] = col * m_UVScale[0];
			m_UVOffset[1] = row * m_UVScale[1];
		}
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

	if (m_bIsUVAnim|| m_bParentIsBar||m_bIsFrameBased|| m_bUseMask)
	{
		m_pShaderCom->Bind_RawValue("g_uvOffset", &m_UVOffset,sizeof(_float2));
		m_pShaderCom->Bind_RawValue("g_uvScale", &m_UVScale, sizeof(_float2));

		if (m_bParentIsBar)
		{
			m_pShaderCom->Bind_RawValue("g_HpRatio", &m_fRatio, sizeof(_float));
		}
	}

	m_pShaderCom->Bind_RawValue("g_Color", &m_vColor, sizeof(_float4));


	_uint use = m_bUseMask ? 1u : 0u;
	m_pShaderCom->Bind_RawValue("useMask", &use, sizeof(_uint));
	m_pShaderCom->Bind_RawValue("maskThreshold", &m_fMaskThreshold, sizeof(_float));
	m_pShaderCom->Bind_RawValue("maskUVOffset", &m_maskUVOffset, sizeof(_float2));
	m_pShaderCom->Bind_RawValue("maskUVScale", &m_maskUVScale, sizeof(_float2));

	// 마스크 텍스처 (t1) 바인딩
	if (m_bUseMask && m_iMaskTexIndex >= 0)
		m_pMaskTextureCom->Bind_ShaderResource(m_pShaderCom, "g_MaskTexture", m_iMaskTexIndex);


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
	if (m_bIsFrameBased)
	{

		j["FrameTime"] = m_fFrameTime;
	j["TotalFrames"] = m_iTotalFrames;
	j["Cols"] = m_fCols;
	j["Rows"] = m_fRows;
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

	if (j.contains("FrameTime"))
	{
		m_fFrameTime = j["FrameTime"].get<_float>();
	}
	if (j.contains("TotalFrames"))
	{
		m_iTotalFrames = j["TotalFrames"].get<_int>();
	}
	if (j.contains("Cols"))
	{
		m_fCols = j["Cols"].get<_float>();
	}
	if (j.contains("Rows"))
	{
		m_fRows = j["Rows"].get<_float>();
	}
}

void CUIImage::EnableUVAnim(_int cols, _int rows, _float frameSec)
{
	m_bIsFrameBased = true;
	m_fCols = static_cast<_float>(cols);
	m_fRows = static_cast<_float>(rows);
	m_iTotalFrames = cols * rows;
	m_fFrameTime = frameSec;

	// 한 프레임 크기
	m_UVScale[0] = 1.f / m_fCols;
	m_UVScale[1] = 1.f / m_fRows;

	// 초기화
	m_fCurrentTime = 0.f;
	m_iCurrentFrame = 0;
	m_UVOffset[0] = 0.f;
	m_UVOffset[1] = 0.f;
}

void CUIImage::EnableMask(const _wstring& maskKey)
{
	m_bUseMask = true;

	m_iShaderPass = 0;
	// Com_Texture 를 하나 더 추가해서 maskTexture 로 사용
	CGameObject::Add_Component(
		TEXT("Com_MaskTexture"),
		m_pGameInstance->GetTexture(maskKey, true),
		reinterpret_cast<CComponent**>(&m_pMaskTextureCom)
	);
	m_iMaskTexIndex = m_pMaskTextureCom->Get_NumTextures() > 0 ? 0 : -1;
}

void CUIImage::SetMaskParams(_float threshold, _float2 uvOffset, _float2 uvScale)
{
	m_fMaskThreshold = threshold;
	m_maskUVOffset = uvOffset;
	m_maskUVScale = uvScale;
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
		Safe_Release(m_pMaskTextureCom);
	}
}
