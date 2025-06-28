#include "CutSceneCamera.h"
#include "GameInstance.h"

CCutSceneCamera::CCutSceneCamera(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCamera{ pDevice, pContext }
	, m_fDuration{ 0.f }
	, m_fElapsedTime{ 0.f }
	, m_iCurrentFrame{ 0 }
	, m_bLoop{ false }
	, m_pTargetObject{ nullptr }
	, m_bMainpulate{ false }
	, m_fMouseSensor{ 0.1f } // 기본 마우스 센서 값
{
}

CCutSceneCamera::CCutSceneCamera(const CCutSceneCamera& Prototype)
	: CCamera(Prototype)
	, m_fDuration{ Prototype.m_fDuration }
	, m_fElapsedTime{ Prototype.m_fElapsedTime }
	, m_iCurrentFrame{ Prototype.m_iCurrentFrame }
	, m_KeyFrames{ Prototype.m_KeyFrames }
	, m_bLoop{ Prototype.m_bLoop }
	, m_pTargetObject{ Prototype.m_pTargetObject }
	, m_bMainpulate{ Prototype.m_bMainpulate }
	, m_fMouseSensor{ Prototype.m_fMouseSensor }
{
	Safe_AddRef(m_pTargetObject);
}
HRESULT CCutSceneCamera::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CCutSceneCamera::Initialize(void* pArg)
{
	CCamera::CAMERA_DESC			Desc{};

	Desc.vEye = _float3(0.f, 20.f, -15.f);
	Desc.vAt = _float3(0.f, 0.f, 0.f);
	Desc.fFov = XMConvertToRadians(60.0f);
	Desc.fNear = 0.1f;
	Desc.fFar = 1000.f;
	Desc.fRotationPerSec = XMConvertToRadians(180.0f);
	Desc.fSpeedPerSec = 25.0f;
	Desc.strName = TEXT("CutSceneCamera");
	m_fMouseSensor = 0.1f;

	if (FAILED(__super::Initialize(&Desc)))
		return E_FAIL;

	return S_OK;
}

void CCutSceneCamera::Update(_float fTimeDelta)
{
	if (m_bMainpulate)
	{
		if (m_pGameInstance->IsKeyDown('W'))
		{
			m_pTransformCom->Go_Straight(fTimeDelta);
		}
		if (m_pGameInstance->IsKeyDown('S'))
		{
			m_pTransformCom->Go_Backward(fTimeDelta);
		}
		if (m_pGameInstance->IsKeyDown('A'))
		{
			m_pTransformCom->Go_Left(fTimeDelta);
		}
		if (m_pGameInstance->IsKeyDown('D'))
		{
			m_pTransformCom->Go_Right(fTimeDelta);
		}

		if (m_pGameInstance->IsMouseDown(1))
		{
			POINT vMouseDelta = m_pGameInstance->GetMouseDelta();
			_long			MouseMove = {};

			if (vMouseDelta.x != 0)
			{
				_float fYawAngle = vMouseDelta.x * fTimeDelta * m_fMouseSensor;
				// Up 축 (0,1,0) 기준으로 Yaw
				m_pTransformCom->Turn(
					XMVectorSet(0.f, 1.f, 0.f, 0.f),
					fYawAngle
				);
			}

			if (vMouseDelta.y != 0)
			{
				_float fPitchAngle = vMouseDelta.y * fTimeDelta * m_fMouseSensor;
				// Right 축 기준으로 Pitch
				m_pTransformCom->Turn(
					m_pTransformCom->Get_State(STATE::RIGHT),
					fPitchAngle
				);
			}
		}
	}
	__super::Update_Camera();
}

void CCutSceneCamera::Late_Update(_float fTimeDelta)
{
	// 재생 중이면 쭉 계속 처리
	if (m_bIsPlaying)
	{
		m_fElapsedTime += fTimeDelta;
		UpdateCameraTransform(fTimeDelta);
	}
	else 
	{
		// 툴에서 사용 안하고 있으면 지금 프레임으로 보기
		if (m_iCurrentFrame < m_KeyFrames.size()&& m_bMainpulate == false)
		{
			const CamKeyFrame& keyFrame = m_KeyFrames[m_iCurrentFrame];
			m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&keyFrame.vPosition),1.f));
			if (m_pTargetObject)
			{
				_vector vTargetPos = m_pTargetObject->GetTransform()->Get_State(STATE::POSITION);
				m_pTransformCom->LookAt(vTargetPos);
			}
		}
	}
}

void CCutSceneCamera::ResetCamera()
{
	if (!m_KeyFrames.empty())
	{
		m_iCurrentFrame = 0;
		m_fElapsedTime = 0.f;
		const CamKeyFrame& keyFrame = m_KeyFrames[m_iCurrentFrame];
		m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&keyFrame.vPosition), 1.f));
		if (m_pTargetObject)
		{
			_vector vTargetPos = m_pTargetObject->GetTransform()->Get_State(STATE::POSITION);
			m_pTransformCom->LookAt(vTargetPos);
		}
	}
}

