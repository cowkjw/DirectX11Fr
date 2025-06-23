#include "Snow.h"

#include "GameInstance.h"

CSnow::CSnow(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject { pDevice, pContext }
{

}

CSnow::CSnow(const CSnow& Prototype)
	: CGameObject { Prototype }
{

}

HRESULT CSnow::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CSnow::Initialize(void* pArg)
{
	GAMEOBJECT_DESC			Desc{};
	Desc.fRotationPerSec = XMConvertToRadians(90.f);
	Desc.fSpeedPerSec = 0.f;
	Desc.strName = TEXT("Snow");
	if (FAILED(__super::Initialize(&Desc)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	return S_OK;
}

void CSnow::Priority_Update(_float fTimeDelta)
{

}

void CSnow::Update(_float fTimeDelta)
{
	m_pParticleCom->UpdateVertexInstances(fTimeDelta);
}

void CSnow::Late_Update(_float fTimeDelta)
{
	/* WeightBlend */	
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::NONBLEND, this); 
}

HRESULT CSnow::Render()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(TRANSFORM:: VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(TRANSFORM::PROJECTION))))
		return E_FAIL;


	if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_Texture", 0)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Begin(0)))
		return E_FAIL;

	if (FAILED(m_pParticleCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pParticleCom->Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CSnow::Ready_Components()
{
	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(TEXT("Com_Shader"), m_pGameInstance->GetShader(TEXT("Shader_VtxRectInstance"), true), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* For.Com_Particle */
	if (FAILED(__super::Add_Component(ToIndex(LEVEL::STATIC), TEXT("Prototype_Component_Particle"),
		TEXT("Com_Particle"), reinterpret_cast<CComponent**>(&m_pParticleCom))))
		return E_FAIL;
	/* For.Com_Texture */

	if (FAILED(__super::Add_Component(TEXT("Com_Texture"), m_pGameInstance->GetTexture(TEXT("TitleLogo"), true), reinterpret_cast<CComponent**>(&m_pTextureCom))))
		return E_FAIL;
	return S_OK;
}

CSnow* CSnow::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSnow* pInstance = new CSnow(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CMainApp");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CSnow::Clone(void* pArg)
{
	CSnow* pInstance = new CSnow(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CSnow");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CSnow::Free()
{
	__super::Free();

	Safe_Release(m_pParticleCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTextureCom);
}
