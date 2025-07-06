#include "TanMigRingEffect.h"
#include "GameInstance.h"
#include <JsonLoader.h>
#include "ParticleSystem.h"
#include "HitParticle.h"

CTanMigRingEffect::CTanMigRingEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMeshEffect(pDevice, pContext)
{
}
CTanMigRingEffect::CTanMigRingEffect(const CTanMigRingEffect& Prototype)
	: CMeshEffect(Prototype),
	m_vColor(Prototype.m_vColor),
	m_fDir(Prototype.m_fDir)
	,m_fRotationSpeed(Prototype.m_fRotationSpeed)
{
}

HRESULT CTanMigRingEffect::Initialize_Prototype()
{
	if (FAILED(Ready_Components()))
		return E_FAIL;
	m_bUseOffset = true;
	m_bRenderMesh = true;
	m_vColor = _float4(1.f, 1.f, 1.f, 1.f); // 초기 색상 설정
	m_vUVOffset = _float2(0.f, 0.f); // UV 오프셋 초기화
	m_iShaderPass = 7;
	return S_OK;
}

HRESULT CTanMigRingEffect::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_pTransformCom->Scaling(_float3(15.f, 15.f, 15.f));
	m_pTransformCom->Set_SpeedPerSec(30.f);

	return S_OK;
}

void CTanMigRingEffect::Priority_Update(_float fTimeDelta)
{
	
	for (auto& particle : m_ParticleEffects)
	{
		if (particle.second && particle.second->IsActive())
		{
			particle.second->Priority_Update(fTimeDelta);
		}
	}
}

void CTanMigRingEffect::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
		m_vUVOffset.x -= fTimeDelta * 1.5f;
		if (m_vUVOffset.x <=0.f)
		{ 
			m_vUVOffset.x = 1.f; 
		}
	m_fScale += fTimeDelta * m_fRotationSpeed;
	m_pTransformCom->Scaling(_float3(m_fScale, m_fScale, m_fScale));
	if (m_fScale >= m_fDestScale)
	{
		m_fScale = m_fDestScale;
	}
	m_pTransformCom->MoveDirection(m_pTransformCom->Get_State(STATE::LOOK), fTimeDelta);
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

void CTanMigRingEffect::Late_Update(_float fTimeDelta)
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

HRESULT CTanMigRingEffect::Render()
{

	if (FAILED(__super::Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CTanMigRingEffect::Ready_Components()
{
	if (FAILED(__super::Add_Component(TEXT("Com_Shader"), m_pGameInstance->GetShader(TEXT("Shader_VtxMesh"), true), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;


	/* For.Com_Model */
	if (FAILED(__super::Add_Component(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Component_Model_MigCrossRing"),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
	{
		return E_FAIL;
	}
	return S_OK;
}

HRESULT CTanMigRingEffect::Bind_Shader()
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

void CTanMigRingEffect::OnDisable()
{
	m_vUVOffset = _float2(1.f, 0.f); 
	m_pTransformCom->Scaling(m_fScale, m_fScale, m_fScale);
}

void CTanMigRingEffect::OnEnable()
{
	m_vUVOffset = _float2(1.f, 0.f);
	m_pTransformCom->Scaling(m_fScale, m_fScale, m_fScale);
}

CTanMigRingEffect* CTanMigRingEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CTanMigRingEffect* pInstance = new CTanMigRingEffect(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CTanMigRingEffect");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CTanMigRingEffect::Clone(void* pArg)
{
	CTanMigRingEffect* pClone = new CTanMigRingEffect(*this);
	if (FAILED(pClone->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CTanMigRingEffect");
		Safe_Release(pClone);
	}
	return pClone;
}

void CTanMigRingEffect::Free()
{
	__super::Free();
}
