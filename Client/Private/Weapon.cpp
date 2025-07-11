#include "Weapon.h"
#include "GameInstance.h"
#include <BaseCharacter.h>
#include "EffectManager.h"	
#include <EnmuParts.h>

CWeapon::CWeapon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject(pDevice, pContext)
	, m_pShaderCom{ nullptr }
	, m_pModelCom{ nullptr }
	, m_pColliderCom{ nullptr }
	, m_pColliderCom1{ nullptr }
	, m_pColliderCom2{ nullptr }

{
	XMStoreFloat4x4(&m_CombinedWorldMatrix, XMMatrixIdentity());
}


CWeapon::CWeapon(const CWeapon& Prototype) :
	CGameObject(Prototype)
	, m_pShaderCom{ Prototype.m_pShaderCom }
	, m_pModelCom{ Prototype.m_pModelCom }
	, m_pColliderCom{ Prototype.m_pColliderCom }
	, m_pColliderCom1{ Prototype.m_pColliderCom1 }
	, m_pColliderCom2{ Prototype.m_pColliderCom2 }
	, m_CombinedWorldMatrix(Prototype.m_CombinedWorldMatrix)

{
}

HRESULT CWeapon::Initialize_Prototype()
{
	Ready_Components();
	return S_OK;
}

HRESULT CWeapon::Initialize(void* pArg)
{
	GAMEOBJECT_DESC			Desc{};

	Desc.fRotationPerSec = XMConvertToRadians(90.f);
	Desc.fSpeedPerSec = 0.f;
	Desc.strName = TEXT("Weapon");

	if (FAILED(__super::Initialize(&Desc)))
		return E_FAIL;

	m_pTransformCom->Scaling(_float3(0.1f, 0.1f, 0.1f));

	Add_Component(TEXT("Com_Collider"), CSphereCollider::Create(m_pDevice, m_pContext,2.f), reinterpret_cast<CComponent**>(&m_pColliderCom));
	m_pColliderCom->SetOffset(_float3(-3.3f, 12.5f, 24.3f)); // z는 앞으로 하면서 y값 올려야함
	m_pColliderCom->Initialize(nullptr);
	m_pColliderCom->SetListener(this);

	Add_Component(TEXT("Com_Collider1"), CSphereCollider::Create(m_pDevice, m_pContext, 2.f), reinterpret_cast<CComponent**>(&m_pColliderCom1));
	m_pColliderCom1->SetOffset(_float3(-5.6f, 21.2f, 49.6f)); // z는 앞으로 하면서 y값 올려야함
	m_pColliderCom1->Initialize(nullptr);
	m_pColliderCom1->SetListener(this);

	Add_Component(TEXT("Com_Collider2"), CSphereCollider::Create(m_pDevice, m_pContext, 2.f), reinterpret_cast<CComponent**>(&m_pColliderCom2));
	m_pColliderCom2->SetOffset(_float3(-9.3f, 29.9f,72.9f)); // z는 앞으로 하면서 y값 올려야함
	m_pColliderCom2->Initialize(nullptr);
	m_pColliderCom2->SetListener(this);


	m_pColliderCom->SetColliderType(ColliderType::HITBOX);
	m_pColliderCom1->SetColliderType(ColliderType::HITBOX);
	m_pColliderCom2->SetColliderType(ColliderType::HITBOX);

	m_pColliderCom->SetActive(false);
	m_pColliderCom1->SetActive(false);
	m_pColliderCom2->SetActive(false);

	return S_OK;
}

void CWeapon::Priority_Update(_float fTimeDelta)
{
}

