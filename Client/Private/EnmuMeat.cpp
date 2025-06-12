#include "EnmuMeat.h"
#include "EnmuArm.h"	
#include "EnmuBody.h"
#include "EnmuHead.h"
#include "BossIdle.h"
#include "GameInstance.h"
#include "EnmuTentacle.h"
#include "BaseCharacter.h"

CEnmuMeat::CEnmuMeat(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject(pDevice, pContext)
	, m_fHp(100.f)
	, m_fMaxHp(100.f)
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


	ChangeState(new BossIdle(TEXT("Idle")));

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

	return S_OK;
}

void CEnmuMeat::Priority_Update(_float fTimeDelta)
{
	if (!m_pTarget)
	{
		m_pTarget = static_cast<CBaseCharacter*>(m_pGameInstance->Find_GameObjectByName(ToIndex(LEVEL::ENMU_BOSS), TEXT("Kyojuro")));

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
	for (auto& child : m_vecChildren)
	{
		if (child->IsActive())
		{
			child->Late_Update(fTimeDelta);
		}
	}
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
	pTentacle->GetTransform()->LookAtXZ(vPos);
	pTentacle->GetTransform()->RotateToDirection(vDir);
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

HRESULT CEnmuMeat::Ready_Parts()
{

	CEnmuArm::ARM_DESC armDesc;
	armDesc.sModelKey = TEXT("Prototype_Component_Model_EnmuLeftArm");

	AddChild(CEnmuArm::Create(m_pDevice, m_pContext, &armDesc));
	armDesc.sModelKey = TEXT("Prototype_Component_Model_EnmuRightArm");
	AddChild(CEnmuArm::Create(m_pDevice, m_pContext, &armDesc));
	AddChild(CEnmuBody::Create(m_pDevice, m_pContext));
	AddChild(CEnmuHead::Create(m_pDevice, m_pContext));

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


