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
    Safe_AddRef(m_pTarget);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (m_pTarget == nullptr)
	{
		MSG_BOX("CThirdPersonCamera::Initialize - Target is null");
		return E_FAIL;
	}

	m_pTargetTransform = m_pTarget->GetTransform();

	m_pLockOnTarget = m_pGameInstance->Find_GameObjectByName(ToIndex(LEVEL::GAMEPLAY), TEXT("Akaza"));
   /* if (m_pLockOnTarget == nullptr)
    {
		m_pLockOnTarget = m_pGameInstance->Find_GameObjectByName(ToIndex(LEVEL::ENMU_BOSS), TEXT("EnmuMeat"));
    }*/
    InitializeCameraPosition();
	/*if (!m_pLockOnTarget)
		m_pLockOnTarget = m_pGameInstance->Find_GameObjectByName(ToIndex(LEVEL::ENMU_BOSS), TEXT("EnmuMeat"));*/

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

    if (!m_pTargetTransform )
        return;

    if (m_bHitCam && m_pHitAttacker && m_pHitTarget)
    {
     
        m_fHitTime += fTimeDelta;
        // 두 캐릭터 중간 위치 계산
        _vector posA = m_pHitAttacker->GetTransform()->Get_State(STATE::POSITION);
        _vector posB = m_pHitTarget->GetTransform()->Get_State(STATE::POSITION);
        _vector mid = XMVectorLerp(posA, posB, 0.5f);

        // 카메라 위치는 중간 위치 + 지정된 오프셋
        _vector desired = mid + m_vHitOffset;
        // 고정 높이 사용하기
        desired = XMVectorSetY(desired, m_fFixedHeight);

        _vector curr = m_pTransformCom->Get_State(STATE::POSITION);
        _float t = min(1.f, m_cameraSmooth * fTimeDelta);
        _vector lerpPos = XMVectorLerp(curr, desired, t);


        if (m_bShaking)
        {
            _float t = m_fShakeTimeLeft / m_fShakeDuration;

            // 진폭을 시간이 지날수록 점점 줄이기
            _float currentAmp = m_fShakeAmplitude * t;

            // 랜덤 단위 벡터에 진폭 곱하기
            _float offX = m_pGameInstance->Compute_Random(-1.f, 1.f) * currentAmp;
            _float offY = m_pGameInstance->Compute_Random(-1.f, 1.f) * currentAmp;
            _float offZ = m_pGameInstance->Compute_Random(-1.f, 1.f) * currentAmp;
            if (m_pLockOnTarget && m_pLockOnTarget != m_pTarget)
            {
                lerpPos = XMVectorAdd(lerpPos,
                    XMVectorSet(offX, offY, offZ, 0.f));
            }
            else
            {

                lerpPos = XMVectorAdd(m_vHitOffset,
                    XMVectorSet(offX, offY, offZ, 0.f));
            }

            // 타이머 감소
            m_fShakeTimeLeft -= fTimeDelta;
            if (m_fShakeTimeLeft <= 0.f)
                m_bShaking = false;

        }
        m_pTransformCom->Set_State(STATE::POSITION, lerpPos);

        // 바라볼 지점도 중간으로
        _vector lookAt = XMVectorSetY(mid, XMVectorGetY(mid) + 3.f);
        if (m_pLockOnTarget && m_pLockOnTarget != m_pTarget)
             m_pTransformCom->LookAtXZ(lookAt);
        else
        {
			auto vCurPos = m_pTransformCom->Get_State(STATE::POSITION);
			_float fLength = XMVectorGetX(XMVector3Length(m_vHitOffset - vCurPos));

            if (fLength >= 1.f)
            {
                lerpPos = XMVectorLerp(vCurPos, m_vHitOffset, 0.2f);
            }
            // 위치는 오프셋으로 두고 중앙 지점 바라보기
            m_pTransformCom->Set_State(STATE::POSITION, lerpPos/* m_bShaking ? lerpPos : m_vHitOffset*/);
			m_pTransformCom->LookAt(mid);
        }
        if (m_fHitTime >= m_fHitDuration)
        {
            m_bHitCam = false;
            Safe_Release(m_pHitAttacker);
            Safe_Release(m_pHitTarget);
        }
        return; 
    }

    _vector currentCamPos = m_pTransformCom->Get_State(STATE::POSITION);
    if (m_pLockOnTarget && m_pLockOnTarget != m_pTarget)
    {
        _vector playerPos = m_pTargetTransform->Get_State(STATE::POSITION);
        _vector enemyPos = m_pLockOnTarget->GetTransform()->Get_State(STATE::POSITION);

        // 1. 두 캐릭터의 중점과 거리 계산
        _vector midPoint = XMVectorLerp(playerPos, enemyPos, 0.5f);
        _float distance = XMVectorGetX(XMVector3Length(enemyPos - playerPos));

        // 고정된 카메라 높이와 기본 거리 설정
        //_float fixedCameraHeight = 15.0f;  // 고정된 Y 위치
        _float baseCameraDistance = max(m_fMinCameraDistance, distance * m_fDistanceMultiplier); // 거리에 따른 기본 카메라 거리

        // 현재 카메라 위치
        _vector currentCamPos = m_pTransformCom->Get_State(STATE::POSITION);

        //데드존 설정 (화면상의 영역)
        _float deadZoneWidth = 100.f;   // 좌우 데드존
        _float deadZoneHeight = 10.0f;  // 상하 데드존

        // 두 캐릭터를 모두 포함하는 바운딩 박스 계산
        _vector boundsMin = XMVectorMin(playerPos, enemyPos);
        _vector boundsMax = XMVectorMax(playerPos, enemyPos);
        _vector boundsCenter = (boundsMin + boundsMax) * 0.5f;

        // Y는 고정
        boundsCenter = XMVectorSetY(boundsCenter, XMVectorGetY(midPoint));

        // 바운딩 박스 크기에 따른 카메라 거리 조정
        _float boundsWidth = XMVectorGetX(boundsMax) - XMVectorGetX(boundsMin);
        _float boundsDepth = XMVectorGetZ(boundsMax) - XMVectorGetZ(boundsMin);
        _float requiredDistance = max(boundsWidth, boundsDepth) * 0.3f + baseCameraDistance;

        // 7. 원하는 카메라 위치 계산 (바운딩 박스 중심에서 뒤쪽으로)
        _vector forwardDir = XMVector3Normalize(boundsCenter - currentCamPos);
        forwardDir = XMVectorSetY(forwardDir, 0); // Y축 제거 (수평 방향만)

        _vector desiredCamPos = boundsCenter - XMVector3Normalize(forwardDir) * requiredDistance;
        desiredCamPos = XMVectorSetY(desiredCamPos, m_fFixedHeight); // Y 고정

        // 8. 데드존 체크 - 현재 카메라에서 바운딩 박스가 화면 밖으로 나가는지 확인
        _vector camToBounds = boundsCenter - currentCamPos;
        _float camToBoundsX = XMVectorGetX(camToBounds);
        _float camToBoundsZ = XMVectorGetZ(camToBounds);

        // 데드존을 벗어났을 때만 카메라 이동
        _vector moveOffset = XMVectorZero();

        if (abs(camToBoundsX) > deadZoneWidth) {
            _float moveX = (abs(camToBoundsX) - deadZoneWidth) * (camToBoundsX > 0 ? 1 : -1);
            moveOffset = XMVectorSetX(moveOffset, moveX);
        }

        if (abs(camToBoundsZ) > deadZoneWidth) {
            _float moveZ = (abs(camToBoundsZ) - deadZoneWidth) * (camToBoundsZ > 0 ? 1 : -1);
            moveOffset = XMVectorSetZ(moveOffset, moveZ);
        }

        // 9. 최종 카메라 위치 계산
        if (XMVectorGetX(XMVector3Length(moveOffset)) > 0.1f) {
            desiredCamPos = currentCamPos + moveOffset;
            desiredCamPos = XMVectorSetY(desiredCamPos, m_fFixedHeight);
        }
        else {
            desiredCamPos = currentCamPos; // 데드존 내부면 이동하지 않음
        }

        // 10. 거리 체크 - 너무 가까우면 뒤로 이동
        _float currentDistance = XMVectorGetX(XMVector3Length(boundsCenter - desiredCamPos));
        if (currentDistance < requiredDistance) {
            _vector toCam = XMVector3Normalize(desiredCamPos - boundsCenter);
            toCam = XMVectorSetY(toCam, 0); // Y축 제거
            desiredCamPos = boundsCenter + XMVector3Normalize(toCam) * requiredDistance;
            desiredCamPos = XMVectorSetY(desiredCamPos, m_fFixedHeight);
        }

        _float t = min(m_cameraSmooth * fTimeDelta, 1.0f);
        _vector lerpPos = XMVectorLerp(currentCamPos, desiredCamPos, t);

        m_pTransformCom->Set_State(STATE::POSITION, lerpPos);

        // 12. 카메라가 바라볼 지점 (두 캐릭터의 중점, 약간 높이 조정)
        _vector lookAtPoint = boundsCenter;
        lookAtPoint = XMVectorSetY(lookAtPoint, XMVectorGetY(boundsCenter) + 3.0f);

        m_pTransformCom->LookAtXZ(lookAtPoint);
    }
    else
    {
     
		//const _float fMaxDistance = 80.f; // 최대 거리 제한
		//_vector vDir = XMVectorSet(0.f, 0.f, 1.f, 0.f); // 기본 뒤쪽 방향
  //      _vector playerPos = m_pTargetTransform->Get_State(STATE::POSITION);
		//_vector idealCamPos = playerPos - vDir *60.f;
	
		//_vector currentCamPos = m_pTransformCom->Get_State(STATE::POSITION);
		//_float currentDistance = XMVectorGetZ(XMVector3Length(playerPos - currentCamPos));
		//if (currentDistance > fMaxDistance)
		//{
		//	idealCamPos = playerPos - vDir * fMaxDistance;
		//}
	 //  
  //      idealCamPos = XMVector3TransformCoord(idealCamPos, XMMatrixRotationX(m_cameraPitch));
  //      idealCamPos = XMVectorSetY(idealCamPos, m_fSingleCameraFixedHeight); // Y축 고정

		//XMVECTOR currentCameraPos = m_pTransformCom->Get_State(STATE::POSITION);
		//_float t = min(1.f, m_cameraSmooth * fTimeDelta);
		//idealCamPos = XMVectorLerp(currentCameraPos, idealCamPos, t);

  //      if (m_bShaking)
  //      {
  //          _float t = m_fShakeTimeLeft / m_fShakeDuration;

  //          // 진폭을 시간이 지날수록 점점 줄이기
  //          _float currentAmp = m_fShakeAmplitude * t;

  //          // 랜덤 단위 벡터에 진폭 곱하기
  //          _float offX = m_pGameInstance->Compute_Random(-1.f, 1.f) * currentAmp;
  //          _float offY = m_pGameInstance->Compute_Random(-1.f, 1.f) * currentAmp;
  //          _float offZ = m_pGameInstance->Compute_Random(-1.f, 1.f) * currentAmp;

  //          idealCamPos = XMVectorAdd(idealCamPos,
  //              XMVectorSet(offX, offY, offZ, 0.f));

  //          // 타이머 감소
  //          m_fShakeTimeLeft -= fTimeDelta;
  //          if (m_fShakeTimeLeft <= 0.f)
  //              m_bShaking = false;
  //      }

		//m_pTransformCom->Set_State(STATE::POSITION, idealCamPos);

        m_vLastPlayerPos = m_pTargetTransform->Get_State(STATE::POSITION);

            const float fixedHeight = 15.f;

            // 1) 플레이어 현재 위치 & 이동량
            _vector currPos = m_pTargetTransform->Get_State(STATE::POSITION);
            _vector delta = currPos - m_vLastPlayerPos;
            delta = XMVectorSetY(delta, 0.f);

            // 2) 이동량이 충분하면 방향 업데이트
            float dist = XMVectorGetX(XMVector3Length(delta));
            if (dist > 0.01f)
                m_vMoveDir = XMVector3Normalize(delta);

            m_vLastPlayerPos = currPos;

            _vector idealCamPos = currPos - m_vMoveDir * m_cameraLookDistance;
            idealCamPos = XMVectorSetY(idealCamPos, fixedHeight);

            if (m_bShaking)
            {
                _float t = m_fShakeTimeLeft / m_fShakeDuration;

                // 진폭을 시간이 지날수록 점점 줄이기
                _float currentAmp = m_fShakeAmplitude * t;

                // 랜덤 단위 벡터에 진폭 곱하기
                _float offX = m_pGameInstance->Compute_Random(-1.f, 1.f) * currentAmp;
                _float offY = m_pGameInstance->Compute_Random(-1.f, 1.f) * currentAmp;
                _float offZ = m_pGameInstance->Compute_Random(-1.f, 1.f) * currentAmp;

                idealCamPos = XMVectorAdd(idealCamPos,
                    XMVectorSet(offX, offY, offZ, 0.f));

                // 타이머 감소
                m_fShakeTimeLeft -= fTimeDelta;
                if (m_fShakeTimeLeft <= 0.f)
                    m_bShaking = false;
            }
            _vector currCamPos = m_pTransformCom->Get_State(STATE::POSITION);
            float t = min(1.f, m_cameraSmooth * fTimeDelta);
            
            _vector lerpPos = XMVectorLerp(currCamPos, idealCamPos, t);
            
            m_pTransformCom->Set_State(STATE::POSITION, lerpPos);
            _vector lookAt = currPos + XMVectorSet(0, 3, 0, 0);
            m_pTransformCom->LookAtXZ(lookAt);
    }
}

