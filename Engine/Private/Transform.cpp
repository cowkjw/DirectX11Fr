#include "Transform.h"
#include "GameObject.h"
#include "Shader.h"

CTransform::CTransform(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent{ pDevice, pContext }
{
}

CTransform::CTransform(const CTransform& Prototype)
	: CComponent( Prototype )
	, m_WorldMatrix{ Prototype.m_WorldMatrix }
	, m_bDirty{ Prototype.m_bDirty }
{
}

_float3 CTransform::Get_Scaled()
{
	return _float3(XMVectorGetX(XMVector3Length(Get_State(STATE::RIGHT))),
		XMVectorGetX(XMVector3Length(Get_State(STATE::UP))),
		XMVectorGetX(XMVector3Length(Get_State(STATE::LOOK))));
}

HRESULT CTransform::Initialize_Prototype()
{
	XMStoreFloat4x4(&m_WorldMatrix, XMMatrixIdentity());

	return S_OK;
}

HRESULT CTransform::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return S_OK;

	TRANSFORM_DESC* pDesc = static_cast<TRANSFORM_DESC*>(pArg);

	m_fSpeedPerSec = pDesc->fSpeedPerSec;
	m_fRotationPerSec = pDesc->fRotationPerSec;

	return S_OK;
}

void CTransform::Scaling(_float fX, _float fY, _float fZ)
{
	Set_State(STATE::RIGHT, XMVector3Normalize(Get_State(STATE::RIGHT)) * fX);
	Set_State(STATE::UP, XMVector3Normalize(Get_State(STATE::UP)) * fY);
	Set_State(STATE::LOOK, XMVector3Normalize(Get_State(STATE::LOOK)) * fZ);
}

void CTransform::Scaling(const _float3& vScale)
{
	Set_State(STATE::RIGHT, XMVector3Normalize(Get_State(STATE::RIGHT)) * vScale.x);
	Set_State(STATE::UP, XMVector3Normalize(Get_State(STATE::UP)) * vScale.y);
	Set_State(STATE::LOOK, XMVector3Normalize(Get_State(STATE::LOOK)) * vScale.z);
}

void CTransform::Go_Straight(_float fTimeDelta)
{
	_vector		vPosition = Get_State(STATE::POSITION);
	_vector		vLook = Get_State(STATE::LOOK);

	vPosition += XMVector3Normalize(vLook) * m_fSpeedPerSec * fTimeDelta;

	Set_State(STATE::POSITION, vPosition);
	m_bDirty = true;
}

void CTransform::Go_Backward(_float fTimeDelta)
{
	_vector		vPosition = Get_State(STATE::POSITION);
	_vector		vLook = Get_State(STATE::LOOK);

	vPosition -= XMVector3Normalize(vLook) * m_fSpeedPerSec * fTimeDelta;

	Set_State(STATE::POSITION, vPosition);
	m_bDirty = true;
}

void CTransform::Go_Right(_float fTimeDelta)
{
	_vector		vPosition = Get_State(STATE::POSITION);
	_vector		vRight = Get_State(STATE::RIGHT);

	vPosition += XMVector3Normalize(vRight) * m_fSpeedPerSec * fTimeDelta;

	Set_State(STATE::POSITION, vPosition);
	m_bDirty = true;
}

void CTransform::Go_Left(_float fTimeDelta)
{
	_vector		vPosition = Get_State(STATE::POSITION);
	_vector		vRight = Get_State(STATE::RIGHT);

	vPosition -= XMVector3Normalize(vRight) * m_fSpeedPerSec * fTimeDelta;

	Set_State(STATE::POSITION, vPosition);
	m_bDirty = true;
}

void CTransform::Follow_Target(_fvector vTarget, _float fTimeDelta, _float fMinDistance)
{
	_vector		vMoveDir = vTarget - Get_State(STATE::POSITION);

	if (fMinDistance <= XMVectorGetX(XMVector3Length(vMoveDir)))
		Set_State(STATE::POSITION, Get_State(STATE::POSITION) + XMVector3Normalize(vMoveDir) * m_fSpeedPerSec * fTimeDelta);
	m_bDirty = true;
}

