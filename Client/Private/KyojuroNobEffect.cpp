#include "KyojuroNobEffect.h"
#include "GameInstance.h"
#include <JsonLoader.h>
#include "ParticleSystem.h"
#include "HitParticle.h"

CKyojuroNobEffect::CKyojuroNobEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMeshEffect(pDevice, pContext)
{
}
CKyojuroNobEffect::CKyojuroNobEffect(const CKyojuroNobEffect& Prototype)
	: CMeshEffect(Prototype),
	m_vColor(Prototype.m_vColor),
	m_fDir(Prototype.m_fDir)
	, m_fRotationSpeed(Prototype.m_fRotationSpeed)
	, m_fDuration(Prototype.m_fDuration)
{
}

HRESULT CKyojuroNobEffect::Initialize_Prototype()
{
	if (FAILED(Ready_Components()))
		return E_FAIL;
	m_bUseOffset = true;
	m_bRenderMesh = true;
	m_vUVOffset = _float2(0.f, 0.f); // UV 오프셋 초기화
	m_vUVScale = _float2(3.f, 1.f); // UV 스케일 설정
	m_iShaderPass = 6;
	return S_OK;
}

HRESULT CKyojuroNobEffect::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_pTransformCom->Scaling(_float3(12.f,12.f,12.f));


	return S_OK;
}

void CKyojuroNobEffect::Priority_Update(_float fTimeDelta)
{
	
	for (auto& particle : m_ParticleEffects)
	{
		if (particle.second && particle.second->IsActive())
		{
			particle.second->Priority_Update(fTimeDelta);
		}
	}
}

void CKyojuroNobEffect::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);

	m_fElapsed += fTimeDelta;
	m_vUVOffset.x += fTimeDelta * 0.2f;
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

void CKyojuroNobEffect::Late_Update(_float fTimeDelta)
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

HRESULT CKyojuroNobEffect::Render()
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

HRESULT CKyojuroNobEffect::Ready_Components()
{
	if (FAILED(__super::Add_Component(TEXT("Com_Shader"), m_pGameInstance->GetShader(TEXT("Shader_VtxMesh"), true), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;


	/* For.Com_Model */
	if (FAILED(__super::Add_Component(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_Nob"),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
	{
		return E_FAIL;
	}
	return S_OK;
}

HRESULT CKyojuroNobEffect::Bind_Shader()
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

void CKyojuroNobEffect::OnDisable()
{
	m_vUVScale = _float2(3.f, 1.f); // UV 스케일 설정
	m_vUVOffset = _float2(0.f, 0.f); // UV 오프셋 초기화
	m_fElapsed = 0.f; // 시간 초기화
}

void CKyojuroNobEffect::OnEnable()
{
	m_vUVScale = _float2(3.f, 1.f); // UV 스케일 설정
	m_vUVOffset = _float2(0.f, 0.f); // UV 오프셋 초기화
	m_fElapsed = 0.f; // 시간 초기화
}

CKyojuroNobEffect* CKyojuroNobEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CKyojuroNobEffect* pInstance = new CKyojuroNobEffect(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CKyojuroNobEffect");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CKyojuroNobEffect::Clone(void* pArg)
{
	CKyojuroNobEffect* pClone = new CKyojuroNobEffect(*this);
	if (FAILED(pClone->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CKyojuroNobEffect");
		Safe_Release(pClone);
	}
	return pClone;
}

void CKyojuroNobEffect::Free()
{
	__super::Free();
}