void CThirdPersonCamera::OnHit(CGameObject* attacker, CGameObject* target, _vector offset, _float fDuration)
{
    if (m_bHitCam)
        return;
    m_pHitAttacker = attacker; 
    m_pHitTarget = target;  
    m_vHitOffset = offset;           
    m_fHitDuration = fDuration;
    m_fHitTime = 0.f;
    m_bHitCam = true;
	Safe_AddRef(m_pHitAttacker);
	Safe_AddRef(m_pHitTarget);

 /*   if ((m_pLockOnTarget && m_pLockOnTarget != m_pTarget) == false)
    {
        m_vSingleOrginPos = m_pTransformCom->Get_State(STATE::POSITION);
   
    }*/
}

void CThirdPersonCamera::InitializeCameraPosition()
{
    if (!m_pTargetTransform)
        return;

    _vector playerPos = m_pTargetTransform->Get_State(STATE::POSITION);
    _vector playerForward = m_pTargetTransform->Get_State(STATE::LOOK);
    playerForward = XMVectorSetY(playerForward, 0.0f);

    _vector initialCamPos = playerPos - playerForward * 700.f;
    initialCamPos = XMVectorSetY(initialCamPos, 17.f);

    m_pTransformCom->Set_State(STATE::POSITION, initialCamPos);

}

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