void CTransform::Rotate_EulerAngles(const _float3& vEulerAngles)
{
	m_vEulerAngles = vEulerAngles;

	// 2) Degree → Radian 변환 후 쿼터니언 생성
	XMVECTOR q = XMQuaternionRotationRollPitchYaw(
		XMConvertToRadians(vEulerAngles.x),  // Pitch (X축 회전)
		XMConvertToRadians(vEulerAngles.y),  // Yaw   (Y축 회전)
		XMConvertToRadians(vEulerAngles.z)   // Roll  (Z축 회전)
	);

	// 3) 쿼터니언 → 회전 행렬
	XMMATRIX matRotation = XMMatrixRotationQuaternion(q);

	// 4) 현재 스케일 가져오기
	_float3 vScale = Get_Scaled();

	// 5) 로컬 축 벡터(1,0,0),(0,1,0),(0,0,1)를 회전 & 스케일
	XMVECTOR right = XMVector3TransformNormal(XMVectorSet(1, 0, 0, 0), matRotation) * vScale.x;
	XMVECTOR up = XMVector3TransformNormal(XMVectorSet(0, 1, 0, 0), matRotation) * vScale.y;
	XMVECTOR look = XMVector3TransformNormal(XMVectorSet(0, 0, 1, 0), matRotation) * vScale.z;

	// 6) 상태 벡터에 반영
	Set_State(STATE::RIGHT, right);
	Set_State(STATE::UP, up);
	Set_State(STATE::LOOK, look);

	// 7) 월드매트릭스 재생성 플래그
	m_bDirty = true;
}

void CTransform::Turn(_fvector vAxis, _float fTimeDelta)
{
	_matrix			RotationMatrix = XMMatrixRotationAxis(vAxis, m_fRotationPerSec * fTimeDelta);

	Set_State(STATE::RIGHT, XMVector4Transform(Get_State(STATE::RIGHT), RotationMatrix));
	Set_State(STATE::UP, XMVector4Transform(Get_State(STATE::UP), RotationMatrix));
	Set_State(STATE::LOOK, XMVector4Transform(Get_State(STATE::LOOK), RotationMatrix));
	m_bDirty = true;
}

void CTransform::RotateToDirection(_fvector dir)
{
	if (XMVector3Equal(dir, XMVectorZero()))
		return;

	// 2) 정규화된 목표 방향
	_vector target = XMVector3Normalize(dir);

	// 3) 현재 전방(LOOK) 벡터
	_vector forward = XMVector3Normalize(Get_State(STATE::LOOK));

	// 4) 축(axis) = forward × target
	_vector axis = XMVector3Cross(forward, target);

	// 5) 평행(또는 반평행) 체크
	if (XMVector3Equal(axis, XMVectorZero()))
	{
		// 반평행(180°)이면 Up 축을 사용, 같은 방향이면 회전 불필요
		_float d = XMVectorGetX(XMVector3Dot(forward, target));
		if (d < 0.f)
			axis = XMVectorSet(0.f, 1.f, 0.f, 0.f);
		else
			return;
	}

	axis = XMVector3Normalize(axis);

	// 6) 회전 각도 계산
	_float dot = XMVectorGetX(XMVector3Dot(forward, target));
	dot = dot < -1.f ? -1.f : (dot > 1.f ? 1.f : dot);
	_float angle = acosf(dot);

	// 7) 쿼터니언 회전 매트릭스
	_vector q = XMQuaternionRotationAxis(axis, angle);
	_matrix R = XMMatrixRotationQuaternion(q);

	// 8) 현재 로컬 축(ROW)들에 적용
	_vector newRight = XMVector4Transform(Get_State(STATE::RIGHT), R);
	_vector newUp = XMVector4Transform(Get_State(STATE::UP), R);
	_vector newLook = XMVector4Transform(Get_State(STATE::LOOK), R);

	Set_State(STATE::RIGHT, newRight);
	Set_State(STATE::UP, newUp);
	Set_State(STATE::LOOK, newLook);

	m_bDirty = true;
}

