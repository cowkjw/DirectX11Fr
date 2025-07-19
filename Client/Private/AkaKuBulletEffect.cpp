#include "AkaKuBulletEffect.h"
#include "GameInstance.h"
#include <JsonLoader.h>
#include "EffectManager.h"
#include "ParticleSystem.h"
#include "RingWindEffect.h"
#include "HitParticle.h"
#include "BaseCharacter.h"	


_int CAkaKuBulletEffect::m_iHitCount = 0;
CAkaKuBulletEffect::CAkaKuBulletEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMeshEffect(pDevice, pContext)
{
}
CAkaKuBulletEffect::CAkaKuBulletEffect(const CAkaKuBulletEffect& Prototype)
	: CMeshEffect(Prototype),
	m_vColor(Prototype.m_vColor),
	m_fDir(Prototype.m_fDir)
	, m_fRotationSpeed(Prototype.m_fRotationSpeed)
	, m_fDuration(Prototype.m_fDuration)
{
}

HRESULT CAkaKuBulletEffect::Initialize_Prototype()
{
	if (FAILED(Ready_Components()))
		return E_FAIL;
	m_bUseOffset = true;
	m_bRenderMesh = true;
	m_vUVOffset = _float2(0.f, 0.f); // UV 오프셋 초기화
	m_vUVScale = _float2(1.f, 12.f); // UV 스케일 설정
	m_vColor = _float4(0.3f, 0.6f, 1.f, 1.f); // 색상 초기화
	m_iShaderPass =14;
	m_fDuration = 1.5f; // 1.5 동안 지속

	return S_OK;
}

HRESULT CAkaKuBulletEffect::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_pTransformCom->Scaling(_float3(3.f,3.f,3.f));
	m_pTransformCom->Set_SpeedPerSec(100.f);

	m_pColliderCom = CSphereCollider::Create(m_pDevice, m_pContext, 3.f);

	if (!m_pColliderCom)
		return E_FAIL;

	Add_Component(TEXT("Com_Collider"), CSphereCollider::Create(m_pDevice, m_pContext, 3.f), reinterpret_cast<CComponent**>(&m_pColliderCom));

	m_pColliderCom->Initialize(nullptr);
	m_pColliderCom->SetListener(this);
	m_pColliderCom->SetColliderType(ColliderType::HITBOX);
	m_pColliderCom->SetActive(false);
//	m_pRingWindEffect = CRingWindEffect::Create(m_pDevice, m_pContext);
	m_pRingWindEffect = m_pGameInstance->Add_GameObject(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Effect_RingWind"), ToIndex(LEVEL::GAMEPLAY), TEXT("RingWind"));
	if (!m_pRingWindEffect)
		return E_FAIL;
	m_pRingWindEffect->Initialize(nullptr);
	m_pRingWindEffect->SetActive(false);
	return S_OK;
}

void CAkaKuBulletEffect::Priority_Update(_float fTimeDelta)
{
	
	for (auto& particle : m_ParticleEffects)
	{
		if (particle.second && particle.second->IsActive())
		{
			particle.second->Priority_Update(fTimeDelta);
		}
	}
}

