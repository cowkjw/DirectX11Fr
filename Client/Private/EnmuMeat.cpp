#include "EnmuMeat.h"
#include "EnmuArm.h"	
#include "EnmuBody.h"
#include "EnmuHead.h"
#include "BossIdle.h"
#include "BossOpen.h"
#include "BossDeath.h"
#include "GameInstance.h"
#include "EnmuTentacle.h"
#include "BaseCharacter.h"
#include "UIProgressBar.h"

CEnmuMeat::CEnmuMeat(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject(pDevice, pContext)
	, m_fHp(300.f)
	, m_fMaxHp(300.f)
	, m_CD_Punch(0.f)
	, m_CD_Swing(0.f)
	, m_CD_Hand(0.f)
	, m_CD_Freeze(0.f)
	, m_CD_FollowPunch(0.f)
	, m_CD_Open(0.f)
	, m_CD_Tentacle(0.f)
	, m_LastPatternIdx(-1)
{
}

CEnmuMeat::CEnmuMeat(const CEnmuMeat& Prototype)
	: CGameObject(Prototype)
	, m_fHp(Prototype.m_fHp)
	, m_fMaxHp(Prototype.m_fMaxHp)
	, m_CD_Punch(Prototype.m_CD_Punch)
	, m_CD_Swing(Prototype.m_CD_Swing)
	, m_CD_Hand(Prototype.m_CD_Hand)
	, m_CD_Freeze(Prototype.m_CD_Freeze)
	, m_CD_FollowPunch(Prototype.m_CD_FollowPunch)
	, m_CD_Open(Prototype.m_CD_Open)
	, m_CD_Tentacle(Prototype.m_CD_Tentacle)
	, m_LastPatternIdx(Prototype.m_LastPatternIdx)
{
}
HRESULT CEnmuMeat::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CEnmuMeat::Initialize(void* pArg)
{
	GAMEOBJECT_DESC			Desc{};
	Desc.fRotationPerSec = XMConvertToRadians(90.f);
	Desc.fSpeedPerSec = 20.f;
	Desc.strName = TEXT("EnmuMeat");

	if (FAILED(__super::Initialize(&Desc)))
		return E_FAIL;

	if (FAILED(Ready_Parts()))
		return E_FAIL;
	if (FAILED(Ready_BehaviorTree()))
		return E_FAIL;


	//ChangeState(new BossIdle(TEXT("Idle")));
	ChangeState(new BossOpen(TEXT("Open")));


	m_vecTentacles.reserve(10);
	for (_int i = 0; i < 10; i++)
	{
		auto pGameObj = m_pGameInstance->Add_GameObject(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_GameObject_EnmuTentacle"),
			ToIndex(LEVEL::ENMU_BOSS), TEXT("Tentacle"));

		if (pGameObj)
		{
			m_vecTentacles.push_back(static_cast<CEnmuTentacle*>(pGameObj));
			pGameObj->SetActive(false);
		}
	}

	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(0.f, 0.f, 10.f, 1.f));

	return S_OK;
}

void CEnmuMeat::Priority_Update(_float fTimeDelta)
{
	if (m_fHp <= 0.f)
	{
		ChangeState(new BossDeath);
		return;
	}
	if (!m_pTarget)
	{
		m_pTarget = static_cast<CBaseCharacter*>(m_pGameInstance->Find_GameObjectByName(ToIndex(LEVEL::ENMU_BOSS), TEXT("Tanjiro")));

		for (auto& tentacle : m_vecTentacles)
		{
			if (tentacle)
			{
				tentacle->SetTarget(m_pTarget);
			}
		}
	}
	for (auto& child : m_vecChildren)
	{
		if (child->IsActive())
		{
			child->Priority_Update(fTimeDelta);
		}
	}
}

void CEnmuMeat::Update(_float fTimeDelta)
{
	if (m_fHitStopTime > 0.f)
	{
		m_fHitStopTime -= fTimeDelta;
		fTimeDelta *= 0.1f; // HitStop 동안 시간 느리게 흐름
	}
	if (m_pBossState)
	{
		m_pBossState->Update(this, fTimeDelta);
	}

	for (auto& child : m_vecChildren)
	{
		if (child->IsActive())
		{
			child->Update(fTimeDelta);
		}
	}
}