void CTransform::FlllowParent(const CTransform* pParentTransform)
{
	if (!pParentTransform) return;

 // 1) 부모 월드
	XMMATRIX parentW = XMLoadFloat4x4(&pParentTransform->m_WorldMatrix);
	// 2) 저장된 로컬
	XMMATRIX localM = XMLoadFloat4x4(&m_LocalMatrix);
	// 3) 월드 = local × 부모월드
	XMMATRIX worldM = XMMatrixMultiply(localM, parentW);
	// 4) 결과 저장
	XMStoreFloat4x4(&m_WorldMatrix, worldM);
}

json CTransform::Serialize()
{
	json j = CComponent::Serialize();
	XMFLOAT3 p;
	XMStoreFloat3(&p, Get_State(STATE::POSITION));
	j["Position"] = { p.x, p.y, p.z };


	
	j["Rotation"] = { m_vEulerAngles.x, m_vEulerAngles.y, m_vEulerAngles.z };

	XMFLOAT3 s = Get_Scaled();
	j["Scale"] = { s.x, s.y, s.z };


	return j;
}

void CTransform::Deserialize(const json& j)
{
	// 월드 매트릭스 초기화
	XMStoreFloat4x4(&m_WorldMatrix, XMMatrixIdentity());

	_matrix s{}, r{}, t{}, w{};
	// 2) 스케일 복원 (축 방향을 정규화한 뒤, 스칼라로 스케일)
	if (j.contains("Scale"))
	{
		_float3 vScale = {
			j["Scale"][0].get<_float>(),
			j["Scale"][1].get<_float>(),
			j["Scale"][2].get<_float>()
		};
		s = XMMatrixScaling(vScale.x, vScale.y, vScale.z);
	//	Scaling(vScale);
	}

	// 2) 회전 복원



	// 1) 위치 복원
	if (j.contains("Position"))
	{
		_float3 vPosition = {
			j["Position"][0].get<_float>(),
			j["Position"][1].get<_float>(),
			j["Position"][2].get<_float>()
		};

		//XMVECTOR vecPosition = XMLoadFloat3(&vPosition);

		//// 3) w를 1.0f 로 설정해야 할 경우
		//vecPosition = XMVectorSetW(vecPosition, 1.0f);
		//Set_State(STATE::POSITION, vecPosition);

		t = XMMatrixTranslation(
			vPosition.x, vPosition.y, vPosition.z
		);
	}

	

	if (j.contains("Rotation"))
	{
		_float3 vRotation = {
			j["Rotation"][0].get<_float>(),
			j["Rotation"][1].get<_float>(),
			j["Rotation"][2].get<_float>()
		};
		r = XMMatrixRotationRollPitchYaw(
			XMConvertToRadians(vRotation.x),
			XMConvertToRadians(vRotation.y),
			XMConvertToRadians(vRotation.z)
		);
			m_vEulerAngles = vRotation;

		w = s * r * t;
	}
	else
	{
		// 회전이 없을 경우
		w= s * t;
	}
	XMStoreFloat4x4(&m_WorldMatrix, w);

}

void CTransform::Set_Parent(CGameObject* pParent)
{
	if (pParent)
	{
		auto pTransform = pParent->GetTransform();

		if (pTransform)
		{
			XMMATRIX parentW = XMLoadFloat4x4(&pTransform->m_WorldMatrix);
			// 3) 내 현재(세계) 월드 행렬 불러오기
			XMMATRIX myW = XMLoadFloat4x4(&m_WorldMatrix);
			// 4) 로컬 행렬 = myW × inv(parentW)
			XMMATRIX invPW = pTransform->Get_WorldMatrix_Inverse();
			XMMATRIX localM = XMMatrixMultiply(myW, invPW);
			// 5) m_LocalMatrix 에 저장
			XMStoreFloat4x4(&m_LocalMatrix, localM);
		}
	}
}