void CCutSceneCamera::SetKeyFrames(const vector<CamKeyFrame>& keyFrames, _float fDuration)
{
	m_KeyFrames = keyFrames;
	m_fDuration = fDuration;
	m_iCurrentFrame = 0;
	m_fElapsedTime = 0.f;
}

void CCutSceneCamera::SetCurrentFrame(_uint iFrameIndex)
{
	// 현재 프레임으로 설정 툴에서 드래그 해서 사용할 때 사용하기
	if (iFrameIndex < m_KeyFrames.size())
	{
		m_iCurrentFrame = iFrameIndex;
		const CamKeyFrame& keyFrame = m_KeyFrames[m_iCurrentFrame];
		m_pTransformCom->Set_State(STATE::POSITION, XMLoadFloat3(&keyFrame.vPosition));
	}
}

json CCutSceneCamera::Serialize()
{
	json j;
	// 현재 지정된 키 프레임들로 내보내기
	j["KeyFrames"] = json::array();
	for (const auto& keyFrame : m_KeyFrames)
	{
		json frameJson;
		frameJson["Position"] = { keyFrame.vPosition.x, keyFrame.vPosition.y, keyFrame.vPosition.z };
		frameJson["OffSet"] = { keyFrame.vOffset.x, keyFrame.vOffset.y, keyFrame.vOffset.z };
		j["KeyFrames"].push_back(frameJson);
	}
	j["Duration"] = m_fDuration;
	j["IsLoop"] = m_bLoop;

	if (m_pTargetObject)
	{
		j["TargetObjectName"] = m_pTargetObject->Get_Name();
	}
	else
	{
		j["TargetObject"] = nullptr; // 타겟 오브젝트가 없을 경우
	}
	return j;
}

void CCutSceneCamera::UpdateCameraTransform(_float fTimeDelta)
{
	if (m_KeyFrames.size() < 2 || m_fDuration <= 0.f)
		return;

	// 전체 진행률 계산
	float totalProgress = m_fElapsedTime / m_fDuration;

	// 현재 키프레임 구간 찾기
	_float segmentTime = m_fDuration / (m_KeyFrames.size() - 1);
	_int currentIndex = static_cast<int>(totalProgress * (m_KeyFrames.size() - 1));
	currentIndex = min(currentIndex, static_cast<int>(m_KeyFrames.size() - 2));

	// 현재 구간에서의 진행률 계산
	_float segmentProgress = (totalProgress * (m_KeyFrames.size() - 1)) - currentIndex;
	segmentProgress = max(0.f, min(1.f, segmentProgress));

	const CamKeyFrame& currentFrame = m_KeyFrames[currentIndex];
	const CamKeyFrame& nextFrame = m_KeyFrames[currentIndex + 1];

	// 위치 보간
	// 오프셋 처리 
	_vector vOffset = XMLoadFloat3(&currentFrame.vOffset);
	_vector vCurrentPosition = XMLoadFloat3(&currentFrame.vPosition) + vOffset;
	_vector vNextPosition = XMLoadFloat3(&nextFrame.vPosition) + XMLoadFloat3(&nextFrame.vOffset);
	_vector vInterPosition = XMVectorLerp(
		vCurrentPosition,
		vNextPosition,
		segmentProgress
	);

	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(vInterPosition, 1.f));

	// 회전 처리
	if (m_pTargetObject)
	{
		_vector vTargetPos = m_pTargetObject->GetTransform()->Get_State(STATE::POSITION);
		m_pTransformCom->LookAt(vTargetPos);
	}

	// 애니메이션 종료 처리
	if (totalProgress >= 1.f)
	{
		if (m_bLoop)
		{
			m_fElapsedTime = 0.f;
		}
		else
		{
			m_bIsPlaying = false;
			m_fElapsedTime = m_fDuration;
		}
	}
	//if (m_bLoop && m_iCurrentFrame >= m_KeyFrames.size())
	//{
	//	m_iCurrentFrame = 0;
	//}
	//else if (m_iCurrentFrame >= m_KeyFrames.size())
	//{
	//	m_bIsPlaying = false; // 재생이 끝나면 멈춤
	//	m_iCurrentFrame = static_cast<_uint>(m_KeyFrames.size()) - 1;
	//}
}

CCutSceneCamera* CCutSceneCamera::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CCutSceneCamera* pInstance = new CCutSceneCamera(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CCutSceneCamera");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CCutSceneCamera::Clone(void* pArg)
{
	CCutSceneCamera* pInstance = new CCutSceneCamera(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone CCutSceneCamera");
		Safe_Release(pInstance);
		return nullptr;
	}
	return pInstance;
}

void CCutSceneCamera::Free()
{
	CCamera::Free();
	Safe_Release(m_pTargetObject);
	m_KeyFrames.clear();
}