void CWeapon::Update(_float fTimeDelta)
{
	if (m_pBoneSocket)
	{
		_float4x4 parentWorld = m_pParent->GetTransform()->Get_WorldMatrix();
		_float4x4 boneLocal = *m_pBoneSocket->Get_CombinedTransformationMatrix();
		_matrix wepaonLocal = m_pTransformCom->Get_WorldMatrix_Inverse();
		_matrix world = XMMatrixMultiply(XMLoadFloat4x4(&boneLocal), XMLoadFloat4x4(&parentWorld));
		_matrix weaponWorld = XMMatrixMultiply(wepaonLocal, world);
		//_matrix World = XMMatrixMultiply(XMLoadFloat4x4(&m_pTransformCom->Get_WorldMatrix()),XMLoadFloat4x4(m_pBoneSocket->Get_CombinedTransformationMatrix()) );
		_float4x4 WorldMatrix{};
		XMStoreFloat4x4(&WorldMatrix, world);
		m_pTransformCom->Set_WorldMatrix(WorldMatrix);
		//	m_pTransformCom->Set_WorldMatrix();

	}
	if (auto pCharacter = static_cast<CBaseCharacter*>(m_pParent))
	{
	/*	_bool bCanAttack = pCharacter->GetState() == CBaseCharacter::CSTATE::ATTACK || pCharacter->GetState() == CBaseCharacter::CSTATE::SKILL;

		m_pColliderCom->SetActive(bCanAttack);
		m_pColliderCom1->SetActive(bCanAttack);
		m_pColliderCom2->SetActive(bCanAttack);


		m_pColliderCom->SetDrawDebug(bCanAttack);
		m_pColliderCom1->SetDrawDebug(bCanAttack);
		m_pColliderCom2->SetDrawDebug(bCanAttack);*/
	}
}

void CWeapon::Late_Update(_float fTimeDelta)
{
	CGameObject::Late_Update(fTimeDelta);


	//if (m_pBoneSocket)
	//{
	//	_float4x4 parentWorld = m_pParent->GetTransform()->Get_WorldMatrix();
	//	_float4x4 boneLocal = *m_pBoneSocket->Get_CombinedTransformationMatrix();
	//	_matrix matBoneLocal = XMLoadFloat4x4(&boneLocal);
	//	for (size_t i = 0; i < 3; i++)
	//		matBoneLocal.r[i] = XMVector3Normalize(matBoneLocal.r[i]);

	//	//	_matrix world = XMMatrixMultiply(XMLoadFloat4x4(&boneLocal), XMLoadFloat4x4(&parentWorld));
	//	_matrix world = /*XMLoadFloat4x4(&m_pTransformCom->Get_WorldMatrix())**/matBoneLocal * XMLoadFloat4x4(&parentWorld);
	//	//world = XMMatrixMultiply(matScale, world); 

	//	_float4x4 WorldMatrix{};
	//	XMStoreFloat4x4(&m_CombinedWorldMatrix, world);
	//	//m_pTransformCom->Set_WorldMatrix(WorldMatrix);
	//}

	m_pGameInstance->Add_RenderGroup(RENDERGROUP::NONBLEND, this);
}

