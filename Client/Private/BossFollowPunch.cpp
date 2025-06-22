#include "BossFollowPunch.h"
#include "BossIdle.h"
#include <EnmuArm.h>

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

		m_pLeftShoulderBone = pModel->Get_Bone("L_TentaclesArmA_1");
		m_pLeftElbowBone = pModel->Get_Bone("L_TentaclesArmA_4");
		m_pLeftWristBone = pModel->Get_Bone("L_TentaclesArmA_7");
	}

	pModel = static_cast<CModel*>(rightArm->Get_Component(TEXT("Com_Model")));


	if (pModel)
	{
		m_pRightArmBone = pModel->Get_Bone("R_TentaclesArmA_7");
		m_pRightShoulderBone = pModel->Get_Bone("R_TentaclesArmA_1");
		m_pRightElbowBone = pModel->Get_Bone("R_TentaclesArmA_4");
		m_pRightWristBone = pModel->Get_Bone("R_TentaclesArmA_7");
	}
	


	// 팔 길이 계산 (한 번만 계산)
	CalculateArmLengths(pChar);
	pChar->SetState(EnmuState::FOLLOWPUNCH);

	m_pWarningZone = CGameInstance::Get_Instance()->Add_GameObject(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_GameObject_WarningZone"), ToIndex(LEVEL::ENMU_BOSS)
		, TEXT("WarningZone"));

	if (m_pWarningZone)
	{
		m_pWarningZone->GetTransform()->Scaling(_float3(40.f, 40.f, 40.f));
	}
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
	_vector rawDir = XMVector3Normalize(vTargetPos - vMyPos);
	// 오른팔 기준 처리
	if (!m_bAttackedRight && m_fTimeElapsed < RIGHTARM_START_TIME)
	{
		_vector vRightArmPos = rightArm->GetTransform()->Get_State(STATE::POSITION);
		_vector vTargetPos = pChar->GetTarget()->GetTransform()->Get_State(STATE::POSITION);
		_vector rawDir = XMVector3Normalize(vTargetPos - vRightArmPos);

		rightArm->GetTransform()->RotateToDirection(XMVector3Normalize(rawDir));

		_vector warningDir = XMVector3Normalize(rawDir);
		_vector warningPos = vRightArmPos + warningDir * 50.f;
		warningPos = XMVectorSetY(warningPos, -18.f);
		m_pWarningZone->GetTransform()->Set_State(STATE::POSITION, warningPos);
	
	}
	else if (!m_bAttackedRight && m_fTimeElapsed >= RIGHTARM_START_TIME)
	{
		m_pLeftArmBone->Set_ExtraMatrix(ComputeMatrix(pChar, false));
		pAnimatorRight->SetTrigger("FollowPunch");
		pAnimatorLeft->SetTrigger("FollowPunchStart");
		m_bAttackedRight = true;
		m_fTimeElapsed = 0.f;
	}

	// 왼팔 기준 처리
	if (m_bAttackedRight && !m_bAttackedLeft && m_fTimeElapsed < LEFTARM_START_TIME)
	{
		_vector vLeftArmPos = leftArm->GetTransform()->Get_State(STATE::POSITION);
		_vector vTargetPos = pChar->GetTarget()->GetTransform()->Get_State(STATE::POSITION);
		_vector rawDir = XMVector3Normalize(vTargetPos - vLeftArmPos);

		leftArm->GetTransform()->RotateToDirection(XMVector3Normalize(rawDir));
	}
	else if (!m_bAttackedLeft && m_fTimeElapsed >= LEFTARM_START_TIME)
	{
		pAnimatorLeft->SetTrigger("FollowPunch");
		m_bAttackedLeft = true;
	}

	if (m_fTimeElapsed >= ATTACK_END_TIME)
	{
		pChar->ChangeState(new BossIdle(TEXT("Idle")));
	}
}