void CEnmuMeat::Late_Update(_float fTimeDelta)
{
	CGameObject::Late_Update(fTimeDelta);
	for (auto& child : m_vecChildren)
	{
		if (child->IsActive())
		{
			child->Late_Update(fTimeDelta);
		}
	}

	m_bAttacking = false;
}

HRESULT CEnmuMeat::Render()
{
	for (auto& child : m_vecChildren)
	{
		if (child->IsActive())
		{
			if (FAILED(child->Render()))
				return E_FAIL;
		}
	}
	return S_OK;
}

void CEnmuMeat::SetState(EnmuState eState)
{
	m_eState = eState;
	for (auto& child : m_vecChildren)
	{
		if (auto pEnmuParts = static_cast<CEnmuParts*>(child))
		{
			pEnmuParts->ChangeState(eState);
		}
	}
}

CEnmuParts* CEnmuMeat::GetPart(Parts ePart)
{
	auto parts = static_cast<CEnmuParts*>(m_vecChildren[ePart]);

	return parts;
}

void CEnmuMeat::SpawnTentacle(_int iIndex, _vector vPos, _vector vDir)
{
	if (iIndex < 0 || iIndex >= static_cast<_int>(m_vecTentacles.size()))
		return;
	auto pTentacle = m_vecTentacles[iIndex];
	if (!pTentacle)
		return;
	if (m_pTarget)
	{
		_vector vTargetPos = m_pTransformCom->Get_State(STATE::POSITION);
		vTargetPos.m128_f32[1] = -18.f; // 타겟의 Y 위치를 고정
		vTargetPos.m128_f32[2] = m_pTarget->GetTransform()->Get_State(STATE::POSITION).m128_f32[2];
		vDir = XMVectorSubtract(vTargetPos, vPos);
		vDir = XMVector3Normalize(vDir);
	}
	else
	{
		vDir = XMVectorSet(0.f, 0.f, 1.f, 0.f); // 기본 방향 설정
	}
//	pTentacle->GetTransform()->LookAtXZ(vPos);
	vDir = XMVectorSetY(vDir, 0.f); // Y 성분 무시하고 XZ 평면에서 방향 설정
	pTentacle->GetTransform()->RotateToDirection(vDir);

	vPos.m128_f32[1] = -18.f;

	pTentacle->GetTransform()->Set_State(STATE::POSITION, vPos);
}

void CEnmuMeat::DestroyTentacle(_int iIndex)
{
	if (iIndex < 0 || iIndex >= static_cast<_int>(m_vecTentacles.size()))
		return;
	auto pTentacle = m_vecTentacles[iIndex];
	if (!pTentacle)
		return;
	pTentacle->SetActive(false);
}

void CEnmuMeat::ActiveTentacle(_int iIndex, _bool bActive)
{
	if (iIndex < 0 || iIndex >= static_cast<_int>(m_vecTentacles.size()))
		return;
	auto pTentacle = m_vecTentacles[iIndex];
	if (!pTentacle)
		return;
	if (bActive)
	{
		// 애니메이션 재생
		pTentacle->PlayAniamtion();
	}
	else
	{ // 일단 보이게만
		pTentacle->SetActive(true);
	}
}

void CEnmuMeat::ResetTentacles()
{
	for (auto& pTentacle : m_vecTentacles)
	{
		if (pTentacle)
		{
			pTentacle->SetActive(false);
		}
	}
}

_float CEnmuMeat::GetDistanceToTarget() const
{
	if (nullptr == m_pTarget)
		return FLT_MAX;

	_vector myPos = XMLoadFloat4x4(&GetTransform()->Get_WorldMatrix()).r[3];
	_vector tgtPos = XMLoadFloat4x4(&m_pTarget->GetTransform()->Get_WorldMatrix()).r[3];
	_vector diff = XMVectorSubtract(tgtPos, myPos);
	return XMVectorGetZ(XMVector3Length(diff));
}

