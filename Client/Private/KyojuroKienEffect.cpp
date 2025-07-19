#include "KyojuroKienEffect.h"
#include "GameInstance.h"
#include <JsonLoader.h>
#include "ParticleSystem.h"
#include "HitParticle.h"

CKyojuroKienEffect::CKyojuroKienEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMeshEffect(pDevice, pContext)
{
}
CKyojuroKienEffect::CKyojuroKienEffect(const CKyojuroKienEffect& Prototype)
	: CMeshEffect(Prototype),
	m_vColor(Prototype.m_vColor),
	m_fDir(Prototype.m_fDir)
	, m_fRotationSpeed(Prototype.m_fRotationSpeed)
{
}

HRESULT CKyojuroKienEffect::Initialize_Prototype()
{
	if (FAILED(Ready_Components()))
		return E_FAIL;
	m_bUseOffset = true;
	m_bRenderMesh = true;
	m_vUVOffset = _float2(1.f, 0.f); // UV 오프셋 초기화
	m_iShaderPass = 5;
	return S_OK;
}

HRESULT CKyojuroKienEffect::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_pTransformCom->Scaling(_float3(9.f,9.f,9.f));


	return S_OK;
}

void CKyojuroKienEffect::Priority_Update(_float fTimeDelta)
{
	
	for (auto& particle : m_ParticleEffects)
	{
		if (particle.second && particle.second->IsActive())
		{
			particle.second->Priority_Update(fTimeDelta);
		}
	}
}

void CKyojuroKienEffect::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
	if (m_bUseOffset)
	{
		m_vUVOffset.x -= fTimeDelta * 1.f;
		if (m_vUVOffset.x <=0.67f)
		{ 
			SetActive(false);
		}

	}
	_float m_fCurScale = m_pTransformCom->Get_Scaled().x;
	if (m_fCurScale < m_fDestScale)
	{
		m_fCurScale += fTimeDelta * 0.1;
		if (m_fCurScale > m_fDestScale)
			m_fCurScale = m_fDestScale;
		m_pTransformCom->Scaling(_float3(m_fCurScale, m_fCurScale, m_fCurScale));
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

void CKyojuroKienEffect::Late_Update(_float fTimeDelta)
{
	if (m_bRenderMesh)
	{
		m_pGameInstance->Add_RenderGroup(RENDERGROUP::NONBLEND, this);
		m_pGameInstance->Add_RenderGroup(RENDERGROUP::BLOOM_EFFECT, this);
	}
	for (auto& particle : m_ParticleEffects)
	{
		if (particle.second && particle.second->IsActive())
		{
			particle.second->Late_Update(fTimeDelta);
		}
	}
}

HRESULT CKyojuroKienEffect::Render()
{
	// 4번에 SmokeMask넣어둠
	// 6번에 distortion 넣어둠
	// 8번에 알파
	// 1번에 디퓨즈
	m_pModelCom->Bind_Material(m_pShaderCom, "g_EmissiveTexture", 0, aiTextureType_EMISSIVE, 0);
	if (FAILED(__super::Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CKyojuroKienEffect::Ready_Components()
{
	if (FAILED(__super::Add_Component(TEXT("Com_Shader"), m_pGameInstance->GetShader(TEXT("Shader_VtxMesh"), true), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;


	/* For.Com_Model */
	if (FAILED(__super::Add_Component(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_Kien"),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
	{
		return E_FAIL;
	}
	return S_OK;
}

HRESULT CKyojuroKienEffect::Bind_Shader()
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

void CKyojuroKienEffect::OnDisable()
{
	//if (m_pHitParticle && m_pHitParticle->IsActive())
	//{
	//	m_pHitParticle->SetActive(false); // 히트 파티클 비활성화
	//}
	m_vUVOffset = _float2(1.f, 0.f); // UV 오프셋 초기화
	m_pTransformCom->Scaling(_float3(m_fScale, m_fScale, m_fScale));
}

void CKyojuroKienEffect::OnEnable()
{
	m_vUVOffset = _float2(1.f, 0.f); // UV 오프셋 초기화
	m_pTransformCom->Scaling(_float3(m_fScale, m_fScale, m_fScale));
}

CKyojuroKienEffect* CKyojuroKienEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CKyojuroKienEffect* pInstance = new CKyojuroKienEffect(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CKyojuroKienEffect");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CKyojuroKienEffect::Clone(void* pArg)
{
	CKyojuroKienEffect* pClone = new CKyojuroKienEffect(*this);
	if (FAILED(pClone->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CKyojuroKienEffect");
		Safe_Release(pClone);
	}
	return pClone;
}

void CKyojuroKienEffect::Free()
{
	__super::Free();
}