void BossFollowPunch::Exit(CEnmuMeat* pChar)
{
	auto leftArm = dynamic_cast<CEnmuArm*>(pChar->GetPart(CEnmuMeat::Parts::LEFTARM));
	auto rightArm = dynamic_cast<CEnmuArm*>(pChar->GetPart(CEnmuMeat::Parts::RIGHTARM));

	leftArm->GetTransform()->Rotate_EulerAngles(_float3(0.f, 180.f, 0.f)); // 기본 방향으로 회전
	rightArm->GetTransform()->Rotate_EulerAngles(_float3(0.f, 180.f, 0.f)); // 기본 방향으로 회전


}


_matrix BossFollowPunch::ComputeMatrix(CEnmuMeat* pChar, _bool bIsLeft)
{
	auto pBone = bIsLeft ? m_pLeftArmBone : m_pRightArmBone;
	auto armPart = bIsLeft ? pChar->GetPart(CEnmuMeat::Parts::LEFTARM) :
		pChar->GetPart(CEnmuMeat::Parts::RIGHTARM);

	_matrix boneLocal = XMLoadFloat4x4(pBone->Get_CombinedTransformationMatrix());
	_matrix armLocal = XMLoadFloat4x4(&armPart->GetTransform()->Get_WorldMatrix());
	_matrix boneWorldMatrix = XMMatrixMultiply(boneLocal, armLocal);

	XMVECTOR vBoneWorldPos = XMVector3Transform(XMVectorSet(0, 0, 0, 1), boneWorldMatrix);
	XMVECTOR vTargetPos = pChar->GetTarget()->GetTransform()->Get_State(STATE::POSITION);

	XMVECTOR vDelta = vTargetPos - vBoneWorldPos;
	XMVECTOR vDirNorm = XMVector3Normalize(vDelta);

	float dx = XMVectorGetX(vDirNorm);
	float dy = XMVectorGetY(vDirNorm);
	float dz = XMVectorGetZ(vDirNorm);
	auto leftArm = dynamic_cast<CEnmuArm*>(pChar->GetPart(CEnmuMeat::Parts::LEFTARM));
	auto rightArm = dynamic_cast<CEnmuArm*>(pChar->GetPart(CEnmuMeat::Parts::RIGHTARM));

	float horizontalDist = sqrtf(dx * dx + dz * dz);
	float yaw = atan2f(dx, dz);
	float pitch = atan2f(dy, horizontalDist);
	float roll = atan2f(dz, dx); 

	if (pitch < 0.f)
	{
		pitch += 2 * XM_PI; 
	}
	if (roll < 0.f)
	{
		roll += 2 * XM_PI;
	}
	//leftArm->GetTransform()->Rotate_EulerAngles(_float3(0.f, 180.f, 0.f)); 
	//rightArm->GetTransform()->Rotate_EulerAngles(_float3(0.f, 180.f, 0.f)); 

	_matrix rotM = XMMatrixRotationRollPitchYaw(pitch, 0.f, roll);
	return rotM;

}

