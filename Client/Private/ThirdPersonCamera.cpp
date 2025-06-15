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
    XMVECTOR currentCamPos = m_pTransformCom->Get_State(STATE::POSITION);
    if (m_pLockOnTarget && m_pLockOnTarget != m_pTarget)
    {
        XMVECTOR playerPos = m_pTargetTransform->Get_State(STATE::POSITION);
        XMVECTOR enemyPos = m_pLockOnTarget->GetTransform()->Get_State(STATE::POSITION);

        // 1. 두 캐릭터의 중점과 거리 계산
        XMVECTOR midPoint = XMVectorLerp(playerPos, enemyPos, 0.5f);
        float distance = XMVectorGetX(XMVector3Length(enemyPos - playerPos));

        // 2. 고정된 카메라 높이와 기본 거리 설정
        //float fixedCameraHeight = 15.0f;  // 고정된 Y 위치
        float baseCameraDistance = max(m_fMinCameraDistance, distance * m_fDistanceMultiplier); // 거리에 따른 기본 카메라 거리

        // 3. 현재 카메라 위치
        XMVECTOR currentCamPos = m_pTransformCom->Get_State(STATE::POSITION);

        // 4. 데드존 설정 (화면상의 영역)
        float deadZoneWidth = 30.f;   // 좌우 데드존
        float deadZoneHeight = 10.0f;  // 상하 데드존

        // 5. 두 캐릭터를 모두 포함하는 바운딩 박스 계산
        XMVECTOR boundsMin = XMVectorMin(playerPos, enemyPos);
        XMVECTOR boundsMax = XMVectorMax(playerPos, enemyPos);
        XMVECTOR boundsCenter = (boundsMin + boundsMax) * 0.5f;

        // Y는 고정
        boundsCenter = XMVectorSetY(boundsCenter, XMVectorGetY(midPoint));

        // 6. 바운딩 박스 크기에 따른 카메라 거리 조정
        float boundsWidth = XMVectorGetX(boundsMax) - XMVectorGetX(boundsMin);
        float boundsDepth = XMVectorGetZ(boundsMax) - XMVectorGetZ(boundsMin);
        float requiredDistance = max(boundsWidth, boundsDepth) * 0.6f + baseCameraDistance;

        // 7. 원하는 카메라 위치 계산 (바운딩 박스 중심에서 뒤쪽으로)
        XMVECTOR forwardDir = XMVector3Normalize(boundsCenter - currentCamPos);
        forwardDir = XMVectorSetY(forwardDir, 0); // Y축 제거 (수평 방향만)

        XMVECTOR desiredCamPos = boundsCenter - XMVector3Normalize(forwardDir) * requiredDistance;
        desiredCamPos = XMVectorSetY(desiredCamPos, m_fFixedHeight); // Y 고정

        // 8. 데드존 체크 - 현재 카메라에서 바운딩 박스가 화면 밖으로 나가는지 확인
        XMVECTOR camToBounds = boundsCenter - currentCamPos;
        float camToBoundsX = XMVectorGetX(camToBounds);
        float camToBoundsZ = XMVectorGetZ(camToBounds);

        // 데드존을 벗어났을 때만 카메라 이동
        XMVECTOR moveOffset = XMVectorZero();

        if (abs(camToBoundsX) > deadZoneWidth) {
            float moveX = (abs(camToBoundsX) - deadZoneWidth) * (camToBoundsX > 0 ? 1 : -1);
            moveOffset = XMVectorSetX(moveOffset, moveX);
        }

        if (abs(camToBoundsZ) > deadZoneWidth) {
            float moveZ = (abs(camToBoundsZ) - deadZoneWidth) * (camToBoundsZ > 0 ? 1 : -1);
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
        float currentDistance = XMVectorGetX(XMVector3Length(boundsCenter - desiredCamPos));
        if (currentDistance < requiredDistance) {
            XMVECTOR toCam = XMVector3Normalize(desiredCamPos - boundsCenter);
            toCam = XMVectorSetY(toCam, 0); // Y축 제거
            desiredCamPos = boundsCenter + XMVector3Normalize(toCam) * requiredDistance;
            desiredCamPos = XMVectorSetY(desiredCamPos, m_fFixedHeight);
        }

        // 11. 스무스 보간
        float t = min(m_fSmooth * fTimeDelta, 1.0f);
        XMVECTOR lerpPos = XMVectorLerp(currentCamPos, desiredCamPos, t);

        m_pTransformCom->Set_State(STATE::POSITION, lerpPos);

        // 12. 카메라가 바라볼 지점 (두 캐릭터의 중점, 약간 높이 조정)
        XMVECTOR lookAtPoint = boundsCenter;
        lookAtPoint = XMVectorSetY(lookAtPoint, XMVectorGetY(boundsCenter) + 3.0f);

        m_pTransformCom->LookAtXZ(lookAtPoint);
    }
    else
    {
     
        XMVECTOR playerForward;

        XMVECTOR playerPos = m_pTargetTransform->Get_State(STATE::POSITION);
            playerForward = m_pTargetTransform->Get_State(STATE::LOOK);

        playerForward = XMVectorSetY(playerForward, 0.0f); // 수평만

        // 2. 플레이어 뒤쪽 방향
        XMVECTOR playerBackward = -playerForward;

        _float cameraDistance = m_fMinCameraDistance;
        XMVECTOR idealCamPos = playerPos + playerBackward * cameraDistance;
        idealCamPos = XMVectorSetY(idealCamPos, m_fFixedHeight);

        // 4. 데드존 체크
        XMVECTOR camToPlayer = playerPos - currentCamPos;
        _float horizontalDistance = XMVectorGetX(XMVector3Length(
            XMVectorSet(XMVectorGetX(camToPlayer), 0, XMVectorGetZ(camToPlayer), 0)
        ));

        _float deadZoneRadius =100.0f; // 싱글 타겟용 데드존 (더 작게)

        XMVECTOR targetCamPos;

        if (horizontalDistance > deadZoneRadius)
        {
            // 플레이어 방향으로 데드존 경계까지 이동
            XMVECTOR dirToPlayer = XMVector3Normalize(camToPlayer);
            dirToPlayer = XMVectorSetY(dirToPlayer, 0);

            XMVECTOR deadZoneEdge = currentCamPos + dirToPlayer * (horizontalDistance - deadZoneRadius);
            targetCamPos = XMVectorSetY(deadZoneEdge, m_fFixedHeight);
        }
        else
        {
            targetCamPos = currentCamPos;
        }

        // 5. 최소 거리 유지
        float currentDistanceToPlayer = XMVectorGetZ(XMVector3Length(playerPos - targetCamPos));
        if (currentDistanceToPlayer < cameraDistance * 1.2f)
        {
            XMVECTOR awayFromPlayer = XMVector3Normalize(targetCamPos - playerPos);
            awayFromPlayer = XMVectorSetY(awayFromPlayer, 0);
            targetCamPos = playerPos + awayFromPlayer * (cameraDistance * 1.2f);
            targetCamPos = XMVectorSetY(targetCamPos, m_fFixedHeight);
        }

        // 6. 스무스 보간
        float smoothFactor = min(m_fSmooth * fTimeDelta * 1.5f, 1.0f); // 싱글 모드에서 조금 더 빠르게
        XMVECTOR finalCamPos = XMVectorLerp(currentCamPos, targetCamPos, smoothFactor);

        m_pTransformCom->Set_State(STATE::POSITION, finalCamPos);

        // 7. 카메라 시선 - 플레이어를 바라보기
        XMVECTOR lookAtPoint = playerPos;
        lookAtPoint = XMVectorSetY(lookAtPoint, XMVectorGetY(playerPos) + 2.0f);
        m_pTransformCom->LookAtXZ(lookAtPoint);
    }
}

void CThirdPersonCamera::InitializeCameraPosition()
{
    if (!m_pTargetTransform)
        return;

    XMVECTOR playerPos = m_pTargetTransform->Get_State(STATE::POSITION);
    XMVECTOR playerForward = m_pTargetTransform->Get_State(STATE::LOOK);
    playerForward = XMVectorSetY(playerForward, 0.0f);

    XMVECTOR initialCamPos = playerPos - playerForward * 900.f;
    initialCamPos = XMVectorSetY(initialCamPos, 18.f);

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
