#include "KyojuroEnkSpiralEffect.h"
#include "GameInstance.h"
#include <JsonLoader.h>
#include "ParticleSystem.h"
#include "EffectManager.h"
#include "HitParticle.h"

CKyojuroEnkSpiralEffect::CKyojuroEnkSpiralEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMeshEffect(pDevice, pContext)
{
}
CKyojuroEnkSpiralEffect::CKyojuroEnkSpiralEffect(const CKyojuroEnkSpiralEffect& Prototype)
	: CMeshEffect(Prototype),
	m_vColor(Prototype.m_vColor),
	m_fDir(Prototype.m_fDir)
	, m_fRotationSpeed(Prototype.m_fRotationSpeed)
{
}

HRESULT CKyojuroEnkSpiralEffect::Initialize_Prototype()
{
	if (FAILED(Ready_Components()))
		return E_FAIL;
	return S_OK;
}

HRESULT CKyojuroEnkSpiralEffect::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;
	m_iShaderPass = 4; 
	m_pTransformCom->Scaling(_float3(0.3f, 0.3f, 0.35f));

	m_bUseOffset = true;
	m_bRenderMesh = true;


	m_fRotationSpeed = 5.f;
	m_pTransformCom->Set_SpeedPerSec(45.f);
	return S_OK;
}

void CKyojuroEnkSpiralEffect::Priority_Update(_float fTimeDelta)
{
	
	for (auto& particle : m_ParticleEffects)
	{
		if (particle.second && particle.second->IsActive())
		{
			particle.second->Priority_Update(fTimeDelta);
		}
	}
}

void CKyojuroEnkSpiralEffect::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
	_vector vAxis = m_pTransformCom->Get_State(STATE::LOOK) * m_fDir;
	m_pTransformCom->Turn(vAxis, fTimeDelta*m_fRotationSpeed);
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

void CKyojuroEnkSpiralEffect::Late_Update(_float fTimeDelta)
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

HRESULT CKyojuroEnkSpiralEffect::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CKyojuroEnkSpiralEffect::Ready_Components()
{
	if (FAILED(__super::Add_Component(TEXT("Com_Shader"), m_pGameInstance->GetShader(TEXT("Shader_VtxMesh"), true), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;


	/* For.Com_Model */
	if (FAILED(__super::Add_Component(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_EnkFireSpiral"),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
	{
		return E_FAIL;
	}
	return S_OK;
}

HRESULT CKyojuroEnkSpiralEffect::Bind_Shader()
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

void CKyojuroEnkSpiralEffect::OnDisable()
{
	m_vUVOffset = _float2(0.0f, 0.f); // UV 오프셋 초기화
}

void CKyojuroEnkSpiralEffect::OnEnable()
{

}

CKyojuroEnkSpiralEffect* CKyojuroEnkSpiralEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CKyojuroEnkSpiralEffect* pInstance = new CKyojuroEnkSpiralEffect(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CKyojuroEnkSpiralEffect");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CKyojuroEnkSpiralEffect::Clone(void* pArg)
{
	CKyojuroEnkSpiralEffect* pClone = new CKyojuroEnkSpiralEffect(*this);
	if (FAILED(pClone->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CKyojuroEnkSpiralEffect");
		Safe_Release(pClone);
	}
	return pClone;
}

void CKyojuroEnkSpiralEffect::Free()
{
	__super::Free();
}
