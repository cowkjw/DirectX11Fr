#include "KyojuroEnkEffect.h"
#include "GameInstance.h"
#include <JsonLoader.h>
#include "ParticleSystem.h"
#include "HitParticle.h"
#include <BaseCharacter.h>

CKyojuroEnkEffect::CKyojuroEnkEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMeshEffect(pDevice, pContext)
{
}
CKyojuroEnkEffect::CKyojuroEnkEffect(const CKyojuroEnkEffect& Prototype)
	: CMeshEffect(Prototype),
	m_vColor(Prototype.m_vColor)
	, m_fRotationSpeed(Prototype.m_fRotationSpeed)
{
}

HRESULT CKyojuroEnkEffect::Initialize_Prototype()
{
	if (FAILED(Ready_Components()))
		return E_FAIL;
	return S_OK;
}

HRESULT CKyojuroEnkEffect::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;
	m_iShaderPass = 4; 
	//m_pTransformCom->Scaling(_float3(10.3f, 10.2f, 30.f));
	m_pTransformCom->Scaling(_float3(0.2f, 0.2f, 0.2f));

	m_bUseOffset = true;
	m_bRenderMesh = true;
//	m_vUVScale = { 1.f,2.5f };
	m_vUVScale = { 1.f,5.f };
	m_fRotationSpeed = 3.5f;


	
	m_pTransformCom->Set_SpeedPerSec(35.f);
	return S_OK;
}

void CKyojuroEnkEffect::Priority_Update(_float fTimeDelta)
{

	for (auto& particle : m_ParticleEffects)
	{
		if (particle.second && particle.second->IsActive())
		{
			particle.second->Priority_Update(fTimeDelta);
		}
	}
}

void CKyojuroEnkEffect::Update(_float fTimeDelta)
{
	if (m_pParent == nullptr)
	{
		SetParent(m_pGameInstance->Find_GameObjectByName(ToIndex(LEVEL::GAMEPLAY), TEXT("Kyojuro")));
	}
	__super::Update(fTimeDelta);

	if (m_bUseOffset)
	{
		m_vUVOffset.y += fTimeDelta*0.7f;
		if (m_vUVOffset.y >= 1.f)
		{
			m_vUVOffset.y = 0.f;
		}
	}
	for (auto& particle : m_ParticleEffects)
	{
		if (particle.second && particle.second->IsActive())
		{
			particle.second->Update(fTimeDelta);
		}
	}

	_float4x4 matWorld = m_pTransformCom->Get_WorldMatrix();
	m_CombinedWorldMatrix = matWorld;
}

void CKyojuroEnkEffect::Late_Update(_float fTimeDelta)
{
	if (m_bRenderMesh)
	{
		__super::Late_Update(fTimeDelta);
	}
	for (auto& particle : m_ParticleEffects)
	{
		if (particle.second && particle.second->IsActive())
		{
			particle.second->Late_Update(fTimeDelta);
		}
	}
}

HRESULT CKyojuroEnkEffect::Render()
{

	if (FAILED(__super::Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CKyojuroEnkEffect::Ready_Components()
{
	if (FAILED(__super::Add_Component(TEXT("Com_Shader"), m_pGameInstance->GetShader(TEXT("Shader_VtxMesh"), true), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* For.Com_Model */
	if (FAILED(__super::Add_Component(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_EnkFire"),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
	{
		return E_FAIL;
	}
	return S_OK;
}

HRESULT CKyojuroEnkEffect::Bind_Shader()
{
	__super::Bind_Shader();
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vUVOffset", &m_vUVOffset, sizeof(_float2))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vUVScale", &m_vUVScale, sizeof(_float2))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vColor", &m_vColor, sizeof(_float4))))
		return E_FAIL;
	return S_OK;
}




CKyojuroEnkEffect* CKyojuroEnkEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CKyojuroEnkEffect* pInstance = new CKyojuroEnkEffect(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CKyojuroEnkEffect");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CKyojuroEnkEffect::Clone(void* pArg)
{
	CKyojuroEnkEffect* pClone = new CKyojuroEnkEffect(*this);
	if (FAILED(pClone->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CKyojuroEnkEffect");
		Safe_Release(pClone);
	}
	return pClone;
}

void CKyojuroEnkEffect::Free()
{
	__super::Free();
}
