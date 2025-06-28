#include "ParticleEffect.h"

#include "GameInstance.h"

CParticleEffect::CParticleEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect{ pDevice, pContext }
{

}

CParticleEffect::CParticleEffect(const CParticleEffect& Prototype)
	: CEffect (Prototype)
{

}

HRESULT CParticleEffect::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CParticleEffect::Initialize(void* pArg)
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

void CParticleEffect::Priority_Update(_float fTimeDelta)
{

}

void CParticleEffect::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
	for(const auto& Pair : m_ParticleSystems )
	{
		if (Pair.second&&Pair.second->IsActive())
		{
			Pair.second->UpdateVertexInstances(fTimeDelta);
		}
	}
}

void CParticleEffect::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::NONBLEND, this); 
}

HRESULT CParticleEffect::Render()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(TRANSFORM:: VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(TRANSFORM::PROJECTION))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_vCamPosition", m_pGameInstance->Get_CamPosition(), sizeof(_float4))))
		return E_FAIL;


	if (FAILED(m_Textures[TEX_DIFFUSE]->Bind_ShaderResource(m_pShaderCom, "g_Texture", 0)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Begin(0)))
		return E_FAIL;

	if (FAILED(m_ParticleSystems[L"Snow"]->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_ParticleSystems[L"Snow"]->Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CParticleEffect::Ready_Components()
{
	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(TEXT("Com_Shader"), m_pGameInstance->GetShader(TEXT("Shader_VtxRectInstance"), true), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* For.Com_Particle */
	if (FAILED(__super::Add_Component(ToIndex(LEVEL::STATIC), TEXT("Prototype_Component_Particle"),
		TEXT("Com_Particle"), reinterpret_cast<CComponent**>(&m_ParticleSystems[L"Snow"]))))
		return E_FAIL;
	/* For.Com_Texture */

	if (FAILED(__super::Add_Component(TEXT("Com_Texture"), m_pGameInstance->GetTexture(TEXT("TitleLogo"), true), reinterpret_cast<CComponent**>(&m_Textures[TEX_DIFFUSE]))))
		return E_FAIL;
	return S_OK;
}

CParticleEffect* CParticleEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CParticleEffect* pInstance = new CParticleEffect(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CMainApp");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CParticleEffect::Clone(void* pArg)
{
	CParticleEffect* pInstance = new CParticleEffect(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CParticleEffect");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CParticleEffect::Free()
{
	__super::Free();
}
