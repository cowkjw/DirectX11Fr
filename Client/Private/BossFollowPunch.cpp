#include "BossFollowPunch.h"
#include "BossIdle.h"

void BossFollowPunch::Enter(CEnmuMeat* pChar)
{
	auto leftArm = pChar->GetPart(CEnmuMeat::Parts::LEFTARM);
	auto rightArm = pChar->GetPart(CEnmuMeat::Parts::RIGHTARM);
	auto pAnimatorLeft = leftArm->Get_Animator();
	auto pAnimatorRight = rightArm->Get_Animator();



	pAnimatorRight->SetTrigger("FollowPunchStart");

	auto pModel = static_cast<CModel*>(leftArm->Get_Component(TEXT("Com_Model")));

	if (pModel)
	{
		m_pLeftArmBone = pModel->Get_Bone("L_TentaclesArmA_7");
	}

	pModel = static_cast<CModel*>(rightArm->Get_Component(TEXT("Com_Model")));


	if (pModel)
	{
		m_pRightArmBone = pModel->Get_Bone("R_TentaclesArmA_7");
	}

	pChar->SetState(EnmuState::FOLLOWPUNCH);
}

void BossFollowPunch::Update(CEnmuMeat* pChar, _float fTimeDelta)
{
	auto leftArm = pChar->GetPart(CEnmuMeat::Parts::LEFTARM);
	auto rightArm = pChar->GetPart(CEnmuMeat::Parts::RIGHTARM);
	auto pAnimatorLeft = leftArm->Get_Animator();
	auto pAnimatorRight = rightArm->Get_Animator();
	m_fTimeElapsed += fTimeDelta;

	_vector vTargetPos = pChar->GetTarget()->GetTransform()->Get_State(STATE::POSITION);
	_vector vMyPos = pChar->GetTransform()->Get_State(STATE::POSITION);
	_vector vDir = XMVector3Normalize(vTargetPos - vMyPos);
	// 아직 오른쪽 공격 안했고 공격 시간까지 안됐으면 일단 따라다니게
	if (!m_bAttackedRight && m_fTimeElapsed < RIGHTARM_START_TIME)
	{
			rightArm->GetTransform()->RotateToDirection(vDir);
		
	}
	else if (!m_bAttackedRight && m_fTimeElapsed >= RIGHTARM_START_TIME)
	{
		pAnimatorRight->SetTrigger("FollowPunch");
		pAnimatorLeft->SetTrigger("FollowPunchStart");// 왼쪽도 이제 시작하게
		m_bAttackedRight = true;
		m_fTimeElapsed = 0.f; // 오른쪽 공격 후 시간 초기화
	}
	
	if (m_bAttackedRight&&!m_bAttackedLeft && m_fTimeElapsed < LEFTARM_START_TIME)
	{
		leftArm->GetTransform()->RotateToDirection(vDir);

	}
	else if (!m_bAttackedLeft && m_fTimeElapsed >= LEFTARM_START_TIME)	
	{
	
		pAnimatorLeft->SetTrigger("FollowPunch");// 왼쪽도 이제 시작하게
		m_bAttackedLeft = true;
	}

	if (m_fTimeElapsed >= ATTACK_END_TIME)
	{
		pChar->ChangeState(new BossIdle(TEXT("Idle")));
	}
}

void BossFollowPunch::Exit(CEnmuMeat* pChar)
{
	auto leftArm = pChar->GetPart(CEnmuMeat::Parts::LEFTARM);
	auto rightArm = pChar->GetPart(CEnmuMeat::Parts::RIGHTARM);

	leftArm->GetTransform()->RotateToDirection(XMVectorSet(0.f, 0.f, 1.f, 0.f)); // 기본 방향으로 회전
	rightArm->GetTransform()->RotateToDirection(XMVectorSet(0.f, 0.f, 1.f, 0.f)); // 기본 방향으로 회전
}

_matrix BossFollowPunch::ComputeMatrix(CEnmuMeat* pChar, _bool bIsLeft)
{
	auto pBone = bIsLeft ? m_pLeftArmBone : m_pRightArmBone;
	auto armPart = bIsLeft ? pChar->GetPart(CEnmuMeat::Parts::LEFTARM) :
		pChar->GetPart(CEnmuMeat::Parts::RIGHTARM);

	// 1) 뼈의 실제 월드 위치 계산
	_matrix boneLocal = XMLoadFloat4x4(pBone->Get_CombinedTransformationMatrix());
	_matrix armLocal = XMLoadFloat4x4(&armPart->GetTransform()->Get_WorldMatrix());
	_matrix boneWorldMatrix = XMMatrixMultiply(boneLocal, armLocal);

	XMVECTOR vBoneWorldPos = XMVector3Transform(XMVectorSet(0, 0, 0, 1), boneWorldMatrix);
	XMVECTOR vTargetPos = pChar->GetTarget()->GetTransform()->Get_State(STATE::POSITION);

	// 2) 뼈 위치에서 타겟으로의 방향 계산
	XMVECTOR vDelta = vTargetPos - vBoneWorldPos;
	XMVECTOR vDirNorm = XMVector3Normalize(vDelta);

	// 나머지는 동일...
	float dx = XMVectorGetX(vDirNorm);
	float dy = XMVectorGetY(vDirNorm);
	float dz = XMVectorGetZ(vDirNorm);

	float horizontalDist = sqrtf(dx * dx + dz * dz);
	float yaw = atan2f(dx, dz);
	float pitch = atan2f(dy, horizontalDist);
	float roll = atan2f(dz, dx); // roll은 z축 회전

	if (pitch < 0.f)
	{
		pitch += 2 * XM_PI; // pitch를 0~2π 범위로 조정
	}
	if (roll < 0.f)
	{
		roll += 2 * XM_PI; // roll을 0~2π 범위로 조정
	}

	_matrix rotM = XMMatrixRotationRollPitchYaw(pitch, 0.f, roll);
	return rotM;
}
