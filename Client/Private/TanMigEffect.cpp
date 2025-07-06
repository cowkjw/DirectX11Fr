#include "TanMigEffect.h"
#include "GameInstance.h"
#include <JsonLoader.h>
#include "ParticleSystem.h"
#include "HitParticle.h"

CTanMigEffect::CTanMigEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMeshEffect(pDevice, pContext)
{
}
CTanMigEffect::CTanMigEffect(const CTanMigEffect& Prototype)
	: CMeshEffect(Prototype),
	m_vColor(Prototype.m_vColor),
	m_fDir(Prototype.m_fDir)
	, m_fRotationSpeed(Prototype.m_fRotationSpeed)
	, m_fDuration(Prototype.m_fDuration)
{
}

HRESULT CTanMigEffect::Initialize_Prototype()
{
	if (FAILED(Ready_Components()))
		return E_FAIL;
	m_vUVOffset = _float2(0.f, 0.f); // UV 오프셋 초기화
	m_vUVScale = _float2(1.f, 1.f); // UV 스케일 설정
	m_iShaderPass = 8;
	m_fDuration = 1.f;

	return S_OK;
}

HRESULT CTanMigEffect::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_pTransformCom->Scaling(_float3(15.f,15.f,15.f));
	m_pTransformCom->Set_SpeedPerSec(30.f);

	return S_OK;
}

void CTanMigEffect::Priority_Update(_float fTimeDelta)
{
	
	for (auto& particle : m_ParticleEffects)
	{
		if (particle.second && particle.second->IsActive())
		{
			particle.second->Priority_Update(fTimeDelta);
		}
	}
}

void CTanMigEffect::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);

	m_fElapsed += fTimeDelta;
	m_vUVOffset.x+= fTimeDelta * 1.f;
	m_vUVOffset.y+= fTimeDelta * 1.f;
	m_fScale += fTimeDelta * m_fRotationSpeed;
	m_pTransformCom->Scaling(_float3(m_fScale, m_fScale, m_fScale));
	if (m_fScale >= m_fDestScale)
	{
		m_fScale = m_fDestScale;
	}
	if (m_fElapsed >= m_fDuration)
	{
		m_fElapsed = 0.f;
		SetActive(false);
	}
	for (auto& particle : m_ParticleEffects)
	{
		if (particle.second && particle.second->IsActive())
		{
			particle.second->Update(fTimeDelta);
		}
	}
	m_pTransformCom->MoveDirection(m_pTransformCom->Get_State(STATE::LOOK), fTimeDelta);
	_float4x4 matWorld = m_pTransformCom->Get_WorldMatrix();
	m_CombinedWorldMatrix = matWorld;
}

void CTanMigEffect::Late_Update(_float fTimeDelta)
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

HRESULT CTanMigEffect::Render()
{

	// 1번에 디퓨즈
	if (FAILED(__super::Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CTanMigEffect::Ready_Components()
{
	if (FAILED(__super::Add_Component(TEXT("Com_Shader"), m_pGameInstance->GetShader(TEXT("Shader_VtxMesh"), true), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;


	/* For.Com_Model */
	if (FAILED(__super::Add_Component(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Component_Model_MigRing"),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
	{
		return E_FAIL;
	}
	return S_OK;
}

HRESULT CTanMigEffect::Bind_Shader()
{
	__super::Bind_Shader();
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vUVOffset", &m_vUVOffset, sizeof(_float2))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vUVScale", &m_vUVScale, sizeof(_float2))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vColor", &m_vColor, sizeof(_float4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fTime", &m_fElapsed, sizeof(_float))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fDuration", &m_fDuration, sizeof(_float))))
		return E_FAIL;
	return S_OK;
}

void CTanMigEffect::OnDisable()
{
	m_vUVScale = _float2(1.f, 1.f); 
	m_vUVOffset = _float2(0.f, 0.f); 
	m_fElapsed = 0.f; 
	m_pTransformCom->Scaling(m_fScale, m_fScale, m_fScale);
}

void CTanMigEffect::OnEnable()
{
	m_vUVScale = _float2(1.f,1.f); 
	m_vUVOffset = _float2(0.f, 0.f); 
	m_pTransformCom->Scaling(m_fScale, m_fScale, m_fScale);
	m_fElapsed = 0.f; 
}

CTanMigEffect* CTanMigEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CTanMigEffect* pInstance = new CTanMigEffect(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CTanMigEffect");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CTanMigEffect::Clone(void* pArg)
{
	CTanMigEffect* pClone = new CTanMigEffect(*this);
	if (FAILED(pClone->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CTanMigEffect");
		Safe_Release(pClone);
	}
	return pClone;
}

void CTanMigEffect::Free()
{
	__super::Free();
}