HRESULT CWeapon::Render()
{
	Bind_Shaders();


	_uint		iNumMesh = m_pModelCom->Get_NumMeshes();

	for (_uint i = 0; i < iNumMesh; i++)
	{
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0)))
			return E_FAIL;

		m_pModelCom->Bind_Bone_Matrices(m_pShaderCom, "g_BoneMatrices", i);

		if (FAILED(m_pShaderCom->Begin(1)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

void CWeapon::LaucnhTargetAirborne(CBaseCharacter* pTarget, _float fForce)
{
	if (!pTarget || pTarget->GetState()== CBaseCharacter::CSTATE::GUARD)
		return;
	pTarget->LaunchAirborne(fForce);
}



HRESULT CWeapon::Ready_Components()
{
	///* For.Com_Shader */
	if (FAILED(__super::Add_Component(TEXT("Com_Shader"), m_pGameInstance->GetShader(TEXT("Shader_VtxMesh"), true), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* For.Com_Model */
	if (FAILED(__super::Add_Component(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_KoujuroWeapon"),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
	{
		if (FAILED(__super::Add_Component(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Component_Model_TanjiroWeapon"),
			TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;
	}
	///* For.Com_Collider */
	//if (FAILED(__super::Add_Component(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Collider_Capsule"),
	//	TEXT("Com_Collider"), reinterpret_cast<CComponent**>(&m_pColliderCom))))
	//	return E_FAIL;
	return S_OK;
}

HRESULT CWeapon::Bind_Shaders()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	/*if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
		return E_FAIL;*/
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(TRANSFORM::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(TRANSFORM::PROJECTION))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vCamPosition", m_pGameInstance->Get_CamPosition(), sizeof(_float4))))
		return E_FAIL;

	_float fCamFar = m_pGameInstance->Get_CameraFar();
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fCameraFar", &fCamFar, sizeof(_float))))
		return E_FAIL;

	return S_OK;
}

CWeapon* CWeapon::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CWeapon* pInstance = new CWeapon(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CWeapon");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CWeapon::Clone(void* pArg)
{
	CWeapon* pInstance = new CWeapon(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CWeapon");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CWeapon::Free()
{
	__super::Free();

	if (!m_bIsCloned)
	{
	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
	}
	Safe_Release(m_pColliderCom);
	Safe_Release(m_pColliderCom1);
	Safe_Release(m_pColliderCom2);
}

void CWeapon::OnCollisionEnter(CCollider* other)
{
	if (auto pTarget = dynamic_cast<CBaseCharacter*>(other->GetOwner()))
	{
		if (m_DamagedTargets.find(pTarget) != m_DamagedTargets.end())
			return; // 이미 데미지를 입힌 대상이면 무시
		m_DamagedTargets.insert(pTarget); // 데미지를 입힌 대상에 추가

		if (auto pChar = dynamic_cast<CBaseCharacter*>(m_pParent))
		{
			if (pTarget->GetState() != CBaseCharacter::CSTATE::DIE)
			{
				CSoundMag::Get_Instance()->PlayEffect("event:/Common/SlashHit");
				pChar->OnAttackHit(pTarget);
				m_pColliderCom->SetActive(false);
				m_pColliderCom1->SetActive(false);
				m_pColliderCom2->SetActive(false);
				m_pColliderCom->SetDrawDebug(false);
				m_pColliderCom1->SetDrawDebug(false);
				m_pColliderCom2->SetDrawDebug(false);
			}
		}
	}
	// 때린게 엔무 파츠면
	else if (auto pBossParts = dynamic_cast<CEnmuParts*>(other->GetOwner()))
	{
		if (m_DamagedTargets.find(pBossParts) != m_DamagedTargets.end())
			return; // 이미 데미지를 입힌 대상이면 무시
		m_DamagedTargets.insert(pBossParts); // 데미지를 입힌 대상에 추가
		if (auto pBoss = static_cast<CEnmuMeat*>(pBossParts->GetParent()))
		{
			if (auto pChar = dynamic_cast<CBaseCharacter*>(m_pParent))
			{
				CSoundMag::Get_Instance()->PlayEffect("event:/Common/SlashHit");
				pChar->OnAttackHit(pBoss);
				m_pColliderCom->SetActive(false);
				m_pColliderCom1->SetActive(false);
				m_pColliderCom2->SetActive(false);
				m_pColliderCom->SetDrawDebug(false);
				m_pColliderCom1->SetDrawDebug(false);
				m_pColliderCom2->SetDrawDebug(false);
			}
		}
	}
	else if (auto pBossParts = dynamic_cast<CEnmuParts*>(other->GetOwner()->GetParent()))
	{
		if (m_DamagedTargets.find(pBossParts) != m_DamagedTargets.end())
			return; // 이미 데미지를 입힌 대상이면 무시
		m_DamagedTargets.insert(pBossParts); // 데미지를 입힌 대상에 추가
		if (auto pBoss = static_cast<CEnmuMeat*>(pBossParts->GetParent()))
		{
			if (auto pChar = dynamic_cast<CBaseCharacter*>(m_pParent))
			{
				CSoundMag::Get_Instance()->PlayEffect("event:/Common/SlashHit");
				pChar->OnAttackHit(pBoss);
				m_pColliderCom->SetActive(false);
				m_pColliderCom1->SetActive(false);
				m_pColliderCom2->SetActive(false);
				m_pColliderCom->SetDrawDebug(false);
				m_pColliderCom1->SetDrawDebug(false);
				m_pColliderCom2->SetDrawDebug(false);
			}
		}
	}
}

void CWeapon::OnCollisionEnter(CCollider* other, const XMFLOAT3& hitPos)
{
	if (auto pTarget = dynamic_cast<CBaseCharacter*>(other->GetOwner()))
	{
		//if (m_DamagedTargets.find(pTarget) != m_DamagedTargets.end())
		//	return; // 이미 데미지를 입힌 대상이면 무시
		//m_DamagedTargets.insert(pTarget); // 데미지를 입힌 대상에 추가
		m_pColliderCom->SetActive(false);
		m_pColliderCom1->SetActive(false);
		m_pColliderCom2->SetActive(false);
		m_pColliderCom->SetDrawDebug(false);
		m_pColliderCom1->SetDrawDebug(false);
		m_pColliderCom2->SetDrawDebug(false);
		CEffectManager::Get_Instance()->SpawnParticleEffect(TEXT("SlashHitParticle"), hitPos);
		CEffectManager::Get_Instance()->SpawnParticleEffect(TEXT("HitCrossParticle"), hitPos);
		CEffectManager::Get_Instance()->SpawnParticleEffect(TEXT("HitShockParticle"), hitPos);

	}
	// 때린게 엔무 파츠면
	else if (auto pBossParts = dynamic_cast<CEnmuParts*>(other->GetOwner()))
	{
		//if (m_DamagedTargets.find(pBossParts) != m_DamagedTargets.end())
		//	return; // 이미 데미지를 입힌 대상이면 무시
		//m_DamagedTargets.insert(pBossParts); // 데미지를 입힌 대상에 추가
		CEffectManager::Get_Instance()->SpawnParticleEffect(TEXT("SlashHitParticle"), hitPos);
	//	CEffectManager::Get_Instance()->SpawnParticleEffect(TEXT("HitCrossParticle"), hitPos);
		m_pColliderCom->SetActive(false);
		m_pColliderCom1->SetActive(false);
		m_pColliderCom2->SetActive(false);
		m_pColliderCom->SetDrawDebug(false);
		m_pColliderCom1->SetDrawDebug(false);
		m_pColliderCom2->SetDrawDebug(false);
	}
	else if (auto pBossParts = dynamic_cast<CEnmuParts*>(other->GetOwner()->GetParent()))
	{
		//if (m_DamagedTargets.find(pBossParts) != m_DamagedTargets.end())
		//	return; // 이미 데미지를 입힌 대상이면 무시
		//m_DamagedTargets.insert(pBossParts); // 데미지를 입힌 대상에 추가
		m_pColliderCom->SetActive(false);
		m_pColliderCom1->SetActive(false);
		m_pColliderCom2->SetActive(false);
		m_pColliderCom->SetDrawDebug(false);
		m_pColliderCom1->SetDrawDebug(false);
		m_pColliderCom2->SetDrawDebug(false);
		CEffectManager::Get_Instance()->SpawnParticleEffect(TEXT("SlashHitParticle"), hitPos);
		//CEffectManager::Get_Instance()->SpawnParticleEffect(TEXT("HitCrossParticle"), hitPos);
	}
}


void CWeapon::OnCollisionStay(CCollider* other, float fTimeDelta)
{

}

void CWeapon::OnCollisionExit(CCollider* other)
{

}
