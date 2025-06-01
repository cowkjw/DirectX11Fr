#include "ThirdPersonCamera.h"
#include "GameInstance.h"	

CThirdPersonCamera::CThirdPersonCamera(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCamera(pDevice, pContext)
	, m_fSmooth(0.f)
	, m_vOffset(0.f, 0.f, 0.f)
	, m_pTarget(nullptr)
	, m_pTargetTransform(nullptr)
{

}

CThirdPersonCamera::CThirdPersonCamera(const CThirdPersonCamera& Prototype)
	: CCamera(Prototype)
	, m_fSmooth(Prototype.m_fSmooth)
	, m_vOffset(Prototype.m_vOffset)
	, m_pTarget(Prototype.m_pTarget)
	, m_pTargetTransform(Prototype.m_pTargetTransform)
{
	Safe_AddRef(m_pTransformCom);
	Safe_AddRef(m_pTarget);
}

HRESULT CThirdPersonCamera::Initialize_Prototype()
{
	m_vOffset = _float3(10.f, 15.f, -50.f); 
	m_fSmooth = 60.f; // 카메라 움직임 스무스 정도
	return S_OK;
}

HRESULT CThirdPersonCamera::Initialize(void* pArg)
{
	if (pArg == nullptr)
	{
		return E_FAIL;
	}
	THRIDCAMERA_DESC* pDesc = reinterpret_cast<THRIDCAMERA_DESC*>(pArg);
	m_fSmooth = pDesc->fSmoth;
	m_pTarget = pDesc->pTarget;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (m_pTarget == nullptr)
	{
		MSG_BOX("CThirdPersonCamera::Initialize - Target is null");
		return E_FAIL;
	}

	m_pTargetTransform = m_pTarget->GetTransform();

	m_pLockOnTarget = m_pGameInstance->Find_GameObjectByName(ToIndex(LEVEL::GAMEPLAY), TEXT("Akaza"));

	return S_OK;
}

void CThirdPersonCamera::Priority_Update(_float fTimeDelta)
{
}

void CThirdPersonCamera::Update(_float fTimeDelta)
{
	__super::Update_Camera();
}
void CThirdPersonCamera::Late_Update(_float fTimeDelta)
{
	//if (!m_pTargetTransform)
	//	return;

	XMVECTOR playerPos = m_pTargetTransform->Get_State(STATE::POSITION);
	XMVECTOR enemyPos = m_pLockOnTarget
		? m_pLockOnTarget->GetTransform()->Get_State(STATE::POSITION)
		: playerPos;
	XMVECTOR midPos = XMVectorLerp(playerPos, enemyPos, 0.5f);

	// 2) 오프셋 회전 → 원하는 카메라 위치
	XMVECTOR targetQuat = m_pTargetTransform->Get_RotationQuaternion();
	XMVECTOR offsetV = XMLoadFloat3(&m_vOffset);
	XMVECTOR worldOff = RotateVectorByQuaternion(offsetV, targetQuat);
	XMVECTOR desiredCamPos = playerPos + worldOff;

	// 3) 뷰 매트릭스 계산 (camera at 원하는 위치, lookAt 중간 지점)
	XMVECTOR camUp = m_pTransformCom->Get_State(STATE::UP);
	XMMATRIX view = XMMatrixLookAtLH(desiredCamPos, midPos, camUp);

	// 4) 데드존 보정
	const float deadZoneX = 1000.f, deadZoneY = 2000.0f;
	XMVECTOR viewPos = XMVector3TransformCoord(playerPos, view);
	float    vx = XMVectorGetX(viewPos);
	float    vy = XMVectorGetY(viewPos);
	XMVECTOR camRight = m_pTransformCom->Get_State(STATE::RIGHT);
	if (vx > deadZoneX) desiredCamPos += camRight * (vx - deadZoneX);
	else if (vx < -deadZoneX) desiredCamPos += camRight * (vx + deadZoneX);
	if (vy > deadZoneY) desiredCamPos += camUp * (vy - deadZoneY);
	else if (vy < -deadZoneY) desiredCamPos += camUp * (vy + deadZoneY);

	// 5) 보간으로 최종 위치
	float    t = m_fSmooth * fTimeDelta;
	t = min(t, 1.0f);
	XMVECTOR currCamPos = m_pTransformCom->Get_State(STATE::POSITION);
	XMVECTOR lerpPos = XMVectorLerp(currCamPos, desiredCamPos, t);
	m_pTransformCom->Set_State(STATE::POSITION, lerpPos);

	// 6) 중간 지점 바라보게
	m_pTransformCom->LookAtXZ(midPos);

	//if (!m_pTargetTransform)
	//	return;

	//// 1) 플레이어 위치와 락온 대상(또는 플레이어) 위치 중간 지점 계산
	//XMVECTOR playerPos = m_pTargetTransform->Get_State(STATE::POSITION);
	//XMVECTOR enemyPos = m_pLockOnTarget
	//	? m_pLockOnTarget->GetTransform()->Get_State(STATE::POSITION)
	//	: playerPos;
	//XMVECTOR midPos = XMVectorLerp(playerPos, enemyPos, 0.5f);

	//// 2) 로컬 오프셋을 회전(quaternion) 적용해서 XZ 오프셋만 구함
	//XMVECTOR targetQuat = m_pTargetTransform->Get_RotationQuaternion();
	//XMVECTOR offsetV = XMLoadFloat3(&m_vOffset);

	//// Y 컴포넌트는 나중에 별도로 설정하므로 0으로
	//offsetV = XMVectorSetY(offsetV, 0.f);
	//XMVECTOR worldOff = XMVector3Rotate(offsetV, targetQuat);

	//// 3) 원하는 카메라 위치 (XZ는 회전 오프셋, Y는 플레이어 Y + m_vOffset.y 고정)
	//XMVECTOR desiredCamPos = playerPos + worldOff;
	//float   fixedY = XMVectorGetY(playerPos) + m_vOffset.y;
	//desiredCamPos = XMVectorSetY(desiredCamPos, fixedY);

	//// 4) 뷰 매트릭스 계산 (보정 전 테스트용, 필요에 따라 사용)
	//// XMVECTOR camUp = m_pTransformCom->Get_State(STATE::UP);
	//// XMMATRIX view = XMMatrixLookAtLH(desiredCamPos, midPos, camUp);

	//// 5) 데드존 보정 (XZ 평면 기준)
	//const float deadZoneX = 1000.f;
	//const float deadZoneY = 2000.f;
	//XMVECTOR viewPos = XMVector3TransformCoord(playerPos,
	//	XMMatrixLookAtLH(desiredCamPos, midPos,
	//		m_pTransformCom->Get_State(STATE::UP)));
	//float vx = XMVectorGetX(viewPos);
	//float vy = XMVectorGetY(viewPos);

	//XMVECTOR camRight = m_pTransformCom->Get_State(STATE::RIGHT);
	//XMVECTOR camUp = m_pTransformCom->Get_State(STATE::UP);

	//if (vx > deadZoneX) desiredCamPos += camRight * (vx - deadZoneX);
	//if (vx < -deadZoneX) desiredCamPos += camRight * (vx + deadZoneX);
	//if (vy > deadZoneY) desiredCamPos += camUp * (vy - deadZoneY);
	//if (vy < -deadZoneY) desiredCamPos += camUp * (vy + deadZoneY);

	//// Y는 여전히 고정
	//desiredCamPos = XMVectorSetY(desiredCamPos, fixedY);

	//// 6) 최종 보간 (XZ 평면 보간, Y 고정)
	//float t = m_fSmooth * fTimeDelta;
	//if (t > 1.f) t = 1.f;

	//XMVECTOR currPos = m_pTransformCom->Get_State(STATE::POSITION);
	//// XZ는 lerp, Y는 fixedY
	//XMVECTOR lerpPos = XMVectorLerp(currPos, desiredCamPos, t);
	//XMVECTOR finalPos = XMVectorSetY(lerpPos, fixedY);

	//m_pTransformCom->Set_State(STATE::POSITION, finalPos);

	//// 7) 카메라가 바라보는 지점의 Y도 플레이어 높이로 맞춰주면 수평 유지
	//midPos = XMVectorSetY(midPos, XMVectorGetY(playerPos));
	//m_pTransformCom->LookAtXZ(playerPos);
}

//void CThirdPersonCamera::Late_Update(_float fTimeDelta)
//{
//	if (!m_pTargetTransform)
//		return;
//
//	// 1) 플레이어 월드 위치·쿼터니언 회전
//	XMVECTOR targetPos = m_pTargetTransform->Get_State(STATE::POSITION);
//	XMVECTOR targetQuat = m_pTargetTransform->Get_RotationQuaternion();
//
//	// 2) 기본 오프셋 회전 적용 → 목표 카메라 월드 위치
//	XMVECTOR offsetV = XMLoadFloat3(&m_vOffset);
//	XMVECTOR worldOff = RotateVectorByQuaternion(offsetV, targetQuat);
//	XMVECTOR desiredCamPos = XMVectorAdd(targetPos, worldOff);
//
//	// 3) 데드존 크기 (뷰 공간에서의 월드 단위)
//	const float deadZoneX = 200.0f;   // 좌우  ±5 유닛
//	const float deadZoneY = 200.0f;   // 상하  ±2 유닛
//
//	// 4) 뷰 공간으로 변환 (월드 → 카메라 기준 좌표)
//	XMMATRIX view = m_pGameInstance->Get_Transform_Matrix(TRANSFORM::VIEW);
//	XMVECTOR viewPos = XMVector3TransformCoord(targetPos, view);
//	float vx = XMVectorGetX(viewPos);
//	float vy = XMVectorGetY(viewPos);
//
//	// 5) 뷰 축 (카메라 우, 상 방향) 가져오기
//	XMVECTOR camRight = m_pTransformCom->Get_State(STATE::RIGHT);
//	XMVECTOR camUp = m_pTransformCom->Get_State(STATE::UP);
//
//	// 6) 데드존 넘은 만큼만 카메라 목표 위치 보정
//	if (vx > deadZoneX)
//		desiredCamPos += camRight * (vx - deadZoneX);
//	if (vx < -deadZoneX) 
//		desiredCamPos += camRight * (vx + deadZoneX);
//	if (vy > deadZoneY) 
//		desiredCamPos += camUp * (vy - deadZoneY);
//	if (vy < -deadZoneY)
//		desiredCamPos += camUp * (vy + deadZoneY);
//
//	// 7) 현재 카메라 위치 → 부드럽게 보간
//	_float t = m_fSmooth * fTimeDelta;
//	t = (t > 1.f ? 1.f : t);
//	XMVECTOR currCamPos = m_pTransformCom->Get_State(STATE::POSITION);
//	XMVECTOR lerpPos = XMVectorLerp(currCamPos, desiredCamPos, t);
//	m_pTransformCom->Set_State(STATE::POSITION, lerpPos);
//
//
//	CGameObject* pCharacter = m_pGameInstance->Find_GameObjectByName(ToIndex(LEVEL::GAMEPLAY), TEXT("BaseCharacter"));
//
//	if (pCharacter)
//	{
//		m_pTargetTransform = pCharacter->GetTransform();
//		targetPos = m_pTargetTransform->Get_State(STATE::POSITION);
//		m_pTransformCom->LookAt(targetPos);
//	}
//	// 8) 항상 플레이어 바라보게
//	
//}

CThirdPersonCamera* CThirdPersonCamera::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CThirdPersonCamera* pInstance = new CThirdPersonCamera(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CThirdPersonCamera");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CThirdPersonCamera::Clone(void* pArg)
{
	CThirdPersonCamera* pInstance = new CThirdPersonCamera(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CThirdPersonCamera");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CThirdPersonCamera::Free()
{
	__super::Free();
	Safe_Release(m_pTargetTransform);
	Safe_Release(m_pTarget);
}
