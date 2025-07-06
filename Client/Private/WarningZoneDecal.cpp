#include "WarningZoneDecal.h"
#include "GameInstance.h"

CWarningZoneDecal::CWarningZoneDecal(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CDecal(pDevice, pContext)
{
}

CWarningZoneDecal::CWarningZoneDecal(const CWarningZoneDecal& Prototype)
	: CDecal(Prototype)
{
}

HRESULT CWarningZoneDecal::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;
    return S_OK;
}

HRESULT CWarningZoneDecal::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;
	m_pTransformCom->Rotate_EulerAngles(_float3(90.f, 0.f, 0.f));
    return S_OK;
}

void CWarningZoneDecal::Update(_float fTimeDelta)
{
	CDecal::Update(fTimeDelta);
	m_fElapsedTime += fTimeDelta;
}

void CWarningZoneDecal::Late_Update(_float fTimeDelta)
{
	CDecal::Late_Update(fTimeDelta);

}

HRESULT CWarningZoneDecal::Render()
{
    if(FAILED(CDecal::Render()))
        return E_FAIL;
	if (FAILED(m_pShaderCom->Begin(m_iShaderPass)))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

    return S_OK;
}

HRESULT CWarningZoneDecal::Ready_Components()
{
	__super::Ready_Components();
	if (FAILED(CGameObject::Add_Component(TEXT("Com_Texture"), m_pGameInstance->GetTexture(TEXT("WarningDecal"), true), reinterpret_cast<CComponent**>(&m_pTextureCom))))
		return E_FAIL;


	if (FAILED(CGameObject::Add_Component(TEXT("Com_Shader"), m_pGameInstance->GetShader(TEXT("Shader_DecalMask"), true), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

    return S_OK;
}

HRESULT CWarningZoneDecal::Bind_Shaders()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(TRANSFORM::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(TRANSFORM::PROJECTION))))
		return E_FAIL;
	//if (FAILED(m_pShaderCom->Bind_RawValue("g_vCamPosition", m_pGameInstance->Get_CamPosition(), sizeof(_float4))))
	//	return E_FAIL;
	//const LIGHT_DESC* pLightDesc = m_pGameInstance->Get_Light(0);
	//if (FAILED(m_pShaderCom->Bind_RawValue("g_vLightDir", &pLightDesc->vDirection, sizeof(_float4))))
	//	return E_FAIL;
	//if (FAILED(m_pShaderCom->Bind_RawValue("g_vLightDiffuse", &pLightDesc->vDiffuse, sizeof(_float4))))
	//	return E_FAIL;
	//if (FAILED(m_pShaderCom->Bind_RawValue("g_vLightAmbient", &pLightDesc->vAmbient, sizeof(_float4))))
	//	return E_FAIL;
	//if (FAILED(m_pShaderCom->Bind_RawValue("g_vLightSpecular", &pLightDesc->vSpecular, sizeof(_float4))))
	//	return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_Time", &m_fElapsedTime, sizeof(_float))))
		return E_FAIL;
	if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_Texture", 0)))
		return E_FAIL;
	if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_NoiseTexture", 1)))
		return E_FAIL;
	if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_MaskTexture", 2)))
		return E_FAIL;

    return S_OK;
}

CWarningZoneDecal* CWarningZoneDecal::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CWarningZoneDecal* pInstance = new CWarningZoneDecal(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CWarningZoneDecal");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CWarningZoneDecal::Clone(void* pArg)
{
	CWarningZoneDecal* pInstance = new CWarningZoneDecal(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CWarningZoneDecal");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CWarningZoneDecal::Free()
{
	__super::Free();
}