void CAkaKuBulletEffect::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);

	if (m_pRingWindEffect && m_pRingWindEffect->IsActive())
	{
		m_pRingWindEffect->Update(fTimeDelta);
	}
	m_fElapsed += fTimeDelta;
	m_vUVOffset.y += fTimeDelta * 0.2f;
	if (m_fElapsed >= m_fDuration)
	{
		m_fElapsed = 0.f;

		SetActive(false);
	}
	else
	{
		m_pTransformCom->MoveDirection(m_pTransformCom->Get_State(STATE::LOOK), fTimeDelta);
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

void CAkaKuBulletEffect::Late_Update(_float fTimeDelta)
{
	if (m_bRenderMesh)
	{
	//	__super::Late_Update(fTimeDelta);
		m_pGameInstance->Add_RenderGroup(RENDERGROUP::DISTORTION, this);
		m_pGameInstance->Add_RenderGroup(RENDERGROUP::BLUR_EFFECT, this);
	}
	if (m_pRingWindEffect && m_pRingWindEffect->IsActive())
	{
		m_pRingWindEffect->Late_Update(fTimeDelta);
	}
	for (auto& particle : m_ParticleEffects)
	{
		if (particle.second && particle.second->IsActive())
		{
			particle.second->Late_Update(fTimeDelta);
		}
	}
}

HRESULT CAkaKuBulletEffect::Render()
{
	if (m_Textures[TEX_NOISE])
	{
		if (FAILED(m_Textures[TEX_NOISE]->Bind_ShaderResource(m_pShaderCom, "g_NoiseTexture", 0)))
			return E_FAIL;
	}
	if (m_Textures[TEX_ALPHA])
	{
		if (FAILED(m_Textures[TEX_ALPHA]->Bind_ShaderResource(m_pShaderCom, "g_AlphaTexture", 0)))
			return E_FAIL;
	}

	if (FAILED(__super::Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CAkaKuBulletEffect::Ready_Components()
{
	if (FAILED(__super::Add_Component(TEXT("Com_Shader"), m_pGameInstance->GetShader(TEXT("Shader_VtxMesh"), true), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;


	/* For.Com_Model */
	if (FAILED(__super::Add_Component(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_KuuBullet"),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
	{
		return E_FAIL;
	}
	m_Textures[TEX_NOISE] = m_pGameInstance->GetTexture(L"KuuNoise", true);
	if (m_Textures[TEX_NOISE] == nullptr)
	{
		m_Textures[TEX_NOISE] = m_pGameInstance->LoadTexture(L"KuuNoise", L"../Asset/Resources/Models/Effect/Akaza/Kuu/T_e_Cmn_Noise004.dds", true);
		Safe_AddRef(m_Textures[TEX_NOISE]);
	}

	m_Textures[TEX_ALPHA] = m_pGameInstance->GetTexture(L"KuuAlp", true);
	if (m_Textures[TEX_ALPHA] == nullptr)
	{
		m_Textures[TEX_ALPHA] = m_pGameInstance->LoadTexture(L"KuuAlp", L"../Asset/Resources/Models/Effect/Akaza/Kuu/AlphaRing.dds", true);
		Safe_AddRef(m_Textures[TEX_ALPHA]);
	}
	return S_OK;
}

HRESULT CAkaKuBulletEffect::Bind_Shader()
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

void CAkaKuBulletEffect::OnDisable()
{
	if (m_pColliderCom)
	{
		m_pColliderCom->SetActive(false);
		m_pColliderCom->SetDrawDebug(false);
	}
	m_vUVOffset = _float2(0.f, 0.f); // UV 오프셋 초기화
	m_fElapsed = 0.f; // 시간 초기화
}

void CAkaKuBulletEffect::OnEnable()
{
	m_vUVOffset = _float2(0.f, 0.f); // UV 오프셋 초기화
	m_fElapsed = 0.f; // 시간 초기화
	if (m_pColliderCom)
	{
		m_pColliderCom->SetActive(true); 
		m_pColliderCom->SetDrawDebug(true);
	}
	//_float3 vPos{};
	//XMStoreFloat3(&vPos, m_pTransformCom->Get_State(STATE::POSITION));
	//CEffectManager::Get_Instance()->SpawnParticleEffect(L"Fire", vPos,_float3(1.f,2.f,1.f));
	//CEffectManager::Get_Instance()->SpawnParticleEffect(L"FireSpread", vPos);

}

CAkaKuBulletEffect* CAkaKuBulletEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CAkaKuBulletEffect* pInstance = new CAkaKuBulletEffect(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CAkaKuBulletEffect");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CAkaKuBulletEffect::Clone(void* pArg)
{
	CAkaKuBulletEffect* pClone = new CAkaKuBulletEffect(*this);
	if (FAILED(pClone->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CAkaKuBulletEffect");
		Safe_Release(pClone);
	}
	return pClone;
}

void CAkaKuBulletEffect::Free()
{
	__super::Free();
}

void CAkaKuBulletEffect::OnCollisionEnter(Engine::CCollider* other)
{
}

void CAkaKuBulletEffect::OnCollisionEnter(CCollider* other, const _float3& hitPos)
{
	if (other->GetOwner() != m_pParent)
	{
		if (auto pTarget = dynamic_cast<CBaseCharacter*>(other->GetOwner()))
		{
			if (pTarget->IsActive())
			{
				m_iHitCount++;

				if (m_iHitCount == 3)
				{
					if (pTarget->GetState() != CBaseCharacter::CSTATE::DIE&&
						pTarget->GetState() != CBaseCharacter::CSTATE::GUARD)
					{
					pTarget->Blow(this,30.f);
					}
				
				}
				if (m_pRingWindEffect && m_pRingWindEffect->IsActive() == false)
				{
					_vector vPos = pTarget->GetTransform()->Get_State(STATE::POSITION);
					vPos = XMVectorSetY(vPos, XMVectorGetY(vPos) + 8.f);
					m_pRingWindEffect->GetTransform()->Set_State(STATE::POSITION, vPos);
					m_pRingWindEffect->SetActive(true);
				}
				pTarget->TakeDamage(3.f);
				SetActive(false);
				if (m_pColliderCom)
				{
					m_pColliderCom->SetActive(false);
					m_pColliderCom->SetDrawDebug(false);
				}
			}
		}
	}
}

void CAkaKuBulletEffect::OnCollisionStay(Engine::CCollider* other, float fTimeDelta)
{
}

void CAkaKuBulletEffect::OnCollisionExit(Engine::CCollider* other)
{
}