_vector CTransform::Get_RotationQuaternion() const
{
	_vector scale, quat, trans;
	XMMatrixDecompose(&scale, &quat, &trans, XMLoadFloat4x4(&m_WorldMatrix));
	return quat;  // 쿼터니언 회전 반환
}

void CTransform::Set_RotationQuaternion(_fvector vQuat)
{
	XMVECTOR quat = XMQuaternionNormalize(vQuat);

	// 2) 쿼터니언→회전 행렬
	XMMATRIX rotM = XMMatrixRotationQuaternion(quat);

	// 3) (옵션) 스케일, 위치 행렬 구성
	_float3 vScale = Get_Scaled();
	XMVECTOR scale = XMLoadFloat3(&vScale);
	XMVECTOR trans = Get_State(STATE::POSITION);

	XMMATRIX scaleM = XMMatrixScalingFromVector(scale);
	XMMATRIX transM = XMMatrixTranslationFromVector(trans);

	// 스케일 → 회전 → 위치 순서로 곱함
	XMMATRIX worldM = scaleM * rotM * transM;

	// 4) 최종 월드 매트릭스 저장
	XMStoreFloat4x4(&m_WorldMatrix, worldM);
}

void CTransform::LookAt(_fvector vAt)
{
	_float3		vScaled = Get_Scaled();

	_vector		vLook = vAt - Get_State(STATE::POSITION);
	_vector		vRight = XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), vLook);
	_vector		vUp = XMVector3Cross(vLook, vRight);

	Set_State(STATE::RIGHT, XMVector3Normalize(vRight) * vScaled.x);
	Set_State(STATE::UP, XMVector3Normalize(vUp) * vScaled.y);
	Set_State(STATE::LOOK, XMVector3Normalize(vLook) * vScaled.z);
	m_bDirty = true;
}

void CTransform::UpdateEulerAngles()
{
	_vector scale, quat, translation;
	XMMatrixDecompose(&scale, &quat, &translation, XMLoadFloat4x4(&m_WorldMatrix));

	// 2) 쿼터니언을 XMFLOAT4 로
	_float4 q;
	XMStoreFloat4(&q, quat);

	// 3) quaternion → Euler (radian) (Pitch=X, Yaw=Y, Roll=Z)

	_float sinr_cosp = 2.0f * (q.w * q.x + q.y * q.z);
	_float cosr_cosp = 1.0f - 2.0f * (q.x * q.x + q.y * q.y);
	_float roll = atan2f(sinr_cosp, cosr_cosp);

	_float sinp = 2.0f * (q.w * q.y - q.z * q.x);
	_float pitch;
	if (fabsf(sinp) >= 1.0f)
		pitch = copysignf(XM_PI / 2.0f, sinp); // gimbal lock
	else
		pitch = asinf(sinp);

	_float siny_cosp = 2.0f * (q.w * q.z + q.x * q.y);
	_float cosy_cosp = 1.0f - 2.0f * (q.y * q.y + q.z * q.z);
	_float yaw = atan2f(siny_cosp, cosy_cosp);

	// 4) 라디안 → 도(°) 변환해서 저장
	m_vEulerAngles.x = XMConvertToDegrees(pitch);
	m_vEulerAngles.y = XMConvertToDegrees(yaw);
	m_vEulerAngles.z = XMConvertToDegrees(roll);
}

HRESULT CTransform::Bind_ShaderResource(CShader* pShader, const _char* pConstantName)
{
	return pShader->Bind_Matrix(pConstantName, &m_WorldMatrix);
}

CTransform* CTransform::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CTransform* pInstance = new CTransform(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CTransform");
		Safe_Release(pInstance);
	}

	return pInstance;
}


CComponent* CTransform::Clone(void* pArg)
{
	CTransform* pInstance = new CTransform(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CTransform");
		Safe_Release(pInstance);
	}

	return pInstance;

}

void CTransform::Free()
{
	__super::Free();

}