_float CEnmuMeat::Hit(_float fDamage)
{
	StartHitStop(0.25f);
	m_fHp -= fDamage;
	if (m_fHp <= 0.f)
	{
		m_fHp = 0.f;
		SetState(EnmuState::DIE);
	}
	auto pBody = GetPart(Parts::BODY);
	if (pBody)
	{
	pBody->Get_Animator()->SetTrigger("Hurt");
	}
	auto pBar = m_pGameInstance->Get_UI(TEXT("GameplayCanvas"), TEXT("RightLifeBar"));
	if (pBar)
	{
		CUIProgressBar* pRightBar = static_cast<CUIProgressBar*>(pBar);
		pRightBar->ApplyDamage(fDamage / m_fMaxHp * 100.f);
	}
	return m_fHp;
}

void CEnmuMeat::OnAttackHit(CGameObject* pTarget)
{
	if (m_bAttacking)
		return; // 이미 공격 중이면 무시
	if (auto pChar = dynamic_cast<CBaseCharacter*>(pTarget))
	{
		m_bAttacking = true;
		switch (m_eState)
		{
		case EnmuState::PUNCH:
			pChar->Blow(this, 40.f);
			pChar->TakeDamage(15.f);
			break;
		case EnmuState::FOLLOWPUNCH:
			pChar->HurtDown();
			pChar->TakeDamage(15.f);
			break;
		case EnmuState::HANDATTACK:
			//pChar->HurtDown();
			//pChar->TakeDamage(15.f);
			break;
		case EnmuState::TENTACLEATTACK:
			break;
		case EnmuState::FREEZEATTACK:
			break;
		case EnmuState::ANGRYFREEZEATTACK:
			break;
		case EnmuState::SWINGATTACK:
			/*pChar->Blow(this, 40.f);
			pChar->TakeDamage(15.f);*/

			break;
		default:
			break;
		}
	}
}

void CEnmuMeat::StartHitStop(_float duration)
{
	m_fHitStopTime = duration;
}

HRESULT CEnmuMeat::Ready_Parts()
{

	CEnmuArm::ARM_DESC armDesc;
	armDesc.sModelKey = TEXT("Prototype_Component_Model_EnmuLeftArm");

	AddChild(CEnmuArm::Create(m_pDevice, m_pContext, &armDesc));
	m_vecChildren.back()->GetTransform()->Rotate_EulerAngles(_float3(0.f, 180.f, 0.f)); // 
	armDesc.sModelKey = TEXT("Prototype_Component_Model_EnmuRightArm");
	AddChild(CEnmuArm::Create(m_pDevice, m_pContext, &armDesc));
	m_vecChildren.back()->GetTransform()->Rotate_EulerAngles(_float3(0.f, -180.f, 0.f)); // 
	AddChild(CEnmuBody::Create(m_pDevice, m_pContext));
	m_vecChildren.back()->GetTransform()->Rotate_EulerAngles(_float3(0.f, 180.f, 0.f)); // 
	AddChild(CEnmuHead::Create(m_pDevice, m_pContext));
	m_vecChildren.back()->GetTransform()->Rotate_EulerAngles(_float3(0.f, 180.f, 0.f)); // 

	return S_OK;
}

HRESULT CEnmuMeat::Ready_BehaviorTree()
{
	return S_OK;
}

void CEnmuMeat::ChangeState(BossState* pState)
{
	if (m_pBossState)
	{
		m_pBossState->Exit(this);

	}
	if (pState)
	{
		pState->Enter(this);
	}
	Safe_Delete(m_pBossState);
	m_pBossState = pState;
}

CEnmuMeat* CEnmuMeat::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg)
{
	CEnmuMeat* pInstance = new CEnmuMeat(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CEnmuMeat");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CEnmuMeat::Clone(void* pArg)
{
	CEnmuMeat* pClone = new CEnmuMeat(*this);
	if (FAILED(pClone->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CEnmuMeat");
		Safe_Release(pClone);
	}
	return pClone;
}

void CEnmuMeat::Free()
{
	__super::Free();
	Safe_Delete(m_pBossState);
}


