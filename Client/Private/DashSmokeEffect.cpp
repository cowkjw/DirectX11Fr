#include "DashSmokeEffect.h"
#include "GameInstance.h"
#include <JsonLoader.h>
#include "ParticleSystem.h"
#include "HitParticle.h"

CDashSmokeEffect::CDashSmokeEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMeshEffect(pDevice, pContext)
{
}
CDashSmokeEffect::CDashSmokeEffect(const CDashSmokeEffect& Prototype)
	: CMeshEffect(Prototype),
	m_vColor(Prototype.m_vColor),
	m_fDir(Prototype.m_fDir)
	,m_fRotationSpeed(Prototype.m_fRotationSpeed)
{
}

HRESULT CDashSmokeEffect::Initialize_Prototype()
{
	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_vColor = _float4(0.705f, 0.317f, 0.168f, 1.f);
	m_vUVOffset = _float2(0.f, 0.f); // UV 오프셋 초기화
	m_vUVScale = _float2(1.f, 5.f); // UV 스케일 설정
	m_iShaderPass = 9;
	m_fDuration = 0.7f;
	return S_OK;
}

HRESULT CDashSmokeEffect::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_pTransformCom->Scaling(_float3(7.f,14.f,20.f));
	//m_pTransformCom->Scaling(_float3(7.f,10.f,20.f));
	m_pTransformCom->Set_SpeedPerSec(30.f);

	return S_OK;
}

void CDashSmokeEffect::Priority_Update(_float fTimeDelta)
{
	
	for (auto& particle : m_ParticleEffects)
	{
		if (particle.second && particle.second->IsActive())
		{
			particle.second->Priority_Update(fTimeDelta);
		}
	}
}

void CDashSmokeEffect::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
		m_fElapsed += fTimeDelta;
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

	_float4x4 matWorld = m_pTransformCom->Get_WorldMatrix();
	m_CombinedWorldMatrix = matWorld;
}

void CDashSmokeEffect::Late_Update(_float fTimeDelta)
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

HRESULT CDashSmokeEffect::Render()
{
	// 디퓨즈에 라인
	// 스페큘러에 그라디언트
	// 노말 스모크 텍스쳐
	if (FAILED(__super::Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CDashSmokeEffect::Ready_Components()
{
	if (FAILED(__super::Add_Component(TEXT("Com_Shader"), m_pGameInstance->GetShader(TEXT("Shader_VtxMesh"), true), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;


	/* For.Com_Model */
	if (FAILED(__super::Add_Component(ToIndex(LEVEL::STATIC), TEXT("Prototype_Component_Model_Dash"),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
	{
		return E_FAIL;
	}
	return S_OK;
}

HRESULT CDashSmokeEffect::Bind_Shader()
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

void CDashSmokeEffect::OnDisable()
{
	m_vUVOffset = _float2(0.f, 0.f); 
}

void CDashSmokeEffect::OnEnable()
{
	m_vUVOffset = _float2(0.f, 0.f);
}

CDashSmokeEffect* CDashSmokeEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CDashSmokeEffect* pInstance = new CDashSmokeEffect(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CDashSmokeEffect");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CDashSmokeEffect::Clone(void* pArg)
{
	CDashSmokeEffect* pClone = new CDashSmokeEffect(*this);
	if (FAILED(pClone->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CDashSmokeEffect");
		Safe_Release(pClone);
	}
	return pClone;
}

void CDashSmokeEffect::Free()
{
	__super::Free();
}