void BossFollowPunch::CalculateArmLengths(CEnmuMeat* pChar)
{
	// 왼팔 길이 계산
	if (m_pLeftShoulderBone && m_pLeftElbowBone && m_pLeftWristBone)
	{
		auto leftArm = pChar->GetPart(CEnmuMeat::Parts::LEFTARM);
		_matrix armMatrix = XMLoadFloat4x4(&leftArm->GetTransform()->Get_WorldMatrix());

		_matrix shoulderMatrix = XMMatrixMultiply(XMLoadFloat4x4(m_pLeftShoulderBone->Get_CombinedTransformationMatrix()), armMatrix);
		_matrix elbowMatrix = XMMatrixMultiply(XMLoadFloat4x4(m_pLeftElbowBone->Get_CombinedTransformationMatrix()), armMatrix);
		_matrix wristMatrix = XMMatrixMultiply(XMLoadFloat4x4(m_pLeftWristBone->Get_CombinedTransformationMatrix()), armMatrix);

		_vector shoulderPos = XMVector3Transform(XMVectorSet(0, 0, 0, 1), shoulderMatrix);
		_vector elbowPos = XMVector3Transform(XMVectorSet(0, 0, 0, 1), elbowMatrix);
		_vector wristPos = XMVector3Transform(XMVectorSet(0, 0, 0, 1), wristMatrix);

		m_fLeftUpperArmLength = XMVectorGetX(XMVector3Length(elbowPos - shoulderPos));
		m_fLeftForearmLength = XMVectorGetX(XMVector3Length(wristPos - elbowPos));
	}

	// 오른팔 길이 계산
	if (m_pRightShoulderBone && m_pRightElbowBone && m_pRightWristBone)
	{
		auto rightArm = pChar->GetPart(CEnmuMeat::Parts::RIGHTARM);
		_matrix armMatrix = XMLoadFloat4x4(&rightArm->GetTransform()->Get_WorldMatrix());

		_matrix shoulderMatrix = XMMatrixMultiply(XMLoadFloat4x4(m_pRightShoulderBone->Get_CombinedTransformationMatrix()), armMatrix);
		_matrix elbowMatrix = XMMatrixMultiply(XMLoadFloat4x4(m_pRightElbowBone->Get_CombinedTransformationMatrix()), armMatrix);
		_matrix wristMatrix = XMMatrixMultiply(XMLoadFloat4x4(m_pRightWristBone->Get_CombinedTransformationMatrix()), armMatrix);

		_vector shoulderPos = XMVector3Transform(XMVectorSet(0, 0, 0, 1), shoulderMatrix);
		_vector elbowPos = XMVector3Transform(XMVectorSet(0, 0, 0, 1), elbowMatrix);
		_vector wristPos = XMVector3Transform(XMVectorSet(0, 0, 0, 1), wristMatrix);

		m_fRightUpperArmLength = XMVectorGetX(XMVector3Length(elbowPos - shoulderPos));
		m_fRightForearmLength = XMVectorGetX(XMVector3Length(wristPos - elbowPos));
	}
}

void BossFollowPunch::ApplyIK(CEnmuMeat* pChar, _bool bIsLeft, _vector vTargetPos)
{
	auto armPart = bIsLeft ? pChar->GetPart(CEnmuMeat::Parts::LEFTARM) :
		pChar->GetPart(CEnmuMeat::Parts::RIGHTARM);

	auto shoulderBone = bIsLeft ? m_pLeftShoulderBone : m_pRightShoulderBone;
	auto elbowBone = bIsLeft ? m_pLeftElbowBone : m_pRightElbowBone;
	auto wristBone = bIsLeft ? m_pLeftWristBone : m_pRightWristBone;

	float upperArmLength = bIsLeft ? m_fLeftUpperArmLength : m_fRightUpperArmLength;
	float forearmLength = bIsLeft ? m_fLeftForearmLength : m_fRightForearmLength;

	if (!shoulderBone || !elbowBone || !wristBone) return;

	// 어깨 위치 계산
	_matrix armMatrix = XMLoadFloat4x4(&armPart->GetTransform()->Get_WorldMatrix());
	_matrix shoulderMatrix = XMMatrixMultiply(XMLoadFloat4x4(shoulderBone->Get_CombinedTransformationMatrix()), armMatrix);
	_vector shoulderPos = XMVector3Transform(XMVectorSet(0, 0, 0, 1), shoulderMatrix);

	// Two-Bone IK 계산
	TwoBoneIKResult ikResult = CalculateTwoBoneIK(shoulderPos, vTargetPos, upperArmLength, forearmLength);

	// 계산된 회전을 본에 적용
	shoulderBone->Set_ExtraMatrix(ikResult.shoulderRotation);
	elbowBone->Set_ExtraMatrix(ikResult.elbowRotation);
	wristBone->Set_ExtraMatrix(ikResult.wristRotation);
}

BossFollowPunch::TwoBoneIKResult BossFollowPunch::CalculateTwoBoneIK(_vector shoulderPos, _vector targetPos, float upperArmLength, float forearmLength)
{
	TwoBoneIKResult result;

	// 어깨에서 타겟까지의 거리
	_vector toTarget = targetPos - shoulderPos;
	float targetDistance = XMVectorGetX(XMVector3Length(toTarget));
	_vector targetDirection = XMVector3Normalize(toTarget);

	// 팔 전체 길이
	float totalArmLength = upperArmLength + forearmLength;

	// 거리가 너무 멀면 팔을 완전히 펴기
	if (targetDistance >= totalArmLength)
	{
		// 어깨 회전: 타겟 방향으로
		_vector forward = XMVectorSet(0, 0, 1, 0);
		_vector up = XMVectorSet(0, 1, 0, 0);
		result.shoulderRotation = LookRotation(targetDirection, up);

		// 팔꿈치는 거의 펴기 (약간의 각도만)
		result.elbowRotation = XMMatrixRotationX(XMConvertToRadians(10.0f));

		// 손목은 기본 상태
		result.wristRotation = XMMatrixIdentity();

		return result;
	}

	// 거리가 너무 가까우면 팔을 많이 굽히기
	if (targetDistance <= abs(upperArmLength - forearmLength))
	{
		result.shoulderRotation = LookRotation(targetDirection, XMVectorSet(0, 1, 0, 0));
		result.elbowRotation = XMMatrixRotationX(XMConvertToRadians(150.0f));
		result.wristRotation = XMMatrixIdentity();
		return result;
	}

	// 코사인 법칙을 사용해서 팔꿈치 각도 계산
	float cosElbowAngle = (upperArmLength * upperArmLength + forearmLength * forearmLength - targetDistance * targetDistance)
		/ (2.0f * upperArmLength * forearmLength);

	// 값 범위 제한
	cosElbowAngle = max(-1.0f, min(1.0f, cosElbowAngle));
	float elbowAngle = acosf(cosElbowAngle);

	// 어깨 각도 계산
	float cosShoulderAngle = (upperArmLength * upperArmLength + targetDistance * targetDistance - forearmLength * forearmLength)
		/ (2.0f * upperArmLength * targetDistance);
	cosShoulderAngle = max(-1.0f, min(1.0f, cosShoulderAngle));
	float shoulderAngle = acosf(cosShoulderAngle);

	// 어깨 회전 계산
	_vector up = XMVectorSet(0, 1, 0, 0);
	_matrix baseRotation = LookRotation(targetDirection, up);
	_matrix shoulderAdjustment = XMMatrixRotationAxis(XMVector3Cross(targetDirection, up), shoulderAngle);
	result.shoulderRotation = XMMatrixMultiply(shoulderAdjustment, baseRotation);

	// 팔꿈치 회전 계산 (내각이므로 180도에서 빼기)
	float elbowRotationAngle = XM_PI - elbowAngle;
	result.elbowRotation = XMMatrixRotationX(elbowRotationAngle);

	// 손목은 타겟을 향하도록
	result.wristRotation = XMMatrixIdentity();

	return result;
}

_matrix BossFollowPunch::LookRotation(_vector direction, _vector up)
{
	_vector forward = XMVector3Normalize(direction);
	_vector right = XMVector3Normalize(XMVector3Cross(up, forward));
	_vector realUp = XMVector3Cross(forward, right);

	_matrix rotMatrix;
	rotMatrix.r[0] = XMVectorSetW(right, 0.0f);
	rotMatrix.r[1] = XMVectorSetW(realUp, 0.0f);
	rotMatrix.r[2] = XMVectorSetW(forward, 0.0f);
	rotMatrix.r[3] = XMVectorSet(0, 0, 0, 1);

	return rotMatrix;
}